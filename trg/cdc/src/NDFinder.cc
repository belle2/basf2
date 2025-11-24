/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/cdc/NDFinder.h"

#include <string>
#include <vector>
#include <array>
#include <utility>
#include <string>
#include <fstream>
#include <set>
#include <cmath>
#include <algorithm>
#include <cstdint>

#include <TMath.h>

#include "framework/logging/Logger.h"

using namespace Belle2;

// Run the necessary initialization of the NDFinder
void NDFinder::init(const NDFinderParameters& ndFinderParameters)
{
  m_ndFinderParams = ndFinderParameters;

  // Initialization of the Hough space
  const std::array<c3index, 3> shapeHough = {{m_nOmega, m_nPhi, m_nCot}};
  m_houghSpace = new c3array(shapeHough);

  // Fill hit to sector LUT
  initHitToSectorMap();

  // Load the axial and stereo track to hit relations from file.
  bool useDB = m_ndFinderParams.axialFile.empty() && m_ndFinderParams.stereoFile.empty();
  std::vector<unsigned short> axialArray;
  std::vector<unsigned short> stereoArray;
  if (useDB) {
    if (!m_LUTsConditionsDB) {
      B2FATAL("CDCTriggerNDFinderLUTs payload not available and no LUT files provided");
    }
    const CDCTriggerNDFinderLUTs& luts = *m_LUTsConditionsDB;
    axialArray = luts.axial;
    stereoArray = luts.stereo;
  } else {
    axialArray = loadFromFile(m_ndFinderParams.axialFile);
    stereoArray = loadFromFile(m_ndFinderParams.stereoFile);
  }
  fillCompressedHitReps(axialArray, m_compAxialBins, m_compAxialHitReps);
  fillCompressedHitReps(stereoArray, m_compStereoBins, m_compStereoHitReps);

  // Fills the expanded hit representations (from compressed hits to weights)
  fillExpandedHitReps(m_compAxialBins, m_compAxialHitReps, m_expAxialHitReps);
  fillExpandedHitReps(m_compStereoBins, m_compStereoHitReps, m_expStereoHitReps);

  // Reset the NDFinder data structure to process next event
  reset();

  // Parameters necessary for the peak finding algorithm
  PeakFindingParameters peakFindingParams = {
    ndFinderParameters.iterations,
    ndFinderParameters.omegaTrim,
    ndFinderParameters.phiTrim,
    m_nOmega,
    m_nPhi,
    m_nCot
  };
  m_peakFinder = NDFinderPeakFinder(peakFindingParams);
}

// Add the hit info of a single track segment to the NDFinder
void NDFinder::addHit(const HitInfo& hitInfo)
{
  m_hitIDs.push_back(hitInfo.hitID);
  m_hitSLIDs.push_back(hitInfo.hitSLID);
  m_priorityWireTime.push_back(hitInfo.hitPrioTime);
  ++m_nHits;
}

