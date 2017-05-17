#ifndef _FILE_H
#define _FILE_H

#include <string>

#include "Record.h"
#include "TwoWayList.cc"

using namespace std;


class Record;

class Page {
private:
	TwoWayList <Record> myRecs;
	
	int numRecs;
	int curSizeInBytes;

public:
	// constructor & destructor
	Page();
	virtual ~Page();

	// write records to bits
	void ToBinary(char* bits);

	// extract records from bits
	void FromBinary(char* bits);

	// delete current record from page and return it
	// return 0 if there are no records in the page, something else otherwise
	int GetFirst(Record& firstOne);

	// append record to the end of page
	// return 1 on success and 0 if there is no space on page
	// record is consumed so it has no value after Append is called
	int Append(Record& addMe);

	// empty records from page
	void EmptyItOut();

	int GetRecNumber(int i, Record& _record);
};


class File {
private:
	int fileDescriptor;
	string fileName;
	off_t curLength;
	int InNode;

public:
	File();
	virtual ~File();
	File(const File& _copyMe);
	File& operator=(const File& _copyMe);

	// return length of file, in pages
	off_t GetLength();

	// open file
	// if length is 0, create new file; existent file is erased
	// return 0 on success, -1 otherwise
	int Open(int length, char* fName);

	int Open();
	
	// get specified page from file
	// return 0 on success, -1 otherwise
	int GetPage(Page& putItHere, off_t whichPage);

	// write page to file
	// if write is past end of file (beyond length in pages), all new pages that
	// are past last page and before page to be written are zeroed out
	void AddPage(Page& addMe, off_t whichPage);

	// close file and return length in number of pages
	int Close ();
};

#endif //_FILE_H
