#ifndef _INEFFICIENT_MAP_C
#define _INEFFICIENT_MAP_C

#include "InefficientMap.h"
#include "Keyify.cc"

#include <cstdlib>
#include <iostream>

using namespace std;


template <class Key, class Data> void
InefficientMap <Key, Data> :: SuckUp (InefficientMap &suckMe) {
	container.MoveToFinish ();
	suckMe.container.MoveToStart ();
	container.SwapRights (suckMe.container);
}

template <class Key, class Data> void
InefficientMap <Key, Data> :: Swap (InefficientMap &withMe) {
	container.Swap (withMe.container);
}

template <class Key, class Data> void
InefficientMap <Key, Data> :: CopyFrom(InefficientMap <Key, Data>& other) {
	// clean up our content
	InefficientMap empty;
	Swap(empty);

	// scan the other map and insert one element at the time
	for (other.MoveToStart(); !other.AtEnd(); other.Advance()){
		Key myKey;
		myKey.CopyFrom(other.CurrentKey());
		Data myData;
		myData.CopyFrom(other.CurrentData());

		Insert(myKey, myData);
	}
}

template <class Key, class Data> int
InefficientMap <Key, Data> :: Length() {
	int length = container.Length();
	return length;
}

template <class Key, class Data> void
InefficientMap <Key, Data> :: Insert (Key &insertKey, Data &insertData) {
	Node foo;
	foo.key.Swap (insertKey);
	foo.data.Swap (insertData);
	container.Insert (foo);
}

template <class Key, class Data> Data &
InefficientMap <Key, Data> :: Find (Key &findMe) {

	static Data garbage;
	int numRight = container.RightLength ();

	while (container.RightLength () != 0) {
		if (container.Current().key.IsEqual (findMe))
			return container.Current().data;
		container.Advance ();
	}

	container.MoveToStart ();
	while (container.RightLength () != numRight) {
		if (container.Current().key.IsEqual (findMe))
			return container.Current().data;
		container.Advance ();
	}
	return garbage;
}

template <class Key, class Data> int
InefficientMap <Key, Data> :: IsThere (Key &findMe) {

	static Data garbage;
	int numRight = container.RightLength ();
	while (container.RightLength () != 0) {
		if (container.Current().key.IsEqual (findMe)) {
			return 1;
		}
		container.Advance ();
	}

	container.MoveToStart ();
	while (container.RightLength () != numRight) {
		if (container.Current().key.IsEqual (findMe)) {
			return 1;
		}
		container.Advance ();
	}
	return 0;
}

template <class Key, class Data> void
InefficientMap <Key, Data> :: Clear(void) {
	MoveToStart();
	while (!AtEnd()){
		Key key;
		Data data;
		Node foo;
		container.Remove(foo);
		key.Swap(foo.key);
		data.Swap(foo.data);
	}
}

template <class Key, class Data> int
InefficientMap <Key, Data> :: Remove (Key &findMe, Key &putKeyHere,
	Data &putDataHere) {

	int numRight = container.RightLength ();
	Node foo;

	while (container.RightLength () != 0) {
		if (container.Current().key.IsEqual (findMe)) {
			container.Remove (foo);
			putKeyHere.Swap (foo.key);
			putDataHere.Swap (foo.data);
			return 1;
		}
		container.Advance ();
	}

	container.MoveToStart ();
	while (container.RightLength () != numRight) {
		if (container.Current().key.IsEqual (findMe)) {
			container.Remove (foo);
			putKeyHere.Swap (foo.key);
			putDataHere.Swap (foo.data);
			return 1;
		}
		container.Advance ();
	}

	return 0;
}

template <class Key, class Data> Key&
InefficientMap <Key, Data> ::CurrentKey(){ return container.Current().key; }

template <class Key, class Data> Data&
InefficientMap <Key, Data> ::CurrentData(){ return container.Current().data; }

template <class Key, class Data> void
InefficientMap <Key, Data> ::Advance(){ container.Advance();}

template <class Key, class Data> void
InefficientMap <Key, Data> ::Retreat(){ container.Retreat(); }

template <class Key, class Data> void
InefficientMap <Key, Data> ::MoveToStart(){ container.MoveToStart(); }

template <class Key, class Data> void
InefficientMap <Key, Data> ::MoveToFinish(){ container.MoveToFinish(); }

template <class Key, class Data> bool
InefficientMap <Key, Data> ::AtStart(){ return container.AtStart(); }

template <class Key, class Data> bool
InefficientMap <Key, Data> ::AtEnd(){ return container.AtEnd(); }

// redefine operator << for printing
template <class Key, class Data> ostream& operator<<(ostream& output,
	const InefficientMap<Key, Data>& _map) {
	InefficientMap<Key, Data> newObject;
	newObject.Swap(const_cast<InefficientMap<Key, Data>&>(_map));

	output << "[";
	for (newObject.MoveToStart(); !newObject.AtEnd(); newObject.Advance()) {
		if (!newObject.AtStart()) {
			output << ", ";
		}

		output << "(" << newObject.CurrentKey() << ", " << newObject.CurrentData() << ")";
	}
	output << "] : " << newObject.Length();

	newObject.Swap(const_cast<InefficientMap<Key, Data>&>(_map));

	return output;
}

#endif
