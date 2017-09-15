%{
//parser.y

//Hue Purkett
//CS445
//2/16/2017


#include <stdio.h>
#include <stdlib.h>
#include "symbolTable.h"
#include "scanType.h"
#include "printtree.h"
#include "semantic.h"
#include "yyerror.h"

#define YYERROR_VERBOSE

extern int yylex();
extern int goffset;

extern void gencode(struct tnd*, int, bool);
extern void emitAll(struct tnd*);
FILE *code;
extern char *optarg;

SymbolTable records;
struct tnd *AST;
int numerrors=0, numwarnings=0;

extern int ourGetopt(int , char**, char*);
extern int optind;
extern SymbolTable declarations;

struct tnd *newNode(struct td *token, struct tnd *c0, struct tnd *c1, struct tnd *c2, int idCase){
  struct tnd *tmp=(struct tnd*)malloc(sizeof(struct tnd));
  tmp->children[0]=c0;
  tmp->children[1]=c1;
  tmp->children[2]=c2;
  tmp->sibling=NULL;
  tmp->token=token;
  tmp->lineno=token->lineno;
  tmp->tClass=token->tClass;
  tmp->isConst=token->isConst;
  tmp->isArray=false;
  tmp->arraylen=0;
  tmp->varType=NULL;
  tmp->idCase=idCase;
  tmp->offset=0;
  tmp->isParam=false;
  tmp->isLocal=false;
  tmp->size=1;
  return tmp;
}

void appendSib(struct tnd *sib, struct tnd *sibs){
  if(sibs==NULL){return;}//may happen now that there are error tokens.
  if(sibs->sibling==NULL){
    sibs->sibling=sib;
  }else{
    appendSib(sib, sibs->sibling);
  }
}

void settype(struct td *type, struct tnd *sibs){
  if(sibs==NULL||sibs->varType!=NULL){return;}
  sibs->varType=type;
  sibs->isStatic=type->isStatic;
  settype(type, sibs->sibling);
}

void setLocal(struct tnd *sibs){
  if(sibs==NULL){return;}
  sibs->isLocal=true;
  setLocal(sibs->sibling);
}

struct td *newType(int w){
  struct td *t=(struct td*)malloc(sizeof(struct td));
  t->lineno=-1;
  t->ival=0;
  t->cval=0;
  t->isStatic=0;
  t->isConst=0;
  switch(w){
    case 0://int
      t->tClass=INT;
      t->input=strdup("int");
      break;
    case 1://bool
      t->tClass=BOOL;
      t->input=strdup("bool");
      break;
    case 2://char
      t->tClass=CHAR;
      t->input=strdup("char");
      break;
  }
}


struct tnd *newTypedNode(int w, struct td *t, int i){
  struct tnd *a=newNode(t, NULL, NULL, NULL, i);
  a->varType=newType(w);
  a->isLocal=true;
  return a;
}


struct td *newID(char *i){
  struct td *t=(struct td*)malloc(sizeof(struct td));
  t->tClass=ID;
  t->input=strdup(i);
  t->lineno=-1;
  t->ival=0;
  t->cval=0;
  t->isStatic=0;
}


%}

%union{
  struct td *tData;
  struct tnd *tNode;
}

%type <tNode> program declarationList declaration
%type <tNode> recDeclaration
%type <tNode> varDeclaration scopedVarDeclaration varDeclList varDeclInitialize varDeclId
%type <tData> scopedTypeSpecifier typeSpecifier returnTypeSpecifier
%type <tNode> funDeclaration params paramList paramTypeList paramIdList paramId
%type <tNode> statement compoundStmt localDeclarations statementList expressionStmt matched unmatched returnStmt breakStmt
%type <tNode> expression simpleExpression andExpression unaryRelExpression relExpression sumExpression term unaryExpression factor mutable
%type <tData> relop sumop mulop unaryop assignop
%type <tNode> immutable call args argList constant

