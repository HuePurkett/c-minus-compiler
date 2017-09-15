//scanType.h

//Hue Purkett
//CS445
//1/26/2017

#ifndef SCANTYPE
#define SCANTYPE

struct td{
  td(void *a){}
  td(int c, char *i){tClass=c; input=i; lineno=-1; ival=0; cval=0; isStatic=0;}
  int tClass;
  int lineno;
  char *input;
  int ival;
  char cval;

  int isStatic;
  int isConst;
};



struct tnd{
  tnd(void *a){undefined=false;}
  struct tnd *children[3];
  struct tnd *sibling;
  struct td *token;
  int tClass;
  int lineno;

  struct td *varType;
  int idCase;//1:record, 2:variable declaration, 3:function, 4:parameter, 5:variable usage, 6:function call    else 0
  bool isArray;
  int arraylen;
  bool isStatic;
  bool isConst;

  int size;//negative for functions, positive otherwise
  int offset;//negative
  bool isLocal;
  bool isParam;
  bool undefined;
};


#endif