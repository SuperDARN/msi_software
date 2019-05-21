
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include "control_program.h"
#include "global_server_variables.h"
#include "utils.h"
#include "coordination_handler.h"
#include "dio_handler.h"
#include "RX_handler.h"
#include "timing_handler.h"
#include "dds_handler.h"
#include "log_handler.h"
#include "settings_handler.h"
#include "priority.h"
#include "rtypes.h"
#include "iniparser.h"

extern pthread_mutex_t cp_list_lock,exit_lock,clr_lock;
extern char *cp_list_lock_buff,*exit_lock_buffer;
extern void *radar_channels[MAX_RADARS*MAX_CHANNELS];
extern struct Thread_List_Item *cp_threads;
extern struct TRTimes bad_transmit_times;
extern int verbose;
extern struct tx_status txstatus[MAX_RADARS];
extern struct SiteSettings site_settings;
extern dictionary *Site_INI;

int unregister_radar_channel(struct ControlProgram *cp)
{
  int i;
  int status=0;

  if (cp != NULL) {
    for (i=0; i<MAX_RADARS*MAX_CHANNELS; i++) {
      if (radar_channels[i] == (void *)cp) {
        printf("Unregistering: %d :: cp: %p\n", i,cp);
        status++;
        radar_channels[i] = NULL;
        cp->parameters->radar   = 0;
        cp->parameters->channel = 0;
      }
    }
  }

  return status;
}

struct ControlProgram* find_reg_cp_by_rchan(int rad, int chan)
{
  int i,r,c;
  struct ControlProgram *cp=NULL;

  for (i=0; i<MAX_RADARS*MAX_CHANNELS; i++) {
    r = (i / MAX_CHANNELS) + 1;
    c = (i % MAX_CHANNELS) + 1;
    if (rad==r && chan==c) {
      cp = radar_channels[i]; 
      break;
    }
  }

  return cp;
}

struct ControlPRM cp_fill_params(struct ControlProgram *ctrlprog)
{
  struct ControlPRM ctrl_params;      
  struct ControlProgram *cp, *best[MAX_RADARS];      
  int priority=99; //Lowest priority wins-- its like golf
  int r,c;

  if (TX_BEAM_PRIORITY | RX_BEAM_PRIORITY | TX_FREQ_PRIORITY |
                         RX_FREQ_PRIORITY |TIME_SEQ_PRIORITY) {  
    for (r=1; r<=MAX_RADARS; r++) {
      priority = 99;
      best[r-1] = NULL; 
      for (c=1; c<=MAX_CHANNELS; c++) {
        cp = find_reg_cp_by_rchan(r,c);
        if (cp != NULL) {
          //if (cp->active != 0) {    // SGS commented out in FH code
            if (cp->parameters != NULL) {
              if (cp->parameters->priority < priority) {
                best[r-1] = cp;
                priority = cp->parameters->priority;
              }
            }
          //}   // SGS commented out in FH code
        }
      }
    }
  }

  if (ctrlprog != NULL) {
    if (ctrlprog->parameters != NULL) {
       //strcpy(ctrl_params.name, cp->parameters->name);
       //strcpy(ctrl_params.description, cp->parameters->description);
       ctrl_params.radar = ctrlprog->radarinfo->radar;
       ctrl_params.channel = ctrlprog->radarinfo->channel;
       r = ctrl_params.radar-1;
       ctrl_params.current_pulseseq_index =
                      ctrlprog->parameters->current_pulseseq_index;
       ctrl_params.priority = ctrlprog->parameters->priority;

       if (TX_BEAM_PRIORITY) {
         ctrl_params.tbeam = best[r]->parameters->tbeam;
         ctrl_params.tbeamcode = best[r]->parameters->tbeamcode;
         ctrl_params.tbeamwidth = best[r]->parameters->tbeamwidth;
         ctrl_params.tbeamazm = best[r]->parameters->tbeamazm;
       } else {
         ctrl_params.tbeam = ctrlprog->parameters->tbeam;
         ctrl_params.tbeamcode = ctrlprog->parameters->tbeamcode;
         ctrl_params.tbeamwidth = ctrlprog->parameters->tbeamwidth;
         ctrl_params.tbeamazm = ctrlprog->parameters->tbeamazm;
       }

       if (TX_FREQ_PRIORITY) {
         ctrl_params.tfreq = best[r]->parameters->tfreq;
         ctrl_params.trise = best[r]->parameters->trise;
       } else {
         ctrl_params.tfreq = ctrlprog->parameters->tfreq;
         ctrl_params.trise = ctrlprog->parameters->trise;
       }

       if (RX_BEAM_PRIORITY) {
         ctrl_params.rbeam = best[r]->parameters->rbeam;
         ctrl_params.rbeamcode = best[r]->parameters->rbeamcode;
         ctrl_params.rbeamwidth = best[r]->parameters->rbeamwidth;
         ctrl_params.rbeamazm = best[r]->parameters->rbeamazm;
       } else {
         ctrl_params.rbeam = ctrlprog->parameters->rbeam;
         ctrl_params.rbeamcode = ctrlprog->parameters->rbeamcode;
         ctrl_params.rbeamwidth = ctrlprog->parameters->rbeamwidth;
         ctrl_params.rbeamazm = ctrlprog->parameters->rbeamazm;
       }

       if (RX_FREQ_PRIORITY) {
         ctrl_params.rfreq = best[r]->parameters->rfreq;
         ctrl_params.number_of_samples = best[r]->parameters->number_of_samples;
       } else {
         ctrl_params.rfreq = ctrlprog->parameters->rfreq;
         ctrl_params.number_of_samples =
                            ctrlprog->parameters->number_of_samples;
       }

       ctrl_params.buffer_index = ctrlprog->parameters->buffer_index;
       ctrl_params.baseband_samplerate =
                            ctrlprog->parameters->baseband_samplerate;
       ctrl_params.filter_bandwidth = ctrlprog->parameters->filter_bandwidth;
       ctrl_params.match_filter = ctrlprog->parameters->match_filter;
       ctrl_params.status = ctrlprog->parameters->status;
    }
  }

