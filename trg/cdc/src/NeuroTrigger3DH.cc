/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/cdc/NeuroTrigger3DH.h"

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

#include "trg/cdc/dataobjects/CDCTrigger3DHTrack.h"
#include "framework/gearbox/Unit.h"
#include "framework/datastore/StoreArray.h"
#include "framework/logging/Logger.h"
#include "cdc/geometry/CDCGeometryPar.h"

using namespace Belle2;

// Initialize CDC geometry
void NeuroTrigger3DH::initialize()
{
  std::array<std::vector<int>, m_nSL> getWireLayerID;

  int layer = 0;
  // First superlayer (8 layers)
  for (int i = 0; i < 8; ++i) {
    getWireLayerID[0].push_back(layer++);
  }
  // Remaining superlayers (6 layers each)
  for (unsigned short superLayerIdx = 1; superLayerIdx < m_nSL; ++superLayerIdx) {
    for (int i = 0; i < 6; ++i) {
      getWireLayerID[superLayerIdx].push_back(layer++);
    }
  }

  const CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();

  for (unsigned short superLayerIdx = 0; superLayerIdx < m_nSL; ++superLayerIdx) {
    if (superLayerIdx == 0) {
      m_radiusWireLayer[superLayerIdx][0] = cdc.senseWireR(getWireLayerID[superLayerIdx][3]);
      m_radiusWireLayer[superLayerIdx][1] = cdc.senseWireR(getWireLayerID[superLayerIdx][4]);
    } else {
      m_radiusWireLayer[superLayerIdx][0] = cdc.senseWireR(getWireLayerID[superLayerIdx][2]);
      m_radiusWireLayer[superLayerIdx][1] = cdc.senseWireR(getWireLayerID[superLayerIdx][3]);
    }
    m_nWires[superLayerIdx] = cdc.nWiresInLayer(getWireLayerID[superLayerIdx][0]);
  }

  m_cumulativeWires[0] = 0;
  for (unsigned short superLayerIdx = 1; superLayerIdx <= m_nSL; ++superLayerIdx) {
    m_cumulativeWires[superLayerIdx] = m_cumulativeWires[superLayerIdx - 1] + m_nWires[superLayerIdx - 1];
  }
}

// Initialize the track segment hit collections
void NeuroTrigger3DH::initializeCollections(std::string hitCollectionName)
{
  m_segmentHits.isRequired(hitCollectionName);
  m_hitCollectionName = hitCollectionName;
}

// Set the MLP (includes the neuro parameters)
void NeuroTrigger3DH::setMLP(const CDCTrigger3DHMLP& mlp)
{
  m_MLP = mlp;
  m_neuroParameters3DH = mlp.getNeuroParameters();
}

// Transform the weights and biases (range [-4, 4]) to precisionWeights bit integers
void NeuroTrigger3DH::createIntWeights()
{
  const std::vector<float>& floatWeights = m_MLP.getFloatWeights();
  float minFloatWeight = *std::min_element(floatWeights.begin(), floatWeights.end());
  float maxFloatWeight = *std::max_element(floatWeights.begin(), floatWeights.end());
  float maxAbsoluteWeight = std::max(std::abs(minFloatWeight), std::abs(maxFloatWeight));

  // Calculate the necessary fractional bit size to not exceed precisionWeights for the weights
  int shift = static_cast<int>(std::ceil(std::log2(maxAbsoluteWeight)));
  int fractionalWeightBits = m_precisionWeights - shift;

  // Transform the weights and return the fractional bit size for scaling
  std::vector<int32_t> intWeights(floatWeights.size());
  std::transform(floatWeights.begin(), floatWeights.end(), intWeights.begin(),
  [fractionalWeightBits](float w) { return static_cast<int32_t>(std::round(w * (1 << fractionalWeightBits))); });

  m_intWeights = intWeights;
  m_fractionalWeightBits = fractionalWeightBits;
}

// Calculate the crossing angles (m_alpha) and reference IDs (m_referenceID) for each super layer
void NeuroTrigger3DH::calculateTrackParameters(const CDCTrigger3DHTrack& track)
{
  const double omega = track.getOmega();
  const double phi = track.getPhi0();
  for (unsigned short superLayerIdx = 0; superLayerIdx < m_nSL; ++superLayerIdx) {
    for (int priorityLayer = 0; priorityLayer < 2; ++priorityLayer) {
      double radiusWireLayer = m_radiusWireLayer[superLayerIdx][priorityLayer];
      double alpha = (radiusWireLayer * std::abs(omega) < 2.0)
                     ? std::asin(radiusWireLayer * omega / 2.0)
                     : M_PI_2;
      double referenceID = (phi - alpha) * m_nWires[superLayerIdx] / (2.0 * M_PI);
      m_alpha[superLayerIdx][priorityLayer] = alpha;
      m_referenceID[superLayerIdx][priorityLayer] = std::remainder(referenceID, m_nWires[superLayerIdx]);
    }
  }
}