%token <tData> CHARCONST ID NUMCONST AND OR NOT BOOLCONST NOTEQ MULASS INC
%token <tData> ADDASS DEC SUBASS DIVASS LESSEQ EQ GRTEQ BOOL BREAK CHAR ELSE
%token <tData> IF IN RETURN STATIC WHILE RECORD INT RECTYPE
%token <tData> '[' ']' '{' '}' '(' ')' ':' ';' '=' '<' '>' '+' '-' '*' '/' '\x25' '?' '.' ','

%%

program			: declarationList					{AST=newTypedNode(0, newID((char*)"input"), 3);
										 appendSib(newNode(newID((char*)"output"), newTypedNode(0, newID((char*)"*dummy*"), 4), NULL, NULL, 3), AST);
										 appendSib(newTypedNode(1, newID((char*)"inputb"), 3), AST);
										 appendSib(newNode(newID((char*)"outputb"), newTypedNode(1, newID((char*)"*dummy*"), 4), NULL, NULL, 3), AST);
										 appendSib(newTypedNode(2, newID((char*)"inputc"), 3), AST);
										 appendSib(newNode(newID((char*)"outputc"), newTypedNode(2, newID((char*)"*dummy*"), 4), NULL, NULL, 3), AST);
										 appendSib(newNode(newID((char*)"outnl"), NULL, NULL, NULL, 3), AST);
										 appendSib($1, AST);
										}
			;
declarationList		: declarationList declaration				{appendSib($2, $1); $$=$1;}
			| declaration						{$$=$1;}
			;
declaration		: varDeclaration					{$$=$1;}
			| funDeclaration					{$$=$1;}
			| recDeclaration					{$$=$1;}
			| error                                                 {$$=NULL; /*ERROR*/}
			;


recDeclaration		: RECORD ID '{' localDeclarations '}'			{$$=newNode($2, $4, NULL, NULL, 1); records.insert($2->input, $$);}
			;


varDeclaration		: typeSpecifier varDeclList ';'				{yyerrok; $$=$2; settype($1, $2);}
			| error varDeclList ';'					{$$=NULL; /*ERROR*/}
			| typeSpecifier error ';'				{yyerrok; $$=NULL; /*ERROR*/}
			;
scopedVarDeclaration	: scopedTypeSpecifier varDeclList ';'			{yyerrok; $$=$2; settype($1, $2);}
			| error varDeclList ';'					{yyerrok; $$=NULL; /*ERROR*/}
			| scopedTypeSpecifier error ';'				{yyerrok; $$=NULL; /*ERROR*/}
			;
varDeclList		: varDeclList ',' varDeclInitialize			{yyerrok; appendSib($3, $1); $$=$1;}
			| varDeclInitialize					{$$=$1;}
			| varDeclList ',' error					{$$=NULL; /*ERROR*/}
                        | error                                                 {$$=NULL; /*ERROR*/}
			;
varDeclInitialize	: varDeclId						{$$=$1;}
			| varDeclId ':' simpleExpression			{$$=$1; $$->children[0]=$3;}
			| error ':' simpleExpression				{yyerrok; $$=NULL; /*ERROR*/}
			| varDeclId ':' error					{$$=NULL; /*ERROR*/}
			;
varDeclId		: ID							{$$=newNode($1, NULL, NULL, NULL, 2);}
			| ID '[' NUMCONST ']'					{$$=newNode($1, NULL, NULL, NULL, 2); $$->isArray=true; $$->arraylen=$3->ival;}
			| ID '[' error						{$$=NULL; /*ERROR*/}
			| error ']'						{yyerrok; $$=NULL; /*ERROR*/}
			;
scopedTypeSpecifier	: STATIC typeSpecifier					{$$=$2; $$->isStatic=1;}
			| typeSpecifier						{$$=$1;}
			;
typeSpecifier		: returnTypeSpecifier					{$$=$1;}
			| RECTYPE						{$$=$1;}
			;
returnTypeSpecifier	: INT							{$$=$1;}
			| BOOL							{$$=$1;}
			| CHAR							{$$=$1;}
			;


