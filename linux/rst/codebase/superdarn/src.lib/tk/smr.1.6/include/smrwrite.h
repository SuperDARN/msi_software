/* smrwrite.h
   ==========
   Author: R.J.Barnes
*/

/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.1-beta-18-gf704e97.txt for more information.
 
 
 
*/



#ifndef _SMRWRITE_H
#define _SMRWRITE_H

int SmrHeaderFwrite(FILE * fp,char *version,char *text,char *rname);
int SmrFwrite(FILE *fp,struct RadarParm *prm,struct FitData *fit,int pwr);
 
#endif
