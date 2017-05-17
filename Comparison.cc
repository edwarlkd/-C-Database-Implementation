#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>

#include "Swap.h"
#include "Config.h"
#include "Schema.h"
#include "ParseTree.h"
#include "Record.h"
#include "Comparison.h"

using namespace std;


Comparison::Comparison(const Comparison& copy_me) :
	operand1(copy_me.operand1), whichAtt1(copy_me.whichAtt1),
	operand2(copy_me.operand2), whichAtt2(copy_me.whichAtt2),
	attType(copy_me.attType), op(copy_me.op) {}

Comparison& Comparison::operator=(const Comparison& copy_me) {
	// handle self-assignment first
	if (this == &copy_me) return *this;

	operand1 = copy_me.operand1; whichAtt1 = copy_me.whichAtt1;
	operand2 = copy_me.operand2; whichAtt2 = copy_me.whichAtt2;
	attType = copy_me.attType; op = copy_me.op;

	return *this;
}

void Comparison::Swap(Comparison& _swap) {
	SWAP(operand1, _swap.operand1);
	SWAP(operand2, _swap.operand2);
	SWAP(whichAtt1, _swap.whichAtt1);
	SWAP(whichAtt2, _swap.whichAtt2);
	SWAP(attType, _swap.attType);
	SWAP(op, _swap.op);
}

bool Comparison :: Run (Record& left, Record& right) {
	char* val1 = NULL;
	char* val2 = NULL;

	char* left_bits = left.GetBits();
	char* lit_bits = right.GetBits();

	// first get a pointer to the first value to compare
	if (operand1 == Left) val1 = left_bits + ((int *) left_bits)[whichAtt1 + 1];
	else val1 = lit_bits + ((int *) lit_bits)[whichAtt1 + 1];

	// next get a pointer to the second value to compare
	if (operand2 == Left) val2 = left_bits + ((int *) left_bits)[whichAtt2 + 1];
	else val2 = lit_bits + ((int *) lit_bits)[whichAtt2 + 1];

	int val1Int, val2Int, tempResult;
	double val1Double, val2Double;

	// now check the type and the comparison operation
	switch (attType) {
		// first case: we are dealing with integers
		case Integer: {
			val1Int = *((int *) val1);
			val2Int = *((int *) val2);

			// and check the operation type in order to actually do the comparison
			switch (op) {
				case LessThan: return (val1Int < val2Int);
				case GreaterThan: return (val1Int > val2Int);
				default: return (val1Int == val2Int);
			}
		}
		// second case: dealing with doubles
		case Float: {
			val1Double = *((double *) val1);
			val2Double = *((double *) val2);

			// and check the operation type in order to actually do the comparison
			switch (op) {
				case LessThan: return (val1Double < val2Double);
				case GreaterThan: return (val1Double > val2Double);
				default: return (val1Double == val2Double);
			}
		}
		// final case: dealing with strings
		default: {
			// so check the operation type in order to actually do the comparison
			tempResult = strcmp (val1, val2);
			switch (op) {
				case LessThan: return (tempResult < 0);
				case GreaterThan: return (tempResult > 0);
				default: return (tempResult == 0);
			}
		}
	}
}

ostream& operator<<(ostream& _os, Comparison& _c) {
	if (_c.operand1 == Left) _os << "Left[" << _c.whichAtt1 << "]";
	else if (_c.operand1 == Right) _os << "Right[" << _c.whichAtt1 << "]";
	else _os << "Const";

	if (_c.op == LessThan) _os << " < ";
	else if (_c.op == GreaterThan) _os << " > ";
	else _os << " = ";

	if (_c.operand2 == Left) _os << "Left[" << _c.whichAtt2 << "]";
	else if (_c.operand2 == Right) _os << "Right[" << _c.whichAtt2 << "]";
	else _os << "Const";

	if (_c.attType == Integer)	_os << " (Integer)";
	else if (_c.attType == Float) _os << " (Float)";
	else _os << " (String)";

	return _os;
}



OrderMaker :: OrderMaker() : numAtts(0) {}

