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
    -1.5
    };
  unsigned ecltaunn_nMLP[N_config] = {
    1
  };
  bool ecltaunn_multiplyHidden[N_config] = {
    false
  };
  std::vector<std::vector<float>> ecltaunn_nHidden[N_config];
  std::vector<float> eclaunn_nHidden_0 = {64, 64, 64};  // hidden layer 0: 64 neurons, hidden layer 1: 64 neurons, hidden layer 2: 64 neurons, matching firmware 24x64x64x64x10 structure
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
        "/home/belle2/liuzd/group/zhaoxinyue/development/trg/grl/data/all_weights.txt"
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
        "/home/belle2/liuzd/group/zhaoxinyue/development/trg/grl/data/all_bias.txt"
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
    {{ 12, 12, 12, 12 }}   // total bits: all layers use b0=12 from firmware (kbi quantization)
  };
  std::vector<std::vector<int>> int_bit_bias[N_config] {
    {{ 6, 6, 6, 6 }}   // integer bits: all layers use i0=6 from firmware (kbi quantization)
  };
  std::vector<std::vector<bool>> is_signed_bias[N_config] {
    {{ true, true, true, true }}  // all biases are signed
  };
  std::vector<std::vector<int>> rounding_bias[N_config] {
    {{ 1, 1, 1, 1 }}   // 1 = RND (round to nearest), matching firmware
  };
  std::vector<std::vector<int>> saturation_bias[N_config] {
    {{ 2, 2, 2, 2 }}   // 2 = SAT_SYM (symmetric saturation), matching firmware
  };

////////////////////////////////////////////////////////////////////////////
  std::vector<std::vector<int>> total_bit_accum[N_config] {
    {{ 18, 18, 18, 18 }}   // total bits for accumulator: input_bits(16) + weight_bits(12) - 10 = 18
  };
  std::vector<std::vector<int>> int_bit_accum[N_config] {
    {{ 12, 12, 12, 12 }}   // integer bits for accumulator: input_int(6) + weight_int(6) = 12
  };
  std::vector<std::vector<bool>> is_signed_accum[N_config] {
    {{ true, true, true, true }}
  };
  std::vector<std::vector<int>> rounding_accum[N_config] {
    {{ 1, 1, 1, 1 }}   // 1 = RND, matching firmware
  };
  std::vector<std::vector<int>> saturation_accum[N_config] {
    {{ 2, 2, 2, 2 }}   // 2 = SAT_SYM, matching firmware
  };
////////////////////////////////////////////////////////////////////
  std::vector<std::vector<int>> total_bit_weight[N_config] = {
    {{ 12, 12, 12, 12 }}   // total bits: all layers use b0=12 from firmware (kbi quantization)
  };

  std::vector<std::vector<int>> int_bit_weight[N_config] = {
    {{ 6, 6, 6, 6 }}   // integer bits: all layers use i0=6 from firmware (kbi quantization)
  };

  std::vector<std::vector<bool>> is_signed_weight[N_config] = {
    {{ true, true, true, true }}  // all weights are signed
  };

  std::vector<std::vector<int>> rounding_weight[N_config] = {
    {{ 1, 1, 1, 1 }}      // 1 = RND (round to nearest), matching firmware
  };

  std::vector<std::vector<int>> saturation_weight[N_config] = {
    {{ 2, 2, 2, 2 }}      // 2 = SAT_SYM (symmetric saturation), matching firmware
  };

///////////////////////////////////////////////////////////////////////////////////////
  std::vector<std::vector<int>> total_bit_relu[N_config] = {
    {{ 16, 16, 16 }}   // total bits: i0(6) + f0(10) = 16 from firmware (kif quantization)
  };

  std::vector<std::vector<int>> int_bit_relu[N_config] = {
    {{ 6, 6, 6 }}   // integer bits: i0=6 from firmware (kif quantization)
  };

  std::vector<std::vector<bool>> is_signed_relu[N_config] = {
    {{ false, false, false }}  // ReLU outputs are unsigned (always positive)
  };

  std::vector<std::vector<int>> rounding_relu[N_config] = {
    {{ 1, 1, 1 }}       // 1 = RND (round to nearest), matching firmware
  };

  std::vector<std::vector<int>> saturation_relu[N_config] = {
    {{ 0, 0, 0 }}       // 0 = WRAP (no saturation for ReLU), matching firmware
  };

//////////////////////////////////////////////////////////////////////////////////////
  std::vector<std::vector<int>> total_bit[N_config] {
    {{ 16, 16, 16, 16 }}   // total bits: i0(6) + f0(10) = 16 from firmware (kif quantization)
  };
  std::vector<std::vector<int>> int_bit[N_config] {
    {{ 6, 6, 6, 6 }}   // integer bits: i0=6 from firmware (kif quantization)
  };
  std::vector<std::vector<bool>> is_signed[N_config] {
    {{true, true, true, true}}  // layer outputs are signed
  };
  std::vector<std::vector<int>> rounding[N_config] {
    {{1, 1, 1, 1}}   // 1 = RND, matching firmware
  };
  std::vector<std::vector<int>> saturation[N_config] {
    {{0, 0, 0, 0}}   // 0 = WRAP, matching firmware for datalane
  };


  std::vector<std::vector<std::vector<int>>> W_input[N_config] {
    {
      {
        { 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12 },  // layer 0 (input): 24 inputs, ap_ufixed<12,12> matching firmware
        {
          13, 10, 16, 13, 13, 13, 13, 13, 13, 13, 15, 14, 14, 15, 12, 16, 16, 15, 14, 16, 14, 13, 14, 14,
          15, 13, 11, 13, 12, 13, 15, 14, 15, 14, 14, 14, 10, 15, 15, 15, 14, 12, 14, 15, 11, 12, 15, 13,
          15, 14, 15, 13, 11, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
        },  // layer 1 (hidden 0): 64 neurons, unique W per neuron from firmware
        {
          16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
          16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
          16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
        },  // layer 2 (hidden 1): 64 neurons, matching firmware
        {
          16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
          16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
          16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
        }   // layer 3 (hidden 2): 64 neurons, matching firmware
      }
    }
  };
  std::vector<std::vector<std::vector<int>>> I_input[N_config] {
    {
      {
        { 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12 },  // layer 0 (input): 24 inputs, ap_ufixed<12,12> matching firmware
        {
          12, 9, 14, 11, 12, 13, 13, 13, 10, 11, 13, 13, 14, 12, 12, 13, 15, 12, 12, 15, 12, 11, 12, 11,
          13, 13, 11, 11, 12, 13, 12, 14, 11, 13, 13, 13, 10, 14, 14, 14, 12, 12, 13, 14, 9, 11, 12, 10,
          14, 12, 14, 13, 10, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
        },  // layer 1 (hidden 0): 64 neurons, unique I per neuron from firmware
        {
          6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
          6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
          6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6
        },  // layer 2 (hidden 1): 64 neurons, matching firmware
        {
          6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
          6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
          6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6
        }   // layer 3 (hidden 2): 64 neurons, matching firmware
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


