/* fitindex.h
   ========== 
   Author: R.J.Barnes
*/


/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.1-beta-18-gf704e97.txt for more information.
 
 
 
*/



#ifndef _FITINDEX_H
#define _FITINDEX_H


struct FitIndex {
  int num;
  double *tme;
  int *inx;
};

void FitIndexFree(struct FitIndex *inx);
struct FitIndex *FitIndexLoad(int fid);
struct FitIndex *FitIndexFload(FILE *fp);
 


#endif