OrderMaker :: OrderMaker(const OrderMaker& _o) : numAtts(_o.numAtts) {
	memcpy(whichAtts, _o.whichAtts, _o.numAtts*sizeof(int));
	memcpy(whichTypes, _o.whichTypes, _o.numAtts*sizeof(Type));
}

OrderMaker& OrderMaker::operator=(const OrderMaker& _o) {
	// handle self-assignment first
	if (this == &_o) return *this;

	memcpy(whichAtts, _o.whichAtts, _o.numAtts*sizeof(int));
	memcpy(whichTypes, _o.whichTypes, _o.numAtts*sizeof(Type));

	return *this;
}

void OrderMaker::Swap(OrderMaker& _swap) {
	int auxWhichAtts[MAX_ANDS];
	Type auxWhichTypes[MAX_ANDS];

	memcpy(auxWhichAtts, whichAtts, numAtts*sizeof(int));
	memcpy(auxWhichTypes, whichTypes, numAtts*sizeof(Type));

	memcpy(whichAtts, _swap.whichAtts, _swap.numAtts*sizeof(int));
	memcpy(whichTypes, _swap.whichTypes, _swap.numAtts*sizeof(Type));

	memcpy(_swap.whichAtts, auxWhichAtts, numAtts*sizeof(int));
	memcpy(_swap.whichTypes, auxWhichTypes, numAtts*sizeof(Type));

	SWAP(numAtts, _swap.numAtts);
}

OrderMaker :: OrderMaker(Schema& schema) : numAtts(0) {
	unsigned int n = schema.GetNumAtts();
	vector<Attribute> atts = schema.GetAtts();

	//first add the Integer attributes
	for (int i = 0; i < n; i++) {
		if (atts[i].type == Integer) {
			whichAtts[numAtts] = i;
			whichTypes[numAtts] = Integer;
			numAtts++;
		}
	}

	// now add in the doubles
	for (int i = 0; i < n; i++) {
		if (atts[i].type == Float) {
			whichAtts[numAtts] = i;
			whichTypes[numAtts] = Float;
			numAtts++;
		}
	}

	// and finally the strings
	for (int i = 0; i < n; i++) {
		if (atts[i].type == String) {
			whichAtts[numAtts] = i;
			whichTypes[numAtts] = String;
			numAtts++;
		}
	}
}

OrderMaker :: OrderMaker(Schema& schema, int* _atts, int _atts_no) {
	numAtts = _atts_no;
	vector<Attribute> atts = schema.GetAtts();

	//only the specified attributes are inserted into the sorting order
	for (int i = 0; i < numAtts; i++) {
		whichAtts[i] = _atts[i];
		whichTypes[i] = atts[_atts[i]].type;
	}
}

void OrderMaker::ANDMerge(OrderMaker& om1, OrderMaker& om2) {
	*this = om1;

	//copy the attributes from om2 that do not appear in om1
	for (int i = 0; i < om2.numAtts; i++) {
		bool found = false;
		for (int j = 0; j < numAtts; j++) {
			if (whichAtts[j] == om2.whichAtts[i]) {
				//attribute already appears in om1; discard it
				found = true;
				break;
			}
		}
		//attribute is not appearing in om1
		if (found == false)	{
			whichAtts[numAtts] = om2.whichAtts[i];
			whichTypes[numAtts] = om2.whichTypes[i];
			numAtts += 1;
		}
	}
}

int OrderMaker :: Run(Record& left, Record& right) {
	char *val1, *val2;

	char* left_bits = left.GetBits();
	char* right_bits = right.GetBits();

	for (int i = 0; i < numAtts; i++) {
		val1 = left_bits + ((int *) left_bits)[whichAtts[i] + 1];
		val2 = right_bits + ((int *) right_bits)[whichAtts[i] + 1];

		// these are used to store the two operands, depending on their type
		int val1Int, val2Int;
		double val1Double, val2Double;

		// now check the type and do the comparison
		switch (whichTypes[i]) {
			case Integer: {
				// cast the two bit strings to integers
				val1Int = *((int *) val1);
				val2Int = *((int *) val2);

				// and do the comparison
				if (val1Int < val2Int) return -1;
				else if (val1Int > val2Int)	return 1;

				break;
			}
			case Float: {
				// cast the two bit strings to doubles
				val1Double = *((double *) val1);
				val2Double = *((double *) val2);

				// and do the comparison
				if (val1Double < val2Double) return -1;
				else if (val1Double > val2Double) return 1;

				break;
			}
			default: {
				int sc = strcmp (val1, val2);
				if (sc != 0) return sc;

				break;
			}
		}
	}

	return 0;
}

