/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <trg/grl/GRLNeuro.h>
#include <trg/grl/dataobjects/GRLMLP.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dbobjects/CDCTriggerNeuroConfig.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <string>
#include <cmath>
#include <TFile.h>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace Belle2;
using namespace CDC;
using namespace std;

// ==========  ap_fixed<Total, Int, AP_TRN, AP_SAT> ==========
const int TRN = 0;
const int RND = 1;
const int NONE = 0;
const int SAT = 1;
const int WRAP = 2;
const int SAT_SYM = 3;


float sim_fixed(float val, int total_bits, int int_bits,
                bool is_signed = true,
                int rounding = 0,
                int saturation = 1)
{
  int frac_bits = total_bits - int_bits;
  float scale = std::pow(2.0f, frac_bits);
  float scaled_val = val * scale;

  int64_t fixed_val;
  if (rounding == 1)
    fixed_val = static_cast<int64_t>(std::round(scaled_val));
  else
    fixed_val = static_cast<int64_t>(std::trunc(scaled_val));

  int64_t max_val, min_val;

  if (is_signed) {
    max_val = (1LL << (total_bits - 1)) - 1;
    min_val = -(1LL << (total_bits - 1));
  } else {
    max_val = (1LL << total_bits) - 1;
    min_val = 0;
  }

  //  wrap
  if (fixed_val > max_val || fixed_val < min_val) {
    switch (saturation) {
      case 1:
        fixed_val = std::min(std::max(fixed_val, min_val), max_val);
        break;
      case 2:
        if (is_signed) {
          int64_t mod = 1LL << total_bits;
          fixed_val = (fixed_val + mod) % mod;
          if (fixed_val >= (1LL << (total_bits - 1)))
            fixed_val -= (1LL << total_bits);
        } else {
          fixed_val = fixed_val % (1LL << total_bits);
        }
        break;
      case 3:
        if (val >= 0)
          fixed_val = std::min(fixed_val, max_val);
        else
          fixed_val = std::max(fixed_val, min_val);
        break;
      case 0:
      default:
        break;
    }
  }

  return static_cast<float>(fixed_val) / scale;
}

// dense_0
inline float sim_fix_dense_0_accum_t(float x)  { return sim_fixed(x, 24, 16); }
inline float sim_fix_dense_0_t(float x)        { return sim_fixed(x, 20, 16); }
inline float sim_fix_dense_0_weight_t(float x) { return sim_fixed(x, 10, 2); }
inline float sim_fix_dense_0_bias_t(float x)   { return sim_fixed(x, 5, 1); }

// dense_0_relu
inline float sim_fix_dense_0_relu_t(float x)        { return sim_fixed(x, 15, 11, false); }
inline float sim_fix_dense_0_relu_table_t(float x)  { return sim_fixed(x, 18, 8); }

// dense_1
inline float sim_fix_dense_1_iq_t(float x)          { return sim_fixed(x, 14, 11, false); }
inline float sim_fix_dense_1_accum_t(float x)       { return sim_fixed(x, 23, 14); }
inline float sim_fix_dense_1_t(float x)             { return sim_fixed(x, 19, 14); }
inline float sim_fix_dense_1_weight_t(float x)      { return sim_fixed(x, 8, 2); }
inline float sim_fix_dense_1_bias_t(float x)        { return sim_fixed(x, 5, 1); }

// dense_1_relu
inline float sim_fix_dense_1_relu_t(float x)        { return sim_fixed(x, 15, 10, false); }
inline float sim_fix_dense_1_relu_table_t(float x)  { return sim_fixed(x, 18, 8); }

// dense_2
inline float sim_fix_dense_2_iq_t(float x)          { return sim_fixed(x, 14, 10, false); }
inline float sim_fix_dense_2_accum_t(float x)       { return sim_fixed(x, 19, 10); }
inline float sim_fix_dense_2_weight_t(float x)      { return sim_fixed(x, 8, 1); }
inline float sim_fix_dense_2_bias_t(float x)        { return sim_fixed(x, 1, 0, false); }

