#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>

#include "Config.h"
#include "Swap.h"
#include "Schema.h"
#include "Record.h"

using namespace std;


Record :: Record () {
	bits = NULL;
}

Record::Record (const Record& copyMe) {
	// this is a deep copy, so allocate the bits and move them over!
	// delete [] bits; // we're in a CONSTRUCTOR. why do we delete?
	bits = new char[((int *) copyMe.bits)[0]];
	memcpy (bits, copyMe.bits, ((int *) copyMe.bits)[0]);
}

Record& Record::operator=(const Record& copyMe) {
	// handle self-assignment first
	if (this == &copyMe) return *this;

	// this is a deep copy, so allocate the bits and move them over!
	delete [] bits;
	bits = new char[((int *) copyMe.bits)[0]];
	memcpy (bits, copyMe.bits, ((int *) copyMe.bits)[0]);

	return *this;
}

Record :: ~Record () {
	delete [] bits;
	bits = NULL;
}

void Record::Swap(Record& _other) {
	SWAP(bits, _other.bits);
}

void Record :: Consume (char*& fromMe) {
	delete [] bits;
	bits = fromMe;
	fromMe = NULL;
}

int Record :: ExtractNextRecord (Schema& mySchema, FILE& textFile) {
	// this is temporary storage
	char* space = new char[PAGE_SIZE];
	char* recSpace = new char[PAGE_SIZE];

	// clear out the present record
	delete [] bits;
	bits = NULL;

	unsigned int n = mySchema.GetNumAtts();
	vector<Attribute> atts = mySchema.GetAtts();

	// this is the current position (int bytes) in the binary
	// representation of the record that we are dealing with
	int currentPosInRec = sizeof (int) * (n + 1);

	// loop through all of the attributes
	for (int i = 0; i < n; i++) {
		// first we suck in the next attribute value
		int len = 0;
		while (1) {
			int nextChar = getc (&textFile);

			if (nextChar == '|') break;
			else if (nextChar == EOF) {
				delete [] space;
				delete [] recSpace;
				return 0;
			}

			space[len] = nextChar;
			len++;
		}

		// set up the pointer to the current attribute in the record
		((int *) recSpace)[i + 1] = currentPosInRec;

		// null terminate the string
		space[len] = 0;
		len++;

		// then we convert the data to the correct binary representation
		if (atts[i].type == Integer) {
			*((int *) &(recSpace[currentPosInRec])) = atoi (space);	
			currentPosInRec += sizeof (int);
		}
		else if (atts[i].type == Float) {
			*((double *) &(recSpace[currentPosInRec])) = atof (space);
			currentPosInRec += sizeof (double);
		}
		else if (atts[i].type == String) {
			// align things to the size of an integer if needed
			if (len % sizeof (int) != 0) {
				len += sizeof (int) - (len % sizeof (int));
			}

			strcpy (&(recSpace[currentPosInRec]), space); 
			currentPosInRec += len;
		} 
	}

	// the last thing is to set up the pointer to just past the end of the record
	((int *) recSpace)[0] = currentPosInRec;

	// and copy over the bits
	bits = new char[currentPosInRec];
	memcpy (bits, recSpace, currentPosInRec);	

	delete [] space;
	delete [] recSpace;

	return 1;
}

char* Record :: GetBits () {
	return bits;
}

char* Record :: GetColumn(int which_column) {
	char* position = bits + ((int*)bits)[which_column + 1];
	return position;
}

int Record :: GetSize() {
	if (bits == NULL) return 0;
	else return *((int *) bits);
}

void Record :: CopyBits(char* _bits, int b_len) {
	delete [] bits;
	bits = new char[b_len];
	memcpy (bits, _bits, b_len);
}

void Record :: Nullify () {
	bits = NULL;
}

void Record :: Project (int* attsToKeep, int numAttsToKeep, int numAttsNow) {
	// first, figure out the size of the new record
	int totSpace = sizeof (int) * (numAttsToKeep + 1);

	for (int i = 0; i < numAttsToKeep; i++) {
		// if we are keeping the last record, be careful!
		if (attsToKeep[i] == numAttsNow - 1) {
			// take the length of the record and subtract the start position
			totSpace += ((int *) bits)[0] - ((int *) bits)[attsToKeep[i] + 1];
		}
		else {
			// subtract the start of the next field from the start of this field
			totSpace += ((int *) bits)[attsToKeep[i] + 2] - ((int *) bits)[attsToKeep[i] + 1]; 
		}
	}

	// now, allocate the new bits
	char *newBits = new char[totSpace];

	// record the total length of the record
	*((int *) newBits) = totSpace;

	// and copy all of the fields over
	int curPos = sizeof (int) * (numAttsToKeep + 1);
	for (int i = 0; i < numAttsToKeep; i++) {
		// this is the length (in bytes) of the current attribute
		int attLen;

		// if we are keeping the last record, be careful!
		if (attsToKeep[i] == numAttsNow - 1) {
			// take the length of the record and subtract the start position
			attLen = ((int *) bits)[0] - ((int *) bits)[attsToKeep[i] + 1];
		}
		else {
			// subtract the start of the next field from the start of this field
			attLen = ((int *) bits)[attsToKeep[i] + 2] - ((int *) bits)[attsToKeep[i] + 1]; 
		}

		// set the start position of this field
		((int *) newBits)[i + 1] = curPos;	

		// and copy over the bits
		memcpy (&(newBits[curPos]), &(bits[((int *) bits)[attsToKeep[i] + 1]]), attLen);

		// note that we are moving along in the record
		curPos += attLen;
	}

	// kill the old bits
	delete [] bits;

	// and attach the new ones
	bits = newBits;
}

