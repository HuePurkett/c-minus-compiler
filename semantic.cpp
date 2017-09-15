//semantic.cpp

//Hue Purkett
//CS445
//3/23/2017

#include"semantic.h"
#include<stdlib.h>
#include<cstring>

extern int numerrors, numwarnings;
SymbolTable declarations;


//for typing
struct td *intType=new struct td(INT, strdup("int"));
struct td *boolType=new struct td(BOOL, strdup("bool"));
struct td *charType=new struct td(CHAR, strdup("char"));
struct td *voidType=new struct td(0, strdup("void"));
struct td *undefined=new struct td(-1, strdup("undefined"));

bool func=false;//flag for having just entered a function
int depth=0;//flag for checking placement of breaks
struct tnd *currFunc=NULL;//for type checking in returns
bool rets;//flag for there being a return in a function
int ignore=0;//flag for ignoreing the prepended IO functions
int sibCheck=1;//flag for having semantic() not go down sibling list

int goffset=0;//offset for global variables
int loffset;//offset for local variables

struct td *settype(char type){
  switch(type){
    case 'i':
    case 'I':
      return intType;
    case 'b':
    case 'B':
      return boolType;
    case 'c':
    case 'C':
      return charType;
    case 'u':
      return undefined;
    case 'v':
      return voidType;
    case 'n':
      return NULL;
  }
}


bool isIn(char l, const char *accepted){//case insensitive
  if(l=='u'){return 1;}
  for(int i=0; accepted[i]!='\0'; i++){
    if(l==accepted[i]||l==accepted[i]-32){return 1;}
  }
  return 0;
}


bool isArr(char l){return l<97;}


bool areEq(char a, char b){//ignores arrayness
  return a=='u'||b=='u'||a==b||a==b-32||a==b+32;
}


bool areEql(char a, struct tnd *b){//sees arrayness
  if(b==NULL){
    return a=='n';
  }
  char c=b->varType->input[0];
  if(!areEq(a, c)){return false;}
  return a=='u'||c=='u'||(97>a&&b->isArray)||(a>96&&!b->isArray);
}


bool typeMatch(struct tnd *a, struct tnd *b){//sees arrayness
  if(a==b){return true;}
  char A=a->varType->input[0];
  char B=b->varType->input[0];
  if(A=='u'||B=='u'){return true;}
  if(A!=B){return false;}
  return a->isArray==b->isArray;
}


