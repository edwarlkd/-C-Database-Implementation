#include <sys/stat.h>
#include <cstring>
#include <vector>
#include <sstream>

#include "Config.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "DBFile.h"

using namespace std;


DBFile::DBFile () : fileName(""), isMovedFirst(false) {}

DBFile::~DBFile () {
}

DBFile::DBFile(const DBFile& _copyMe) :
	file(_copyMe.file),	fileName(_copyMe.fileName), isMovedFirst(false) {}

DBFile& DBFile::operator=(const DBFile& _copyMe) {
	// handle self-assignment first
	if (this == &_copyMe) return *this;

	file = _copyMe.file;
	fileName = _copyMe.fileName;

	return *this;
}

int DBFile::Create (char* f_path, FileType f_type) {
	fileName = f_path;
	fileType = f_type;

	// return 0 on success, -1 otherwise
	return file.Open(0, f_path); // mode = O_TRUNC | O_RDWR | O_CREAT;
}

int DBFile::Open (char* f_path) {
	fileName = f_path;
	
	// check whether file exists or not
	struct stat fileStat;
	if(stat(f_path, &fileStat) != 0) {
		return Create(f_path, Heap);
	} else {
		// return 0 on success, -1 otherwise
		return file.Open(fileStat.st_size, f_path); // mode = O_RDWR;
	}
}

void DBFile::Load (Schema& schema, char* textFile) {
	MoveFirst();
	FILE* textData = fopen(textFile, "r");

	while(true) {
		Record record;
		if(record.ExtractNextRecord(schema, *textData)) { // success on extract
			AppendRecord(record);
		} else { // no data left or error
			break;
		}
	}
	WriteToFile(); // add the last page to the file
	fclose(textData);
}

int DBFile::Close () {
	return file.Close();
}

void DBFile::MoveFirst () {
	iPage = 0; // reset page index to 0
	isMovedFirst = true;
	pageNow.EmptyItOut(); // the first page has no data
}

void DBFile::AppendRecord (Record& rec) {
	if(!pageNow.Append(rec)) { // no space in the current page, pageNow
		WriteToFile(); // add pageNow to the file
		pageNow.EmptyItOut(); // clear pageNow
		pageNow.Append(rec); // add rec to the pageNow
	}
}

void DBFile::WriteToFile() {
	file.AddPage(pageNow, iPage++);
}

int DBFile::GetNext (Record& rec) {
	if(!isMovedFirst) {
		MoveFirst();
	}
	
	off_t numPage = file.GetLength();
	while(true) {
		if(!pageNow.GetFirst(rec)) { // no record in the current page
			// check whether this is the last page
			if(iPage == numPage) { // EOF
				break;
			} else { // move on to the next page
				file.GetPage(pageNow, iPage++);
			}
		} else { // record exists
			return 0;
		}
	}
	return -1;
}

string DBFile::GetTableName() {
	vector<string> path;
	stringstream ss(fileName); string tok;
	while(getline(ss, tok, '/')) {
		path.push_back(tok);
	}
	string tempFileName = path[path.size()-1];
	stringstream ss2(tempFileName); string tableName = "";
	getline(ss2, tableName, '.');
	return tableName;
}

const char* DBFile::GetFileName() {
	char* fileNameC = new char[fileName.length()+1];
	strcpy(fileNameC, fileName.c_str());
	return fileNameC;
}

void DBFile::GetPageNumber(int num, Page& PgIndex){
	file.GetPage(PgIndex, num);
}

int DBFile::GetSpecificRec(int pgNum, int RecNum, Record& _record){
	
	pageNow.EmptyItOut();

	if(file.GetPage(pageNow, pgNum) == -1)
	{
		return 0;
	}
	if(pageNow.GetRecNumber(RecNum, _record) != 1)
	{
		return 0;
	}
	return 1;
}