// merge the left and right records into the current record object
// projection of unnecessary attributes is done at the same time
void Record :: MergeRecords (Record& left, Record& right,
	int numAttsLeft, int numAttsRight,
	int* attsToKeep, int numAttsToKeep, int startOfRight) {

	delete [] bits;
	bits = NULL;

	// if one of the records is empty, new record is non-empty record
	if (numAttsLeft == 0) {
		*this  = right;
		return;
	}
	else if (numAttsRight == 0) {
		*this = left;
		return;
	}

	// first, figure out the size of the new record
	int totSpace = sizeof (int) * (numAttsToKeep + 1);

	int numAttsNow = numAttsLeft;
	char *rec_bits = left.bits;

	for (int i = 0; i < numAttsToKeep; i++) {
		if (i == startOfRight) {
			numAttsNow = numAttsRight;
			rec_bits = right.bits;
		}
		// if we are keeping the last record, be careful!
		if (attsToKeep[i] == numAttsNow - 1) {
			// take the length of the record and subtract the start position
			totSpace += ((int *) rec_bits)[0] - ((int *) rec_bits)[attsToKeep[i] + 1];
		}
		else {
			// subtract the start of the next field from the start of this field
			totSpace += ((int *) rec_bits)[attsToKeep[i] + 2] - ((int *) rec_bits)[attsToKeep[i] + 1]; 
		}
	}

	// now, allocate the new bits
	bits = new char[totSpace];

	// record the total length of the record
	*((int *) bits) = totSpace;

	numAttsNow = numAttsLeft;
	rec_bits = left.bits;

	// and copy all of the fields over
	int curPos = sizeof (int) * (numAttsToKeep + 1);
	for (int i = 0; i < numAttsToKeep; i++) {
		if (i == startOfRight) {
			numAttsNow = numAttsRight;
			rec_bits = right.bits;
		}
		
		// this is the length (in bytes) of the current attribute
		int attLen;

		// if we are keeping the last record, be careful!
		if (attsToKeep[i] == numAttsNow - 1) {
			// take the length of the record and subtract the start pos
			attLen = ((int *) rec_bits)[0] - ((int *) rec_bits)[attsToKeep[i] + 1];
		}
		else {
			// subtract the start of the next field from the start of this field
			attLen = ((int *) rec_bits)[attsToKeep[i] + 2] - ((int *) rec_bits)[attsToKeep[i] + 1];
		}

		// set the start position of this field
		((int *) bits)[i + 1] = curPos;	

		// and copy over the bits
		memmove (&(bits[curPos]), &(rec_bits[((int *) rec_bits)[attsToKeep[i] + 1]]), attLen);

		// note that we are moving along in the record
		curPos += attLen;
	}
}

// merge the left and right records into the current record object
void Record :: AppendRecords (Record& left, Record& right,
	int numAttsLeft, int numAttsRight) {

	delete [] bits;
	bits = NULL;

	// if one of the records is empty, new record is non-empty record
	if (numAttsLeft == 0) {
		*this = right;
		return;

	}
	else if (numAttsRight == 0) {
		*this = left;
		return;
	}

	// first, figure out the size of the new record
	int totSpace = sizeof (int) * (numAttsLeft + numAttsRight + 1);

	char *rec_bits = left.bits;
	for (int i = 0; i < numAttsLeft; i++) {
		// last record, be careful!
		if (i == numAttsLeft - 1) {
			// take the length of the record and subtract the start pos
			totSpace += ((int *) rec_bits)[0] - ((int *) rec_bits)[i + 1];
		}
		else {
			// subtract the start of the next field from the start of this field
			totSpace += ((int *) rec_bits)[i + 2] - ((int *) rec_bits)[i + 1];
		}
	}


	rec_bits = right.bits;
	for (int i = 0; i < numAttsRight; i++) {
		// last record, be careful!
		if (i == numAttsRight - 1) {
			// take the length of the record and subtract the start pos
			totSpace += ((int *) rec_bits)[0] - ((int *) rec_bits)[i + 1];
		}
		else {
			// subtract the start of the next field from the start of this field
			totSpace += ((int *) rec_bits)[i + 2] - ((int *) rec_bits)[i + 1];
		}
	}

	// now, allocate the new bits
	bits = new char[totSpace];

	// record the total length of the record
	*((int *) bits) = totSpace;
	int curPos = sizeof (int) * (numAttsLeft + numAttsRight + 1);


	// and copy all of the fields over
	rec_bits = left.bits;
	for (int i = 0; i < numAttsLeft; i++) {
		// this is the length (in bytes) of the current attribute
		int attLen;

		// last record, be careful!
		if (i == numAttsLeft - 1) {
			// take the length of the record and subtract the start pos
			attLen = ((int *) rec_bits)[0] - ((int *) rec_bits)[i + 1];
		}
		else {
			// subtract the start of the next field from the start of this field
			attLen = ((int *) rec_bits)[i + 2] - ((int *) rec_bits)[i + 1];
		}

		// set the start position of this field
		((int *) bits)[i + 1] = curPos;

		// and copy over the bits
		memmove (&(bits[curPos]), &(rec_bits[((int *) rec_bits)[i + 1]]), attLen);

		// note that we are moving along in the record
		curPos += attLen;
	}

	rec_bits = right.bits;
	for (int i = 0; i < numAttsRight; i++) {
		// this is the length (in bytes) of the current attribute
		int attLen;

		// last record, be careful!
		if (i == numAttsRight - 1) {
			// take the length of the record and subtract the start pos
			attLen = ((int *) rec_bits)[0] - ((int *) rec_bits)[i + 1];
		} else {
			// subtract the start of the next field from the start of this field
			attLen = ((int *) rec_bits)[i + 2] - ((int *) rec_bits)[i + 1];
		}

		// set the start position of this field
		((int *) bits)[numAttsLeft + i + 1] = curPos;

		// and copy over the bits
		memmove (&(bits[curPos]), &(rec_bits[((int *) rec_bits)[i + 1]]), attLen);

		// note that we are moving along in the record
		curPos += attLen;
	}
}

