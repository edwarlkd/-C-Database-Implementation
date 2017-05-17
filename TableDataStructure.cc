#include <string>

#include "TableDataStructure.h"

void TableDataStructure::setTname(string _t_name){
	t_name = _t_name;
}

void TableDataStructure::setNoTuples(unsigned int _no_tuples){
	no_tuples = _no_tuples;
}

void TableDataStructure::setTpath(string _t_path){
	t_path = _t_path;
}

void TableDataStructure::setSchema(Schema& _schema){
	schema = _schema;
}

void TableDataStructure::setTableProperties(string _t_name, int _no_tuples, string _t_path) {
	t_name = _t_name;
	no_tuples = _no_tuples;
	t_path = _t_path;
}
void TableDataStructure::Swap (TableDataStructure &withMe){
	schema.Swap(withMe.schema);
	string& t_name_tmp = t_name; t_name = withMe.getTname(); withMe.setTname(t_name_tmp);
	unsigned int no_tuples_tmp = no_tuples; no_tuples = withMe.getNoTuples(); withMe.setNoTuples(no_tuples_tmp);
	string& t_path_tmp = t_path; t_path = withMe.getTpath(); withMe.setTpath(t_path_tmp);
}

int TableDataStructure::IsEqual (TableDataStructure &checkMe){
	return (t_name == checkMe.getTname());
}

void TableDataStructure::CopyFrom (TableDataStructure &withMe) {
	schema = withMe.schema;
}