// dense_2  result_t，RND + 2 + UNSIGNED = false
inline float sim_fix_result_t(float x)
{
  return sim_fixed(x, 15, 8, true, 1, 2);
}


float sim_float_to_ufixed(float val, int W, int I)
{
  int F = W - I;
  float scale = std::pow(2.0f, F);
  float rounded = std::round(val * scale) / scale;

  //  [0, 2^I - 1 - 1/scale]
  float max_val = std::pow(2.0f, I) - 1.0f / scale;
  float min_val = 0.0f;

  if (rounded > max_val) return max_val;
  if (rounded < min_val) return min_val;

  return rounded;
}

float sim_dense_1_input_quant(unsigned index, float val)
{
  switch (index) {
    case 0:  return sim_float_to_ufixed(val, 7, 8);
    case 1:  return sim_float_to_ufixed(val, 12, 9);
    case 2:  return sim_float_to_ufixed(val, 14, 11);
    case 3:  return sim_float_to_ufixed(val, 11, 9);
    case 4:  return sim_float_to_ufixed(val, 7, 7);
    case 5:  return sim_float_to_ufixed(val, 4, 9);
    case 6:  return sim_float_to_ufixed(val, 13, 10);
    case 7:  return sim_float_to_ufixed(val, 10, 8);
    case 8:  return sim_float_to_ufixed(val, 9, 8);
    case 9:  return sim_float_to_ufixed(val, 8, 8);
    case 10: return sim_float_to_ufixed(val, 8, 7);
    case 11: return sim_float_to_ufixed(val, 12, 10);
    case 12: return sim_float_to_ufixed(val, 7, 7);
    case 13: return sim_float_to_ufixed(val, 6, 9);
    case 14: return sim_float_to_ufixed(val, 11, 10);
    case 15: return sim_float_to_ufixed(val, 9, 7);
    case 16: return sim_float_to_ufixed(val, 14, 11);
    case 17: return sim_float_to_ufixed(val, 13, 10);
    case 18: return sim_float_to_ufixed(val, 8, 11);
    case 19: return sim_float_to_ufixed(val, 10, 8);
    case 20: return sim_float_to_ufixed(val, 7, 7);
    case 21: return sim_float_to_ufixed(val, 12, 10);
    case 22: return sim_float_to_ufixed(val, 7, 8);
    case 23: return sim_float_to_ufixed(val, 4, 7);
    case 24: return sim_float_to_ufixed(val, 13, 10);
    case 25: return sim_float_to_ufixed(val, 6, 5);
    case 26: return sim_float_to_ufixed(val, 12, 10);
    case 27: return sim_float_to_ufixed(val, 7, 9);
    case 28: return sim_float_to_ufixed(val, 11, 10);
    case 29: return sim_float_to_ufixed(val, 12, 10);
    case 30: return sim_float_to_ufixed(val, 13, 10);
    case 31: return sim_float_to_ufixed(val, 11, 10);
    case 32: return sim_float_to_ufixed(val, 12, 10);
    case 33: return sim_float_to_ufixed(val, 12, 9);
    case 34: return sim_float_to_ufixed(val, 14, 11);
    case 35: return sim_float_to_ufixed(val, 12, 10);
    case 36: return sim_float_to_ufixed(val, 12, 10);
    case 37: return sim_float_to_ufixed(val, 10, 10);
    case 38: return sim_float_to_ufixed(val, 11, 10);
    case 39: return sim_float_to_ufixed(val, 13, 10);
    case 40: return sim_float_to_ufixed(val, 10, 10);
    case 41: return sim_float_to_ufixed(val, 7, 9);
    case 42: return sim_float_to_ufixed(val, 11, 10);
    case 43: return sim_float_to_ufixed(val, 7, 8);
    case 44: return sim_float_to_ufixed(val, 10, 8);
    case 45: return 0.0f;
    case 46: return sim_float_to_ufixed(val, 7, 8);
    case 47: return sim_float_to_ufixed(val, 5, 7);
    case 48: return 0.0f;
    case 49: return sim_float_to_ufixed(val, 11, 11);
    case 50: return sim_float_to_ufixed(val, 13, 10);
    case 51: return sim_float_to_ufixed(val, 7, 8);
    case 52: return sim_float_to_ufixed(val, 11, 11);
    case 53: return sim_float_to_ufixed(val, 14, 11);
    case 54: return sim_float_to_ufixed(val, 7, 7);
    case 55: return sim_float_to_ufixed(val, 3, 7);
    case 56: return sim_float_to_ufixed(val, 8, 9);
    case 57: return sim_float_to_ufixed(val, 8, 8);
    case 58: return sim_float_to_ufixed(val, 13, 10);
    case 59: return sim_float_to_ufixed(val, 11, 9);
    case 60: return sim_float_to_ufixed(val, 11, 9);
    case 61: return sim_float_to_ufixed(val, 11, 8);
    case 62: return sim_float_to_ufixed(val, 10, 9);
    case 63: return sim_float_to_ufixed(val, 7, 9);
    default: return val;
  }
}

