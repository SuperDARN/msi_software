#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(void)
{
	int i;
  int scnsc=120;
  int scnus=0;
  int integ_durr = 3;   /* Themis instruments make measurements every 3 s */

	int bufsc=0;		/* a buffer at the end of scan; historically this has   */
	int bufus=0;		/*   been set to 3.0s to account for ???                */

	int *intgt;
	int *bms;

	int nintgs;		/* number of integration periods possible */

	int sbm, ebm, cbm;
	int temp;

	int backwards;
	int odd;

	sbm = 1;
	ebm = 13;
	cbm = 1;

	backwards = (sbm > ebm) ? 1 : 0;

  /* number of integration periods possible in scan time */
  /* How sure are we about the 3s buffer? */
  nintgs = (int)floor((scnsc+scnus*1e-6 - (bufsc+bufus*1e-6))/integ_durr);

printf("number of integrations (beams): %d\n", nintgs);
printf("\n");
printf("Start Beam: %2d\n", sbm);
printf("End   Beam: %2d\n", ebm);
printf("Camp  Beam: %2d\n", cbm);
printf("\n");

  /* arrays for integration start times and beam sequences */
  intgt = (int *)malloc(nintgs*sizeof(int));
  bms   = (int *)malloc(nintgs*sizeof(int));

  for (i=0; i<nintgs; i++) {
    intgt[i] = i*integ_durr;        /* start time of each integration period */
//		printf("  %3d\n", intgt[i]);
	}


  /* Create a list of the beams that will be sequenced through, alternating
   * between the camping beam and each other beam in turn */

	odd = (sbm == cbm);

	if (backwards) {
		temp = sbm;
		if (sbm == cbm) temp--;

    for (i=0; i<nintgs; i++){
      if ((i%2 == odd) && (temp >= ebm)) {
        if ((temp == cbm) && (sbm != cbm)) temp--;	/* exclude camp bm from regular beams */
				if (temp >= ebm) bms[i] = temp;
				else bms[i] = cbm;
        temp--;
      } else {
        bms[i] = cbm;
      }
//			printf("    temp: %2d %2d\n", i, temp);
    }
	} else {
		temp = sbm;
		if (sbm == cbm) temp++;

    for (i=0; i<nintgs; i++){
      if ((i%2 == odd) && (temp <= ebm)) {
        if ((temp == cbm) && (sbm != cbm)) temp++;	/* exclude camp bm from regular beams */
				if (temp <= ebm) bms[i] = temp;
				else bms[i] = cbm;
        temp++;
      } else {
        bms[i] = cbm;
      }
    }
	}

	printf("sequence  beam no.\n");
	for (i=0; i<nintgs; i++) {
		printf("  %2d       %2d", i, bms[i]);
		if (bms[i] == cbm) printf(" CAMP");
		printf("\n");
	}

	free(intgt);
	free(bms);

	return (0);
}

