#ifndef _TABLE_DATA_STRUCTURE_H
#define _TABLE_DATA_STRUCTURE_H

#include <string>
#include "Schema.h"

class TableDataStructure {

public:
	string t_name;
	unsigned int no_tuples;
	string t_path;
	Schema schema;
	// default constructor
	TableDataStructure() {}
	// destructor
	virtual ~TableDataStructure() {}

	void setTname(string _t_name);
	void setNoTuples(unsigned int _no_tuples);
	void setTpath(string _t_path);
	void setSchema(Schema& _schema);
	void setTableProperties(string _t_name, int _num_tuples, string _t_path);

	string getTname() { return t_name; }
	unsigned int getNoTuples() { return no_tuples; }
	string getTpath() { return t_path; }
	Schema& getSchema() { return schema; }

	void Swap (TableDataStructure &withMe);
	int IsEqual(TableDataStructure &checkMe);
	void CopyFrom (TableDataStructure &withMe);
	
};

#endif