int OrderMaker :: Run (Record& left, Record& right, OrderMaker& orderRight) {
	char *val1, *val2;

	char* left_bits = left.GetBits();
	char* right_bits = right.GetBits();

	for (int i = 0; i < numAtts; i++) {
		val1 = left_bits + ((int *) left_bits)[whichAtts[i] + 1];
		val2 = right_bits + ((int *) right_bits)[orderRight.whichAtts[i] + 1];

		// these are used to store the two operands, depending on their type
		int val1Int, val2Int;
		double val1Double, val2Double;

		// now check the type and do the comparison
		switch (whichTypes[i]) {
			case Integer: {
				// cast the two bit strings to integers
				val1Int = *((int *) val1);
				val2Int = *((int *) val2);

				// and do the comparison
				if (val1Int < val2Int) return -1;
				else if (val1Int > val2Int)	return 1;

				break;
			}
			case Float: {
				// cast the two bit strings to doubles
				val1Double = *((double *) val1);
				val2Double = *((double *) val2);

				// and do the comparison
				if (val1Double < val2Double) return -1;
				else if (val1Double > val2Double) return 1;

				break;
			}
			default: {
				int sc = strcmp (val1, val2);
				if (sc != 0) return sc;

				break;
			}
		}
	}

	return 0;
}

ostream& operator<<(ostream& _os, OrderMaker& _o) {
	_os << "{";
	for (int i = 0; i < _o.numAtts; i++) {
		_os << _o.whichAtts[i] << ":";
		if (_o.whichTypes[i] == Integer) _os << "Integer";
		else if (_o.whichTypes[i] == Float) _os << "Float";
		else _os << "String";

		if (i < _o.numAtts-1) _os << ", ";
	}

	_os << "}";

	return _os;
}



CNF::CNF() : numAnds(0) {}

CNF::CNF(const CNF& _copy) : numAnds(_copy.numAnds) {
	memcpy(andList, _copy.andList, _copy.numAnds*sizeof(Comparison));
}

CNF& CNF::operator=(const CNF& _o) {
	// handle self-assignment first
	if (this == &_o) return *this;

	memcpy(andList, _o.andList, _o.numAnds*sizeof(Comparison));
	numAnds = _o.numAnds;

	return *this;
}

void CNF::Swap(CNF& _swap) {
	for (int i = 0; i < MAX_ANDS; i++)
		andList[i].Swap(_swap.andList[i]);

	SWAP(numAnds, _swap.numAnds);
}

int CNF :: GetSortOrders (OrderMaker& left, OrderMaker& right) {
	// initialize the size of the OrderMakers
	left.numAtts = 0;
	right.numAtts = 0;

	// loop through all of the conditions in the CNF and find those
	// that are acceptable for use in a sort ordering:
	// - one attribute comes from one side and the other from another side
	for (int i = 0; i < numAnds; i++) {
		// verify that it operates over attributes from both tables
		if (!((andList[i].operand1 == Left && andList[i].operand2 == Right) ||
		      (andList[i].operand2 == Left && andList[i].operand1 == Right))) {
			continue;
		}

		// since we are here, we have found a join attribute!!!
		// so all we need to do is add the new comparison info into the
		// relevant structures
		if (andList[i].operand1 == Left) {
			left.whichAtts[left.numAtts] = andList[i].whichAtt1;
			left.whichTypes[left.numAtts] = andList[i].attType;
		}
		else if (andList[i].operand1 == Right) {
			right.whichAtts[right.numAtts] = andList[i].whichAtt1;
			right.whichTypes[right.numAtts] = andList[i].attType;
		}

		if (andList[i].operand2 == Left) {
			left.whichAtts[left.numAtts] = andList[i].whichAtt2;
			left.whichTypes[left.numAtts] = andList[i].attType;
		}
		else if (andList[i].operand2 == Right) {
			right.whichAtts[right.numAtts] = andList[i].whichAtt2;
			right.whichTypes[right.numAtts] = andList[i].attType;
		}

		// note that we have found two new attributes
		left.numAtts++;
		right.numAtts++;
	}

	return left.numAtts;
}

