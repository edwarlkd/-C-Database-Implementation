#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>

#include "Config.h"
#include "Record.h"
#include "TwoWayList.cc"
#include "File.h"

using namespace std;


Page :: Page() : curSizeInBytes(sizeof (int)), numRecs(0) {
}

Page :: ~Page() {
}

void Page :: EmptyItOut() {
	// get rid of all of the records
	TwoWayList<Record> aux; aux.Swap(myRecs);

	// reset the page size
	curSizeInBytes = sizeof(int);
	numRecs = 0;
}

int Page :: GetFirst(Record& firstOne) {
	// move to the first record
	myRecs.MoveToStart ();

	// make sure there is data 
	if (myRecs.AtEnd()) return 0;

	// and remove it
	myRecs.Remove (firstOne);
	numRecs--;

	char* b = firstOne.GetBits();
	curSizeInBytes -= ((int*)b)[0];

	return 1;
}

int Page :: Append (Record& addMe) {
	char* b = addMe.GetBits();

	// first see if we can fit the record
	if (curSizeInBytes + ((int *) b)[0] > PAGE_SIZE) return 0;

	curSizeInBytes += ((int *) b)[0];
	myRecs.Append(addMe);
	numRecs++;

	return 1;	
}

void Page :: ToBinary (char* bits) {
	// first write the number of records on the page
	((int *) bits)[0] = numRecs;

	char* curPos = bits + sizeof (int);

	// and copy the records one-by-one
	for (myRecs.MoveToStart(); !myRecs.AtEnd(); myRecs.Advance()) {
		char* b = myRecs.Current().GetBits();
		
		// copy over the bits of the current record
		memcpy (curPos, b, ((int *) b)[0]);
		curPos += ((int *) b)[0];
	}
}

void Page :: FromBinary (char* bits) {
	// first read the number of records on the page
	numRecs = ((int *) bits)[0];

	// and now get the binary representations of each
	char* curPos = bits + sizeof (int);

	// first, empty out the list of current records
	TwoWayList<Record> aux; aux.Swap(myRecs);

	// now loop through and re-populate it
	Record temp;
	curSizeInBytes = sizeof (int);
	for (int i = 0; i < numRecs; i++) {
		// get the length of the current record
		int len = ((int *) curPos)[0];
		curSizeInBytes += len;

		// create the record
		temp.CopyBits(curPos, len);

		// add it
		myRecs.Append(temp);
		curPos += len;
	}
}

int Page :: GetRecNumber(int i, Record& _record)
{
	int cnt = 1;
	myRecs.MoveToStart();

	if(myRecs.AtEnd())
	{
		return 0;
	}

	while(!myRecs.AtEnd())
	{
		if(i == cnt)
		{
			myRecs.Remove(_record);
			numRecs--;
			char* bts = _record.GetBits();
			curSizeInBytes -= ((int*)bts)[0];
			return 1;
		}

		myRecs.Advance();
		cnt++;
	}
	
	return 0;
}


File :: File () : fileDescriptor(-1), fileName(""), curLength(0) {
}

File :: ~File () {
}

File::File(const File& _copyMe) : fileDescriptor(_copyMe.fileDescriptor),
	fileName(_copyMe.fileName), curLength(_copyMe.curLength) {}

File& File::operator=(const File& _copyMe) {
	// handle self-assignment first
	if (this == &_copyMe) return *this;

	fileDescriptor = _copyMe.fileDescriptor;
	fileName = _copyMe.fileName;
	curLength = _copyMe.curLength;

	return *this;
}

int File :: Open() {
	return (Open(1, &fileName[0]));
}

int File :: Open (int fileLen, char* fName) {
	// figure out the flags for the system open call
	int mode;
	if (fileLen == 0) mode = O_TRUNC | O_RDWR | O_CREAT;
	else mode = O_RDWR;

	// actually do the open
	fileName = fName;
	fileDescriptor = open (fName, mode, S_IRUSR | S_IWUSR);

	// see if there was an error
	if (fileDescriptor < 0) {
		cerr << endl << "ERROR: Open file did not work for " << fileName << "!" << endl;
		return -1;
	}

	// read in the buffer if needed
	if (fileLen != 0) {
		// read in the first few bits, which is the number of pages
		lseek (fileDescriptor, 0, SEEK_SET);
		read (fileDescriptor, &curLength, sizeof (off_t));
	}
	else curLength = 0;

	return 0;
}

int File :: Close () {
	// write out the current length in pages
	lseek (fileDescriptor, 0, SEEK_SET);
	write (fileDescriptor, &curLength, sizeof (off_t));

	// close the file
	close (fileDescriptor);

	// and return the size
	return curLength;
}

int File :: GetPage (Page& putItHere, off_t whichPage) {
	if (whichPage >= curLength) {
		cerr << endl << "ERROR: Read past end of the file " << fileName << ": ";
		cerr << "page = " << whichPage << " length = " << curLength << endl;
		return -1;
	}

	// this is because the first page has no data
	whichPage++;

	// read in the specified page
	char* bits = new char[PAGE_SIZE];
	
	lseek (fileDescriptor, PAGE_SIZE * whichPage, SEEK_SET);
	read (fileDescriptor, bits, PAGE_SIZE);
	putItHere.FromBinary(bits);

	delete [] bits;
}

void File :: AddPage (Page& addMe, off_t whichPage) {
	if(whichPage >= curLength) {
		// do the zeroing
		for (off_t i = curLength; i < whichPage; i++) {
			char zero[PAGE_SIZE]; bzero(zero, PAGE_SIZE);
			lseek (fileDescriptor, PAGE_SIZE * (i+1), SEEK_SET);
			write (fileDescriptor, &zero, PAGE_SIZE);
		}

		// now write the page
		char* bits = new char[PAGE_SIZE];

		addMe.ToBinary(bits);
		lseek (fileDescriptor, PAGE_SIZE * (whichPage+1), SEEK_SET);
		write (fileDescriptor, bits, PAGE_SIZE);

		curLength = whichPage + 1; // increase length

		delete [] bits;
	} else {
		cerr << endl << "Warning: Can't add a page on " << whichPage << ": ";
		cerr << "length = " << curLength << endl;
	}
}

off_t File :: GetLength () {
	return curLength;
}
