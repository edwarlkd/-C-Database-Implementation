#include <iostream>
#include <cstring>
#include <map>
#include <unordered_map>

#include "Config.h"
#include "DBFile.h"
#include "Catalog.h"
#include "Schema.h"
#include "CompositeKey.h"

using namespace std;

bool testCompositeKey(Schema _schema, string _heapPath) {
  // open DBFile
  char* heapPath = new char[_heapPath.length()+1];
  strcpy(heapPath, _heapPath.c_str());
  DBFile dbFile;
  if(dbFile.Open(heapPath) == -1) {
    cerr << "ERROR: DBFile::Open" << endl << endl;
    return false;
  }

  // map<CompositeKey, int> m;
  map<string, int> m;
  // retrieve data from DBFile
  dbFile.MoveFirst();
  int i = 0; Record record;
  while(dbFile.GetNext(record) == 0) {
  	// CompositeKey key;
    // key.extractRecord(record, _schema);
		string key = record.createKeyFromRecord(_schema);

    // m.insert(pair<CompositeKey, int>(key, i));
    m.insert(pair<string, int>(key, i));
  }

  // close DBFile
  if(dbFile.Close() == -1) {
    cerr << "ERROR: DBFile::Close" << endl << endl;
    return false;
  }

  // map<CompositeKey, int>::iterator it;
  map<string, int>::iterator it;
  for(it = m.begin(); it != m.end(); ++it) {
  	// CompositeKey key = it->first;
  	string key = it->first;
  	
  	cout << key << endl;
  }

  return true;
}

int main(int argc, char* argv[]) {
  // load catalog
  string database = "catalog.sqlite";
  Catalog catalog(database);

  string tableName = "customer";

  Schema schema;
  if(!catalog.GetSchema(tableName, schema)) {
    cerr << "ERROR: Table '" << tableName << "' does not exist." << endl << endl;
    exit(-1);
  }
  string heapPath; catalog.GetDataFile(tableName, heapPath);
  if(!testCompositeKey(schema, heapPath)) {
    return -1;
  }
  
  return 0;
}