  return ctrl_params;
}

struct ControlPRM* cp_link_params(struct ControlPRM *ctrl_params)
{
  return ctrl_params;
}

struct ControlPRM* cp_verify_params(struct ControlPRM *ctrl_params)
{
/*
       ctrl_params.name=cp->parameters->name;
       ctrl_params.description=cp->parameters->description;
       ctrl_params.radar=cp->radarinfo->radar;
       ctrl_params.channel=cp->radarinfo->channel;
       ctrl_params.current_pulseseq_index=cp->parameters->current_pulseseq_index;
       ctrl_params.priority=cp->parameters->priority;
       ctrl_params.tbeam=cp->parameters->tbeam;
       ctrl_params.tbeamcode=cp->parameters->tbeamcode;
       ctrl_params.tbeamwidth=cp->parameters->tbeamwidth;
       ctrl_params.tbeamazm=cp->parameters->tbeamazm;
       ctrl_params.tfreq=cp->parameters->tfreq;
       ctrl_params.trise=cp->parameters->trise;
       ctrl_params.rbeam=cp->parameters->rbeam;
       ctrl_params.rbeamcode=cp->parameters->rbeamcode;
       ctrl_params.rbeamwidth=cp->parameters->rbeamwidth;
       ctrl_params.rbeamazm=cp->parameters->rbeamazm;
       ctrl_params.rfreq=cp->parameters->rfreq;
       ctrl_params.number_of_samples=cp->parameters->number_of_samples;
       ctrl_params.buffer_index=cp->parameters->buffer_index;
       ctrl_params.baseband_samplerate=cp->parameters->baseband_samplerate;
       ctrl_params.filter_bandwidth=cp->parameters->filter_bandwidth;
       ctrl_params.match_filter=cp->parameters->match_filter;
       ctrl_params.status=cp->parameters->status;
*/
  return ctrl_params;
}

int register_radar_channel(struct ControlProgram *cp, int radar, int chan)
{
  int i,r,c,status;

  if (cp != NULL) unregister_radar_channel(cp);
  status=-1;
  for (i=0; i<MAX_RADARS*MAX_CHANNELS; i++) {
    r = (i / MAX_CHANNELS) + 1;
    c = (i % MAX_CHANNELS) + 1;
    if (radar_channels[i] == NULL) {
      if (radar <= 0) radar=r;
      if (chan <= 0) chan=c;
      if (radar==r && chan==c) {
        printf("Registering: %d :: radar: %d channel: %d cp: %p\n",
                i,radar,chan,cp);
        status = 1;
        cp->parameters->radar=radar; 
        cp->parameters->channel=chan; 
        cp->radarinfo->radar=radar; 
        cp->radarinfo->channel=chan; 
        radar_channels[i]=cp;
        break;
      }
    }
  }

  return status;
}

struct ControlProgram *control_init(void) {

  int i;
  struct ControlProgram *cp;

  cp=malloc(sizeof(struct ControlProgram));
  cp->active=1;
  cp->clrfreqsearch.start=0;
  cp->clrfreqsearch.end=0;
  cp->parameters=malloc(sizeof(struct ControlPRM));
  cp->state=malloc(sizeof(struct ControlState));
  cp->radarinfo=malloc(sizeof(struct RadarPRM));
  cp->data=malloc(sizeof(struct DataPRM));
  cp->main=NULL;
  cp->back=NULL;
  cp->main_address=NULL;
  cp->back_address=NULL;
  strcpy(cp->parameters->name,"Generic Control Program Name - 80");
  strcpy(cp->parameters->description,
          "Generic  Control Program  Description - 120");
  cp->parameters->radar=-1;
  cp->parameters->channel=-1;
  cp->parameters->current_pulseseq_index=-1;
  cp->parameters->priority=50;
  cp->parameters->tbeam=-1;
  cp->parameters->tbeamcode=-1;
  cp->parameters->tbeamwidth=-1;
  cp->parameters->tbeamazm=-1;
  cp->parameters->tfreq=-1;
  cp->parameters->trise=10;
  cp->parameters->rbeam=-1;
  cp->parameters->rbeamcode=-1;
  cp->parameters->rbeamwidth=-1;
  cp->parameters->rbeamazm=-1;
  cp->parameters->rfreq=-1;
  cp->parameters->number_of_samples=-1;
  cp->parameters->buffer_index=-1;
  cp->parameters->baseband_samplerate=-1;
  cp->parameters->filter_bandwidth=-1;
  cp->parameters->match_filter=-1;
  cp->parameters->status=-1;

//       cp->parameters->phased=-1;
//       cp->parameters->filter=-1;
//       cp->parameters->gain=-1;
//       cp->parameters->seq_no=-1;
//       cp->parameters->seq_id=-1;
//       cp->parameters->fstatus=-1;
//       cp->parameters->center_freq=-1;