float sim_dense_2_input_quant(unsigned index, float val)
{
  switch (index) {
    case 0:  return sim_float_to_ufixed(val, 3, -1);
    case 1:  return sim_float_to_ufixed(val, 13, 9);
    case 2:  return 0.0f;
    case 3:  return sim_float_to_ufixed(val, 10, 6);
    case 4:  return sim_float_to_ufixed(val, 9, 5);
    case 5:  return sim_float_to_ufixed(val, 10, 6);
    case 6:  return sim_float_to_ufixed(val, 8, 6);
    case 7:  return sim_float_to_ufixed(val, 14, 10);
    case 8:  return 0.0f;
    case 9:  return 0.0f;
    case 10: return 0.0f;
    case 11: return sim_float_to_ufixed(val, 10, 6);
    case 12: return sim_float_to_ufixed(val, 9, 5);
    case 13: return sim_float_to_ufixed(val, 1, 5);
    case 14: return sim_float_to_ufixed(val, 8, 5);
    case 15: return 0.0f;
    case 16: return sim_float_to_ufixed(val, 9, 5);
    case 17: return sim_float_to_ufixed(val, 13, 9);
    case 18: return sim_float_to_ufixed(val, 10, 6);
    case 19: return sim_float_to_ufixed(val, 10, 7);
    case 20: return sim_float_to_ufixed(val, 7, 3);
    case 21: return 0.0f;
    case 22: return sim_float_to_ufixed(val, 11, 7);
    case 23: return sim_float_to_ufixed(val, 10, 6);
    case 24: return sim_float_to_ufixed(val, 11, 7);
    case 25: return 0.0f;
    case 26: return sim_float_to_ufixed(val, 10, 6);
    case 27: return 0.0f;
    case 28: return 0.0f;
    case 29: return sim_float_to_ufixed(val, 9, 5);
    case 30: return sim_float_to_ufixed(val, 3, 4);
    case 31: return sim_float_to_ufixed(val, 10, 7);
    case 32: return 0.0f;
    case 33: return sim_float_to_ufixed(val, 10, 7);
    case 34: return 0.0f;
    case 35: return sim_float_to_ufixed(val, 8, 7);
    case 36: return sim_float_to_ufixed(val, 7, 3);
    case 37: return sim_float_to_ufixed(val, 9, 5);
    case 38: return sim_float_to_ufixed(val, 11, 7);
    case 39: return sim_float_to_ufixed(val, 4, 5);
    case 40: return sim_float_to_ufixed(val, 10, 6);
    case 41: return sim_float_to_ufixed(val, 10, 6);
    case 42: return sim_float_to_ufixed(val, 12, 9);
    case 43: return sim_float_to_ufixed(val, 10, 6);
    case 44: return sim_float_to_ufixed(val, 6, 6);
    case 45: return sim_float_to_ufixed(val, 7, 3);
    case 46: return sim_float_to_ufixed(val, 10, 6);
    case 47: return sim_float_to_ufixed(val, 9, 5);
    case 48: return sim_float_to_ufixed(val, 7, 4);
    case 49: return sim_float_to_ufixed(val, 10, 6);
    case 50: return sim_float_to_ufixed(val, 13, 9);
    case 51: return 0.0f;
    case 52: return sim_float_to_ufixed(val, 9, 5);
    case 53: return sim_float_to_ufixed(val, 10, 8);
    case 54: return 0.0f;
    case 55: return sim_float_to_ufixed(val, 9, 5);
    case 56: return sim_float_to_ufixed(val, 9, 5);
    case 57: return sim_float_to_ufixed(val, 13, 9);
    case 58: return 0.0f;
    case 59: return sim_float_to_ufixed(val, 10, 7);
    case 60: return sim_float_to_ufixed(val, 10, 6);
    case 61: return sim_float_to_ufixed(val, 8, 4);
    case 62: return sim_float_to_ufixed(val, 10, 6);
    case 63: return 0.0f;
    default: return val;
  }
}



