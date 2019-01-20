/* magcmp.h
   ========
   Author: R.J.Barnes
*/

/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.1-beta-18-gf704e97.txt for more information.
 
 
 
*/




#ifndef _MAGCMP_H
#define _MAGCMP_H

int IGRFMagCmp(double date, double frho, double flat, double flon, 
                double *bx, double *by, double *bz, double *b);

#endif