// Fills the m_hitToSectorIDs array
void NDFinder::initHitToSectorMap()
{
  // Number of first priority wires in each super layer (TS per SL)
  constexpr std::array<unsigned short, 9> nWires = {160, 160, 192, 224, 256, 288, 320, 352, 384};
  // Number of priority wires (= number of TS) per SL in a single (1/32) phi sector
  std::vector<unsigned short> wiresPerSector;
  // Lookup table: Maps the TS id to the SL number
  std::vector<unsigned short> hitToSuperLayer;
  // Integrated number of priority wires for each SL
  std::vector<unsigned short> cumulativeWires = {0};
  // Integrated number of sector priority wires for each SL (Axial even, Stereo odd)
  std::vector<unsigned short> cumulativeSectorWires = {0, 0};
  for (unsigned short superLayer = 0; superLayer < m_nSL; ++superLayer) {
    wiresPerSector.push_back(nWires[superLayer] / m_phiGeo);
    for (unsigned short _ = 0; _ < nWires[superLayer]; ++_) {
      hitToSuperLayer.push_back(superLayer);
    }
    cumulativeWires.push_back(cumulativeWires[superLayer] + nWires[superLayer]);
    cumulativeSectorWires.push_back(cumulativeSectorWires[superLayer] + nWires[superLayer] / m_phiGeo);
  }
  for (unsigned short hit = 0; hit < m_nTS; ++hit) {
    unsigned short superLayer = hitToSuperLayer[hit];
    bool isAxial = (superLayer % 2 == 0);
    unsigned short wireIDinSL = hit - cumulativeWires[superLayer];
    unsigned short wireIDinSector = wireIDinSL % wiresPerSector[superLayer];
    unsigned short relativeWireIDinSector = cumulativeSectorWires[superLayer] + wireIDinSector;
    unsigned short relativeSectorIDinSuperLayer = static_cast<unsigned short>(floor(wireIDinSL / wiresPerSector[superLayer]));
    m_hitToSectorIDs[hit][0] = static_cast<unsigned short>(isAxial);
    m_hitToSectorIDs[hit][1] = relativeWireIDinSector;
    m_hitToSectorIDs[hit][2] = relativeSectorIDinSuperLayer;
  }
}

// Fills the m_compAxialHitReps/m_compStereoHitReps (see below) arrays with the hit representations (hits to weights)
void NDFinder::fillCompressedHitReps(const std::vector<unsigned short>& flatArray, const SectorBinning& compBins,
                                     c4array& compHitsToWeights) const
{
  size_t arrayIndex = 0;
  for (c4index hitID = 0; hitID < compBins.nHitIDs; ++hitID) {
    for (c4index omegaIdx = 0; omegaIdx < compBins.omega; ++omegaIdx) {
      for (c4index phiIdx = 0; phiIdx < compBins.phi; ++phiIdx) {
        for (c4index cotIdx = 0; cotIdx < compBins.cot; ++cotIdx) {
          compHitsToWeights[hitID][omegaIdx][phiIdx][cotIdx] =
            flatArray[arrayIndex++];
        }
      }
    }
  }
}

// Loads the hit representations from a plain text file if the ConditionsDB is not used
std::vector<unsigned short> NDFinder::loadFromFile(const std::string& fileName) const
{
  std::vector<unsigned short> flatArray;
  std::ifstream arrayFile(fileName);
  if (!arrayFile) {
    B2ERROR("Could not open file: " << fileName);
    return flatArray;
  }
  unsigned short value;
  while (arrayFile >> value) {
    flatArray.push_back(value);
  }
  return flatArray;
}

// Fills the m_expAxialHitReps/m_expStereoHitReps arrays
void NDFinder::fillExpandedHitReps(const SectorBinning& compBins, const c4array& compHitsToWeights, c4array& expHitsToWeights) const
{
  for (c4index hitID = 0; hitID < compBins.nHitIDs; ++hitID) {
    for (c4index omegaIdx = 0; omegaIdx < compBins.omega; ++omegaIdx) {
      for (c4index cotIdx = 0; cotIdx < compBins.cot; ++cotIdx) {
        unsigned short phiStart = compHitsToWeights[hitID][omegaIdx][0][cotIdx];
        unsigned short nPhiEntries = compHitsToWeights[hitID][omegaIdx][1][cotIdx];
        for (c4index phiEntry = 0; phiEntry < nPhiEntries; ++phiEntry) {
          unsigned short houghPhiIdx = (phiStart + phiEntry) % m_nPhi; // houghPhiIdx goes now over the complete Hough space
          expHitsToWeights[hitID][omegaIdx][houghPhiIdx][cotIdx] =
            compHitsToWeights[hitID][omegaIdx][phiEntry + 2][cotIdx];
          if (compBins.cot == 1) { // Axial case: expand the same curve in all cot bins
            for (c4index axialCotIdx = 1; axialCotIdx < m_nCot; ++axialCotIdx) {
              expHitsToWeights[hitID][omegaIdx][houghPhiIdx][axialCotIdx] =
                compHitsToWeights[hitID][omegaIdx][phiEntry + 2][cotIdx];
            }
          }
        }
      }
    }
  }
}

