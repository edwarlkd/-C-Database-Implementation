#ifndef _EFFICIENT_MAP_H
#define _EFFICIENT_MAP_H

// This is a template for a lookup table, associating a key
// with some data.  For large tables, this is going to be more
// efficient than the InefficientMap template, since it makes use
// of a reasonably complex skip list data structure to obtain log-n
// inserts, lookups, and removes.  However, for small tables (less
// than 20 items) it is probably going to be less efficient due to
// the extra overhead that the more complex data structure incurs.
// Also, unlike the InefficientMap, it requires the LessThan () over keys.
//
// Key requires Swap (), IsEqual (), LessThan (), CopyFrom ()
// Data requires Swap (), CopyFrom ()
//

// this constant is relatively unimportant... the data structure will have
// efficient performance up to approximately 2^MAXLEVELS items, but there is
// no reason to have it too large!

#include <cstring>

#define MAXLEVELS 64

using namespace std;


template <class Key, class Data>
class EfficientMap {

	// forward declaration
	struct Node;

public:
	typedef Key keyType;
	typedef Data dataType;


	// constructor and destructor
	EfficientMap ();
	virtual ~EfficientMap ();

	// remove all the content
	void Clear(void);

	// the length of the map
	int Length();

	// inserts the key/data pair into the structure
	void Insert (Key& key, Data& data);

	// eat up another map
	// plays nicely and removes duplicates
	void SuckUp(EfficientMap& other);

	// get the content from another map (without destroying it)
	void CopyFrom(EfficientMap& other);

	// removes one (any) instance of the given key from the map...
	// returns a 1 on success and a zero if the given key was not found
	int Remove (Key& findMe, Key& putKeyHere, Data& putDataHere);

	// attempts to locate the given key
	// returns 1 if it is, 0 otherwise
	int IsThere (Key& findMe);

	// returns a reference to the data associated with the given search key
	// if the key is not there, then a garbage (newly initialized) Data item is
	// returned.  "Plays nicely" with IsThere in the sense that if IsThere found
	// an item, Find will immediately return that item w/o having to locate it
	Data &Find (Key& findMe);

	// swap two of the maps
	void Swap (EfficientMap& withMe);

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
	// versions of the above that work only at a specific level of the skip list
	void Insert (Node* temp, int whichLevel);
	void Remove (Node*& removeMe, int whichLevel);
	void Advance (int whichLevel);
	int AtEnd (int whichLevel);
	Key& CurrentKey (int i);
	Data& CurrentData (int i);

	struct Node {
		// data
		Key key;
		Data data;

		// backward link
		Node* previous;

		// forward links
		Node** next;
		int numLinks;

		// constructors and destructor
		Node (int numLinksIn) {numLinks = numLinksIn; next = new Node*[numLinks];}
		Node () {previous = next = NULL; numLinks = 0;}
		virtual ~Node () {delete [] next;}
	};

	struct Header {
		// data
		Node* first;
		Node* last;
		Node* current;
		int curDepth;
	};

	// the list itself is pointed to by this pointer
	Header* list;
};

#endif
