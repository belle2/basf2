//+
// File : PrintEventRate.cc
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 18 - Jun - 2015
//-
#include <daq/rawdata/modules/DAQConsts.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <daq/rawdata/modules/PrintEventRate.h>
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
}



PrintEventRateModule::~PrintEventRateModule()
{
}


void PrintEventRateModule::printCOPPEREvent(RawCOPPER* raw_copper, int i)
{
  unsigned int cur_utime = raw_copper->GetTTUtime(i);
  unsigned int cur_event = raw_copper->GetEveNo(i);
  unsigned int run = raw_copper->GetRunNo(i);

  if (cur_event != 1) {
    printf("utime %u run %d eve %u intv %5d [s] blk %d rate %.2lf [Hz]\n",
           cur_utime, run, cur_event, (cur_utime - prev_utime), m_ncpr,
           (double)(cur_event - prev_event) / (cur_utime - prev_utime));
  }
  prev_utime = cur_utime;
  prev_event = cur_event;


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

  if ((n_basf2evt % EVENT_UNIT) == 0) {
    if (klm_blknum > 0) {
      printCOPPEREvent(raw_klmarray[ 0 ], 0);
    }
    if (ecl_blknum > 0) {
      printCOPPEREvent(raw_eclarray[ 0 ], 0);
    }
    if (cpr_blknum > 0) {
      printCOPPEREvent(rawcprarray[ 0 ], 0);
    }
  }

  //  printf("loop %d\n", n_basf2evt);
  n_basf2evt++;

}