// Reset the NDFinder data structure to process next event
void NDFinder::reset()
{
  // Clear the vector of the found tracks
  m_rawFinderTracks.clear();

  // Clear the hit informations
  m_hitIDs.clear();
  m_hitSLIDs.clear();
  m_nHits = 0;
  m_priorityWireTime.clear();

  // Create a new Hough space
  delete m_houghSpace;
  std::array<c3index, 3> shapeHough = {{m_nOmega, m_nPhi, m_nCot}};
  m_houghSpace = new c3array(shapeHough);
}

// Public: Adds the hits to the Hough space and finds the tracks
void NDFinder::findTracks()
{
  // Build the Hough plane by summing up all single hit contributions
  for (unsigned short hitIdx = 0; hitIdx < m_nHits; ++hitIdx) {
    processHitForHoughSpace(hitIdx);
  }
  runTrackFinding();
}

// Process a single axial or stereo hit
void NDFinder::processHitForHoughSpace(const unsigned short hitIdx)
{
  unsigned short orientation = m_hitToSectorIDs[m_hitIDs[hitIdx]][0];
  unsigned short relativeWireID = m_hitToSectorIDs[m_hitIDs[hitIdx]][1];
  unsigned short relativeSectorID = m_hitToSectorIDs[m_hitIDs[hitIdx]][2];
  unsigned short phiSectorStart = relativeSectorID * m_nPhiSector;

  if (orientation == 1) {
    writeHitToHoughSpace(relativeWireID, phiSectorStart, m_expAxialHitReps);
  } else {
    writeHitToHoughSpace(relativeWireID, phiSectorStart, m_expStereoHitReps);
  }
}

// Write (add) a single hit (Hough curve) to the Hough space
void NDFinder::writeHitToHoughSpace(const unsigned short relativeWireID, const unsigned short phiSectorStart,
                                    const c4array& expHitsToWeights)
{
  c3array& houghSpace = *m_houghSpace;
  for (unsigned short cotIdx = 0; cotIdx < m_nCot; ++cotIdx) {
    for (unsigned short omegaIdx = 0; omegaIdx < m_nOmega; ++omegaIdx) {
      for (unsigned short phiIdx = 0; phiIdx < m_nPhi; ++phiIdx) {
        unsigned short houghPhiIdx = (phiIdx + phiSectorStart) % m_nPhi;
        houghSpace[omegaIdx][houghPhiIdx][cotIdx] +=
          expHitsToWeights[relativeWireID][omegaIdx][phiIdx][cotIdx];
      }
    }
  }
}

// Core track finding logic in the constructed Hough space
void NDFinder::runTrackFinding()
{
  c3array& houghSpace = *m_houghSpace;
  m_peakFinder.setNewPlane(houghSpace);
  std::vector<HoughPeak> allPeaks = m_peakFinder.findPeaks();
  std::vector<HoughPeak> validPeaks = relateHitsToPeaks(allPeaks);

  for (HoughPeak& peak : validPeaks) {
    cell_index peakCell = peak.cell;
    std::array<double, 3> estimatedParameters = getTrackParameterEstimate(peakCell);

    // Readout of the complete Hough space
    std::vector<uint8_t> readoutHoughSpace;
    if (m_ndFinderParams.storeHoughSpace) {
      readoutHoughSpace.reserve(m_nOmega * m_nPhi * m_nCot);
      readoutHoughSpace.assign(houghSpace.data(), houghSpace.data() + houghSpace.num_elements());
    }

    m_rawFinderTracks.push_back(RawFinderTrack(estimatedParameters, std::move(peak), std::move(readoutHoughSpace)));
  }
}

