#include <iostream>
#include <cstring>
#include <regex>
#include <unordered_set>
#include "sqlite3.h"

#include "Schema.h"
#include "Catalog.h"
#include "DBFile.h"
#include <algorithm>

using namespace std;
using namespace regex_constants;

const string CATALOG_TABLES = "jarvis_tables";
const string CATALOG_ATTRS  = "jarvis_attrs";

sqlite3 *db;

// print errmsg
void printErrmsg() {
	cerr << endl << "ERROR: " << sqlite3_errmsg(db) << endl << endl;
}

// print errmsg and exit
void printErrmsgExit() {
	printErrmsg();
	throw sqlite3_errmsg(db);
}

// return true if SQL is valid, else return false
bool isValidSQL(int rc) {
	// only OK, ROW, DONE are non-error result codes
	if(rc != SQLITE_OK && rc != SQLITE_ROW && rc != SQLITE_DONE) {
		return false;
	} else {
		return true;
	}
}

bool hasDuplicates(vector<string> v) {
	unordered_set<string> tmp;
	size_t size = v.size();
	for(size_t i = 0; i < size; i++) {
		if(i == 0) {
			tmp.insert(v[i]);
		} else {
			if(!tmp.insert(v[i]).second)
				return true;
		}
	}
	return false;
}

Catalog::Catalog(string& _fileName) {
	sqlite3_stmt *stmt; int rc;	string sql;
	int numExist;

	// open sqlite file
	rc = sqlite3_open(_fileName.c_str(), &db);
	if(isValidSQL(rc)) {
		// cout << _fileName << " is opened successfully." << endl;
	} else {
		printErrmsgExit();
	}

	// check whether catalog exists
	sql = "SELECT COUNT(*) FROM sqlite_master " \
		  "WHERE type='table' AND name=?1 OR name=?2";
	sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, CATALOG_TABLES.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, CATALOG_ATTRS.c_str(), -1, SQLITE_STATIC);
	while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		numExist = sqlite3_column_int(stmt, 0);
	}
	if(!isValidSQL(rc)) { printErrmsgExit(); }
	sqlite3_finalize(stmt);

	// drop either CATALOG_TABLES or CATALOG_ATTRS which exists
	// because they must exist together!
	if(numExist == 1) {
		cout << "Resolving error in Catalog... " << flush;

		sql = "DROP TABLE IF EXISTS " + CATALOG_TABLES + ";";
		sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
		if(sqlite3_step(stmt) != SQLITE_DONE) {
			printErrmsgExit();
		}
		sqlite3_finalize(stmt);

		sql = "DROP TABLE IF EXISTS " + CATALOG_ATTRS + ";";
		sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
		if(sqlite3_step(stmt) != SQLITE_DONE) {
			printErrmsgExit();
		}
		sqlite3_finalize(stmt);

		cout << "OK!" << endl;
	}

	// create CATALOG_TABLES and CATALOG_ATTRS
	if(numExist < 2) {
		if(numExist == 0) {
			cout << "Catalog does not exist. " << flush;
		}

		cout << "Creating Catalog... " << flush;
		sql = "CREATE TABLE " + CATALOG_TABLES + "(" \
				"t_name TEXT PRIMARY KEY NOT NULL, " \
				"no_tuples INTEGER DEFAULT 0, " \
				"t_path TEXT " \
			  ");";

		sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
		if(sqlite3_step(stmt) != SQLITE_DONE) {
			printErrmsgExit();
		}
		sqlite3_finalize(stmt);

		sql = "CREATE TABLE " + CATALOG_ATTRS + "(" \
				"a_id INTEGER PRIMARY KEY AUTOINCREMENT, " \
				"t_name TEXT NOT NULL, " \
				"a_name TEXT NOT NULL, " \
				"a_type TEXT NOT NULL, " \
				"no_distinct INTEGER DEFAULT 0," \
				"FOREIGN KEY(t_name) REFERENCES " + CATALOG_TABLES + "(t_name)" \
			  ");";

		sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
		if(sqlite3_step(stmt) != SQLITE_DONE) {
			printErrmsgExit();
		}
		sqlite3_finalize(stmt);

		cout << "OK!" << endl << endl;
	} else { // load data from CATALOG_TABLES and CATALOG_ATTRS
		// cout << "Loading data from Catalog... " << flush;
		int t_id, a_id, no_tuples, no_distinct;
		string t_name, t_path;
		const unsigned char *a_name, *a_type;

		// retrieve list of tables
		sql = "SELECT t_name, no_tuples, t_path " \
				"FROM " + CATALOG_TABLES + ";";
		sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
		vector<TableDataStructure> tables;

		while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
			TableDataStructure tds;
			t_name = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
			no_tuples = sqlite3_column_int(stmt, 1);
			t_path = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
			tds.setTableProperties(t_name, no_tuples, t_path);
			tables.push_back(tds);
		}
		if(!isValidSQL(rc)) { printErrmsgExit(); }
		sqlite3_finalize(stmt);

		string attribute, type;
		vector<string> attributes, types;
		vector<unsigned int> distincts;
		vector<TableDataStructure>::iterator it;
		for (it = tables.begin(); it != tables.end(); ++it) {
			attributes.clear(); types.clear(); distincts.clear();
			t_name = (*it).getTname();
			attributes.clear(); types.clear();

			sql = "SELECT a_name, a_type, no_distinct " \
					"FROM " +  CATALOG_ATTRS + " " \
					"WHERE t_name=?1;";
			sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
			sqlite3_bind_text(stmt, 1, t_name.c_str(), -1, SQLITE_STATIC);

			while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
				attribute = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))); attributes.push_back(attribute);
				type = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))); types.push_back(type);
				distincts.push_back(sqlite3_column_int(stmt, 2));
			}
			if(!isValidSQL(rc)) { printErrmsgExit(); }

			Schema s(attributes, types, distincts);
			(*it).setSchema(s);

			KeyString key = t_name;
			tableMap.Insert(key, *it);
		}
		// cout << "OK!" << endl << endl;
	}
}

