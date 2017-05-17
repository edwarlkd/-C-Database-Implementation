#ifndef _FIB_HEAP_H
#define _FIB_HEAP_H

#include <vector>
#include "Record.h"
#include "DBFile.h"
#include "CompositeKey.h"

using namespace std;

class FibHeap {
private:

  struct HeapNode {
    CompositeKey key;
    Record rec;
    DBFile* dbfile;
    int degree; //Number of children
    HeapNode *parent;
    vector<HeapNode*> children;
  };

  vector<vector<HeapNode*> > degree_arr;
  HeapNode* min;
  vector<HeapNode*> roots;
  void updateDegree_arr(HeapNode *hn);
  void findMinAmongRoots();
  void mergeRoots();
  HeapNode* merge2trees(HeapNode* hn1, HeapNode* hn2);

public:

  void insert(CompositeKey key, Record rec, DBFile* dbfile);
  void extractmin(CompositeKey& key, Record& rec, DBFile*& dbfile);
};

#endif
