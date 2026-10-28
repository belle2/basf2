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
        [](float x) -> int64_t { return static_cast<int64_t>(std::floor(x)); },
        [](float x) -> int64_t { return static_cast<int64_t>(std::floor(x + 0.5f)); },
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
  // for total_bits < int_bits case (e.g., ap_fixed<8,11>)
  else {
    // When W < I, the ap_fixed value has negative fractional bits.
    // Firmware behavior is: quantize val / 2^(I-W), wrap that W-bit integer,
    // then multiply back by 2^(I-W).
    const int shift_amount = int_bits - total_bits;
    const float scale = std::ldexp(1.0f, shift_amount);

    const auto quantize = [](float scaled_val, int mode) -> int64_t {
      switch (mode)
      {
        case 0:  // AP_TRN
          return static_cast<int64_t>(std::floor(scaled_val));
        case 1:  // AP_RND
          return static_cast<int64_t>(std::floor(scaled_val + 0.5f));
        case 2:  // AP_RND_ZERO
          return (scaled_val >= 0) ? static_cast<int64_t>(std::floor(scaled_val))
          : static_cast<int64_t>(std::ceil(scaled_val));
        case 3:  // AP_RND_MIN_INF
          return static_cast<int64_t>(std::floor(scaled_val));
        case 4:  // AP_RND_INF
          return static_cast<int64_t>(std::ceil(scaled_val));
        case 5:  // AP_RND_CONV
          return llrint(scaled_val);
        default:
          return static_cast<int64_t>(std::trunc(scaled_val));
      }
    };

    int64_t fixed_val = quantize(val / scale, quant_mode);
    const int64_t mask = (1LL << total_bits) - 1;

    if (overflow_mode == 2) {  // AP_WRAP
      fixed_val &= mask;
      if (is_signed) {
        const int64_t sign_bit = 1LL << (total_bits - 1);
        if (fixed_val & sign_bit) {
          fixed_val -= (1LL << total_bits);
        }
      }
    } else {
      const int64_t min_val = is_signed ? -(1LL << (total_bits - 1)) : 0;
      const int64_t max_val = is_signed ? ((1LL << (total_bits - 1)) - 1) : ((1LL << total_bits) - 1);
      if (overflow_mode == 1 && (fixed_val < min_val || fixed_val > max_val)) {
        fixed_val = 0;
      } else {
        fixed_val = std::clamp(fixed_val, min_val, max_val);
      }
    }

    return static_cast<float>(fixed_val) * scale;
  }
}

// ---  Direct sim_ap_fixed calls for firmware compatibility ---
// Parameters from firmware/defines.h:
// input_layer_t: ap_fixed<13,13>
// dense_0_iq_t: ap_fixed<12,13>
// dense_0_weight_t: ap_fixed<9,4>
// dense_0_accum_t: ap_fixed<18,16>
// dense_0_t: ap_fixed<16,16>
// dense_0_relu_t: ap_fixed<14,14>
// dense_5_iq_t: ap_ufixed<13,8>
// result_t: ap_fixed<19,10>
// quant_mode: 0=trunc, 1=round
// overflow_mode: 2=wrap, 4=AP_SAT_SYM


