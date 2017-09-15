//printtree.h

//Hue Purkett
//CS445
//2/16/2017


#include<cstdio>
#include<cstdlib>
#include<cstring>
#include"parser.tab.h"
#include"scanType.h"

void pt(FILE *out, struct tnd *n, int level, int sibling);
void printTree(FILE *out, struct tnd *root, int which);