// Relate the hits in the peak cell of the peak. Applies a cut on the found peaks.
std::vector<HoughPeak> NDFinder::relateHitsToPeaks(std::vector<HoughPeak>& peaks) const
{
  std::vector<std::vector<unsigned short>> hitsVsPeaks = getHitsVsPeaksTable(peaks);
  std::vector<HoughPeak> goodPeaks;
  if (hitsVsPeaks.empty()) return goodPeaks;

  for (unsigned short peakIdx = 0; peakIdx < hitsVsPeaks.size(); ++peakIdx) {
    std::vector<ContributionInfo> contributionInfos = extractContributionInfos(hitsVsPeaks[peakIdx]);
    std::vector<unsigned short> peakHits;
    for (unsigned short superLayer = 0; superLayer < 9; ++superLayer) {
      int maximumHit = getMaximumHitInSuperLayer(contributionInfos, superLayer);
      if (maximumHit >= 0) { // there exists a hit
        peakHits.push_back(static_cast<unsigned short>(maximumHit));
      }
    }
    peaks[peakIdx].hits = peakHits;
    if (checkHitSuperLayers(peaks[peakIdx])) {
      goodPeaks.push_back(peaks[peakIdx]);
    }
  }
  return goodPeaks;
}

// Create hits to peaks confusion matrix
std::vector<std::vector<unsigned short>> NDFinder::getHitsVsPeaksTable(const std::vector<HoughPeak>& peaks) const
{
  // Creates a (Number Peaks)x(Number Hits) matrix
  std::vector<unsigned short> hitElem(m_nHits, 0);
  std::vector<std::vector<unsigned short>> hitsVsPeaks(peaks.size(), hitElem);
  // Fill the matrix with all the hit contributions
  for (unsigned short peakIdx = 0; peakIdx < peaks.size(); ++peakIdx) {
    HoughPeak peak = peaks[peakIdx];
    cell_index peakCell = peak.cell;
    for (unsigned short hitIdx = 0; hitIdx < m_hitIDs.size(); ++hitIdx) {
      unsigned short contribution = getHitContribution(peakCell, hitIdx);
      hitsVsPeaks[peakIdx][hitIdx] = contribution;
    }
  }
  return hitsVsPeaks;
}

// Returns the hit contribution of a TS at a certain maximum cell
unsigned short NDFinder::getHitContribution(const cell_index& peakCell, const unsigned short hitIdx) const
{
  unsigned short contribution = 0;
  unsigned short omegaIdx = peakCell[0];
  unsigned short houghPhiIdx = peakCell[1];
  unsigned short cotIdx = peakCell[2];

  unsigned short orientation = m_hitToSectorIDs[m_hitIDs[hitIdx]][0];
  unsigned short relativeWireID = m_hitToSectorIDs[m_hitIDs[hitIdx]][1];
  unsigned short relativeSectorID = m_hitToSectorIDs[m_hitIDs[hitIdx]][2];
  unsigned short phiSectorStart = relativeSectorID * m_nPhiSector;

  // Inverse Hough transformation (inverse of writeHitToHoughSpace method)
  unsigned short phiIdx = (houghPhiIdx - phiSectorStart + m_nPhi) % m_nPhi;

  if (orientation == 1) { // Axial TS
    contribution = m_expAxialHitReps[relativeWireID][omegaIdx][phiIdx][cotIdx];
  } else { // Stereo TS
    contribution = m_expStereoHitReps[relativeWireID][omegaIdx][phiIdx][cotIdx];
  }

  return contribution;
}

// Extract relevant hit information (hitIdx, contribution, super layer, drift time)
std::vector<NDFinder::ContributionInfo> NDFinder::extractContributionInfos(const std::vector<unsigned short>& peakHits) const
{
  std::vector<ContributionInfo> contributionInfos;
  for (unsigned short hitIdx = 0; hitIdx < m_hitIDs.size(); ++hitIdx) {
    unsigned short contribution = peakHits[hitIdx];
    if (contribution > 0) {
      ContributionInfo contributionInfo = {
        hitIdx,
        contribution,
        m_hitSLIDs[hitIdx],
        m_priorityWireTime[hitIdx]
      };
      contributionInfos.push_back(contributionInfo);
    }
  }
  return contributionInfos;
}

