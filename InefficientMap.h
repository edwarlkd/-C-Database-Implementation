#ifndef _INEFFICIENT_MAP_H
#define _INEFFICIENT_MAP_H

#include "Swap.h"
#include "TwoWayList.cc"

using namespace std;


template <class Key, class Data>
class InefficientMap {

public:
	typedef Key keyType;
	typedef Data dataType;

	// constructor & destructor
	InefficientMap() {}
	virtual ~InefficientMap() {}

	// remove all the content
	void Clear(void);

	// the length of the map
	int Length();

	// inserts the key/data pair into the structure
	void Insert (Key &key, Data &data);

	// this takes the contents of suckMeUp and adds them, all at once,
	// to *this; no checks of any kind are done to remove duplicates, etc.
	void SuckUp (InefficientMap &suckMeUp);

	// removes one (any) instance of the given key from the map...
	// returns a 1 on success and a zero if the given key was not found
	int Remove (Key &findMe, Key &putKeyHere, Data &putDataHere);

	// attempts to locate the given key
	// returns 1 if it is, 0 otherwise
	int IsThere (Key &findMe);

	// returns a reference to the data associated with the given search key
	// if the key is not there, then a garbage (newly initialized) Data item is
	// returned.  "Plays nicely" with IsThere in the sense that if IsThere found
	// an item, Find will immediately return that item w/o having to locate it
	Data &Find (Key &findMe);

	// swap two of the maps
	void Swap (InefficientMap &withMe);

	// get the content from another map (without destroying it)
	void CopyFrom(InefficientMap& other);

	///////////// ITERATOR INTERFAACE //////////////
	// look at the current item
	Key& CurrentKey ();
	Data& CurrentData ();

	// move the current pointer position backward through the list
	void Retreat ();

	// move the current pointer position forward through the list
	void Advance ();

	// operations to consult state
	bool AtStart ();
	bool AtEnd ();

	// operations to move the the start of end of a list
	void MoveToStart ();
	void MoveToFinish ();

private:

	struct Node {
		Key key;
		Data data;

		void Swap (Node &swapMe) {
			key.Swap (swapMe.key);
			data.Swap (swapMe.data);
		}
	};

	TwoWayList <Node> container;
};

#endif
