#ifndef _SWAPIFY_CC_
#define _SWAPIFY_CC_

#include <iostream>

#include "Swapify.h"
#include "Swap.h"

using namespace std;


template <class Type>
Swapify <Type> :: Swapify () {
}

template <class Type>
Swapify <Type> :: Swapify (const Type castFromMe) {
	data = castFromMe;
}

template <class Type>
Swapify <Type> :: ~Swapify () {
}

template <class Type>
Swapify <Type> :: operator Type () {
	return data;
}

template <class Type> void
Swapify <Type> :: Swap (Swapify &withMe) {
	SWAP(data, withMe.data);
}

template <class Type> void
Swapify <Type> :: CopyFrom (Swapify &fromMe) {
	data = fromMe.data;
}

// redefine operator << for printing
template <class Type> ostream&
operator<<(ostream& output, const Swapify<Type>& _s) {
	Swapify<Type> newObject;
	newObject.Swap(const_cast<Swapify<Type>&>(_s));

	Type st = newObject;
	output << st;

	newObject.Swap(const_cast<Swapify<Type>&>(_s));

	return output;
}

#endif //_SWAPIFY_CC_
