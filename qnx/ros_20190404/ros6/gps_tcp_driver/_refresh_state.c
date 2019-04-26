#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#ifdef __QNX__
  #include <hw/pci.h>
  #include <hw/inout.h>
  #include <sys/neutrino.h>
  #include <sys/iofunc.h>
  #include <sys/dispatch.h>
#endif
#ifdef __linux__
  #include <linux/rtc.h>
  #include <sys/ioctl.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "include/registers.h"
//#include "include/_prog_conventions.h"
#include "include/_isr_handler.h"
#include "include/_refresh_state.h"
//#include "include/_display_structure.h"
#include "control_program.h"
#include "global_server_variables.h"
#include "utils.h"

#define MAX_ERR 0.01
#define MAX_SECDIFF 1000 

extern unsigned char *BASE0, *BASE1;
extern int IRQ;
extern int tc_count;
extern int intid;
extern int configured;
extern struct sigevent interruptevent;
extern struct GPSStatus dstat;
extern struct timespec timecompare,event;
extern int RateSynthInterrupt, EventInterrupt, TimeCompareInterrupt;
extern int timecompareupdate, eventupdate;
extern pthread_t int_thread_id,refresh_thread_id;
extern int verbose;
extern pthread_mutex_t gps_state_lock;

int set_time_compare(int mask,struct timespec *nexttime);
int set_time_compare_register(int mask,struct timespec *nexttime);

/*-GET_HDW_STAT------------------------------------------------------*/
int get_hdw_stat()
{
  int temp=0;

  // check hardware status
  if (BASE1 != NULL) {
    temp = *((uint08*)(BASE1+0x11c));
    temp = temp & 0x0f;
  }

  return temp;
}

/*-GET_LOCK_STAT------------------------------------------------------*/
int get_lock_stat()
{
  int temp=0;

  // check lock status
  if (BASE1 != NULL) {
    temp = *((uint32_t*)(BASE1+0x104));
    temp = (temp & 0x00007000) >> 12;
  }

  return temp;
}

/*-GET_ANT_STAT------------------------------------------------------*/
int get_ant_stat()
{
  int temp=0;

  // check antenna status
  if (BASE1 != NULL) {
    temp = *((uint32_t*)(BASE1+0xfc));
    temp = temp & 0x00300000;
    temp = temp >> 20;
  }

  return temp;
}

/*-GET_SAT_STAT------------------------------------------------------*/
int get_sat_stat(int sv[6], float signal[6])
{
  int i;
  int temp=0;

  if (BASE1 != NULL) {
    for (i=0; i<6; i++) {
      //read satellite signal levels
      temp = *((uint32_t*)(BASE1+0x198+4*i));
      sv[i] = 10*((temp & 0xf0) >> 4) + 1*(temp & 0x0f); 
      signal[i] = 10*(float)((temp & 0xf0000000) >> 28) +
                   1*(float)((temp & 0xf000000) >> 24) +
                 0.1*(float)((temp & 0xf00000) >>20) +
                0.01*(float)((temp & 0xf0000) >> 16);
    }
  }

  temp = get_lock_stat();
  temp = temp & 1;
  if (temp == 0) return -1;
  if (temp == 1) return 1;
}

/*-INCREMENT_TIME-------------------------------------------------*/
// This function simply increments the time in 'nexttime' by
// 'increment' ns.
int increment_time(struct timespec *nexttime, int increment)
{
  nexttime->tv_nsec += increment;
  if ((nexttime->tv_nsec) >= 1000000000) {
    nexttime->tv_sec += 1;
    nexttime->tv_nsec -= 1000000000;
  }

  return 0;
}

/*-SET_TIME_COMPARE-------------------------------------------------*/
int set_time_compare(int mask, struct timespec *nexttime)
{
  int temp, gpssecond, gpsnsecond, validtime, checkagain;
  int settimecompareerror=0;
  struct timespec start, stop;

  temp = set_time_compare_register(mask, nexttime);
  dstat.nextcomparesec = nexttime->tv_sec;
  dstat.nextcomparensec = nexttime->tv_nsec;

  return 0;
}

