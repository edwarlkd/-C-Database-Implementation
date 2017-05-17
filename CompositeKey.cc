#include "CompositeKey.h"

using namespace std;

CompositeKey::CompositeKey() : numVars(0) { }

CompositeKey::~CompositeKey(){ }

void CompositeKey::addInt(int _addMe) {
	MultiTypeVar var;
	var.type = Integer; var.valInt = _addMe;
	multiTypeList[numVars++] = var;
}

void CompositeKey::addDbl(double _addMe) {
	MultiTypeVar var;
	var.type = Float; var.valDbl = _addMe;
	multiTypeList[numVars++] = var;
}

void CompositeKey::addStr(char* _addMe) {
	MultiTypeVar var;
	var.type = String; var.valStr = string(_addMe);
	multiTypeList[numVars++] = var;
}

void CompositeKey::extractRecord(char*& _bits, CNF& _predicate, bool& _isLeft) {
	int whichAtt;
	for (int i = 0; i < _predicate.numAnds; i++) {
		if(_isLeft ^ (_predicate.andList[i].operand1 == Left)) { // XOR
			whichAtt = _predicate.andList[i].whichAtt2;
		} else {
			whichAtt = _predicate.andList[i].whichAtt1;
		}

		switch(_predicate.andList[i].attType) {
			case Integer:
				addInt(*((int*) (_bits + ((int*) _bits)[whichAtt + 1])));
				break;
			case Float:
				addDbl(*((double*) (_bits + ((int*) _bits)[whichAtt + 1])));
				break;
			case String:
				addStr(_bits + ((int*) _bits)[whichAtt + 1]);
				break;
			default:
				cerr << "ERROR: Unknown type '" << _predicate.andList[i].attType << "'." << endl << endl;
				break;
		}
	}
}

void CompositeKey::extractRecord(Record& _rec, Schema& _schema) {
	char* bits = _rec.GetBits();
	int pointer;
	size_t n = _schema.GetNumAtts();
	for (size_t i = 0; i < n; i++) {
		pointer = ((int *) bits)[i + 1];

		switch(_schema.GetAtts()[i].type) {
			case Integer:
				addInt(*(int *) &(bits[pointer]));
				break;
			case Float:
				addDbl(*(double *) &(bits[pointer]));
				break;
			case String:
				addStr(&(bits[pointer]));
				break;
		}
	}
}

// int CompositeKey::getNumVars() const {
// 	return numVars;
// }

// Type CompositeKey::getTypeAt(int _pos) const {
// 	return multiTypeList[_pos].type;
// }

// int CompositeKey::getIntAt(int _pos) const {
// 	return multiTypeList[_pos].valInt;
// }

// double CompositeKey::getDblAt(int _pos) const {
// 	return multiTypeList[_pos].valDbl;
// }

// string CompositeKey::getStrAt(int _pos) const {
// 	return multiTypeList[_pos].valStr;
// }

bool CompositeKey::operator==(const CompositeKey& _other) const {
	for(int i = 0; i < numVars; i++) {
		switch(multiTypeList[i].type) {
			case Integer:
				if(multiTypeList[i].valInt != _other.multiTypeList[i].valInt)
					return false;
				break;
			case Float:
				if(multiTypeList[i].valDbl != _other.multiTypeList[i].valDbl)
					return false;
				break;
			case String:
				if(multiTypeList[i].valStr.compare(_other.multiTypeList[i].valStr) != 0)
					return false;
				break;
			default:
				cerr << "ERROR: Unknown type '" << multiTypeList[i].type << "'." << endl << endl;
				return false;
		}
	}

	return true;
}

bool CompositeKey::operator<(const CompositeKey& _other) const {
	for(int i = 0; i < numVars; i++) {
		switch(multiTypeList[i].type) {
			case Integer:
				if(multiTypeList[i].valInt != _other.multiTypeList[i].valInt)
					return multiTypeList[i].valInt < _other.multiTypeList[i].valInt;
				break;
			case Float:
				if(multiTypeList[i].valDbl != _other.multiTypeList[i].valDbl)
					return multiTypeList[i].valDbl < _other.multiTypeList[i].valDbl;
				break;
			case String:
				{
				int ret = multiTypeList[i].valStr.compare(_other.multiTypeList[i].valStr);
				if(ret != 0)
					return ret < 0 ? true : false;
				}
				break;
			default:
				cerr << "ERROR: Unknown type '" << multiTypeList[i].type << "'." << endl << endl;
				return false;
		}
	}

	return false; // equal
}

bool CompositeKey::operator>(const CompositeKey& _other) const {
	for(int i = 0; i < numVars; i++) {
		switch(multiTypeList[i].type) {
			case Integer:
				if(multiTypeList[i].valInt != _other.multiTypeList[i].valInt)
					return multiTypeList[i].valInt > _other.multiTypeList[i].valInt;
				break;
			case Float:
				if(multiTypeList[i].valDbl != _other.multiTypeList[i].valDbl)
					return multiTypeList[i].valDbl > _other.multiTypeList[i].valDbl;
				break;
			case String:
				{
				int ret = multiTypeList[i].valStr.compare(_other.multiTypeList[i].valStr);
				if(ret != 0)
					return ret > 0 ? true : false;
				}
				break;
			default:
				cerr << "ERROR: Unknown type '" << multiTypeList[i].type << "'." << endl << endl;
				return false;
		}
	}

	return false; // equal
}

ostream& operator<<(ostream& _out, CompositeKey& _this) {
	_out << "(";

	for(int i = 0; i < _this.numVars; i++) {
		if(i > 0) { _out << ", "; }

		switch(_this.multiTypeList[i].type) {
			case Integer: _out << _this.multiTypeList[i].valInt; break;
			case Float: _out << _this.multiTypeList[i].valDbl; break;
			case String: _out << "\"" << _this.multiTypeList[i].valStr << "\""; break;
			default: break;
		}
	}

	return _out << ")";
}