  cp->state->cancelled=0;
  cp->state->ready=0;
  cp->state->linked=0;
  cp->state->processing=0;
  cp->state->best_assigned_freq=0;
  cp->state->current_assigned_freq=0;
  cp->state->freq_change_needed=0;
  cp->state->thread=NULL;
  cp->state->fft_array=NULL;
  cp->radarinfo->site=-1;
  cp->radarinfo->radar=-1;
  cp->radarinfo->channel=-1;

  for (i=0;i<MAX_SEQS;i++) cp->state->pulseseqs[i]=NULL;

  return cp;
}

void cp_exit(struct ControlProgram *cp)
{
  pthread_t tid;
  pthread_t thread;
  int i,rc;
  int socket;
  struct ControlProgram *linker_program,*data;
  struct Thread_List_Item *thread_list,*thread_item,*thread_next,*thread_prev;
  int r,c;

  pthread_t threads[10];
  if (cp != NULL) {
    fprintf(stderr,"Client: Exit Command\n");
    cp->state->cancelled=1;
    r = cp->radarinfo->radar-1;
    c = cp->radarinfo->channel-1;

    //logger(&exit_lock_buffer,PRE,LOCK,"client_exit_init",r,c,0); 
    pthread_mutex_lock(&exit_lock);
    //logger(&exit_lock_buffer,POST,LOCK,"client_exit_init",r,c,0); 

    thread_list=cp_threads;
    tid = pthread_self();
    cp->active=0;
    rc = pthread_create(&thread, NULL,
                        (void *)&coordination_handler, (void *)cp);
    pthread_join(thread,NULL);
    i=0;
    rc = pthread_create(&threads[i], NULL,(void *) &timing_wait, NULL);
    pthread_join(threads[0],NULL);
    i=0;
    rc = pthread_create(&threads[i], NULL, (void *)&dds_end_cp, NULL);
    i++;
    rc = pthread_create(&threads[i], NULL, (void *)&timing_end_cp, NULL);
    //i++;
    //rc = pthread_create(&threads[i], NULL, (void *)&DIO_end_controlprogram,
    //                    NULL);
    i++;
    rc = pthread_create(&threads[i], NULL, (void *)&rx_end_cp, NULL);
    for (;i>=0;i--) pthread_join(threads[i], NULL);

    fprintf(stderr,"Closing Client Socket: %d\n", cp->state->socket);
    close(cp->state->socket);
    unregister_radar_channel(cp);
/*
    if (verbose>0) printf("Checking for programs linked to %p\n",cp); 
    while (thread_list!=NULL) {
      linker_program=thread_list->data;
      if (verbose>0)
        printf("%p  %p %p\n",linker_program,
                        linker_program->state->linked_program,cp); 
      if (linker_program->state->linked_program==cp) {
        if (verbose>0)
          printf("Found linked program %p that need to also die\n",
                  linker_program); 
        pthread_cancel(thread_list->id);       
        pthread_join(thread_list->id,NULL);
        thread_item=thread_list;   
        thread_next=thread_item->next;
        thread_list=thread_item->prev;
        if (thread_next != NULL) thread_next->prev=thread_list;
        else cp_threads=thread_list;
        if (thread_list != NULL) thread_list->next=thread_item->next;
        if(thread_item!=NULL) {
          free(thread_item);
          thread_item=NULL; 
        }
     } else {
       thread_list=thread_list->prev;
     }
   }
*/
    if (verbose>1)
      fprintf(stderr,"Client Exit: Freeing internal structures for %p\n", cp); 
    for (i=0;i<MAX_SEQS;i++) {
      if (cp->state->pulseseqs[i]!=NULL)
        TSGFree(cp->state->pulseseqs[i]);
    }
    if (verbose>1)
      fprintf(stderr,"Client Exit: Freeing controlprogram state %p\n",
                      cp->state); 
    if (cp->state!=NULL) {
      if (cp->state->fft_array!=NULL) {
        free(cp->state->fft_array);
        cp->state->fft_array=NULL;
      }
      free(cp->state);
      cp->state=NULL;
    }
    if (verbose>1) {
      fprintf(stderr,"Client Exit: Freed controlprogram state %p\n", cp->state);
      fprintf(stderr,"Client Exit: Freeing controlprogram parameters %p ",
                cp->parameters); 
    }
    if (cp->parameters!=NULL) {
       free(cp->parameters);
       cp->parameters=NULL;
    }
    if (verbose>1) {
      fprintf(stderr," %p\n",cp->parameters); 
      fprintf(stderr,"Client Exit: Freeing controlprogram data %p\n",
                cp->data); 
    }
    if (cp->main!=NULL) munmap(cp->main);
    if (cp->back!=NULL) munmap(cp->back);
    cp->main=NULL;
    cp->back=NULL;
    cp->main_address=NULL;
    cp->back_address=NULL;
    if (cp->data!=NULL) {
      free(cp->data);
      cp->data=NULL;
    }
    cp->active=0;
    if (verbose>1)
      fprintf(stderr,"Client Exit: Done with control program %p\n",cp); 
  } 

  if (verbose>1) {
    fprintf(stderr,"Client Exit: Waiting on Coordination thread\n");
    fprintf(stderr,"Client Exit: Done Waiting\n");
  }

  //logger(&exit_lock_buffer,PRE,UNLOCK,"client_exit_init",r,c,0); 
  pthread_mutex_unlock(&exit_lock);
  //logger(&exit_lock_buffer,POST,UNLOCK,"client_exit_init",r,c,0); 

  //printf("Client Exit: List UnLock\n");
  //pthread_mutex_unlock(&cp_list_lock);
  if (verbose>1) fprintf(stderr,"Client Exit: Done\n");
}

