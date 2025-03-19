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
void NDFinder::init(unsigned short minSuperAxial, unsigned short minSuperStereo, double thresh,
                    unsigned short minTotalWeight, unsigned short minPeakWeight, unsigned short iterations,
                    unsigned short omegaTrim, unsigned short phiTrim, bool storeAdditionalReadout,
                    std::string& axialFile, std::string& stereoFile)
{
  m_params.minSuperAxial = minSuperAxial;
  m_params.minSuperStereo = minSuperStereo;
  m_params.thresh = thresh;
  m_params.axialFile = axialFile;
  m_params.stereoFile = stereoFile;
  m_params.storeAdditionalReadout = storeAdditionalReadout;

  // Initialization of the pointer arrays, fills hit to sector LUT
  initLookUpArrays();
  initHitToSectorMap();

  // Load the axial and stereo track to hit relations from file.
  loadCompressedHitReps(m_params.axialFile, m_compAxialBins, *m_compAxialHitReps);
  loadCompressedHitReps(m_params.stereoFile, m_compStereoBins, *m_compStereoHitReps);

  // Fills the expanded hit representations (from compressed hits to weights)
  fillExpandedHitReps(m_compAxialBins, *m_compAxialHitReps, *m_expAxialHitReps);
  fillExpandedHitReps(m_compStereoBins, *m_compStereoHitReps, *m_expStereoHitReps);

  // Reset the NDFinder data structure to process next event
  reset();

  // Parameters necessary for the clustering algorithm
  m_clustererParams.minTotalWeight = minTotalWeight;
  m_clustererParams.minPeakWeight = minPeakWeight;
  m_clustererParams.iterations = iterations;
  m_clustererParams.omegaTrim = omegaTrim;
  m_clustererParams.phiTrim = phiTrim;
  m_clustererParams.nOmega = m_nOmega;
  m_clustererParams.nPhi = m_nPhi;
  m_clustererParams.nTheta = m_nTheta;
  m_clusterer = Clusterizend(m_clustererParams);
}

// Initialization of the LUT arrays
void NDFinder::initLookUpArrays()
{
  // Shapes of the arrays holding the hit patterns
  std::array<c2index, 2> shapeHitToSectorIDs = {{m_nTS, m_nPrio}};
  std::array<c5index, 5> shapeCompAxialHitReps = {{m_nAxial, m_nPrio, m_nOmega, m_nPhiComp, 1}};
  std::array<c5index, 5> shapeCompStereoHitReps = {{m_nStereo, m_nPrio, m_nOmega, m_nPhiComp, m_nTheta}};
  std::array<c5index, 5> shapeExpAxialHitReps = {{m_nAxial, m_nPrio, m_nOmega, m_nPhiExp, m_nTheta}};
  std::array<c5index, 5> shapeExpStereoHitReps = {{m_nStereo, m_nPrio, m_nOmega, m_nPhiExp, m_nTheta}};
  std::array<c3index, 3> shapeHough = {{m_nOmega, m_nPhi, m_nTheta}};

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
          for (c5index thetaIdx = 0; thetaIdx < compBins.theta; ++thetaIdx) {
            compHitsToWeights[hitID][priorityWire][omegaIdx][phiIdx][thetaIdx] = flatArray[arrayIndex];
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
        for (c5index thetaIdx = 0; thetaIdx < compBins.theta; ++thetaIdx) {
          unsigned short phiStart = compHitsToWeights[hitID][priorityWire][omegaIdx][0][thetaIdx];
          unsigned short nPhiEntries = compHitsToWeights[hitID][priorityWire][omegaIdx][1][thetaIdx];
          for (c5index phiEntry = 0; phiEntry < nPhiEntries; ++phiEntry) {
            unsigned short currentPhi = phiStart + phiEntry; // currentPhi goes now from [0, 131]
            expHitsToWeights[hitID][priorityWire][omegaIdx][currentPhi][thetaIdx] =
              compHitsToWeights[hitID][priorityWire][omegaIdx][phiEntry + 2][thetaIdx];
            if (compBins.theta == 1) { // Axial case: expand the same curve in all theta bins
              for (c5index axialThetaIdx = 1; axialThetaIdx < m_nTheta; ++axialThetaIdx) {
                expHitsToWeights[hitID][priorityWire][omegaIdx][currentPhi][axialThetaIdx] =
                  compHitsToWeights[hitID][priorityWire][omegaIdx][phiEntry + 2][thetaIdx];
              }
            }
          }
        }
      }
    }
  }
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
  unsigned short phiSectorStart = computePhiSectorStart(relativeSectorID);
  unsigned short priorityWire = m_priorityWirePos[hitIdx];

  HitInfo hitInfo = {relativeWireID, phiSectorStart, priorityWire};
  if (orientation == 1) {
    writeHitToHoughSpace(hitInfo, *m_expAxialHitReps);
  } else {
    writeHitToHoughSpace(hitInfo, *m_expStereoHitReps);
  }
}

