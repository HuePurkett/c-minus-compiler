//codegen.cpp

//Hue Purkett
//CS445
//4/17/2017

#include"emitcode.h"
#include"scanType.h"
#include"parser.tab.h"
#include"symbolTable.h"
#include<cstring>
#include<string>
#include<iostream>

using namespace std;

extern SymbolTable declarations;
extern int goffset;
int breakpoint;

void gencode(struct tnd *n, int toff, bool folSib);


void emitBinary(struct tnd *n, int toff){
  gencode(n->children[0], toff, true);
  emitRM((char*)"ST", 3, toff, 1, (char*)"store result of left");
  gencode(n->children[1], toff-1, true);
  emitRM((char*)"LD", 4, toff, 1, (char*)"load result of left");
}


void emitModAss(struct tnd *n, int toff, char *op, char *comment){
  if(n->children[0]->tClass=='['){//array
    emitBinary(n->children[0], toff);
    emitRO((char*)"SUB", 5, 4, 3, (char*)"compute address");
    emitRM((char*)"ST", 5, toff, 1, (char*)"store address");
    gencode(n->children[1], toff-1, true);
    emitRM((char*)"LD", 5, toff, 1, (char*)"load address");
    emitRM((char*)"LD", 4, 0, 5, (char*)"get old value");
    emitRO(op, 3, 4, 3, comment);
    emitRM((char*)"ST", 3, 0, 5, (char*)"set new value of (arr)", n->children[0]->children[0]->token->input);
  }else{//non-array
    emitRM((char*)"LD", 3, n->children[0]->offset, n->children[0]->isLocal, (char*)"retrieve old value of", n->children[0]->token->input);
    emitRM((char*)"ST", 3, toff, 1, (char*)"store old value of", n->children[0]->token->input);
    gencode(n->children[1], toff-1, true);
    emitRM((char*)"LD", 4, toff, 1, (char*)"get old value of", n->children[0]->token->input);
    emitRO(op, 3, 4, 3, comment);
    emitRM((char*)"ST", 3, n->children[0]->offset, n->children[0]->isLocal, (char*)"set new value of", n->children[0]->token->input);
  }
}


void emitReturn(){//emits return block. assumes return value is already in R3
  emitRM((char*)"LD", 2, -1, 1, (char*)"pull return address");
  emitRM((char*)"LD", 1, 0, 1, (char*)"reset frame ptr");
  emitRM((char*)"LDA", 7, 0, 2, (char*)"return PC");
}


int builtIn(struct tnd *n, int toff, bool emit){//handles the io functions
  if(!strcmp(n->token->input, "input")){
    if(emit){
      emitRO((char*)"IN", 3, 3, 3, (char*)"int input()");
    }
    return 1;
  }
  if(!strcmp(n->token->input, "output")){
    if(emit){
      gencode(n->children[0], toff, true);
      emitRO((char*)"OUT", 3, 3, 3, (char*)"output(int)");
    }
    return 1;
  }
  if(!strcmp(n->token->input, "inputb")){
    if(emit){
      emitRO((char*)"INB", 3, 3, 3, (char*)"bool input()");
    }
    return 1;
  }
  if(!strcmp(n->token->input, "outputb")){
    if(emit){
      gencode(n->children[0], toff, true);
      emitRO((char*)"OUTB", 3, 3, 3, (char*)"output(bool)");
    }
    return 1;
  }
  if(!strcmp(n->token->input, "inputc")){
    if(emit){
      emitRO((char*)"INC", 3, 3, 3, (char*)"char input()");
    }
    return 1;
  }
  if(!strcmp(n->token->input, "outputc")){
    if(emit){
      gencode(n->children[0], toff, true);
      emitRO((char*)"OUTC", 3, 3, 3, (char*)"output(char)");
    }
    return 1;
  }
  if(!strcmp(n->token->input, "outnl")){
    if(emit){
      emitRO((char*)"OUTNL", 3, 3, 3, (char*)"outnl()");
    }
    return 1;
  }
  return 0;
}


void parameterize(struct tnd *n, int offset){
  if(n==NULL){return;}
  gencode(n, offset, false);
  emitRM((char*)"ST", 3, offset, 1, (char*)"store parameter");
  parameterize(n->sibling, offset-1);
}


