//printtree.cpp

//Hue Purkett
//CS445
//2/16/2017


#include"printtree.h"

void pt(FILE *out, struct tnd *n, int level, int sibling){
  int i, j;
  switch(n->tClass){
    case ID:
      switch(n->idCase){
        case 1://record
          fprintf(out, "Record %s [line: %d]\n", n->token->input, n->lineno);
          break;
        case 2://variable
          fprintf(out, "Var %s ", n->token->input);
          if(n->isArray){fprintf(out, "is array ");}
          fprintf(out, "of type %s [line: %d]\n", n->varType->input, n->lineno);
          break;
        case 3://function
          fprintf(out, "Func %s returns type ", n->token->input);
          if(n->varType==NULL){
            fprintf(out, "void ");
          }else{
            fprintf(out, "%s ", n->varType->input);
          }
          fprintf(out, "[line: %d]\n", n->lineno);
          break;
        case 4://paramId
          fprintf(out, "Param %s ", n->token->input);
          if(n->isArray){fprintf(out, "is array ");}
          fprintf(out, "of type %s [line: %d]\n", n->varType->input, n->lineno);
          break;
        case 5://mutable
          fprintf(out, "Id: %s ", n->token->input);
          if(n->isArray){fprintf(out, "is array ");}
          fprintf(out, "[line: %d]\n", n->lineno);
          break;
        case 6://call
          fprintf(out, "Call: %s [line: %d]\n", n->token->input, n->lineno);
          break;
        default:
          fprintf(out, "INVALID ID SPECIFIER %s [line: %d]\n", n->token->input, n->lineno);
          break;
      }
      break;
    case IF:
      fprintf(out, "If [line: %d]\n", n->lineno);
      break;
    case WHILE:
      fprintf(out, "While [line: %d]\n", n->lineno);
      break;
    case RETURN:
      fprintf(out, "Return [line: %d]\n", n->lineno);
      break;
    case BREAK:
      fprintf(out, "Break [line: %d]\n", n->lineno);
      break;
    case CHARCONST:
      fprintf(out, "Const: '%c' [line: %d]\n", n->token->cval, n->lineno);
      break;
    case NUMCONST:
    case BOOLCONST:
      fprintf(out, "Const: %s [line: %d]\n", n->token->input, n->lineno);
      break;
    case MULASS:
    case INC:
    case ADDASS:
    case DEC:
    case SUBASS:
    case DIVASS:
    case '=':
      fprintf(out, "Assign: %s [line: %d]\n", n->token->input, n->lineno);
      break;
    case '{':
      fprintf(out, "Compound [line: %d]\n", n->lineno);
      break;
    default:
      fprintf(out, "Op: %s [line: %d]\n", n->token->input, n->lineno);
      break;
  }
  for(i=0; i<3; i++){
    if(n->children[i]!=NULL){
      for(j=-1; j<level; j++){fprintf(out, "!   ");}
      fprintf(out, "Child: %d  ", i);
      pt(out, n->children[i], level+1, 0);
    }
  }
  if(n->sibling!=NULL){
    for(j=0; j<level; j++){fprintf(out, "!   ");}
    fprintf(out, "Sibling: %d  ", sibling);
    pt(out, n->sibling, level, sibling+1);
  }
}


