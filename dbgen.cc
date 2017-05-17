#include <iostream>
#include <cstring>

#include "Config.h"
#include "DBFile.h"
#include "Catalog.h"
#include "Schema.h"

using namespace std;

// create a DBFile from corresponding TPC-H data
bool createDBFile(Schema _schema, string _heapPath, string _textPath) {
	char* heapPath = new char[_heapPath.length()+1];
	char* textPath = new char[_textPath.length()+1];
	strcpy(heapPath, _heapPath.c_str());
	strcpy(textPath, _textPath.c_str());

	// create DBFile
	DBFile dbFile;
	if(dbFile.Create(heapPath, Heap) == -1) {
		return false;
	}

	// load text data into heap file
	dbFile.Load(_schema, textPath);

	// close DBFile
	if(dbFile.Close() == -1) {
		return false;
	}

	return true;
}

bool testDBFile(Schema _schema, string _heapPath) {
	// open DBFile
	char* heapPath = new char[_heapPath.length()+1];
	strcpy(heapPath, _heapPath.c_str());
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
	if(argc == 2 || argc == 4) {
		// load catalog
		string database = "catalog.sqlite";
		Catalog catalog(database);

		if(argc == 2) {
			if(argv[1] == string("-all")) {
				string userHome = getenv("HOME");
				string defaultPath = userHome + "/.sqlite-jarvis/";
				string heapPath = "heap/"; string textPath = "text/";

				string yn;
				cout << "Jarvis: DBFiles for the 8 tables will be automatically generated." << endl;
				cout << "Make sure that you have following directories before proceeding." << endl << endl;
				cout << userHome << endl;
				cout << " └ .sqlite-jarvis" << endl;
				cout << "     ├ heap (where your DBFiles will be stored.)" << endl;
				cout << "     └ text (where your .tbl files should be stored.)" << endl << endl;
				cout << "You should put .tbl files under " << defaultPath << textPath << " first!" << endl;
				cout << "Existing files will be overwritten. Continue? (Y/N) " << flush;
				cin >> yn;
				if(yn == "Y" || yn == "y") {
					cout << "Generating DBFiles" << flush;

					// preparing
					int size = 8;
					vector<string> tableNames = {"customer", "lineitem", "nation", "orders", 
												 "part", "partsupp", "region", "supplier"};
					vector<string> heapPaths, textPaths;
					for(size_t i = 0; i < size; i++) {
						heapPaths.push_back(defaultPath + heapPath + tableNames[i] + ".dat");
						textPaths.push_back(defaultPath + textPath + tableNames[i] + ".tbl");
					}

					// batch
					for(size_t i = 0; i < size; i++) {
						cout << "." << flush;
						Schema schema;
						if(!catalog.GetSchema(tableNames[i], schema)) {
							cerr << endl << "ERROR: Table '" << tableNames[i] << "' does not exist." << endl << endl;
							return -1;
						}
						if(createDBFile(schema, heapPaths[i], textPaths[i])) {
							// update new DBFile path in catalog
							catalog.SetDataFile(tableNames[i], heapPaths[i]);
						} else {
							cerr << endl << "FAILED! (createDBFile of " << tableNames[i] << ")" << endl << endl;
							return -1;
						}
					}

					// save catalog to update new DBFile path
					if(catalog.Save()) {
						cout << " OK!" << endl;
						return 0;
					} else {
						cerr << " FAILED! (Catalog::Save)" << endl << endl;
						return -1;
					}
				} else {
					return 1;
				}
			} else { // print
				string tableName = argv[1];

				Schema schema;
				if(!catalog.GetSchema(tableName, schema)) {
					cerr << endl << "ERROR: Table '" << tableName << "' does not exist." << endl << endl;
					return -1;
				}
				string heapPath; catalog.GetDataFile(tableName, heapPath);
				if(!testDBFile(schema, heapPath)) {
					return -1;
				}
			}
		} else if(argc == 4) {
			string tableName, heapPath, textPath;
			tableName = argv[1]; heapPath = argv[2]; textPath = argv[3];

			cout << "Generating a DBFile for table '" << tableName << "'... " << flush;

			Schema schema;
			if(!catalog.GetSchema(tableName, schema)) {
				cerr << "ERROR: Table '" << tableName << "' does not exist." << endl << endl;
				exit(-1);
			}
			if(createDBFile(schema, heapPath, textPath)) {
				// update new DBFile path in catalog
				catalog.SetDataFile(tableName, heapPath); 
				// save catalog to update new DBFile path
				if(catalog.Save()) {
					cout << "OK!" << endl;
					return 0;
				} else {
					cerr << "FAILED! (Catalog::Save)" << endl << endl;
					return -1;
				}
			} else {
				cerr << "FAILED! (createDBFile of " << tableName << ")" << endl << endl;
				return -1;
			}
		}

	} else {
		cout << endl;
		cout << "Jarvis: This simple program generates a DBFile from TPC-H data." << endl << endl;
		cout << "Usage: " << endl;
		cout << "\tCreate:\t"<< argv[0] << " [TABLE_NAME] [HEAP_FILE_PATH] [TEXT_FILE_PATH]" << endl;
		cout << "\tPrint:\t"<< argv[0] << " [TABLE_NAME]" << endl;
		cout << "\tBatch:\t"<< argv[0] << " -all" << endl << endl;
	}
	
	return -1;
}
