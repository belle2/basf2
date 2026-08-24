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
#include <iostream>
#include <sstream>

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

  // 10 thresholds for 10 MLP outputs: any output[i] > threshold[i] triggers TauNN
  std::vector<float> ecltaunn_threshold[N_config] = {
    {
      -0.6015625, -1.001953125, -0.568359375, -0.939453125, -1.111328125,
        -0.583984375, -0.708984375, -0.37890625, -0.474609375, -0.591796875
      }
  };
  // firmware: 6 dense layers (Dense 0-5)
  unsigned ecltaunn_nMLP[N_config] = {
    1
  };
  bool ecltaunn_multiplyHidden[N_config] = {
    false
  };
  std::vector<std::vector<float>> ecltaunn_nHidden[N_config];
  // firmware: 24 -> 64 -> 64 -> 64 -> 64 -> 64 -> 10 (input + 5 hidden + output)
  std::vector<float> eclaunn_nHidden_0 = {64, 64, 64, 64, 64};
  ecltaunn_nHidden[0].push_back(eclaunn_nHidden_0);
  // Number of output nodes
  std::vector<unsigned> ecltaunn_nOutput[N_config] = {
    {10}
  };
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

  // firmware: 6 dense layer weight files, merged in order
  // w3.txt(24*64=1536) + w6.txt(64*64=4096) + w9.txt(64*64=4096) + w12.txt(64*64=4096) + w15.txt(64*64=4096) + w18.txt(64*10=640) = 18560
  const char* weight_files[] = {
    "./weights/w3.txt",
    "./weights/w6.txt",
    "./weights/w9.txt",
    "./weights/w12.txt",
    "./weights/w15.txt",
    "./weights/w18.txt"
  };
  const char* bias_files[] = {
    "./weights/b3.txt",
    "./weights/b6.txt",
    "./weights/b9.txt",
    "./weights/b12.txt",
    "./weights/b15.txt",
    "./weights/b18.txt"
  };

  // Load and merge all weight files
  std::vector<std::vector<float>> ecltaunn_weight[N_config];
  for (int i = 0; i < N_config; i++) {
    std::vector<float> all_weights;
    for (unsigned j = 0; j < 6; j++) {  // 6 weight files
      std::ifstream wfile(weight_files[j]);
      if (!wfile.is_open()) {
        std::cout << "trg-grl-writedb: cannot open weight file: " << weight_files[j] << std::endl;
        return;
      }
      std::cout << "trg-grl-writedb: loading weight file: " << weight_files[j] << std::endl;

      // Read entire file content, replace commas with spaces
      std::stringstream buffer;
      buffer << wfile.rdbuf();
      std::string content = buffer.str();

      // Replace commas with spaces
      for (char& c : content) {
        if (c == ',') c = ' ';
      }

      // Read all floats from processed string
      std::stringstream ss(content);
      float element;
      int count = 0;
      while (ss >> element) {
        all_weights.push_back(element);
        count++;
        if (count <= 3) {
          std::cout << "    element[" << (count - 1) << "] = " << element << std::endl;
        }
      }
      std::cout << "  file loaded " << count << " elements" << std::endl;
      std::cout << "  loaded " << all_weights.size() << " total elements so far" << std::endl;
    }
    ecltaunn_weight[i] = std::vector<std::vector<float>> { all_weights }; // One MLP expert contains all weights
    std::cout << "trg-grl-writedb: loaded weights total: " << all_weights.size() << " elements" << std::endl;
  }

  // Load and merge all bias files
  std::vector<std::vector<float>> ecltaunn_bias[N_config];
  for (int i = 0; i < N_config; i++) {
    std::vector<float> all_biases;
    for (unsigned j = 0; j < 6; j++) {  // 6 bias files
      std::ifstream bfile(bias_files[j]);
      if (!bfile.is_open()) {
        std::cout << "trg-grl-writedb: cannot open bias file: " << bias_files[j] << std::endl;
        return;
      }
      std::cout << "trg-grl-writedb: loading bias file: " << bias_files[j] << std::endl;

      // Read entire file content, replace commas with spaces
      std::stringstream buffer;
      buffer << bfile.rdbuf();
      std::string content = buffer.str();

      // Replace commas with spaces
      for (char& c : content) {
        if (c == ',') c = ' ';
      }

      // Read all floats from processed string
      std::stringstream ss(content);
      float element;
      int count = 0;
      while (ss >> element) {
        all_biases.push_back(element);
        count++;
      }
      std::cout << "  file loaded " << count << " elements" << std::endl;
    }
    ecltaunn_bias[i] = std::vector<std::vector<float>> { all_biases }; // One MLP expert contains all biases
    std::cout << "trg-grl-writedb: loaded biases: " << all_biases.size() << " elements" << std::endl;
  }

  // firmware: 6 layers (Dense_0, Dense_1, Dense_2, Dense_3, Dense_4, Dense_5)
  // bias params: Layer 0-3 use ap_ufixed<2,32>, Layer 4 uses ap_fixed<4,2>, Layer 5 uses ap_fixed<6,1>
  std::vector<std::vector<int>> total_bit_bias[N_config] = {
    {{ 2, 2, 2, 2, 4, 6 }}   // total bits
  };
  std::vector<std::vector<int>> int_bit_bias[N_config] {
    {{ 32, 32, 32, 32, 2, 1 }}   // integer bits
  };
  std::vector<std::vector<bool>> is_signed_bias[N_config] {
    {{ false, false, false, false, true, true }}
  };
  std::vector<std::vector<int>> rounding_bias[N_config] {
    {{ 0, 0, 0, 0, 0, 0 }}   // 0 = truncate
  };
  std::vector<std::vector<int>> saturation_bias[N_config] {
    {{ 2, 2, 2, 2, 2, 2 }}   // 2 = saturate
  };

