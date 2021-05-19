#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "common.h"
#include "midas.h"
#include "web_server.h"
#include "hist_output.h"


/*#include "TH1.h"
#include "TH1D.h"
#include "TFile.h"*/
#undef USE_INFLUXDB
extern ANA_MODULE griffin_module;
extern ANA_MODULE mdpp16_module;

unsigned int mdpp16_temporal_hist[MDPP_CHAN_NUM][HIST_SIZE] = {};
unsigned int timer_thread_termination = 0;
unsigned long long mdpp16_tdc_last_time;


char *analyzer_name = "Analyzer"; /* The analyzer name (client name)   */
INT analyzer_loop_period = 0; /* analyzer_loop call interval[ms](0:disable) */
INT odb_size = DEFAULT_ODB_SIZE; /* default ODB size */

//ANA_MODULE *trigger_module[] = { &griffin_module, NULL };
ANA_MODULE *griffin_trigger_module[] = { &griffin_module, NULL };
ANA_MODULE *mdpp_trigger_module[] = { &mdpp16_module, NULL };

BANK_LIST griffin_ana_trigger_bank_list[] = { /* online banks */
        {"GRF4", TID_DWORD, 256, NULL}, {""},
};
BANK_LIST mdpp_ana_trigger_bank_list[] = { /* online banks */
        {"MDPP", TID_DWORD, 256, NULL}, {""},
};
//BANK_LIST ana_trigger_bank_list[] = { /* online banks */
//  {"GRF3", TID_DWORD, 256, NULL}, {""} , {"GRF4", TID_DWORD, 256, NULL}, {"MDPP", TID_DWORD, 256, NULL}, {""}
//};

ANALYZE_REQUEST analyze_request[] = {
        {"GRIF16",               /* equipment name */
         {200,                    /* event ID */
       TRIGGER_ALL,             /* trigger mask */
       //GET_SOME,                  /* get some events */ /* Removed ?? */
       GET_NONBLOCKING,         /* get some events */ /* later midas's */
       "SYSTEM",                /* event buffer */
       TRUE,                    /* enabled */
       "", "",},
         NULL,                  /* analyzer routine */
         griffin_trigger_module,        /* module list */
         griffin_ana_trigger_bank_list, /* bank list */
         1000,                  /* RWNT buffer size */
         TRUE,                  /* Use tests for this event */
        },

        {"MDPP16",              /* equipment name */
         {400,                    /* event ID */
      TRIGGER_ALL,              /* trigger mask */
      //GET_SOME,                  /* get some events */ /* Removed ?? */
      GET_NONBLOCKING,          /* get some events */ /* later midas's */
      "SYSTEM",                 /* event buffer */
      TRUE,                     /* enabled */
      "", "",},
         NULL,                  /* analyzer routine */
         mdpp_trigger_module,        /* module list */
         mdpp_ana_trigger_bank_list, /* bank list */
         1000,                  /* RWNT buffer size */
         TRUE,                  /* Use tests for this event */
        },

        {""}
};

/*--------------------------------------------------------------------------*/

volatile int shutdown_webserver;
static pthread_t web_thread;
static pthread_t timer_thread;

INT analyzer_init(){
        int a1=1;
        pthread_create(&web_thread, NULL,(void* (*)(void*))web_server_main, &a1);
        return SUCCESS;
}

INT analyzer_exit(){
        printf("waiting for server to shutdown ...\n");
        shutdown_webserver = 1;
        pthread_join(web_thread, NULL);
        return CM_SUCCESS;
}

INT ana_begin_of_run(INT run_number, char *error){
  printf("Starting Run: %i\n", run_number);

  #ifdef USE_REDIS
      long redis_output_timing = 1000;  // milliseconds
      timer_thread_termination = 0;
      pthread_create(&timer_thread, NULL,(void* (*)(void*))hist_timer, (void*) redis_output_timing);
  #endif

  return CM_SUCCESS;
}
INT ana_end_of_run(INT run_number, char *error){
  //root_file->Write();
  //root_file->Close();
  #ifdef USE_REDIS
      void *thread_status;
      timer_thread_termination = 1;
      pthread_join(timer_thread, &thread_status);
  #endif

  return CM_SUCCESS;
}
INT ana_pause_run(INT run_number, char *error){
        return CM_SUCCESS;
}
INT ana_resume_run(INT run_number, char *error){
        return CM_SUCCESS;
}
INT analyzer_loop(){
        return CM_SUCCESS;
}
