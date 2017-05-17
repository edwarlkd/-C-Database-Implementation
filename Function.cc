#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>

#include "Config.h"
#include "Schema.h"
#include "ParseTree.h"
#include "Record.h"
#include "Function.h"

using namespace std;


Function :: Function () {
	opList = new Arithmetic[MAX_FUNCTION_DEPTH];
}

Function :: Function(const Function& _copyMe) : numOps(_copyMe.numOps),
	returnsInt(_copyMe.returnsInt) {
	opList = new Arithmetic[MAX_FUNCTION_DEPTH];
	memcpy(opList, _copyMe.opList, MAX_FUNCTION_DEPTH*sizeof(Arithmetic));
}

Function& Function :: operator=(const Function& _copyMe) {
	// handle self-assignment first
	if (this == &_copyMe) return *this;

	numOps = _copyMe.numOps;
	returnsInt = _copyMe.returnsInt;
	opList = new Arithmetic[MAX_FUNCTION_DEPTH];
	memcpy(opList, _copyMe.opList, MAX_FUNCTION_DEPTH*sizeof(Arithmetic));

	return *this;
}

Function :: ~Function () {
	delete [] opList;
}

Type Function :: RecursivelyBuild (FuncOperator* parseTree, Schema& mySchema) {
	// different cases; in the first case, simple, unary operation
	if ((parseTree->right == NULL) && (parseTree->leftOperand == NULL) &&
		(parseTree->code == '-')) {
		// figure out the operations on the subtree
		Type myType = RecursivelyBuild (parseTree->leftOperator, mySchema);

		// and do the operation
		if (myType == Integer) {
			opList[numOps].myOp = IntUnaryMinus;
			numOps++;
			return Integer;
		}
		else if (myType == Float) {
			opList[numOps].myOp = DblUnaryMinus;
			numOps++;
			return Float;
		}
		else {
			cerr << "ERROR: Unknown type in function!" << endl;
			return Integer;
		}
	}
	else if ((parseTree->leftOperator == NULL) && (parseTree->right == NULL)) {
		// we have either a literal value or a variable value, so do a push

		// there are two sub-cases
		// in the first case, the value is from the record that we are operating
		// over, so we will find it in the schema
		if (parseTree->leftOperand->code == NAME) {
			// first, make sure that the attribute is there
			string s(parseTree->leftOperand->value);
			int myNum = mySchema.Index (s);
			if (myNum == -1) {
				cerr << "ERROR: Attribute " << s << " in function not found!" << endl;
				return Integer;
			}

			// it is there, so get the type
			Type myType = mySchema.FindType (s);
			if (myType == String) {
				cerr << "ERROR: No string " << s << " in function!" << endl;
				return Integer;
			}

			// encode the instructions for loading from the record
			if (myType == Integer) {
				opList[numOps].myOp = PushInt;
				opList[numOps].recInput = myNum;
				opList[numOps].litInput = NULL;
				numOps++;	
				return Integer;
			}
			else {
				opList[numOps].myOp = PushDouble;
				opList[numOps].recInput = myNum;
				opList[numOps].litInput = NULL;
				numOps++;	
				return Float;
			}
		}
		else if (parseTree->leftOperand->code == INTEGER) {
			// we were given a literal integer value!
			opList[numOps].myOp = PushInt;
			opList[numOps].recInput = -1;
			opList[numOps].litInput = (void *) (new int);
			*((int *) opList[numOps].litInput) =
				atoi (parseTree->leftOperand->value);
			numOps++;
			return Integer;

		}
		else {
			opList[numOps].myOp = PushDouble;
			opList[numOps].recInput = -1;
			opList[numOps].litInput = (void *) (new double);
			*((double *) opList[numOps].litInput) =
				atof (parseTree->leftOperand->value);
			numOps++;
			return Float;
		}
	}
	else {
		// last is to deal with an arithmetic operator

		// so first, we recursively handle the left; this should give us the left
		// side's value, sitting on top of the stack
		Type myTypeLeft = RecursivelyBuild (parseTree->leftOperator, mySchema);

		// now we recursively handle the right
		Type myTypeRight = RecursivelyBuild (parseTree->right, mySchema);

		// the two values to be operated over are sitting on the stack.
		// So next we do the operation.
		// But there are potentially some typing issues.
		// If both are integers, then we do an integer operation
		if ((myTypeLeft == Integer) && (myTypeRight == Integer)) {
			if (parseTree->code == '+') {
				opList[numOps].myOp = IntPlus;
				numOps++;
				return Integer;
			}
			else if (parseTree->code == '-') {
				opList[numOps].myOp = IntMinus;
				numOps++;
				return Integer;
			}
			else if (parseTree->code == '*') {
				opList[numOps].myOp = IntMultiply;
				numOps++;
				return Integer;
			}
			else if (parseTree->code == '/') {
				opList[numOps].myOp = IntDivide;
				numOps++;
				return Integer;
			}
			else {
				cerr << "ERROR: Unknown integer operator in function!" << endl;
				return Integer;
			}
		}

		// if we got here, then at least one of the two is a double, so
		// the integer must be cast as appropriate
		if (myTypeLeft == Integer) {
			// the left operand is an ant and needs to be cast
			opList[numOps].myOp = ToDouble2Down;
			numOps++;	
		}

		if (myTypeRight == Integer) {
			// the left operand is an ant and needs to be cast
			opList[numOps].myOp = ToDouble;
			numOps++;
		}

		// now, we know that the top two items on the stack are doubles,
		// so we go ahead and do the math
		if (parseTree->code == '+') {
			opList[numOps].myOp = DblPlus;
			numOps++;
			return Float;
		}
		else if (parseTree->code == '-') {
			opList[numOps].myOp = DblMinus;
			numOps++;
			return Float;
		}
		else if (parseTree->code == '*') {
			opList[numOps].myOp = DblMultiply;
			numOps++;
			return Float;
		}
		else if (parseTree->code == '/') {
			opList[numOps].myOp = DblDivide;
			numOps++;
			return Float;
		}
		else {
			cerr << "ERROR: Unknown float operator in function!" << endl;
			return Integer;
		}
	}

	return Integer;
}