Catalog::~Catalog() {
	Catalog::Save();
	sqlite3_close(db);
}

bool Catalog::Save() {
	sqlite3_stmt *stmt; int rc;	string sql;

	sql = "BEGIN TRANSACTION;";
	rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);	
	
	// drop
	for(auto it = drop_list.begin(); it != drop_list.end(); it++) {
		sql = "DELETE FROM " + CATALOG_ATTRS + " WHERE t_name=$1;";
		sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
		sqlite3_bind_text(stmt, 1, (*it).c_str(), -1, SQLITE_STATIC);
		rc = sqlite3_step(stmt);
		if(!isValidSQL(rc)) {
			printErrmsg();
			sql = "ROLLBACK;";
			rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);
			return false;
		}

		sql = "DELETE FROM " + CATALOG_TABLES + " WHERE t_name=$1;";
		sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
		sqlite3_bind_text(stmt, 1, (*it).c_str(), -1, SQLITE_STATIC);
		rc = sqlite3_step(stmt);
		if(!isValidSQL(rc)) {
			printErrmsg();
			sql = "ROLLBACK;";
			rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);
			return false;
		}
	}

	// update
	for(auto it = numtup_map.begin(); it != numtup_map.end(); it++) {
		sql = "UPDATE " + CATALOG_TABLES + " SET no_tuples=$1 WHERE t_name=$2;";
		sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
		sqlite3_bind_int(stmt, 1, (*it).second);
		sqlite3_bind_text(stmt, 2, (*it).first.c_str(), -1, SQLITE_STATIC);
		rc = sqlite3_step(stmt);
		if(!isValidSQL(rc)) {
			printErrmsg();
			sql = "ROLLBACK;";
			rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);
			return false;
		}
	}

	for(auto it = datafile_map.begin(); it != datafile_map.end(); it++) {
		sql = "UPDATE " + CATALOG_TABLES + " SET t_path=$1 WHERE t_name=$2;";
		sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
		sqlite3_bind_text(stmt, 1, (*it).second.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, (*it).first.c_str(), -1, SQLITE_STATIC);
		rc = sqlite3_step(stmt);
		if(!isValidSQL(rc)) {
			printErrmsg();
			sql = "ROLLBACK;";
			rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);
			return false;
		}
	}

	for(auto it = numdistinct_map.begin(); it != numdistinct_map.end(); it++) {
		sql = "UPDATE " + CATALOG_ATTRS + " SET no_distinct=$1 WHERE t_name=$2 AND a_name=$3;";
		sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
		sqlite3_bind_int(stmt, 1, (*it).second);
		sqlite3_bind_text(stmt, 2, (*it).first.first.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 3, (*it).first.second.c_str(), -1, SQLITE_STATIC);
		rc = sqlite3_step(stmt);
		if(!isValidSQL(rc)) {
			printErrmsg();
			sql = "ROLLBACK;";
			rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);
			return false;
		}
	}
	// create

	// iterate through map
	for(auto it = create_list.begin(); it != create_list.end(); it++) {
		KeyString key = (*it);
		TableDataStructure tds = tableMap.Find(key);
		sql = "INSERT INTO " + CATALOG_TABLES + "(t_name, no_tuples, t_path) " \
				"VALUES(?1, ?2, '" + tds.getTpath() + "');";
		sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
		sqlite3_bind_text(stmt, 1, tds.getTname().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 2, tds.getNoTuples());
		// sqlite3_bind_text(stmt, 3, tds.getTpath().c_str(), -1, SQLITE_STATIC);
		rc = sqlite3_step(stmt);
		if(!isValidSQL(rc)) {
			printErrmsg();
			sql = "ROLLBACK;";
			rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);
			return false;
		}

		vector<Attribute> atts = tds.getSchema().GetAtts();
		for(int i = 0; i < atts.size(); i++) {
			sql = "INSERT INTO " + CATALOG_ATTRS + "(t_name, a_name, a_type, no_distinct) " \
				"VALUES(?1, ?2, ?3, ?4);";
			string typeString;
			switch(atts[i].type) {
				case Integer:
					typeString = "INTEGER"; break;
				case Float:
					typeString = "FLOAT"; break;
				case String:
					typeString = "STRING"; break;
				default:
					typeString = "UNKNOWN"; break;
			}
			sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
			sqlite3_bind_text(stmt, 1, tds.getTname().c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 2, atts[i].name.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 3, typeString.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_int(stmt, 4, atts[i].noDistinct);
			rc = sqlite3_step(stmt);
			if(!isValidSQL(rc)) {
				printErrmsg();
				sql = "ROLLBACK;";
				rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);
				return false;
			}
		}
	}

	sql = "COMMIT;";
	rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);

	// clear temp
	create_list.clear(); drop_list.clear(); 
	numtup_map.clear(); datafile_map.clear(); numdistinct_map.clear();

	return true;
}

