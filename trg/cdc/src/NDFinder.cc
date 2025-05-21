/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <string>
#include <vector>
#include <array>
#include <utility>
#include <string>
#include <fstream>
#include <set>
#include <cmath>
#include "framework/logging/Logger.h"
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "TMath.h"
#include "trg/cdc/NDFinder.h"

using namespace Belle2;

// Run the necessary initialization of the NDFinder
void NDFinder::init(const NDFinderParameters& ndFinderParameters)
{
  m_ndFinderParams = ndFinderParameters;

  // Initialization of the pointer arrays, fills hit to sector LUT
  initLookUpArrays();
  initHitToSectorMap();

  // Load the axial and stereo track to hit relations from file.
  loadCompressedHitReps(m_ndFinderParams.axialFile, m_compAxialBins, *m_compAxialHitReps);
  loadCompressedHitReps(m_ndFinderParams.stereoFile, m_compStereoBins, *m_compStereoHitReps);

  // Fills the expanded hit representations (from compressed hits to weights)
  fillExpandedHitReps(m_compAxialBins, *m_compAxialHitReps, *m_expAxialHitReps);
  fillExpandedHitReps(m_compStereoBins, *m_compStereoHitReps, *m_expStereoHitReps);

  // Reset the NDFinder data structure to process next event
  reset();

  // Parameters necessary for the clustering algorithm
  ClustererParameters clustererParams = {
    ndFinderParameters.iterations,
    ndFinderParameters.omegaTrim,
    ndFinderParameters.phiTrim,
    m_nOmega,
    m_nPhi,
    m_nCot
  };
  m_clusterer = Clusterizend(clustererParams);
}

// Add the hit info of a single track segment to the NDFinder
void NDFinder::addHit(const HitInfo& hitInfo)
{
  // Priority position from the track segment finder: 0 = no hit, 3 = 1st priority, 1 = 2nd right, 2 = 2nd left
  if (hitInfo.hitPrioPos > 0) { // Skip "no hit" case
    m_hitIDs.push_back(hitInfo.hitID);
    m_hitSLIDs.push_back(hitInfo.hitSLID);
    m_priorityWirePos.push_back(3 - hitInfo.hitPrioPos); // 0 = 1st priority, 1 = 2nd left, 2 = 2nd right
    m_priorityWireTime.push_back(hitInfo.hitPrioTime);
    ++m_nHits;
  }
}

// Initialization of the LUT arrays
void NDFinder::initLookUpArrays()
{
  // Shapes of the arrays holding the hit patterns
  const std::array<c2index, 2> shapeHitToSectorIDs = {{m_nTS, m_nPrio}};
  const std::array<c5index, 5> shapeCompAxialHitReps = {{m_nAxial, m_nPrio, m_nOmega, m_nPhiComp, 1}};
  const std::array<c5index, 5> shapeCompStereoHitReps = {{m_nStereo, m_nPrio, m_nOmega, m_nPhiComp, m_nCot}};
  const std::array<c5index, 5> shapeExpAxialHitReps = {{m_nAxial, m_nPrio, m_nOmega, m_nPhi, m_nCot}};
  const std::array<c5index, 5> shapeExpStereoHitReps = {{m_nStereo, m_nPrio, m_nOmega, m_nPhi, m_nCot}};
  const std::array<c3index, 3> shapeHough = {{m_nOmega, m_nPhi, m_nCot}};

  m_hitToSectorIDs    = new c2array(shapeHitToSectorIDs);
  m_compAxialHitReps  = new c5array(shapeCompAxialHitReps);
  m_compStereoHitReps = new c5array(shapeCompStereoHitReps);
  m_expAxialHitReps   = new c5array(shapeExpAxialHitReps);
  m_expStereoHitReps  = new c5array(shapeExpStereoHitReps);
  m_houghSpace        = new c3array(shapeHough);
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
    c2array& hitToSectorIDs = *m_hitToSectorIDs;
    hitToSectorIDs[hit][0] = static_cast<unsigned short>(isAxial);
    hitToSectorIDs[hit][1] = relativeWireIDinSector;
    hitToSectorIDs[hit][2] = relativeSectorIDinSuperLayer;
  }
}