funDeclaration		: typeSpecifier ID '(' params ')' statement		{$$=newNode($2, $4, $6, NULL, 3); $$->varType=$1;}
			| ID '(' params ')' statement				{$$=newNode($1, $3, $5, NULL, 3); $$->varType=NULL;}
			| typeSpecifier error					{$$=NULL; /*ERROR*/}
			| typeSpecifier ID '(' error				{$$=NULL; /*ERROR*/}
			| typeSpecifier ID '(' params ')' error			{$$=NULL; /*ERROR*/}
			| ID '(' error						{$$=NULL; /*ERROR*/}
			| ID '(' params ')' error				{$$=NULL; /*ERROR*/}
			;
params			: paramList 						{$$=$1;}
			| /*empty*/						{$$=NULL;}
			;
paramList		: paramList ';' paramTypeList				{yyerrok; $$=$1; appendSib($3, $1);}
			| paramTypeList						{$$=$1;}
			| paramList ';' error					{$$=NULL; /*ERROR*/}
			| error							{$$=NULL; /*ERROR*/}
			;
paramTypeList		: typeSpecifier paramIdList				{$$=$2; settype($1, $2);}
			| typeSpecifier error					{$$=NULL; /*ERROR*/}
			;
paramIdList		: paramIdList ',' paramId				{yyerrok; $$=$1; appendSib($3, $1);}
			| paramId						{$$=$1;}
			| paramIdList ',' error					{$$=NULL; /*ERROR*/}
                        | error                                                 {$$=NULL; /*ERROR*/}
			;
paramId			: ID							{$$=newNode($1, NULL, NULL, NULL, 4); $$->isParam=true; $$->isLocal=true;}
			| ID '[' ']'						{$$=newNode($1, NULL, NULL, NULL, 4); $$->isParam=true; $$->isLocal=true; $$->isArray=true;}
			| error ']'						{yyerrok; $$=NULL; /*ERROR*/}
			;


statement		: matched						{$$=$1;}
			| unmatched						{$$=$1;}
			;
compoundStmt		: '{' localDeclarations statementList '}'		{yyerrok; $$=newNode($1, $2, $3, NULL, 0);}
                        | '{' error statementList '}'                           {yyerrok; $$=NULL; /*ERROR*/}
                        | '{' localDeclarations error '}'                       {yyerrok; $$=NULL; /*ERROR*/}
			;
localDeclarations	: localDeclarations scopedVarDeclaration		{if($1==NULL){$$=$2;}else{$$=$1;} setLocal($2); appendSib($2, $1);}
			| /*empty*/						{$$=NULL;}
			;
statementList		: statementList statement				{if($1==NULL){$$=$2;}else{$$=$1; appendSib($2, $1);}}
			| /*empty*/						{$$=NULL;}
			;
expressionStmt		: expression ';'					{yyerrok; $$=$1;}
			| ';'							{yyerrok; $$=NULL;}
			;
matched			: IF '(' simpleExpression ')' matched ELSE matched	{$$=newNode($1, $3, $5, $7, 0);}
			| WHILE '(' simpleExpression ')' matched		{$$=newNode($1, $3, $5, NULL, 0);}
			| expressionStmt					{$$=$1;}
			| compoundStmt						{$$=$1;}
			| returnStmt						{$$=$1;}
			| breakStmt						{$$=$1;}
                        | IF '(' error                                          {$$=NULL; /*ERROR*/}
			| IF error ')' matched ELSE matched			{yyerrok; $$=NULL; /*ERROR*/}
			| WHILE error ')' matched				{yyerrok; $$=NULL; /*ERROR*/}
			| WHILE '(' error ')' matched				{yyerrok; $$=NULL; /*ERROR*/}
			| WHILE error						{$$=NULL; /*ERROR*/}
                        | error                                                 {$$=NULL; /*ERROR*/}
			;
