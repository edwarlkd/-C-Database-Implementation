#ifndef _REL_OP_H
#define _REL_OP_H

#include <iostream>
#include <fstream>
#include <unordered_set>
#include <unordered_map>

#include "Schema.h"
#include "Record.h"
#include "DBFile.h"
#include "Function.h"
#include "Comparison.h"
#include "FibHeap.h"
#include "CompositeKey.h"

using namespace std;

// limit number of pages available for a DBFile for sort-merge join
extern int NUM_PAGES_AVAILABLE;

struct GroupVal {
	double sum;
	Record rec;
};

class RelationalOp {
protected:
	// the number of pages that can be used by the operator in execution
	int noPages;
public:
	// empty constructor & destructor
	RelationalOp() : noPages(-1) {}
	virtual ~RelationalOp() {}

	// set the number of pages the operator can use
	void SetNoPages(int _noPages) {noPages = _noPages;}

	// every operator has to implement this method
	virtual bool GetNext(Record& _record) = 0;

	/* Virtual function for polymorphic printing using operator<<.
	 * Each operator has to implement its specific version of print.
	 */
    virtual ostream& print(ostream& _os) = 0;

    /* Get schema for the current op */
    virtual Schema GetSchema() = 0;

    /* Overload operator<< for printing.
     */
    friend ostream& operator<<(ostream& _os, RelationalOp& _op);
};

class Scan : public RelationalOp {
private:
	// schema of records in operator
	Schema schema;

	// physical file where data to be scanned are stored
	DBFile file;

public:
	Scan(Schema& _schema, DBFile& _file);
	virtual ~Scan();

	virtual bool GetNext(Record& _record);

	virtual Schema GetSchema() { return schema; }

	virtual ostream& print(ostream& _os);
};

////////////////////SCAN INDEX//////////////////////////////
class ScanIndex : public RelationalOp {
private:

	/*Schema schema;
	Schema schema2;
	CNF predicate;
	Record constants;
	string leafNode;
	string internalNode;
	string tab;
	vector<Record> lastRecord;
	int check;
	int vectorCount;*/

	Schema schema;
	Schema schema2;
	CNF predicate;
	Record constants;
	vector<Record> Recs;
	string tab;
	string att;
	string k;
	Type typ;
	//int check;
	Schema Leaf;
	Schema Internal;
	File in;
	File pgheader;
	int vectorCount;
	bool check;

public:

	

	ScanIndex(Schema& _schema, Schema _schema2, CNF& _predicate, Record& _constants, string& _tab, string& _att, string& _k, Type _typ, File _in, File _pgheader);
	
	virtual ~ScanIndex();

	virtual void GetSchema (Schema & _schema){_schema = schema;}

	virtual bool GetNext(Record& _record);

	virtual ostream& print(ostream& _os);

};
////////////////////////////////////////////////////////////



class Select : public RelationalOp {
private:
	// schema of records in operator
	Schema schema;

	// selection predicate in conjunctive normal form
	CNF predicate;
	// constant values for attributes in predicate
	Record constants;

	// operator generating data
	RelationalOp* producer;

public:
	Select(Schema& _schema, CNF& _predicate, Record& _constants,
		RelationalOp* _producer);
	virtual ~Select();

	virtual bool GetNext(Record& _record);

	virtual Schema GetSchema() { return schema; }

	virtual ostream& print(ostream& _os);
};

class Project : public RelationalOp {
private:
	// schema of records input to operator
	Schema schemaIn;
	// schema of records output by operator
	Schema schemaOut;

	// number of attributes in input records
	int numAttsInput;
	// number of attributes in output records
	int numAttsOutput;
	// index of records from input to keep in output
	// size given by numAttsOutput
	int* keepMe;

	// operator generating data
	RelationalOp* producer;

public:
	Project(Schema& _schemaIn, Schema& _schemaOut, int _numAttsInput,
		int _numAttsOutput, int* _keepMe, RelationalOp* _producer);
	virtual ~Project();

	virtual bool GetNext(Record& _record);

	virtual Schema GetSchema() { return schemaOut; }

	virtual ostream& print(ostream& _os);
};

class Join : public RelationalOp {
private:
	// schema of records in left operand
	Schema schemaLeft;
	// schema of records in right operand
	Schema schemaRight;
	// schema of records output by operator
	Schema schemaOut;

	// selection predicate in conjunctive normal form
	CNF predicate;

