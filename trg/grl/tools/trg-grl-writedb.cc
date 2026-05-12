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

  //number of network. now it is 1, tauNN only. Maximum 10.
  const int N_config = 1;

  const int run[N_config][4] = { //itnitial exp, initial run, end exp, end run
    {0,    0,   -1,   -1}  // 0
  };

  //number of MVA output and its threshold
  std::vector<unsigned> ecltaunn_nOutput[N_config];
  ecltaunn_nOutput[0].push_back(1);

  std::vector<std::vector<float>> ecltaunn_threshold[N_config];
  std::vector<float> ecltaunn_threshold_0 = {-1.5};
  ecltaunn_threshold[0].push_back(ecltaunn_threshold_0);

  unsigned ecltaunn_nMLP[N_config] = {
    1
  };
  bool ecltaunn_multiplyHidden[N_config] = {
    false
  };
  std::vector<std::vector<float>> ecltaunn_nHidden[N_config];
  std::vector<float> eclaunn_nHidden_0 = {24, 24, 24};
  ecltaunn_nHidden[0].push_back(eclaunn_nHidden_0);
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
        "/group/belle/users/zhaoxy/hls1112/neuralGRL/hls4ml_Latency/firmware/weights/all_weights.txt"
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
        "/group/belle/users/zhaoxy/hls1112/neuralGRL/hls4ml_Latency/firmware/weights/all_bias.txt"
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

  std::vector<std::vector<int>> total_bit_bias[N_config] = {
    {{ 5, 7, 6, 2 }}   // total bits
  };
  std::vector<std::vector<int>> int_bit_bias[N_config] {
    {{ 5, 5, 3, 32 }}   // integer bits
  };
  std::vector<std::vector<bool>> is_signed_bias[N_config] {
    {{ true, true, true, false }}
  };
  std::vector<std::vector<int>> rounding_bias[N_config] {
    {{ 0, 0, 0, 0 }}   // 0 = truncate
  };
  std::vector<std::vector<int>> saturation_bias[N_config] {
    {{ 2, 2, 2, 2 }}   // 2 = saturate
  };

////////////////////////////////////////////////////////////////////////////
  std::vector<std::vector<int>> total_bit_accum[N_config] {
    {{29, 29, 24, 25 }}
  };
  std::vector<std::vector<int>> int_bit_accum[N_config] {
    {{15, 14, 12, 9 }}
  };
  std::vector<std::vector<bool>> is_signed_accum[N_config] {
    {{ true, true, true, true }}
  };
  std::vector<std::vector<int>> rounding_accum[N_config] {
    {{ 0, 0, 0, 0 }}
  };
  std::vector<std::vector<int>> saturation_accum[N_config] {
    {{ 2, 2, 2, 2 }}
  };
////////////////////////////////////////////////////////////////////
  std::vector<std::vector<int>> total_bit_weight[N_config] = {
    {{ 17, 17, 13, 12 }}
  };

  std::vector<std::vector<int>> int_bit_weight[N_config] = {
    {{ 3, 5, 3, 1 }}
  };

  std::vector<std::vector<bool>> is_signed_weight[N_config] = {
    {{ true, true, true, true }}  //
  };

  std::vector<std::vector<int>> rounding_weight[N_config] = {
    {{ 0, 0, 0, 0 }}      // 0 = truncate
  };

  std::vector<std::vector<int>> saturation_weight[N_config] = {
    {{ 2, 2, 2, 2 }}      // 2 = saturate
  };

///////////////////////////////////////////////////////////////////////////////////////
  std::vector<std::vector<int>> total_bit_relu[N_config] = {
    {{ 15, 15, 15 }}  //
  };

  std::vector<std::vector<int>> int_bit_relu[N_config] = {
    {{ 10, 9, 8 }}  //
  };

  std::vector<std::vector<bool>> is_signed_relu[N_config] = {
    {{ false, false, false }}  //
  };

  std::vector<std::vector<int>> rounding_relu[N_config] = {
    {{ 0, 0, 0 }}       // 0 = truncate
  };

  std::vector<std::vector<int>> saturation_relu[N_config] = {
    {{ 2, 2, 2 }}       // 2 = saturate
  };