// Calculate the same track parameters but with fixed floating point accuracy
void NeuroTrigger3DH::calculateTrackParametersFixedPrecision(const CDCTrigger3DHTrack& track)
{
  const long scalePhi = 1L << m_precisionPhi;
  const long scaleAlpha = 1L << m_precisionAlpha;
  const long scaleFactor = 1L << m_precisionScaleFactor;
  const long scaleID = 1L << m_precisionReferenceID;

  const double omega = track.getOmega();
  const long phiFixed = std::round(track.getPhi0() * scalePhi);

  for (unsigned short int superLayerIdx = 0; superLayerIdx < m_nSL; ++superLayerIdx) {
    const long scaledWireFactor = std::round((m_nWires[superLayerIdx] / (2.0 * M_PI)) * scaleFactor);
    for (int priorityLayer = 0; priorityLayer < 2; ++priorityLayer) {
      const double radius = m_radiusWireLayer[superLayerIdx][priorityLayer];
      long alphaFixed = (radius * std::abs(omega) < 2.0)
                        ? std::round(std::asin(radius * omega / 2.0) * scaleAlpha)
                        : std::round(M_PI_2 * scaleAlpha);
      long deltaPhi = phiFixed - alphaFixed;
      long refIDFixed = (deltaPhi * scaledWireFactor) / (1L << (m_precisionPhi + m_precisionScaleFactor - m_precisionReferenceID));
      m_alpha[superLayerIdx][priorityLayer] = static_cast<double>(alphaFixed) / scaleAlpha;
      m_referenceID[superLayerIdx][priorityLayer] = static_cast<double>(refIDFixed) / scaleID;
    }
  }
}

// Set the event time based on the 3DFinder track segment hits
void NeuroTrigger3DH::setEventTime(const CDCTrigger3DHTrack& track)
{
  m_T0 = getLowestTime(track.getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName));
  if (m_T0 < 9999) {
    m_hasT0 = true;
  } else {
    m_T0 = 0;
    m_hasT0 = false;
  }
}

// Get the lowest priority time from the track segment hits
int NeuroTrigger3DH::getLowestTime(const RelationVector<CDCTriggerSegmentHit>& trackSegmentHits) const
{
  int lowestTime = 9999;
  for (unsigned int hitIdx = 0; hitIdx < trackSegmentHits.size(); ++hitIdx) {
    if (trackSegmentHits.weight(hitIdx) < 0) continue;
    if (trackSegmentHits[hitIdx]->priorityTime() < lowestTime) {
      lowestTime = trackSegmentHits[hitIdx]->priorityTime();
    }
  }
  return lowestTime;
}

// Load the 3DFinder track segment hits
std::vector<size_t> NeuroTrigger3DH::load3DHits(const CDCTrigger3DHTrack& track) const
{
  RelationVector<CDCTriggerSegmentHit> trackSegmentHits =
    track.getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName);
  std::vector<size_t> allHitIDs;
  for (size_t hitIdx = 0; hitIdx < trackSegmentHits.size(); ++hitIdx) {
    if (trackSegmentHits.weight(hitIdx) >= 0) {  // Ignore hits with negative relation weight
      allHitIDs.push_back(trackSegmentHits[hitIdx]->getArrayIndex());
    }
  }
  return allHitIDs;
}

// Calculate the input vector for the MLP
std::vector<float> NeuroTrigger3DH::getInputVector(const std::vector<size_t>& hitIds) const
{
  const size_t nInput = m_neuroParameters3DH.nInput;

  if (nInput != m_nStandardInputNodes && nInput != m_nExtendedInputNodes) {
    B2FATAL("Invalid neural network input size");
  }
  // Maximal drift time (for scaling), hits with larger values are ignored.
  constexpr unsigned short tMax = 256;

  const bool hasExtendedInput = (nInput == m_nExtendedInputNodes);
  const size_t featuresPerHit = nInput / m_nSL;
  const size_t patternSize = 11;

  std::vector<float> inputVector(nInput, 0.0f);
  for (const size_t hitIdx : hitIds) {
    const CDCTriggerSegmentHit* trackSegmentHit = m_segmentHits[hitIdx];
    const unsigned short priority = trackSegmentHit->getPriorityPosition();
    const unsigned short superLayerIdx = trackSegmentHit->getISuperLayer();
    const size_t priorityLayer = (priority == 3) ? 0 : 1;
    const size_t baseIdx = featuresPerHit * superLayerIdx;

    const float relativeID = scaleRelativeID(getRelativeID(*trackSegmentHit), superLayerIdx);
    const float alpha = m_alpha[superLayerIdx][priorityLayer];
    const float scaledDriftTime = getScaledDriftTime(*trackSegmentHit, tMax);

    inputVector[baseIdx] = relativeID;
    inputVector[baseIdx + 1] = scaledDriftTime;
    inputVector[baseIdx + 2] = alpha;

    if (m_hasT0 && hasExtendedInput) {
      unsigned int hitPattern = trackSegmentHit->gethitpattern();
      for (size_t j = 0; j < patternSize; ++j) {
        inputVector[baseIdx + 3 + j] = (hitPattern >> j) & 1;
      }
    }
  }
  return inputVector;
}