void gencode(struct tnd *n, int toff, bool folSib){
  if(n==NULL){return;}
  int bak, bak2, bak3, breakBack;
  switch(n->tClass){
    case ID:
      switch(n->idCase){
        case 1://record
          /*not required*/
          break;
        case 2://variable declaration
          if(n->isLocal){//ignore globals. They should be done later.
            if(n->children[0]){//initialized variable
              gencode(n->children[0], toff, true);
              emitRM((char*)"ST", 3, n->offset, 1, (char*)"initialize", n->token->input);
            }
            if(n->isArray){//array size setting
              emitRM((char*)"LDC", 3, n->size-1, 0, (char*)"load size of", n->token->input);
              emitRM((char*)"ST", 3, n->offset+1, 1, (char*)"store size of", n->token->input);
            }
          }
          break;
        case 3://function declaration
          if(!builtIn(n, toff, false)){
            emitComment((char*)"FUNCTION", n->token->input);
            n->offset=emitSkip(0);
            emitRM((char*)"ST", 2, -1, 1, (char*)"store return address");
            toff+=n->size;
            gencode(n->children[1], n->size, true);//parameter list should not generate code, so do not traverse
            emitRM((char*)"LDC", 3, 0, 0, (char*)"load 0 for default return");
            emitReturn();
            emitComment((char*)"END FUNCTION", n->token->input);
          }
          break;
        case 4://parameter declaration
          /*do nothing*/
          break;
        case 5://variable usage
          if(n->isLocal){
            if(n->isArray&&!n->isParam){
              emitRM((char*)"LDC", 3, n->offset, 0, (char*)"load offset of", n->token->input);
              emitRO((char*)"ADD", 3, 3, 1, (char*)"compute absolute address of", n->token->input);
            }else{
              emitRM((char*)"LD", 3, n->offset, 1, (char*)"retrieve local variable", n->token->input);
            }
          }else{
            if(n->isArray){
              emitRM((char*)"LDC", 3, n->offset, 0, (char*)"load offset of", n->token->input);
              emitRO((char*)"ADD", 3, 3, 0, (char*)"compute absolute address of", n->token->input);
            }else{
              emitRM((char*)"LD", 3, n->offset, 0, (char*)"retrieve global variable", n->token->input);
            }
          }
          break;
        case 6://function call
          if(!builtIn(n, toff, true)){
            emitComment((char*)"Calling function", n->token->input);
            parameterize(n->children[0], toff-2);
            emitRM((char*)"ST", 1, toff, 1, (char*)"store old frame pointer");
            emitRM((char*)"LDC", 2, 3+emitSkip(0), 0, (char*)"compute return address");
            emitRM((char*)"LDA", 1, toff, 1, (char*)"change frame pointer");
            emitRM((char*)"LDC", 7, ((struct tnd*)(declarations.lookup(n->token->input)))->offset, 0, (char*)"goto", n->token->input);
            emitComment((char*)"end call to", n->token->input);
          }
          break;
        default://problem
          printf("We have a problem. Recieved n->idCase of %i.\n", n->idCase);
          break;
      }
      break;
    case '{'://compound
      emitComment((char*)"COMPUND DECLARATIONS");
      gencode(n->children[0], toff, true);
      emitComment((char*)"COMPUND BODY");
      gencode(n->children[1], toff, true);
      emitComment((char*)"END COMPUND");
      break;
    case IF:
      emitComment((char*)"IF");
      gencode(n->children[0], toff, true);
      bak=emitSkip(1);
      emitComment((char*)"THEN");
      gencode(n->children[1], toff, true);
      if(n->children[2]){bak2=emitSkip(1);}
      backPatchAJumpToHere((char*)"JZR", 3, bak, (char*)"jump on conditional failiure [backpatched]");
      if(n->children[2]){
        emitComment((char*)"ELSE");
        gencode(n->children[2], toff, true);
        backPatchAJumpToHere(bak2, (char*)"skip else [backpatched]");
      }
      emitComment((char*)"ENDIF");
      break;
    case WHILE:
      breakBack=breakpoint;
      emitComment((char*)"WHILE");
      bak3=emitSkip(0);
      gencode(n->children[0], toff, true);
      bak=emitSkip(2);
      breakpoint=bak+1;
      backPatchAJumpToHere((char*)"JNZ", 3, bak, (char*)"skip jump to end of while on success [backpatched]");
      emitComment((char*)"DO");
      gencode(n->children[1], toff, true);
      emitRM((char*)"LDC", 7, bak3, 0, (char*)"go to beginning of while");
      backPatchAJumpToHere(bak+1, (char*)"jump to end of while [backpatched]");
      emitComment((char*)"END WHILE");
      breakpoint=breakBack;
      break;
    case RETURN:
      emitComment((char*)"RETURN");
      if(n->children[0]!=NULL){
        gencode(n->children[0], toff, true);
      }
      emitReturn();
      emitComment((char*)"END RETURN");
      folSib=false;
      break;
    case BREAK:
      emitRM((char*)"LDC", 7, breakpoint, 0, (char*)"break");
      break;
    case CHARCONST:
      emitRM((char*)"LDC", 3, n->token->cval, 0, (char*)"load constant");
      break;
    case NUMCONST:
    case BOOLCONST:
      emitRM((char*)"LDC", 3, n->token->ival, 0, (char*)"load constant");
      break;
    case MULASS:
      emitModAss(n, toff, (char*)"MUL", (char*)"multiply for MULASS");
      break;
    case ADDASS:
      emitModAss(n, toff, (char*)"ADD", (char*)"add for ADDASS");
      break;
    case SUBASS:
      emitModAss(n, toff, (char*)"SUB", (char*)"subtract for SUBASS");
      break;
    case DIVASS:
      emitModAss(n, toff, (char*)"DIV", (char*)"divide for DIVASS");
      break;
    case '=':
      if(n->children[0]->tClass=='['){
        emitBinary(n->children[0], toff);
        emitRO((char*)"SUB", 3, 4, 3, (char*)"compute address");
        emitRM((char*)"ST", 3, toff, 1, (char*)"store address (arr)", n->children[0]->children[0]->token->input);
        gencode(n->children[1], toff-1, true);
        emitRM((char*)"LD", 4, toff, 1, (char*)"load address (arr)", n->children[0]->children[0]->token->input);
        emitRM((char*)"ST", 3, 0, 4, (char*)"set new value of (arr)", n->children[0]->children[0]->token->input);
      }else{
        gencode(n->children[1], toff, true);
        emitRM((char*)"ST", 3, n->children[0]->offset, n->children[0]->isLocal, (char*)"store new value for", n->children[0]->token->input);
      }
      break;
    case INC:
      if(n->children[0]->tClass=='['){
        emitBinary(n->children[0], toff-1);
        emitRO((char*)"SUB", 4, 4, 3, (char*)"compute address");
        emitRM((char*)"LD", 3, 0, 4, (char*)"load old value of (arr)", n->children[0]->children[0]->token->input);
        emitRO((char*)"LDA", 3, 1, 3, (char*)"increment");
        emitRM((char*)"ST", 3, 0, 4, (char*)"set new value of (arr)", n->children[0]->children[0]->token->input);
      }else{
        emitRM((char*)"LD", 3, n->children[0]->offset, n->children[0]->isLocal, (char*)"retrieve old value of", n->children[0]->token->input);
        emitRO((char*)"LDA", 3, 1, 3, (char*)"increment");
        emitRM((char*)"ST", 3, n->children[0]->offset, n->children[0]->isLocal, (char*)"store new value for", n->children[0]->token->input);
      }
      break;
    case DEC:
      if(n->children[0]->tClass=='['){
        emitBinary(n->children[0], toff-1);
        emitRO((char*)"SUB", 4, 4, 3, (char*)"compute address");
        emitRM((char*)"LD", 3, 0, 4, (char*)"load old value of (arr)", n->children[0]->children[0]->token->input);
        emitRO((char*)"LDA", 3, -1, 3, (char*)"decrement");
        emitRM((char*)"ST", 3, 0, 4, (char*)"set new value of (arr)", n->children[0]->children[0]->token->input);
      }else{
        emitRM((char*)"LD", 3, n->children[0]->offset, n->children[0]->isLocal, (char*)"retrieve old value of", n->children[0]->token->input);
        emitRO((char*)"LDA", 3, -1, 3, (char*)"decrement");
        emitRM((char*)"ST", 3, n->children[0]->offset, n->children[0]->isLocal, (char*)"store new value for", n->children[0]->token->input);
      }
      break;
    case OR:
      emitBinary(n, toff);
      emitRO((char*)"OR", 3, 4, 3, (char*)"OR");
      break;
    case AND:
      emitBinary(n, toff);
      emitRO((char*)"AND", 3, 4, 3, (char*)"AND");
      break;
    case LESSEQ:
      emitBinary(n, toff);
      emitRO((char*)"TLE", 3, 4, 3, (char*)"<=");
      break;
    case GRTEQ:
      emitBinary(n, toff);
      emitRO((char*)"TGE", 3, 4, 3, (char*)">=");
      break;
    case NOTEQ:
      emitBinary(n, toff);
      emitRO((char*)"TNE", 3, 4, 3, (char*)"!=");
      break;
    case EQ:
      emitBinary(n, toff);
      emitRO((char*)"TEQ", 3, 4, 3, (char*)"==");
      break;
    case '<':
      emitBinary(n, toff);
      emitRO((char*)"TLT", 3, 4, 3, (char*)"<");
      break;
    case '>':
      emitBinary(n, toff);
      emitRO((char*)"TGT", 3, 4, 3, (char*)">");
      break;
    case '+':
      emitBinary(n, toff);
      emitRO((char*)"ADD", 3, 3, 4, (char*)"+");
      break;
    case '/':
      emitBinary(n, toff);
      emitRO((char*)"DIV", 3, 4, 3, (char*)"/");
      break;
    case '%':
      emitBinary(n, toff);
      emitRO((char*)"DIV", 5, 4, 3, (char*)"%:integer divide");
      emitRO((char*)"MUL", 5, 5, 3, (char*)"%:*");
      emitRO((char*)"SUB", 3, 4, 5, (char*)"%:-");
      break;
    case '-':
      if(n->children[1]){//binary
        emitBinary(n, toff);
        emitRO((char*)"SUB", 3, 4, 3, (char*)"-");
      }else{//unary
        gencode(n->children[0], toff, true);
        emitRM((char*)"LDC", 4, -1, 0, (char*)"load -1");
        emitRO((char*)"MUL", 3, 3, 4, (char*)"* by -1");
      }
      break;
    case '*'://unary and binary
      if(n->children[1]){//binary
        emitBinary(n, toff);
        emitRO((char*)"MUL", 3, 4, 3, (char*)"*");
      }else{//unary
        gencode(n->children[0], toff, true);
        emitRM((char*)"LD", 3, 1, 3, (char*)"get length of array");
      }
      break;
    case NOT:
      gencode(n->children[0], toff, true);
      emitRM((char*)"LDC", 4, 0, 0, (char*)"load 0");
      emitRO((char*)"TEQ", 3, 3, 4, (char*)"test equal to zero");
      break;
    case '?':
      gencode(n->children[0], toff, true);
      emitRO((char*)"RND", 3, 3, 0, (char*)"get random number");
      break;
    case '[':
      emitBinary(n, toff);
      emitRO((char*)"SUB", 3, 4, 3, (char*)"compute address");
      emitRM((char*)"LD", 3, 0, 3, (char*)"get value");
      break;
    case '.':
      /*not required*/
      break;
    default://problem
      printf("We have a problem. Recieved n->tClass of %i.\n", n->tClass);
      break;
  }
  if(folSib){gencode(n->sibling, toff, true);}
}


