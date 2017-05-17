#include <iostream>
#include <cstring>

#include "Config.h"
#include "DBFile.h"
#include "Catalog.h"
#include "Schema.h"
#include "CompositeKey.h"
#include "FibHeap.h"

using namespace std;

bool testCompositeKey(Schema _schema, string _heapPath) {
  // open DBFile
  char* heapPath = new char[_heapPath.length()+1];
  strcpy(heapPath, _heapPath.c_str());
  DBFile* dbFile = new DBFile();
  if(dbFile->Open(heapPath) == -1) {
    cerr << "ERROR: DBFile::Open" << endl << endl;
    return false;
  }

  FibHeap m;
  // retrieve data from DBFile
  dbFile->MoveFirst();
  int numRec = 0; Record record;
  while(dbFile->GetNext(record) == 0) {
  	CompositeKey key;
    key.extractRecord(record, _schema);
    m.insert(key, record, dbFile);
    numRec++;
  }

  // close DBFile
  if(dbFile->Close() == -1) {
    cerr << "ERROR: DBFile::Close" << endl << endl;
    return false;
  }

  // CompositeKey k; Record r; DBFile* d;
  // for(int i = 0; i < numRec; i++) {
  //   m.extractmin(k, r, d);
  //   cout << k << endl;
  // }

  return true;
}

int main(int argc, char* argv[]) {
  if(argc == 2) {
    // load catalog
    string database = "catalog.sqlite";
    Catalog catalog(database);

    string tableName = string(argv[1]);

    Schema schema;
    if(!catalog.GetSchema(tableName, schema)) {
      cerr << "ERROR: Table '" << tableName << "' does not exist." << endl << endl;
      exit(-1);
    }
    string heapPath; catalog.GetDataFile(tableName, heapPath);
    if(!testCompositeKey(schema, heapPath)) {
      return -1;
    }
  } else {
    cout << "Usage: " << argv[0] << " [TABLE_NAME]" << endl << endl;
  }
  
  return 0;
}
