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

  const int N_config = 2;

  const int run[N_config][4] = { //itnitial exp, initial run, end exp, end run
    0,    0,   -1,   -1, // 0
    9, 1400,   -1,   -1  // 1
  };

  int nTS[N_config] = {
    10, 15
  };

  DBImportObjPtr<CDCTrigger2DConfig> db_2dconfig;
  db_2dconfig.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_config; i++) {
      IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
      db_2dconfig->setnTS(nTS[i]);
      db_2dconfig.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = N_config - 1; i < N_config; i++) {
      IntervalOfValidity iov(9, 0, -1, -1);
      db_2dconfig->setnTS(nTS[i]);
      db_2dconfig.import(iov);
    }
  }
}

int main()
{

  set2dconfig();

}


