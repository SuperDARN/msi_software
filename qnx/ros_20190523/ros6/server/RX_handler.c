#include <pthread.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include "utils.h"
#include "global_server_variables.h"
#include "iniparser.h"

extern int recvsock;
extern int verbose;
extern pthread_mutex_t recv_comm_lock, thread_list_lock;
extern int *ready_state_pointer;
extern struct Thread_List_Item *cp_threads;
extern struct BlackList *blacklist;
extern int *blacklist_count_pointer;
extern struct ClrPwr *latest_clr_fft[MAX_RADARS];
extern struct timeval latest_full_clr_time;
extern int max_freqs,full_clr_wait,full_clr_start,full_clr_end;
extern unsigned long error_count,collection_count;
extern dictionary *Site_INI;

/* compare the average power of two fft structs:
 * return  1 : a > b
 *        -1 : b > a
 *         0 : a == b
 */
int compare_structs(const void *a, const void *b)
{
  t_fft_index *struct_a = (t_fft_index *)a;
  t_fft_index *struct_b = (t_fft_index *)b;

  if (struct_a->apwr > struct_b->apwr) return 1;
  else if (struct_a->apwr == struct_b->apwr) return 0;
  else return -1;
}

void *rx_rxfe_settings(void *arg)
{
  struct DriverMsg msg;
  struct SiteSettings *site_settings;

  site_settings=arg;
  pthread_mutex_lock(&recv_comm_lock);

//  printf("RECV_rxfe_settins\n");
  if (site_settings!=NULL) {
    msg.type=RECV_RXFE_SETTINGS;
    msg.status=1;
    send_data(recvsock, &msg, sizeof(struct DriverMsg));
    send_data(recvsock, &site_settings->ifmode,
              sizeof(site_settings->ifmode));
    send_data(recvsock, &site_settings->rf_settings,
              sizeof(struct RXFESettings));
    send_data(recvsock, &site_settings->if_settings,
              sizeof(struct RXFESettings));
    recv_data(recvsock, &msg, sizeof(struct DriverMsg));
  }                                                                                        
  pthread_mutex_unlock(&recv_comm_lock);
}