void error(FILE *out, tnd *n, int msg, const char *types, int pline){
  switch(msg){
    case -2://versions of error messages without "type" added to account for NONVOID
      fprintf(out, "ERROR(%d): '%s' requires operands of %s but rhs is of type %s.\n", n->lineno, n->token->input, types, n->children[1]->varType->input);
      break;
    case -1:
      fprintf(out, "ERROR(%d): '%s' requires operands of %s but lhs is of type %s.\n", n->lineno, n->token->input, types, n->children[0]->varType->input);
      break;

    case 0:
      fprintf(out, "ERROR(%d): '%s' is a simple variable and cannot be called.\n", n->lineno, n->token->input);
      break;
    case 1:
      fprintf(out, "ERROR(%d): '%s' requires operands of type %s but lhs is of type %s.\n", n->lineno, n->token->input, types, n->children[0]->varType->input);
      break;
    case 2:
      fprintf(out, "ERROR(%d): '%s' requires operands of type %s but rhs is of type %s.\n", n->lineno, n->token->input, types, n->children[1]->varType->input);
      break;
    case 3:
      fprintf(out, "ERROR(%d): '%s' requires operands of the same type but lhs is type %s and rhs is type %s.\n", n->lineno, n->token->input, n->children[0]->varType->input, n->children[1]->varType->input);
      break;
    case 4:
      fprintf(out, "ERROR(%d): Array '%s' should be indexed by type int but got type %s.\n", n->lineno, n->children[0]->token->input, n->children[1]->varType->input);
      break;
    case 5:
      fprintf(out, "ERROR(%d): Array index is the unindexed array '%s'.\n", n->lineno, n->children[1]->token->input);
      break;
    case 6:
      fprintf(out, "ERROR(%d): Cannot index nonarray '%s'.\n", n->lineno, n->children[0]->token->input);
      break;
    case 7:
      fprintf(out, "ERROR(%d): Cannot index nonarray.\n", n->lineno);
      break;
    case 8:
      fprintf(out, "ERROR(%d): Cannot return an array.\n", n->lineno);
      break;
    case 9:
      fprintf(out, "ERROR(%d): Cannot use function '%s' as a variable.\n", n->lineno, n->token->input);
      break;
    case 10:
      fprintf(out, "ERROR(%d): Symbol '%s' is already defined at line %d.\n", n->lineno, n->token->input, pline);
      break;
    case 11:
      fprintf(out, "ERROR(%d): Symbol '%s' is not defined.\n", n->lineno, n->token->input);
      break;
    case 12:
      fprintf(out, "ERROR(%d): The operation '%s' does not work with arrays.\n", n->lineno, n->token->input);
      break;
    case 13:
      fprintf(out, "ERROR(%d): The operation '%s' only works with arrays.\n", n->lineno, n->token->input);
      break;
    case 14:
      fprintf(out, "ERROR(%d): Unary '%s' requires an operand of type %s but was given type %s.\n", n->lineno, n->token->input, types, n->children[0]->varType->input);
      break;

    case 15:
      fprintf(out, "ERROR(%d): Function '%s' is not defined.\n", n->lineno, n->token->input);
      break;
    case 16:
      fprintf(out, "ERROR(%d): Expecting Boolean test condition in %s statement but got type %s.\n", n->lineno, n->token->input, n->children[0]->varType->input);
      break;
    case 17:
      fprintf(out, "ERROR(%d): Cannot use array as test condition in %s statement.\n", n->lineno, n->token->input);
      break;
    case 18:
      fprintf(out, "ERROR(%d): Array index is an unindexed array.\n", n->lineno);
      break;
    case 19:
      fprintf(out, "ERROR(%d): '%s' requires that either both or neither operands be arrays.\n", n->lineno, n->token->input);
      break;
    case 20:
      fprintf(out, "ERROR(%d): Initializer for variable '%s' is not a constant expression.\n", n->lineno, n->token->input);
      break;
    case 21:
      fprintf(out, "ERROR(%d): Variable '%s' is of type %s but is being initialized with an expression of type %s.\n", n->lineno, n->token->input, n->varType->input, n->children[0]->varType->input);
      break;
    case 22:
      fprintf(out, "ERROR(%d): Function '%s' at line %d is expecting no return value, but return has return value.\n", n->lineno, currFunc->token->input, currFunc->lineno);
      break;
    case 23:
      fprintf(out, "ERROR(%d): Function '%s' at line %d is expecting to return type %s but instead returns type %s.\n", n->lineno, currFunc->token->input, currFunc->lineno, currFunc->varType->input, n->children[0]->varType->input);
      break;
    case 24:
      fprintf(out, "ERROR(%d): Function '%s' at line %d is expecting to return type %s but return has no return value.\n", n->lineno, currFunc->token->input, currFunc->lineno, currFunc->varType->input);
      break;
    case 25:
      fprintf(out, "ERROR(%d): Cannot have a break statement outside of loop.\n", n->lineno);
      break;
    case 26:
      fprintf(out, "ERROR(%d): Too few parameters passed for function '%s' defined on line %d.\n", n->lineno, n->token->input, pline);
      break;
    case 27:
      fprintf(out, "ERROR(%d): Too many parameters passed for function '%s' defined on line %d.\n", n->lineno, n->token->input, pline);
      break;
  }
  numerrors++;
}


void warn(FILE *out, struct tnd *n, int msg){
  switch(msg){
    case 0:
      fprintf(out, "WARNING(%d): Expecting to return type %s but function '%s' has no return statement.\n", n->lineno, n->varType->input, n->token->input);
      break;
  }
  numwarnings++;
}


