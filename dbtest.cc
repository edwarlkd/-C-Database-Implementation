#include <iostream>
#include <cstring>

#include "Config.h"
#include "DBFile.h"
#include "Catalog.h"
#include "Schema.h"

using namespace std;

bool testDBFile(Schema _schema, string _sortedPath) {
	// open DBFile
	char* heapPath = new char[_sortedPath.length()+1];
	strcpy(heapPath, _sortedPath.c_str());
	DBFile dbFile;
	if(dbFile.Open(heapPath) == -1) {
		cerr << "ERROR: DBFile::Open" << endl << endl;
		return false;
	}

	// retrieve data from DBFile
	dbFile.MoveFirst();
	while(true) {
		Record record;
		if(dbFile.GetNext(record) == 0) {
			record.print(cout, _schema); cout << endl;
		} else {
			break;
		}
	}

	// close DBFile
	if(dbFile.Close() == -1) {
		cerr << "ERROR: DBFile::Close" << endl << endl;
		return false;
	}

	return true;
}

int main(int argc, char* argv[]) {
	if(argc == 3) {
		// load catalog
		string database = "catalog.sqlite";
		Catalog catalog(database);

		string tableName, sortedPath;
		tableName = argv[1]; sortedPath = argv[2];

		Schema schema;
		if(!catalog.GetSchema(tableName, schema)) {
			cerr << "ERROR: Table '" << tableName << "' does not exist." << endl << endl;
			exit(-1);
		}
		if(!testDBFile(schema, sortedPath)) {
			return -1;
		}
	} else {
		cout << "Usage: " << argv[0] << " [TABLE_NAME] [SORTED_DBFILE_PATH]" << endl << endl;
	}
	
	return 0;
}
