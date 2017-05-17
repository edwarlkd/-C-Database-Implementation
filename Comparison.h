#ifndef _COMPARISON_H
#define _COMPARISON_H

#include <iostream>

#include "Config.h"
#include "Schema.h"
#include "ParseTree.h"
#include "Record.h"

using namespace std;


// This structure stores an individual comparison that is part of a CNF
// Although defined as a class, its main role is grouping
// This is the reason that all the members are public
class Comparison {
public:
	//source of first comparison operand
	//target is the record and it can be Left, Right, or constant
	//whichAtt specifies position of comparison attribute in record schema
	Target operand1;
	int whichAtt1;

	//source of second comparison operand
	//similar to first comparison operand
	Target operand2;
	int whichAtt2;

	//type of comparison operands
	Type attType;

	//comparison operator
	CompOperator op;


	Comparison() {}
	//copy constructor
	Comparison(const Comparison& copy_me);
	Comparison& operator=(const Comparison& copy_me);
	void Swap(Comparison& swap_me);

	virtual ~Comparison() {}

	// execute comparison on records
	// return true if comparison holds, false otherwise
	bool Run(Record& left, Record& right);

	// print to the screen
    friend ostream& operator<<(ostream& _os, Comparison& _c);
};



// This structure encapsulates a sort order for records
// A sort order consists of an ordered set of attributes from the record schema
// Records are sorted based on the order of the attributes
// Although defined as a class, its main role is grouping
// This is the reason that all the members are public
class OrderMaker {
public:
	//number of attributes from the schema used for sorting
	int numAtts;

	//position and type of attributes used for sorting
	//this specifies the ordered set of attributes used for sorting the records
	int whichAtts[MAX_ANDS];
	Type whichTypes[MAX_ANDS];

	
	OrderMaker();
	//copy constructor
	OrderMaker(const OrderMaker& om);
	OrderMaker& operator=(const OrderMaker& om);
	void Swap(OrderMaker& om);

	virtual ~OrderMaker() {}

	//the sorting order incorporates all the attributes from the record schema
	//Integer attributes are first, followed by Double attributes, and, finally,
	//String attributes
	OrderMaker(Schema& schema);

	//the sorting order is explicitly specified
	//atts contains the indices of the attributes appearing in the sort order
	//atts_no gives the number of attributes from the sort order
	//the sorting attributes are extracted from the schema
	OrderMaker(Schema& schema, int* atts, int atts_no);


	//union of sort orders om1 and om2 is generated in the current object
	//it is assumed that om1 and om2 are built from the same schema
	//it is a set union because attributes are not duplicated
	void ANDMerge(OrderMaker& om1, OrderMaker& om2);

    // The OrderMaker encapsulates the specification for a sort order.
	// For example, say you are joining two tables on R.att1 = S.att2, and so
    // you want to sort R using att1. The OrderMaker specifies this sort ordering.
	// return -1, 0, or 1 if left is less than, equal to, or greater than right.
	// use the same order for both records
	int Run(Record& left, Record& right);
	// use different orders: *this for left and orderRight for right
	int Run(Record& left, Record& right, OrderMaker& orderRight);

	// print to the screen
    friend ostream& operator<<(ostream& _os, OrderMaker& _o);
};



// CNF expression to be evaluated during query execution
// Although defined as a class, its main role is grouping
// This is the reason that all the members are public
// A CNF is a comparison (logical) expression in conjunctive normal form
// It consists of multiple comparisons connected with ANDs
class CNF {
public:
	//number of comparisons
	int numAnds;
	//comparisons
	Comparison andList[MAX_ANDS];

	CNF();
	CNF(const CNF& _copy);
	CNF& operator=(const CNF& _copy);
	void Swap(CNF& _swap);

	virtual ~CNF() {}


	// this returns an instance of the OrderMaker class that
	// allows the CNF to be implemented using a sort-based
	// algorithm such as a sort-merge join.  Returns a 0 if and
	// only if it is impossible to determine an acceptable ordering
	// for the given comparison
	int GetSortOrders (OrderMaker& left, OrderMaker& right);

    // build CNF from AndList only for attributes in Schema
	// constants go into literal record
	// return 0 on success, -1 otherwise
    int ExtractCNF (AndList& parseTree, Schema& schema, Record& literal);

    // build CNF from AndList only for attributes across the two schemas
    // conditions have to involve one attribute from each schema; no constants
	// constants go into literal record
	// return 0 on success, -1 otherwise
    int ExtractCNF (AndList& parseTree,	Schema& leftSchema, Schema& rightSchema);

	// apply CNF to the records and either accept or reject them
    // return false if CNF fails, true otherwise
	bool Run(Record& left, Record& right);

	// print to the screen
    friend ostream& operator<<(ostream& _os, CNF& _o);
};


// check if the condition can be verified based on schema
// return true if yes, false otherwise
bool ConditionOnSchema(AndList& _cond, Schema& _schema);

// check if the condition can be verified based on schemas
// one attribute has to come from one schema and the other from another
// return true if yes, false otherwise
bool ConditionOnSchemas(AndList& _cond, Schema& _schemaL, Schema& _schemaR);

#endif //_COMPARISON_H