/*-SET_TIME_COMPARE_register-------------------------------------------------*/
int set_time_compare_register(int mask, struct timespec *nexttime)
{
  int hmsec,tmsec,msec,husec,tusec,usec;
  int tmins,mins,tsec,sec;
  int thours,hours,hdays,tdays,days;
  int count, nextsec, nextnsec;
  struct tm *localtime;
  float second;

  nextsec = nexttime->tv_sec;
  nextnsec = nexttime->tv_nsec;
  localtime = gmtime((time_t*)&nextsec);
  localtime->tm_yday += 1;
  
  //seconds
  tsec = (int)(localtime->tm_sec/10);
  localtime->tm_sec -= (tsec*10);
  sec = localtime->tm_sec;
  //minutes
  tmins = (int)(localtime->tm_min/10);
  localtime->tm_min -= (tmins*10);
  mins = localtime->tm_min;
  //hours
  thours = (int)(localtime->tm_hour/10);
  localtime->tm_hour -= (thours*10);
  hours = localtime->tm_hour;
  //days
  hdays = (int)(localtime->tm_yday/100);
  localtime->tm_yday -= (hdays*100);
  tdays = (int)(localtime->tm_yday/10);
  localtime->tm_yday -= (tdays*10);
  days = localtime->tm_yday;
  //mseconds
  hmsec = (int)(nextnsec/100000000);
  nextnsec -= (hmsec*100000000);
  tmsec = (int)(nextnsec/10000000);
  nextnsec -= (tmsec*10000000);
  msec = (int)(nextnsec/1000000);
  nextnsec -= (msec*1000000);
  //microseconds
  husec = (int)(nextnsec/100000);
  nextnsec -= (husec*100000);
  tusec = (int)(nextnsec/10000);
  nextnsec -= (tusec*10000);
  usec = (int)(nextnsec/1000);
  nextnsec -= (usec*1000);

  //mask=0x0;
  if (BASE1 != NULL) {
    *((uint32_t*)(BASE1+0x138)) = ((tsec & 0xf)<<28) + ((sec & 0xf)<<24) +
                                  ((hmsec & 0xf)<<20) + ((tmsec & 0xf)<<16) +
                                  ((msec & 0xf)<<12) + ((husec & 0xf)<<8) +
                                  ((tusec & 0xf)<<4) + (usec & 0xf);

    *((uint32_t*)(BASE1+0x13c)) = ((mask & 0xf)<<28) + ((hdays & 0xf)<<24) +
                                  ((tdays & 0xf)<<20) + ((days & 0xf)<<16) +
                                  ((thours & 0xf)<<12) + ((hours & 0xf)<<8) +
                                  ((tmins & 0xf)<<4) + (mins & 0xf);
  }

  return 0; 
}

/*-GET_LAT-----------------------------------------------------------*/
float get_lat()
{
  int deg,min,sec,tenthsec;
  int temp;
  float degf=0.0;

  if (BASE1 != NULL) {
    temp = *((uint32_t*)(BASE1+0x108));
    deg = 100*((temp & 0xf00) >> 8) + 10*((temp & 0xf0) >> 4) + ((temp & 0xf));
    min = 10*((temp &0xf00000) >> 20) + ((temp & 0xf0000) >> 16);
    temp = *((uint32_t*)(BASE1+0x10c));
    sec = 10*((temp & 0xf000) >> 12) + ((temp & 0xf00) >> 8);
    tenthsec = (temp & 0xf);

    degf = (float)deg+((float)min/60)+(((float)sec + 0.1*(float)tenthsec)/3600);

    temp = *((uint32_t*)(BASE1+0x108));
    temp = ((temp&0xff000000) >> 24);

    if (temp == 0x4e) return degf;
    if (temp == 0x53) degf = 0-degf;
  }

  return degf;
}

/*-GET_LON-----------------------------------------------------------*/
float get_lon()
{
  int deg, min, sec, tenthsec;
  int temp;
  float degf=0.0;

  if (BASE1 != NULL) {
    temp = *((uint32_t*)(BASE1+0x10c));
    deg = 100*((temp & 0xf000000) >> 24) + 10*((temp & 0xf00000) >> 20) +
              ((temp & 0xf0000) >> 16);
    temp = *((uint32_t*)(BASE1+0x110));
    min = 10*((temp & 0xf0) >> 4) + (temp & 0xf);
    sec = 10*((temp & 0xf0000000) >> 28) + ((temp & 0xf000000) >> 24);
    tenthsec = ((temp & 0xf0000) >> 16);

    degf = (float)deg+((float)min/60)+(((float)sec + 0.1*(float)tenthsec)/3600);
  
    temp = ((temp & 0xff00) >> 8);

    if (temp == 0x57) return degf;
    if (temp == 0x45) degf = 360-degf;
  } 

  return degf;
}

