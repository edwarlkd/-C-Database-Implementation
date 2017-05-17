#ifndef _FUNCTION_H
#define _FUNCTION_H

#include <iostream>

#include "Config.h"
#include "Schema.h"
#include "ParseTree.h"
#include "Record.h"

using namespace std;


struct Arithmetic {
	ArithOperator myOp;
	int recInput;
	void* litInput;
};

class Function {
private:
	Arithmetic* opList;
	int numOps;
	int returnsInt;

	// helper function
	Type RecursivelyBuild (FuncOperator* parseTree, Schema& mySchema);

public:
	Function ();
	Function(const Function& _copyMe);
	Function& operator=(const Function& _copyMe);
	virtual ~Function ();

	// this grows the specified function from a parse tree and converts
	// it into an accumulator-based computation over the attributes in
	// a record with the given schema; the record "literal" is produced
	// by the GrowFromParseTree method
	void GrowFromParseTree (FuncOperator* parseTree, Schema& mySchema);

	// applies the function to the given record and returns the result
	Type Apply (Record& toMe, int& intResult, double &doubleResult);

	// return type of result of this function (Integer or Float)
	Type GetType();

	// return type of result of this function as string (INTGER or FLOAT)
	string GetTypeAsString();

	// return true if it has operations, otherwise false
	// so that we can know whether 'sum' is required or not
	bool HasOps();
	
};

#endif // _FUNCTION_H
