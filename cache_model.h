/*
 * cache_model.h
 *
 *  Created on: 2017/11/18
 *      Author: Sitao Mei
 *
 *  Descr :
 *  This is head file containing cache model classes,which
 *  support multi replacement policy:
 *  1) RANDOM;
 *  2) Tree based LRU;
 */

#ifndef CACHE_MODEL_H_
#define CACHE_MODEL_H_

#include <string>
#include <vector>
using namespace std;

/*
 * A class for block of cache *
 */
class Cacheblock {

public:
	// Constructor
	Cacheblock();
	// Destructor
	~Cacheblock();

	// Accessor
	unsigned long get_tag() const;
	unsigned long get_ts() const;
	int get_valid() const;

	// update timestamp/tag fields for new access.
	void update_ts(unsigned long newts);
	void update_tag(unsigned long newtag);

	// method : compare tag in block.Return true if matched.
	// bool compare_tag(unsigned long newtag);

private:
	/*Components of cache block:
	 * tag :  address tag.
	 * timestamp : latest access's timestamp.
	 * valid : 1, this cache block valid.
	 */
	unsigned long data ; // Not used for miss/hit sim.
	unsigned long tag;
	unsigned long timestamp;
	int valid ;
};

class SetsBtreeNode {
public:
	// Constructor
	SetsBtreeNode ();
	// Destructor
	//~SetsBtreeNode ();
	/*
	 * Pointer to left/right node;
	 */
	SetsBtreeNode* leftset;
	SetsBtreeNode* rightset;

	friend ostream& operator <<(ostream& outs,const SetsBtreeNode& node);

	int brch ;
	int height ;
};

/*
 * Cache set class
 */
class Cacheset {
public :
	// Constructor
	Cacheset (unsigned int blksnum);
	// Destructor
	~Cacheset ();

	//Cacheblock *cacheset ;
	vector <Cacheblock> cacheset ;
	// Each Cacheset including assignable cache blocks;
	// Cacheblock *Blk ;
	// Tree based lru root node.
	SetsBtreeNode* rt_tlru;

	int findfirstempty ();
	int search(unsigned long tag) const;

	void init_tlru(SetsBtreeNode& node_tlru,int total_lvs);
	void update_tlru(SetsBtreeNode& node_tlru,int total_lvs,int blkidx);
	void lkup_tlru(SetsBtreeNode& node_tlru,int total_lvs,int& blkidx);

	int victim_tlru();
	int victim_random();

	int access(unsigned long tag,string replace_string);

	int numblks=1;
	bool full;
	bool empty;
	string replace_policy ;

};


class Cache {
public:
	// Constructor
	Cache (int csize,int blksize,int setsize,string rpolicy);
	// Destructor
	~Cache();
	int access(unsigned long addr) ;
	void print_cache_result();

	//Cacheset *csets ;
	vector <Cacheset> csets;

	int num_sets ;//= cache_size/block_size;

	//Counters
	int cnt_access ;

	//int cnt_miss;
	int cnt_hits;
	int cnt_new;
	int cnt_replace;

	string replace_policy;
private:
	int cache_size;
	int set_size ;
	int block_size;

	int occupied_sets;
};

#endif /* CACHE_MODEL_H_ */
