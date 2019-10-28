#include <stdio.h>
#include <stdlib.h> /* rand() */
#include <time.h>
#include <unistd.h> /* usleep */
#include <math.h>
#include <string.h>  /* memset */
#include <stdint.h>

#include "midas.h"
#include "histogram.h"
#include "web_server.h"
#include "experim.h"

//#include <TH1F.h>
//#include <TH2F.h>
//#include <TTree.h>
//#include <TDirectory.h>

#define NUM_ODB_CHAN     459 // size of msc table in odb
#define MAX_SAMPLE_LEN  4096
#define ENERGY_BINS    65536 /* 65536 131072 262144 */
#define NUM_CLOVER        16
#define MAX_CHAN        1024
#define STRING_LEN       256
#define MIDAS_STRLEN      32
#define MAX_ADDRESS  0x10000
#define E_SPEC_LENGTH   8192
#define T_SPEC_LENGTH   8192
#define WV_SPEC_LENGTH  4096
#define N_HITPAT 5
#define N_SUM 5

static short       waveform[MAX_SAMPLE_LEN];
static int        rate_data[MAX_CHAN];
//static float          gains[MAX_CHAN];
//static float        offsets[MAX_CHAN];
static char       chan_name[MAX_CHAN][MIDAS_STRLEN];
static short   chan_address[MAX_CHAN];
static int     num_chanhist;
static short   address_chan[MAX_ADDRESS];


static int debug; // only accessible through gdb
//TH1I *hit_hist[N_HITPAT];

/*-- Module declaration --------------------------------------------*/
int mdpp16_event(EVENT_HEADER *, void *);
int mdpp16_init(void);
int mdpp16_bor(INT run_number);
int mdpp16_eor(INT run_number);
static void odb_callback(INT hDB, INT hseq, void *info) { }

ANA_MODULE mdpp16_module = {
  "mdpp16",               /* module name           */
  "CP",                   /* author                */
  mdpp16_event,          /* event routine         */
  mdpp16_bor,            /* BOR routine           */
  mdpp16_eor,            /* EOR routine           */
  mdpp16_init,           /* init routine          */
  NULL,                   /* exit routine          */
  NULL, //&mdpp16_settings,         /* parameter structure   */
  0,    //sizeof(mdpp16_settings),  /* structure size        */
  NULL, //mdpp16_settings_str,      /* initial parameters    */
};
/*-------------------------------------------------------------------*/

#define NUM_CHAN    16
#define HIT_CHAN    64
#define ADC_CHAN    65536

// Declare histograms here. TH1F = 4 bytes/cell (float), TH1D = 8 bytes/cell (double)
//static TH1F *hHitPat, *hEnergy[NUM_CHAN], *hEnergy_flagsrm[NUM_CHAN], *hTDC[NUM_CHAN], *hRate[NUM_CHAN];
//static TH1F *hCalEnergy1[NUM_CHAN];
//static TH2F *hEnergy_vs_ts[NUM_CHAN];


extern TH1I **hit_hist;
extern TH1I **sum_hist;
//extern TH1I *ph_hist;
extern TH1I **e_hist;
extern TH1I **cfd_hist;
extern TH1I **wave_hist;
TH1I *ph_hist_mdpp[MAX_CHAN];

int hist_init_roody();
int hist_mdpp_init();
//---------------------------------------------------------------------

int mdpp16_bor(INT run_number) { return SUCCESS; }
int mdpp16_eor(INT run_number) { return SUCCESS; }

extern HNDLE hDB;                     /* Odb Handle */
MDPP16_ANALYSER_PARAMETERS ana_param;                    /* Odb settings */
MDPP16_ANALYSER_PARAMETERS_STR(ana_param_str);     /* Book Setting space */

int mdpp16_init(void)
{
  char odb_str[128], errmsg[128];
  char set_str[80];
  int size, status;
  HNDLE hSet;
  printf("Were init'ing the crap out of the MDPP16 stuff...\n");
  size = sizeof(MDPP16_ANALYSER_PARAMETERS);

  sprintf(set_str, "/mdpp16_analyser/Parameters");
  status = db_create_record(hDB, 0, set_str, strcomb(ana_param_str));
  if ( (status = db_find_key (hDB, 0, set_str, &hSet)) != DB_SUCCESS ) {
    cm_msg(MINFO, "FE", "Key %s not found", set_str);
  }
  if ((status = db_open_record(hDB, hSet, &ana_param, size, /* enable link */
                               MODE_READ, NULL, NULL)) != DB_SUCCESS ) {
    cm_msg(MINFO, "FE", "Failed to enable ana param hotlink", set_str);

  }

  //hist_init_roody();
  hist_mdpp_init();
  return SUCCESS;
}