bool Catalog::GetNoTuples(string& _table, unsigned int& _noTuples) {
	KeyString key = _table;
	if (tableMap.IsThere(key)) {
		TableDataStructure& tds = tableMap.Find(key);
		_noTuples = tds.getNoTuples();// Need to change
	}
	else {
		return false;
	}
	return true;
}

void Catalog::SetNoTuples(string& _table, unsigned int& _noTuples) {
	KeyString key = _table;
	TableDataStructure& tds = tableMap.Find(key);
	tds.setNoTuples(_noTuples);
	if (find(create_list.begin(), create_list.end(), _table) == create_list.end()) {
		numtup_map[_table] = _noTuples;
	}
}

bool Catalog::GetDataFile(string& _table, string& _path) {
	KeyString key = _table;
	if (tableMap.IsThere(key)) {
		TableDataStructure& tds = tableMap.Find(key);
		_path = tds.getTpath();
	}
	else {
		return false;
	}
	return true;
}

void Catalog::SetDataFile(string& _table, string& _path) {
	KeyString key = _table;
	TableDataStructure& tds = tableMap.Find(key);
	tds.setTpath(_path);
	if (find(create_list.begin(), create_list.end(), _table) == create_list.end()) {
		datafile_map[_table] = _path;
	}

}

bool Catalog::GetNoDistinct(string& _table, string& _attribute,
	unsigned int& _noDistinct) {
	KeyString key = _table;
	if (tableMap.IsThere(key)) {
		TableDataStructure& tds = tableMap.Find(key);
		Schema& schm = tds.getSchema();
		_noDistinct = schm.GetDistincts(_attribute);
	}
	else {
		return false;
	}
	return true;
}
void Catalog::SetNoDistinct(string& _table, string& _attribute,
	unsigned int& _noDistinct) {
	KeyString key = _table;
	TableDataStructure& tds = tableMap.Find(key);
	vector<Attribute>& atts = tds.getSchema().GetAtts();
	vector<Attribute>::iterator it;
	for (it = atts.begin(); it != atts.end(); ++it) {
		if ((*it).name ==_attribute) {
			(*it).noDistinct = _noDistinct;
			break;
		}
	}
	if (find(create_list.begin(), create_list.end(), _table) == create_list.end()) {
		pair <string, string> p = make_pair(_table, _attribute);
		numdistinct_map[p] = _noDistinct;
	}
}

void Catalog::GetTables(vector<string>& _tables) {
	//Iterate through map and add each key to vector
	vector<string> tables;
	tableMap.MoveToStart();
	while (!tableMap.AtEnd()) {
		tables.push_back(tableMap.CurrentKey());
		tableMap.Advance();
	}
	_tables = tables;
}