unmatched		: IF '(' simpleExpression ')' matched			{$$=newNode($1, $3, $5, NULL, 0);}
			| IF '(' simpleExpression ')' unmatched			{$$=newNode($1, $3, $5, NULL, 0);}
			| IF '(' simpleExpression ')' matched ELSE unmatched	{$$=newNode($1, $3, $5, $7, 0);}
			| WHILE '(' simpleExpression ')' unmatched		{$$=newNode($1, $3, $5, NULL, 0);}
			| IF error						{$$=NULL; /*ERROR*/}
			| IF error ')' statement				{yyerrok; $$=NULL; /*ERROR*/}
			| IF error ')' matched ELSE unmatched			{yyerrok; $$=NULL; /*ERROR*/}
			| WHILE error ')' unmatched				{yyerrok; $$=NULL; /*ERROR*/}
			| WHILE '(' error ')' unmatched				{yyerrok; $$=NULL; /*ERROR*/}
			;
returnStmt		: RETURN ';'						{yyerrok; $$=newNode($1, NULL, NULL, NULL, 0);}
			| RETURN expression ';'					{yyerrok; $$=newNode($1, $2, NULL, NULL, 0);}
			;
breakStmt		: BREAK ';'						{yyerrok; $$=newNode($1, NULL, NULL, NULL, 0);}
			;


expression		: mutable assignop expression				{$$=newNode($2, $1, $3, NULL, 0);}
			| mutable INC						{yyerrok; $$=newNode($2, $1, NULL, NULL, 0);}
			| mutable DEC						{yyerrok; $$=newNode($2, $1, NULL, NULL, 0);}
			| simpleExpression					{$$=$1;}
			| error assignop error					{$$=NULL; /*ERROR*/}
			| error INC						{yyerrok; $$=NULL; /*ERROR*/}
			| error DEC						{yyerrok; $$=NULL; /*ERROR*/}
			;
assignop		: '='							{$$=$1;}
			| ADDASS						{$$=$1;}
			| SUBASS						{$$=$1;}
			| MULASS						{$$=$1;}
			| DIVASS						{$$=$1;}
			;
simpleExpression	: simpleExpression OR andExpression			{$$=newNode($2, $1, $3, NULL, 0);}
			| andExpression						{$$=$1;}
                        | simpleExpression OR error                             {$$=NULL; /*ERROR*/}
			;
andExpression		: andExpression AND unaryRelExpression			{$$=newNode($2, $1, $3, NULL, 0);}
			| unaryRelExpression					{$$=$1;}
                        | andExpression AND error                               {$$=NULL; /*ERROR*/}
			;
unaryRelExpression	: NOT unaryRelExpression				{$$=newNode($1, $2, NULL, NULL, 0);}
			| relExpression						{$$=$1;}
                        | NOT error                                             {$$=NULL; /*ERROR*/}
			;
relExpression		: sumExpression relop sumExpression			{$$=newNode($2, $1, $3, NULL, 0);}
			| sumExpression						{$$=$1;}
                        | sumExpression relop error                             {$$=NULL; /*ERROR*/}
			| error relop sumExpression				{yyerrok; $$=NULL; /*ERROR*/}
			;
relop			: LESSEQ						{$$=$1;}
			| '<'							{$$=$1;}
			| '>'							{$$=$1;}
			| GRTEQ							{$$=$1;}
			| EQ							{$$=$1;}
			| NOTEQ							{$$=$1;}
			;
sumExpression		: sumExpression sumop term				{$$=newNode($2, $1, $3, NULL, 0);}
			| term							{$$=$1;}
                        | sumExpression sumop error                             {yyerrok; $$=NULL; /*ERROR*/}
			;
sumop			: '+'							{$$=$1;}
			| '-'							{$$=$1;}
			;
term			: term mulop unaryExpression				{$$=newNode($2, $1, $3, NULL, 0);}
			| unaryExpression					{$$=$1;}
                        | term mulop error                                      {$$=NULL; /*ERROR*/}
			;
mulop			: '*'							{$$=$1;}
			| '/'							{$$=$1;}
			| '%'							{$$=$1;}
			;