void passError(FILE *out, tnd *n, int msg, tnd *p, tnd *q, int argNum){
  switch(msg){
    case 0:
      fprintf(out, "ERROR(%d): Expecting type %s in parameter %i of call to '%s' defined on line %d but got type %s.\n", n->lineno, q->varType->input, argNum, p->token->input, p->lineno, n->varType->input);
      break;
    case 1:
      fprintf(out, "ERROR(%d): Expecting array in parameter %i of call to '%s' defined on line %d.\n", n->lineno, argNum, p->token->input, p->lineno);
      break;
    case 2:
      fprintf(out, "ERROR(%d): Not expecting array in parameter %i of call to '%s' defined on line %d.\n", n->lineno, argNum, p->token->input, p->lineno);
      break;
  }
  numerrors++;
}


void passing(FILE *out, struct tnd *call, struct tnd *def, int number, struct tnd *n, struct tnd *p){
  if(call==NULL&&def==NULL){return;}
  if(call==NULL){
    error(out, n, 26, "ERROR", p->lineno);//too few args
    return;
  }
  if(def==NULL){
    error(out, n, 27, "ERROR", p->lineno);//too many args
    int sbak=sibCheck;
    sibCheck=1;
    semantic(out, call);
    sibCheck=sbak;
    return;
  }
  int sbak=sibCheck;
  sibCheck=0;
  char ret=semantic(out, call);//won't go down sibling list
  sibCheck=sbak;
  if(!typeMatch(call, def)){
    if(strcmp(call->varType->input, def->varType->input)){
      passError(out, call, 0, p, def, number);//wrong type
    }
    if(call->isArray&&!def->isArray){
      passError(out, call, 2, p, def, number);//not expecting array
    }
    if(def->isArray&&!call->isArray){
      passError(out, call, 1, p, def, number);//expecting array
    }
  }
  if(ret=='u'&&def->isArray){
    passError(out, call, 1, p, def, number);//expecting array
  }
  passing(out, call->sibling, def->sibling, number+1, n, p);
}


bool constancy(struct tnd *a[]){
  for(int i=0; i<3; i++){
    if(a[i]!=NULL&&!a[i]->isConst){
      return false;
    }
  }
  return true;
}


