/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef GRLMLP_H
#define GRLMLP_H

#include <TObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  /** Class to keep all parameters of an expert MLP for the neuro trigger. */
  class GRLMLP : public TObject {

    // weights etc. are set only by the trainer
    friend class GRLNeuroTrainerModule;

  public:
    /** default constructor. */
    GRLMLP();

    /** constructor to set all parameters (not weights and relevantID ranges). */
    GRLMLP(std::vector<unsigned short>& nodes, unsigned short targets, const std::vector<float>& outputscale);

    /** destructor, empty because we don't allocate memory anywhere. */
    ~GRLMLP() { }

    /** check if weights are default values or set by some trainer */
    bool isTrained() const { return m_trained; }
    /** get number of layers */
    unsigned getNumberOfLayers() const { return m_nNodes.size(); }
    /** get number of nodes in a layer */
    unsigned getNumberOfNodesLayer(unsigned iLayer) const { return m_nNodes[iLayer]; }
    /** get number of weights from length of weights vector */
    unsigned getNumberOfWeights() const { return m_weights.size(); }
    /** calculate number of weights from number of nodes */
    unsigned nWeightsCal() const;
    /** calculate number of weights from number of nodes */
    unsigned nBiasCal() const;
    /** get weights vector */
    std::vector<float> getWeights() const { return m_weights; }
    /** get bias vector */
    std::vector<float> getBias() const { return m_bias; }
    /** set weights vector */
    void setWeights(std::vector<float>& weights) { m_weights = weights; }
    /** set bias vector */
    void setBias(std::vector<float>& bias) { m_bias = bias; }

    /** get bit width etc. constant in each node */
    std::vector<int> get_total_bit_bias() const { return m_total_bit_bias; }
    std::vector<int> get_int_bit_bias() const { return m_int_bit_bias; }
    std::vector<bool> get_is_signed_bias() const { return m_is_signed_bias; }
    std::vector<int> get_rounding_bias() const { return m_rounding_bias; }
    std::vector<int> get_saturation_bias() const { return m_saturation_bias; }
    std::vector<int> get_total_bit_accum() const { return m_total_bit_accum; }
    std::vector<int> get_int_bit_accum() const { return m_int_bit_accum; }
    std::vector<bool> get_is_signed_accum() const { return m_is_signed_accum; }
    std::vector<int> get_rounding_accum() const { return m_rounding_accum; }
    std::vector<int> get_saturation_accum() const { return m_saturation_accum; }
    std::vector<int> get_total_bit_weight() const { return m_total_bit_weight; }
    std::vector<int> get_int_bit_weight() const { return m_int_bit_weight; }
    std::vector<bool> get_is_signed_weight() const { return m_is_signed_weight; }
    std::vector<int> get_rounding_weight() const { return m_rounding_weight; }
    std::vector<int> get_saturation_weight() const { return m_saturation_weight; }
    std::vector<int> get_total_bit_relu() const { return m_total_bit_relu; }
    std::vector<int> get_int_bit_relu() const { return m_int_bit_relu; }
    std::vector<bool> get_is_signed_relu() const { return m_is_signed_relu; }
    std::vector<int> get_rounding_relu() const { return m_rounding_relu; }
    std::vector<int> get_saturation_relu() const { return m_saturation_relu; }
    std::vector<int> get_total_bit() const { return m_total_bit; }
    std::vector<int> get_int_bit() const { return m_int_bit; }
    std::vector<bool> get_is_signed() const { return m_is_signed; }
    std::vector<int> get_rounding() const { return m_rounding; }
    std::vector<int> get_saturation() const { return m_saturation; }
    std::vector<std::vector<int>> get_W_input() const { return m_W_input; }
    std::vector<std::vector<int>> get_I_input() const { return m_I_input; }

    /** set bit width etc. constant in each node */
    void set_total_bit_bias(const std::vector<int>& i) { m_total_bit_bias = i; }
    void set_int_bit_bias(const std::vector<int>& i) { m_int_bit_bias = i; }
    void set_is_signed_bias(const std::vector<bool>& i) { m_is_signed_bias = i; }
    void set_rounding_bias(const std::vector<int>& i) { m_rounding_bias = i; }
    void set_saturation_bias(const std::vector<int>& i) { m_saturation_bias = i; }
    void set_total_bit_accum(const std::vector<int>& i) { m_total_bit_accum = i; }
    void set_int_bit_accum(const std::vector<int>& i) { m_int_bit_accum = i; }
    void set_is_signed_accum(const std::vector<bool>& i) { m_is_signed_accum = i; }
    void set_rounding_accum(const std::vector<int>& i) { m_rounding_accum = i; }
    void set_saturation_accum(const std::vector<int>& i) { m_saturation_accum = i; }
    void set_total_bit_weight(const std::vector<int>& i) { m_total_bit_weight = i; }
    void set_int_bit_weight(const std::vector<int>& i) { m_int_bit_weight = i; }
    void set_is_signed_weight(const std::vector<bool>& i) { m_is_signed_weight = i; }
    void set_rounding_weight(const std::vector<int>& i) { m_rounding_weight = i; }
    void set_saturation_weight(const std::vector<int>& i) { m_saturation_weight = i; }
    void set_total_bit_relu(const std::vector<int>& i) { m_total_bit_relu = i; }
    void set_int_bit_relu(const std::vector<int>& i) { m_int_bit_relu = i; }
    void set_is_signed_relu(const std::vector<bool>& i) { m_is_signed_relu = i; }
    void set_rounding_relu(const std::vector<int>& i) { m_rounding_relu = i; }
    void set_saturation_relu(const std::vector<int>& i) { m_saturation_relu = i; }
    void set_total_bit(const std::vector<int>& i) { m_total_bit = i; }
    void set_int_bit(const std::vector<int>& i) { m_int_bit = i; }
    void set_is_signed(const std::vector<bool>& i) { m_is_signed = i; }
    void set_rounding(const std::vector<int>& i) { m_rounding = i; }
    void set_saturation(const std::vector<int>& i) { m_saturation = i; }
    void set_W_input(const std::vector<std::vector<int>>& i) { m_W_input = i; }
    void set_I_input(const std::vector<std::vector<int>>& i) { m_I_input = i; }

    /** check if weights are default values or set by some trainer */
    void Trained(bool trained) { m_trained = trained; }

  private:
    /** Number of nodes in each layer, not including bias nodes. */
    std::vector<unsigned short> m_nNodes;
    /** Weights of the network. */
    std::vector<float> m_weights;
    /** bias of the network. */
    std::vector<float> m_bias;
    /** bit width etc. constant in each node */
    std::vector<int> m_total_bit_bias;
    std::vector<int> m_int_bit_bias;
    std::vector<bool> m_is_signed_bias;
    std::vector<int> m_rounding_bias;
    std::vector<int> m_saturation_bias;
    std::vector<int> m_total_bit_accum;
    std::vector<int> m_int_bit_accum;
    std::vector<bool> m_is_signed_accum;
    std::vector<int> m_rounding_accum;
    std::vector<int> m_saturation_accum;
    std::vector<int> m_total_bit_weight;
    std::vector<int> m_int_bit_weight;
    std::vector<bool> m_is_signed_weight;
    std::vector<int> m_rounding_weight;
    std::vector<int> m_saturation_weight;
    std::vector<int> m_total_bit_relu;
    std::vector<int> m_int_bit_relu;
    std::vector<bool> m_is_signed_relu;
    std::vector<int> m_rounding_relu;
    std::vector<int> m_saturation_relu;
    std::vector<int> m_total_bit;
    std::vector<int> m_int_bit;
    std::vector<bool> m_is_signed;
    std::vector<int> m_rounding;
    std::vector<int> m_saturation;
    std::vector<std::vector<int>> m_W_input;
    std::vector<std::vector<int>> m_I_input;

    /** Indicator whether the weights are just default values or have
     *  been set by some trainer (set to true when setWeights() is first called). */
    bool m_trained;

    /** output variables: 1: z, 2: theta, 3: (z, theta) */
    unsigned short m_targetVars;
    /** Output[i] of the MLP is scaled from [-1, 1]
     *  to [outputScale[2i], outputScale[2i+1]]. */
    std::vector<float> m_outputScale;

    //! Needed to make the ROOT object storable
    ClassDef(GRLMLP, 3);
  };
}
#endif
