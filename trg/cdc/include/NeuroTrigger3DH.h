/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <cmath>

#include "trg/cdc/dataobjects/CDCTriggerHoughMLP.h"
#include "trg/cdc/dataobjects/CDCTriggerSegmentHit.h"
#include "trg/cdc/dataobjects/CDCTrigger3DHTrack.h"
#include "framework/datastore/StoreArray.h"

namespace Belle2 {

  // Represents the neuro trigger using a single MLP with 3DHough Finder (NDFinder) input
  class NeuroTrigger3DH {
  public:

    // Default constructor
    NeuroTrigger3DH() = default;

    // Default destructor
    virtual ~NeuroTrigger3DH() = default;

    // Initialize CDC geometry
    void initialize();
    // Initialize the track segment hit collections
    void initializeCollections(std::string hitCollectionName);
    // Create the integer weights for the fixed point precision
    void createIntWeights();
    // Calculate the crossing angles (m_alpha) and reference IDs (m_referenceID) for each super layer
    void calculateTrackParameters(const CDCTrigger3DHTrack& track);
    // Calculate the same track parameters but with fixed floating point accuracy
    void calculateTrackParametersFixedPrecision(const CDCTrigger3DHTrack& track);
    // Set the event time based on the 3DFinder track segment hits
    void setEventTime(const CDCTrigger3DHTrack& track);
    // Load the 3DFinder track segment hits
    std::vector<size_t> load3DHits(const CDCTrigger3DHTrack& track) const;
    // Calculate the input vector for the MLP
    std::vector<float> getInputVector(const std::vector<size_t>& hitIds) const;
    // Run the neural network (MLP) with the input vector
    std::vector<float> runMLP(const std::vector<float>& input) const;
    // Run the MLP with fixed point arithmetic
    std::vector<float> runMLPFixedPrecision(const std::vector<float>& input) const;
    // Scale target value from [-1, 1] to outputScale
    std::vector<float> unscaleTarget(const std::vector<float>& target) const;
    // Scale target value from outputScale to [-1, 1]
    std::vector<float> scaleTarget(const std::vector<float>& target) const;

    // Get the full precision of the weights of the fixed point (integer) network
    int getWeightBits() const { return m_precisionWeights; }
    // Get the fractional weight bits of the fixed point (integer) network
    int getFractionalWeightBits() const { return m_fractionalWeightBits; }
    // Get the integerized weights of the fixed point network
    const std::vector<int32_t>& getIntWeights() const { return m_intWeights; }
    // Get the event time
    int getEventTime() const { return m_T0; };
    // Set the network parameters
    void setNeuroParameters(const NeuroParametersHough& neuroParameters3DH) { m_neuroParameters3DH = neuroParameters3DH; };
    // Set the MLP (includes the neuro parameters)
    void setMLP(const CDCTrigger3DHMLP& mlp);
    // Get the MLP
    const CDCTrigger3DHMLP& getMLP() const { return m_MLP; };

  private:
    // Get the lowest priority time from the track segment hits
    int getLowestTime(const RelationVector<CDCTriggerSegmentHit>& trackSegmentHits) const;
    // Calculate the scaled drift time of a track segment hit for the network input
    float getScaledDriftTime(const CDCTriggerSegmentHit& trackSegmentHit, const unsigned short maxTime) const;
    // Calculates the relative wire ID difference between the 3DFinder track and the priority hit
    double getRelativeID(const CDCTriggerSegmentHit& hit) const;
    // Scale relative TS ID from relevant range to approximately [-1, 1]
    float scaleRelativeID(const double relativeID, const unsigned superLayerIdx) const;

    // One instance of the MLP (network)
    CDCTrigger3DHMLP m_MLP;
    // Integer weights for fixed point precision
    std::vector<int32_t> m_intWeights;
    // Fractional weight bits of the network
    int m_fractionalWeightBits;
    // Settings loaded from the root file
    NeuroParametersHough m_neuroParameters3DH;
    // StoreArray containing the input track segment hits
    StoreArray<CDCTriggerSegmentHit> m_segmentHits;
    // Name of the StoreArray containing the input track segment hits
    std::string m_hitCollectionName;