int CNF::ExtractCNF (AndList& parseTree, Schema& schema, Record& literal) {
	// build a new CNF with conditions based on the schema
	// literal will contain all the constants in the conditions

	// previous values stored in CNF are lost
	numAnds = 0;

	// this tells us the size of the literal record
	int numFieldsInLiteral = 0;
	int recSize = 0;
	int* attStart = new int[MAX_ANDS];
	char* recBits = new char[PAGE_SIZE];
	char* recPos = recBits;

	// now we go through and build the comparison structure
	AndList* currCond = &parseTree;
	while (currCond != NULL) {
		// check if at least one operand in condition is an attribute from schema
		if (ConditionOnSchema(*currCond, schema) == false) {
			// NO; go further along in the list
			currCond = currCond->rightAnd;
			continue;
		}

		// these store the types of the two values that are found
		Type typeLeft;
		Type typeRight;

		if (currCond->left->left->code == NAME) {
			// first thing is to deal with the left operand
			// so we check to see if it is an attribute name, and if so,
			// we look it up in the schema
			// see if we can find this attribute in the schema
			string s(currCond->left->left->value);
			int leftIdx = schema.Index(s);
			if (leftIdx != -1) {
				andList[numAnds].operand1 = Left;
				andList[numAnds].whichAtt1 = leftIdx;
				typeLeft = schema.FindType(s);
			}
		}
		else if (currCond->left->left->code == STRING) {
			// the next thing is to see if we have a string; if so, add it to the
			// literal record that stores all of the comparison values
			andList[numAnds].operand1 = Literal;
			andList[numAnds].whichAtt1 = numFieldsInLiteral;
			typeLeft = String;

			// add to record literal
			attStart[numFieldsInLiteral] = recSize;
			int cLen = strlen(currCond->left->left->value);
			memcpy(recPos, currCond->left->left->value, cLen);

			if (cLen % sizeof (int) != 0) {
				cLen += sizeof (int) - (cLen % sizeof (int));
			} else {
				cLen += sizeof (int);
			}			
			recSize += cLen;
			recPos += cLen;
			numFieldsInLiteral += 1;
		}
		else if (currCond->left->left->code == INTEGER) {
			// see if it is an integer
			andList[numAnds].operand1 = Literal;
			andList[numAnds].whichAtt1 = numFieldsInLiteral;
			typeLeft = Integer;

			// add to record literal
			attStart[numFieldsInLiteral] = recSize;
			int cLen = sizeof(int);
			*((int *) recPos) = atoi (currCond->left->left->value);
			recSize += cLen;
			recPos += cLen;
			numFieldsInLiteral += 1;
		}
		else if (currCond->left->left->code == FLOAT) {
			// see if it is a double
			andList[numAnds].operand1 = Literal;
			andList[numAnds].whichAtt1 = numFieldsInLiteral;
			typeLeft = Float;

			// add to record literal
			attStart[numFieldsInLiteral] = recSize;
			int cLen = sizeof(double);
			*((double *) recPos) = atof (currCond->left->left->value);
			recSize += cLen;
			recPos += cLen;
			numFieldsInLiteral += 1;
		}
		else {
			// catch-all case
			cerr << "ERROR: Unknown type for " << currCond->left->left->value << "!" << endl;
			return -1;
		}

		// now that we have dealt with the left operand, we need to deal with the
		// right operand
		if (currCond->left->right->code == NAME) {
			// first thing is to deal with the left operand
			// so we check to see if it is an attribute name, and if so,
			// we look it up in the schema
			// see if we can find this attribute in the schema
			string s(currCond->left->right->value);
			int leftIdx = schema.Index(s);
			if (leftIdx != -1) {
				andList[numAnds].operand2 = Left;
				andList[numAnds].whichAtt2 = leftIdx;
				typeRight = schema.FindType(s);
			}
		}
		else if (currCond->left->right->code == STRING) {
			// the next thing is to see if we have a string; if so, add it to the
			// literal record that stores all of the comparison values
			andList[numAnds].operand2 = Literal;
			andList[numAnds].whichAtt2 = numFieldsInLiteral;
			typeRight = String;

			// add to record literal
			attStart[numFieldsInLiteral] = recSize;
			int cLen = strlen(currCond->left->right->value);
			memcpy(recPos, currCond->left->right->value, cLen);

			if (cLen % sizeof (int) != 0) {
				cLen += sizeof (int) - (cLen % sizeof (int));
			} else {
				cLen += sizeof (int);
			}
			recSize += cLen;
			recPos += cLen;
			numFieldsInLiteral += 1;
		}
		else if (currCond->left->right->code == INTEGER) {
			// see if it is an integer
			andList[numAnds].operand2 = Literal;
			andList[numAnds].whichAtt2 = numFieldsInLiteral;
			typeRight = Integer;

			// add to record literal
			attStart[numFieldsInLiteral] = recSize;
			int cLen = sizeof(int);
			*((int *) recPos) = atoi (currCond->left->right->value);
			recSize += cLen;
			recPos += cLen;
			numFieldsInLiteral += 1;
		}
		else if (currCond->left->right->code == FLOAT) {
			// see if it is a double
			andList[numAnds].operand2 = Literal;
			andList[numAnds].whichAtt2 = numFieldsInLiteral;
			typeRight = Float;

			// add to record literal
			attStart[numFieldsInLiteral] = recSize;
			int cLen = sizeof(double);
			*((double *) recPos) = atof (currCond->left->right->value);
			recSize += cLen;
			recPos += cLen;
			numFieldsInLiteral += 1;
		}
		else {
			// catch-all case
			cerr << "ERROR: Unknown type for " << currCond->left->right->value << "!" << endl;
			return -1;
		}


		// now we check to make sure that there was not a type mismatch
		if (typeLeft != typeRight) {
			cerr << "ERROR: Type mismatch for " << currCond->left->left->value
				<< " AND "	<< currCond->left->right->value << "!" << endl;
			return -1;
		}

		// set up the type info for this comparison
		andList[numAnds].attType = typeLeft;

		// and finally set up the comparison operator for this comparison
		if (currCond->left->code == LESS_THAN) andList[numAnds].op = LessThan;
		else if (currCond->left->code == GREATER_THAN) andList[numAnds].op = GreaterThan;
		else if (currCond->left->code == EQUALS) andList[numAnds].op = Equals;
		else {
			cerr << "ERROR: Unknown comparison operator for " << currCond->left->left->value
					<< " AND "	<< currCond->left->right->value << "!" << endl;
			return -1;
		}

		numAnds += 1;
		currCond = currCond->rightAnd;
	}

	if(numAnds > 0) {
		// create the literal record
		char* recComplete = new char[(numFieldsInLiteral+1)*sizeof(int)+recSize];
		((int*) recComplete)[0] = (numFieldsInLiteral+1)*sizeof(int)+recSize;
		for (int i = 0; i < numFieldsInLiteral; i++)
			((int*) recComplete)[i+1] = (numFieldsInLiteral+1)*sizeof(int)+attStart[i];
		memcpy(recComplete+(numFieldsInLiteral+1)*sizeof(int), recBits, recSize);

		literal.Consume(recComplete);
	}

	return 0;
}

