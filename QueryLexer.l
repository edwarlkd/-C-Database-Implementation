/***********************************************************************
 * SECTION 1
 ***********************************************************************/
/* The code in %{ %} is included as it is in lex.yy.c.
 * It has C global variables, prototypes, and comments. 
 */


%{

#include <string.h> // for strdup()
#include <stdlib.h> // for malloc()
#include "QueryParser.h"
#include "ParseTree.h"

int lineno = 1;
void yyerror(char* s);

%}


/******************************************************************************
 * SECTION 2
 ******************************************************************************/
/* This is the DEFINITION section which contains substitutions, code, and stats.
 * It is copied into lex.yy.c as is.
 */

/******************************************************************************
 * SECTION 3
 ******************************************************************************/
/* This is the RULES section which defines how to scan and what action to take
 * for each token.
 */

%%

"SELECT"		return(SELECT);

"FROM"			return(FROM);

"WHERE"			return(WHERE);

"SUM"			return(SUM);

"AND"			return(AND);

"GROUP"			return(GROUP);

"DISTINCT"		return(DISTINCT);

"BY"			return(BY);

"CREATE_TABLE"		return(CREATE_TABLE);

"LOAD_DATA"		return(LOAD_DATA);

"CREATE_INDEX"		return(CREATE_INDEX);

"TABLE"			return(TABLE);

"ON"			return(ON);

"LIST"			return(LIST);

"DROP"			return(DROP);

"SCHEMA"		return(SCHEMA);

"("				return('(');

"<"				return('<');

">"				return('>');

"="				return('=');

")"				return(')');

"+"				return('+');

"-"				return('-');

"/"				return('/');

"*"				return('*');

","				return(',');

-?[0-9]+ {
	yylval.actualChars = strdup(yytext);
  	return(YY_INTEGER); 
}

-?[0-9]+\.[0-9]* {
	yylval.actualChars = strdup(yytext); 
  	return(YY_FLOAT);
}

[A-Za-z][A-Za-z0-9_-]* {
	yylval.actualChars = strdup(yytext);
  	return(YY_NAME);
}     

\'[^'\n]*\' {
	/* take care of ' in a string */
	if (yytext[yyleng-2] == '\\') yymore();
	else {
		yylval.actualChars = strdup(yytext+1);
		yylval.actualChars[strlen(yylval.actualChars)-1] = 0;
		return(YY_STRING);
	}
}

\n				lineno++;

[ \t]			;

.				yyerror("LEX_ERROR: invalid character");

%%

void yyerror(char *s) {
  printf("%d: %s at %s\n", lineno, s, yytext);
}

int yywrap(void) {
  return 1;
}
