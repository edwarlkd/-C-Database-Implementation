/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_QUERYPARSER_H_INCLUDED
# define YY_YY_QUERYPARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YY_NAME = 258,
    YY_FLOAT = 259,
    YY_INTEGER = 260,
    YY_STRING = 261,
    SELECT = 262,
    GROUP = 263,
    DISTINCT = 264,
    BY = 265,
    FROM = 266,
    WHERE = 267,
    SUM = 268,
    AND = 269,
    CREATE_TABLE = 270,
    LOAD_DATA = 271,
    CREATE_INDEX = 272,
    TABLE = 273,
    ON = 274,
    LIST = 275,
    DROP = 276,
    SCHEMA = 277
  };
#endif
/* Tokens.  */
#define YY_NAME 258
#define YY_FLOAT 259
#define YY_INTEGER 260
#define YY_STRING 261
#define SELECT 262
#define GROUP 263
#define DISTINCT 264
#define BY 265
#define FROM 266
#define WHERE 267
#define SUM 268
#define AND 269
#define CREATE_TABLE 270
#define LOAD_DATA 271
#define CREATE_INDEX 272
#define TABLE 273
#define ON 274
#define LIST 275
#define DROP 276
#define SCHEMA 277

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 29 "QueryParser.y" /* yacc.c:1909  */

 	struct FuncOperand* myOperand;
	struct FuncOperator* myOperator; 
	struct TableList* myTables;
	struct ComparisonOp* myComparison;
	struct Operand* myBoolOperand;
	struct AndList* myAndList;
	struct NameList* myNames;
	char* actualChars;
	char whichOne;
	struct creatingTable* myCreateTable;

#line 111 "QueryParser.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_QUERYPARSER_H_INCLUDED  */