	// operators generating data
	RelationalOp* left;
	RelationalOp* right;
	// Data structures for left and right Records
	FibHeap left_ds;
	FibHeap right_ds;
	// Collection of the data structure outputted by the output schema
	vector<Record> recs_left, recs_right;
	int leftidx, rightidx;
	// Check if the iteration is happening for the first time
	bool isFirst;

	// vectors of pointers to DBFiles for each relation
	vector<DBFile*> DBFilesLeft;
	vector<DBFile*> DBFilesRight;

public:
	Join(Schema& _schemaLeft, Schema& _schemaRight, Schema& _schemaOut,
		CNF& _predicate, RelationalOp* _left, RelationalOp* _right);
	virtual ~Join();

	bool GetNext(Record& _record);

	Schema GetSchema() { return schemaOut; }

	// create temporary DBFiles with sorted records for sort-merge join
	bool CreateSortedDBFiles(RelationalOp*& _rel, bool _isLeft);

	// close temporary DBFiles and remove them from disk
	bool RemoveSortedDBFiles(vector<DBFile*>& _DBFiles);
	bool RemoveDBFile(DBFile* _dbfile);
	bool ExtractMinAndInsert(CompositeKey& key, \
																	Record& rec, \
		 															bool _isLeft, \
																	bool _ismin, \
																	CompositeKey& min);

	int* GenerateAttsToKeep();

	ostream& print(ostream& _os);

	int depth;

	int numTuples;

};

class DuplicateRemoval : public RelationalOp {
private:
	// schema of records in operator
	Schema schema;

	// operator generating data
	RelationalOp* producer;

	//hash table
	unordered_set<string> hash_tbl;

public:
	DuplicateRemoval(Schema& _schema, RelationalOp* _producer);
	virtual ~DuplicateRemoval();

	virtual bool GetNext(Record& _record);

	virtual Schema GetSchema() { return schema; }

	virtual ostream& print(ostream& _os);
};

class Sum : public RelationalOp {
private:
	// schema of records input to operator
	Schema schemaIn;
	// schema of records output by operator
	Schema schemaOut;

	// function to compute
	Function compute;

	// operator generating data
	RelationalOp* producer;

public:
	Sum(Schema& _schemaIn, Schema& _schemaOut, Function& _compute,
		RelationalOp* _producer);
	virtual ~Sum();

	virtual bool GetNext(Record& _record);

	virtual Schema GetSchema() { return schemaOut; }

	virtual ostream& print(ostream& _os);
};

class GroupBy : public RelationalOp {
private:
	// schema of records input to operator
	Schema schemaIn;
	// schema of records output by operator
	Schema schemaOut;

	// grouping attributes
	OrderMaker groupingAtts;
	// function to compute
	Function compute;

	// operator generating data
	RelationalOp* producer;

	// first-run indicator
	bool isFirst;

	// map for each grouping attribute
	unordered_map<string, GroupVal> groups;
	// unordered_map<CompositeKey, GroupVal> groups;


	// iterator for the groups
	unordered_map<string, GroupVal>::iterator groupsIt;
	// unordered_map<CompositeKey, GroupVal>::iterator groupsIt;

public:
	GroupBy(Schema& _schemaIn, Schema& _schemaOut, OrderMaker& _groupingAtts,
		Function& _compute,	RelationalOp* _producer);
	virtual ~GroupBy();

	virtual bool GetNext(Record& _record);

	virtual Schema GetSchema() { return schemaOut; }

	virtual ostream& print(ostream& _os);
};

class WriteOut : public RelationalOp {
private:
	// schema of records in operator
	Schema schema;

	// output file where to write the result records
	string outFile;

	// operator generating data
	RelationalOp* producer;

	//File output stream
	ofstream outFileStream;

public:
	WriteOut(Schema& _schema, string& _outFile, RelationalOp* _producer);
	virtual ~WriteOut();

	virtual bool GetNext(Record& _record);

	virtual Schema GetSchema() { return schema; }

	virtual ostream& print(ostream& _os);
};


class QueryExecutionTree {
private:
	RelationalOp* root;

public:
	QueryExecutionTree() {}
	virtual ~QueryExecutionTree() {}

	void ExecuteQuery() {
		while(true) {
			Record rec; if(!root->GetNext(rec)) break;
		}
	}
	void SetRoot(RelationalOp& _root) {root = &_root;}

    friend ostream& operator<<(ostream& _os, QueryExecutionTree& _op);
};

#endif //_REL_OP_H