void
GRLNeuro::initialize(const Parameters& p)
{
  // check parameters
  bool okay = true;
  // ensure that length of lists matches number of sectors
  if (p.nHidden.size() != 1 && p.nHidden.size() != p.nMLP) {
    B2ERROR("Number of nHidden lists should be 1 or " << p.nMLP);
    okay = false;
  }
  if (p.outputScale.size() != 1 && p.outputScale.size() != p.nMLP) {
    B2ERROR("Number of outputScale lists should be 1 or " << p.nMLP);
    okay = false;
  }
  // ensure that number of target nodes is valid
  unsigned short nTarget = int(p.targetresult);
  if (nTarget < 1) {
    B2ERROR("No outputs! Turn on targetresult.");
    okay = false;
  }
  for (unsigned iScale = 0; iScale < p.outputScale.size(); ++iScale) {
    if (p.outputScale[iScale].size() != 2 * nTarget) {
      B2ERROR("outputScale should be exactly " << 2 * nTarget << " values");
      okay = false;
    }
  }

  if (!okay) return;
  // initialize MLPs
  m_MLPs.clear();
  for (unsigned iMLP = 0; iMLP < p.nMLP; ++iMLP) {
    //get indices for sector parameters
    unsigned short nInput = p.i_cdc_sector[iMLP] + p.i_ecl_sector[iMLP];
    vector<float> nhidden =  p.nHidden[iMLP];
    vector<unsigned short> nNodes = {nInput};
    for (unsigned iHid = 0; iHid < nhidden.size(); ++iHid) {
      if (p.multiplyHidden) {
        nNodes.push_back(nhidden[iHid] * nNodes[0]);
      } else {
        nNodes.push_back(nhidden[iHid]);
      }
    }
    nNodes.push_back(nTarget);
    unsigned short targetVars = int(p.targetresult);
    vector<float> outputScale = (p.outputScale.size() == 1) ? p.outputScale[0] : p.outputScale[iMLP];
    m_MLPs.push_back(GRLMLP(nNodes, targetVars, outputScale));
  }
}


float sim_ap_fixed(float val, int total_bits = 12, int int_bits = 12,
                   bool round = true, bool wrap = true)
{
  int frac_bits = total_bits - int_bits;  //
  float scale = std::pow(2, frac_bits);   // scale = 1

  // Apply rounding if needed
  float scaled_val = val * scale;
  int fixed_val = round ? std::round(scaled_val) : std::floor(scaled_val);

  int max_int = std::pow(2, total_bits) - 1;  // For 12-bit unsigned, max = 4095
  int raw_val;

  if (wrap) {
    //
    raw_val = fixed_val & max_int;  // == fixed_val % (1 << total_bits)
  } else {
    //
    raw_val = std::min(std::max(fixed_val, 0), max_int);
  }

  return raw_val / scale;
}

