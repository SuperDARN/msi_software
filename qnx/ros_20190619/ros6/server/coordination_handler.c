#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include "control_program.h"
#include "global_server_variables.h"
#include "dio_handler.h"
#include "timing_handler.h"
#include "RX_handler.h"
#include "dds_handler.h"

extern struct Thread_List_Item *cp_threads;
extern pthread_mutex_t coord_lock;
extern int *trigger_state_pointer;  // [0|1|2] [no activity|pre-trig|triggering]
extern int *ready_state_pointer;    // [0|1|2] [no|some|all] ctrl progs ready
extern int *ready_count_pointer;
extern int trigger_type;            // 0 - strict controlprogram ready
                                    // 1 - elasped software time
                                    // 2 - external gps event 
extern int txread[MAX_RADARS];
extern int verbose;
extern int gpssock;
//int oldv;

//extern struct timeval t_ready_first, t_ready_final, t_pre_start, t_pre_end;
//extern struct timeval t_post_start,  t_post_end;

void *coordination_handler(struct ControlProgram *cp)
{
  int ncp=0,nready=0,nproc=0;
  int rc,i,temp;
  char *timestr;
  pthread_t threads[4];
  struct Thread_List_Item *thread_list;
  int gpssecond,gpsnsecond;
  struct DriverMsg msg;
  struct ControlProgram *cprog;
  int ready_state,trigger_state,ready_count;
  struct timeval t0,t1,t2,t3,t4,t5,t6;
  unsigned long elapsed;

  if (verbose > 0) {
    gettimeofday(&t0, NULL);
    fprintf(stderr, "Coord: Start Coord Handler: %ld %ld\n",
                    (long)t0.tv_sec, (long)t0.tv_usec);
  }
//  pthread_mutex_lock(&coord_lock); //lock the global structures   // SGS

  ready_state = *ready_state_pointer;
  trigger_state = *trigger_state_pointer;
//  ready_count = *ready_count_pointer;   // SGS
  if (cp->active == 1) {
//    ready_count++;    // SGS
    cp->state->ready = 1;
    cp->state->processing = 0;
    txread[cp->parameters->radar-1] = 1;
  }

  /* Calculate Ready State */
  if (trigger_state < 2) { 
    thread_list = cp_threads;
    while (thread_list != NULL) {
      cprog = thread_list->data;
      if (cprog != NULL && cprog->active == 1) {
        ncp++;
        if (cprog->state->ready == 1) nready++;
        if (cprog->state->processing == 1) nproc++;   // SGS: not used
      }
      thread_list = thread_list->prev;
    }

    if (nready == 0) {
      ready_state = 0;
    } else if ((nready!=ncp) && (nready > 0)) {
      ready_state=1;
    } else if ((nready==ncp) && (nready > 0)) {
      ready_state=2;
    }
  }

  /* Coordinate trigger events from multiple control programs */
  switch (ready_state) {
    case 0: break; /* no   control program is ready to trigger */
    case 1: break; /* some control programs are ready to trigger */
    case 2:        /* all  control programs ready to trigger */

      if (verbose > 1) { 
        gettimeofday(&t1, NULL);
        elapsed = (t1.tv_sec-t0.tv_sec)*1E6;
        elapsed += (t1.tv_usec-t0.tv_usec);
        printf("Coord: Start Ready State Case 2 Elapsed Microseconds: %ld\n",
                elapsed);
      }

      trigger_state = 1;

      // SGS: this whole construct does nothing...
      /*
      thread_list = cp_threads;
      thread_list = cp_threads;   // SGS: why twice???
      while (thread_list != NULL) { // SGS: this whole thing is dumb
        cprog = thread_list->data;
        if (cprog != NULL) {
          if (cprog->active == 1) {
            if (cprog->state->ready == 1) { } // SGS: who does this?!
              if (cprog->state->processing == 1) { }
          }
        } else { }
        thread_list = thread_list->prev;
      }
      */

      if (verbose > 1) { 
        gettimeofday(&t2, NULL);
        elapsed = (t2.tv_sec-t1.tv_sec)*1E6;
        elapsed += (t2.tv_usec-t1.tv_usec);
        printf("Coord: Pre-Trigger Active Check %d Elapsed Microseconds: "
                "%ld\n",i,elapsed);
      }
 
      /* start pretrigger threads and wait for them to finish */
      i = 0;
      rc = pthread_create(&threads[i], NULL, (void *)&rx_pretrigger, NULL);
      i++;
      rc = pthread_create(&threads[i], NULL, (void *)&dds_pretrigger, NULL);
      i++;
      rc = pthread_create(&threads[i], NULL, (void *)&DIO_pretrigger, NULL);
      i++;
      rc = pthread_create(&threads[i], NULL, (void *)&timing_pretrigger, NULL);

      // pthread_join(threads[i],NULL);   // SGS - in FH code, but correct???
      for (;i>=0;i--) pthread_join(threads[i], NULL);

      i = 0;
      trigger_state = 2; //trigger
      // trigger_type:  0: free run  1: elapsed-time  2: gps

      usleep(1000); /* waiting for 1 millisecond for some reason... */

      /* start a thread that tells the timing card we are ready to trigger,
         it knows the trigger type, and wait for it to finish */
      rc = pthread_create(&threads[0], NULL, (void *)&timing_trigger,
                                             (void *)trigger_type);
      pthread_join(threads[0], NULL);

      if (verbose > 1) { 
        gettimeofday(&t4,NULL);
        elapsed = (t4.tv_sec-t3.tv_sec)*1E6;
        elapsed += (t4.tv_usec-t3.tv_usec);
        printf("Coord: Trigger Elapsed Microseconds: %ld\n",elapsed);
      }

      trigger_state = 3; //post-trigger
      msg.type = GPS_GET_EVENT_TIME; // pickup the time saved in the event
                                     // register on the GPS card
      msg.status = 1;
      send_data(gpssock, &msg, sizeof(struct DriverMsg));
      recv_data(gpssock, &gpssecond, sizeof(int));
      recv_data(gpssock, &gpsnsecond, sizeof(int));
      recv_data(gpssock, &msg, sizeof(struct DriverMsg));

      if (verbose > 1)
        printf("Coord: GPS_GET_EVENT_TIME: %d %d\n", gpssecond, gpsnsecond);

      i = 0;
      if (cp->active == 1) { 
        cp->state->gpssecond = gpssecond;
        cp->state->gpsnsecond = gpsnsecond;
        if (txread[cp->parameters->radar-1]) {
          rc = pthread_create(&threads[i], NULL, (void *)&DIO_tx_status,
                                                 (void *)cp->parameters->radar);
          pthread_join(threads[i],NULL);
          txread[cp->parameters->radar-1] = 0;
          i++;    // SGS: this is missing from FH code... !!!
        }
      }

      // initiate post-trigger threads
      i = 0;    // SGS: was missing from CV
      rc = pthread_create(&threads[i], NULL, (void *)&rx_posttrigger, NULL);
//      pthread_join(threads[i],NULL);    // SGS from FH code
      i++;    // SGS: from FH code
      rc = pthread_create(&threads[i], NULL, (void *)&timing_posttrigger, NULL);
//      pthread_join(threads[i],NULL);    // SGS from FH code

      for (;i>=0;i--) pthread_join(threads[i],NULL);

      thread_list = cp_threads;
      while (thread_list != NULL) {
        cprog = thread_list->data;
        if (cprog != NULL) {
          if (cprog->state != NULL) {
            if (cprog->state->ready == 1) {
              cprog->state->ready = 0;
              cprog->state->processing = 1;
            }
          }
        }
        thread_list = thread_list->prev;
      }

      trigger_state = 0; //post-trigger
//      ready_count = 0;    // SGS
      ready_state = 0;

      if (verbose > 1) { 
        gettimeofday(&t6,NULL);
        elapsed = (t6.tv_sec-t4.tv_sec)*1E6;
        elapsed += (t6.tv_usec-t4.tv_usec);
        printf("Coord: Post Trigger Elapsed Microseconds: %ld\n",elapsed);

        elapsed = (t6.tv_sec-t0.tv_sec)*1E6;
        elapsed += (t6.tv_usec-t0.tv_usec);
        printf("Coord: Total Elapsed Microseconds: %ld\n",elapsed);
      }
      break; 

   } // end of ready_state switch

   *ready_state_pointer = ready_state;
   *trigger_state_pointer = trigger_state;
//   *ready_count_pointer = ready_count;    // SGS

//   pthread_mutex_unlock(&coord_lock); //unlock    // SGS

   pthread_exit(NULL);
}