int hist_mdpp_init()
{
  printf("Do we get here?!?\n");
  char hit_titles[N_HITPAT][32] = {
    "HITPATTERN_Energy",   "HITPATTERN_Time",
    "HITPATTERN_Waveform", "HITPATTERN_Pulse_Height", "HITPATTERN_Rate"
  };
  char sum_titles[N_SUM][32] = { "SUM_Singles_Low_gain_Energy", "SUM_Singles_High_gain_Energy", "SUM_Addback_Energy", "SUM_PACES_Energy", "SUM_LaBr3_Energy"};
  char hit_names[N_HITPAT][32] = {"e_hit", "t_hit", "w_hit", "q_hit", "r_hit"};
  char sum_names[N_SUM][32] = {"el_sum", "eh_sum", "a_sum", "p_sum", "l_sum"};
  char title[STRING_LEN], handle[STRING_LEN];
  int i;
  for (i = 0; i < MAX_CHAN; i++) { // Create each histogram for this channel

   // if ( i >= num_chanhist ) { break; }
     //     printf("222Do we get here?!?\n");
    sprintf(chan_name[i], "mdpp16_%i", i);
    printf("%d = %d[0x%08x]: %s\n", i, chan_address[i], chan_address[i], chan_name[i]);
    sprintf(title,  "%s_Pulse_Height",   chan_name[i] );
    sprintf(handle, "%s_Q",              chan_name[i] );
    ph_hist_mdpp[i] = H1_BOOK(handle, title, ENERGY_BINS, 0, ENERGY_BINS);
    //      printf("Lets see if we crash..%i\n", ph_hist_mdpp[0]);
      //ph_hist_mdpp[0] -> GetBinContent(ph_hist_mdpp[0],0);

//    ph_hist[i] = H1_BOOK(handle, title, E_SPEC_LENGTH, 0, E_SPEC_LENGTH);
/* JONR
    sprintf(title,  "%s_Energy",         chan_name[i] );
    sprintf(handle, "%s_E",              chan_name[i] );
    e_hist[i] = H1_BOOK(handle, title, E_SPEC_LENGTH, 0, E_SPEC_LENGTH);
    sprintf(title,  "%s_Time",           chan_name[i] );
    sprintf(handle, "%s_T",              chan_name[i] );
    cfd_hist[i] = H1_BOOK(handle, title, T_SPEC_LENGTH, 0, T_SPEC_LENGTH);
    sprintf(title,  "%s_Waveform",       chan_name[i] );
    sprintf(handle, "%s_w",              chan_name[i] );
    wave_hist[i] = H1_BOOK(handle, title, WV_SPEC_LENGTH, 0, WV_SPEC_LENGTH);
  }
  for (i = 0; i < N_HITPAT; i++) { // Create Hitpattern spectra
    hit_hist[i] = H1_BOOK(hit_names[i], hit_titles[i], MAX_CHAN, 0, MAX_CHAN);
  }
  for (i = 0; i < N_SUM; i++) { // Create Sum spectra
    sum_hist[i] = H1_BOOK(sum_names[i], sum_titles[i], E_SPEC_LENGTH, 0, E_SPEC_LENGTH);
  } JONR END*/
    }
  return (0);
}

