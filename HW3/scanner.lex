%{
#include "SemanticalStructs.h"
#include <iostream>
#include <string>
#include "parser.tab.hpp"
#include "hw3_output.hpp"
using std::string;
using namespace output;
%}
%option nounput
%option yylineno
%option noyywrap
%x REALLYEND

EQUALITY "=="|"!="
RELATIONAL "<"|">"|"<="|">="
MULBINOP "*"|"/"
ADDBINOP "+"|"-"
COMMENT \/\/[^\r\n]*(\r|\n|\r\n)?
ID [a-zA-Z]([a-zA-Z]|[0-9])*
NUM ([1-9][0-9]*)|0
ESCAPE ((\\t|\\n|\\r|\\\"|\\\\|\\0|\\(x([0-9]|[a-fA-F])([0-9]|[a-fA-F])))
UNDEFIND_ESCAPE \\[^\n\r\"]|\\x[^\n\r\"]|\\x
STRING \"({ESCAPE})|[^\n\r\"\\]|({UNDEFIND_ESCAPE}))*\"
UNCLOSED_STRING \"[^\"]*


%%
"void" 		 		{return VOID;}
"int" 		 		{return INT;}
"byte" 		 		{return BYTE;}
"b" 		 		{return B;}
"bool" 		 		{return BOOL;}
"and"			 	{return AND;}
"or" 		 		{return OR;}
"not" 				{return NOT;}
"true" 				{return TRUE;}
"false" 			{return FALSE;}
"return" 			{return RETURN;}
"if" 	 			{return IF;}
"else" 	 			{return ELSE;}
"while" 	 		{return WHILE;}
"break" 	 		{return BREAK; }
"continue" 	 		{return CONTINUE;}
"switch" 	 		{return SWITCH;}
"case" 	 			{return CASE;}
"default" 	 		{return DEFAULT;}
":"		 	 		{return COLON;}
";" 	 			{return SC;}
","		 			{return COMMA;}
"("		 	 		{return LPAREN;}
")"		 	 		{return RPAREN;}
"{"		 	 		{return LBRACE;}
"}"		 	 		{return RBRACE;}
"="		 	 		{return ASSIGN;}
{EQUALITY} 	 		{return EQUALITY;}
{RELATIONAL}        {return RELATIONAL;}
{MULBINOP} 	 		{return MULBINOP;}
{ADDBINOP}          {return ADDBINOP;}
{COMMENT} 	 		{}
{ID} 	 	 		{yylval.string_val = yytext; return ID;}
{NUM} 	 	 		{yylval.string_val = yytext; return NUM;}
{STRING}		 	{yylval.string_val = yytext; return STRING;}
[ \t\r\n]		 	{}
{UNCLOSED_STRING}   { errorLex(yylineno); exit(0);}
. 			 		{ errorLex(yylineno); exit(0);}
<INITIAL><<EOF>>        { BEGIN(REALLYEND); return EOP; }
<REALLYEND><<EOF>>      { return 0; }
%%