unaryExpression		: unaryop unaryExpression				{$$=newNode($1, $2, NULL, NULL, 0);}
			| factor						{$$=$1;}
                        | unaryop error                                         {$$=NULL; /*ERROR*/}
			;
unaryop			: '-'							{$$=$1;}
			| '*'							{$$=$1;}
			| '?'							{$$=$1;}
			;
factor			: immutable						{$$=$1;}
			| mutable						{$$=$1;}
			;
mutable			: ID							{$$=newNode($1, NULL, NULL, NULL, 5);}
			| mutable '[' expression ']'				{$$=newNode($2, $1, $3, NULL, 0);}
			| mutable '.' ID					{$$=newNode($2, $1, newNode($3, NULL, NULL, NULL, 5), NULL, 5);}
			;

immutable		: '(' expression ')'					{yyerrok; $$=$2;}
			| call							{$$=$1;}
			| constant						{$$=$1;}
                        | '(' error                                             {$$=NULL; /*ERROR*/}
                        | error ')'                                             {yyerrok; $$=NULL; /*ERROR*/}
			;
call			: ID '(' args ')'					{$$=newNode($1, $3, NULL, NULL, 6);}
			| error '('						{yyerrok; $$=NULL; /*ERROR*/}
			;
args			: argList						{$$=$1;}
			| /*empty*/						{$$=NULL;}
			;
argList			: argList ',' expression				{yyerrok; $$=$1; appendSib($3, $1);}
			| expression						{$$=$1;}
                        | argList ',' error                                     {$$=NULL; /*ERROR*/}
			;
constant		: NUMCONST						{$$=newNode($1, NULL, NULL, NULL, 0);}
			| CHARCONST						{$$=newNode($1, NULL, NULL, NULL, 0);}
			| BOOLCONST						{$$=newNode($1, NULL, NULL, NULL, 0);}
			;

%%
int main(int argc, char *argv[]){
  int pflag, Pflag, errflag, oflag;
  char c, *name;
  pflag=Pflag=errflag=oflag=0;
  while((c=ourGetopt(argc, argv, (char*)"dpPo:"))!=EOF){
    switch(c){
      case 'd':
        yydebug=1;
        break;
      case 'p':
        pflag=1;
        break;
      case 'P':
        Pflag=1;
        break;
      case 'o':
        oflag=1;
        name=optarg;
        break;
      default:
        errflag=1;
        break;
    }
  }
  if(errflag){printf("usage: cmd file [-d] [-p] [-P] [-o filename]\n"); exit(2);}
  if(optind<argc){
    if(argv[optind][strlen(argv[optind])-2]!='-'&&argv[optind][strlen(argv[optind])-1]!='c'&&argv[optind][strlen(argv[optind])-3]!='.'){
      printf("Warning:unrecognized file extension.\n");
    }
    if(freopen(argv[optind], "r", stdin)==NULL){
      printf("%s: file not found\n", argv[1]);
      return 0;
    }
  }
  initErrorProcessing();
  yyparse();
  if(pflag){printTree(stdout, AST, 0);}
  if(numerrors==0){
    semantic(stdout, AST);
    if(declarations.lookupGlobal("main")==NULL){printf("ERROR(LINKER): Procedure main is not defined.\n"); numerrors++;}
    if(Pflag){
      printTree(stdout, AST, 1);
      printf("Offset for end of global space: %d\n", goffset);
    }
    if(numerrors==0){
      if(oflag){
        code=fopen(optarg, (char*)"w");
      }else{
        if(optind<argc){
          argv[optind][strlen(argv[optind])-2]='t';
          argv[optind][strlen(argv[optind])-1]='m';
          code=fopen(argv[optind], (char*)"w");
        }else{
          code=fopen((char*)"a.tm", (char*)"w");
        }
      }
      emitAll(AST);
    }
  }
  printf("Number of warnings: %d\n", numwarnings);
  printf("Number of errors: %d\n", numerrors);
  return 0;
}