int CNF::ExtractCNF (AndList& parseTree, Schema& leftSchema, Schema& rightSchema) {
	// build a new CNF with conditions based on the schemas
	// one attribute has to come from one schema and the other from another

	// previous values stored in CNF are lost
	numAnds = 0;

	// now we go through and build the comparison structure
	AndList* currCond = &parseTree;
	while (currCond != NULL) {
		// check if at least one operand in condition is an attribute from schema
		if (ConditionOnSchemas(*currCond, leftSchema, rightSchema) == false) {
			// NO; go further along in the list
			currCond = currCond->rightAnd;
			continue;
		}

		// these store the types of the two values that are found
		Type typeLeft;
		Type typeRight;

		string s(currCond->left->left->value);
		int leftIdx = leftSchema.Index(s);
		if (leftIdx != -1) {
			andList[numAnds].operand1 = Left;
			andList[numAnds].whichAtt1 = leftIdx;
			typeLeft = leftSchema.FindType(s);
		}
		else {
			int rightIdx = rightSchema.Index(s);
			andList[numAnds].operand1 = Right;
			andList[numAnds].whichAtt1 = rightIdx;
			typeRight = rightSchema.FindType(s);
		}

		s = currCond->left->right->value;
		leftIdx = leftSchema.Index(s);
		if (leftIdx != -1) {
			andList[numAnds].operand2 = Left;
			andList[numAnds].whichAtt2 = leftIdx;
			typeLeft = leftSchema.FindType(s);
		}
		else {
			int rightIdx = rightSchema.Index(s);
			andList[numAnds].operand2 = Right;
			andList[numAnds].whichAtt2 = rightIdx;
			typeRight = rightSchema.FindType(s);
		}

		// now we check to make sure that there was not a type mismatch
		if (typeLeft != typeRight) {
			cerr << "ERROR: Type mismatch for " << currCond->left->left->value
				<< " AND "	<< currCond->left->right->value << "!" << endl;
			return -1;
		}

		// set up the type info for this comparison
		andList[numAnds].attType = typeLeft;

		// and finally set up the comparison operator for this comparison
		if (currCond->left->code == LESS_THAN) andList[numAnds].op = LessThan;
		else if (currCond->left->code == GREATER_THAN) andList[numAnds].op = GreaterThan;
		else if (currCond->left->code == EQUALS) andList[numAnds].op = Equals;
		else {
			cerr << "ERROR: Unknown comparison operator for " << currCond->left->left->value
					<< " AND "	<< currCond->left->right->value << "!" << endl;
			return -1;
		}

		numAnds += 1;
		currCond = currCond->rightAnd;
	}

	return 0;
}