////////////////////////////////////////////////////////////////////////////
  // firmware accum params: ap_fixed<18,16>, ap_fixed<17,16>, ap_fixed<15,15>, ap_fixed<13,9>, ap_fixed<17,8>, ap_fixed<19,10>
  std::vector<std::vector<int>> total_bit_accum[N_config] {
    {{ 18, 17, 15, 13, 17, 19 }}
  };
  std::vector<std::vector<int>> int_bit_accum[N_config] {
    {{ 16, 16, 15, 9, 8, 10 }}
  };
  std::vector<std::vector<bool>> is_signed_accum[N_config] {
    {{ true, true, true, true, true, true }}
  };
  std::vector<std::vector<int>> rounding_accum[N_config] {
    {{ 1, 1, 1, 1, 1, 1 }}   // 1 = round (AP_RND)
  };
  std::vector<std::vector<int>> saturation_accum[N_config] {
    {{ 2, 2, 2, 2, 2, 2 }}   // 2 = wrap
  };
////////////////////////////////////////////////////////////////////
  // firmware weight params: ap_fixed<9,4>, ap_fixed<7,3>, ap_fixed<6,2>, ap_ufixed<6,0>, ap_fixed<10,1>, ap_fixed<7,1>
  std::vector<std::vector<int>> total_bit_weight[N_config] = {
    {{ 9, 7, 6, 6, 10, 7 }}
  };
  std::vector<std::vector<int>> int_bit_weight[N_config] = {
    {{ 4, 3, 2, 0, 1, 1 }}
  };
  std::vector<std::vector<bool>> is_signed_weight[N_config] = {
    {{ true, true, true, false, true, true }}  // Layer 3 is unsigned (ap_ufixed)
  };
  std::vector<std::vector<int>> rounding_weight[N_config] = {
    {{ 0, 0, 0, 0, 0, 0 }}      // 0 = truncate
  };
  std::vector<std::vector<int>> saturation_weight[N_config] = {
    {{ 2, 2, 2, 2, 2, 2 }}      // 2 = wrap
  };

