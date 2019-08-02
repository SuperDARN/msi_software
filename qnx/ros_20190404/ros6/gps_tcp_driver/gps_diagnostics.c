#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/types.h>
#ifdef __QNX__
  #include <hw/pci.h>
  #include <hw/inout.h>
  #include <sys/neutrino.h>
  #include <sys/iofunc.h>
  #include <sys/dispatch.h>
#endif
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "control_program.h"
#include "global_server_variables.h"
#include "include/registers.h"
#include "include/_refresh_state.h"
#include "utils.h"

#define MAX_ERR 1e-3
#define MAX_SECDIFF 1000

int configured=0, locked=0;
int IRQ;
unsigned char *BASE0=NULL, *BASE1=NULL;
struct GPSStatus dstat;

int verbose = 0;

pthread_mutex_t gps_state_lock;
struct timespec timecompare, event;
int timecompareupdate=0, eventupdate=0;

int set_time_compare(int mask, struct timespec *nexttime);
int set_time_compare_register(int mask, struct timespec *nexttime);

/*-MAIN--------------------------------------------------------------*/
int main(void)
{
  unsigned long k=0;
  int temp, pci_handle;
  struct timespec now, start;
  unsigned int *mmap_io_ptr;
  float timediff, oldtimediff=0;
  int sys_tv_sec, gps_tv_sec;

  pthread_mutex_init(&gps_state_lock, NULL);

  /* OPEN THE PLX9656 AND GET LOCAL BASE ADDRESSES */
  temp = _open_PLX9050(&BASE0, &BASE1, &pci_handle, &mmap_io_ptr, &IRQ);
  if (temp == -1) {
    fprintf(stdout, " PLX9695 configuration failed\n");
    fprintf(stdout, " Exit now\n");
    return (-1);
  }

  fprintf(stdout, "IRQ is %d\n",IRQ);
  fprintf(stdout, "BASE0 %p  BASE1 %p\n", BASE0, BASE1);

  //INITIALIZE CARD

  fprintf(stdout, "Configuring GPS Card\n");

  // set card for synchronized generator mode, GPS reference enabled
  // Daylight Savings Time Disabled 
  *((uint32_t*)(BASE1+0x118)) = 0x90000021; //x90 to address x11B produced
                                            // 10 MHz reference signal output


  // Not really sure what this accomplishes... SGS
  //clear time compare register
  //now.tv_sec += 0;  // SGS: this HAS to be a mistake
  now.tv_sec  = 0;
  now.tv_nsec = 0;
  temp = set_time_compare(0, &now);

  // clear time compare event
  *((uint08*)(BASE1+0xf8)) |= 0x02;

  *((uint32_t*)(BASE1+0x128)) = 2;  // set rate synthesiser rate to a default
                                    //   of 100ms (2pps)

  *((uint32_t*)(BASE1+0x12c)) = 0x02000f00;// x12e: event trigger on external
                                           //       event, falling edge 
                                           // x12d: rising edge rate
                                           //       synthesizer on pin 6
                                           // x12f: rate generator on code-out

  fprintf(stdout,"****************************************");
  fprintf(stdout,"****************************************\n");
  fprintf(stdout,"GPS Card Registers\n");
  fprintf(stdout,"Rate synth:   0x%08x\n",*((uint32_t*)(BASE1+0x128)));
  fprintf(stdout,"Cntl:         0x%08x\n",*((uint32_t*)(BASE1+0x12c)));
  fprintf(stdout,"Misc ctrl:          0x%02x\n",*((uint8_t*)(BASE1+0x12C)));
  fprintf(stdout,"Rate Sync ctrl:     0x%02x\n",*((uint8_t*)(BASE1+0x12D)));
  fprintf(stdout,"Event Capture ctrl: 0x%02x\n",*((uint8_t*)(BASE1+0x12E)));
  fprintf(stdout,"Code Out cntrl:     0x%02x\n",*((uint8_t*)(BASE1+0x12F)));
  fprintf(stdout,"****************************************");
  fprintf(stdout,"****************************************\n");
  fprintf(stdout,"Antenna Status:     0x%02x\n",*((uint8_t*)(BASE1+0xFE)));
  fprintf(stdout,"Backgrnd Test Stat: 0x%02x\n",*((uint8_t*)(BASE1+0x11C)));
  fprintf(stdout,"Satellite A:        0x%02x\n",*((uint8_t*)(BASE1+0x198)));
  fprintf(stdout,"Satellite B:        0x%02x\n",*((uint8_t*)(BASE1+0x19C)));
  fprintf(stdout,"Satellite C:        0x%02x\n",*((uint8_t*)(BASE1+0x1A0)));
  fprintf(stdout,"Satellite D:        0x%02x\n",*((uint8_t*)(BASE1+0x1A4)));
  fprintf(stdout,"Satellite E:        0x%02x\n",*((uint8_t*)(BASE1+0x1A8)));
  fprintf(stdout,"Satellite F:        0x%02x\n",*((uint8_t*)(BASE1+0x1AC)));
  fprintf(stdout,"****************************************");
  fprintf(stdout,"****************************************\n");
  fprintf(stdout,"\n\n");
 
  fprintf(stdout, "Waiting for GPS lock\n");
  while (1) {

    pthread_mutex_lock(&gps_state_lock);
    dstat.hardware = get_hdw_stat(); //check hardware status
    dstat.antenna = get_ant_stat();  //check antenna status
    dstat.lock = get_lock_stat();    //check GPS lock status
    pthread_mutex_unlock(&gps_state_lock);

    if (!dstat.hardware && dstat.antenna && dstat.lock) break;

    if (k % 15 == 0) {
      fprintf(stdout, "hdw:  %d\n", dstat.hardware);
      fprintf(stdout, "ant:  %d\n", dstat.antenna);
      fprintf(stdout, "lock: %d\n", dstat.lock);
    }
    sleep(1);
    k++;
  }
  fprintf(stdout, "GPS lock: %lu\n", k);

  /* */
  temp = clock_gettime(CLOCK_REALTIME, &start);
  fprintf(stdout, "\nSTART\n");
  fprintf(stdout, "%03d %s\n", start.tv_sec, ctime(&start.tv_sec));

  dstat.poscnt = 0;
  dstat.mlat   = 0;
  dstat.mlon   = 0;
  dstat.alt    = 0;
  dstat.mdrift = 0.;
  k = 0;
  while (1) {
    // system time
    temp = clock_gettime(CLOCK_REALTIME, &now);
    dstat.syssecond = now.tv_sec;
    dstat.sysnsecond = now.tv_nsec;
    sys_tv_sec = now.tv_sec;

    // GPS time
    //pthread_mutex_lock(&gps_state_lock);
    temp = get_software_time(&dstat.gpssecond, &dstat.gpsnsecond, BASE1);
    //pthread_mutex_unlock(&gps_state_lock);

    gps_tv_sec = dstat.gpssecond;

    //caclulate time difference in seconds
    timediff = (float)(dstat.syssecond-dstat.gpssecond) +
                     ((float)(dstat.sysnsecond-dstat.gpsnsecond))/1e9;
    //calculate time drift in microseconds
    dstat.drift = 1e6*(timediff-oldtimediff);
    oldtimediff = timediff;
    dstat.mdrift = ((dstat.mdrift*dstat.poscnt) + dstat.drift) /
                    ((float)dstat.poscnt+1);

    fprintf(stdout, "%03d: SYSTEM: %d %s", k, sys_tv_sec, ctime(&sys_tv_sec));
    fprintf(stdout, "     GPS   : %d %s", gps_tv_sec, ctime(&gps_tv_sec));
    fprintf(stdout, " difference: %f\n", timediff);
    fprintf(stdout, "      drift: %f us\n", dstat.drift);
    fprintf(stdout, "  ave drift: %f us\n", dstat.mdrift);
    fprintf(stdout, "\n");

    if (fabs(timediff) > MAX_ERR) {
      fprintf(stdout, "****************************************************\n");
      fprintf(stdout, "Updating System Time...\n");
      now.tv_sec  = dstat.gpssecond;
      now.tv_nsec = dstat.gpsnsecond;
      clock_settime(CLOCK_REALTIME, &now);
      system("rtc -r 100 -s hw");
      fprintf(stdout, "System Time updated successfully!\n");
      fprintf(stdout, "****************************************************\n");
      //return (0);
    } else {
      fprintf(stdout, "System Time OK.\n");
    }

    //get GPS position and calculate mean position
    dstat.lat = get_lat();
    dstat.mlat = ((dstat.mlat*dstat.poscnt) + dstat.lat)/
                        ((float)dstat.poscnt+1);

    dstat.lon = get_lon();
    dstat.mlon = ((dstat.mlon*dstat.poscnt) + dstat.lon)/
                        ((float)dstat.poscnt+1);

    dstat.alt = get_alt();
    dstat.malt = ((dstat.malt*dstat.poscnt) + dstat.alt)/
                        ((float)dstat.poscnt+1);
    dstat.poscnt++;

    fprintf(stdout, "\n");
    fprintf(stdout, " lat: %f\n", dstat.lat);
    fprintf(stdout, " lon: %f\n", dstat.lon);
    fprintf(stdout, " alt: %f\n", dstat.alt);
    fprintf(stdout, "\n");

    sleep(1);   // pause for 1 second
    k++;
  }

  /* CLOSE THE PLX9656 AND CLEAR ALL MEMORY MAPS */
  temp = _close_PLX9656(pci_handle, BASE0, BASE1, mmap_io_ptr);
  printf("close:    0x%x\n", 3);

}

