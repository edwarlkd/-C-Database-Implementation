#ifndef _SWAPIFY_H_
#define _SWAPIFY_H_

// The following templates are used to quickly and easily create a class that
// wraps around a simple type (such as int) and can be put into TwoWayList or
// (In)EfficientMap.
// The "Swapify" template attaches only a Swap operation (so a simple type can
// be used with TwoWayList).

// SPECIAL NOTE: ONLY USE THESE TEMPLATES WITH SIMPLE TYPES (int, double, etc.)
// These templates use the = operation, so they are only safe with such types.
// If the type is more complicated, then create a class with Swap and IsEqual.

#include <string>

using namespace std;


template <class Type>
class Swapify {
private:
	Type data;

public:
	void Swap (Swapify& withMe);
	Swapify (const Type castFromMe);
	void CopyFrom(Swapify& FromMe);
	operator Type();

	Swapify ();
	virtual ~Swapify ();
};

typedef Swapify<double> SwapDouble;
typedef Swapify<int> SwapInt;
typedef Swapify<string> SwapString;

#endif //_SWAPIFY_H_