    // Number of super layers
    static constexpr unsigned int m_nSL = 9;
    // The priority wire radius of each super layer (first and second priorities)
    std::array<std::array<double, 2>, m_nSL> m_radiusWireLayer{};
    // The cumulative priority wires (i.e., track segments) in each super layer
    std::array < unsigned short, m_nSL + 1 > m_cumulativeWires{};
    // The number of wires in each super layer
    std::array<unsigned short, m_nSL> m_nWires{};
    // referenceID = (phiTrack - alpha) * wiresInLayer / (2 * pi)
    double m_referenceID[m_nSL][2] = {};
    // 2D crossing angle of current track
    double m_alpha[m_nSL][2] = {};
    // Event time of current event/track
    int m_T0 = 0;
    // Flag to show if stored event time is valid
    bool m_hasT0 = false;

    // Number of supported input nodes for standard and extended input of the neural network
    static constexpr unsigned int m_nStandardInputNodes = 27;
    static constexpr unsigned int m_nExtendedInputNodes = 126;

    // The integer bit precisions for the fixed point parameter calculation
    static constexpr int m_precisionPhi = 12;
    static constexpr int m_precisionAlpha = 12;
    static constexpr int m_precisionScaleFactor = 8;
    static constexpr int m_precisionReferenceID = 8;

    // The integer bit precisions for the fixed point network calculation
    static constexpr int m_precisionInputs = 13;
    static constexpr int m_precisionWeights = 17;

    // Tanh activation function LUT (hardware)
    struct TanhLUT {
      // How many entries the LUT has
      static constexpr int LUT_INDEX_BITS = 12;
      static constexpr std::size_t LUT_SIZE = 1 << LUT_INDEX_BITS;

      // Quantization of the outputs
      static constexpr int LUT_FRAC_BITS = 12;
      static constexpr int LUT_SCALE = 1 << LUT_FRAC_BITS;

      // Function domain limit (saturation limit)
      static constexpr int MAX_FLOAT_SHIFT = 2;
      static constexpr int MAX_FLOAT = 1 << MAX_FLOAT_SHIFT;
      static constexpr int LUT_SATURATION = 1 << (LUT_FRAC_BITS + MAX_FLOAT_SHIFT);

      // Headroom for the DSP cut window (high bit cutoff to ensure full representation):
      // Using MAX_OBSERVED = 10.439 (in simulation of > 50 mio tracks)
      // With HEADROOM_BITS = std::ceil(std::log2(MAX_OBSERVED / MAX_FLOAT)) + 1
      static constexpr int HEADROOM_BITS = 3;

      inline static const std::array<int32_t, LUT_SIZE> table = []()
      {
        std::array<int32_t, LUT_SIZE> lut{};
        for (std::size_t i = 0; i < LUT_SIZE; ++i) {
          double input = MAX_FLOAT * static_cast<double>(i) / LUT_SIZE;
          double val   = std::tanh(input) * LUT_SCALE;
          int32_t qval = static_cast<int32_t>(std::round(val));
          // Clamp to maximum bit range [0, LUT_SCALE - 1] (maps LUT_SCALE -> LUT_SCALE - 1)
          qval = std::min<int32_t>(qval, (LUT_SCALE - 1));
          lut[i] = qval;
        }
        return lut;
      }();

      static int32_t get(std::size_t idx) { return table[idx]; }
    };

    // Cut a bit window from signed int (hardware truncation)
    static constexpr int32_t extractBits(int64_t value, int high, int low)
    {
      int width = high - low + 1;
      int64_t shifted = value >> low;
      int32_t mask = (1 << width) - 1;
      int32_t result = static_cast<int32_t>(shifted & mask);
      if (result & (1 << (width - 1))) {
        result |= ~mask;
      }
      return result;
    }
  };
}
