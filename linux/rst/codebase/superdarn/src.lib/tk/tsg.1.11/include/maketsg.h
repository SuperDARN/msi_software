/* maketsg.h
   ==========
   Author: R.J.Barnes
*/

/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.1-beta-18-gf704e97.txt for more information.
 
 
 
*/



#ifndef _MAKETSG_H
#define _MAKETSG_H

void TSGFree(struct TSGbuf *ptr);
struct TSGbuf *TSGMake(struct TSGprm *tsg,int *flg);


#endif
