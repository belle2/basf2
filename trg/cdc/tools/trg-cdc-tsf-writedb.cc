/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <trg/cdc/dbobjects/CDCTriggerTSFConfig.h>

using namespace Belle2;

#define ONLINE 0
//#define ONLINE 1

void settsfconfig()
{

  const int N_config = 3;

  const int run[N_config][4] = { //itnitial exp, initial run, end exp, end run
    {0,     0,   10, 3929}, // 0
    {10, 3930,   34,   -1}, // 1 TDC cross talk filter
    {35,    0,   -1,   -1}  // 2 ADC
  };

  bool TDCfilter[N_config] = {
    false,  false,  false
    //false,  true,  true, //this is consistent with experiment. it is disabled on TSIM to reduce CPU time.
  };
  bool ADC[N_config] = {
    false,  false,  true
  };
  int ADC_threshold[N_config] = {
    -1, -1, 10
    };

  DBImportObjPtr<CDCTriggerTSFConfig> db_tsfconfig;
  db_tsfconfig.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_config; i++) {
      IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
      db_tsfconfig->setTDCfilter(TDCfilter[i]);
      db_tsfconfig->setADC(ADC[i]);
      db_tsfconfig->setADC_threshold(ADC_threshold[i]);
      db_tsfconfig.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = N_config - 1; i < N_config; i++) {
      IntervalOfValidity iov(0, 0, -1, -1);
      db_tsfconfig->setTDCfilter(TDCfilter[i]);
      db_tsfconfig->setADC(ADC[i]);
      db_tsfconfig->setADC_threshold(ADC_threshold[i]);
      db_tsfconfig.import(iov);
    }
  }
}

int main()
{

  settsfconfig();

}


