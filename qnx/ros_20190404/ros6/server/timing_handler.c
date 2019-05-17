#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include "control_program.h"
#include "global_server_variables.h"

extern int timingsock;
extern pthread_mutex_t timing_comm_lock;

extern int verbose;
extern struct TRTimes bad_transmit_times;

void *timing_ready_controlprogram(struct ControlProgram *cp)
{
  struct DriverMsg msg;

  pthread_mutex_lock(&timing_comm_lock);
  if (cp != NULL) {
    if (cp->state->pulseseqs[cp->parameters->current_pulseseq_index]!=NULL) {
      msg.type=TIMING_CtrlProg_READY;
      msg.status=1;
      send_data(timingsock, &msg, sizeof(struct DriverMsg));
      send_data(timingsock, cp->parameters, sizeof(struct ControlPRM));
      recv_data(timingsock, &msg, sizeof(struct DriverMsg));
    } 
  }
  pthread_mutex_unlock(&timing_comm_lock);
  pthread_exit(NULL);
}

void *timing_end_controlprogram(void *arg)
{
  struct DriverMsg msg;

  pthread_mutex_lock(&timing_comm_lock);

  msg.type = TIMING_CtrlProg_END;
  msg.status = 1;
  send_data(timingsock, &msg, sizeof(struct DriverMsg));
  pthread_mutex_unlock(&timing_comm_lock);
  pthread_exit(NULL);
}

void *timing_register_seq(struct ControlProgram *cp)
{
  struct DriverMsg msg;
  int index;

  pthread_mutex_lock(&timing_comm_lock);

  msg.type = TIMING_REGISTER_SEQ;
  msg.status = 1;
  send_data(timingsock, &msg, sizeof(struct DriverMsg));
  send_data(timingsock, cp->parameters, sizeof(struct ControlPRM));

  index = cp->parameters->current_pulseseq_index;
  send_data(timingsock, &index, sizeof(index)); //requested index
  /* requested pulseseq */
  send_data(timingsock,cp->state->pulseseqs[index], sizeof(struct TSGbuf));
  send_data(timingsock,cp->state->pulseseqs[index]->rep,
            sizeof(unsigned char) * cp->state->pulseseqs[index]->len);
  send_data(timingsock,cp->state->pulseseqs[index]->code,
            sizeof(unsigned char)* cp->state->pulseseqs[index]->len);
  recv_data(timingsock, &msg, sizeof(struct DriverMsg));

  pthread_mutex_unlock(&timing_comm_lock);
  pthread_exit(NULL);
}

void *timing_pretrigger(void *arg)
{
  struct DriverMsg msg;

  pthread_mutex_lock(&timing_comm_lock);

  msg.type = TIMING_PRETRIGGER;
  msg.status = 1;
  if (verbose > 0) printf("TIMING: PRETRIGGER: Send msg\n");
  send_data(timingsock, &msg, sizeof(struct DriverMsg));

  if (verbose > 0) printf("TIMING: PRETRIGGER: free %p %p\n",
                          bad_transmit_times.start_usec,
                          bad_transmit_times.duration_usec);
  if (bad_transmit_times.start_usec != NULL)
    free(bad_transmit_times.start_usec);
  if (bad_transmit_times.duration_usec != NULL)
    free(bad_transmit_times.duration_usec);
  bad_transmit_times.start_usec = NULL;
  bad_transmit_times.duration_usec = NULL;

  if (verbose > 0) printf("TIMING: PRETRIGGER: free end\n"
                          "TIMING: PRETRIGGER: recv bad_tx times object\n");

  recv_data(timingsock, &bad_transmit_times.length,
            sizeof(bad_transmit_times.length));
  if (verbose > 0)
    printf("TIMING: PRETRIGGER: length %d\n",bad_transmit_times.length);

  if (bad_transmit_times.length > 0) {
    bad_transmit_times.start_usec =
                  malloc(sizeof(unsigned int)*bad_transmit_times.length);
    bad_transmit_times.duration_usec =
                  malloc(sizeof(unsigned int)*bad_transmit_times.length);
  } else {
    bad_transmit_times.start_usec = NULL;
    bad_transmit_times.duration_usec = NULL;
  }

  //printf("TIMING: PRETRIGGER: recv start usec object\n");
  recv_data(timingsock, bad_transmit_times.start_usec,
            sizeof(unsigned int)*bad_transmit_times.length);
  //printf("TIMING: PRETRIGGER: recv duration usec object\n");
  recv_data(timingsock, bad_transmit_times.duration_usec,
            sizeof(unsigned int)*bad_transmit_times.length);
  //printf("TIMING: PRETRIGGER: recv msg\n");
  recv_data(timingsock, &msg, sizeof(struct DriverMsg));
  pthread_mutex_unlock(&timing_comm_lock);

  if (verbose > 0) printf("TIMING: PRETRIGGER: exit\n");

  pthread_exit(NULL);
}

void *timing_trigger(int trigger_type)
{
  struct DriverMsg msg;

  pthread_mutex_lock(&timing_comm_lock);

  if (verbose > 0) printf("TIMING: TRIGGER: type ");
  switch (trigger_type) {
    case 0:
      msg.type = TIMING_TRIGGER;
      if (verbose > 0) printf(" free-run\n");
      break;
    case 1:
      msg.type = TIMING_TRIGGER;
      if (verbose > 0) printf(" elapsed time\n");
      break;
    case 2:
      msg.type = TIMING_GPS_TRIGGER;
      if (verbose > 0) printf(" GPS\n");
      break;
  }

  msg.status = 1;
  send_data(timingsock, &msg, sizeof(struct DriverMsg));
  recv_data(timingsock, &msg, sizeof(struct DriverMsg));
  pthread_mutex_unlock(&timing_comm_lock);

  pthread_exit(NULL);
}

void *timing_wait(void *arg)
{
  struct DriverMsg msg;

  pthread_mutex_lock(&timing_comm_lock);

  msg.type = TIMING_WAIT;
  msg.status = 1;
  send_data(timingsock, &msg, sizeof(struct DriverMsg));
  recv_data(timingsock, &msg, sizeof(struct DriverMsg));
  pthread_mutex_unlock(&timing_comm_lock);

  pthread_exit(NULL);
}

void *timing_posttrigger(void *arg)
{
  struct DriverMsg msg;

  pthread_mutex_lock(&timing_comm_lock);

  msg.type = TIMING_POSTTRIGGER;
  msg.status = 1;
  send_data(timingsock, &msg, sizeof(struct DriverMsg));
  recv_data(timingsock, &msg, sizeof(struct DriverMsg));
  pthread_mutex_unlock(&timing_comm_lock);

  pthread_exit(NULL);
}

/*
void *timing_handler(void *arg)
{

  pthread_mutex_lock(&timing_comm_lock);
   if (verbose>1) fprintf(stderr,"Inside the timing handler\n");
   if (verbose>1) fprintf(stderr,"Timing: Do some work\n");
   if (verbose>1) fprintf(stderr,"Leaving timing handler\n");
  pthread_mutex_unlock(&timing_comm_lock);
   pthread_exit(NULL);
};
*/

