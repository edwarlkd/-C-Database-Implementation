#ifndef _COMPOSITE_KEY_H
#define _COMPOSITE_KEY_H

#include <iostream>
#include <cstring>

#include "Comparison.h"
#include "Record.h"
#include "Config.h"

using namespace std;

struct MultiTypeVar {
	Type type;
	// union {
		int valInt;
		double valDbl;
		string valStr;
	// };
};

class CompositeKey {
public:	
	CompositeKey();
	~CompositeKey();

	// add MultiTypeVar to the list
	void addInt(int _addMe);
	void addDbl(double _addMe);
	void addStr(char* _addMe);

	// extract values from Record to CompositeKey based on predicate
	void extractRecord(char*& _bits, CNF& _predicate, bool& _isLeft);

	// extract value from Record to CompositeKey with Schema
	void extractRecord(Record& _rec, Schema& _schema);

	// // retreive values
	// int getNumVars() const;
	// Type getTypeAt(int _pos) const;
	// int getIntAt(int _pos) const;
	// double getDblAt(int _pos) const;
	// string getStrAt(int _pos) const;

	bool operator==(const CompositeKey& _withMe) const;
	bool operator<(const CompositeKey& _withMe) const;
	bool operator>(const CompositeKey& _withMe) const;

	friend ostream& operator<<(ostream& _out, CompositeKey& _this);

	// members
	MultiTypeVar multiTypeList[MAX_ANDS];
	int numVars; // number of MultiTypeVars in multiTypeList
};

// magic function to combine hash
template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
	seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

// add hash function to std for convenience
namespace std {
	template<>
	struct hash<CompositeKey> {
		size_t operator()(const CompositeKey& k) const {
			size_t seed = 0;
			for(int i = 0; i < k.numVars; i++) {
				switch(k.multiTypeList[i].type) {
					case Integer:	hash_combine(seed, k.multiTypeList[i].valInt); break;
					case Float:		hash_combine(seed, k.multiTypeList[i].valDbl); break;
					case String:	hash_combine(seed, k.multiTypeList[i].valStr); break;
				}
			}
			return seed;
		}
	};
}

#endif