void
GRLNeuro::initialize(const Parameters& p)
{
  using std::vector;

  // ------------------------------------------------------------------
  // Basic parameter validation (fatal in initialize)
  // ------------------------------------------------------------------

  if (p.nHidden.size() != 1 && p.nHidden.size() != p.nMLP) {
    B2FATAL("Number of nHidden lists should be 1 or " << p.nMLP);
  }

  if (p.outputScale.size() != 1 && p.outputScale.size() != p.nMLP) {
    B2FATAL("Number of outputScale lists should be 1 or " << p.nMLP);
  }

  unsigned short nTarget = static_cast<unsigned short>(p.targetresult);
  if (nTarget < 1) {
    B2FATAL("No outputs! Turn on targetresult.");
  }

  for (unsigned iScale = 0; iScale < p.outputScale.size(); ++iScale) {
    if (p.outputScale.at(iScale).size() != 2 * nTarget) {
      B2FATAL("outputScale should be exactly " << 2 * nTarget << " values");
    }
  }

  // ------------------------------------------------------------------
  // Comprehensive parameter-size validation
  // ------------------------------------------------------------------

  auto check_size = [&](const auto & v, const char* name) {
    const size_t s = v.size();
    if (s != 1 && s != static_cast<size_t>(p.nMLP)) {
      B2FATAL(std::string(name) + " size (" + std::to_string(s)
              + ") != 1 and != nMLP (" + std::to_string(p.nMLP) + ")");
    }
  };

  // sector / structure parameters
  check_size(p.i_cdc_sector, "i_cdc_sector");
  check_size(p.i_ecl_sector, "i_ecl_sector");
  check_size(p.nHidden, "nHidden");
  check_size(p.outputScale, "outputScale");

  // bias
  check_size(p.total_bit_bias, "total_bit_bias");
  check_size(p.int_bit_bias, "int_bit_bias");
  check_size(p.is_signed_bias, "is_signed_bias");
  check_size(p.rounding_bias, "rounding_bias");
  check_size(p.saturation_bias, "saturation_bias");

  // accumulator
  check_size(p.total_bit_accum, "total_bit_accum");
  check_size(p.int_bit_accum, "int_bit_accum");
  check_size(p.is_signed_accum, "is_signed_accum");
  check_size(p.rounding_accum, "rounding_accum");
  check_size(p.saturation_accum, "saturation_accum");

  // weights
  check_size(p.total_bit_weight, "total_bit_weight");
  check_size(p.int_bit_weight, "int_bit_weight");
  check_size(p.is_signed_weight, "is_signed_weight");
  check_size(p.rounding_weight, "rounding_weight");
  check_size(p.saturation_weight, "saturation_weight");

  // relu
  check_size(p.total_bit_relu, "total_bit_relu");
  check_size(p.int_bit_relu, "int_bit_relu");
  check_size(p.is_signed_relu, "is_signed_relu");
  check_size(p.rounding_relu, "rounding_relu");
  check_size(p.saturation_relu, "saturation_relu");

  // generic
  check_size(p.total_bit, "total_bit");
  check_size(p.int_bit, "int_bit");
  check_size(p.is_signed, "is_signed");
  check_size(p.rounding, "rounding");
  check_size(p.saturation, "saturation");

  // inputs
  check_size(p.W_input, "W_input");
  check_size(p.I_input, "I_input");
  check_size(p.W_input_layer, "W_input_layer");
  check_size(p.I_input_layer, "I_input_layer");

  // ------------------------------------------------------------------
  // Initialize MLPs
  // ------------------------------------------------------------------

  m_MLPs.clear();
  m_MLPs.reserve(p.nMLP);

  for (unsigned iMLP = 0; iMLP < p.nMLP; ++iMLP) {

    // helper: broadcast if size == 1
    auto pick = [&](const auto & container) -> const auto& {
      return (container.size() == 1) ? container.at(0) : container.at(iMLP);
    };

    unsigned short i_cdc = static_cast<unsigned short>(pick(p.i_cdc_sector));
    unsigned short i_ecl = static_cast<unsigned short>(pick(p.i_ecl_sector));
    unsigned short nInput = static_cast<unsigned short>(i_cdc + i_ecl);

    // nHidden: vector<vector<float>>
    const vector<float>& nhidden = pick(p.nHidden);
    vector<unsigned short> nNodes = { nInput };

    for (unsigned iHid = 0; iHid < nhidden.size(); ++iHid) {
      if (p.multiplyHidden) {
        nNodes.push_back(static_cast<unsigned short>(nhidden.at(iHid) * nNodes.at(0)));
      } else {
        nNodes.push_back(static_cast<unsigned short>(nhidden.at(iHid)));
      }
    }

    nNodes.push_back(nTarget);
    unsigned short targetVars = static_cast<unsigned short>(p.targetresult);

    const vector<float>& outputScale = pick(p.outputScale);

    GRLMLP grlmlp_temp(nNodes, targetVars, outputScale);

    // bias
    grlmlp_temp.set_total_bit_bias(pick(p.total_bit_bias));
    grlmlp_temp.set_int_bit_bias(pick(p.int_bit_bias));
    grlmlp_temp.set_is_signed_bias(pick(p.is_signed_bias));
    grlmlp_temp.set_rounding_bias(pick(p.rounding_bias));
    grlmlp_temp.set_saturation_bias(pick(p.saturation_bias));

    // accumulator
    grlmlp_temp.set_total_bit_accum(pick(p.total_bit_accum));
    grlmlp_temp.set_int_bit_accum(pick(p.int_bit_accum));
    grlmlp_temp.set_is_signed_accum(pick(p.is_signed_accum));
    grlmlp_temp.set_rounding_accum(pick(p.rounding_accum));
    grlmlp_temp.set_saturation_accum(pick(p.saturation_accum));

    // weights
    grlmlp_temp.set_total_bit_weight(pick(p.total_bit_weight));
    grlmlp_temp.set_int_bit_weight(pick(p.int_bit_weight));
    grlmlp_temp.set_is_signed_weight(pick(p.is_signed_weight));
    grlmlp_temp.set_rounding_weight(pick(p.rounding_weight));
    grlmlp_temp.set_saturation_weight(pick(p.saturation_weight));

    // relu
    grlmlp_temp.set_total_bit_relu(pick(p.total_bit_relu));
    grlmlp_temp.set_int_bit_relu(pick(p.int_bit_relu));
    grlmlp_temp.set_is_signed_relu(pick(p.is_signed_relu));
    grlmlp_temp.set_rounding_relu(pick(p.rounding_relu));
    grlmlp_temp.set_saturation_relu(pick(p.saturation_relu));

    // generic
    grlmlp_temp.set_total_bit(pick(p.total_bit));
    grlmlp_temp.set_int_bit(pick(p.int_bit));
    grlmlp_temp.set_is_signed(pick(p.is_signed));
    grlmlp_temp.set_rounding(pick(p.rounding));
    grlmlp_temp.set_saturation(pick(p.saturation));

    // inputs
    grlmlp_temp.set_W_input(pick(p.W_input));
    grlmlp_temp.set_I_input(pick(p.I_input));
    // input layer quantization parameters
    grlmlp_temp.set_W_input_layer(pick(p.W_input_layer));
    grlmlp_temp.set_I_input_layer(pick(p.I_input_layer));

    m_MLPs.push_back(std::move(grlmlp_temp));
  }
}