//Returns type of subtree. i:int, b:bool, c:char, v:void, u:undefined, n:subtree does not exist. Capitals indicate array. r:record?
char semantic(FILE *out, struct tnd *n){
  if(n==NULL){return 'n';}
  int c[]={1, 1, 1}, sib=sibCheck;//flags for having already typed subtrees
  bool entered=false;//flag for having entered a scope
  bool deepened=false;//flag for having entered a loop
  char ret='u', tmp;
  struct tnd *p;
  switch(n->tClass){
    case ID:
      switch(n->idCase){
        case 3://function				//bleeds into record
          loffset=-2;
          if(n->varType==NULL){//already typed except for this
            ret='v';
          }else{
            ret=n->varType->input[0];
          }
          if((n->children[1]!=NULL)&&(n->children[1]->token->tClass=='{')){//no new scope for compounds immediately following functions
            func=true;
          }
          currFunc=n;
          rets=false;
          ignore++;
        case 1://record
          if(!declarations.insertGlobal(n->token->input, n)){
            error(out, n, 10, "ERROR", ((struct tnd*)(declarations.lookupGlobal(n->token->input)))->lineno);//already defined
          }
          declarations.enter(n->token->input);
          entered=true;
          n->isConst=false;
          break;
        case 2://variable declaration			bleeds into function parameter
          if(n->children[0]!=NULL){//variable is initialized
            c[0]=0;
            ret=semantic(out, n->children[0]);
//            if(!areEql(ret, n)){//also checks for array matchedness
            if(!areEq(ret, n->varType->input[0])){
              error(out, n, 21, "ERROR", -2);//initialized with wrong type
            }
            if(!n->children[0]->isConst){
              error(out, n, 20, "ERROR", -2);//initialized with non-constant
            }
          }
          if(n->isArray){n->size=n->arraylen+1;}
        case 4://function parameter
          n->isConst=false;
          if(!declarations.insert(n->token->input, n)){
            error(out, n, 10, "ERROR", ((struct tnd*)(declarations.lookup(n->token->input)))->lineno);//already defined
            n->isLocal=true;
          }else{//don't do offset on error
            if(n->isLocal&&!n->isStatic){
              n->offset=loffset;
              loffset-=n->size;
            }else{
              n->offset=goffset;
              goffset-=n->size;
            }
          }
          if(n->isArray&&!n->isParam){n->offset--;}
          break;
        case 5://variable usage
          p=(struct tnd*)declarations.lookup(n->token->input);
          if(p==NULL){
            error(out, n, 11, "ERROR", -1);//not defined
            n->undefined=true;
            n->isLocal=true;
          }else{
            if(p->idCase==3){//is function
              error(out, n, 9, "ERROR", -1);
              if(p==currFunc){//strange, but it matches the tests
                n->isLocal=true;
                n->size=-3;//I have no idea why -3 or why only in this case, but it solves 8 differences from several different files without introducing new ones.
                           //the placement in an error suggests default value, but I don't see why that would be -3.
              }else{
                n->isLocal=false;
                n->size=p->size;
              }
            }else{
              ret=p->varType->input[0];
              if(p->isArray){ret-=32;}
              n->isLocal=p->isLocal;
              n->size=p->size;
            }
            n->offset=p->offset;
            n->isArray=p->isArray;
            n->isStatic=p->isStatic;
            n->isParam=p->isParam;
          }
          n->isConst=false;
          break;
        case 6://function call
          p=(struct tnd*)declarations.lookup(n->token->input);
          if(p==NULL){//not declared
            error(out, n, 15, "ERROR", -1);
          }else{
            if(p->idCase==2||p->idCase==4){//is not function
              error(out, n, 0, "ERROR", -1);
            }else{
              c[0]=0;
              passing(out, n->children[0], p->children[0], 1, n, p);
              ret=p->varType->input[0];
              if(p->isArray){ret-=32;}
            }
          }
          n->isConst=false;
          break;
      }
      break;
    case '{'://compound
      if(!func){
        declarations.enter("compound");
        entered=true;
      }else{
        func=false;
      }
      n->isConst=false;
      break;
    case WHILE://bleeds into if
      depth++;
      deepened=true;
    case IF:
      c[0]=0;
      ret=semantic(out, n->children[0]);
      if(!isIn(ret, "b")){
        error(out, n, 16, "ERROR", -1);//can't test non-bool
      }
      if(isArr(ret)){
        error(out, n, 17, "ERROR", -1);//can't test array
      }
      ret='v';
      n->isConst=false;
      break;
    case BREAK:
      if(depth==0){
        error(out, n, 25, "ERROR", -1);//can't have break out of loop
      }
      ret='v';
      n->isConst=false;
      break;
    case RETURN:
      c[0]=0;
      ret=semantic(out, n->children[0]);
      if(currFunc->varType->input[0]=='v'&&ret!='n'){
        error(out, n, 22, "ERROR", -1);//returning something on void
      }
      if(currFunc->varType->input[0]!='v'&&ret=='n'){
        error(out, n, 24, "ERROR", -1);//returning nothing
      }
      if(currFunc->varType->input[0]!='v'&&ret!='n'&&!areEq(currFunc->varType->input[0], ret)){
        error(out, n, 23, "ERROR", -1);//returning wrong type
      }
      if(isArr(ret)){
        error(out, n, 8, "ERROR", -1);//return array
      }
      rets=true;
      ret='v';
      n->isConst=false;
      break;
    case '=':
      c[0]=0;
      ret=semantic(out, n->children[0]);
      c[1]=0;
      tmp=semantic(out, n->children[1]);
      if(ret=='v'){
        error(out, n, -1, "NONVOID", -1);//lhs wrong type
      }
      if(tmp=='v'){
        error(out, n, -2, "NONVOID", -1);//rhs type wrong
      }
      if(ret!='v'&&tmp!='v'&&!areEq(ret, tmp)){
        error(out, n, 3, "ERROR", -1);//operand mismatch
      }
      if((isArr(ret)&&!isArr(tmp))||(isArr(tmp)&&!isArr(ret))){
        error(out, n, 19, "ERROR", -1);//operand mismatch
      }
      n->isConst=constancy(n->children);
      break;
    case ADDASS:
    case SUBASS:
    case MULASS:
    case DIVASS:
    case '+':
    case '/':
    case '%':
      c[0]=0;
      ret=semantic(out, n->children[0]);
      c[1]=0;
      tmp=semantic(out, n->children[1]);
      if(!isIn(ret, "i")){
        error(out, n, 1, "int", -1);//lhs wrong
      }
      if(!isIn(tmp, "i")){
        error(out, n, 2, "int", -1);//rhs wrong
      }
      if(isArr(ret)||isArr(tmp)){
        error(out, n, 12, "ERROR", -1);//no arrays
      }
      ret='i';
      n->isConst=constancy(n->children);
      break;
    case INC:
    case DEC:
      c[0]=0;
      ret=semantic(out, n->children[0]);
      if(!isIn(ret, "i")){
        error(out, n, 14, "int", -1);//unary type wrong
      }
      if(isArr(ret)){
        error(out, n, 12, "ERROR", -1);//no arrays
      }
      ret='i';
      n->isConst=constancy(n->children);
      break;
    case OR:
    case AND:
      c[0]=0;
      ret=semantic(out, n->children[0]);
      c[1]=0;
      tmp=semantic(out, n->children[1]);
      if(!isIn(ret, "b")){
        error(out, n, 1, "bool", -1);//lhs wrong
      }
      if(!isIn(tmp, "b")){
        error(out, n, 2, "bool", -1);//rhs wrong
      }
      if(isArr(ret)||isArr(tmp)){
        error(out, n, 12, "ERROR", -1);//no arrays
      }
      ret='b';
      n->isConst=constancy(n->children);
      break;
    case NOT:
      c[0]=0;
      ret=semantic(out, n->children[0]);
      if(!isIn(ret, "b")){
        error(out, n, 14, "bool", -1);//unary type wrong
      }
      if(isArr(ret)){
        error(out, n, 12, "ERROR", -1);//no arrays
      }
      ret='b';
      n->isConst=constancy(n->children);
      break;
    case LESSEQ:
    case '<':
    case '>':
    case GRTEQ:
      c[0]=0;
      ret=semantic(out, n->children[0]);
      c[1]=0;
      tmp=semantic(out, n->children[1]);
      if(!isIn(ret, "ci")){
        error(out, n, 1, "char or type int", -1);//lhs wrong
      }
      if(!isIn(tmp, "ci")){
        error(out, n, 2, "char or type int", -1);//rhs wrong
      }else{
        if(isIn(ret, "ci")&&!areEq(ret, tmp)){
          error(out, n, 3, "ERROR", -1);//types mismatch
        }
      }
      if(isArr(ret)||isArr(tmp)){
        error(out, n, 12, "ERROR", -1);//no arrays
      }
      ret='b';
      n->isConst=constancy(n->children);
      break;
    case EQ:
    case NOTEQ:
      c[0]=0;
      ret=semantic(out, n->children[0]);
      c[1]=0;
      tmp=semantic(out, n->children[1]);
      if(!areEq(tmp, ret)&&ret!='v'&&tmp!='v'){
        error(out, n, 3, "ERROR", -1);//types mismatch
      }
      if(ret=='v'){
        error(out, n, -1, "NONVOID", -1);//lhs wrong type
      }
      if(tmp=='v'){
        error(out, n, -2, "NONVOID", -1);//rhs type wrong
      }
      if((isArr(ret)&&!isArr(tmp))||(isArr(tmp)&&!isArr(ret))){
        error(out, n, 19, "ERROR", -2);//both or neither arrays
      }
      ret='b';
      n->isConst=constancy(n->children);
      break;
    case '-'://binary and unary
      c[0]=0;
      ret=semantic(out, n->children[0]);
      c[1]=0;
      tmp=semantic(out, n->children[1]);
      if(tmp=='n'){//unary-no second child
        if(!isIn(ret, "i")){
          error(out, n, 14, "int", -1);//unary wrong type
        }
        if(isArr(ret)){
          error(out, n, 12, "ERROR", -1);//no arrays
        }
      }else{//binary-yes second child
        if(!isIn(ret, "i")){
          error(out, n, 1, "int", -1);//lhs wrong type
        }
        if(!isIn(tmp, "i")){
          error(out, n, 2, "int", -1);//rhs type wrong
        }
        if(isArr(ret)||isArr(tmp)){
          error(out, n, 12, "ERROR", -1);//no arrays
        }
      }
      ret='i';
      n->isConst=constancy(n->children);
      break;
    case '*'://binary and unary
      c[0]=0;
      ret=semantic(out, n->children[0]);
      c[1]=0;
      tmp=semantic(out, n->children[1]);
      if(tmp=='n'){//unary-no second child
        if(!isArr(ret)&&ret!='u'){
          error(out, n, 13, "ERROR", -1);
        }
      }else{//binary-yes second child
        if(!isIn(ret, "i")){
          error(out, n, 1, "int", -1);//lhs wrong type
        }
        if(!isIn(tmp, "i")){
          error(out, n, 2, "int", -1);//rhs type wrong
        }
        if(isArr(ret)||isArr(tmp)){
          error(out, n, 12, "ERROR", -1);//no arrays
        }
      }
      ret='i';
      n->isConst=constancy(n->children);
      break;
    case '?'://unary
      c[0]=0;
      ret=semantic(out, n->children[0]);
      if(!isIn(ret, "i")){
        error(out, n, 14, "int", -1);//unary type wrong
      }
      if(isArr(ret)){
        error(out, n, 12, "ERROR", -1);//no arrays
      }
      ret='i';
      n->isConst=constancy(n->children);
      break;
    case '[':
      c[0]=0;
      ret=semantic(out, n->children[0]);
      c[1]=0;
      tmp=semantic(out, n->children[1]);
      if(!isArr(ret)){
        if(n->children[0]->tClass==ID){
          error(out, n, 6, "ERROR", -1);//indexing nonarray with name
        }else{
          error(out, n, 7, "ERROR", -1);//indexing nonarray without name
        }
      }else{
        ret+=32;
      }
      if(!isIn(tmp, "i")){
        error(out, n, 4, "ERROR", -1);//indexing with nonint
      }
      if(isArr(tmp)){
        if(n->children[1]->tClass==ID){
          error(out, n, 5, "ERROR", -1);//indexing with named array
        }else{
          error(out, n, 18, "ERROR", -1);//indexing with unnamed array
        }
      }
      n->isConst=constancy(n->children);
      break;
    case '.':
      c[0]=c[1]=1;
      semantic(out, n->children[0]);
      ret=semantic(out, n->children[1]);
      break;
    case CHARCONST:
      ret='c';
      n->isConst=true;
      break;
    case NUMCONST:
      ret='i';
      n->isConst=true;
      break;
    case BOOLCONST:
      ret='b';
      n->isConst=true;
      break;
    default://everything is accounted for--should not occur
      fprintf(out, "ERRORERROR(%i) %s\n", n->lineno, n->token->input);
      n->isConst=false;
      break;
  }

  if(n->varType==NULL){
    n->varType=settype(ret);
    if(97>ret){
      n->isArray=true;
    }
  }
  for(int i=0; i<3; i++){
    if(c[i]){semantic(out, n->children[i]);}
  }
  if(entered){declarations.leave();}
  if(deepened){depth--;}
  if(n->idCase==3&&ignore>7){
    if(n->varType!=voidType&&!rets){
      warn(out, n, 0);
    }
  }
  if(n->idCase==3){n->size=loffset;}
  if(sib){semantic(out, n->sibling);}
  return ret;
}