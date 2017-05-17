#include "FibHeap.h"
#include <algorithm>

void FibHeap::insert(CompositeKey key, Record rec, DBFile* dbfile) {
  //make new node
  HeapNode *hn = new HeapNode;
  hn->key = key;
  hn->rec = rec;
  hn->dbfile = dbfile;
  hn->degree = 0;

  //Add as a root
  roots.push_back(hn);
  updateDegree_arr(hn);

  //find new minimum
  findMinAmongRoots();
}

void FibHeap::extractmin(CompositeKey& key, Record& rec, DBFile*& dbfile) {
  key = min->key;
  rec = min->rec;
  dbfile = min->dbfile;

  //Adding children of min to roots and degree array
  vector<HeapNode*>::iterator itchild = min->children.begin();
  for(; itchild < min->children.end(); ++itchild) {
    roots.push_back((*itchild));
    updateDegree_arr((*itchild));
  }

  //Deleting min from roots and degree array
  degree_arr[min->degree].erase(remove(degree_arr[min->degree].begin(),\
                                degree_arr[min->degree].end(),\
                                min),\
                                degree_arr[min->degree].end());
  roots.erase(remove(roots.begin(), roots.end(), min), roots.end());

  //Merging roots according to degrees
  mergeRoots();

  //finding new minimum
  findMinAmongRoots();
}

void FibHeap::updateDegree_arr(HeapNode *hn) {
  int size = degree_arr.size();

  //When degree_arr has non initialized vectors
  if (hn->degree > size - 1) {
    for(int i = size; i <= hn->degree; i++) {
      vector<HeapNode*> v;
      degree_arr.push_back(v);
    }
  }

  //Updating appropriate vector
  degree_arr[hn->degree].push_back(hn);
}

void FibHeap::findMinAmongRoots() {
  vector<HeapNode*>::iterator it  = roots.begin();
  min = (*it);
  ++it;
  for (; it < roots.end(); ++it) {
    min = (*it)->key < min->key? (*it): min;
  }
}

void FibHeap::mergeRoots() {
  for(int degree = 0; degree < degree_arr.size(); degree++) {
    int dsize = degree_arr[degree].size();
    while(dsize > 1) {

      //Taking out last 2 roots and merging them
      HeapNode* root1 = degree_arr[degree].back();
      degree_arr[degree].pop_back();
      HeapNode* root2 = degree_arr[degree].back();
      degree_arr[degree].pop_back();

      // Updating new root in degree_arr
      HeapNode* nroot = merge2trees(root1, root2);
      updateDegree_arr(nroot);
      dsize -= 2;
    }
  }
}

FibHeap::HeapNode* FibHeap::merge2trees(HeapNode* hn1, HeapNode* hn2) {
  if (hn1->key < hn2->key) {

    //Add bigger key to tree of smaller tree
    hn1->children.push_back(hn2);
    hn1->degree++;

    //Remove bigger key from root
    roots.erase(remove(roots.begin(), roots.end(), hn2), roots.end());

    return hn1;
  }
  else {

    //Add bigger key to tree of smaller tree
    hn2->children.push_back(hn1);
    hn2->degree++;

    //Remove bigger key from root
    roots.erase(remove(roots.begin(), roots.end(), hn1), roots.end());

    return hn2;
  }
}