// Find the hit with the maximum contribution in a given super layer
int NDFinder::getMaximumHitInSuperLayer(const std::vector<ContributionInfo>& contributionInfos, unsigned short superLayer) const
{
  std::vector<std::vector<int>> contributionsInSL;
  for (const ContributionInfo& contributionInfo : contributionInfos) {
    if (contributionInfo.superLayer == superLayer) {
      unsigned short hitIdx = contributionInfo.hitIndex;
      unsigned short contribution = contributionInfo.contribution;
      short priorityTime = contributionInfo.priorityTime;
      contributionsInSL.push_back({hitIdx, contribution, priorityTime});
    }
  }
  if (contributionsInSL.empty()) return -1;
  // Sort by drift time
  std::sort(contributionsInSL.begin(), contributionsInSL.end(),
  [](const std::vector<int>& a, const std::vector<int>& b) { return a[2] < b[2]; });
  // Find max contribution
  int maximumHit = contributionsInSL[0][0];
  int maximumContribution = contributionsInSL[0][1];
  for (const auto& hit : contributionsInSL) {
    if (hit[1] > maximumContribution) {
      maximumHit = hit[0];
      maximumContribution = hit[1];
    }
  }
  return maximumHit;
}

// Cut on the number of hit axial/stereo super layers
bool NDFinder::checkHitSuperLayers(const HoughPeak& peak) const
{
  std::vector<unsigned short> peakHits = peak.hits;
  std::set<unsigned short> uniqueSLNumbers;
  // Add all hit super layers
  for (unsigned short hitIdx : peakHits) {
    uniqueSLNumbers.insert(m_hitSLIDs[hitIdx]);
  }
  // Calculate the number of hit axial and stereo super layers
  unsigned short nSL = uniqueSLNumbers.size();
  uniqueSLNumbers.insert({0, 2, 4, 6, 8});
  unsigned short withAxialSLs = uniqueSLNumbers.size();
  unsigned short axialNumber = 5 - (withAxialSLs - nSL);
  unsigned short stereoNumber = nSL - axialNumber;
  // Cut away all peaks that do have enough hit super layers
  bool isValid = axialNumber >= m_ndFinderParams.minSuperAxial && stereoNumber >= m_ndFinderParams.minSuperStereo;
  return isValid;
}

// Transform the center of gravity (of cells) into the estimated track parameters
std::array<double, 3> NDFinder::getTrackParameterEstimate(const cell_index& peakCell) const
{
  std::array<double, 3> estimatedParameters;
  for (unsigned short dimension = 0; dimension < 3; ++dimension) {
    double trackParameter = m_acceptanceRanges[dimension][0] + (static_cast<double>(peakCell[dimension]) + 0.5) * m_binSizes[dimension];
    estimatedParameters[dimension] = trackParameter;
  }
  return transformTrackParameters(estimatedParameters);
}

// Transform to physical units
std::array<double, 3> NDFinder::transformTrackParameters(const std::array<double, 3>& estimatedParameters) const
{
  std::array<double, 3> transformed;
  // Omega
  if (estimatedParameters[0] == 0.) {
    transformed[0] = estimatedParameters[0];
  } else { // omega = sign(q)/r, r in cm
    transformed[0] = -1 / getTrackRadius(1. / estimatedParameters[0]);
  }
  // Phi
  if (estimatedParameters[1] > 180) {
    transformed[1] = (estimatedParameters[1] - 360) * TMath::DegToRad();
  } else {
    transformed[1] = (estimatedParameters[1]) * TMath::DegToRad();
  }
  // Cot
  transformed[2] = estimatedParameters[2];
  return transformed;
}
