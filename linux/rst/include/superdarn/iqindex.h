/* iqindex.h
   ========= 
   Author: R.J.Barnes
*/


/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.1-beta-18-gf704e97.txt for more information.
 
 
 
*/



#ifndef _IQINDEX_H
#define _IQINDEX_H


struct IQIndex {
  int num;
  double *tme;
  int *inx;
};

void IQIndexFree(struct IQIndex *inx);
struct IQIndex *IQIndexLoad(int fid);
struct IQIndex *IQIndexFload(FILE *fp);
 


#endif
