#ifndef _KEYIFY_H_
#define _KEYIFY_H_

// The following templates are used to quickly and easily create a class that
// wraps around a simple type (such as an int) and can be put into TwoWayList
// or (In)EfficientMap.
// The "Keyify" template attaches both a Swap and an IsEqual (so a simple type
// can be used with either TwoWayList or (In)EfficientMap).
// For example, the following is valid code, which uses Keyify to attach a Swap
// and IsEqual to an int:
//
// void IntSwap (int& a, int& b) {
//         int temp;
//         temp = a;
//         a = b;
//         b = temp;
// }
//
// int IntCheck (int& a, int& b) {
//         return (a == b);
// }
//
// int main () {
//
// 	    typedef Keyify <int, IntSwap, IntCheck> keyifiedInt;
//      InefficientMap <keyifiedInt, keyifiedInt> foo;
//      keyifiedInt bar1, bar2;
//
//      bar1 = 12;
//      bar2 = 43;
//      foo.Insert (bar1, bar2);
//      ...
//
// SPECIAL NOTE: ONLY USE THESE TEMPLATES WITH SIMPLE TYPES (int, double, etc.)
// These templates use the = operation, so they are only safe with such types.
// If the type is more complicated, then create a class with Swap and IsEqual.

#include <string>

using namespace std;


template <class Type>
class Keyify {
private:
	Type data;

public:
	void Swap (Keyify &withMe);
	void CopyFrom (Keyify &withMe);
	Keyify (const Type castFromMe);

	operator Type();
	int IsEqual(Keyify &checkMe);
	int LessThan(Keyify &checkMe);

	Keyify ();
	virtual ~Keyify ();
};

typedef Keyify<double> KeyDouble;
typedef Keyify<int> KeyInt;
typedef Keyify<string> KeyString;

#endif //_KEYIFY_H_