/*-GET_ALT-----------------------------------------------------------*/
float get_alt()
{
  int alt, tenthmeter, temp;
  float altf=0.0;

  if (BASE1 != NULL) {
    temp = *((uint32_t*)(BASE1+0x114));

    alt = 10000*((temp & 0xf0) >> 4) + 1000*((temp & 0xf)) +
                ((temp & 0xf00000) >> 20) + 10*((temp & 0xf000000) >> 24) +
            100*((temp & 0xf0000000) >> 28);
    tenthmeter = (temp & 0xf0000) >> 16;
    altf = (float)alt + 0.1*((float)tenthmeter);
    temp = ((temp & 0xff00) >> 8);

    if (temp == 0x2b) return altf;
    if (temp == 0x2d) altf = 0-altf;
  }

  return altf;
}

/*-MONITOR_TIMECOMPARE---------------------------------------------------*/ 
void *monitor_timecompare(void *arg)
{
  struct timespec start, stop, sleep, timeA, timeB;
  int    gpssecond,gpsnsecond;
  int    temp;

  sleep.tv_sec  = 0;
  sleep.tv_nsec = 100000; 
  while (1) { 
    if (dstat.intervalmode || dstat.scheduledintervalmode ||
        dstat.oneshot) {

      temp = clock_gettime(CLOCK_REALTIME,&start);  
      temp = get_compare_time(&timeA.tv_sec,&timeA.tv_nsec,BASE1);
      temp = get_software_time(&timeB.tv_sec,&timeB.tv_nsec,BASE1);
      if (time_greater(&timeA,&timeB)) {
        dstat.timecompareupdateerror = 0; 
      } else {
        increment_time(&timeA,TIME_INTERVAL);
        temp = set_time_compare(0, &timeA);
        dstat.timecompareupdateerror = 1; 
      }
      temp = clock_gettime(CLOCK_REALTIME,&stop); 
      dstat.settimecomparetime = (float)((stop.tv_sec-start.tv_sec) +
                                              (stop.tv_nsec-start.tv_nsec)/1e9);
      nanosleep(&sleep,NULL); 
    } else nanosleep(&sleep,NULL);
  }
}

/*-TIME_GREATER---------------------------------------------------*/  
int time_greater(struct timespec *timeA, struct timespec *timeB)
{
  if ((timeA->tv_sec > timeB->tv_sec) || ((timeA->tv_sec == timeB->tv_sec) &&
                                          (timeA->tv_nsec > timeB->tv_nsec)))
    return(1);
  else
    return(0);
}

/*-TIME_STRING---------------------------------------------------*/ 
char *time_string(struct timespec *timeA)
{
  struct tm* localtime;
  static char time_s[30];
  int i, temp;
  float fseconds;

  //for(i=0;i<10;i++) time_s[i]=118;

  localtime = gmtime(&timeA->tv_sec);
  fseconds = (float)((float)localtime->tm_sec+(float)timeA->tv_nsec/1e9);
  if (fseconds < 10)
    sprintf(time_s, "%02d/%02d/%04d %02d:%02d:0%08.6f UT", localtime->tm_mon+1,
                    localtime->tm_mday, localtime->tm_year+1900,
                    localtime->tm_hour, localtime->tm_min, fseconds);
  else
    sprintf(time_s, "%02d/%02d/%04d %02d:%02d:%08.6f UT", localtime->tm_mon+1,
                    localtime->tm_mday, localtime->tm_year+1900,
                    localtime->tm_hour, localtime->tm_min, fseconds);
  //time_s[10]='\0';

  return time_s;
}

/*-REFRESH_---------------------------------------------------*/  
void *refresh_state(void *arg)
{
  int temp;
  struct timespec start, stop;

  if (verbose > 0) printf("Starting REFRESH\n");
  temp = clock_gettime(CLOCK_REALTIME, &start);
  while(1) { 
    temp = clock_gettime(CLOCK_REALTIME, &stop);
    if ((stop.tv_sec-start.tv_sec) >= REFRESHRATE && configured) {
      pthread_mutex_lock(&gps_state_lock);
      get_state();
      pthread_mutex_unlock(&gps_state_lock);
      start = stop;
    }
    usleep(10000);  // sleep for 10 ms
  }
  if (verbose > 0) printf("Exiting REFRESH\n");
}

