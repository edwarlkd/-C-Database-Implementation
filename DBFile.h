#ifndef DBFILE_H
#define DBFILE_H

#include <string>

#include "Config.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"

using namespace std;


class DBFile {
private:
	File file; // group of pages
	string fileName; // absolute path for the DBFile
	FileType fileType; // Heap, Sorted, Index (see Config.h)
	Page pageNow;
	off_t iPage; // index of the current page, pageNow, in file
	bool isMovedFirst;

public:
	DBFile ();
	virtual ~DBFile ();
	DBFile(const DBFile& _copyMe);
	DBFile& operator=(const DBFile& _copyMe);

	// creates a new heap file. FileType has to be Heap. 
	// This is done only once, when a SQL table is created.
	int Create (char* fpath, FileType file_type);

	// gives access to the heap file. 
	// The name is taken from the catalog, for every table
	int Open (char* fpath);

	// closes the file
	int Close ();

	// extracts records with a known schema from a text file passed as parameters. 
	// Essentially, it converts the data from text to binary. 
	// Method ExtractNextRecord from class Record does all the work for a given schema.
	void Load (Schema& _schema, char* textFile);

	// resets the file pointer to the beginning of the file, i.e., the first record
	void MoveFirst ();
	
	// appends the record passed as parameter to the end of the file. 
	// This is the only method to add records to a heap file.
	void AppendRecord (Record& _addMe);

	// simply write the current page to the file
	// call this function to finalize appending records into a DBFile
	void WriteToFile();

	// returns the next record in the file. 
	// The file pointer is moved to the following record
	// return 0 on success, -1 otherwise
	int GetNext (Record& _fetchMe);

	// this function retrieves tableName from fileName
	// defaultPath = "~/.sqlite-jarvis/heap/" + _table + ".dat" (see Catalog.cc)
	string GetTableName();

	// returns fileName
	const char* GetFileName();

	void GetPageNumber(int num, Page& PgIndex);
	int GetSpecificRec(int pgNum, int RecNum, Record& _record);
};

#endif //DBFILE_H