void *control_handler(struct ControlProgram *cp)
{
  int i,tid,status,rc,tmp;
  int r=-1,c=-1;
  useconds_t usecs_to_sleep;
  char usleep_file[120];
  FILE *fd;
  fd_set rfds;
  char command;
  int retval,socket,oldv,socket_err;
  int length=sizeof(int);
  int32 current_freq;
  int32 radar=0,channel=0;
  struct timeval tv,current_time,last_report;
  struct ROSMsg msg; 
  struct DriverMsg dmsg; 
  struct DataPRM control_data; 
  struct ControlPRM ctrl_params; 
  struct SiteSettings settings;
  struct TSGbuf *pulseseq;
  //struct TSGprm *tsgprm;
  struct SeqPRM tprm;
  int data_int;
  pthread_t thread,threads[10];
  struct timeval t0,t1,t2,t3,t4;
  unsigned long elapsed;
  unsigned long ultemp;
  int32 data_length;
  char entry_type,entry_name[80];
  int return_type,entry_exists;
  char *temp_strp;
  int32 temp_int32;
  double temp_double;

  /* Init the Control Program state */

  r = cp->radarinfo->radar-1;
  c = cp->radarinfo->channel-1;
  //logger(&exit_lock_buffer,PRE,LOCK,"client_handler_init",r,c,0); 

  pthread_mutex_lock(&exit_lock);
  //logger(&exit_lock_buffer,POST,LOCK,"client_handler_init",r,c,0); 
  //printf("control_list_buffer %p\n",cp_list_lock_buff);
  //logger(&cp_list_lock_buff,PRE,LOCK,"client_handler_init",r,c,1); 
  //printf("control_list_buffer %p\n",cp_list_lock_buff);

  pthread_mutex_lock(&cp_list_lock);
  //logger(&cp_list_lock_buff,POST,LOCK,"client_handler_init",r,c,1); 

  setbuf(stdout, 0);
  setbuf(stderr, 0);
  tid = pthread_self();

  /* set the cancellation parameters --
   - Enable thread cancellation 
   - Defer the action of the cancellation 
  */
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  pthread_cleanup_push((void *)&cp_exit,(void *)cp);
  if (cp != NULL)
    socket=cp->state->socket;
  //logger(&cp_list_lock_buff,PRE,UNLOCK,"client_handler_init",r,c,0); 

  pthread_mutex_unlock(&cp_list_lock);
  //logger(&cp_list_lock_buff,POST,UNLOCK,"client_handler_init",r,c,0); 
  //logger(&exit_lock_buffer,PRE,UNLOCK,"client_handler_init",r,c,0); 

  pthread_mutex_unlock(&exit_lock);
  //logger(&exit_lock_buffer,POST,UNLOCK,"client_handler_init",r,c,0); 

  gettimeofday(&last_report,NULL);
  while (1) {
    if (cp == NULL) break;
    retval = getsockopt(socket, SOL_SOCKET, SO_ERROR, &socket_err, &length);
    if ((retval!=0) || (socket_err!=0)) {
      printf("Error: socket error: %d : %d %d\n",socket,retval,socket_err);
      break;
    }

    /* Look for messages from external controlprogram process */
    FD_ZERO(&rfds);
    FD_SET(socket, &rfds);

    /* Wait up to five seconds. */
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    retval = select(socket+1, &rfds, NULL, NULL, &tv);

    /* Don’t rely on the value of tv now! */
    if (retval == -1) perror("select()");
    else if (retval) {
      r = cp->radarinfo->radar-1;
      c = cp->radarinfo->channel-1;
      //logger(&cp_list_lock_buff,PRE,LOCK,"client_command_init",r,c,1); 

      pthread_mutex_lock(&cp_list_lock);
      //logger(&cp_list_lock_buff,POST,LOCK,"client_command_init",r,c,1); 

      r = cp->radarinfo->radar-1;
      c = cp->radarinfo->channel-1;
      if ((r<0) || (c<0)) cp->data->status=-1;
 
      /* Read controlprogram msg */
      recv_data(socket, &msg, sizeof(struct ROSMsg));
      gettimeofday(&current_time,NULL);
      if ((current_time.tv_sec-last_report.tv_sec) > 5) {
        system("date -t > /tmp/server_cmd_time");
        last_report=current_time;
      }
      cp->state->thread->last_seen = current_time;
      //logger(&cp_list_lock_buff,PRE,UNLOCK,"client_command_init",r,c,1); 
      pthread_mutex_unlock(&cp_list_lock);
      //logger(&cp_list_lock_buff,POST,UNLOCK,"client_command_init",r,c,1); 
      /* Process controlprogram msg */
      switch(msg.type) {

        case PING:
          if (verbose > 1) printf("PING: START\n");
          gettimeofday(&t0,NULL);
          msg.status=1;
          send_data(socket, &msg, sizeof(struct ROSMsg));
          if (verbose > 1) printf("PING: END\n");
          break;

        case SET_INACTIVE:
          if (verbose > 1) printf("SET_RADAR_INACTIVE\n");
          gettimeofday(&t0,NULL);
          if ((r < 0) || (c < 0)) {
            if (verbose > 1) printf("  status -1: r: %d  c: %d\n", r,c);
            msg.status=-1;
            send_data(socket, &msg, sizeof(struct ROSMsg));
          } else {
            pthread_mutex_lock(&cp_list_lock);
            if (cp->active!=0) {
              cp->active=-1;
              rc = pthread_create(&thread, NULL, (void *)&coordination_handler,
                                  (void *)cp);
              pthread_join(thread,NULL);
            }

            pthread_mutex_unlock(&cp_list_lock);
            msg.status=1;
            send_data(socket, &msg, sizeof(struct ROSMsg));
          }
          if (verbose > 1) printf("end SET_RADAR_INACTIVE\n");
          break;

        case SET_ACTIVE:
          if (verbose > 1) printf("SET_RADAR_ACTIVE\n");
          gettimeofday(&t0,NULL);
          if ((r < 0) || (c < 0)) {
            if (verbose > 1) printf("  status -1: r: %d  c: %d\n", r,c);
            msg.status=-1;
            send_data(socket, &msg, sizeof(struct ROSMsg));
          } else {
            pthread_mutex_lock(&cp_list_lock);
            if (cp->active!=0) {
              cp->active=1;
              rc = pthread_create(&thread, NULL, (void *)&coordination_handler,
                                  (void *)cp);
              pthread_join(thread,NULL);
            }
            pthread_mutex_unlock(&cp_list_lock);
            msg.status=1;
            send_data(socket, &msg, sizeof(struct ROSMsg));
          }
          if (verbose > 1) printf("end SET_RADAR_ACTIVE\n");
          break;

/*
          case UPDATE_SITE_SETTINGS:
            gettimeofday(&t0,NULL);
            SettingsInit(&settings);
            recv_data(socket, &settings, sizeof(struct SiteSettings));
            msg.status=-1;
            send_data(socket, &msg, sizeof(struct ROSMsg));
            SettingsCpy(&settings,&site_settings);
            rc = pthread_create(&thread, NULL, (void *)&settings_rxfe_update_rf,(void *)&site_settings.rf_settings);
            pthread_join(thread,NULL);
            rc = pthread_create(&thread, NULL, (void *)&settings_rxfe_update_if,(void *)&site_settings.if_settings);
            pthread_join(thread,NULL);
            break;
*/
        case QUERY_INI_SETTING:
          if (verbose > 1) printf("start QUERY_INI_SETTING\n");
          recv_data(socket, &data_length, sizeof(int32));
          recv_data(socket, &entry_name, data_length*sizeof(char));
          recv_data(socket, &entry_type, sizeof(char));
          entry_exists=iniparser_find_entry(Site_INI,entry_name);
          msg.status=entry_exists;
          switch (entry_type) {
            case 'i':
              return_type='i';
              temp_int32=iniparser_getint(Site_INI,entry_name,-1);
              send_data(socket, &return_type, sizeof(char));
              data_length=1;
              send_data(socket, &data_length, sizeof(int32));
              send_data(socket, &temp_int32, data_length*sizeof(int32));
              break;
            case 'b':
              return_type='b';
              temp_int32=iniparser_getboolean(Site_INI,entry_name,-1);
              send_data(socket, &return_type, sizeof(char));
              data_length=1;
              send_data(socket, &data_length, sizeof(int32));
              send_data(socket, &temp_int32, data_length*sizeof(int32));
              break;
            case 's':
              return_type='s';
              temp_strp=iniparser_getstring(Site_INI,entry_name,NULL);
              send_data(socket, &return_type, sizeof(char));
              data_length=strlen(temp_strp);
              send_data(socket, &data_length, sizeof(int32));
              send_data(socket, temp_strp, data_length*sizeof(char));
            default:
              return_type=' ';
              send_data(socket, &return_type, sizeof(char));
              data_length=0;
              send_data(socket, &data_length, sizeof(int32));
              send_data(socket, temp_strp, data_length*sizeof(char));
          }

          send_data(socket, &msg, sizeof(struct ROSMsg));
          if (verbose > 1) printf("end QUERY_INI_SETTING\n");
          break;

        case GET_SITE_SETTINGS:
          gettimeofday(&t0,NULL);
          settings=site_settings;
          send_data(socket, &settings, sizeof(struct SiteSettings));
          msg.status=-1;
          send_data(socket, &msg, sizeof(struct ROSMsg));
          break;

        case SET_SITE_IFMODE:
          gettimeofday(&t0,NULL);
          settings=site_settings;
          recv_data(socket, &settings.ifmode, sizeof(settings.ifmode));
          msg.status=-1;
          send_data(socket, &msg, sizeof(struct ROSMsg));
          break;

        case SET_RADAR_CHAN:
          if (verbose > 1) printf("SET_RADAR_CHAN\n");
          gettimeofday(&t0,NULL);
          msg.status=1;
          recv_data(socket, &radar, sizeof(int32)); //requested radar
          recv_data(socket, &channel, sizeof(int32)); //requested channel
          if (verbose > 1) printf("Radar: %d Chan: %d\n",radar,channel);
          pthread_mutex_lock(&cp_list_lock);
          status=register_radar_channel(cp,radar,channel);
          if (!status) {
            if (verbose > -1)
              fprintf(stderr,"Control Program thread %p Bad status %d no "
                              "radar channel registered\n", tid,status);
          }
          msg.status=status;
          pthread_mutex_unlock(&cp_list_lock);
          send_data(socket, &msg, sizeof(struct ROSMsg));
          if (verbose > 1) printf("end SET_RADAR_CHAN\n");
          break;

        case LINK_RADAR_CHAN:
          if (verbose > 1) printf("LINK_RADAR_CHAN\n");
          gettimeofday(&t0,NULL);
          msg.status=1;
          recv_data(socket, &r, sizeof(r)); //requested radar
          recv_data(socket, &c, sizeof(c)); //requested channel
          pthread_mutex_lock(&cp_list_lock);
          cp->state->linked_program = find_reg_cp_by_rchan(r,c);
          cp->state->linked=1;
          if (cp->state->linked_program!=NULL) status=1;
          else status=0;

          msg.status=status;
          pthread_mutex_unlock(&cp_list_lock);
          send_data(socket, &msg, sizeof(struct ROSMsg));
          if (verbose > 1) printf("end LINK_RADAR_CHAN\n");
          break;

        case GET_PARAMETERS:
          if (verbose > 1) printf("GET_PARAMETERS: START\n");
          gettimeofday(&t0,NULL);
          if ((r < 0) || (c < 0)) {
            if (verbose > 1) printf("  status -1: r: %d  c: %d\n", r,c);
            send_data(socket, &ctrl_params, sizeof(struct ControlPRM));
            msg.status=-1;
            send_data(socket, &msg, sizeof(struct ROSMsg));
          } else {
            pthread_mutex_lock(&cp_list_lock);
            msg.status=status;
            ctrl_params = cp_fill_params(cp);
            pthread_mutex_unlock(&cp_list_lock);
            send_data(socket, &ctrl_params, sizeof(struct ControlPRM));
            send_data(socket, &msg, sizeof(struct ROSMsg));
          }
          if (verbose > 1) printf("GET_PARAMETERS: END\n");
          break;

        case GET_DATA:
          if (verbose > 1) printf("GET_DATA: START\n");
          gettimeofday(&t0,NULL);
          if ((r < 0) || (c < 0)) {
            if (verbose > 1) printf("  status -1: r: %d  c: %d\n", r,c);
            cp->data->status=-1;
            send_data(socket, cp->data, sizeof(struct DataPRM));
            msg.status=-1;
            send_data(socket, &msg, sizeof(struct ROSMsg));
          } else {
            msg.status=status;
            rc = pthread_create(&thread, NULL, (void *)&rx_cp_get_data,
                                (void *) cp);
            pthread_join(thread,NULL);

            //JDS: TODO do some GPS timestamp checking here
            pthread_mutex_lock(&cp_list_lock);
            cp->data->event_secs=cp->state->gpssecond;
            cp->data->event_nsecs=cp->state->gpsnsecond;
            send_data(socket, cp->data, sizeof(struct DataPRM));
            if (cp->data->status==0) {
              if (verbose > 1)
                printf("GET_DATA: main: %d %d\n",sizeof(uint32),
                        sizeof(uint32)*cp->data->samples);
              send_data(socket, cp->main,
                        sizeof(uint32)*cp->data->samples);
              send_data(socket, cp->back,
                        sizeof(uint32)*cp->data->samples);
              send_data(socket, &bad_transmit_times.length,
                        sizeof(bad_transmit_times.length));
              if (verbose > 1)
                printf("GET_DATA: bad_transmit_times: %d %d\n",
                       sizeof(uint32),sizeof(uint32)*bad_transmit_times.length);
              send_data(socket, bad_transmit_times.start_usec,
                        sizeof(uint32)*bad_transmit_times.length);
              send_data(socket, bad_transmit_times.duration_usec,
                        sizeof(uint32)*bad_transmit_times.length);
              tmp=MAX_TRANSMITTERS;
              send_data(socket,&tmp,sizeof(int));
              send_data(socket,&txstatus[r].AGC,sizeof(int)*tmp);
              send_data(socket,&txstatus[r].LOWPWR,sizeof(int)*tmp);
            } else {
              if (verbose > 1) printf("GET_DATA: Bad status %d\n",
                        cp->data->status);
            } 
            send_data(socket, &msg, sizeof(struct ROSMsg));
            pthread_mutex_unlock(&cp_list_lock);
          }

          gettimeofday(&t1,NULL);
          if (verbose > 1) {
            elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
            elapsed+=(t1.tv_usec-t0.tv_usec);
            if (verbose > 1)
              printf("Client:  Get Data Elapsed Microseconds: %ld\n",elapsed);
          }
          if (verbose > 1) printf("GET_DATA: END\n");
          break;

        case SET_PARAMETERS:
          if (verbose > 1) printf("SET_PARAMETERS: START\n");
          gettimeofday(&t0,NULL);
          if ((r < 0) || (c < 0)) {
            if (verbose > 1) printf("  status -1: r: %d  c: %d\n", r,c);
            recv_data(socket, cp->parameters,
                      sizeof(struct ControlPRM));
            msg.status=-1;
            send_data(socket, &msg, sizeof(struct ROSMsg));
          } else {
            msg.status=1;
            pthread_mutex_lock(&cp_list_lock);
            recv_data(socket, cp->parameters,
                      sizeof(struct ControlPRM));
            if (cp->parameters->rfreq<0)
              cp->parameters->rfreq = cp->parameters->tfreq;
            send_data(socket, &msg, sizeof(struct ROSMsg));
            pthread_mutex_unlock(&cp_list_lock);
          }
          if (verbose > 1) printf("SET_PARAMETERS: END\n");
          break;

        case REGISTER_SEQ:
          if (verbose > 1) printf("REGISTER_SEQ: START\n");
          gettimeofday(&t0,NULL);
          msg.status=1;
          recv_data(socket,&tprm, sizeof(struct SeqPRM)); // requested pulseseq
          pthread_mutex_lock(&cp_list_lock);
          cp->state->pulseseqs[tprm.index] = malloc(sizeof(struct TSGbuf));
          cp->parameters->current_pulseseq_index=tprm.index;
          cp->state->pulseseqs[tprm.index]->len=tprm.len;
          cp->state->pulseseqs[tprm.index]->step=tprm.step;
          cp->state->pulseseqs[tprm.index]->index=tprm.index;
          cp->state->pulseseqs[tprm.index]->prm=NULL;
          cp->state->pulseseqs[tprm.index]->rep=
                malloc(sizeof(unsigned char)*
                        cp->state->pulseseqs[tprm.index]->len);
          cp->state->pulseseqs[tprm.index]->code=
                malloc(sizeof(unsigned char)*
                        cp->state->pulseseqs[tprm.index]->len);
          /* JDS: 20121017 : TSGprm structure is deprecated and should not be
                              allocated */
          /* cp->state->pulseseqs[tprm.index]->prm=
                            malloc(sizeof(struct TSGprm)); */
          recv_data(socket,cp->state->pulseseqs[tprm.index]->rep, 
                      sizeof(unsigned char)*
                      cp->state->pulseseqs[tprm.index]->len);
                      // requested pulseseq
          recv_data(socket,cp->state->pulseseqs[tprm.index]->code, 
                      sizeof(unsigned char)*
                      cp->state->pulseseqs[tprm.index]->len);
                      // requested pulseseq
          if ((r < 0) || (c < 0)) {
            if (verbose > 1) printf("  status -1: r: %d  c: %d\n", r,c);
            msg.status=-1;
          } else {
            //send on to timing socket
            rc = pthread_create(&threads[0], NULL,
                                (void *)&timing_register_seq,
                                (void *)cp);
            //send on to dds socket
            rc = pthread_create(&threads[1], NULL,
                                (void *)&dds_register_seq,
                                (void *)cp);
            //printf("Waiting on Timing Thread\n");
            pthread_join(threads[0],NULL);
            //printf("Waiting on DDS\n"); 
            pthread_join(threads[1],NULL);
          }

          pthread_mutex_unlock(&cp_list_lock);
          if (verbose > 1) printf("REGISTER_SEQ: SEND ROSMsg\n");
          send_data(socket, &msg, sizeof(struct ROSMsg));
          gettimeofday(&t1,NULL);
          if (verbose > 1) {
            elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
            elapsed+=(t1.tv_usec-t0.tv_usec);
          //  if (verbose > 1 )
              printf("Client:  Reg Seq Elapsed Microseconds: %ld\n",elapsed);
          }
          if (verbose > 1) printf("REGISTER_SEQ: END\n");
          break;

        case SET_READY_FLAG:
          if (verbose > 1) printf("SET READY : START :: r=%d  c=%d stat=",r,c);
          gettimeofday(&t0,NULL);
          if ((r < 0) || (c < 0)) {
            if (verbose > 1) printf("-1\n");
            msg.status=-1;
          } else {
            if (verbose > 1) printf("0\n");
            msg.status=0;
            pthread_mutex_lock(&cp_list_lock);
            if (cp->active!=0) cp->active=1;
            pthread_mutex_unlock(&cp_list_lock);
            i=0;
            rc = pthread_create(&threads[i], NULL,
                                (void *)&timing_wait, NULL);
            pthread_join(threads[0],NULL);
            pthread_mutex_lock(&cp_list_lock);
            //cp->state->ready=1;
            i=0;
            rc = pthread_create(&threads[i],NULL, (void *)&DIO_ready_cp, cp);
            i++;
            rc = pthread_create(&threads[i],NULL, (void *)&timing_ready_cp, cp);
            i++;
            rc = pthread_create(&threads[i], NULL, (void *)&dds_ready_cp, cp);
            i++;
            rc = pthread_create(&threads[i], NULL, (void *) &rx_ready_cp, cp);
            for (;i>=0;i--) {
              gettimeofday(&t2,NULL);
              pthread_join(threads[i],NULL);
              gettimeofday(&t3,NULL);
              if (verbose > 1) {
                elapsed=(t3.tv_sec-t2.tv_sec)*1E6;
                elapsed+=(t3.tv_usec-t2.tv_usec);
                //if (verbose > 1 )
                  printf("Client:Set Ready: %d Elapsed Microseconds: %ld\n",
                          i,elapsed);
              }
            }

            gettimeofday(&t2,NULL);
            rc = pthread_create(&thread, NULL,
                                (void *)&coordination_handler,
                                (void *)cp);
            pthread_join(thread,NULL);
            gettimeofday(&t3,NULL);
            if (verbose > 1) {
              elapsed=(t3.tv_sec-t2.tv_sec)*1E6;
              elapsed+=(t3.tv_usec-t2.tv_usec);
              //if (verbose > 1 )
                printf("Client:Set Ready: Coord Elapsed Microseconds: %ld\n",
                        elapsed);
            }
            pthread_mutex_unlock(&cp_list_lock);
          }

          send_data(socket, &msg, sizeof(struct ROSMsg));
          gettimeofday(&t1,NULL);
          if (verbose > 1) {
            elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
            elapsed+=(t1.tv_usec-t0.tv_usec);
            //if (verbose > 1 )
              printf("Client:  Set Ready Elapsed Microseconds: %ld\n",elapsed);
           }
          if (verbose > 1) printf("SET READY : END\n");
          break;

        case REQUEST_CLEAR_FREQ_SEARCH:
          if (verbose > 1) printf("REQUEST_CLEAR_FREQ_SEARCH\n");
          pthread_mutex_lock(&clr_lock);
          usecs_to_sleep=0;
          sprintf(usleep_file,"/ros_usleep.txt");
  	      fprintf(stdout,"Opening usleep file: %s\n",usleep_file);
  	      fd=fopen(usleep_file,"r+");
  	      retval=fscanf(fd,"%d\n",&usecs_to_sleep);
          fclose(fd);
          if (retval!=1) usecs_to_sleep=0;
 
  	      fprintf(stdout,"  usecs: %d\n",usecs_to_sleep);

          gettimeofday(&t0,NULL);
          pthread_mutex_lock(&cp_list_lock);
          recv_data(socket,&cp->clrfreqsearch,
                    sizeof(struct CLRFreqPRM)); // requested search parameters
          //printf("Client: Requst CLRSearch: %d %d\n",
                  //cp->clrfreqsearch.start,
                  //cp->clrfreqsearch.end);
          if ((r < 0) || (c < 0)) {
            if (verbose > 1) printf("  status -1: r: %d  c: %d\n", r,c);
            msg.status=-1;
          } else {
            rc = pthread_create(&threads[0], NULL,
                                (void *)&DIO_clrfreq,cp);
            pthread_join(threads[0],NULL);
            usleep(usecs_to_sleep);
            rc = pthread_create(&threads[0], NULL, (void *)&rx_clrfreq,cp);
            pthread_join(threads[0],NULL);
            rc = pthread_create(&threads[0], NULL,
                                (void *)&DIO_rxfe_reset,NULL);
            pthread_join(threads[0],NULL);
            usleep(usecs_to_sleep);
            msg.status=cp->state->freq_change_needed;
          }

          send_data(socket, &msg, sizeof(struct ROSMsg));
          pthread_mutex_unlock(&cp_list_lock);
          gettimeofday(&t1,NULL);
          if (verbose > 1) {
            elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
            elapsed+=(t1.tv_usec-t0.tv_usec);
            //if (verbose > 1 )
              printf("Client:  CLR Elapsed Microseconds: %ld\n",elapsed);
          }
          pthread_mutex_unlock(&clr_lock);
          if (verbose > 1) printf("REQUEST_CLEAR_FREQ_SEARCH : END\n");
          break;

        case REQUEST_ASSIGNED_FREQ:
          if (verbose > 1) printf("REQUEST_ASSIGNED_FREQ\n");
          gettimeofday(&t0,NULL);
          pthread_mutex_lock(&cp_list_lock);
          if ( (r < 0) || (c < 0)) {
            if (verbose > 1) printf("  status -1: r: %d  c: %d\n", r,c);
            msg.status=-1;
            cp->state->current_assigned_freq=0;
            cp->state->current_assigned_noise=0;
          } else {
            rc = pthread_create(&threads[0], NULL, (void *)&rx_assign_freq,
                                (void *)cp);
            pthread_join(threads[0],NULL);
            msg.status = cp->state->best_assigned_freq !=
                          cp->state->current_assigned_freq;
          }

          //cp->state->current_assigned_noise=1;
          current_freq=cp->state->current_assigned_freq; 
          send_data(socket, &current_freq, sizeof(int32));
          send_data(socket, &cp->state->current_assigned_noise,
                    sizeof(float));
          send_data(socket, &msg, sizeof(struct ROSMsg));
          pthread_mutex_unlock(&cp_list_lock);
          if (verbose > 1) {
            gettimeofday(&t1,NULL);
            elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
            elapsed+=(t1.tv_usec-t0.tv_usec);
            printf("Client:  Request Freq Elapsed Microseconds: %ld\n",elapsed);
          }
          if (verbose > 1) printf("REQUEST_ASSIGNED_FREQ : END\n");
          break;

        case QUIT:
          gettimeofday(&t0,NULL);
          printf("Client QUIT\n");
          msg.status=0;
          send_data(socket, &msg, sizeof(struct ROSMsg));
          //cp_exit(cp);
          pthread_exit(NULL);
          break;

        default:
          msg.status=1;
          send_data(socket, &msg, sizeof(struct ROSMsg));
      }
      /* FD_ISSET(0, &rfds) will be true. */
    } else {
      if (verbose > 1)
        printf("Client: No data within five seconds.\n");
    }//else printf("No data within five seconds.\n");
//        if (verbose>1) printf("Client: test cancel\n");
        
    pthread_testcancel();
  }

  pthread_testcancel();
  pthread_cleanup_pop(0);
  cp_exit(cp);
  pthread_exit(NULL);
}

void *controlprogram_free(struct ControlProgram *cp)
{

}