bool Catalog::GetAttributes(string& _table, vector<string>& _attributes) {
	KeyString key = _table;
	if (tableMap.IsThere(key)) {
		TableDataStructure& tds = tableMap.Find(key);
		vector<Attribute>& atts = tds.getSchema().GetAtts();
		//Iterate and add att names
		vector<string> anames;
		vector<Attribute>::iterator it;
		for (it = atts.begin(); it != atts.end(); ++it) {
			anames.push_back((*it).name);
		}
		_attributes = anames;
	}
	else {
		return false;
	}
	return true;
}

bool Catalog::GetSchema(string& _table, Schema& _schema) {
	KeyString key = _table;
	if (tableMap.IsThere(key)) {
		TableDataStructure& tds = tableMap.Find(key);
		_schema = tds.getSchema();
	}
	else {
		return false;
	}
	return true;
}

bool Catalog::CreateTable(string& _table, vector<string>& _attributes,
	vector<string>& _attributeTypes) {
	KeyString key = _table;
	if(!tableMap.IsThere(key)) { // check duplicated tables
		if(hasDuplicates(_attributes)) { // check duplicated attrs
			cerr << "ERROR: Duplicated attribute name." << endl << endl;
			return false;
		}

		vector<unsigned int> distincts;
		string type; smatch m;
		regex type_int("INT|INTEGER", ECMAScript | icase);
		regex type_float("FLOAT", ECMAScript | icase);
		regex type_string("STR|STRING", ECMAScript | icase);
		for(size_t i = 0; i < _attributeTypes.size(); i++) {
			distincts.push_back(0); // init no_distincts as 0

			type = _attributeTypes[i];
			if(regex_match(type, m, type_int))
				_attributeTypes[i] = "INTEGER";
			else if(regex_match(type, m, type_float))
				_attributeTypes[i] = "FLOAT";
			else if(regex_match(type, m, type_string))
				_attributeTypes[i] = "STRING";
			else
				_attributeTypes[i] = "UNKNOWN";
		}

		TableDataStructure tds;
		string userHome = getenv("HOME");
		string dbFilePath = userHome + "/.sqlite-jarvis/heap/" + _table + ".dat";
		char* dbFilePathC = new char[dbFilePath.length()+1]; 
		strcpy(dbFilePathC, dbFilePath.c_str());
		DBFile dbFile;
		if(dbFile.Open(dbFilePathC) == -1) { // DBFile does not exist
			cout << "Creating a new DBFile for table '" << _table << "'... " << flush;
			if(dbFile.Create(dbFilePathC, Heap) == 0) {
				cout << "OK!" << endl;
			} else {
				return false;
			}
		}
		tds.setTableProperties(_table, 0, dbFilePath);
		Schema s(_attributes, _attributeTypes, distincts);
		tds.setSchema(s);

		tableMap.Insert(key, tds);
		create_list.push_back(_table);
		return true;
	} else { // found duplicated tables!
		cerr << "ERROR: Duplicated table name." << endl << endl;
		return false;
	}
}

bool Catalog::DropTable(string& _table) {
	KeyString key = _table;
	if(tableMap.IsThere(key)) {
		KeyString key_tmp; TableDataStructure tds_tmp;
		if(tableMap.Remove(key, key_tmp, tds_tmp)) {
			vector <string>::iterator it = find(create_list.begin(), create_list.end(), _table);
			if (it != create_list.end()) {
				create_list.erase(it);				
			}
			else {
				unordered_map <string, int>::iterator it1 = numtup_map.find(_table);
				if(it1 != numtup_map.end()) {
					numtup_map.erase(it1);
				}
				unordered_map <string, string>::iterator it2 = datafile_map.find(_table);
				if(it2 != datafile_map.end()) {
					datafile_map.erase(it2);
				}				
				map<pair<string, string>,int>::iterator it3 = numdistinct_map.begin();
				while (it3 != numdistinct_map.end()) {
					if (it3->first.first == _table) {
						it3 = numdistinct_map.erase(it3);
					}
					else {
						it3++;
					}
				}
				drop_list.push_back(_table);
			}
			return true;
		} else {
			cerr << "ERROR: Key does not exist." << endl << endl;
			return false;
		}
	} else {
		cerr << "ERROR: Table does not exist." << endl << endl;
		return false;
	}
}

ostream& operator<<(ostream& _os, Catalog& _c) {
	_os << endl;
	for(_c.tableMap.MoveToStart(); !_c.tableMap.AtEnd(); _c.tableMap.Advance()) {
		_os << _c.tableMap.CurrentKey() << "\t" \
			<< _c.tableMap.CurrentData().getNoTuples() << "\t" \
			<< _c.tableMap.CurrentData().getTpath() << endl;

		_os << _c.tableMap.CurrentData().getSchema() << endl;
	}

	return _os;
}