// Fills the m_compAxialHitReps/m_compStereoHitReps arrays
void NDFinder::loadCompressedHitReps(const std::string& fileName, const SectorBinning& compBins, c5array& compHitsToWeights)
{
  // Array of the entries in trg/cdc/data/ndFinderArray*Comp.txt.gz
  std::vector<unsigned short> flatArray;
  std::ifstream arrayFileGZ(fileName, std::ios_base::in | std::ios_base::binary);
  if (!arrayFileGZ.is_open()) {
    B2ERROR("Could not open array file: " << fileName);
    return;
  }
  boost::iostreams::filtering_istream arrayStream;
  arrayStream.push(boost::iostreams::gzip_decompressor());
  arrayStream.push(arrayFileGZ);
  unsigned short uline;
  while (arrayStream >> uline) {
    flatArray.push_back(uline);
  }
  arrayFileGZ.close();
  unsigned long arrayIndex = 0;
  for (c5index hitID = 0; hitID < compBins.nHitIDs; ++hitID) {
    for (c5index priorityWire = 0; priorityWire < compBins.nPriorityWires; ++priorityWire) {
      for (c5index omegaIdx = 0; omegaIdx < compBins.omega; ++omegaIdx) {
        for (c5index phiIdx = 0; phiIdx < compBins.phi; ++phiIdx) {
          for (c5index cotIdx = 0; cotIdx < compBins.cot; ++cotIdx) {
            compHitsToWeights[hitID][priorityWire][omegaIdx][phiIdx][cotIdx] = flatArray[arrayIndex];
            ++arrayIndex;
          }
        }
      }
    }
  }
}

