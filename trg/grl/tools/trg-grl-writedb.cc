/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <trg/grl/dbobjects/TRGGRLConfig.h>
#include <fstream>

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
  unsigned ecltaunn_nMLP[N_config] = {
    1
  };
  bool ecltaunn_multiplyHidden[N_config] = {
    false
  };
  std::vector<std::vector<float>> ecltaunn_nHidden[N_config];
  ecltaunn_nHidden[0].push_back(std::vector<float>(2, 64));
  unsigned ecltaunn_n_cdc_sector[N_config] = {
    0
  };
  unsigned ecltaunn_n_ecl_sector[N_config] = {
    1
  };
  std::vector<float> ecltaunn_i_cdc_sector[N_config];
  ecltaunn_i_cdc_sector[0].push_back(0 * 35 * 3);

  std::vector<float> ecltaunn_i_ecl_sector[N_config]; //only 24 is supported for now
  ecltaunn_i_ecl_sector[0].push_back(1 * 6 * 4);

  char wfilename[N_config][10][1000] = {{{
        "/home/t2k/taichiro/public/grl/weights.dat"
      }
    }
  };
  std::vector<std::vector<float>> ecltaunn_weight[N_config];
  for (int i = 0; i < N_config; i++) {
    for (unsigned j = 0; j < ecltaunn_nMLP[i]; j++) {
      std::ifstream wfile(wfilename[i][j]);
      if (!wfile.is_open()) {
        return;
      } else {
        float element;
        std::vector<float> ecltaunn_weight_temp;
        while (wfile >> element) {
          ecltaunn_weight_temp.push_back(element);
        }
        ecltaunn_weight[i].push_back(ecltaunn_weight_temp);
      }
    }
  }

  char bfilename[N_config][10][1000] = {{{
        "/home/t2k/taichiro/public/grl/bias.dat"
      }
    }
  };
  std::vector<std::vector<float>> ecltaunn_bias[N_config];
  for (int i = 0; i < N_config; i++) {
    for (unsigned j = 0; j < ecltaunn_nMLP[i]; j++) {
      std::ifstream bfile(bfilename[i][j]);
      if (!bfile.is_open()) {
        return;
      } else {
        float element;
        std::vector<float> ecltaunn_bias_temp;
        while (bfile >> element) {
          ecltaunn_bias_temp.push_back(element);
        }
        ecltaunn_bias[i].push_back(ecltaunn_bias_temp);
      }
    }
  }

  DBImportObjPtr<TRGGRLConfig> db_grlconfig;
  db_grlconfig.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_config; i++) {
      IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
      db_grlconfig->set_ecltaunn_threshold(ecltaunn_threshold[i]);
      db_grlconfig->set_ecltaunn_nMLP(ecltaunn_nMLP[i]);
      db_grlconfig->set_ecltaunn_multiplyHidden(ecltaunn_multiplyHidden[i]);
      db_grlconfig->set_ecltaunn_nHidden(ecltaunn_nHidden[i]);
      db_grlconfig->set_ecltaunn_n_cdc_sector(ecltaunn_n_cdc_sector[i]);
      db_grlconfig->set_ecltaunn_n_ecl_sector(ecltaunn_n_ecl_sector[i]);
      db_grlconfig->set_ecltaunn_i_cdc_sector(ecltaunn_i_cdc_sector[i]);
      db_grlconfig->set_ecltaunn_i_ecl_sector(ecltaunn_i_ecl_sector[i]);
      db_grlconfig->set_ecltaunn_weight(ecltaunn_weight[i]);
      db_grlconfig->set_ecltaunn_bias(ecltaunn_bias[i]);
      db_grlconfig.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = N_config - 1; i < N_config; i++) {
      IntervalOfValidity iov(0, 0, -1, -1);
      db_grlconfig->set_ecltaunn_threshold(ecltaunn_threshold[i]);
      db_grlconfig->set_ecltaunn_nMLP(ecltaunn_nMLP[i]);
      db_grlconfig->set_ecltaunn_multiplyHidden(ecltaunn_multiplyHidden[i]);
      db_grlconfig->set_ecltaunn_nHidden(ecltaunn_nHidden[i]);
      db_grlconfig->set_ecltaunn_n_cdc_sector(ecltaunn_n_cdc_sector[i]);
      db_grlconfig->set_ecltaunn_n_ecl_sector(ecltaunn_n_ecl_sector[i]);
      db_grlconfig->set_ecltaunn_i_cdc_sector(ecltaunn_i_cdc_sector[i]);
      db_grlconfig->set_ecltaunn_i_ecl_sector(ecltaunn_i_ecl_sector[i]);
      db_grlconfig->set_ecltaunn_weight(ecltaunn_weight[i]);
      db_grlconfig->set_ecltaunn_bias(ecltaunn_bias[i]);
      db_grlconfig.import(iov);
    }
  }
}

int main()
{

  setgrlconfig();

}