//
float sim_fix_input_layer_t(float val)
{
  return sim_ap_fixed(val, 12, 12, true, true);  // AP_RND + AP_WRAP
}

std::vector<float> sim_dense_0_iq(const std::vector<float>& input)
{
  const std::vector<std::pair<int, int>> dense_0_iq_config = {
    {12, 5}, {12, 4}, {10, 6}, {8, 4}, {8, 4}, {9, 5},
    {6, 2}, {8, 3}, {6, 3}, {5, 4}, {7, 4}, {9, 5},
    {8, 2}, {8, 2}, {6, 3}, {5, 3}, {8, 4}, {6, 2}
  };

  std::vector<float> output;
  output.reserve(input.size());
  for (size_t i = 0; i < input.size(); ++i) {
    int total_bits = dense_0_iq_config[i].first;
    int int_bits = dense_0_iq_config[i].second;
    output.push_back(sim_ap_fixed(input[i], total_bits, int_bits, true, true));

  }
  return output;
}
///////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////////////////////
float
GRLNeuro::runMLP(unsigned isector, const std::vector<float>& input)
{


  const GRLMLP& expert = m_MLPs[isector];
  vector<float> weights = expert.getWeights();
  vector<float> bias = expert.getBias();
  vector<float> layerinput = input;

  vector<float> layeroutput2 = {};
  vector<float> layeroutput3 = {};
  vector<float> layeroutput4 = {};

  /////////////////////////////////////////////////////////////
  for (size_t i = 0; i < layerinput.size(); ++i) {
    layerinput[i] = sim_fix_input_layer_t(layerinput[i]);
  }
  layeroutput2.clear();
  layeroutput2.assign(expert.getNumberOfNodesLayer(2), 0.);

  unsigned num_inputs = layerinput.size();
  unsigned num_neurons = expert.getNumberOfNodesLayer(2);  // 64
  for (unsigned io = 0; io < num_neurons; ++io) {
    float bias_raw = bias[io];
    float bias_fixed = sim_fix_dense_0_bias_t(bias_raw);
    float bias_contrib = sim_fix_dense_0_accum_t(bias_fixed);
    layeroutput2[io] = bias_contrib;
  }

  unsigned iw = 0;
//  input*weight
  for (unsigned ii = 0; ii < num_inputs; ++ii) {
    float input_val = layerinput[ii];
    for (unsigned io = 0; io < num_neurons; ++io) {
      float weight_raw = weights[iw];
      float weight_fixed = sim_fix_dense_0_weight_t(weight_raw);
      float product = input_val * weight_fixed;
      float contrib = sim_fix_dense_0_accum_t(product);

      layeroutput2[io] += contrib;

      ++iw;
    }
  }


//apply activation function, ReLU for hidden layer and output layer
// === dense_0_t + ReLU  ===
  std::vector<float> layeroutput2_fixed_relu(num_neurons);

  for (unsigned io = 0; io < num_neurons; ++io) {
    // dense_0_t）
    float fixed_val = sim_fix_dense_0_t(layeroutput2[io]);

    // ReLU
    float relu_val = (fixed_val > 0) ? fixed_val : 0;

    layeroutput2_fixed_relu[io] = relu_val;

  }

  std::vector<float> dense1_input(64);
  for (unsigned i = 0; i < 64; ++i) {
    dense1_input[i] = sim_dense_1_input_quant(i, layeroutput2_fixed_relu[i]);
  }

  layeroutput3.clear();
  layeroutput3.assign(expert.getNumberOfNodesLayer(1), 0.);
  unsigned num_inputs_1 = layeroutput2_fixed_relu.size();
  unsigned num_neurons_1 = expert.getNumberOfNodesLayer(2);
  for (unsigned io = 64; io < num_neurons_1 + 64; ++io) {
    float bias_raw = bias[io];
    float bias_fixed = sim_fix_dense_1_bias_t(bias_raw);
    float bias_contrib = sim_fix_dense_1_accum_t(bias_fixed);
    layeroutput3[io - 64] = bias_contrib;

  }


  for (unsigned ii = 0; ii < num_inputs_1; ++ii) {
    float input_val = dense1_input[ii];
    for (unsigned io = 0; io < num_neurons_1; ++io) {

      float weight_raw = weights[iw];

      float weight_fixed = sim_fix_dense_1_weight_t(weight_raw);
      float product = input_val * weight_fixed;
      float contrib = sim_fix_dense_1_accum_t(product);

      layeroutput3[io] += contrib;
      ++iw;
    }
  }


  std::vector<float> layeroutput3_fixed_relu(num_neurons);


  for (unsigned io = 0; io < num_neurons_1; ++io) {
    float fixed_val = sim_fix_dense_1_t(layeroutput3[io]);
    // ReLU
    float relu_val = (fixed_val > 0) ? fixed_val : 0;

    layeroutput3_fixed_relu[io] = relu_val;

  }
  std::vector<float> dense2_input(64);
  for (unsigned i = 0; i < 64; ++i) {
    dense2_input[i] = sim_dense_2_input_quant(i, layeroutput3_fixed_relu[i]);
  }
  layeroutput4.clear();
  layeroutput4.assign(expert.getNumberOfNodesLayer(3), 0.);

  unsigned num_inputs_2 = layeroutput2_fixed_relu.size();
  unsigned num_neurons_2 = expert.getNumberOfNodesLayer(3);
  for (unsigned io = 128; io < num_neurons_2 + 128; ++io) {
    float bias_raw = bias[io];
    float bias_fixed = sim_fix_dense_2_bias_t(bias_raw);
    float bias_contrib = sim_fix_dense_2_accum_t(bias_fixed);
    layeroutput4[io - 128] = bias_contrib;

  }

  for (unsigned ii = 0; ii < num_inputs_2; ++ii) {
    float input_val = dense2_input[ii];
    for (unsigned io = 0; io < num_neurons_2; ++io) {
      float weight_raw = weights[iw];
      float weight_fixed = sim_fix_dense_2_weight_t(weight_raw);
      float product = input_val * weight_fixed;
      float contrib = sim_fix_dense_2_accum_t(product);

      layeroutput4[io] += contrib;

      ++iw;
    }
  }
  return layeroutput4[0];

}


