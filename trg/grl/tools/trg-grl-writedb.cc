/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <trg/grl/dbobjects/TRGGRLConfig.h>

using namespace Belle2;

#define ONLINE 0
//#define ONLINE 1

void setgrlconfig()
{

  const int N_config = 1;

  const int run[N_config][4] = { //itnitial exp, initial run, end exp, end run
    {0,    0,   -1,   -1}  // 0
  };

  float ecltaunn_threshold[N_config] = {
    -1
    };

  DBImportObjPtr<TRGGRLConfig> db_grlconfig;
  db_grlconfig.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_config; i++) {
      IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
      db_grlconfig->set_ecltaunn_threshold(ecltaunn_threshold[i]);
      db_grlconfig.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = N_config - 1; i < N_config; i++) {
      IntervalOfValidity iov(0, 0, -1, -1);
      db_grlconfig->set_ecltaunn_threshold(ecltaunn_threshold[i]);
      db_grlconfig.import(iov);
    }
  }
}

int main()
{

  setgrlconfig();

}