// Calculates the relative wire ID difference between the 3DFinder track and the priority hit
double NeuroTrigger3DH::getRelativeID(const CDCTriggerSegmentHit& trackSegmentHit) const
{
  const int superLayerIdx = trackSegmentHit.getISuperLayer();
  const unsigned int priority = trackSegmentHit.getPriorityPosition();
  const unsigned int priorityLayer = (priority == 3) ? 0 : 1;
  // 0 = no hit, 1 = 2nd right, 2 = 2nd left, 3 = fist priority
  constexpr std::array<short, 4> priorityOffsetFactor = {0, -1, 1, 0};
  double relativeID = trackSegmentHit.getSegmentID()
                      - m_cumulativeWires[superLayerIdx]
                      + 0.5 * priorityOffsetFactor[priority]
                      - m_referenceID[superLayerIdx][priorityLayer];
  return std::remainder(relativeID, m_nWires[superLayerIdx]);
}

// Scale relative TS ID from relevant range to approximately [-1, 1]
float NeuroTrigger3DH::scaleRelativeID(const double relativeID, const unsigned superLayerIdx) const
{
  // Relevant ID ranges for each super layer (2x9)
  constexpr std::array<float, 18> relevantID =
  {-1.5, 1.5, -7.5, -0.5, -1.5, 1.5, 0.5, 7.5, -1.5, 1.5, -8.5, 0.5, -2.5, 1.5, -0.5, 10.5, -3.5, 2.5};
  const float idMin = relevantID[2 * superLayerIdx];
  const float idMax = relevantID[2 * superLayerIdx + 1];
  const float range = idMax - idMin;
  float scale = 2.0f / range;
  // Round down to nearest power of 2 for FPGA implementation
  scale = std::pow(2.0f, std::floor(std::log2(scale)));
  const float offset = (idMin + idMax) * 0.5f;
  return scale * static_cast<float>(relativeID - offset);
}

// Calculate the scaled drift time of a track segment hit for the network input
float NeuroTrigger3DH::getScaledDriftTime(const CDCTriggerSegmentHit& trackSegmentHit, const unsigned short maxTime) const
{
  const float scaleT = std::pow(2.0f, std::floor(std::log2(1.0f / maxTime)));
  int driftTime = (m_hasT0) ? trackSegmentHit.priorityTime() - m_T0 : 0;
  driftTime = std::clamp(driftTime, 0, static_cast<int>(maxTime));
  // 0 = no hit, 1 = right, 2 = left, 3 = undetermined
  constexpr std::array<short, 4> leftRightFactor = {0, -1, 1, 0};
  float scaledDriftTime = leftRightFactor[trackSegmentHit.getLeftRight()] * driftTime * scaleT;
  return scaledDriftTime;
}

// Run the neural network (MLP) with the input vector
std::vector<float> NeuroTrigger3DH::runMLP(const std::vector<float>& input) const
{
  const CDCTrigger3DHMLP& network = m_MLP;
  const std::vector<float>& weights = network.getFloatWeights();

  std::vector<float> layerInput = input;
  std::vector<float> layerOutput;
  size_t weightIndex = 0;

  const size_t nLayers = network.getNumberOfLayers();
  for (size_t layerIdx = 1; layerIdx < nLayers; ++layerIdx) {
    layerInput.push_back(1.0f); // add bias
    const size_t nInputs = layerInput.size();
    const size_t nOutputs = network.getNumberOfNodes(layerIdx);
    layerOutput.assign(nOutputs, 0.0f); // create new empty output vector
    for (size_t outIdx = 0; outIdx < nOutputs; ++outIdx) {
      for (size_t inputIdx = 0; inputIdx < nInputs; ++inputIdx) {
        layerOutput[outIdx] += layerInput[inputIdx] * weights[weightIndex++];
      }
      layerOutput[outIdx] = std::tanh(layerOutput[outIdx]);
    }
    layerInput = layerOutput;
  }
  return unscaleTarget(layerOutput);
}

