/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <string>
#include <cmath>
#include <iostream>
#include <set>
#include <fstream>
#include "framework/logging/Logger.h"
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "trg/cdc/NDFinder.h"
#include "TMath.h"

using namespace Belle2;

void NDFinder::init(unsigned char minSuperAxial, unsigned char minSuperStereo, float thresh,
                    unsigned short minTotalWeight, unsigned short minPeakWeight, unsigned char iterations,
                    unsigned char omegaTrim, unsigned char phiTrim, unsigned char thetaTrim,
                    bool storeAdditionalReadout, std::string& axialFile, std::string& stereoFile)
{
  m_params.minSuperAxial = minSuperAxial;
  m_params.minSuperStereo = minSuperStereo;
  m_params.thresh = thresh;
  m_params.axialFile = axialFile;
  m_params.stereoFile = stereoFile;
  m_params.storeAdditionalReadout = storeAdditionalReadout;

  initLookUpArrays();
  initHitToSectorMap(*m_hitToSectorIDs);

  // Load the axial and stereo track to hit relations from file.
  loadCompressedHitReps(m_params.axialFile, m_compAxialBins, *m_compAxialHitReps);
  loadCompressedHitReps(m_params.stereoFile, m_compStereoBins, *m_compStereoHitReps);

  // Fills the the expanded hit representations (from compressed hits to weights)
  fillExpandedHitReps(m_compAxialBins, *m_compAxialHitReps, *m_expAxialHitReps);
  fillExpandedHitReps(m_compStereoBins, *m_compStereoHitReps, *m_expStereoHitReps);

  reset();

  // Parameters necessary for the clustering algorithm
  m_clustererParams.minTotalWeight = minTotalWeight;
  m_clustererParams.minPeakWeight = minPeakWeight;
  m_clustererParams.iterations = iterations;
  m_clustererParams.omegaTrim = omegaTrim;
  m_clustererParams.phiTrim = phiTrim;
  m_clustererParams.thetaTrim = thetaTrim;
  m_clustererParams.nOmega = m_nOmega;
  m_clustererParams.nPhi = m_nPhi;
  m_clustererParams.nTheta = m_nTheta;
  m_clusterer = Clusterizend(m_clustererParams);
}