///////////////////////////////////////////////////////////////////////////////////////
  // firmware relu params: ap_fixed<14,14>, ap_fixed<13,13>, ap_fixed<12,12>, ap_ufixed<9,8>, ap_ufixed<13,7>
  std::vector<std::vector<int>> total_bit_relu[N_config] = {
    {{ 14, 13, 12, 9, 13 }}  // 5 relu layers (Layer 0-4, last layer has no relu)
  };
  std::vector<std::vector<int>> int_bit_relu[N_config] = {
    {{ 14, 13, 12, 8, 7 }}
  };
  std::vector<std::vector<bool>> is_signed_relu[N_config] = {
    {{ true, true, true, false, false }}  // Layer 3, 4 are unsigned
  };
  std::vector<std::vector<int>> rounding_relu[N_config] = {
    {{ 0, 0, 0, 0, 0 }}       // 0 = truncate
  };
  std::vector<std::vector<int>> saturation_relu[N_config] = {
    {{ 2, 2, 2, 2, 2 }}       // 2 = saturate
  };

//////////////////////////////////////////////////////////////////////////////////////
  // firmware general quantization params: ap_fixed<16,16>, ap_fixed<16,16>, ap_fixed<15,15>, ap_fixed<10,9>
  std::vector<std::vector<int>> total_bit[N_config] {
    {{ 16, 16, 15, 10, 14 }}
  };
  std::vector<std::vector<int>> int_bit[N_config] {
    {{ 16, 16, 15, 9, 8 }}
  };
  std::vector<std::vector<bool>> is_signed[N_config] {
    {{ true, true, true, true, true }}
  };
  std::vector<std::vector<int>> rounding[N_config] {
    {{ 1, 1, 1, 1, 1 }}   // 1 = round
  };
  std::vector<std::vector<int>> saturation[N_config] {
    {{ 2, 2, 2, 2, 2 }}
  };


  // firmware: 5 hidden layers (Dense_0-4), 64 neurons each
  // dense_0_iq: ap_fixed<12,13>, dense_1_iq: ap_fixed<14,14>, dense_2_iq: ap_fixed<13,13>
  // dense_3_iq: ap_fixed<12,12>, dense_4_iq: ap_ufixed<9,9>
  // TRGGRLConfig expects 3D: [MLP][layer][neuron] but since nMLP=1, it's [1][5][64]
  // To simplify, use 2D and wrap in 3D when setting to db
  std::vector<std::vector<int>> W_input_2d[N_config];
  std::vector<std::vector<int>> I_input_2d[N_config];
  for (int i = 0; i < N_config; i++) {
    // Layer 0 (dense_1_iq): 64 inputs, sparse quantization
    // Non-zero entries from firmware: out[i] = ap_fixed<W,I>(inp[i])
    // out[i] = 0 means input is zeroed
    W_input_2d[i].push_back(std::vector<int> {
      0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 8, 0, 9, 0, 5, 6, 6,
      0, 0, 0, 7, 0, 8, 7, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 7, 5, 6,
      8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 6, 6, 5, 0, 6, 8,
      0, 9, 9, 0
    });
    I_input_2d[i].push_back(std::vector<int> {
      0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 13, 0, 13, 0, 10, 11, 11,
      0, 0, 0, 12, 0, 12, 12, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 13, 10, 10,
      12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 10, 11, 11, 0, 11, 12,
      0, 12, 14, 0
    });
    // Layer 1 (dense_2_iq): 64 inputs, sparse quantization
    W_input_2d[i].push_back(std::vector<int> {
      0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 8, 0, 7, 0, 0, 0, 0, 0, 0,
      7, 0, 0, 6, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 8, 0, 10, 0,
      0, 6, 0, 0, 0, 0, 7, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 8
    });
    I_input_2d[i].push_back(std::vector<int> {
      0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 12, 0, 12, 0, 0, 0, 0, 0, 0,
      12, 0, 0, 11, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 13, 0, 13, 0,
      0, 11, 0, 0, 0, 0, 12, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 13
    });
    // Layer 2 (dense_3_iq): 64 inputs, sparse quantization
    W_input_2d[i].push_back(std::vector<int> {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0,
      0, 0, 0, 0
    });
    I_input_2d[i].push_back(std::vector<int> {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0,
      0, 0, 0, 0
    });
    // Layer 3 (dense_4_iq): 64 inputs, sparse quantization (ap_ufixed)
    W_input_2d[i].push_back(std::vector<int> {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      9, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0
    });
    I_input_2d[i].push_back(std::vector<int> {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      9, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0
    });
    // Layer 4 (dense_5_iq): 64 inputs, sparse quantization (ap_ufixed)
    W_input_2d[i].push_back(std::vector<int> {
      0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0
    });
    I_input_2d[i].push_back(std::vector<int> {
      0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0
    });
  }
  // Wrap 2D in 3D for TRGGRLConfig (which expects 3D)
  std::vector<std::vector<std::vector<int>>> W_input[N_config];
  std::vector<std::vector<std::vector<int>>> I_input[N_config];
  for (int i = 0; i < N_config; i++) {
    W_input[i].push_back(W_input_2d[i]);  // MLP 0
    I_input[i].push_back(I_input_2d[i]);  // MLP 0
  }
  std::cout << "  W_input[0].size=" << W_input[0].size() << " (should be 1)" << std::endl;
  if (W_input[0].size() > 0) {
    std::cout << "  W_input[0][0].size=" << W_input[0][0].size() << " (should be 5 layers)" << std::endl;
    if (W_input[0][0].size() > 0) {
      std::cout << "  W_input[0][0][0].size=" << W_input[0][0][0].size() << " (should be 64 neurons)" << std::endl;
    }
  }

  // Input layer quantization parameters for the first dense layer (24 inputs)
  // firmware dense_0_iq uses per-input ap_fixed types (W, I):
  // out[0]=<10,13>, [1]=<11,13>, [2]=<10,13>, [3]=<9,12>, [4]=<8,11>, [5]=<6,10>,
  // [6]=<8,9>, [7]=<8,9>, [8]=<5,9>, [9]=<5,9>, [10]=<5,9>, [11]=<4,9>,
  // [12]=<5,10>, [13]=<5,10>, [14]=<4,10>, [15]=<4,10>, [16]=<3,10>, [17]=<3,10>,
  // [18]=<5,10>, [19]=<5,10>, [20]=<5,10>, [21]=<3,10>, [22]=<3,10>, [23]=<3,10>
  // W_input_layer: 1 layer, 24 total_bit values
  std::vector<std::vector<std::vector<int>>> W_input_layer[N_config] {
    {
      {
        { 10, 11, 10, 9, 8, 6, 8, 8, 5, 5, 5, 4, 5, 5, 4, 4, 3, 3, 5, 5, 5, 3, 3, 3 }
      }
    }
  };
  // I_input_layer: 1 layer, 24 int_bit values
  std::vector<std::vector<std::vector<int>>> I_input_layer[N_config] {
    {
      {
        { 13, 13, 13, 12, 11, 10, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 }
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
      db_grlconfig->set_ecltaunn_W_input_layer(W_input_layer[i][0]);
      db_grlconfig->set_ecltaunn_I_input_layer(I_input_layer[i][0]);

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
      db_grlconfig->set_ecltaunn_W_input_layer(W_input_layer[i][0]);
      db_grlconfig->set_ecltaunn_I_input_layer(I_input_layer[i][0]);

      db_grlconfig.import(iov);
    }
  }
}

int main()
{

  setgrlconfig();

}