bool CNF :: Run (Record& left, Record& right) {
	for (int i = 0; i < numAnds; i++) {
		bool result = andList[i].Run(left, right);
		if (result == false) return false;
	}

	return true;
}

ostream& operator<<(ostream& _os, CNF& _o) {
	for (int i = 0; i < _o.numAnds; i++) {
		if (i == 0) _os << "("; else _os << " (";
		_os << _o.andList[i];
		if (i < _o.numAnds-1) _os << ") AND"; else _os << ")";
	}

	return _os;
}


bool ConditionOnSchema(AndList& _cond, Schema& _schema) {
	bool isGood = false;

	if (_cond.left->left->code == NAME) {
		string s(_cond.left->left->value);
		int leftIdx = _schema.Index(s);
		if (leftIdx != -1) isGood = true;
		else return false;
	}

	if (_cond.left->right->code == NAME) {
		string s(_cond.left->right->value);
		int rightIdx = _schema.Index(s);
		if (rightIdx != -1) isGood = true;
		else return false;
	}

	return isGood;
}

bool ConditionOnSchemas(AndList& _cond, Schema& _schemaL, Schema& _schemaR) {
	int sL = 0, sR = 0;

	if (_cond.left->left->code == NAME) {
		string s(_cond.left->left->value);
		int idx = _schemaL.Index(s);
		if (idx != -1) sL += 1;
		idx = _schemaR.Index(s);
		if (idx != -1) sR += 1;
	}

	if (sL+sR != 1) return false;

	if (_cond.left->right->code == NAME) {
		string s(_cond.left->right->value);
		int idx = _schemaL.Index(s);
		if (idx != -1) sL += 1;
		idx = _schemaR.Index(s);
		if (idx != -1) sR += 1;
	}

	if ((sL == 1) && (sR == 1)) return true;
	else return false;
}
