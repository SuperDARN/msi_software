/* make.c
   ====== 
   Author:R.J.Barnes
*/


/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.1-beta-18-gf704e97.txt for more information.
 
 
 
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rfbuffer.h"
#include "iplot.h"

struct Plot *PlotMake() {
  struct Plot *ptr;
  ptr=malloc(sizeof(struct Plot));
  if (ptr==NULL) return ptr;
  memset(ptr,0,sizeof(struct Plot));
  return ptr;
}

void PlotFree(struct Plot *ptr) {
  if (ptr==NULL) return;
  free(ptr);
};




