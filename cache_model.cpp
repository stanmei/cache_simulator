


#include "cache_model.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
/*
 * Class Cacheblock
 */

Cacheblock::Cacheblock(){
	valid=0;
	tag=0;
	timestamp = 0;
	data = 0 ;
}
Cacheblock::~Cacheblock(){};

// Accessor
unsigned long Cacheblock::get_tag() const{return tag;};
unsigned long Cacheblock::get_ts() const{return timestamp;};
int Cacheblock::get_valid() const {return valid;};

// update timestamp/tag fields for new access.
void Cacheblock::update_ts(unsigned long newts){
	timestamp = newts ;
	return ;
}

void Cacheblock::update_tag(unsigned long newtag){
	valid=1;
	tag = newtag ;
	return ;
}

// method : compare tag in block.Return true if matched.
//bool Cacheblock::compare_tag(unsigned long newtag){
//	return (valid==1) && (tag==newtag);
//}


/*
 * Class Cachesets
 */

// Constructor
Cacheset::Cacheset(unsigned int blksnum){
	empty = 1;
	full = 0 ;
	numblks=blksnum ;
	replace_policy="random";
	//cacheset = new  Cacheblock [numblks] ;
	for (int idx=0;idx<numblks;idx++)
		cacheset.push_back(Cacheblock());

	rt_tlru = new SetsBtreeNode();
	int tlru_lvs=log2(numblks)-1;
	init_tlru(*rt_tlru,tlru_lvs);
}
// Destructor
Cacheset::~Cacheset () {
	//delete [] cacheset ;
}


int Cacheset::findfirstempty (){
	//Check set's cache blocks's empty/full status.
	int chk_full=0;
	int chk_empty=1;
	int idx ;
	//cache set empty check via traverse all cacheblocks.
	for (idx=0;idx<numblks;idx++) {
		if ( cacheset[idx].get_valid()==1 ) {
			chk_empty = 0;
			break ;
		}
	}
	empty = chk_empty ;

	//cache set empty check via traverse all cacheblocks.
	for (idx=0;idx<numblks;idx++) {
		if ( cacheset[idx].get_valid()==0 ) {
			chk_full = 0 ;
			break;
		}
	}
	full  = chk_full ;

	if ( (chk_full==0) && (idx<numblks) ) {
		return idx;
	} else {
		return -1;
	}
}

int Cacheset::search(unsigned long tag) const {
	//Check whether there is matched cache blocks.
	for (int idx=0;idx<numblks;idx++) {
//		bool matched=cacheset[idx].compare_tag(tag);
		if ( (cacheset[idx].get_valid()==1) &&
				(cacheset[idx].get_tag()==tag) ) {
				return idx ;
			}
	}

	return -1;
}

int Cacheset::victim_random() {
	srand(time(0));
	return (rand()% numblks);
}

void Cacheset::init_tlru(SetsBtreeNode& node_tlru,int total_lvs) {
	node_tlru.height = total_lvs;
	node_tlru.brch = 0 ;

	if (total_lvs==0) {
		return ;
	}

	total_lvs--;
	node_tlru.leftset = new SetsBtreeNode();
	Cacheset::init_tlru(*node_tlru.leftset,total_lvs);

	node_tlru.rightset = new SetsBtreeNode();
	Cacheset::init_tlru(*node_tlru.rightset,total_lvs);
}

void Cacheset::update_tlru(SetsBtreeNode& node_tlru,int total_lvs,int blkidx){
	if (total_lvs==0) {
		return ;
	}

	total_lvs--;
	if ( (blkidx & ~(0x1<< total_lvs) ) ==0 ){
		node_tlru.brch = 1;
		Cacheset::update_tlru(*node_tlru.leftset,total_lvs,blkidx);
	} else {
		node_tlru.brch = 0;
		Cacheset::update_tlru(*node_tlru.rightset,total_lvs,blkidx);
	}
	//cout <<"--update trlu, lvs: " <<total_lvs << ";blkidx: " << blkidx << "; brch: " << node_tlru.brch <<endl;
	//cout << node_tlru <<endl;
}