void pt2(FILE *out, struct tnd *n, int level, int sibling){
  int i, j;
  char *type;
  if(!strcmp(n->varType->input, "undefined")){
    type=strdup("[undefined type] ");
  }else{
    type=new char[10+strlen(n->varType->input)];
    strcpy(type, "[type ");
    strcat(type, n->varType->input);
    strcat(type, "] ");
  }
  switch(n->tClass){
    case ID:
      switch(n->idCase){
        case 1://record
          fprintf(out, "Record %s [ref: Global, size: %i, loc: %i] %s[line: %d]\n", n->token->input, n->size, n->offset, type, n->lineno);
          break;
        case 2://variable
          fprintf(out, "Var %s ", n->token->input);
          if(n->isArray){fprintf(out, "is array ");}
          fprintf(out, " [ref: ");
          if(n->isLocal){
            if(n->isParam){
              fprintf(out, "Param");
            }else{
              if(n->isStatic){
                fprintf(out, "Static");
              }else{
                fprintf(out, "Local");
              }
            }
          }else{
            fprintf(out, "Global");
          }
          fprintf(out, ", size: %i, loc: %i] %s[line: %d]\n", n->size, n->offset, type, n->lineno);
          break;
        case 3://function
          fprintf(out, "Func %s returns type ", n->token->input);
          if(n->varType==NULL){
            fprintf(out, "void ");
          }else{
            fprintf(out, "%s ", n->varType->input);
          }
          fprintf(out, "[ref: Global, size: %i, loc: %i] %s[line: %d]\n", n->size, n->offset, type, n->lineno);
          break;
        case 4://paramId
          fprintf(out, "Param %s ", n->token->input);
          if(n->isArray){fprintf(out, "is array ");}
          fprintf(out, "[ref: Param, size: %i, loc: %i] %s[line: %d]\n", n->size, n->offset, type, n->lineno);
          break;
        case 5://mutable
          fprintf(out, "Id: %s ", n->token->input);
          if(n->isArray){fprintf(out, "is array ");}
          fprintf(out, "[ref: ");
          if(n->undefined){
            fprintf(out, "None");
          }else{
            if(n->isLocal){
              if(n->isParam){
                fprintf(out, "Param");
              }else{
                if(n->isStatic){
                  fprintf(out, "Static");
                }else{
                  fprintf(out, "Local");
                }
              }
            }else{
              fprintf(out, "Global");
            }
          }
          fprintf(out, ", size: %i, loc: %i] %s[line: %d]\n", n->size, n->offset, type, n->lineno);
          break;
        case 6://call
          fprintf(out, "Call: %s [ref: None, size: %i, loc: %i] %s[line: %d]\n", n->token->input, n->size, n->offset, type, n->lineno);
          break;
        default:
          fprintf(out, "INVALID ID SPECIFIER %s %s [line: %d]\n", n->token->input, type, n->lineno);
          break;
      }
      break;
    case IF:
      fprintf(out, "If %s [line: %d]\n", type, n->lineno);
      break;
    case WHILE:
      fprintf(out, "While %s [line: %d]\n", type, n->lineno);
      break;
    case RETURN:
      fprintf(out, "Return %s [line: %d]\n", type, n->lineno);
      break;
    case BREAK:
      fprintf(out, "Break %s [line: %d]\n", type, n->lineno);
      break;
    case CHARCONST:
      fprintf(out, "Const: '%c' %s [line: %d]\n", n->token->cval, type, n->lineno);
      break;
    case NUMCONST:
    case BOOLCONST:
      fprintf(out, "Const: %s %s [line: %d]\n", n->token->input, type, n->lineno);
      break;
    case MULASS:
    case INC:
    case ADDASS:
    case DEC:
    case SUBASS:
    case DIVASS:
    case '=':
      fprintf(out, "Assign: %s %s [line: %d]\n", n->token->input, type, n->lineno);
      break;
    case '{':
      fprintf(out, "Compound %s [line: %d]\n", type, n->lineno);
      break;
    default:
      fprintf(out, "Op: %s %s [line: %d]\n", n->token->input, type, n->lineno);
      break;
  }
  delete[] type;
  for(i=0; i<3; i++){
    if(n->children[i]!=NULL){
      for(j=-1; j<level; j++){fprintf(out, "!   ");}
      fprintf(out, "Child: %d  ", i);
      pt2(out, n->children[i], level+1, 0);
    }
  }
  if(n->sibling!=NULL){
    for(j=0; j<level; j++){fprintf(out, "!   ");}
    fprintf(out, "Sibling: %d  ", sibling);
    pt2(out, n->sibling, level, sibling+1);
  }
}


void printTree(FILE *out, struct tnd *root, int which){
  if(which==0){
    pt(out, root, 0, 0);
  }else{
    pt2(out, root, 0, 0);
  }
}
