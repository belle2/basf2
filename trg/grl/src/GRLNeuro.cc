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

float sim_ap_ufixed(float val, int W, int I)
{
  if (W == 0) {
    return 0.0f;
  }

  int F = W - I;
  float scale = std::pow(2.0f, F);

  int64_t fixed_val = static_cast<int64_t>(std::round(val * scale));

  int64_t max_val = (1LL << W) - 1;

  fixed_val = fixed_val % (max_val + 1);
  if (fixed_val < 0) fixed_val += (max_val + 1);

  return static_cast<float>(fixed_val) / scale;
}



// ========== ap_fixed  ====== trunc + wrap ==========
inline float sim_fixed(float val, int total_bits, int int_bits,
                       bool is_signed = true,
                       int rounding = 0,     // 0: trunc, 1: round
                       int saturation = 2)   // 2: wrap
{
  int frac_bits = total_bits - int_bits;
  float scale = std::pow(2.0f, frac_bits);


  float scaled_val = val * scale;
  int64_t fixed_val;
  if (rounding == 1) {
    fixed_val = static_cast<int64_t>(std::round(scaled_val));
  } else {
    fixed_val = static_cast<int64_t>(std::trunc(scaled_val));
  }


  int64_t max_val, min_val;
  if (is_signed) {
    max_val = (1LL << (total_bits - 1)) - 1;
    min_val = -(1LL << (total_bits - 1));
  } else {
    max_val = (1LL << total_bits) - 1;
    min_val = 0;
  }

  switch (saturation) {
    case 2: // wrap
      if (is_signed) {
        int64_t mod = 1LL << total_bits;
        fixed_val = (fixed_val + mod) % mod;
        if (fixed_val >= (1LL << (total_bits - 1))) {
          fixed_val -= mod;
        }
      } else {
        fixed_val = fixed_val % (1LL << total_bits);
      }
      break;
    case 1: // saturate
      fixed_val = std::min(std::max(fixed_val, min_val), max_val);
      break;
    case 3: // saturate by sign
      fixed_val = (val >= 0) ? std::min(fixed_val, max_val)
                  : std::max(fixed_val, min_val);
      break;
    case 0:
    default: // none
      break;
  }

  return static_cast<float>(fixed_val) / scale;
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
    GRLMLP grlmlp_temp = GRLMLP(nNodes, targetVars, outputScale);
    grlmlp_temp.set_total_bit_bias(p.total_bit_bias[iMLP]);
    grlmlp_temp.set_int_bit_bias(p.int_bit_bias[iMLP]);
    grlmlp_temp.set_is_signed_bias(p.is_signed_bias[iMLP]);
    grlmlp_temp.set_rounding_bias(p.rounding_bias[iMLP]);
    grlmlp_temp.set_saturation_bias(p.saturation_bias[iMLP]);
    grlmlp_temp.set_total_bit_accum(p.total_bit_accum[iMLP]);
    grlmlp_temp.set_int_bit_accum(p.int_bit_accum[iMLP]);
    grlmlp_temp.set_is_signed_accum(p.is_signed_accum[iMLP]);
    grlmlp_temp.set_rounding_accum(p.rounding_accum[iMLP]);
    grlmlp_temp.set_saturation_accum(p.saturation_accum[iMLP]);
    grlmlp_temp.set_total_bit_weight(p.total_bit_weight[iMLP]);
    grlmlp_temp.set_int_bit_weight(p.int_bit_weight[iMLP]);
    grlmlp_temp.set_is_signed_weight(p.is_signed_weight[iMLP]);
    grlmlp_temp.set_rounding_weight(p.rounding_weight[iMLP]);
    grlmlp_temp.set_saturation_weight(p.saturation_weight[iMLP]);
    grlmlp_temp.set_total_bit_relu(p.total_bit_relu[iMLP]);
    grlmlp_temp.set_int_bit_relu(p.int_bit_relu[iMLP]);
    grlmlp_temp.set_is_signed_relu(p.is_signed_relu[iMLP]);
    grlmlp_temp.set_rounding_relu(p.rounding_relu[iMLP]);
    grlmlp_temp.set_saturation_relu(p.saturation_relu[iMLP]);
    grlmlp_temp.set_total_bit(p.total_bit[iMLP]);
    grlmlp_temp.set_int_bit(p.int_bit[iMLP]);
    grlmlp_temp.set_is_signed(p.is_signed[iMLP]);
    grlmlp_temp.set_rounding(p.rounding[iMLP]);
    grlmlp_temp.set_saturation(p.saturation[iMLP]);
    grlmlp_temp.set_W_input(p.W_input[iMLP]);
    grlmlp_temp.set_I_input(p.I_input[iMLP]);
    m_MLPs.push_back(grlmlp_temp);
    //m_MLPs.push_back(GRLMLP(nNodes, targetVars, outputScale));
  }
}

