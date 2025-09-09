/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <trg/cdc/dbobjects/CDCTrigger2DConfig.h>

using namespace Belle2;

#define ONLINE 0
//#define ONLINE 1

void set2dconfig()
{

  const int N_config = 4;

  const int run[N_config][4] = { //itnitial exp, initial run, end exp, end run
    {0,     0,    9, 1399}, // 0
    {9,  1400,   29,   -1}, // 1 15 TS
    {30,    0,   34,   -1}, // 2 full hit 2D
    {35,    0,   -1,   -1}, // 3 ADC
  };

  int nTS[N_config] = {
    10, 15, 15, 15
  };
  bool fullhit[N_config] = {
    false,  false,  true,  true
  };
  int hitthreshold[N_config] = {
    4,  4,  16,  16
  };
  bool ADC[N_config] = {
    false,  false,  false,  true
  };

  DBImportObjPtr<CDCTrigger2DConfig> db_2dconfig;
  db_2dconfig.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_config; i++) {
      IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
      db_2dconfig->setnTS(nTS[i]);
      db_2dconfig->setfullhit(fullhit[i]);
      db_2dconfig->sethitthreshold(hitthreshold[i]);
      db_2dconfig->setADC(ADC[i]);
      db_2dconfig.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = N_config - 1; i < N_config; i++) {
      IntervalOfValidity iov(0, 0, -1, -1);
      db_2dconfig->setnTS(nTS[i]);
      db_2dconfig->setfullhit(fullhit[i]);
      db_2dconfig->sethitthreshold(hitthreshold[i]);
      db_2dconfig->setADC(ADC[i]);
      db_2dconfig.import(iov);
    }
  }
}

int main()
{

  set2dconfig();

}