// Run the MLP with fixed point arithmetic
std::vector<float> NeuroTrigger3DH::runMLPFixedPrecision(const std::vector<float>& input) const
{
  const CDCTrigger3DHMLP& network = m_MLP;
  const int fractionalInputBits = m_precisionInputs - 1;

  // Transform the input floats (range [-2, 2]) to m_precisionInputs bit integers
  std::vector<int32_t> layerInput(input.size());
  std::transform(input.begin(), input.end(), layerInput.begin(),
  [](float i) { return static_cast<int32_t>(std::round(i * (1 << fractionalInputBits))); });

  std::vector<int32_t> layerOutput;
  size_t weightIndex = 0;
  const size_t nLayers = network.getNumberOfLayers();
  for (size_t layerIdx = 1; layerIdx < nLayers; ++layerIdx) {
    // Set bias input and scaling shift (different precisions for network inputs and tanh outputs possible)
    unsigned int scalingShift;
    if (layerIdx == 1) {
      layerInput.push_back(1 << fractionalInputBits);
      scalingShift = fractionalInputBits + m_fractionalWeightBits - TanhLUT::LUT_FRAC_BITS;
    } else {
      layerInput.push_back(1 << TanhLUT::LUT_FRAC_BITS);
      scalingShift = TanhLUT::LUT_FRAC_BITS + m_fractionalWeightBits - TanhLUT::LUT_FRAC_BITS;
    }
    const size_t nOutputs = network.getNumberOfNodes(layerIdx);
    const size_t nInputs = layerInput.size();
    layerOutput.assign(nOutputs, 0);
    for (size_t outIdx = 0; outIdx < nOutputs; ++outIdx) {
      int64_t mulAccSum = 0;
      for (size_t inputIdx = 0; inputIdx < nInputs; ++inputIdx) {
        mulAccSum += static_cast<int64_t>(layerInput[inputIdx]) * static_cast<int64_t>(m_intWeights[weightIndex++]);
      }
      int mulAccSumSign = (mulAccSum < 0) ? -1 : 1;

      // Cutting a LUT_FRAC_BITS + MAX_FLOAT_SHIFT + HEADROOM_BITS + 1 window from the DSP
      // This is equivalent to uint32_t lutInput = static_cast<uint32_t>(std::abs(mulAccSum) >> scalingShift);
      const int high = scalingShift + TanhLUT::LUT_FRAC_BITS + TanhLUT::MAX_FLOAT_SHIFT + TanhLUT::HEADROOM_BITS;
      const int low = scalingShift;
      uint32_t lutInput = static_cast<uint32_t>(std::abs(extractBits(mulAccSum, high, low)));

      int32_t outFix;
      if (lutInput >= TanhLUT::LUT_SATURATION) {
        outFix = static_cast<int32_t>(mulAccSumSign * ((1 << TanhLUT::LUT_FRAC_BITS) - 1));
      } else {
        // This is just a scaling shift (equivalent to lutInput >> MAX_FLOAT_SHIFT if LUT_FRAC_BITS == LUT_INDEX_BITS)
        size_t lutIdx = static_cast<size_t>((lutInput * TanhLUT::LUT_SIZE) / TanhLUT::LUT_SATURATION);
        int32_t lutVal = TanhLUT::get(lutIdx);
        outFix = static_cast<int32_t>(mulAccSumSign * lutVal);
      }
      layerOutput[outIdx] = outFix;
    }
    layerInput = layerOutput;
  }
  // Convert back to float and unscale
  std::vector<float> output(layerOutput.size(), 0.0f);
  std::transform(layerOutput.begin(), layerOutput.end(), output.begin(),
  [](int32_t val) { return val / static_cast<float>(1 << TanhLUT::LUT_FRAC_BITS); });
  return unscaleTarget(output);
}

// Scale target value from [-1, 1] to outputScale
std::vector<float> NeuroTrigger3DH::unscaleTarget(const std::vector<float>& target) const
{
  std::vector<float> unscaled(target.size());
  for (size_t i = 0; i < target.size(); ++i) {
    float min = m_neuroParameters3DH.outputScale[2 * i];
    float max = m_neuroParameters3DH.outputScale[2 * i + 1];
    if (i == 1) {
      min *= Unit::deg;
      max *= Unit::deg;
    }
    float range = max - min;
    unscaled[i] = (target[i] + 1.0f) * range * 0.5f + min;
  }
  return unscaled;
}

// Scale target value from outputScale to [-1, 1]
std::vector<float> NeuroTrigger3DH::scaleTarget(const std::vector<float>& target) const
{
  std::vector<float> scaled(target.size());
  for (size_t i = 0; i < target.size(); ++i) {
    float min = m_neuroParameters3DH.outputScale[2 * i];
    float max = m_neuroParameters3DH.outputScale[2 * i + 1];
    if (i == 1) {
      min *= Unit::deg;
      max *= Unit::deg;
    }
    float range = max - min;
    scaled[i] = 2.0f * (target[i] - min) / range - 1.0f;
  }
  return scaled;
}