std::vector<float>
GRLNeuro::runMLP(unsigned isector, const std::vector<float>& input)
{
  const GRLMLP& expert = m_MLPs.at(isector);
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
  // Input layer quantization parameters
  vector<vector<int>> W_input_layer = expert.get_W_input_layer();
  vector<vector<int>> I_input_layer = expert.get_I_input_layer();

  // Use actual input
  vector<float> layerinput = input;

  // Input layer quantization: firmware uses TWO steps:
  // Step 1: float -> input_layer_t (ap_fixed<13,13>) with AP_RND, AP_SAT_SYM
  // Step 2: input_layer_t -> dense_0_iq_t (per-input ap_fixed<W,I>) with AP_RND, AP_WRAP

  // First, quantize raw input to input_layer_t (ap_fixed<13,13>)
  // Firmware: ap_fixed<13,13> with AP_TRN (signed: floor toward -inf), AP_WRAP (defaults)
  for (size_t i = 0; i < layerinput.size(); ++i) {
    // AP_TRN for signed = floor (toward -inf), AP_WRAP
    layerinput.at(i) = sim_ap_fixed(layerinput.at(i), 13, 13, true, 3, 2,
                                    0);  // signed, quant_mode=3 (floor), overflow_mode=2 (AP_WRAP)
  }

  // Use the first layer's W/I for input quantization (dense_0_iq)
  const auto& w_layer = W_input_layer.at(0);
  const auto& i_layer = I_input_layer.at(0);

  // Step 2: dense_0_iq quantization with per-input W, I
  for (size_t i = 0; i < layerinput.size(); ++i) {
    // dense_0_iq quantization: signed, quant_mode=1 (AP_RND), overflow_mode=2 (AP_WRAP)
    layerinput.at(i) = sim_ap_fixed(layerinput.at(i), w_layer.at(i), i_layer.at(i), true, 1, 2, 0);
  }

  //hidden layer and output layer
  vector<float> layeroutput = {};
  unsigned num_layers = expert.get_number_of_layers();

  unsigned num_total_neurons = 0;
  unsigned iw = 0;
  for (unsigned iw_layer = 0; iw_layer < num_layers - 1; iw_layer++) {
    //read bias
    unsigned num_neurons = expert.get_number_of_nodes_layer(iw_layer + 1);
    layeroutput.clear();
    layeroutput.assign(num_neurons, 0.);
    layeroutput.shrink_to_fit();


    for (unsigned io = 0; io < num_neurons; ++io) {
      float bias_raw = bias.at(io + num_total_neurons);
      float bias_fixed   = sim_ap_fixed(bias_raw, total_bit_bias.at(iw_layer), int_bit_bias.at(iw_layer), is_signed_bias.at(iw_layer),
                                        rounding_bias.at(iw_layer), saturation_bias.at(iw_layer), 0);
      float bias_contrib = sim_ap_fixed(bias_fixed, total_bit_accum.at(iw_layer), int_bit_accum.at(iw_layer),
                                        is_signed_accum.at(iw_layer),
                                        rounding_accum.at(iw_layer), saturation_accum.at(iw_layer), 0);
      layeroutput.at(io) = bias_contrib;
    }
    num_total_neurons += num_neurons;

    //input*weight
    unsigned num_inputs = layerinput.size();

    for (unsigned ii = 0; ii < num_inputs; ++ii) {
      float input_val = layerinput.at(ii);
      for (unsigned io = 0; io < num_neurons; ++io) {
        float weight_raw = weights.at(iw);
        float weight_fixed = sim_ap_fixed(weight_raw, total_bit_weight.at(iw_layer), int_bit_weight.at(iw_layer),
                                          is_signed_weight.at(iw_layer),
                                          rounding_weight.at(iw_layer), saturation_weight.at(iw_layer), 0);
        float product = input_val * weight_fixed;
        float contrib = sim_ap_fixed(product, total_bit_accum.at(iw_layer), int_bit_accum.at(iw_layer), is_signed_accum.at(iw_layer),
                                     rounding_accum.at(iw_layer), saturation_accum.at(iw_layer), 0);
        layeroutput.at(io) = sim_ap_fixed(layeroutput.at(io) + contrib, total_bit_accum.at(iw_layer), int_bit_accum.at(iw_layer),
                                          is_signed_accum.at(iw_layer), rounding_accum.at(iw_layer), saturation_accum.at(iw_layer), 0);
        ++iw;
      }
    }
    //     input*weight done

    if (iw_layer < num_layers - 2) {
      //relu
      for (unsigned io = 0; io < num_neurons; ++io) {
        float fixed_val = sim_ap_fixed(layeroutput.at(io), total_bit.at(iw_layer), int_bit.at(iw_layer), is_signed.at(iw_layer), 0,
                                       saturation.at(iw_layer), 0);
        float relu_val = (fixed_val > 0) ? fixed_val : 0;
        layeroutput.at(io) = sim_ap_fixed(relu_val, total_bit_relu.at(iw_layer), int_bit_relu.at(iw_layer), is_signed_relu.at(iw_layer),
                                          rounding_relu.at(iw_layer), saturation_relu.at(iw_layer), 0);
      }
      //input to next layer
      layerinput.clear();
      layerinput.assign(num_neurons, 0);
      layerinput.shrink_to_fit();

      for (unsigned i = 0; i < num_neurons; ++i) {
        // firmware: use ap_fixed with parameters from W_input/I_input
        // quant_mode=1 (AP_RND), overflow_mode=2 (AP_WRAP)
        // W=0 means this input is zeroed (sparse quantization)
        if (W_input.at(iw_layer).at(i) == 0) {
          layerinput.at(i) = 0.0f;
        } else {
          const bool iq_is_signed = (iw_layer != 3);  // dense_4_iq is ap_ufixed
          layerinput.at(i) = sim_ap_fixed(layeroutput.at(i), W_input.at(iw_layer).at(i), I_input.at(iw_layer).at(i), iq_is_signed, 1, 2, 0);
        }
      }
    } else {
      // Final output layer - return all 10 outputs
      // firmware: result_t is ap_fixed<19,10> with default AP_TRN (signed floor), AP_WRAP
      vector<float> final_output(10, 0.0f);
      for (unsigned io = 0; io < 10; ++io) {
        final_output.at(io) = sim_ap_fixed(layeroutput.at(io), 19, 10, true, 3, 2, 0);
      }
      return final_output;
    }
  }

  return vector<float>(10, 0.0f);
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
        GRLMLP& expert = m_MLPs.at(isector);
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
  GRLMLP& expert = m_MLPs.at(isector);
  expert.set_weights(warray);
  expert.set_bias(barray);
  return true;
}