void *get_state()
{
  int gpssecond,gpsnsecond,lastsetsec,softwaretime;
  int syssecond,sysnsecond,lastsetnsec;
  int sv[6];
  float signal[6],fractional,integral;
  float timediff, oldtimediff=0, drift=0, mdrift;
  int cnt, rst, ch;
  int temp,i,system_tv_sec,gps_tv_sec;
  struct timespec start_p, stop_p, sleep, now;
  float tempf;

  /* Get status of hardware and satellites */
  if (verbose > 0) printf("Starting GET_STATE()\n");

  //check hardware status
  dstat.hardware = get_hdw_stat();

  //check antenna status
  dstat.antenna = get_ant_stat();

  //check GPS lock status
  dstat.lock = get_lock_stat();

  if ((dstat.lock & 1) == 1) dstat.gps_lock = 1;
  else                             dstat.gps_lock = 0; 

  if ((dstat.lock & 2) == 2) dstat.reference_lock = 1;
  else                             dstat.reference_lock = 0; 

  if ((dstat.lock & 4) == 4) dstat.phase_lock = 1;
  else                             dstat.phase_lock = 0; 

  //check satellite status
  temp = get_sat_stat(dstat.sv,dstat.signal);
//      for (i=0;i<6;i+=2){
//      }

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

  /* determine time and update clock, if necessary */
  //get GPS time and system time
  temp = clock_gettime(CLOCK_REALTIME, &now);
  system_tv_sec = now.tv_sec;
  gps_tv_sec = dstat.gpssecond;
  temp = get_software_time(&dstat.gpssecond, &dstat.gpsnsecond, BASE1);
  dstat.syssecond = now.tv_sec;
  dstat.sysnsecond = now.tv_nsec;

  //caclulate time difference
  timediff = (float)(dstat.syssecond-dstat.gpssecond) +
                   ((float)(dstat.sysnsecond-dstat.gpsnsecond))/1e9;
  dstat.drift = 1e6*(timediff-oldtimediff);
  oldtimediff = timediff;
  dstat.mdrift = ((dstat.mdrift*dstat.poscnt) +
                         dstat.drift ) / ((float)dstat.poscnt+1);
  dstat.poscnt++;

  if (verbose > 0) {
    printf("  get_state(): %d\n", dstat.poscnt);
    printf("    time diff  %13.9f s\n", timediff);
    printf("    drift      %ld    us\n", (long)dstat.drift);
    printf("    ave drift  %ld    us\n", (long)dstat.mdrift);
    printf("    MAX_ERR    %f     s\n", MAX_ERR);
  }

  if ((fabs(timediff) > MAX_ERR) && (dstat.reference_lock) &&
      (dstat.gps_lock) && (dstat.phase_lock)) {
    if (verbose > 0) printf("Preparing to Update System Time\n");
    if (fabs(timediff) < MAX_SECDIFF) {
      now.tv_sec = dstat.gpssecond;
      now.tv_nsec = dstat.gpsnsecond;
      clock_settime(CLOCK_REALTIME,&now);
      #ifdef __QNX__              
        system("rtc -r 100 -s hw"); // QNX real-time clock
                                    //
                                    // -r 100  Rate at which to adjust the OS
                                    //         time if it's currently within 60
                                    //         seconds (or value set by -S) of
                                    //         the time from the source
                                    //         specified by clock_type. The
                                    //         rate is turned into a percentage
                                    //         1/rate. The default rate is 100
                                    //         (1%). 
                                    //
                                    // -s hw   Hardware clock (automatically
                                    //         selects one based on information
                                    //         provided by the startup)
        if (verbose > 0) printf("System Time Updated Successfully\n");
      #endif
      #ifdef __linux__
//                  printf("Setting hdw clock on linux\n");
//                  int fd = open("/dev/rtc", O_RDONLY);
//                  struct rtc_time  *rt=NULL;
//                  rt=(struct rtc_time*)gmtime(&now.tv_sec);              
//                  ioctl(fd, RTC_SET_TIME, rt);
//                  close(fd);
      #endif
    } else {
      fprintf(stderr,"ERROR: Time Difference between GPS and System Time too "
                    "large\nCurrent Time:\n System: %d %s",
                    system_tv_sec,ctime(&system_tv_sec));
      fprintf(stderr," GPS   : %d %s", gps_tv_sec,ctime(&system_tv_sec));
      fprintf(stderr," Nsec Tdiff: %f Max_error: %f\n",timediff,MAX_ERR);
      fflush(stderr);
    }

    dstat.lastsetsec = dstat.gpssecond;
    dstat.lastsetnsec = dstat.gpsnsecond;
    temp = clock_gettime(CLOCK_REALTIME, &now);
    temp = get_software_time(&dstat.gpssecond, &dstat.gpsnsecond, BASE1);
    dstat.syssecond = now.tv_sec;
    dstat.sysnsecond = now.tv_nsec;

    //caclulate time difference
    timediff = (float)(dstat.syssecond-dstat.gpssecond) +
              ((float)(dstat.sysnsecond-dstat.gpsnsecond))/1e9;
    oldtimediff = timediff;

    if (verbose > 0)
      printf("  * time diff  %13.9f s\n", timediff);

  } else
    if (verbose > 0) printf("get_state(): No need to update system time\n");

  if (dstat.timecompareupdate) {
    now.tv_sec = dstat.nextcomparesec;
    now.tv_nsec = dstat.nextcomparensec;
    dstat.timecompareupdate = 0;
  }

  if (eventupdate) {
    //display GPS time
    dstat.lasttriggersecond = event.tv_sec;
    dstat.lasttriggernsecond = event.tv_nsec;
    eventupdate=0;
  }

    //wait for time compare event
    //sleep.tv_sec=0;
    //sleep.tv_nsec=1000000;
    //nanosleep(&sleep,NULL); 
}
/*-INITIALIZE_DISPLAY---------------------------------------------------*/  
/*
void * initialize_display(void *arg){
  
     //INITIALIZE SCREEN
  initscr();
  noecho();
  raw();
  keypad(stdscr, TRUE);
  halfdelay(1);
  start_color();
  init_pair(1,COLOR_WHITE,COLOR_BLUE);
  init_pair(2,COLOR_RED,COLOR_BLUE);
  init_pair(3,COLOR_GREEN,COLOR_BLUE);
  init_pair(4,COLOR_YELLOW,COLOR_BLUE);
  //set background color to blue
  wbkgd(stdscr,COLOR_PAIR(1));
  //print title
  attron(COLOR_PAIR(4));
  attron(A_BOLD);
  mvprintw(0,0,"SuperDARN GPS Timing Software");
  attroff(COLOR_PAIR(4));
  attron(COLOR_PAIR(4));
  mvprintw(1,0,"for the Symmetricom 560-5908");
  attroff(A_BOLD);
  attroff(COLOR_PAIR(4));
  attron(COLOR_PAIR(1));
  attron(A_BOLD);
  mvprintw(0,64,"R. T. Parris");
  mvprintw(1,64,"GI, UAF 4/5/2007");
  mvprintw(2,0,"--------------------");
  mvprintw(2,20,"--------------------");
  mvprintw(2,40,"--------------------");
  mvprintw(2,60,"--------------------");
  //print the footer
  mvprintw(22,0,"____________________");
  mvprintw(22,20,"____________________");
  mvprintw(22,40,"____________________");
  mvprintw(22,60,"____________________");
  mvprintw(23,0,"  'q' |            'S'           |      'u'       |");
  mvprintw(24,0," QUIT | Sync system clock to GPS | Update Display |");
  attroff(A_BOLD);
  attroff(COLOR_PAIR(1));
  attron(COLOR_PAIR(2));
  //mvprintw(1,0,"Hello World!!!\n");
  attroff(COLOR_PAIR(2));

  attron(A_BOLD);
  mvprintw(3,0,"Hardware status:");
  mvprintw(4,0,"Antenna status: ");
  mvprintw(5,0,"GPS status: ");
  mvprintw(6,0,"Reference status: ");
  mvprintw(7,0,"Phase lock status: ");
  mvprintw(9,0,"Satellite signal:");
  mvprintw(14,0,"Current GPS Position: ");
  mvprintw(18,0,"Mean GPS Position: ");
  mvprintw(3,32,"GPS Time:");
  mvprintw(5,32,"System Time:");
  mvprintw(8,32,"System time error:");
  mvprintw(6,32,"Last sync:");
  mvprintw(16,32,"Last scope sync:");
  mvprintw(17,32,"Time between syncs:");
  mvprintw(9,32,"System drift:");
  mvprintw(10,32,"Mean Drift:");
  mvprintw(19,32,"TCP/IP status:");
  mvprintw(19,61,"Port:", DEFAULT_PORT);
  mvprintw(14,32,"Next GPS trigger:");
  mvprintw(13,32,"Trigger mode:");
  mvprintw(20,32,"Last TCP msg:");
  mvprintw(21,32,"Last msg time:");
  attroff(A_BOLD);
  mvprintw(23,79," ");
  getch();
  endwin();
*/

