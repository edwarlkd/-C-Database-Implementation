#ifndef _SCHEMA_H
#define _SCHEMA_H

#include <string>
#include <vector>
#include <iostream>
#include "Config.h"

using namespace std;


/* Data structure for schema attributes:
 * name of attribute
 * type of attribute
 * number of distinct values
 */
class Attribute {
public:
	string name;
	Type type;
	unsigned int noDistinct;

	// constructors and destructor
	Attribute();
	Attribute(const Attribute& _other);
	Attribute& operator=(const Attribute& _other);
	void Swap(Attribute& _other);

	virtual ~Attribute() {}
};


/* Class to manage schema of relations:
 * materialized on disk
 * intermediate result during query execution
 */
class Schema {
private:
	// attributes in schema
	vector<Attribute> atts;

public:
	// default constructor
	Schema() {}
	// full constructor
	Schema(vector<string>& _attributes,	vector<string>& _attributeTypes,
		vector<unsigned int>& _distincts);
	// copy constructor
	Schema(const Schema& _other);
	// assignment operator
	Schema& operator=(const Schema& _other);
	// swap function
	void Swap(Schema& _other);

	// destructor
	virtual ~Schema() {atts.clear();}

	// get functions
	unsigned int GetNumAtts() {return atts.size();}
	vector<Attribute>& GetAtts() {return atts;}

	// append other schema
	int Append(Schema& _other);

	// find index of specified attribute
	// return -1 if attribute is not present
	int Index(string& _attName);

	// find number of distincts of specified attribute
	// return -1 if attribute is not present
	int GetDistincts(string& _attName);

	// rename an attribute
	int RenameAtt(string& _oldName, string& _newName);

	// project attributes of a schema
	// only attributes indexed in the input vector are kept after projection
	// index begins from 0
	// return -1 if failure, 0 otherwise
	int Project(vector<int>& _attsToKeep);

	// find type of the specified attribute
	// return arbitrary type if attribute is not present
	// call only after Index returns valid result
	Type FindType(string& _attName);

	// operator for printing
	friend ostream& operator<<(ostream& _os, Schema& _c);
};

#endif //_SCHEMA
