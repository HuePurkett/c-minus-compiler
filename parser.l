%{
//parser.l

//Hue Purkett
//CS445
//1/26/2017

#include <stdlib.h>
#include "scanType.h"
#include "parser.tab.h"
#include "symbolTable.h"
#include "yyerror.h"

int lineno=1;
extern SymbolTable records;
extern int numwarnings;

int fillTok(int tclass){
  yylval.tData=(struct td*)malloc(sizeof(struct td));
  yylval.tData->lineno=lineno;
  yylval.tData->tClass=tclass;
  yylval.tData->input=strdup(yytext);
  if(tclass==CHARCONST||tclass==BOOLCONST||tclass==NUMCONST){
    yylval.tData->isConst=1;
  }else{
    yylval.tData->isConst=0;
  }
  return tclass;
}
%}

%option noyywrap

%%
\n						{lineno++;}
record						{return fillTok(RECORD);}
static						{return fillTok(STATIC);}
int						{return fillTok(INT);}
bool						{return fillTok(BOOL);}
char						{return fillTok(CHAR);}
if						{return fillTok(IF);}
else						{return fillTok(ELSE);}
while						{return fillTok(WHILE);}
return						{return fillTok(RETURN);}
break						{return fillTok(BREAK);}
or						{return fillTok(OR);}
and						{return fillTok(AND);}
not						{return fillTok(NOT);}
true						{fillTok(BOOLCONST); yylval.tData->ival=1; return BOOLCONST;}
false						{fillTok(BOOLCONST); yylval.tData->ival=0; return BOOLCONST;}
\+\=						{return fillTok(ADDASS);}
\-\=						{return fillTok(SUBASS);}
\*\=						{return fillTok(MULASS);}
\/\=						{return fillTok(DIVASS);}
\+\+						{return fillTok(INC);}
\-\-						{return fillTok(DEC);}
\<\=						{return fillTok(LESSEQ);}
\>\=						{return fillTok(GRTEQ);}
\=\=						{return fillTok(EQ);}
\!\=						{return fillTok(NOTEQ);}
[\[\]\{\}\(\)\;\=\<\>\+\-\*\/\%\?\.\:\,]	{return fillTok(*yytext);}
[a-zA-Z][a-zA-Z0-9]*				{if(records.lookup(yytext)==NULL){
						   return fillTok(ID);
						 }else{
						   fillTok(RECTYPE);
						   delete yylval.tData->input;						//REMOVE THIS LINE FOR RECORDS
						   yylval.tData->input=strdup("record");				//REMOVE THIS LINE FOR RECORDS
						   return RECTYPE;
						 }
						}
[0-9]+						{fillTok(NUMCONST); yylval.tData->ival=atoi(yytext); return NUMCONST;}
\'\\.\'						{fillTok(CHARCONST);
                                        	 switch(yytext[2]){
                                        	   case '0':
                                        	     yylval.tData->cval='\0';
                                        	     break;
                                        	   case 'n':
                                        	     yylval.tData->cval='\n';
                                        	     break;
                                        	   default:
                                        	     yylval.tData->cval=yytext[2];
                                        	     break;
                                        	 }
						 yylval.tData->ival=yylval.tData->cval;
                                        	 return CHARCONST;}
\'.\'						{fillTok(CHARCONST); yylval.tData->cval=yytext[1]; return CHARCONST;}
\/\/.*						{/*empty*/}
[ \t]						{/*empty*/}
.						{printf("WARNING(%i): Invalid input character: '%c'.  Character ignored.\n", lineno, *yytext); numwarnings++;}

%%