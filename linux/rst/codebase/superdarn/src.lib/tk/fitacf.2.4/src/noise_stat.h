/* noise_stat.h
   ============
   Author: R.J.Barnes & K.Baker & P.Ponomarenko
*/


/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.1-beta-18-gf704e97.txt for more information.
 
 
 
 */



double lag_power(struct complex *a);

double noise_stat(double mnpwr,struct FitPrm *ptr,
                  struct FitACFBadSample *badsmp,
		  struct complex *acf);