// Fills the m_expAxialHitReps/m_expStereoHitReps arrays
void NDFinder::fillExpandedHitReps(const SectorBinning& compBins, const c5array& compHitsToWeights, c5array& expHitsToWeights)
{
  for (c5index hitID = 0; hitID < compBins.nHitIDs; ++hitID) {
    for (c5index priorityWire = 0; priorityWire < compBins.nPriorityWires; ++priorityWire) {
      for (c5index omegaIdx = 0; omegaIdx < compBins.omega; ++omegaIdx) {
        for (c5index cotIdx = 0; cotIdx < compBins.cot; ++cotIdx) {
          unsigned short phiStart = compHitsToWeights[hitID][priorityWire][omegaIdx][0][cotIdx];
          unsigned short nPhiEntries = compHitsToWeights[hitID][priorityWire][omegaIdx][1][cotIdx];
          for (c5index phiEntry = 0; phiEntry < nPhiEntries; ++phiEntry) {
            unsigned short houghPhiIdx = (phiStart + phiEntry) % m_nPhi; // houghPhiIdx goes now over the complete Hough space
            expHitsToWeights[hitID][priorityWire][omegaIdx][houghPhiIdx][cotIdx] =
              compHitsToWeights[hitID][priorityWire][omegaIdx][phiEntry + 2][cotIdx];
            if (compBins.cot == 1) { // Axial case: expand the same curve in all cot bins
              for (c5index axialCotIdx = 1; axialCotIdx < m_nCot; ++axialCotIdx) {
                expHitsToWeights[hitID][priorityWire][omegaIdx][houghPhiIdx][axialCotIdx] =
                  compHitsToWeights[hitID][priorityWire][omegaIdx][phiEntry + 2][cotIdx];
              }
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
  m_ndFinderTracks.clear();

  // Clear the hit informations
  m_hitIDs.clear();
  m_hitSLIDs.clear();
  m_nHits = 0;
  m_priorityWirePos.clear();
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
  const c2array& hitToSectorIDs = *m_hitToSectorIDs;
  unsigned short orientation = hitToSectorIDs[m_hitIDs[hitIdx]][0];
  unsigned short relativeWireID = hitToSectorIDs[m_hitIDs[hitIdx]][1];
  unsigned short relativeSectorID = hitToSectorIDs[m_hitIDs[hitIdx]][2];
  unsigned short phiSectorStart = relativeSectorID * m_nPhiSector;
  unsigned short priorityWire = m_priorityWirePos[hitIdx];

  WireInfo wireInfo = {relativeWireID, phiSectorStart, priorityWire};
  if (orientation == 1) {
    writeHitToHoughSpace(wireInfo, *m_expAxialHitReps);
  } else {
    writeHitToHoughSpace(wireInfo, *m_expStereoHitReps);
  }
}

// Write (add) a single hit (Hough curve) to the Hough space
void NDFinder::writeHitToHoughSpace(const WireInfo& wireInfo, const c5array& expHitsToWeights)
{
  c3array& houghSpace = *m_houghSpace;
  for (unsigned short cotIdx = 0; cotIdx < m_nCot; ++cotIdx) {
    for (unsigned short omegaIdx = 0; omegaIdx < m_nOmega; ++omegaIdx) {
      for (unsigned short phiIdx = 0; phiIdx < m_nPhi; ++phiIdx) {
        unsigned short houghPhiIdx = (phiIdx + wireInfo.phiSectorStart) % m_nPhi;
        houghSpace[omegaIdx][houghPhiIdx][cotIdx] +=
          expHitsToWeights[wireInfo.relativeWireID][wireInfo.priorityWire][omegaIdx][phiIdx][cotIdx];
      }
    }
  }
}

// Core track finding logic in the constructed Hough space
void NDFinder::runTrackFinding()
{
  c3array& houghSpace = *m_houghSpace;
  m_clusterer.setNewPlane(houghSpace);
  std::vector<SimpleCluster> allClusters = m_clusterer.makeClusters();
  std::vector<SimpleCluster> validClusters = relateHitsToClusters(allClusters);

  for (SimpleCluster& cluster : validClusters) {
    std::array<double, 3> centerOfGravity = calculateCenterOfGravity(cluster);
    std::array<double, 3> estimatedParameters = getTrackParameterEstimate(centerOfGravity);

    // Readouts for the 3DFinderInfo class for analysis (Hough space + cluster info)
    std::vector<ROOT::Math::XYZVector> readoutHoughSpace;
    std::vector<ROOT::Math::XYZVector> readoutCluster;

    if (m_ndFinderParams.storeAdditionalReadout) {
      // Readout of the complete Hough space
      for (c3index omegaIdx = 0; omegaIdx < m_nOmega; ++omegaIdx) {
        for (c3index phiIdx = 0; phiIdx < m_nPhi; ++phiIdx) {
          for (c3index cotIdx = 0; cotIdx < m_nCot; ++cotIdx) {
            unsigned short element = houghSpace[omegaIdx][phiIdx][cotIdx];
            readoutHoughSpace.push_back(ROOT::Math::XYZVector(element, 0, 0));
          }
        }
      }
      // Readout of the peak cluster weight
      unsigned int peakWeight = cluster.getPeakWeight();
      readoutCluster.push_back(ROOT::Math::XYZVector(peakWeight, 0, 0));
      // Readout of the number of cluster cells
      unsigned short nCells = cluster.getCells().size();
      readoutCluster.push_back(ROOT::Math::XYZVector(nCells, 0, 0));
      // Readout of the cluster center of gravity
      readoutCluster.push_back(ROOT::Math::XYZVector(centerOfGravity[0], centerOfGravity[1], centerOfGravity[2]));
      // Readout of the cluster weights
      for (const cell_index& cellIdx : cluster.getCells()) {
        unsigned short element = houghSpace[cellIdx[0]][cellIdx[1]][cellIdx[2]];
        readoutCluster.push_back(ROOT::Math::XYZVector(element, 0, 0));
      }
      // Readout of the cluster cell indices
      for (const cell_index& cellIdx : cluster.getCells()) {
        readoutCluster.push_back(ROOT::Math::XYZVector(cellIdx[0], cellIdx[1], cellIdx[2]));
      }
    }
    m_ndFinderTracks.push_back(NDFinderTrack(estimatedParameters,
                                             std::move(cluster), std::move(readoutHoughSpace), std::move(readoutCluster)));
  }
}

// Relate the hits in the peak cell of the cluster to the cluster. Applies a cut on the clusters.
std::vector<SimpleCluster> NDFinder::relateHitsToClusters(std::vector<SimpleCluster>& clusters)
{
  std::vector<std::vector<unsigned short>> hitsVsClusters = getHitsVsClustersTable(clusters);
  std::vector<SimpleCluster> goodClusters;
  if (hitsVsClusters.empty()) return goodClusters;

  for (unsigned short clusterIdx = 0; clusterIdx < hitsVsClusters.size(); ++clusterIdx) {
    std::vector<ContributionInfo> contributionInfos = extractContributionInfos(hitsVsClusters[clusterIdx]);
    for (unsigned short superLayer = 0; superLayer < 9; ++superLayer) {
      int maximumHit = getMaximumHitInSuperLayer(contributionInfos, superLayer);
      if (maximumHit >= 0) { // there exists a hit
        clusters[clusterIdx].addHitToCluster(static_cast<unsigned short>(maximumHit));
      }
    }
    if (checkHitSuperLayers(clusters[clusterIdx])) {
      goodClusters.push_back(clusters[clusterIdx]);
    }
  }
  return goodClusters;
}

// Create hits to clusters confusion matrix
std::vector<std::vector<unsigned short>> NDFinder::getHitsVsClustersTable(const std::vector<SimpleCluster>& clusters)
{
  // Creates a (Number Clusters)x(Number Hits) matrix
  std::vector<unsigned short> hitElem(m_nHits, 0);
  std::vector<std::vector<unsigned short>> hitsVsClusters(clusters.size(), hitElem);
  // Fill the matrix with all the hit contributions
  for (unsigned short clusterIdx = 0; clusterIdx < clusters.size(); ++clusterIdx) {
    SimpleCluster cluster = clusters[clusterIdx];
    cell_index peakCell = cluster.getPeakCell();
    for (unsigned short hitIdx = 0; hitIdx < m_hitIDs.size(); ++hitIdx) {
      unsigned short contribution = getHitContribution(peakCell, hitIdx);
      hitsVsClusters[clusterIdx][hitIdx] = contribution;
    }
  }
  return hitsVsClusters;
}

// Returns the hit contribution of a TS at a certain maximum cell
unsigned short NDFinder::getHitContribution(const cell_index& peakCell, const unsigned short hitIdx)
{
  unsigned short contribution = 0;
  unsigned short omegaIdx = peakCell[0];
  unsigned short houghPhiIdx = peakCell[1];
  unsigned short cotIdx = peakCell[2];

  const c2array& hitToSectorIDs = *m_hitToSectorIDs;
  unsigned short orientation = hitToSectorIDs[m_hitIDs[hitIdx]][0];
  unsigned short relativeWireID = hitToSectorIDs[m_hitIDs[hitIdx]][1];
  unsigned short relativeSectorID = hitToSectorIDs[m_hitIDs[hitIdx]][2];
  unsigned short phiSectorStart = relativeSectorID * m_nPhiSector;
  unsigned short priorityWire = m_priorityWirePos[hitIdx];

  // Inverse Hough transformation (inverse of writeHitToHoughSpace method)
  unsigned short phiIdx = (houghPhiIdx - phiSectorStart + m_nPhi) % m_nPhi;

  if (orientation == 1) { // Axial TS
    contribution = (*m_expAxialHitReps)[relativeWireID][priorityWire][omegaIdx][phiIdx][cotIdx];
  } else { // Stereo TS
    contribution = (*m_expStereoHitReps)[relativeWireID][priorityWire][omegaIdx][phiIdx][cotIdx];
  }

  return contribution;
}

// Extract relevant hit information (hitIdx, contribution, super layer, drift time)
std::vector<NDFinder::ContributionInfo> NDFinder::extractContributionInfos(const std::vector<unsigned short>& clusterHits)
{
  std::vector<ContributionInfo> contributionInfos;
  for (unsigned short hitIdx = 0; hitIdx < m_hitIDs.size(); ++hitIdx) {
    unsigned short contribution = clusterHits[hitIdx];
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
int NDFinder::getMaximumHitInSuperLayer(const std::vector<ContributionInfo>& contributionInfos, unsigned short superLayer)
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
bool NDFinder::checkHitSuperLayers(const SimpleCluster& cluster)
{
  std::vector<unsigned short> clusterHits = cluster.getClusterHits();
  std::set<unsigned short> uniqueSLNumbers;
  // Add all hit super layers
  for (unsigned short hitIdx : clusterHits) {
    uniqueSLNumbers.insert(m_hitSLIDs[hitIdx]);
  }
  // Calculate the number of hit axial and stereo super layers
  unsigned short nSL = uniqueSLNumbers.size();
  uniqueSLNumbers.insert({0, 2, 4, 6, 8});
  unsigned short withAxialSLs = uniqueSLNumbers.size();
  unsigned short axialNumber = 5 - (withAxialSLs - nSL);
  unsigned short stereoNumber = nSL - axialNumber;
  // Cut away all clusters that do have enough hit super layers
  bool isValid = axialNumber >= m_ndFinderParams.minSuperAxial && stereoNumber >= m_ndFinderParams.minSuperStereo;
  return isValid;
}

// Calculate the center of gravity for the track parameters
std::array<double, 3> NDFinder::calculateCenterOfGravity(const SimpleCluster& cluster)
{
  double weightedSumOmega = 0.;
  double weightedSumPhi = 0.;
  double weightedSumCot = 0.;
  unsigned int weightSum = 0;
  for (const cell_index& cellIdx : cluster.getCells()) {
    unsigned short cellWeight = (*m_houghSpace)[cellIdx[0]][cellIdx[1]][cellIdx[2]];
    weightedSumOmega += cellIdx[0] * cellWeight;
    weightedSumPhi += cellIdx[1] * cellWeight;
    weightedSumCot += cellIdx[2] * cellWeight;
    weightSum += cellWeight;
  }
  weightedSumOmega /= weightSum;
  weightedSumPhi /= weightSum;
  weightedSumCot /= weightSum;
  std::array<double, 3> centerOfGravity = {weightedSumOmega, weightedSumPhi, weightedSumCot};
  return centerOfGravity;
}

// Transform the center of gravity (of cells) into the estimated track parameters
std::array<double, 3> NDFinder::getTrackParameterEstimate(const std::array<double, 3>& centerOfGravity)
{
  std::array<double, 3> estimatedParameters;
  for (unsigned short dimension = 0; dimension < 3; ++dimension) {
    double trackParameter = m_acceptanceRanges[dimension][0] + (centerOfGravity[dimension] + 0.5) * m_binSizes[dimension];
    estimatedParameters[dimension] = trackParameter;
  }
  return transformTrackParameters(estimatedParameters);
}

// Transform to physical units
std::array<double, 3> NDFinder::transformTrackParameters(const std::array<double, 3>& estimatedParameters)
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
