#ifndef _KEYIFY_CC_
#define _KEYIFY_CC_

#include <iostream>

#include "Keyify.h"
#include "Swap.h"

using namespace std;


template <class Type>
Keyify <Type> :: Keyify () {
}

template <class Type>
Keyify <Type> :: Keyify (const Type castFromMe) {
	data = castFromMe;
}

template <class Type>
Keyify <Type> :: ~Keyify () {
}

template <class Type>
Keyify <Type> :: operator Type () {
	return data;
}

template <class Type> void
Keyify <Type> :: Swap (Keyify &withMe) {
	SWAP(data, withMe.data);
}

template <class Type> void
Keyify <Type> :: CopyFrom (Keyify &withMe) {
	data = withMe.data;
}

template <class Type> int
Keyify <Type> :: IsEqual(Keyify &checkMe) {
	return (data == checkMe.data);
}

template <class Type> int
Keyify <Type> :: LessThan (Keyify &checkMe) {
	return (data < checkMe.data);
}

// redefine operator << for printing
template <class Type> ostream&
operator<<(ostream& output, const Keyify<Type>& _s) {
	Keyify<Type> newObject;
	newObject.Swap(const_cast<Keyify<Type>&>(_s));

	Type st = newObject;
	output << st;

	newObject.Swap(const_cast<Keyify<Type>&>(_s));

	return output;
}

#endif //_KEYIFY_CC_