//////////////////////////////////////////////////////////////////////////////////////
  std::vector<std::vector<int>> total_bit[N_config] {
    {{20, 20, 19}}
  };
  std::vector<std::vector<int>> int_bit[N_config] {
    {{15, 14, 12}}
  };
  std::vector<std::vector<bool>> is_signed[N_config] {
    {{true, true, true}}
  };
  std::vector<std::vector<int>> rounding[N_config] {
    {{0, 0, 0}}
  };
  std::vector<std::vector<int>> saturation[N_config] {
    {{2, 2, 2}}
  };


  std::vector<std::vector<std::vector<int>>> W_input[N_config] {
    {
      {
        { 9, 10, 9, 8, 12, 11, 9, 7, 8, 9, 9, 9, 11, 9, 11, 13, 11, 12, 11, 12, 12, 11, 13, 10 }
        ,
        { 6, 0, 9, 10, 9, 7, 9, 10, 9, 9, 0, 9, 7, 12, 8, 9, 9, 8, 0, 10, 7, 8, 8, 11 }
        ,
        { 11, 8, 0, 8, 11, 12, 11, 11, 12, 9, 12, 11, 10, 10, 12, 12, 0, 12, 12, 9, 12, 10, 8, 11 }

      }
    }
  };
  std::vector<std::vector<std::vector<int>>> I_input[N_config] {
    {
      {
        { 7, 9, 7, 7, 8, 8, 7, 6, 6, 7, 8, 8, 9, 7, 8, 9, 10, 10, 9, 9, 9, 9, 10, 8 }
        ,
        { 5, 0, 7, 7, 6, 6, 7, 8, 7, 6, 0, 6, 6, 9, 6, 6, 6, 6, 0, 7, 6, 6, 5, 6 }
        ,
        { 6, 7, 0, 5, 7, 7, 7, 6, 7, 6, 7, 6, 6, 4, 7, 7, 0, 7, 8, 6, 8, 6, 6, 6 }

      }
    }
  };


  DBImportObjPtr<TRGGRLConfig> db_grlconfig;
  db_grlconfig.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_config; i++) {
      IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
      db_grlconfig->set_ecltaunn_threshold(ecltaunn_threshold[i]);
      db_grlconfig->set_ecltaunn_nMLP(ecltaunn_nMLP[i]);
      db_grlconfig->set_ecltaunn_multiplyHidden(ecltaunn_multiplyHidden[i]);
      db_grlconfig->set_ecltaunn_nHidden(ecltaunn_nHidden[i]);
      db_grlconfig->set_ecltaunn_nOutput(ecltaunn_nOutput[i]);
      db_grlconfig->set_ecltaunn_n_cdc_sector(ecltaunn_n_cdc_sector[i]);
      db_grlconfig->set_ecltaunn_n_ecl_sector(ecltaunn_n_ecl_sector[i]);
      db_grlconfig->set_ecltaunn_i_cdc_sector(ecltaunn_i_cdc_sector[i]);
      db_grlconfig->set_ecltaunn_i_ecl_sector(ecltaunn_i_ecl_sector[i]);
      db_grlconfig->set_ecltaunn_weight(ecltaunn_weight[i]);
      db_grlconfig->set_ecltaunn_bias(ecltaunn_bias[i]);
      db_grlconfig->set_ecltaunn_total_bit_bias(total_bit_bias[i]);
      db_grlconfig->set_ecltaunn_int_bit_bias(int_bit_bias[i]);
      db_grlconfig->set_ecltaunn_is_signed_bias(is_signed_bias[i]);
      db_grlconfig->set_ecltaunn_rounding_bias(rounding_bias[i]);
      db_grlconfig->set_ecltaunn_saturation_bias(saturation_bias[i]);
      db_grlconfig->set_ecltaunn_total_bit_accum(total_bit_accum[i]);
      db_grlconfig->set_ecltaunn_int_bit_accum(int_bit_accum[i]);
      db_grlconfig->set_ecltaunn_is_signed_accum(is_signed_accum[i]);
      db_grlconfig->set_ecltaunn_rounding_accum(rounding_accum[i]);
      db_grlconfig->set_ecltaunn_saturation_accum(saturation_accum[i]);
      db_grlconfig->set_ecltaunn_total_bit_weight(total_bit_weight[i]);
      db_grlconfig->set_ecltaunn_int_bit_weight(int_bit_weight[i]);
      db_grlconfig->set_ecltaunn_is_signed_weight(is_signed_weight[i]);
      db_grlconfig->set_ecltaunn_rounding_weight(rounding_weight[i]);
      db_grlconfig->set_ecltaunn_saturation_weight(saturation_weight[i]);
      db_grlconfig->set_ecltaunn_total_bit_relu(total_bit_relu[i]);
      db_grlconfig->set_ecltaunn_int_bit_relu(int_bit_relu[i]);
      db_grlconfig->set_ecltaunn_is_signed_relu(is_signed_relu[i]);
      db_grlconfig->set_ecltaunn_rounding_relu(rounding_relu[i]);
      db_grlconfig->set_ecltaunn_saturation_relu(saturation_relu[i]);
      db_grlconfig->set_ecltaunn_total_bit(total_bit[i]);
      db_grlconfig->set_ecltaunn_int_bit(int_bit[i]);
      db_grlconfig->set_ecltaunn_is_signed(is_signed[i]);
      db_grlconfig->set_ecltaunn_rounding(rounding[i]);
      db_grlconfig->set_ecltaunn_saturation(saturation[i]);
      db_grlconfig->set_ecltaunn_W_input(W_input[i]);
      db_grlconfig->set_ecltaunn_I_input(I_input[i]);

      db_grlconfig.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = N_config - 1; i < N_config; i++) {
      IntervalOfValidity iov(0, 0, -1, -1);
      db_grlconfig->set_ecltaunn_threshold(ecltaunn_threshold[i]);
      db_grlconfig->set_ecltaunn_nMLP(ecltaunn_nMLP[i]);
      db_grlconfig->set_ecltaunn_multiplyHidden(ecltaunn_multiplyHidden[i]);
      db_grlconfig->set_ecltaunn_nHidden(ecltaunn_nHidden[i]);
      db_grlconfig->set_ecltaunn_nOutput(ecltaunn_nOutput[i]);
      db_grlconfig->set_ecltaunn_n_cdc_sector(ecltaunn_n_cdc_sector[i]);
      db_grlconfig->set_ecltaunn_n_ecl_sector(ecltaunn_n_ecl_sector[i]);
      db_grlconfig->set_ecltaunn_i_cdc_sector(ecltaunn_i_cdc_sector[i]);
      db_grlconfig->set_ecltaunn_i_ecl_sector(ecltaunn_i_ecl_sector[i]);
      db_grlconfig->set_ecltaunn_weight(ecltaunn_weight[i]);
      db_grlconfig->set_ecltaunn_bias(ecltaunn_bias[i]);
      db_grlconfig->set_ecltaunn_total_bit_bias(total_bit_bias[i]);
      db_grlconfig->set_ecltaunn_int_bit_bias(int_bit_bias[i]);
      db_grlconfig->set_ecltaunn_is_signed_bias(is_signed_bias[i]);
      db_grlconfig->set_ecltaunn_rounding_bias(rounding_bias[i]);
      db_grlconfig->set_ecltaunn_saturation_bias(saturation_bias[i]);
      db_grlconfig->set_ecltaunn_total_bit_accum(total_bit_accum[i]);
      db_grlconfig->set_ecltaunn_int_bit_accum(int_bit_accum[i]);
      db_grlconfig->set_ecltaunn_is_signed_accum(is_signed_accum[i]);
      db_grlconfig->set_ecltaunn_rounding_accum(rounding_accum[i]);
      db_grlconfig->set_ecltaunn_saturation_accum(saturation_accum[i]);
      db_grlconfig->set_ecltaunn_total_bit_weight(total_bit_weight[i]);
      db_grlconfig->set_ecltaunn_int_bit_weight(int_bit_weight[i]);
      db_grlconfig->set_ecltaunn_is_signed_weight(is_signed_weight[i]);
      db_grlconfig->set_ecltaunn_rounding_weight(rounding_weight[i]);
      db_grlconfig->set_ecltaunn_saturation_weight(saturation_weight[i]);
      db_grlconfig->set_ecltaunn_total_bit_relu(total_bit_relu[i]);
      db_grlconfig->set_ecltaunn_int_bit_relu(int_bit_relu[i]);
      db_grlconfig->set_ecltaunn_is_signed_relu(is_signed_relu[i]);
      db_grlconfig->set_ecltaunn_rounding_relu(rounding_relu[i]);
      db_grlconfig->set_ecltaunn_saturation_relu(saturation_relu[i]);
      db_grlconfig->set_ecltaunn_total_bit(total_bit[i]);
      db_grlconfig->set_ecltaunn_int_bit(int_bit[i]);
      db_grlconfig->set_ecltaunn_is_signed(is_signed[i]);
      db_grlconfig->set_ecltaunn_rounding(rounding[i]);
      db_grlconfig->set_ecltaunn_saturation(saturation[i]);
      db_grlconfig->set_ecltaunn_W_input(W_input[i]);
      db_grlconfig->set_ecltaunn_I_input(I_input[i]);

      db_grlconfig.import(iov);
    }
  }
}

int main()
{

  setgrlconfig();

}