int mdpp16_event(EVENT_HEADER *pheader, void *pevent)
{
  /* BeginTime needs to be global? startTime should be set to 0 at the beginning of each event? and then
  something something...  */
  int i, bank_len, err = 0;
  DWORD *data;

  int hsig, subhead, mod_id, tdc_res, adc_res, nword;
  int dsig, fix, flags = 0, t, chan, evdata;
  uint32_t ts; // needed for 30-bit ts
  int esig, counter;
  int evadcdata, evtdcdata, evrstdata, extts, trigchan;
  static int evcount;

  /* Added these to give a time interval to accrue counts. Current bugs:
     - drop in count rate at regular intervals. For INTERVAL=5, counts drop every 5 bins... odd
         DOES depend on interval and can start in the middle of the 5 bin set
     - does not reset to 0 between runs, requires analyzer be reset.
   */
  static time_t startTime, beginTime;
  time_t currentTime = time(NULL);
  static int rates[NUM_CHAN];
  float cal_energy;
  printf("We got an MDPP16 event!\n");
  // Added this chunk for the count rate vs time histogram. startTime is run, beginTime is interval
  if ( startTime == 0 ) {
    startTime = beginTime = currentTime;
  } // equivalent to beginTime=currentTime; startTime=beginTime;
  if ( currentTime - startTime > ana_param.update_interval ) {
    for (i = 0; i < NUM_CHAN; i++) {
      // If enough time elapsed, populate hRate at beginning of event with previous values
      //JON FIXME hRate[i] -> SetBinContent((currentTime-beginTime)/ana_param.update_interval, rates[i]);
      rates[i] = 0;

    }
    startTime = currentTime;
  }

  // bank_len defined here. bk_locate(event,name,pdata) finds "MDPP" in event and returns bank length
  if ( (bank_len = bk_locate(pevent, "MDPP", &data) ) == 0 ) { return (0); }
  ++evcount;
  debug = 1;
  if ( debug ) {
    printf("Event Dump ...\n");
    for (i = 0; i < bank_len; i += 4) {
      printf("   word[%d] = 0x%08x    0x%08x    ", i, data[i], data[i + 1]);
      printf("0x%08x    0x%08x    \n", data[i + 2], data[i + 3] );
    }
  }

  // hsig 2 + subhead 2 + xxxx + mod_id 8 + tdc_res 3 + adc_res 3 + num of following 4byte words incl EOE 10
  hsig    = (data[0] >> 30) & 0x3;
  subhead = (data[0] >> 24) & 0x3F;
  mod_id  = (data[0] >> 16) & 0xFF;
  tdc_res = (data[0] >> 13) & 0x7;
  adc_res = (data[0] >> 10) & 0x7;
  nword   = (data[0] >> 0) & 0x3FF;

  if ( debug ) {
    printf("Header ...\n");
    printf("   hsig  =%d    subheader=%d    mod_id=%d\n", hsig, subhead, mod_id);
    printf("   tdc_res=%d    adc_res=%d    nword=%d\n", tdc_res, adc_res, nword);
  }

  for (i = 1; i < bank_len; i++) { // covers both ADC and TDC event words
    /* Highest 4 bits:
          0100 for header
          0001 for data event
          0010 for ext ts
          11   for EOE mark (30-bit ts)
          0000 for fill dummy */
    dsig    = (data[i] >> 30) & 0x3;
    fix     = (data[i] >> 28) & 0x3;

    // DATA word for TDC, ADC or reset event. 0b0001
    if ( ( (data[i] >> 28) & 0xF) == 1 ) {

      flags    = (data[i] >> 22) & 0x3; // pileup or overflow/underflow
      trigchan = (data[i] >> 16) & 0x3F; // All 6 bits for determining ADC, TDC or trig0/trig1 (reset)

      if ( trigchan == 33 ) { // Reset event is 33 on 6 bits: 100001
        evrstdata = (data[i] >> 0) & 0xF; // channel index, 4 bits
        //JON  hHitPat->Fill(evrstdata); // Fill on reset events
      }

      // hHitPat->Fill( chan );

      else if (     trigchan < 16 ) { // ADC value caught
        chan      = (data[i] >> 16) & 0x1F;
        evadcdata = (data[i] >> 0 ) & 0xFFFF;
      }
      else if (trigchan < 32) { // TDC time difference caught if above 16 and less than 32
        evtdcdata = (data[i] >> 0 ) & 0xFFFF;
      }
    }

    // Extended timestamp word. If dsig+fix==0010, ext ts caught
    if ( ((data[i] >> 28) & 0xF) == 2 ) {
      extts = (data[i] >> 0) & 0xFFFF;
    }

    // EOE marker, event counter / timestamp. If dsig == 0b11, ts caught
    if ( dsig == 3 ) {
      ts =  ((data[i] >> 0) & 0x3FFFFFFF); // concatenate 14 bits and 16 bits...
    }
    //hEnergy          [chan]-> Fill(evadcdata); // raw ADC
  }
    printf("Is it just a flag thing? If it got here but not the end one it is..%i\n", evadcdata);
    if (flags == 0) {
//JON     hEnergy_flagsrm[chan]-> Fill(evadcdata); // raw ADC, flags removed
      printf("Adding entry for energy hit %i on channel : %i\n", evadcdata, chan);
       //ph_hist[chan]-> Fill(evadcdata); // raw ADC, flags removed
   //   ph_hist[chan] -> Fill(ph_hist[chan],  (int)15,     1);
      printf("Lets see if we crash..%i\n", ph_hist_mdpp[chan]);
      //ph_hist_mdpp[0] -> GetBinContent(ph_hist_mdpp[0],0);

      //ph_hist_mdpp[0] -> Fill(ph_hist_mdpp[0],  1,     1);

      ph_hist_mdpp[chan] -> Fill(ph_hist_mdpp[chan],  (int)evadcdata,     1);
      printf("Do we crash here?\n");
    }
//JON  hEnergy_vs_ts    [chan]-> Fill(evadcdata, ts/16000000);
    //JON hTDC             [chan]-> Fill(evtdcdata); // TDC value (event time after window opened)
    //hCalEnergy1      [chan]->Fill((evadcdata-ana_param.peak1_channel)*(ana_param.peak2_energy-ana_param.peak1_energy)/(ana_param.peak2_channel - ana_param.peak1_channel )+ ana_param.peak1_energy);


    //hHitPat[chan   ]-> Fill(chan);
    /* if (flags==0){ // Fill ADC data only when no flags are on */
    /*   hEnergy_flagsrm[chan]-> Fill(evadcdata); */
    /* } */
    esig    = (data[i] >> 30) & 0x3;
    counter = (data[i] >> 0) & 0x3FFFFFFF; // low 30bits
    if ( debug ) {
      printf("Trailer ...\n");
      printf("   esig  =%d    counter=%d\n", esig, counter);
      printf("\n");
    }

    if ( hsig != 1 || esig != 3 || t != 0 || subhead != 0 || mod_id != 1 ) {
      err = 1;
    }
    //if( err == 1 ){ printf("Error: event %d\n", evcount); }
    return (0);
  }