void initGlobs(string, void *m){
  struct tnd *n=(struct tnd*)m;
  if(n->idCase==2&&n->children[0]){//globals that require initialization
    gencode(n->children[0], -2, true);
    emitRM((char*)"ST", 3, n->offset, 0, (char*)"initialize", n->token->input);
  }
  if(n->idCase==2&&n->isArray){//globals that are arrays
    emitRM((char*)"LDC", 3, n->size-1, 0, (char*)"load size of", n->token->input);
    emitRM((char*)"ST", 3, n->offset+1, 0, (char*)"store size of", n->token->input);
  }
}


void emitAll(struct tnd *n){
  emitSkip(1);
  gencode(n, 0, true);
  backPatchAJumpToHere(0, (char*)"jump to init code [backpatched]");
  emitComment((char*)"INIT");
  emitRM((char*)"LD", 0, 0, 0, (char*)"set global pointer");
  emitRM((char*)"LDA", 1, goffset, 0, (char*)"set frame pointer");
  emitRM((char*)"ST", 1, 0, 1, (char*)"store frame pointer");
  emitComment((char*)"INIT globals");
  declarations.applyToAllGlobal(initGlobs);
  emitComment((char*)"END INIT globals");


  emitComment((char*)"Calling function main");
  emitRM((char*)"LDC", 2, 3+emitSkip(0), 0, (char*)"compute return address");
  emitRM((char*)"LDA", 1, -2, 1, (char*)"change frame pointer");
  emitRM((char*)"LDC", 7, ((struct tnd*)(declarations.lookup((char*)"main")))->offset, 0, (char*)"goto main");
  emitComment((char*)"end call to main");


  emitComment((char*)"END INIT");
  emitRO((char*)"HALT", 0, 0, 0, (char*)"end of program");
}