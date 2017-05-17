#include <iostream>
#include <cstring>
#include <stdlib.h>     /* srand, rand */
#include <time.h>

#include "Heapdatastructure.h"
#include "CompositeKey.h"
#include "Record.h"
#include "DBFile.h"


using namespace std;

int main (int argc, char* argv[]) {
  cout<<"Hello World?"<<endl;
  Heapdatastructure fh;
  srand (time(NULL));
  int num = 10000;
  for(int i = 0; i < num; i++) {
    CompositeKey key;

    // int randInt = rand()%10 + 1; // changed divisor to see more duplicates
    int randInt = i;
    key.addInt(randInt);
    //
    // double randDbl = (rand()%100 + 1)/10.0;
    // key.addDbl(randDbl);
    //
    // string randStr = to_string(rand()%num + 1);
    // char* randStrC = new char[randStr.length()+1];
    // strcpy(randStrC, randStr.c_str());
    // key.addStr(randStrC);

    // Record rec; DBFile* dbf;
    fh.Insert(key);//, rec, dbf);
  }
  CompositeKey prev = fh.GetMin();
  cout<<"Numbers inserted in Fibonacci Heap -"<<endl;
  cout << prev << endl;
  for(int i = 1; i < num; i++) {
    CompositeKey key = fh.GetMin();
    cout << key << endl;
    fh.DeleteMin();
    if (key < prev) {
      cout<< "Wrong Result!!!!"<<endl;
      break;
    }
    else {
      prev = key;
    }
  }
  return 0;
}