void rx_assign_freq(struct ControlProgram *arg)
{
  struct Thread_List_Item *thread_list;
  struct ControlProgram *controlprogram;
  t_fft_index *fft_array=NULL, *detrend_fft_array=NULL, *sub_fft_array=NULL;
  int i,j,acount,k,clean;
  int ncfs, nfreq;
  int blacklist_count=0;
  int numclients=0;
  int best_index=0; 
  int finding_best=0;
  int finding_current=0;
  int assigning_best=0;
  int assigning_current=0;
  int padded_tx_sideband,padded_rx_sideband; //in KHz
  int detrend_enabled=0; 
  int detrend_sideband=0; 
  /* Variables for FFT diagnostic output
  *  This is a temporary diagnostic format and is not be documented.
  *  The format of the file will change based on testing needs. 
  *  TODO: re-implement as part of general diagnostic viewer for the ROS */ 
  int f_fft=-1;
  FILE *ftest=NULL;
  char data_file[255],test_file[255],strtemp[255];
  struct timespec time_now;
  struct tm* time_struct;
  int32 temp;
  float tempf;

  if (verbose > -1) {
    fprintf(stdout, "ASSIGN FREQ: %d %d\n",
          arg->parameters->radar-1,arg->parameters->channel-1);
    fprintf(stdout, " FFT FREQ: %d %d\n",
          arg->clrfreqsearch.start,arg->clrfreqsearch.end);
    fprintf(stdout, " Start Freq: %lf\n",
          arg->state->fft_array[0].freq);
  }

  /* abort if no FFT data */
  if (arg->state->fft_array==NULL) { 
    fprintf(stderr,"Error in assigning frequency function\n"); 
    pthread_exit(NULL);
  }

  /* Check to see if fft diagnostic output is requested */
  test_file[0]='\0';
  strcat(test_file,"/collect.fft");
  /* If requested setup fft diagnostic file */
  ftest=fopen(test_file, "r");
  if (ftest != NULL) {
    fclose(ftest);
    data_file[0]='\0';
    clock_gettime(CLOCK_REALTIME, &time_now);
    time_struct=gmtime(&time_now.tv_sec);
    strcat(data_file, "/data/fft_samples/");
    // data file year
    temp=(int)time_struct->tm_year+1900;
    ltoa(temp, strtemp, 10);
    strcat(data_file, strtemp);
    // data file month
    temp=(int)time_struct->tm_mon;
    ltoa(temp+1,strtemp,10);
    if (temp<10) strcat(data_file, "0");
    strcat(data_file, strtemp);
    // data file day
    temp=(int)time_struct->tm_mday;
    ltoa(temp,strtemp,10);
    if (temp<10) strcat(data_file, "0");
    strcat(data_file, strtemp);
    // data file hour
    temp=(int)time_struct->tm_hour;
    ltoa(temp,strtemp,10);
    if (temp<10) strcat(data_file, "0");
    strcat(data_file, strtemp);
    // data file tens of minutes
    temp=(int)time_struct->tm_min;
    temp=(int)(temp/10);
    ltoa(temp,strtemp,10);
    strcat(data_file, strtemp);
    if (temp<10) strcat(data_file, "0");
    // data file suffix
    strcat(data_file, ".");
    temp=arg->parameters->radar-1;
    ltoa(temp,strtemp,10);
    strcat(data_file, strtemp);
    strcat(data_file, ".fft");
    f_fft = open(data_file, O_WRONLY|O_CREAT|O_NONBLOCK|O_APPEND,
                            S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  }

  /* Write Diagnostic header info */ 
  if (f_fft >= 0) {
    temp=(int)time_struct->tm_year+1900;
    write(f_fft, &temp, sizeof(int32));
    temp=(int)time_struct->tm_mon+1;
    write(f_fft, &temp, sizeof(int32));
    temp=(int)time_struct->tm_mday;
    write(f_fft, &temp, sizeof(int32));
    temp=(int)time_struct->tm_hour;
    write(f_fft, &temp, sizeof(int32));
    temp=(int)time_struct->tm_min;
    write(f_fft, &temp, sizeof(int32));
    temp=(int)time_struct->tm_sec;
    write(f_fft, &temp, sizeof(int32));
    temp=(int)time_now.tv_nsec;
    write(f_fft, &temp, sizeof(int32));

    temp=arg->parameters->tbeam;
    write(f_fft, &temp, sizeof(int32));
    temp=arg->parameters->tfreq;
    write(f_fft, &temp, sizeof(int32));
    temp=arg->parameters->radar-1;
    write(f_fft, &temp, sizeof(int32));
    temp=arg->parameters->channel-1;
    write(f_fft, &temp, sizeof(int32));
    tempf=arg->parameters->baseband_samplerate;
    write(f_fft, &tempf, sizeof(float));

    // CLRFreqPRM
    temp=arg->clrfreqsearch.start;
    write(f_fft, &temp, sizeof(int32));
    temp=arg->clrfreqsearch.end;
    write(f_fft, &temp, sizeof(int32));
    tempf=arg->clrfreqsearch.filter_bandwidth;
    write(f_fft, &tempf, sizeof(float));
  }         


  /* Note here from Jeff, the FFT is typically performed on N = 2^n samples
   * so the size of the FFT window is typically larger than the band from
   * which we are selecting frequencies from.  This is however not a garuntee.
   * Each receiver driver is responsible for handling the details of how the
   * fft is performed based on the bandwidth limitation imposed on the
   * receiver hardware. It cannot be assumed that the receiver driver was able
   * to return a clear frequency search encompassing the requested band.  
   * 
   * For current operational purposes a receiver driver is required to send 
   * back an FFT in 1 kHz bins (see the clear frequency search function for
   * specifics). Both the gc214 and the gc31X drivers comply with this
   * implicit assumption */

  /* malloc and copy data into FFT array */
  fft_array = (t_fft_index *) malloc(sizeof(t_fft_index) * arg->state->N);
  memcpy(fft_array,arg->state->fft_array,arg->state->N*sizeof(t_fft_index));

  /* the padded sidebands account for the finite bandwidths associated with
   * transmitters and receivers for a requested pulse sequence.  
   * Minimum sideband is currently set to 0 kHz.
   * These values will be used for two things:
   * 1) noise calculation for a pulse sequence relavent bandwidth   
   * 2) impose adequate spacing for frequency assignment in multiple ctrlprgrm 
   *     operation scenarios to avoid cross-contamination of signals
   *
   * Note that it is padded_rx_sideband which is most critical as it is
   * anticipated that receiver bandwidth can be higher than transmit when
   * using oversampling modes. There is room for improvement here...
   * TODO: 1.) include a parameter which allows a controlprogram to explicitly
   *           override the fft 
   *       2.) smoothing separate from the sideband use for blacklisting an
   *           assigned frequency window
   */ 

  /* padded_tx_sideband defined but unused currently */
  padded_tx_sideband =
        MAX(2*ceil(arg->parameters->baseband_samplerate/1000.0),0);
  /* TODO: optimize padded_rx_sideband  to account for over sampling and
   *       wide-band modes*/
  padded_rx_sideband =
        MAX(2*ceil(arg->parameters->baseband_samplerate/1000.0),0);

  if (verbose > -1) {
    fprintf(stderr," Padded Rx Side Band: %d [kHz]\n",padded_rx_sideband);
    fprintf(stderr, "%d frequencies in fft \n",arg->state->N);
    fprintf(stderr, "  freq[%8d]: %8.3lf\n",0,(double)fft_array[0].freq);
    fprintf(stderr, "  freq[%8d]: %8.3lf\n",
            arg->state->N-1,(double)fft_array[arg->state->N-1].freq);
    fflush(stderr);
  } 
  /* JDS: If site.ini is configured to use the detrend step then use it.
   * Default to disabled if not defined in site.ini file 
   */
  detrend_enabled = iniparser_getboolean(Site_INI,
                                   "site_settings:use_clr_detrend",0);
  if (detrend_enabled) {
    if (verbose > -1) 
      fprintf(stderr," Detrending Enabled\n");
    /* Get the sidebandwidth to use for detrending. Default to 50 if not set */
    detrend_sideband = iniparser_getint(Site_INI,
                                    "site_settings:detrend_sideband",50);
    /* Copy the fft array. Need to keep the detrended value to add back later
     * to get the noise power right */ 
    detrend_fft_array =
                (t_fft_index *)malloc(sizeof(t_fft_index)*arg->state->N);
    memcpy(detrend_fft_array,fft_array,arg->state->N*sizeof(t_fft_index));
    for (i=0; i<arg->state->N; i++) {  /* run through all FFT samples */
      detrend_fft_array[i].apwr = 0;
      acount = 0;
      for (j=-detrend_sideband; j<=detrend_sideband; j++) {
        if ( ((i+j) >= 0) && ((i+j) < arg->state->N) ) {
          detrend_fft_array[i].apwr += detrend_fft_array[i+j].pwr;
          acount++;
        }
      }
      detrend_fft_array[i].apwr =
                  (double)detrend_fft_array[i].apwr/(double)acount;
      /* Save the detrended power to add back later */  
      fft_array[i].detrend=detrend_fft_array[i].apwr;
      /* Subtract off the detrended power from the original fft power */  
      fft_array[i].pwr=detrend_fft_array[i].pwr-detrend_fft_array[i].apwr;
    }
    if (detrend_fft_array != NULL) free(detrend_fft_array);
    detrend_fft_array = NULL;
  } else {
    if (verbose > -1) 
      fprintf(stderr," Detrending Disabled\n");
    for (i=0; i<arg->state->N; i++) fft_array[i].detrend = 0;
  }

  /* Now do a less agressive narrow band smoothing over the optionally
   * detrended data */

  /* Calculate power over receiver bandwidth */
  /* Take a running average of power using 2*padded_rx_bandwith+1
                                            number of points.*/

  /* SGS: Note here that padded_rx_sideband is specified in kHz above and
   * and the loop over the filter_bandwidth assumes that frequencies
   * in the FFT array are separated by 1 kHz exactly. */
  for (i=0; i<arg->state->N; i++) {  /* run through all FFT samples */
    fft_array[i].apwr = 0;
    acount = 0;
    for (j=-padded_rx_sideband; j<=padded_rx_sideband; j++) {
      if ( ((i+j) >= 0) && ((i+j) < arg->state->N) ) {
        fft_array[i].apwr += fft_array[i+j].pwr;
        acount++;
      }
    }

    if (acount > 2*padded_rx_sideband) {
      fft_array[i].apwr = (double)fft_array[i].apwr/(double)acount;
    } else {
      /* Bias against selecting a frequency too near the edge of the search
       * band. Set the average pwr artificially high so that index is
       * selected against in qsort.
       */
      fft_array[i].apwr = 1E10;
    }
  }

  /* Put input fft info into diagnostic file */
  if (f_fft>=0) {
    //sidebands in use
    temp=padded_rx_sideband;
    write(f_fft, &temp, sizeof(int32));
    temp=padded_tx_sideband;
    write(f_fft, &temp, sizeof(int32));
    //ControlState
    temp=arg->state->N;
    write(f_fft, &temp, sizeof(int32));
    for (i=0;i<arg->state->N;i++) {
      temp=arg->state->fft_array[i].index;
      write(f_fft, &temp, sizeof(int32));
    }
    for (i=0;i<arg->state->N;i++) {
      tempf=arg->state->fft_array[i].freq;
      write(f_fft, &tempf, sizeof(float));
    }
    for (i=0;i<arg->state->N;i++) {
      tempf=arg->state->fft_array[i].pwr;
      write(f_fft, &tempf, sizeof(float));
    }
    for (i=0;i<arg->state->N;i++) {
      tempf=arg->state->fft_array[i].apwr;
      write(f_fft, &tempf, sizeof(float));
    }
  }

  /* SGS: new logic
   * limit the array of frequencies to the band of interest which is
   * defined by the clear frequency search bandwidth. */

  /* note that the assumption here is that this band is an integer
   * number of kHz and that the spacing between frequencies is 1 kHz.
   * perhaps this a bad assumption to make... */

  ncfs = arg->clrfreqsearch.end - arg->clrfreqsearch.start;
  sub_fft_array = (t_fft_index *)malloc(sizeof(t_fft_index)*ncfs);

  j = 0;
  /* loop over all FFT frequencies and copy only those in the clrfreqsearch
   * band to the sub array */
  for (i=0; i<arg->state->N; i++) {
    if ( (fft_array[i].freq <= arg->clrfreqsearch.end) &&
        (fft_array[i].freq >= arg->clrfreqsearch.start) ) {
      if (j >= ncfs) {  /* this should never happend, but good to check... */
        /* Jeff, not sure what level of verbosity this should be ... */
        if (verbose > -1)
          fprintf(stderr, "Too many frequencies in "
                          "clrfreqsearch band: %d\n", j);
        j = ncfs - 1; /* make sure there is no overstepping array bounds */
      }
      /* fill the elements of the sub__fft_array from the fft_array */
      sub_fft_array[j].freq = fft_array[i].freq;
      sub_fft_array[j].pwr = fft_array[i].pwr;
      sub_fft_array[j].apwr = fft_array[i].apwr;
      sub_fft_array[j].index = fft_array[i].index;
      sub_fft_array[j].detrend = fft_array[i].detrend;
                        j++;
    }
  }

  if (verbose > -1)
    fprintf(stderr, "%d frequencies in clrfreqsearch band\n", j);

  ncfs = j; /* note that these should be the same, but perhaps should check */

  /* this should be a really high level of verbosity */
  if (verbose > 10) {
    for (i=0; i<ncfs; i++) {
      fprintf(stderr, "%5d\n", sub_fft_array[i]);
    }
  }
  /* We now have an array of only the frequencies in the clrfreqsearch
   * band and must now reduce the array further by eliminating frequencies
   * that are in restricted frequency bands. */

  /* Let's resuse the fft_array instead of mallocing something else and
   * check every band in the restricted list */
  for (i=0; i<arg->state->N; i++) {
      fft_array[i].freq=0.0;
      fft_array[i].pwr=1E10;
      fft_array[i].apwr=1E10;
      fft_array[i].index=0;
      fft_array[i].detrend=0;
  }

  /*
  * Let's check the frequency blacklist...for science!!!!!
  */  

  if (blacklist != NULL) {  
    /* Set the counter to the unmodified blacklist count */
    blacklist_count = *blacklist_count_pointer;
    if (verbose > -1) fprintf(stderr,"%d %d :: Filling backlist %d\n",
                              arg->parameters->radar,arg->parameters->channel,
                              blacklist_count); 
    /*
     * First add all other control program's assigned frequencies to the
     * blacklist priority is like golf: lower number wins
     */
    thread_list=cp_threads;
    while (thread_list != NULL) {
      controlprogram=thread_list->data;
      if (controlprogram != arg) {                   
        if (verbose>1)
          fprintf(stderr,"  %d %d :: Checking Control Program :: %p  :: %d "
                          "%d\n", arg->parameters->radar,
                          arg->parameters->channel,controlprogram,
                          controlprogram->parameters->radar,
                          controlprogram->parameters->channel);
        if (controlprogram->active!=0) {
          //if (controlprogram->parameters->priority < arg->parameters->priority){
          if (blacklist_count < (Max_Control_THREADS+*blacklist_count_pointer)){
            /* place controlprogram's assigned frequency on the blacklist */
            blacklist[blacklist_count].start=controlprogram->state->best_assigned_freq-controlprogram->state->rx_sideband;
            blacklist[blacklist_count].end=controlprogram->state->best_assigned_freq+controlprogram->state->rx_sideband;
            blacklist[blacklist_count].program=(unsigned int)controlprogram;
            if (verbose>1)
              fprintf(stderr,"  %d %d :: Adding backlist :: %d %d :  %d %d\n",
                      arg->parameters->radar,arg->parameters->channel,
                      controlprogram->parameters->radar,controlprogram->parameters->channel,
                      blacklist[blacklist_count].start,blacklist[blacklist_count].end); 
            blacklist_count++;
            if (blacklist_count >= (Max_Control_THREADS+*blacklist_count_pointer)) blacklist_count=(Max_Control_THREADS+*blacklist_count_pointer); 
          }
              //} 
        }
      }
      thread_list=thread_list->prev;
    }
    if (verbose > -1 ) {
      fprintf(stderr,"%d %d :: Done Filling backlist %d\n",
              arg->parameters->radar,arg->parameters->channel,blacklist_count);
      fprintf(stderr,"Current blacklist:\n");
      for (k=0; k<blacklist_count; k++)
        fprintf(stderr," %8d %8d : %p\n",blacklist[k].start,blacklist[k].end,
                        blacklist[k].program);
    }

    /* Put the updated blacklist information into the diagnostic file */
    if (f_fft >= 0) {
      temp=blacklist_count;
      write(f_fft, &temp, sizeof(int32));     
      for (k=0; k<blacklist_count; k++) {
        temp=blacklist[k].start;
        write(f_fft, &temp, sizeof(int32));
      }
      for (k=0; k<blacklist_count; k++) {
        temp=blacklist[k].end;
        write(f_fft, &temp, sizeof(int32));
      }
      for (k=0; k<blacklist_count; k++) {
        temp=blacklist[k].program;
        write(f_fft, &temp, sizeof(int32));
      }
    }

    /* Now let's select the subset of fft frequencies not in the updated
        blacklist */
    j = 0;
    for (i=0; i<ncfs-1; i++) {
      clean = 1;
      for (k=0; k<blacklist_count; k++) {
        if ((sub_fft_array[i].freq <= blacklist[k].end) &&
            (sub_fft_array[i].freq >= blacklist[k].start)) {
          clean = 0;    /* frequency is in a restricted band, don't bother */
          break;
        }
      }

      if (clean) {  /* this is a good one */
              fft_array[j].freq= sub_fft_array[i].freq;
              fft_array[j].pwr= sub_fft_array[i].pwr;
              fft_array[j].apwr= sub_fft_array[i].apwr;
              fft_array[j].index= sub_fft_array[i].index;
              fft_array[j].detrend= sub_fft_array[i].detrend;
        j++;
      }
    }

    nfreq = j;  /* these are the number of allowed frequencies */

    if (verbose > 1)
      fprintf(stderr, "%d frequencies that are allowed\n", nfreq);

    /* now let's get an array that is the proper size and transfer
     * everything to that array */
    if (sub_fft_array!=NULL) free(sub_fft_array);
    sub_fft_array=NULL; 
    sub_fft_array = (t_fft_index *)malloc(sizeof(t_fft_index)*nfreq);
    memcpy(sub_fft_array,fft_array,nfreq*sizeof(t_fft_index));
    if (fft_array != NULL) free(fft_array);
    fft_array=NULL; 
                    
  } else {
    /* I am assuming that if blacklist is NULL there is no restricted
     * frequencies (which is unlikely) or there was a problem with reading
     * the restrict.dat file */
    nfreq = ncfs;

    if (fft_array!=NULL) free(fft_array);
    fft_array=NULL;
  }

  /* The only array we now have is sub_fft_array and it contains only those
   * frequencies that are in the clear frequency search band and not in
   * any of the restricted frequency bands */

  /* sort the array of allowable frequencies */
  qsort(sub_fft_array, nfreq, sizeof(sub_fft_array[0]), compare_structs);
  if (detrend_enabled) {
    /* Add back the detrended value to get the correct noise value */
    for (i=0;i<nfreq;i++) {
      sub_fft_array[i].pwr=sub_fft_array[i].pwr+sub_fft_array[i].detrend;
      sub_fft_array[i].apwr=sub_fft_array[i].apwr+sub_fft_array[i].detrend;
    }
  }
  /* Put sorted and available frequencies into diagnostic file */
  if (f_fft >= 0) {
    temp = nfreq;
    write(f_fft, &temp, sizeof(int32));
    for (i=0;i<nfreq;i++) {
      temp=sub_fft_array[i].index;
      write(f_fft, &temp, sizeof(int32));
    }
    for (i=0;i<nfreq;i++) {
      tempf=sub_fft_array[i].freq;
      write(f_fft, &tempf, sizeof(float));
    }
    for (i=0;i<nfreq;i++) {
      tempf=sub_fft_array[i].pwr;
      write(f_fft, &tempf, sizeof(float));
    }
    for (i=0;i<nfreq;i++) {
      tempf=sub_fft_array[i].apwr;
      write(f_fft, &tempf, sizeof(float));
    }
  }


  if (nfreq > 0) {
    /* Just here for logging purposes */
    if (verbose > -1) { 
      fprintf(stderr," Highest 5 freqs: \n");
      for (i = nfreq-1; i > nfreq-6; i--) {
        if (i >= 0) { 
          fprintf(stderr,"%d: freq = %8.3lf, pwr = %8.3lf apwr = %8.3lf\n",
                  i,sub_fft_array[i].freq,sub_fft_array[i].pwr,
                  sub_fft_array[i].apwr);
        } else {
          fprintf(stderr,"%d: Not enough valid frequencies in fft %d\n",
                  i,nfreq);
        }
      }
      fprintf(stderr," Lowest 5 freqs: \n");
      for (i = 0; i< 5;i++) {
        if (i < nfreq) { 
          fprintf(stderr,"%d: freq = %8.3lf, pwr = %8.3lf apwr = %8.3lf\n",
                  i,sub_fft_array[i].freq,sub_fft_array[i].pwr,
                  sub_fft_array[i].apwr);
        } else {
          fprintf(stderr,"%d: Not enough valid frequencies in fft %d\n",
                  i,nfreq);
        }
      }
    } 

    /*
     * Now set best available frequency from the sorted sub_fft_array as this
     *  controlprogram's best assigned frequency   
     */
    best_index=0; 
    arg->state->current_assigned_freq=sub_fft_array[best_index].freq;
    arg->state->current_assigned_noise=sub_fft_array[best_index].apwr;
    arg->state->best_assigned_freq=sub_fft_array[best_index].freq;
    arg->state->best_assigned_noise=sub_fft_array[best_index].apwr;

    if (verbose > 1)
      fprintf(stderr,"%lf best frequency: %d assigned frequency: %d\n",
              sub_fft_array[best_index].freq, arg->state->best_assigned_freq,
              arg->state->current_assigned_freq);

    arg->state->tx_sideband=padded_tx_sideband;
    arg->state->rx_sideband=padded_rx_sideband;
    /*
     * TODO: check to see if any controlprogram has a currently assigned
     *       frequency that conflicts with the best freq   
     * priority :high number wins
     * Kick lower priority control programs to give up their frequency.
     */

  } else {
    /* JDS QUESTION: Is this the best fallback when no valid frequency is
     * available?
     * A simple single _default_ for all controlprograms is not valid in a
     * multi channel configuration
     */ 
    if (verbose >- 1)
      fprintf(stderr,"No valid frequencies Setting best available frequency "
                     "to zero with very high noise\n");
    arg->state->current_assigned_freq=0;
    arg->state->current_assigned_noise=1E10;
    arg->state->best_assigned_freq=0;
    arg->state->best_assigned_noise=0;
  }

  /* Put assignment data into diagnostic file */
  if (f_fft >= 0){
    temp=arg->state->current_assigned_freq;
    write(f_fft, &temp, sizeof(int32));
    tempf=arg->state->current_assigned_noise;
    write(f_fft, &tempf, sizeof(float));
    temp=arg->state->best_assigned_freq;
    write(f_fft, &temp, sizeof(int32));
    tempf=arg->state->best_assigned_noise;
    write(f_fft, &tempf, sizeof(float));
    close(f_fft);
  }

  /*
   * Final Clean up
   */
  if (sub_fft_array != NULL) free(sub_fft_array);
  sub_fft_array=NULL;
  pthread_exit(NULL);
}

void rx_exit(void *arg)
{
  int *sockfd = (int *)arg;

  pthread_t tid;
  /* get the calling thread's ID */
  tid = pthread_self();
  /* print where the thread was in its search when it was cancelled */
}

void *rx_end_cp(struct ControlProgram *arg)
{
  struct DriverMsg msg;

  pthread_mutex_lock(&recv_comm_lock);

  if (arg!=NULL) {
    if (arg->state->pulseseqs[arg->parameters->current_pulseseq_index]!=NULL) {
      msg.type=RECV_CtrlProg_END;
      msg.status=1;
      send_data(recvsock, &msg, sizeof(struct DriverMsg));
      send_data(recvsock, arg->parameters, sizeof(struct ControlPRM));
    }
  }
  pthread_mutex_unlock(&recv_comm_lock);
  pthread_exit(NULL);
}

void *rx_ready_cp(struct ControlProgram *arg)
{
  struct DriverMsg msg;

  pthread_mutex_lock(&recv_comm_lock);

  if (arg!=NULL) {
    if (arg->state->pulseseqs[arg->parameters->current_pulseseq_index]!=NULL) {
      msg.type=RECV_CtrlProg_READY;
      msg.status=1;
      send_data(recvsock, &msg, sizeof(struct DriverMsg));
      send_data(recvsock, arg->parameters, sizeof(struct ControlPRM));
      recv_data(recvsock, &msg, sizeof(struct DriverMsg));
    } 
  }
  pthread_mutex_unlock(&recv_comm_lock);
  pthread_exit(NULL);
}

void *rx_pretrigger(void *arg)
{
  struct DriverMsg msg;

  pthread_mutex_lock(&recv_comm_lock);

  msg.type=RECV_PRETRIGGER;
  msg.status=1;
  if (verbose > 0) printf("RX: RECV_PRETRIGGER: Send msg\n");
  send_data(recvsock, &msg, sizeof(struct DriverMsg));
  recv_data(recvsock, &msg, sizeof(struct DriverMsg));
  pthread_mutex_unlock(&recv_comm_lock);
  if (verbose > 0) {
    printf("RX: RECV_PRETRIGGER: end\n");
    fflush(stdout);
  }
  pthread_exit(NULL);
}

void *rx_posttrigger(void *arg)
{
  struct DriverMsg msg;

  pthread_mutex_lock(&recv_comm_lock);

  msg.type=RECV_POSTTRIGGER;
  msg.status=1;
  send_data(recvsock, &msg, sizeof(struct DriverMsg));
  recv_data(recvsock, &msg, sizeof(struct DriverMsg));
  pthread_mutex_unlock(&recv_comm_lock);
  pthread_exit(NULL);
}

void *rx_cp_get_data(struct ControlProgram *arg)
{
  struct DriverMsg msg;
  struct timeval t0,t1,t3;
  char *timestr;
  int rval,ready_state;
  char shm_device[80];
  int shm_fd;
  int r,c,b;
  unsigned long wait_elapsed;
  double error_percent=0;
  int error_flag=0;

  if (collection_count==ULONG_MAX) {
    error_count=0;
    collection_count=0;
  }

  if (verbose > 1) {
    fprintf(stderr, "s: %d; r: %d; c: %d\n", arg->radarinfo->site,
                  arg->radarinfo->radar, arg->radarinfo->channel);
    fprintf(stderr, "** error_flag %d\n", error_flag);
  }

  if (arg != NULL) {
    if (arg->state != NULL) {
      gettimeofday(&t0,NULL);
      while (arg->state->ready != 0) {
        gettimeofday(&t3,NULL);
        if (verbose > 10)
          fprintf(stderr,"RECV::Waiting in RX_ctrlprog_get_data(): %s\n",
                  ctime(&t3.tv_sec));
        wait_elapsed =  (t3.tv_sec-t0.tv_sec)*1E6;
        wait_elapsed += t3.tv_usec-t0.tv_usec;
        if (wait_elapsed > 10*1E6) {
          error_flag = -101; 
          fprintf(stderr,"RECV::Wait too long in RX_ctrlprog_get_data()\n");
          break;  
        } else {
          usleep(1);
        }
      } 
      if (verbose > 1) fprintf(stderr, "** error_flag %d\n", error_flag);
      pthread_mutex_lock(&recv_comm_lock);
      collection_count++;
      if (verbose > 1) fprintf(stderr, "** error_flag %d\n", error_flag);

      if (error_flag == 0) {
        if (verbose > 1)
          fprintf(stderr, "RECV::No error in RX_ctrlprog_get_data()\n");
        arg->data->samples=arg->parameters->number_of_samples;
        if (arg->main != NULL)
          munmap(arg->main,sizeof(unsigned int)*arg->data->samples);
        if (arg->back != NULL)
          munmap(arg->back,sizeof(unsigned int)*arg->data->samples);

        msg.type=RECV_GET_DATA;
        msg.status=1;
        send_data(recvsock, &msg, sizeof(struct DriverMsg));
        send_data(recvsock, arg->parameters, sizeof(struct ControlPRM));
        recv_data(recvsock,&arg->data->status,sizeof(arg->data->status));
      } else {
        if (verbose > 1)
          fprintf(stderr, "RECV::Error %d in RX_ctrlprog_get_data()\n",
                          error_flag);
        arg->data->status = error_flag;
        arg->data->samples = 0;
      }

      if (arg->data->status == 0) {
        if (verbose > 1)
          fprintf(stderr, "RECV:: status good in RX_ctrlprog_get_data()\n");
        //printf("RECV: GET_DATA: status good\n");
        recv_data(recvsock,&arg->data->shm_memory,
                  sizeof(arg->data->shm_memory));
        recv_data(recvsock,&arg->data->frame_header,
                  sizeof(arg->data->frame_header));
        recv_data(recvsock,&arg->data->bufnum,sizeof(arg->data->bufnum));
        recv_data(recvsock,&arg->data->samples,sizeof(arg->data->samples));
        recv_data(recvsock,&arg->main_address,sizeof(arg->main_address));
        recv_data(recvsock,&arg->back_address,sizeof(arg->back_address));
        //printf("RECV: GET_DATA: data recv'd\n");
        r = arg->parameters->radar-1;
        c = arg->parameters->channel-1;
        b = arg->data->bufnum;

        //printf("RECV: GET_DATA: samples %d\n",arg->data->samples);
        //printf("RECV: GET_DATA: frame header %d\n",arg->data->frame_header);
        //printf("RECV: GET_DATA: shm flag %d\n",arg->data->shm_memory);
        if (arg->data->shm_memory) {
          //printf("RECV: GET_DATA: set up shm memory space\n");
          sprintf(shm_device,"/receiver_main_%d_%d_%d",r,c,b);
          shm_fd=shm_open(shm_device,O_RDONLY,S_IRUSR | S_IWUSR);
          if (shm_fd == -1) fprintf(stderr,"shm_open error\n");              
          arg->main=mmap(0,sizeof(unsigned int)*arg->data->samples,
                          PROT_READ,MAP_SHARED,shm_fd,0);
          close(shm_fd);
          sprintf(shm_device,"/receiver_back_%d_%d_%d",r,c,b);
          shm_fd=shm_open(shm_device,O_RDONLY,S_IRUSR | S_IWUSR);
          arg->back=mmap(0,sizeof(unsigned int)*arg->data->samples,
                          PROT_READ,MAP_SHARED,shm_fd,0);
          close(shm_fd);
          //printf("RECV: GET_DATA: end set up shm memory space\n");

        } else {
#ifdef __QNX__
          //printf("RECV: GET_DATA: set up non-shm memory space\n");
          arg->main = mmap(0, sizeof(unsigned int)*arg->data->samples, 
                            PROT_READ|PROT_NOCACHE, MAP_PHYS, NOFD, 
                            arg->main_address+sizeof(unsigned int)*
                              arg->data->frame_header);
                              //arg->main_address);
          arg->back = mmap(0, sizeof(unsigned int)*arg->data->samples, 
                            PROT_READ|PROT_NOCACHE, MAP_PHYS, NOFD, 
                            arg->back_address+sizeof(unsigned int)*
                              arg->data->frame_header);
                              //arg->back_address);
#else
          arg->main=NULL;
          arg->back=NULL;
#endif
          //printf("RECV: GET_DATA: end set up non-shm memory space\n");
        }

      } else { //error occurred
        fprintf(stderr, "RECV:: status bad %d in RX_ctrlprog_get_data()\n",
                arg->data->status);
        error_count++;
        error_percent=(double)error_count/(double)collection_count*100.0;
        arg->data->samples=0;
        arg->main=NULL;
        arg->back=NULL;
        gettimeofday(&t1,NULL);
        fprintf(stderr,"RECV::GET_DATA: Bad Status: %d Time: %s",
                        arg->data->status, ctime(&t1.tv_sec));
        fprintf(stderr,"  Collected: %ld  Errors: %ld  Percentage: %lf\n",
                        collection_count,error_count,error_percent);
        fflush(stderr);
      }

      if (error_flag == 0) {
        //printf("RECV: GET_DATA: recv RosMsg\n");
        recv_data(recvsock, &msg, sizeof(struct DriverMsg));
      }
      //printf("RECV: GET_DATA: unlock comm lock\n");
      pthread_mutex_unlock(&recv_comm_lock);
    }
  }

  pthread_exit(NULL);
}

void *rx_clrfreq(struct ControlProgram *arg)
{
  struct DriverMsg msg;
  struct timeval t0;
  struct CLRFreqPRM clrfreq_parameters;
  unsigned long wait_elapsed;
  int i,j,r,bandwidth,index,min_index,max_index,radars,length;
  int temp1,temp2;
  int i_min[MAX_CHANNELS*MAX_RADARS];
  double m_min[MAX_CHANNELS*MAX_RADARS];
  double best_freq[MAX_CHANNELS*MAX_RADARS];
  double best_bandwidth_min[MAX_CHANNELS*MAX_RADARS];
  double *pwr=NULL;
  int start,end,centre,acount;
  int clr_needed=0;

  //printf("CLRFREQ: %d %d\n",arg->parameters->radar-1,
  //                          arg->parameters->channel-1);
  //printf(" FFT FREQ: %d %d\n",arg->clrfreqsearch.start,
  //                            arg->clrfreqsearch.end);
  pthread_mutex_lock(&recv_comm_lock);
  gettimeofday(&t0,NULL);

  /* Check to see if Clr search request falls within full scan parameters */
  if ((arg->clrfreqsearch.start >= full_clr_start) &&
      (arg->clrfreqsearch.end <= full_clr_end)) {
    /* Check to see if Full Clr was done recently enough to be useful*/
    wait_elapsed=t0.tv_usec-latest_full_clr_time.tv_usec;
    wait_elapsed*=1E6;
    wait_elapsed+=t0.tv_sec-latest_full_clr_time.tv_sec;
    if (wait_elapsed < full_clr_wait*1E6) {
      //New Full clr search not needed
      clr_needed=0;
    } else {
      //New Full clr search needed
      clr_needed=1;
    }
  } else {
    //Do not perform Full Search
    clr_needed=1;
  }

  switch (clr_needed) {
   case 0:
     break; 
/*
   case 2:
    msg.type=FULL_CLRFREQ;
    msg.status=1;
    send_data(recvsock, &msg, sizeof(struct DriverMsg));
    recv_data(recvsock, &msg, sizeof(struct DriverMsg));
    recv_data(recvsock, &radars, sizeof(radars));
    for(r=0;r<radars;r++) {
      recv_data(recvsock, &msg, sizeof(struct DriverMsg));
      recv_data(recvsock, &start, sizeof(start));
      recv_data(recvsock, &end, sizeof(end));
      recv_data(recvsock, &length, sizeof(end));
      min_index=(start-full_clr_start);
      max_index=(end-full_clr_start);
      if(pwr!=NULL) free(pwr); 
      pwr=NULL;
      pwr = (double*) malloc(sizeof(double) * length);
      recv_data(recvsock, pwr, sizeof(double)* length);
      for(i=0;i<length;i++) {
        index=min_index+i; 
        if ((index >= 0)&& (index < max_freqs)){
          latest_clr_fft[r][index].pwr=pwr[i];
        } 
      }
    }
    recv_data(recvsock, &msg, sizeof(struct DriverMsg));
    break;   
*/
   case 1:
    r=arg->parameters->radar-1;
    msg.type=RECV_CLRFREQ;
    msg.status=1;
    send_data(recvsock, &msg, sizeof(struct DriverMsg));
    send_data(recvsock, &arg->clrfreqsearch, sizeof(struct CLRFreqPRM));
    send_data(recvsock, arg->parameters, sizeof(struct ControlPRM));
    recv_data(recvsock, &arg->clrfreqsearch, sizeof(struct CLRFreqPRM));
    if(verbose > -1 ) printf("  final search parameters\n");  
    if(verbose > -1 ) printf("  start: %d\n",arg->clrfreqsearch.start);        
    if(verbose > -1 ) printf("  end: %d\n",arg->clrfreqsearch.end);    
    if(verbose > -1 ) printf("  nave:  %d\n",arg->clrfreqsearch.nave); 
    recv_data(recvsock, &arg->state->N, sizeof(int));
    if(verbose > -1 ) printf("  N:  %d\n",arg->state->N); 
    if(pwr!=NULL) free(pwr); 
    pwr=NULL;
    pwr = (double*) malloc(sizeof(double) * arg->state->N);
    recv_data(recvsock, pwr, sizeof(double)*arg->state->N);
    recv_data(recvsock, &msg, sizeof(struct DriverMsg));
    centre=(arg->clrfreqsearch.end+arg->clrfreqsearch.start)/2;
    bandwidth=arg->state->N;
    start=centre-arg->state->N/2;
    end=centre+arg->state->N/2;
    for (i=0; i< arg->state->N;i++) {
      index=(start+i-full_clr_start);
      if ((index >= 0) && (index < max_freqs))
        latest_clr_fft[r][index].pwr=pwr[i];
    }
    break;
  } // END of Switch 

  /* Fill the requested client data */
  if (arg->state->fft_array!=NULL) free(arg->state->fft_array);
  arg->state->fft_array=NULL;
  arg->state->fft_array = (t_fft_index *)malloc(sizeof(t_fft_index)*
                                                arg->state->N);
  for (i=0; i<arg->state->N;i++) {
    arg->state->fft_array[i].apwr=0;
    arg->state->fft_array[i].index = i;
    arg->state->fft_array[i].freq = start+i;
    index=arg->state->fft_array[i].freq-full_clr_start;
    if ((index >=0) && (index < max_freqs)) {
      arg->state->fft_array[i].pwr=latest_clr_fft[r][index].pwr;
    } else {
      arg->state->fft_array[i].pwr=1E6;
    }
  }

  //printf(" Start Freq: %lf\n",arg->state->fft_array[0].freq);
  if (pwr!=NULL) free(pwr);
  pwr=NULL;
  pthread_mutex_unlock(&recv_comm_lock);
  pthread_exit(NULL);
}