void Function :: GrowFromParseTree (FuncOperator* parseTree, Schema& mySchema) {
	// zero out the list of operations
	numOps = 0;

	// now recursively build the list
	Type resType = RecursivelyBuild (parseTree, mySchema);

	// remember if we get back an integer or if we get a double
	if (resType == Integer)	returnsInt = 1;
	else returnsInt = 0;
}

Type Function :: Apply (Record& toMe, int &intResult, double &doubleResult) {
	// this is rather simple; we just loop through and apply all of the 
	// operations that are specified during the function

	// this is the stack that holds the intermediate results from the function
	double stack[MAX_FUNCTION_DEPTH];
	double* lastPos = stack - 1;
	char *bits = toMe.GetBits();

	for (int i = 0; i < numOps; i++) {
		switch (opList[i].myOp) {
			case PushInt: {
				lastPos++;	

				// see if we need to get the integer from the record
				if (opList[i].recInput >= 0) {
					int pointer = ((int *) bits)[opList[i].recInput + 1];
					*((int *) lastPos) = *((int *) &(bits[pointer]));
				}
				else {
					// or from the literal value
					*((int *) lastPos) = *((int *) opList[i].litInput);
				}

				break;
			}
			case PushDouble: {
				lastPos++;	

				// see if we need to get the double from the record
				if (opList[i].recInput >= 0) {
					int pointer = ((int *) bits)[opList[i].recInput + 1];
					*((double *) lastPos) = *((double *) &(bits[pointer]));
				}
				else {
					// or from the literal value
					*((double *) lastPos) = *((double *) opList[i].litInput);
				}

				break;
			}
			case ToDouble: {
				*((double *) lastPos) = *((int *) lastPos);
				break;
			}
			case ToDouble2Down: {
				*((double *) (lastPos - 1)) = *((int *) (lastPos - 1));
				break;
			}
			case IntUnaryMinus: {
				*((int *) lastPos) = -(*((int *) lastPos));
				break;
			}
			case DblUnaryMinus: {
				*((double *) lastPos) = -(*((double *) lastPos));
				break;
			}
			case IntMinus: {
				*((int *) (lastPos - 1)) =
					*((int *) (lastPos - 1)) - *((int *) lastPos);
				lastPos--;
				break;
			}
			case DblMinus: {
				*((double *) (lastPos - 1)) =
					*((double *) (lastPos - 1)) - *((double *) lastPos);
				lastPos--;
				break;
			}
			case IntPlus: {
				*((int *) (lastPos - 1)) =
					*((int *) (lastPos - 1)) + *((int *) lastPos);
				lastPos--;
				break;
			}
			case DblPlus: {
				*((double *) (lastPos - 1)) =
					*((double *) (lastPos - 1)) + *((double *) lastPos);
				lastPos--;
				break;
			}
			case IntDivide: {
				*((int *) (lastPos - 1)) =
					*((int *) (lastPos - 1)) / *((int *) lastPos);
				lastPos--;
				break;
			}
			case DblDivide: {
				*((double *) (lastPos - 1)) =
					*((double *) (lastPos - 1)) / *((double *) lastPos);
				lastPos--;
				break;
			}
			case IntMultiply: {
				*((int *) (lastPos - 1)) =
					*((int *) (lastPos - 1)) * *((int *) lastPos);
				lastPos--;
				break;
			}
			case DblMultiply: {
				*((double *) (lastPos - 1)) =
					*((double *) (lastPos - 1)) * *((double *) lastPos);
				lastPos--;
				break;
			}
			default: {
				cerr << "ERROR: Unknown function operation!" << endl;
				return Integer;
			}
		}
	}

	// now, we are just about done.  First we have a sanity check to make sure
	// that exactly one value is on the stack!
	if (lastPos != stack) {
		cerr << "ERROR: Function evaluation fails!" << endl;
		return Integer;
	}

	// got here, so we are good to go; just return the final value
	if (returnsInt) {
		intResult = *((int *) lastPos);
		return Integer;
	}
	else {
		doubleResult = *((double *) lastPos);
		return Float;
	}
}

Type Function :: GetType() {
	if(returnsInt == 1)
		return Integer;
	else
		return Float;
}

string Function :: GetTypeAsString() { 
	if(returnsInt == 1)
		return "INTEGER";
	else
		return "FLOAT";
}

bool Function :: HasOps() {
	if(numOps > 0)
		return true;
	else
		return false;
}

