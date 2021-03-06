#ifndef  __COMMON_H__
#define  __COMMON_H__

/*#include "InfluxDBFactory.h"
#include "Transport.h"
#include "Point.h"
#include "TFile.h"
#include "TTree.h"
#include "TNtuple.h"
*/
#include <iostream>
/*
#ifdef USE_INFLUXDB
#include "InfluxDBFactory.h"
#include "Transport.h"
#include "Point.h"

using namespace influxdb;
extern std::unique_ptr<InfluxDB> influxdb_conn;
int write_pulse_height_event_influxdb(int chan, int flags, int timestamp, int evadcdata);
#endif

extern TFile *root_file;
extern TNtuple *myntuple;
extern TTree *myttree;
*/
int report_counts(int interval, std::string daq_prefix, int MAX_CHANNELS, int addr_count[], unsigned int event_count);
int write_pulse_height_event(std::string daq_prefix, int daq_chan, int flags, unsigned long long timestamp, int evadcdata);
int write_pulse_height_event_root(int chan, int flags, unsigned long long timestamp, int evadcdata);

#endif
