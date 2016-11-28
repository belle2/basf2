//+
// File : PrintEventRate.cc
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 18 - Jun - 2015
//-

#include <rawdata/modules/PrintEventRate.h>
//#include <daq/rawdata/modules/DAQConsts.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <framework/core/InputController.h>


using namespace std;
using namespace Belle2;

#define EVENT_UNIT 75000

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PrintEventRate)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PrintEventRateModule::PrintEventRateModule() : PrintDataTemplateModule()
{
  prev_utime = 0;
  prev_event = 0;
  start_event = 0;
  start_utime = 0;
  m_run = 0;
  m_cur_utime = 0;
  m_cur_event = 0;
  m_size = 0;
}



PrintEventRateModule::~PrintEventRateModule()
{

}


void PrintEventRateModule::printCOPPEREvent(RawCOPPER* raw_copper, int i)
{
  m_cur_utime = raw_copper->GetTTUtime(i);
  m_cur_event = raw_copper->GetEveNo(i);
  m_run = raw_copper->GetRunNo(i);


  // printf("EVe utime %u %u %u\n", m_cur_event, n_basf2evt, start_utime);
  // fflush(stdout);
  if (start_utime != 0) {
    if (((n_basf2evt + 1) % EVENT_UNIT) == 0) {
      m_size = raw_copper->GetBlockNwords(i);
      printf("utime %u run %d eve %u intv %5d [s] blk %d rate %.2lf [Hz] %u blkwords %d\n",
             m_cur_utime, m_run, m_cur_event, (m_cur_utime - prev_utime), m_ncpr,
             (double)(m_cur_event - prev_event) / (m_cur_utime - prev_utime), start_utime, m_size);
      fflush(stdout);
      prev_utime = m_cur_utime;
      prev_event = m_cur_event;
    }
  } else {
    start_utime = m_cur_utime;
    start_event = m_cur_event;
    prev_utime = m_cur_utime;
    prev_event = m_cur_event;
    m_size = raw_copper->GetBlockNwords(i);
  }
}

void PrintEventRateModule::endRun()
{
  if (prev_utime != 0) {
    printf("END utime %u run %d eve %u intv %5d [s] blk %d rate %.2lf [Hz] runtime %u start time %u eve %u blknwords %d\n",
           m_cur_utime, m_run, m_cur_event, (m_cur_utime - prev_utime), m_ncpr,
           (double)(m_cur_event - start_event) / (m_cur_utime - start_utime),
           m_cur_utime - start_utime, start_utime, start_event, m_size) ;

  } else {
    printf("ENAD utime\n");
  }
  fflush(stdout);
}

void PrintEventRateModule::event()
{

  StoreArray<RawECL> raw_eclarray;
  StoreArray<RawKLM> raw_klmarray;
  StoreArray<RawCOPPER> rawcprarray;

  int klm_blknum = raw_klmarray.getEntries();
  int ecl_blknum = raw_eclarray.getEntries();
  int cpr_blknum = rawcprarray.getEntries();

  m_ncpr = klm_blknum + ecl_blknum + cpr_blknum;

  if (klm_blknum > 0) {
    printCOPPEREvent(raw_klmarray[ 0 ], 0);
  } else if (ecl_blknum > 0) {
    printCOPPEREvent(raw_eclarray[ 0 ], 0);
  } else if (cpr_blknum > 0) {
    printCOPPEREvent(rawcprarray[ 0 ], 0);
  }

  //  printf("loop %d\n", n_basf2evt);
  n_basf2evt++;

}
