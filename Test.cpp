/*
 * Test.cpp
 *
 *  Created on: 2017Äê11ÔÂ19ÈÕ
 *      Author: acer-
 */

#include <iostream>
#include <fstream>
#include <string>
#include "cache_model.h"

#define MS 1000

int m1[MS][MS];
int m2[MS][MS];
int res[MS][MS];

using namespace std;

void print_blk (const Cacheblock& blk);
void print_set (const Cacheset& cset);
void print_cache (const Cache& cinst);
void print_tlru (SetsBtreeNode& node_tlru) ;

void init(int m[][MS]);
void multiply(int m1[][MS], int m2[][MS]);

void sanity_test();
void test_mult(int cache_size,int sets_size,int blk_size,string replace_policy);
/*
 * Test for class cacheblock
 */
int main() {
    init(m1);
    init(m2);
    multiply(m1, m2);

    //case 3.1 : { Tage: 22 bits, Index 8 bits,  Block Data 2 bits or 4 Bytes }
    test_mult(1024,4,4*4,"random");
    test_mult(1024,4,4*4,"plru");

    //case 3.2 : { Tage: 18 bits, Index 8 bits,  Block Data 6 bits or 64 Bytes }
    test_mult(4096,64,4*64,"random");
    test_mult(4096,64,4*64,"plru");

	return 0;

}

/*
 * Array Mulitplication method.
 */
void multiply(int m1[][MS], int m2[][MS])
{
    int x, i, j;
    for (i = 0; i < MS; i++) {
        for (j = 0; j < MS; j++) {
	    //printf("SW %p\n", &res[i][j]);
            res[i][j] = 0;
            for (x = 0; x < MS; x++) {
        /*
		printf("LW %p\n", &m1[i][x]);
		printf("LW %p\n", &m2[x][j]);
		printf("LW %p\n", &res[i][j]);
		printf("SW %p\n", &res[i][j]);
		*/
                res[i][j] += m1[i][x] * m2[x][j];
            }
        }
    }
    /*
    for (i = 0; i < MS; i++) {
        for (j = 0; j < MS; j++) {
            printf("%d ", res[i][j]);
        }
        printf("\n");
    }
    */
    printf("\n");
}

void init(int m[][MS])
{
  for(int i=0; i<MS; i++) {
    for(int j=0; j<MS; j++) {
        m[i][j] = i*MS +j;
    }
  }
}

/*
 * Print methods
 */
void print_blk (const Cacheblock& blk) {
//	cout << "---------------Cache Block status------------"<<endl;
	cout << "valid=" << blk.get_valid() << "; tag= " << blk.get_tag() <<";ts= " << blk.get_ts() <<endl;
	return;
}

void print_set (const Cacheset& cset) {
	cout << "full=" << cset.full << "; empty= " << cset.empty <<endl;

	for(int idx=0;idx<cset.numblks;idx++){
		cout <<"block id " <<idx << " : "<<endl;
		print_blk(cset.cacheset[idx]);
		cout <<endl;
	}

	return;
}

void print_cache (const Cache& cinst) {
	for(int idx=0;idx<cinst.num_sets;idx++){
		cout <<"set id " <<idx << " : "<<endl;
		print_set(cinst.csets[idx]);
		cout <<endl;
	}

	return;
}

void print_tlru (SetsBtreeNode& node_tlru) {

//	if ( (node_tlru.leftset==nullptr) &&
//			(node_tlru.rightset==nullptr)	) {
	if ( &node_tlru!=NULL) {
		cout <<"nd ptr: " << &node_tlru<<endl;
		cout << node_tlru ;
		print_tlru(*node_tlru.leftset);
		print_tlru(*node_tlru.rightset);
	}
}

/*
 * Sanity Test main
 */
void sanity_test() {

	Cacheblock blk ;
	cout <<"----------------init cacheblock----------------" <<endl;
	print_blk (blk);

	cout <<"----------------first setblock----------------" <<endl;
	unsigned long newtag=256;
	unsigned long newts = 1024;
	blk.update_tag(newtag);
	blk.update_ts (newts) ;

	print_blk (blk);

	//cout <<"compare tag result:"<< blk.compare_tag (newtag)<<endl;
	cout << endl;

	//-----------Test cacheset-------------------------
	cout << "------------start to test set---------------"<<endl;
	int access_idx=0;
	Cacheset cset(4);

	access_idx= cset.access(101,"random");
	access_idx= cset.access(102,"random");
	access_idx= cset.access(103,"random");
	access_idx= cset.access(104,"random");

	cout <<"---------insert 4 cache blocks-----------"<<endl;
	cout <<endl;
	print_set(cset);


	access_idx= cset.access(103,"random");
	cout <<"---------insert 5 cache blocks-----------"<<endl;
	print_set(cset);
	cout <<endl;

	access_idx= cset.access(105,"random");
	cout <<"---------insert 6 cache blocks-----------"<<endl;
	print_set(cset);



	//**************cache sanity test**************
	Cache cinst (128,4,32,"random");
	cout <<endl;
	cout <<"-----------------print cache status--------------"<<endl;

	int setidx;
	setidx =cinst.access(1023);
	print_cache(cinst);

	for (int idx=0;idx<16;idx++) {
		cout << "SET IDX : " << idx<<endl;
		print_tlru(*cinst.csets[idx].rt_tlru);
		cout <<endl;
	}

	for (int idx=0;idx<8;idx++){
		setidx=cinst.access(1023);
	}

	for (int idx=0;idx<16;idx++) {
		cout << "SET IDX : " << idx<<endl;
		print_tlru(*cinst.csets[idx].rt_tlru);
		cout <<endl;
	}

	return ;
}


void test_mult(int cache_size,int sets_size,int blk_size,string replace_policy) {
	//Cache cinst (128,4,32,"random");
	Cache cinst (cache_size,sets_size,blk_size,replace_policy);
    int setidx;

    for (int i=0;i<MS;i++)
    	for (int j=0;j<MS;j++) {
     		setidx =cinst.access(res[i][j]);
    	}

    cout <<endl;
    cinst.print_cache_result() ;
}
