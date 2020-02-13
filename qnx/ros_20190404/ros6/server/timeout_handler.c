#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "global_server_variables.h"
#include "log_handler.h"

extern struct Thread_List_Item *cp_threads;
extern pthread_mutex_t cp_list_lock,exit_lock;
extern char *cp_list_lock_buff,*exit_lock_buffer;
extern int verbose;

void timeout_exit(void *arg)
{
  pthread_t tid;
  /* get the calling thread's ID */
  tid = pthread_self();
}

void *timeout_handler(void *arg)
{
  struct Thread_List_Item *thread_list,*thread_item,*thread_next;
  struct ControlProgram *data;
  struct timeval t1;
  unsigned long elapsed;
  int i,cancelled;
  struct ControlProgram *cp;

/* set the cancellation parameters --
   - Enable thread cancellation 
   - Defer the action of the cancellation 
*/
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  pthread_cleanup_push(timeout_exit,NULL);

  while (1) {
    pthread_mutex_lock(&exit_lock);  //hold the exit lock
    gettimeofday(&t1,NULL);

    if (verbose > 0) {
      fprintf(stdout, "Checking for dead clients: %d.%d\n",
                       (int)t1.tv_sec,(int)t1.tv_usec);
      fflush(stdout);
      fprintf(stderr, "Checking for dead clients: %d.%d\n",
                       (int)t1.tv_sec,(int)t1.tv_usec);
      fflush(stderr);
    }

    thread_list = cp_threads;
    while (thread_list != NULL) {

      cancelled = 0;
      elapsed = (long)t1.tv_sec - (long)thread_list->last_seen.tv_sec;
      if (elapsed < 0) elapsed = 0;   /* not possible since unsigned long */

      if (verbose > 0) {
        fprintf(stdout, "TIMEOUT: %p elapsed: %ld timeout: %d\n",
                        thread_list->data,elapsed,thread_list->timeout.tv_sec);
        fflush(stdout);
      }

      if (elapsed > thread_list->timeout.tv_sec) {

        if (verbose > -1) {
          fprintf(stderr,"TIMEOUT: elapsed time! %d %d\n",
                          elapsed,thread_list->timeout.tv_sec);
          fprintf(stderr,"  controlprogram addr: %p thread addr: %p id: %p\n",
                          thread_list->data,thread_list,thread_list->id);
        }

        cp = thread_list->data;
        if (verbose > 0) {
          fprintf(stderr,"    thread next: %p\n",thread_list->next);
          fprintf(stderr,"    thread prev: %p\n",thread_list->prev);
          fprintf(stderr,"    control: %p\n",cp);
        }

        if (cp != NULL) {
          if (verbose > 0)
            fprintf(stderr,"    control state: %p\n",cp->state);

          if (cp->state != NULL) {      
            cancelled = cp->state->cancelled; 
            if (verbose > -1) {
              fprintf(stderr,"    control cancelled: %d\n",cancelled);
              fprintf(stderr,"controlprogram thread %p %p over ran timeout..."
                             "trying to cancel\n", thread_list,thread_list->id);
            }

            if (cancelled == 0) {
              pthread_mutex_unlock(&exit_lock);  //unlock exit lock
              if (verbose > -1)
                fprintf(stderr,"TIMEOUT: Canceling client thread %p %d\n",
                                thread_list, thread_list->id);
              pthread_cancel(thread_list->id);       
              pthread_join(thread_list->id,NULL);
              pthread_mutex_lock(&exit_lock);  //hold the exit lock
            } else {
              if (verbose > 0)
                fprintf(stderr, "TIMEOUT: already cancelled\n");
            }
          } else {
            if (verbose > 0)
              fprintf(stderr, "TIMEOUT: cp is NULL\n");
          } 
        }
        fflush(stderr);

         if (verbose> 1) fprintf(stderr,"    Adjusting thread list\n");
         thread_item = thread_list;   
         thread_next = thread_item->next;
         thread_list = thread_item->prev;
         if (thread_next != NULL) thread_next->prev = thread_list;
         else cp_threads = thread_list;

         if (thread_list != NULL) thread_list->next=thread_item->next;

         if (thread_item != NULL) {
           if (thread_item->data != NULL) free(thread_item->data);
           thread_item->data = NULL;
           free(thread_item);
         }
         thread_item=NULL;
       } else { 
         thread_list=thread_list->prev;
       }
     }

     thread_list = cp_threads;
     while (thread_list != NULL)
       thread_list = thread_list->prev;

     pthread_mutex_unlock(&exit_lock);  //unlock exit lock
     pthread_testcancel();
     sleep(5);

   }

   if (verbose > -1) fprintf(stderr,"timeout handler: outside of While Loop\n");
   pthread_cleanup_pop(0);
   timeout_exit(NULL);
   pthread_exit(NULL);
}