// ==================== input_layer_t ====================
// HLS: ap_ufixed<12,12,AP_RND,AP_WRAP,0>
inline float sim_fix_input_layer_t(float val)
{
  const int W = 12;
  const int I = 12;   // int
  const int F = W - I; // float
  //const bool is_signed = false; // ap_ufixed //not used

  double scaled = val * std::pow(2.0, F); //
  long long q = llrint(scaled); // round-to-even (C99)

  // wrap
  //unsigned long long max_val = (1ULL << W) - 1; // 2^12 - 1 = 4095 //not used
  if (q < 0) {

    q = (q % (1ULL << W) + (1ULL << W)) % (1ULL << W);
  } else {
    q = q % (1ULL << W);
  }

  return (float)q / std::pow(2.0, F); //
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
float
GRLNeuro::runMLP(unsigned isector, const std::vector<float>& input)
{


  const GRLMLP& expert = m_MLPs[isector];
  vector<float> weights = expert.getWeights();
  vector<float> bias = expert.getBias();
  vector<int> total_bit_bias = expert.get_total_bit_bias();
  vector<int> int_bit_bias = expert.get_int_bit_bias();
  vector<bool> is_signed_bias = expert.get_is_signed_bias();
  vector<int> rounding_bias = expert.get_rounding_bias();
  vector<int> saturation_bias = expert.get_saturation_bias();
  vector<int> total_bit_accum = expert.get_total_bit_accum();
  vector<int> int_bit_accum = expert.get_int_bit_accum();
  vector<bool> is_signed_accum = expert.get_is_signed_accum();
  vector<int> rounding_accum = expert.get_rounding_accum();
  vector<int> saturation_accum = expert.get_saturation_accum();
  vector<int> total_bit_weight = expert.get_total_bit_weight();
  vector<int> int_bit_weight = expert.get_int_bit_weight();
  vector<bool> is_signed_weight = expert.get_is_signed_weight();
  vector<int> rounding_weight = expert.get_rounding_weight();
  vector<int> saturation_weight = expert.get_saturation_weight();
  vector<int> total_bit_relu = expert.get_total_bit_relu();
  vector<int> int_bit_relu = expert.get_int_bit_relu();
  vector<bool> is_signed_relu = expert.get_is_signed_relu();
  vector<int> rounding_relu = expert.get_rounding_relu();
  vector<int> saturation_relu = expert.get_saturation_relu();
  vector<int> total_bit = expert.get_total_bit();
  vector<int> int_bit = expert.get_int_bit();
  vector<bool> is_signed = expert.get_is_signed();
  vector<int> rounding = expert.get_rounding();
  vector<int> saturation = expert.get_saturation();
  vector<vector<int>> W_input = expert.get_W_input();
  vector<vector<int>> I_input = expert.get_I_input();

  //input layer
  vector<float> layerinput = input;
  for (size_t i = 0; i < layerinput.size(); ++i) {
    layerinput[i] = sim_fix_input_layer_t(layerinput[i]);
  }

  //hidden layer and output layer
  vector<float> layeroutput = {};
  unsigned num_layers = expert.getNumberOfLayers();
  unsigned num_total_neurons = 0;
  unsigned iw = 0;
  for (unsigned i_layer = 0; i_layer < num_layers - 1; i_layer++) {
    //read bias
    unsigned num_neurons = expert.getNumberOfNodesLayer(i_layer + 1);
    layeroutput.clear();
    layeroutput.assign(num_neurons, 0.);
    layeroutput.shrink_to_fit();
    for (unsigned io = 0; io < num_neurons; ++io) {
      float bias_raw = bias[io + num_total_neurons];
      float bias_fixed   = sim_fixed(bias_raw, total_bit_bias[i_layer], int_bit_bias[i_layer], is_signed_bias[i_layer],
                                     rounding_bias[i_layer], saturation_bias[i_layer]);
      float bias_contrib = sim_fixed(bias_fixed, total_bit_accum[i_layer], int_bit_accum[i_layer], is_signed_accum[i_layer],
                                     rounding_accum[i_layer], saturation_accum[i_layer]);
      layeroutput[io] = bias_contrib;
    }
    num_total_neurons += num_neurons;

    //input*weight
    unsigned num_inputs = layerinput.size();
    for (unsigned ii = 0; ii < num_inputs; ++ii) {
      float input_val = layerinput[ii];
      for (unsigned io = 0; io < num_neurons; ++io) {
        float weight_raw = weights[iw];
        float weight_fixed = sim_fixed(weight_raw, total_bit_weight[i_layer], int_bit_weight[i_layer], is_signed_weight[i_layer],
                                       rounding_weight[i_layer], saturation_weight[i_layer]);
        float product = input_val * weight_fixed;
        float contrib = sim_fixed(product, total_bit_accum[i_layer], int_bit_accum[i_layer], is_signed_accum[i_layer],
                                  rounding_accum[i_layer], saturation_accum[i_layer]);
        layeroutput[io] += contrib;
        ++iw;
      }
    }

    if (i_layer < num_layers - 2) {
      //relu
      for (unsigned io = 0; io < num_neurons; ++io) {
        float fixed_val = sim_fixed(layeroutput[io], total_bit[i_layer], int_bit[i_layer], is_signed[i_layer], rounding[i_layer],
                                    saturation[i_layer]);
        float relu_val = (fixed_val > 0) ? fixed_val : 0;
        layeroutput[io] = sim_fixed(relu_val, total_bit_relu[i_layer], int_bit_relu[i_layer], is_signed_relu[i_layer],
                                    rounding_relu[i_layer], saturation_relu[i_layer]);
      }

      //input to next layer
      layerinput.clear();
      layerinput.assign(num_neurons, 0);
      layerinput.shrink_to_fit();
      for (unsigned i = 0; i < num_neurons; ++i) {
        layerinput[i] = sim_ap_ufixed(layeroutput[i], W_input[i_layer][i], I_input[i_layer][i]);
      }
    } else {
      return layeroutput[0];
    }
  }
  return 0;
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
