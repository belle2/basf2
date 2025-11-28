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
#include <array>
#include <cstdint>
#include <utility>
using namespace Belle2;
using namespace CDC;
using namespace std;



// ========== ap_(u)fixed  ========

/**
 * Simulates Xilinx ap_fixed/ap_ufixed behavior with configurable quantization and overflow modes.
 *
 * Key Features:
 * - Supports all Vivado HLS quantization/overflow modes
 * - Handles cases where total_bits < int_bits (effective bit truncation)
 * - Optimized with bit shifts and lookup tables
 * - Explicit integer/fractional bit separation
 *
 * @param val            Input floating-point value
 * @param total_bits     Total bit width (W)
 * @param int_bits       Integer bits (I)
 * @param is_signed      true=ap_fixed (signed), false=ap_ufixed (unsigned)
 * @param quant_mode     Quantization mode:
 *                       0=AP_TRN (truncate), 1=AP_RND (round),
 *                       2=AP_RND_ZERO, 3=AP_RND_MIN_INF, 4=AP_RND_INF, 5=AP_RND_CONV
 * @param overflow_mode  Overflow mode:
 *                       0=AP_SAT (saturate), 1=AP_SAT_ZERO,
 *                       2=AP_WRAP (wrap), 3=AP_WRAP_SM (sign-magnitude wrap),4=AP_SAT_SYM
 * @param saturation_bits Saturation bit width (0=use total_bits)
 * @return               Simulated fixed-point value
 */
inline float sim_ap_fixed(float val, int total_bits, int int_bits,
                          bool is_signed = true,
                          int quant_mode = 0,
                          int overflow_mode = 0,
                          int saturation_bits = 0)
{
  //for normal ap_types
  if (total_bits >= int_bits) {

    // Effective bits (handle saturation_bits and total_bits < int_bits cases)
    int effective_total = (saturation_bits > 0) ?
                          std::min(saturation_bits, total_bits) : total_bits;
    int effective_I = std::min(int_bits, effective_total);
    int F = effective_total - effective_I;  // Fractional bits ≥ 0
    if (int_bits == 0 && is_signed == true) {
      F = total_bits ;
      effective_I = total_bits + 1;
    }

    // --- 2. Fast Scale Calculation (2^F) ---
    const float scale = [F]() {
      if (F <= 0) {return 1.0f;}  // No fractional bits
      else {
        // Bit-shift is faster than std::pow(2, F)
        const uint32_t scale_int = 1U << F;
        return 1.0f / static_cast<float>(scale_int);
      }
    }();

    // --- 3. Quantization (Mode-Specific Rounding) ---
    const auto quantize = [](float scaled_val, int mode) -> int64_t {
      using QuantizerFuncPtr = int64_t(*)(float);
      static constexpr QuantizerFuncPtr quantizers[] = {
        [](float x) -> int64_t { return static_cast<int64_t>(std::trunc(x)); },
        [](float x) -> int64_t { return static_cast<int64_t>(std::round(x)); },
        [](float x) -> int64_t {
          return (x >= 0) ? static_cast<int64_t>(std::floor(x))
          : static_cast<int64_t>(std::ceil(x));
        },
        [](float x) -> int64_t { return static_cast<int64_t>(std::floor(x)); },
        [](float x) -> int64_t { return static_cast<int64_t>(std::ceil(x)); },
        [](float x) -> int64_t { return llrint(x); }
      };
      const int clamped_mode = std::clamp(mode, 0, static_cast<int>(sizeof(quantizers) / sizeof(quantizers[0]) - 1));
      return quantizers[clamped_mode](scaled_val);
    };

    int64_t fixed_val = quantize(val / scale, quant_mode);

    // --- 4. Dynamic Range Calculation ---
    const auto [min_val, max_val] = [ = ]() -> std::pair<int64_t, int64_t> {
      if (is_signed)
      {
        const int64_t int_max = (1LL << (effective_I - 1)) - 1;
        const int64_t int_min = -(1LL << (effective_I - 1));
        const int64_t frac_mask = (F > 0) ? ((1LL << F) - 1) : 0;
        return {int_min << F, (int_max << F) | frac_mask};
      } else
      {
        const int64_t int_max = (1LL << effective_I) - 1;
        const int64_t frac_mask = (F > 0) ? ((1LL << F) - 1) : 0;
        return {0, (int_max << F) | frac_mask};
      }
    }();


    // --- 5. Overflow Handling ---
    const auto handle_overflow = [&]() {
      const int64_t bit_mask = (1LL << effective_total) - 1;
      const int64_t sign_bit = 1LL << (effective_total - 1);

      switch (overflow_mode) {
        case 0:  // AP_SAT: Clamp to [min_val, max_val]
          fixed_val = std::clamp(fixed_val, min_val, max_val);
          break;

        case 1:  // AP_SAT_ZERO: Set to 0 on overflow
          if (fixed_val > max_val || fixed_val < min_val) fixed_val = 0;
          break;

        case 2:  // AP_WRAP: Modular wrap-around
          if (is_signed) {
            fixed_val = (fixed_val & bit_mask);
            if (fixed_val & sign_bit) {
              fixed_val -= (1LL << effective_total);
            }
          } else {
            fixed_val &= bit_mask;
          }
          break;

        case 3:  // AP_WRAP_SM: Sign-magnitude wrap
          if (is_signed) {
            const int64_t magnitude_mask = (1LL << (effective_total - 1)) - 1;
            const bool is_negative = (fixed_val < 0);
            fixed_val = std::abs(fixed_val) & magnitude_mask;
            if (is_negative) fixed_val = -fixed_val;
          } else {
            fixed_val &= bit_mask;
          }
          break;

        case 4:  // AP_SAT_SYM: Symmetric saturation
          if (is_signed) {
            const int64_t sym_limit = std::min(max_val, -min_val);
            fixed_val = std::clamp(fixed_val, -sym_limit, sym_limit);
          } else {
            fixed_val = std::min(fixed_val, max_val);
          }
          break;

        default:  // Default to AP_WRAP
          fixed_val &= bit_mask;
          if (is_signed && (fixed_val & sign_bit)) {
            fixed_val -= (1LL << effective_total);
          }
      }
    };
    handle_overflow();

    // --- 6. Final Conversion ---
    return static_cast<float>(fixed_val) * scale;
  }
  //for sperical case with total_bits < int-bits (only used to this code, need modify for generic using)
  else {
    // 1. calculate the bits
    const int zero_bits = std::max(int_bits - total_bits, 0);//So called negative fraction bits
    const int useful_bits = total_bits - 1;

    // 2. value collections
    std::vector<float> legal_values;
    for (int64_t i = 0; i < (1LL << useful_bits); ++i) {
      int64_t value = i << zero_bits;
      legal_values.push_back(static_cast<float>(value));
      legal_values.push_back(static_cast<float>(-1 * value)); //need confirm
    }
    std::sort(legal_values.begin(), legal_values.end());

    // 3. find the closest values in legal_values
    auto closest = std::min_element(legal_values.begin(), legal_values.end(),
    [val](float a, float b) {
      float dist_a = std::abs(a - val);
      float dist_b = std::abs(b - val);

      if (std::abs(dist_a - dist_b) < 1e-6f) {
        return a > b;
      }
      return dist_a < dist_b;
    });

    return *closest;
  }

}

