//semantic.h

//Hue Purkett
//CS445
//3/23/2017

#include<cstdlib>
#include<cstring>
#include"parser.tab.h"
#include"scanType.h"
#include"symbolTable.h"

extern int numwarnings, numerrors;

char semantic(FILE*, struct tnd*);