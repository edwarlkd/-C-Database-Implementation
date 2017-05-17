#ifndef _RECORD_H
#define _RECORD_H

#include <cstdio>
#include <iostream>
#include "Schema.h"

using namespace std;


/* Record data structure. Data are stored in bits.
 * The layout of bits is as follows:
 *	1) First sizeof(int) bytes: length of the record in bytes
 *	2) Next sizeof(int) bytes: byte offset to the start of the first att
 *	3) Byte offset to the start of the att in position numAtts
 *	4) Bits encoding the record's data
 */

class Record {
private:
	//the binary content of the record or the actual data in the record
	char* bits;

public:
	Record ();
	Record(const Record& _other);
	Record& operator=(const Record& _other);
	// swap function
	void Swap(Record& _other);

	virtual ~Record();

	// reads the next record from a pointer to a text file; also requires
	// that the schema be given; returns a 0 if there is no data left or
	// if there is an error and returns a 1 otherwise
	int ExtractNextRecord (Schema& mySchema, FILE& textFile);

	//gives access to the internal bits of a record; not encapsulation anymore
	//it is used only when necessary, no abuse
	char* GetBits ();

	//this returns the specified attribute from the record
	char* GetColumn(int which_column);

	//returns the size of the record, that is, the first integer in bits
	int GetSize();

	//copy bits of length b_len into the current record
	void CopyBits(char *bits, int b_len);

	// suck the contents of the record fromMe into this; note that after
	// this call, fromMe will no longer have anything inside of it
	void Consume (char*& fromMe);

	// makes the data bits NULL
	void Nullify ();

	// this projects away attributes from the record
	// only the attributes specified in attsToKeep (the indices) are kept
	// numAttsToKeep gives the number of attributes to be kept
	// numAttsNow specifies the current number of attributes present in the record
	void Project (int* attsToKeep, int numAttsToKeep, int numAttsNow);

	//left and right records are merged into the current record object
	//numAttsLeft and numAttsRight specify the number of attributes to be kept
	//attsToKeep gives the indices of the attributes to be kept
	//startOfRight gives the index in attsToKeep where the attributes from right start
	//notice that while merging, projection is also applied
	void MergeRecords (Record& left, Record& right,
		int numAttsLeft, int numAttsRight,
		int *attsToKeep, int numAttsToKeep, int startOfRight);

	//left and right records are merged into the current record object
	//numAttsLeft and numAttsRight specify the number of attributes in Left and Right
	void AppendRecords (Record& left, Record& right,
		int numAttsLeft, int numAttsRight);

	// print record content based on schema
	ostream& print(ostream& _os, Schema& mySchema);

	// print record directly to stdout (for debug)
	// e.g. (gdb) call rec.show(schemaIn)
	void show(Schema& mySchema);

	// create minified key from record (used in GroupBy::GetNext())
	string createKeyFromRecord(Schema& _schema);
};

#endif //_RECORD_H