ostream& Record :: print(ostream& _os, Schema& mySchema) {
	int n = mySchema.GetNumAtts();
	vector<Attribute> atts = mySchema.GetAtts();

	_os << '{';

	// loop through all of the attributes
	for (int i = 0; i < n; i++) {
		// print the attribute name
		_os << atts[i].name << ": ";

		// use the i^th slot at the head of the record to get the
		// offset to the correct attribute in the record
		int pointer = ((int *) bits)[i + 1];

		// here we determine the type, which given in the schema;
		// depending on the type we then print out the contents
		// first is integer
		if (atts[i].type == Integer) {
			int *myInt = (int *) &(bits[pointer]);
			_os << *myInt;
		}
		// then is a double
		else if (atts[i].type == Float) {
			double *myDouble = (double *) &(bits[pointer]);
			_os << fixed << *myDouble;
		}
		// then is a character string
		else if (atts[i].type == String) {
			char *myString = (char *) &(bits[pointer]);
			_os << myString;
		} 

		// print out a comma as needed to make things pretty
		if (i != n - 1) {
			_os << ", ";
		}
	}

	_os << '}';

	return _os;
}

void Record :: show(Schema& mySchema) {
	int n = mySchema.GetNumAtts();
	vector<Attribute> atts = mySchema.GetAtts();

	cout << '{';

	// loop through all of the attributes
	for (int i = 0; i < n; i++) {
		// print the attribute name
		cout << atts[i].name << ": ";

		// use the i^th slot at the head of the record to get the
		// offset to the correct attribute in the record
		int pointer = ((int *) bits)[i + 1];

		// here we determine the type, which given in the schema;
		// depending on the type we then print out the contents
		// first is integer
		if (atts[i].type == Integer) {
			int *myInt = (int *) &(bits[pointer]);
			cout << *myInt;
		}
		// then is a double
		else if (atts[i].type == Float) {
			double *myDouble = (double *) &(bits[pointer]);
			cout << fixed << *myDouble;
		}
		// then is a character string
		else if (atts[i].type == String) {
			char *myString = (char *) &(bits[pointer]);
			cout << myString;
		} 

		// print out a comma as needed to make things pretty
		if (i != n - 1) {
			cout << ", ";
		}
	}

	cout << '}' << endl;
}

string Record :: createKeyFromRecord(Schema& _schema) {
	stringstream ss;
	int n = _schema.GetNumAtts();
	vector<Attribute> atts = _schema.GetAtts();
	
	for (int i = 0; i < n; i++) {
		// print the attribute index, instead of name to make it short
		ss << i << ":";

		// use the i^th slot at the head of the record to get the
		// offset to the correct attribute in the record
		int pointer = ((int *) bits)[i + 1];

		// here we determine the type, which given in the schema;
		// depending on the type we then print out the contents
		// first is integer
		if (atts[i].type == Integer) {
			int *myInt = (int *) &(bits[pointer]);
			ss << *myInt;
		}
		// then is a double
		else if (atts[i].type == Float) {
			double *myDouble = (double *) &(bits[pointer]);
			ss << fixed << *myDouble;
		}
		// then is a character string
		else if (atts[i].type == String) {
			char *myString = (char *) &(bits[pointer]);
			ss << myString;
		} 

		// print out a comma as needed to separate tuples
		if (i != n - 1) {
			ss << ",";
		}
	}

	return ss.str();
}