bool GRLNeuro::load(unsigned isector, const string& weightfilename, const string& biasfilename)
{
  if (weightfilename.size() < 1) {
    B2ERROR("Could not load Neurotrigger weights from database!");
    return false;
  } else if (biasfilename.size() < 1) {
    B2ERROR("Could not load Neurotrigger bias from database!");
    return false;
  } else {
    std::ifstream wfile(weightfilename);
    if (!wfile.is_open()) {
      B2WARNING("Could not open file " << weightfilename);
      return false;
    } else {
      std::ifstream bfile(biasfilename);
      if (!bfile.is_open()) {
        B2WARNING("Could not open file " << biasfilename);
        return false;
      } else {
        GRLMLP& expert = m_MLPs[isector];
        std::vector<float> warray;
        std::vector<float> barray;
        warray.clear();
        barray.clear();

        float element;
        while (wfile >> element) {
          warray.push_back(element);
        }
        while (bfile >> element) {
          barray.push_back(element);
        }

        if (warray.size() != expert.nWeightsCal()) {
          B2ERROR("Number of weights is not equal to registered architecture!");
          return false;
        } else expert.setWeights(warray);
        if (barray.size() != expert.nBiasCal()) {
          B2ERROR("Number of bias is not equal to registered architecture!");
          return false;
        }

        expert.setWeights(warray);
        expert.setBias(barray);
        return true;
      }
    }
  }
}

bool GRLNeuro::load(unsigned isector, std::vector<float> warray, std::vector<float> barray)
{
  GRLMLP& expert = m_MLPs[isector];
  expert.setWeights(warray);
  expert.setBias(barray);
  return true;
}