void NDFinder::initLookUpArrays()
{
  /*
    Create the arrays of the track to hit relations for axial and stereo hits.

    Since the CDC wire pattern is repeated 32 times,
    the hit ids are stored for 1/32 of the CDC only.
    The total number of 2336 TS corresponds to (41 axial + 32 stereo) * 32.

    The number of track bins (full phi) is: (omega, phi, theta) = (40, 384, 9)
   */

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
void NDFinder::initHitToSectorMap(c2array& hitsToSectors)
{
  // Number of first priority wires in each super layer (TS per SL)
  constexpr std::array<int, 9> nWires = {160, 160, 192, 224, 256, 288, 320, 352, 384};
  // Number of priority wires (= number of TS) per SL in a single (1/32) phi sector
  std::vector<int> wiresPerSector;
  // Lookup table: Maps the TS id to the SL number
  std::vector<int> hitToSuperLayer;
  // Integrated number of priority wires for each SL
  std::vector<int> cumulativeWires = {0};
  // Integrated number of sector priority wires for each SL (Axial even, Stereo odd)
  std::vector<int> cumulativeSectorWires = {0, 0};
  for (int sl = 0; sl < m_nSL; ++sl) {
    wiresPerSector.push_back(nWires[sl] / m_phiGeo);
    for (int _ = 0; _ < nWires[sl]; ++_) {
      hitToSuperLayer.push_back(sl);
    }
    cumulativeWires.push_back(cumulativeWires[sl] + nWires[sl]);
    cumulativeSectorWires.push_back(cumulativeSectorWires[sl] + nWires[sl] / m_phiGeo);
  }
  for (int hit = 0; hit < m_nTS; ++hit) {
    int sl = hitToSuperLayer[hit];
    bool isAxial = (sl % 2 == 0);
    int wireIDinSL = hit - cumulativeWires[sl];
    int wireIDinSector = wireIDinSL % wiresPerSector[sl];
    int relativeWireIDinSector = cumulativeSectorWires[sl] + wireIDinSector;
    int relativeSectorIDinSuperLayer = (int) floor(wireIDinSL / wiresPerSector[sl]);
    hitsToSectors[hit][0] = (int)(isAxial);
    hitsToSectors[hit][1] = relativeWireIDinSector;
    hitsToSectors[hit][2] = relativeSectorIDinSuperLayer;
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
            if (compBins.theta == 1) { // axial case: expand the same curve in all theta bins
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


void NDFinder::findTracks()
{
  // Build the Houghplane by summing up all single hit contributions
  for (unsigned short hitIdx = 0; hitIdx < m_nHits; ++hitIdx) {
    addLookup(hitIdx);
  }
  getCM();
}

// Writes the hit to the Hough space and creates the inverse lookup table
void NDFinder::addLookup(unsigned short hitIdx)
{
  const c2array& hitToSectorIDs = *m_hitToSectorIDs;
  unsigned short orient = hitToSectorIDs[m_hitIDs[hitIdx]][0];
  unsigned short relativeWireID = hitToSectorIDs[m_hitIDs[hitIdx]][1];
  unsigned short relativeSectorID = hitToSectorIDs[m_hitIDs[hitIdx]][2];
  unsigned short priorityWire = m_priorityWirePos[hitIdx];

  // 11 of 32 phi parcels: center = 5
  unsigned short phiSectorStart = ((relativeSectorID - 5) % m_phiGeo + m_phiGeo) % m_phiGeo * m_nPhiSector;

  m_phiSectorStarts.push_back(phiSectorStart);

  HitInfo hitInfo = {relativeWireID, priorityWire, phiSectorStart};

  if (orient == 1) {
    writeHitToHoughSpace(hitInfo, *m_expAxialHitReps);
  } else {
    writeHitToHoughSpace(hitInfo, *m_expStereoHitReps);
  }
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


void NDFinder::getCM()
{
  c3array& houghSpace = *m_houghSpace;
  m_clusterer.setNewPlane(houghSpace);
  std::vector<SimpleCluster> allClusters = m_clusterer.makeClusters();
  std::vector<std::vector<unsigned short>> hitsVsClusters = getHitsVsClusters(allClusters);
  // New hit to cluster relation
  std::vector<SimpleCluster> validClusters = allHitsToClusters(hitsVsClusters, allClusters);

  for (SimpleCluster& cluster : validClusters) {
    std::vector<cell_index> entries = cluster.getEntries();
    cell_index maxid = getMax(entries);
    unsigned short maxval = houghSpace[maxid[0]][maxid[1]][maxid[2]];
    float cutoff = m_params.thresh * maxval;
    std::vector<CellWeight> highWeight = getHighWeight(entries, cutoff);
    std::vector<double> result = getWeightedMean(highWeight);
    std::vector<double> estimate = getBinToVal(result);
    std::vector<double> transformed = transform(estimate);

    // Readouts for the 3DFinderInfo class for analysis (Hough space + cluster info)
    std::vector<ROOT::Math::XYZVector> readoutHoughSpace;
    std::vector<ROOT::Math::XYZVector> readoutCluster;

    if (m_params.storeAdditionalReadout) {
      // Readout of the complete Hough space
      for (c3index i = 0; i < static_cast<c3index>(houghSpace.shape()[0]); ++i) {
        for (c3index j = 0; j < static_cast<c3index>(houghSpace.shape()[1]); ++j) {
          for (c3index k = 0; k < static_cast<c3index>(houghSpace.shape()[2]); ++k) {
            unsigned short element = houghSpace[i][j][k];
            readoutHoughSpace.push_back(ROOT::Math::XYZVector(element, 0, 0));
          }
        }
      }
      // Readout of the peak weight
      readoutCluster.push_back(ROOT::Math::XYZVector(maxval, 0, 0));
      // Readout of the total weight
      unsigned long totalWeight = 0;
      for (const cell_index& cellIndex : entries) {
        totalWeight += houghSpace[cellIndex[0]][cellIndex[1]][cellIndex[2]];
      }
      readoutCluster.push_back(ROOT::Math::XYZVector(totalWeight, 0, 0));
      // Readout of the number of cluster cells
      int ncells = entries.size();
      readoutCluster.push_back(ROOT::Math::XYZVector(ncells, 0, 0));
      // Readout of the cluster center of gravity
      readoutCluster.push_back(ROOT::Math::XYZVector(result[0], result[1], result[2]));
      // Readout of the cluster weights
      for (const cell_index& cellIndex : entries) {
        unsigned short element = houghSpace[cellIndex[0]][cellIndex[1]][cellIndex[2]];
        readoutCluster.push_back(ROOT::Math::XYZVector(element, 0, 0));
      }
      // Readout of the cluster cell indices
      for (const cell_index& cellIndex : entries) {
        readoutCluster.push_back(ROOT::Math::XYZVector(cellIndex[0], cellIndex[1], cellIndex[2]));
      }
    }
    m_NDFinderTracks.push_back(NDFinderTrack(transformed, std::move(cluster), std::move(readoutHoughSpace), std::move(readoutCluster)));
  }
}


std::vector<std::vector<unsigned short>> NDFinder::getHitsVsClusters(std::vector<SimpleCluster>& clusters)
{
  unsigned short nClusters = clusters.size();
  unsigned short defaultValue = 0;
  std::vector<unsigned short> hitElem(m_nHits, defaultValue);
  std::vector<std::vector<unsigned short>> hitsVsClusters(nClusters, hitElem);

  for (unsigned long clusterIdx = 0; clusterIdx < clusters.size(); ++clusterIdx) {
    SimpleCluster cluster = clusters[clusterIdx];
    std::vector<cell_index> entries = cluster.getEntries();
    cell_index maxid = getMax(entries);
    for (unsigned long hitIdx = 0; hitIdx < m_hitIDs.size(); ++hitIdx) {
      unsigned short contribution = getHitContribution(maxid, hitIdx);
      hitsVsClusters[clusterIdx][hitIdx] = contribution;
    }
  }
  return hitsVsClusters;
}

// Returns the hit contribution of a TS at a certain peak cell
unsigned short NDFinder::getHitContribution(const cell_index& peakCell, const unsigned short hitIdx)
{
  unsigned short contribution = 0;

  unsigned short omegaIdx = peakCell[0];
  unsigned short houghPhiIdx = peakCell[1];
  unsigned short thetaIdx = peakCell[2];

  unsigned short phiSectorStart = m_phiSectorStarts[hitIdx];

  // Inverse Hough transformation (inverse of writeHitToHoughSpace method)
  unsigned short phiIdx = (houghPhiIdx - phiSectorStart + m_nPhi) % m_nPhi;

  const c2array& hitToSectorIDs = *m_hitToSectorIDs;

  unsigned short orient = hitToSectorIDs[m_hitIDs[hitIdx]][0];
  unsigned short relativeWireID = hitToSectorIDs[m_hitIDs[hitIdx]][1];
  unsigned short priorityWire = m_priorityWirePos[hitIdx];

  if (phiIdx < m_nPhiExp) { // Get the contribution if the hit covers the current phi-area
    if (orient == 1) { // Axial TS
      contribution = (*m_expAxialHitReps)[relativeWireID][priorityWire][omegaIdx][phiIdx][thetaIdx];
    } else { // Stereo TS
      contribution = (*m_expStereoHitReps)[relativeWireID][priorityWire][omegaIdx][phiIdx][thetaIdx];
    }
  }
  return contribution;
}

// New hit-to-cluster relation, replaces relateHitsToClusters
std::vector<SimpleCluster>
NDFinder::allHitsToClusters(const std::vector<std::vector<unsigned short>>& hitsVsClusters, std::vector<SimpleCluster>& clusters)
{
  std::vector<SimpleCluster> useClusters;
  if (hitsVsClusters.size() > 0) {
    // Iteration over the number of clusters
    for (unsigned long iclus = 0; iclus < hitsVsClusters.size(); ++iclus) {
      std::vector<std::vector<long>> superLayerNumbers;
      // Iteration over all track segment hits
      for (unsigned long hitIdx = 0; hitIdx < m_hitIDs.size(); ++hitIdx) {
        unsigned short contribution = hitsVsClusters[iclus][hitIdx];
        if (contribution > 0) {
          superLayerNumbers.push_back({static_cast<long>(hitIdx), contribution, m_hitSLIDs[hitIdx], m_priorityWireTime[hitIdx]});
        }
      }
      // Iteration over all super layers
      for (unsigned short sl = 0; sl < 9; ++sl) {
        std::vector<std::vector<long>> oneSuperLayerContributions;
        for (unsigned long nTS = 0; nTS < superLayerNumbers.size(); ++nTS) {
          if (superLayerNumbers[nTS][2] == sl) {
            oneSuperLayerContributions.push_back({superLayerNumbers[nTS][0], superLayerNumbers[nTS][1], superLayerNumbers[nTS][3]});
          }
        }
        // Continue if there are no hits in the current super layer
        if (oneSuperLayerContributions.size() == 0) {
          continue;
        }
        // Sorting after the drift times
        struct sortingClass {
          bool operator()(std::vector<long> i, std::vector<long> j) {return (i[2] < j[2]);}
        } sortingTimes;
        sort(oneSuperLayerContributions.begin(), oneSuperLayerContributions.end(), sortingTimes);
        long maxHit = oneSuperLayerContributions[0][0];
        long maxContribution = oneSuperLayerContributions[0][1];
        // Iteration over all track segments in this super layer
        for (size_t index = 0; index < oneSuperLayerContributions.size(); ++index) {
          // The maximum weight contribution gets identified
          if (oneSuperLayerContributions[index][1] > maxContribution) {
            maxHit = oneSuperLayerContributions[index][0];
          }
        }
        clusters[iclus].addHitToCluster(maxHit);
      }
      SimpleCluster& clu = clusters[iclus];
      // The hits of the current cluster get extracted
      std::vector<unsigned short> clusterHits = clu.getClusterHits();
      std::vector<unsigned short> clusterSLNumbers;
      for (const auto& element : clusterHits) {
        clusterSLNumbers.push_back(m_hitSLIDs[element]);
      }
      // A cut on the super layer numbers is applied
      std::set<unsigned short> uniqueSLNumbers(clusterSLNumbers.begin(), clusterSLNumbers.end());
      size_t nSL = uniqueSLNumbers.size();
      uniqueSLNumbers.insert({0, 2, 4, 6, 8});
      size_t withAxialSLs = uniqueSLNumbers.size();
      size_t axialNumber = 5 - (withAxialSLs - nSL);
      size_t stereoNumber = nSL - axialNumber;
      if (axialNumber >= m_params.minSuperAxial && stereoNumber >= m_params.minSuperStereo) {
        useClusters.push_back(clusters[iclus]);
      }
    }
  }
  return useClusters;
}


float NDFinder::transformVar(float estVal, int idx)
{
  if (idx == 0) { //omega
    if (estVal == 0.) {
      return estVal;
    } else {
      return - 1 / cdcTrackRadius(1. / estVal);
    }
  } else if (idx == 1) { // phi
    float phiMod = estVal;
    if (estVal > 180) {
      phiMod -= 360.;
    }
    return phiMod * TMath::DegToRad();
  } else { // theta
    float thetRad = estVal * TMath::DegToRad();
    return cos(thetRad) / sin(thetRad);
  }
}


std::vector<double> NDFinder::transform(std::vector<double> estimate)
{
  std::vector<double> transformed;
  for (int idx = 0; idx < 3; ++idx) {
    transformed.push_back(transformVar(estimate[idx], idx));
  }
  return transformed;
}


std::vector<double> NDFinder::getBinToVal(std::vector<double> thisAv)
{
  std::vector<double> estimate;
  for (unsigned short idim = 0; idim < 3; ++idim) {
    double trafd = m_acceptanceRanges[idim][0] + (thisAv[idim] + 0.5) * m_binSizes[idim];
    estimate.push_back(trafd);
  }
  return estimate;
}


std::vector<double> NDFinder::getWeightedMean(std::vector<CellWeight> highWeight)
{
  double axomega = 0.;
  double axphi = 0.;
  double axtheta = 0.;
  long weightSum = 0;
  for (CellWeight& elem : highWeight) {
    axomega += elem.index[0] * elem.weight;
    axphi += elem.index[1] * elem.weight;
    axtheta += elem.index[2] * elem.weight;
    weightSum += elem.weight;
  }
  axomega /= weightSum;
  axphi /= weightSum;
  axtheta /= weightSum;
  std::vector<double> result = {axomega, axphi, axtheta};
  return result;
}


cell_index NDFinder::getMax(const std::vector<cell_index>& entries)
{
  unsigned short curWeight = 0;
  cell_index curMaxIndex = {0, 0, 0};
  const c3array& houghSpace = *m_houghSpace;
  for (const cell_index& entry : entries) {
    if (houghSpace[entry[0]][entry[1]][entry[2]] > curWeight) {
      curWeight = houghSpace[entry[0]][entry[1]][entry[2]];
      curMaxIndex = entry;
    }
  }
  return curMaxIndex;
}


std::vector<CellWeight> NDFinder::getHighWeight(std::vector<cell_index> entries, float cutoff)
{
  std::vector<CellWeight> cellsAndWeight;
  const c3array& houghSpace = *m_houghSpace;
  for (cell_index& entry : entries) {
    unsigned short cellWeight = houghSpace[entry[0]][entry[1]][entry[2]];
    if (cellWeight > cutoff) {
      CellWeight curElem;
      curElem.index = entry;
      curElem.weight = cellWeight;
      cellsAndWeight.push_back(curElem);
    }
  }
  return cellsAndWeight;
}