// ---  Wrappers for this code---

//=== for input layer ===//
inline float sim_input_layer_t(float val)
{
  return sim_ap_fixed(val, 13, 12, false, 0, 4, 0); // AP_TRN,AP_SAT
}

inline float sim_dense_0_iq_t(float val)
{
  return sim_ap_fixed(val, 12, 12, false, 1, 2, 0); // AP_RND,AP_WAP
}

inline float sim_ap_dense_0_iq(float val, int w, int i)
{
  return sim_ap_fixed(val, w, i, false, 1, 4, 0);  // AP_RND, AP_SAT_SYM
}

// === for weights, bias and hiden layers ===
inline float sim_fixed(float val, int total_bits, int int_bits,  bool is_signed = true,
                       int rounding = 0,     // 0: trunc, 1: round
                       int saturation = 2)   // 2: wrap
{
  return sim_ap_fixed(val, total_bits, int_bits, is_signed, rounding, saturation, 0);  // AP_TRN, AP_SAT_SYM
}

// === unsign for result ===
inline float sim_result_t(float val)
{
  return sim_ap_fixed(val, 25, 9, true, 0, 2, 0);  // AP_RND, AP_SAT_SYM
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

float
GRLNeuro::runMLP(unsigned isector, const std::vector<float>& input)
{
  const GRLMLP& expert = m_MLPs[isector];
  vector<float> weights = expert.get_weights();
  vector<float> bias = expert.get_bias();
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

  // quantizer the inputs
  for (size_t i = 0; i < layerinput.size(); ++i) {

    int W_arr[24] = { 12, 12, 11, 11, 8, 8, 7, 7, 5, 6, 6, 6, 8, 8, 6, 5, 4, 5, 7, 7, 6, 4, 5, 5 };
    int I_arr[24] = { 12, 12, 11, 11, 10, 9, 7, 7, 7, 7, 7, 7, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9 };
    if (i != 23) {
      layerinput[i] = sim_input_layer_t(layerinput[i]);
      layerinput[i] = sim_ap_dense_0_iq(layerinput[i], W_arr[i], I_arr[i]);

    } else layerinput[i] = 0;
  }

  //hidden layer and output layer
  vector<float> layeroutput = {};
  unsigned num_layers = expert.get_number_of_layers();

  unsigned num_total_neurons = 0;
  unsigned iw = 0;
  for (unsigned i_layer = 0; i_layer < num_layers - 1; i_layer++) {
    //read bias
    unsigned num_neurons = expert.get_number_of_nodes_layer(i_layer + 1);
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
    //     input*weight done
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

        layerinput[i] = sim_ap_fixed(layeroutput[i], W_input[i_layer][i], I_input[i_layer][i], false, 1, 4, 0);

      }
      if (i_layer == 0) {}
      else if (i_layer == 1) {

        layerinput[1] = 0;
        layerinput[10] = 0;
        layerinput[18] = 0;

      } else if (i_layer == 2) {
        layerinput[2] = 0;
        layerinput[16] = 0;

      }


    } else {


      // ===  HLS result_t: ap_fixed<19,5,AP_RND,AP_SAT_SYM,0> ===
      float final_fixed = sim_result_t(layeroutput[0]);

      return final_fixed;
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

        if (warray.size() != expert.n_weights_cal()) {
          B2ERROR("Number of weights is not equal to registered architecture!");
          return false;
        } else expert.set_weights(warray);
        if (barray.size() != expert.n_bias_cal()) {
          B2ERROR("Number of bias is not equal to registered architecture!");
          return false;
        }

        expert.set_weights(warray);
        expert.set_bias(barray);
        return true;
      }
    }
  }
}

bool GRLNeuro::load(unsigned isector, std::vector<float> warray, std::vector<float> barray)
{
  GRLMLP& expert = m_MLPs[isector];
  expert.set_weights(warray);
  expert.set_bias(barray);
  return true;
}
