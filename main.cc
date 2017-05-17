#include <iostream>
#include <cstring>
#include <unistd.h>

#include "Catalog.h"
#include "QueryOptimizer.h"
#include "QueryCompiler.h"
#include "RelOp.h"
extern "C" { // due to "previous declaration with ‘C++’ linkage"
	#include "QueryParser.h"
}

using namespace std;


// these data structures hold the result of the parsing
extern struct FuncOperator* finalFunction; // the aggregate function
extern struct TableList* tables; // the list of tables in the query
extern struct AndList* predicate; // the predicate in WHERE
extern struct NameList* groupingAtts; // grouping attributes
extern struct NameList* attsToSelect; // the attributes in SELECT
extern int distinctAtts; // 1 if there is a DISTINCT in a non-aggregate query
extern char* command; // command to execute other functionalities (e.g. exit)
extern int Query;
extern char* table; // single table name for CREATE and LOAD
extern struct creatingTable* createTable; // attributes and types to be inserted
extern char* textFile; // text file to be loaded into a table
extern char* indexName; // index name for CREATE INDEX
extern char* attr; // single attribute name for CREATE INDEX


extern "C" int yyparse();
extern "C" int yylex_destroy();

int NUM_PAGES_AVAILABLE = 100; // default number of pages available in a DBFile

int main (int argc, char* argv[]) {
	// this is the catalog

	string FileName = "catalog.sqlite";
	Catalog catalog(FileName);

	// this is the query optimizer
	// it is not invoked directly but rather passed to the query compiler
	QueryOptimizer optimizer(catalog);

	// this is the query compiler
	// it includes the catalog and the query optimizer
	QueryCompiler compiler(catalog, optimizer);

	// set NUM_PAGES_AVAILABLE from the first argument
	if(argc == 2) {
		NUM_PAGES_AVAILABLE = atoi(argv[1]);
	}

	while(1)
	{
		int enter;

		cout<<"Choose from the following Options:"<<endl;
		cout<<"Enter 1 TO RUN A QUERY"<<endl;
		cout<<"Enter 2 TO CREATE A TABLE"<<endl;
		cout<<"Enter 3 TO LOAD DATA INTO A TABLE"<<endl;
		cout<<"Enter 4 TO CREATE A SCAN INDEX"<<endl;
		cout<<"Enter 5 TO EXIT THE PROGRAM"<<endl;
		cin>>enter;
		//Query = enter;
		
		switch(enter)
		{

			case 1: 
			{
				

				while(true) 
				{
					//string query;
					cout << "ENTER QUERY HERE: "<<endl;
					/*cin>>query;
					cout<<"QUERY: "<<endl;
					cout<<query<<endl;*/

					// the query parser is accessed directly through yyparse
					// this populates the extern data structures
					int parse = -1;
					if (yyparse() == 0) {
						parse = 0;
					} else {
						cout << "Error: Query is not correct!" << endl << endl;
						parse = -1;
					}

					yylex_destroy();


					if(parse == 0) {
						if(command != NULL) {
							if(strcmp(command, "exit") == 0) {
								cout << endl << "Exiting...!" << endl << endl;
								return 0;
							} else {
								cout << endl << "Error: Command not found." << endl << endl;
								command = NULL;
							}
						} else {
							cout << endl << "OK!" << endl;

							// at this point we have the parse tree in the ParseTree data structures
							// we are ready to invoke the query compiler with the given query
							// the result is the execution tree built from the parse tree and optimized
							QueryExecutionTree queryTree;
							compiler.Compile(tables, attsToSelect, finalFunction, predicate,
								groupingAtts, distinctAtts, queryTree, createTable, Query);

							cout << queryTree << endl;

							queryTree.ExecuteQuery();
							break;
						}
					}

					// detect input redirection
					if(!isatty(STDIN_FILENO)) { return 0; }
				}
				break;
			}
			case 2:
			{
				cout<<"CREATE TABLE:"<<endl;
				int parse = -1;
					if (yyparse() == 0) 
					{
						parse = 0;
					} 
					else 
					{
						cout << "Error: Query is not correct!" << endl << endl;
						parse = -1;
					}

					yylex_destroy();
				if(parse == 0) 
				{
						if(table != NULL && createTable != NULL)
						{
							cout<<"HERE IN CREATE TABLE"<<endl;
							vector<string> atts;
							vector<string> typs;

							while(createTable != NULL)
							{
								string att(createTable->attName);
								atts.push_back(att);
								string typ(createTable->attType);
								typs.push_back(typ);
								createTable = createTable->next;
							}

							string tabl(table);

							cout<<tabl<<endl;
							for(int i = 0; i <typs.size(); i++)
							{
								cout<<"attributes: "<<atts[i]<<endl;
								cout<<"type: "<<typs[i]<<endl; 
							}
							catalog.CreateTable(tabl, atts, typs);
							catalog.Save();

						}
						if(command != NULL) 
						{
							if(strcmp(command, "exit") == 0) 
							{
								cout << endl << "Exiting...!" << endl << endl;
								return 0;
							} 
							else 
							{
								cout << endl << "Error: Command not found." << endl << endl;
								command = NULL;
							}
						} 
				}
				break;
			}

			case 3: 
			{
				cout<<"LOAD DATA IN TABLES"<<endl;
				int parse = -1;
					if (yyparse() == 0) 
					{
						parse = 0;
					} 
					else 
					{
						cout << "Error: Query is not correct!" << endl << endl;
						parse = -1;
					}

					yylex_destroy();
				if(parse == 0) 
				{
					if(table != NULL && textFile != NULL)
					{
						string tab(table);
						//string attts = attsToSelect->name;
						//string txtFile = tab + ".tbl";
						FileType typ = Heap;
						DBFile db;

						//cout<<"TABLE: "<<tab<<endl;

						db.Create(&tab[0], typ);
						Schema s;
						catalog.GetSchema(tab, s);
						//cout<<"SCHEMA IS: "<<s<<endl;
						db.Load(s, &textFile[0]);
						
					}
					if(command != NULL) 
					{
						if(strcmp(command, "exit") == 0) 
						{
							cout << endl << "Exiting...!" << endl << endl;
							return 0;
						} 
						else 
						{
							cout << endl << "Error: Command not found." << endl << endl;
							command = NULL;
						}
					}
				}
				break;
			}
			case 4:
			{

				cout<<"SCAN INDEX"<<endl;
				
									int parse = -1;
					if (yyparse() == 0) {
						parse = 0;
					} else {
						cout << "Error: Query is not correct!" << endl << endl;
						parse = -1;
					}

					yylex_destroy();


					if(parse == 0) {
						if(command != NULL) {
							if(strcmp(command, "exit") == 0) {
								cout << endl << "Exiting...!" << endl << endl;
								return 0;
							} else {
								cout << endl << "Error: Command not found." << endl << endl;
								command = NULL;
							}
						} else {
							cout << endl << "OK!" << endl;

							// at this point we have the parse tree in the ParseTree data structures
							// we are ready to invoke the query compiler with the given query
							// the result is the execution tree built from the parse tree and optimized
							QueryExecutionTree queryTree;
							compiler.Compile(tables, attsToSelect, finalFunction, predicate,
								groupingAtts, distinctAtts, queryTree, createTable, Query);

							//cout << queryTree << endl;

							queryTree.ExecuteQuery();
							break;
						}
					}

					// detect input redirection
					if(!isatty(STDIN_FILENO)) { return 0; }
				
				break;
			}
			case 5:
			{
				cout<<"THANK YOU GOOD BYE"<<endl;
				return 0;
			}
		}
	}

	return 0;
}