void Cacheset::lkup_tlru(SetsBtreeNode& node_tlru,int total_lvs,int& blkidx){
	if (total_lvs==0) {
		return ;
	}

	total_lvs--;
	blkidx += node_tlru.brch <<total_lvs;
	if ( node_tlru.brch == 1 ){
		Cacheset::update_tlru(*node_tlru.rightset,total_lvs,blkidx);
	} else {
		Cacheset::update_tlru(*node_tlru.leftset,total_lvs,blkidx);
	}
}
int Cacheset::victim_tlru() {
	int blkidx=0;

	lkup_tlru(*rt_tlru,log2(numblks),blkidx);

	return blkidx;
}

int Cacheset::access(unsigned long tag,string replace_polcy)  {

	/*
	 * 1) Search matched cache block;
	 * 2) if step 1 fail, select first empty;
	 * 3) if step 2 fail, victim;
	 */
	int rslt_typ = 0;
	int srslt = search (tag);

	if ( srslt == -1 )
	{
		srslt = findfirstempty();
		rslt_typ = 1;
	}

	if (srslt==-1) {
		if (replace_policy=="random") {
			srslt = victim_random() ;
		} else {
			srslt = victim_tlru() ;
		}
		rslt_typ = 2;
	}
	cacheset[srslt].update_tag(tag) ;
	cacheset[srslt].update_ts(time(0)) ;

	//cout << "Type of search set: " << rslt_typ <<" ; srslt : " << srslt <<endl;

	update_tlru(*rt_tlru,log2(numblks),srslt);

	int exp_numblks=log2(numblks);
	return srslt + (rslt_typ << exp_numblks) ;
}

// Constructor
SetsBtreeNode::SetsBtreeNode (){
	brch=0;
	leftset = nullptr;
	rightset= nullptr;
	height= 0;
}
// Destructor
//SetsBtreeNode::~SetsBtreeNode ();

ostream& operator <<(ostream& outs,const SetsBtreeNode& node) {
	cout << "TLRU Node[height:"<<node.height<<"; brch: "<<node.brch<<"; left: "<< node.leftset << "; right: " << node.rightset <<"; ]" <<endl;

	//return 1 ;
}

/*
 * Class cache
 */
	// Constructor
Cache::Cache (int csize,int blksize,int setsize,string rpolicy) {
	replace_policy = rpolicy ;
	cache_size =csize;
	set_size =setsize;
	block_size=blksize;
	occupied_sets=0;

	num_sets = cache_size/set_size;
	int num_blks_set = set_size/block_size;

	cout <<"cache size: "<<cache_size <<";num_sets: " << num_sets << "; num_blks: " << num_blks_set <<endl;
	//csets = new Cacheset [num_sets];
	for (int idx=0;idx<num_sets;idx++)
		csets.push_back(Cacheset(num_blks_set));

	//Counters
	cnt_access = 0 ;
	cnt_hits = 0 ;
	cnt_new = 0 ;
	cnt_replace = 0 ;
}
	// Destructor
Cache::~Cache(){
	//delete [] csets;
}
int Cache::access(unsigned long addr) {
	int tag = addr/set_size;
	int setidx = (addr%cache_size)/set_size;

	int rslt= csets[setidx].access(tag,replace_policy);
	int numblks = set_size/block_size;
	int exp_numblks=log2(numblks);
	int rslt_typ = rslt >> (exp_numblks);
	int blkidx = rslt & ~(0x1>>(32-numblks));

	cnt_access++ ;
	if (rslt_typ ==2) {
		cnt_replace++;
	}else if (rslt_typ==1){
		cnt_new++;
	}else {
		cnt_hits++;
	}

	return (setidx*numblks+blkidx) ;
}

void Cache::print_cache_result() {
	cout<<setw(18)<<" "<<setw(16)<<"total"<<setw(16)<<"hit"<<setw(16)<<"miss" <<setw(16)<<"replace"<<setw(16)<<"new"<<endl;
	cout<<setw(18)<<replace_policy<<setw(16)<<cnt_access<<setw(16)<<cnt_hits<<setw(16)
						<<(cnt_access-cnt_hits)<<setw(16)<<cnt_replace<<setw(16)<<cnt_new<<endl;
}