// Computes the phi bin where the corresponding sector starts given the relative SectorID and saves the result
unsigned short NDFinder::computePhiSectorStart(unsigned short relativeSectorID)
{
  // 11/32 phi Sectors (size of the expanded hit representations): Center = 5
  unsigned short phiSectorStart = ((relativeSectorID - 5) % m_phiGeo + m_phiGeo) % m_phiGeo * m_nPhiSector;
  m_phiSectorStarts.push_back(phiSectorStart);
  return phiSectorStart;
}

// Write (add) a single hit (Hough curve) to the Hough space
void NDFinder::writeHitToHoughSpace(const HitInfo& hitInfo, const c5array& expHitsToWeights)
{
  c3array& houghSpace = *m_houghSpace;
  for (unsigned short thetaIdx = 0; thetaIdx < m_nTheta; ++thetaIdx) {
    for (unsigned short omegaIdx = 0; omegaIdx < m_nOmega; ++omegaIdx) {
      for (unsigned short phiIdx = 0; phiIdx < m_nPhiExp; ++phiIdx) {
        unsigned short houghPhiIdx = (phiIdx + hitInfo.phiSectorStart) % m_nPhi;
        houghSpace[omegaIdx][houghPhiIdx][thetaIdx] +=
          expHitsToWeights[hitInfo.relativeWireID][hitInfo.priorityWire][omegaIdx][phiIdx][thetaIdx];
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
    cell_index maximumIdx = getMaximumInCluster(cluster);
    unsigned short maximum = houghSpace[maximumIdx[0]][maximumIdx[1]][maximumIdx[2]];
    std::vector<CellWeight> validCells = getCenterOfGravityCells(cluster, maximum);
    std::array<double, 3> centerOfGravity = calculateCenterOfGravity(validCells);
    std::array<double, 3> estimatedParameters = getTrackParameterEstimate(centerOfGravity);

    // Readouts for the 3DFinderInfo class for analysis (Hough space + cluster info)
    std::vector<ROOT::Math::XYZVector> readoutHoughSpace;
    std::vector<ROOT::Math::XYZVector> readoutCluster;

    if (m_params.storeAdditionalReadout) {
      // Readout of the complete Hough space
      for (c3index omegaIdx = 0; omegaIdx < m_nOmega; ++omegaIdx) {
        for (c3index phiIdx = 0; phiIdx < m_nPhi; ++phiIdx) {
          for (c3index thetaIdx = 0; thetaIdx < m_nTheta; ++thetaIdx) {
            unsigned short element = houghSpace[omegaIdx][phiIdx][thetaIdx];
            readoutHoughSpace.push_back(ROOT::Math::XYZVector(element, 0, 0));
          }
        }
      }
      // Readout of the maximum cluster weight
      readoutCluster.push_back(ROOT::Math::XYZVector(maximum, 0, 0));
      // Readout of the total cluster weight
      unsigned int totalWeight = 0;
      for (const cell_index& cellIndex : cluster.getCells()) {
        totalWeight += houghSpace[cellIndex[0]][cellIndex[1]][cellIndex[2]];
      }
      readoutCluster.push_back(ROOT::Math::XYZVector(totalWeight, 0, 0));
      // Readout of the number of cluster cells
      unsigned short nCells = cluster.getCells().size();
      readoutCluster.push_back(ROOT::Math::XYZVector(nCells, 0, 0));
      // Readout of the cluster center of gravity
      readoutCluster.push_back(ROOT::Math::XYZVector(centerOfGravity[0], centerOfGravity[1], centerOfGravity[2]));
      // Readout of the cluster weights
      for (const cell_index& cellIndex : cluster.getCells()) {
        unsigned short element = houghSpace[cellIndex[0]][cellIndex[1]][cellIndex[2]];
        readoutCluster.push_back(ROOT::Math::XYZVector(element, 0, 0));
      }
      // Readout of the cluster cell indices
      for (const cell_index& cellIndex : cluster.getCells()) {
        readoutCluster.push_back(ROOT::Math::XYZVector(cellIndex[0], cellIndex[1], cellIndex[2]));
      }
    }
    m_NDFinderTracks.push_back(NDFinderTrack(estimatedParameters,
                                             std::move(cluster), std::move(readoutHoughSpace), std::move(readoutCluster)));
  }
}

// Relate the hits in the maximum of the cluster to the cluster. Applies a cut on the clusters.
std::vector<SimpleCluster> NDFinder::relateHitsToClusters(std::vector<SimpleCluster>& clusters)
{
  std::vector<std::vector<unsigned short>> hitsVsClusters = getHitsVsClustersTable(clusters);
  std::vector<SimpleCluster> goodClusters;
  if (hitsVsClusters.empty()) return goodClusters;

  for (unsigned short clusterIdx = 0; clusterIdx < hitsVsClusters.size(); ++clusterIdx) {
    std::vector<ContributionInfo> contributionInfos = extractContributionInfos(hitsVsClusters[clusterIdx]);
    for (unsigned short superLayer = 0; superLayer < 9; ++superLayer) {
      long maximumHit = getMaximumHitInSuperLayer(contributionInfos, superLayer);
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
    cell_index maximumIdx = getMaximumInCluster(cluster);
    for (unsigned short hitIdx = 0; hitIdx < m_hitIDs.size(); ++hitIdx) {
      unsigned short contribution = getHitContribution(maximumIdx, hitIdx);
      hitsVsClusters[clusterIdx][hitIdx] = contribution;
    }
  }
  return hitsVsClusters;
}

// Returns the cell index of the maximum in the cluster
cell_index NDFinder::getMaximumInCluster(const SimpleCluster& cluster)
{
  unsigned short currentWeight = 0;
  cell_index currentMaximumIndex = {0, 0, 0};
  const c3array& houghSpace = *m_houghSpace;
  std::vector<cell_index> clusterCells = cluster.getCells();
  for (const cell_index& clusterCell : clusterCells) {
    if (houghSpace[clusterCell[0]][clusterCell[1]][clusterCell[2]] > currentWeight) {
      currentWeight = houghSpace[clusterCell[0]][clusterCell[1]][clusterCell[2]];
      currentMaximumIndex = clusterCell;
    }
  }
  return currentMaximumIndex;
}

// Returns the hit contribution of a TS at a certain maximum cell
unsigned short NDFinder::getHitContribution(const cell_index& maximumCell, const unsigned short hitIdx)
{
  unsigned short contribution = 0;
  unsigned short omegaIdx = maximumCell[0];
  unsigned short houghPhiIdx = maximumCell[1];
  unsigned short thetaIdx = maximumCell[2];
  unsigned short phiSectorStart = m_phiSectorStarts[hitIdx];

  // Inverse Hough transformation (inverse of writeHitToHoughSpace method)
  unsigned short phiIdx = (houghPhiIdx - phiSectorStart + m_nPhi) % m_nPhi;

  const c2array& hitToSectorIDs = *m_hitToSectorIDs;
  unsigned short orientation = hitToSectorIDs[m_hitIDs[hitIdx]][0];
  unsigned short relativeWireID = hitToSectorIDs[m_hitIDs[hitIdx]][1];
  unsigned short priorityWire = m_priorityWirePos[hitIdx];

  if (phiIdx < m_nPhiExp) { // Get the contribution if the hit covers the current phi-area
    if (orientation == 1) { // Axial TS
      contribution = (*m_expAxialHitReps)[relativeWireID][priorityWire][omegaIdx][phiIdx][thetaIdx];
    } else { // Stereo TS
      contribution = (*m_expStereoHitReps)[relativeWireID][priorityWire][omegaIdx][phiIdx][thetaIdx];
    }
  }
  return contribution;
}

// Extract relevant hit information (hitIdx, contribution, super layer, drift time)
std::vector<ContributionInfo> NDFinder::extractContributionInfos(const std::vector<unsigned short>& clusterHits)
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
long NDFinder::getMaximumHitInSuperLayer(const std::vector<ContributionInfo>& contributionInfos, unsigned short superLayer)
{
  std::vector<std::vector<long>> contributionsInSL;
  for (const ContributionInfo& contributionInfo : contributionInfos) {
    if (contributionInfo.superLayer == superLayer) {
      unsigned short hitIdx = contributionInfo.hitIndex;
      unsigned short contribution = contributionInfo.contribution;
      long priorityTime = contributionInfo.priorityTime;
      contributionsInSL.push_back({hitIdx, contribution, priorityTime});
    }
  }
  if (contributionsInSL.empty()) return -1;
  // Sort by drift time
  std::sort(contributionsInSL.begin(), contributionsInSL.end(),
  [](const std::vector<long>& a, const std::vector<long>& b) { return a[2] < b[2]; });
  // Find max contribution
  long maximumHit = contributionsInSL[0][0];
  long maximumContribution = contributionsInSL[0][1];
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
  bool isValid = axialNumber >= m_params.minSuperAxial && stereoNumber >= m_params.minSuperStereo;
  return isValid;
}

// Select the cells which are used in the center of gravity calculation
std::vector<CellWeight> NDFinder::getCenterOfGravityCells(const SimpleCluster& cluster, unsigned short maximum)
{
  std::vector<CellWeight> validCells;
  const c3array& houghSpace = *m_houghSpace;
  std::vector<cell_index> clusterCells = cluster.getCells();
  for (const cell_index& clusterCell : clusterCells) {
    unsigned short cellWeight = houghSpace[clusterCell[0]][clusterCell[1]][clusterCell[2]];
    if (cellWeight > m_params.thresh * maximum) {
      CellWeight validCell = {clusterCell, cellWeight};
      validCells.push_back(validCell);
    }
  }
  return validCells;
}

// Calculate the center of gravity for the track parameters
std::array<double, 3> NDFinder::calculateCenterOfGravity(const std::vector<CellWeight>& validCells)
{
  double weightedSumOmega = 0.;
  double weightedSumPhi = 0.;
  double weightedSumTheta = 0.;
  unsigned int weightSum = 0;
  for (const CellWeight& cell : validCells) {
    weightedSumOmega += cell.index[0] * cell.weight;
    weightedSumPhi += cell.index[1] * cell.weight;
    weightedSumTheta += cell.index[2] * cell.weight;
    weightSum += cell.weight;
  }
  weightedSumOmega /= weightSum;
  weightedSumPhi /= weightSum;
  weightedSumTheta /= weightSum;
  std::array<double, 3> centerOfGravity = {weightedSumOmega, weightedSumPhi, weightedSumTheta};
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
  } else {
    transformed[0] = -1 / getTrackRadius(1. / estimatedParameters[0]);
  }
  // Phi
  if (estimatedParameters[1] > 180) {
    transformed[1] = (estimatedParameters[1] - 360) * TMath::DegToRad();
  } else {
    transformed[1] = (estimatedParameters[1]) * TMath::DegToRad();
  }
  // Theta
  double theta = estimatedParameters[2] * TMath::DegToRad();
  transformed[2] = cos(theta) / sin(theta);
  return transformed;
}
