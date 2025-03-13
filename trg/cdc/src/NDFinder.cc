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

  initBins();
  initHitToSectorMap(*m_hitToSectorIDs);

  /* Load the axial and stereo track to hit relations from file.*/
  loadHitRepresentations(m_params.axialFile, m_compAxialBins, *m_compAxialHitReps);
  B2DEBUG(25, "loaded zero suppressed axial array ");
  loadHitRepresentations(m_params.stereoFile, m_compStereoBins, *m_compStereoHitReps);
  B2DEBUG(25, "loaded zero suppressed stereo array ");

  /* Unpack zero suppresed track to hit relations.*/
  restoreZeros(m_expAxialBins, m_compAxialBins, *m_parrayAxialExp, *m_compAxialHitReps);
  B2DEBUG(25, "restored expanded axial array (11/32 phi) ");
  restoreZeros(m_expStereoBins, m_compStereoBins, *m_parrayStereoExp, *m_compStereoHitReps);
  B2DEBUG(25, "restored expanded stereo array (11/32 phi) ");
  squeezeAll(m_axialBins, *m_parrayAxial, *m_parrayAxialExp, m_parcels, m_parcelsExp);
  B2DEBUG(25, "squeezed axial array (11/32 phi) --> (7/32 phi): ");
  squeezeAll(m_stereoBins, *m_parrayStereo, *m_parrayStereoExp, m_parcels, m_parcelsExp);
  B2DEBUG(25, "squeezed stereo array (11/32 phi) --> (7/32 phi)");

  reset();

  /* Parameters necessary for the clustering algorithm */
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

void NDFinder::initBins()
{
  /** Create the arrays of the track to hit relations
   * for axial and stereo hits.
   *
   * Since the CDC wire pattern is repeated 32 times,
   * the hit ids are stored for 1/32 of the CDC only.
   * The total number of 2336 TS corresponds to (41 axial + 32 stereo) * 32.
   *
   * The number of track bins (full phi) is: (omega, phi, theta) = (40, 384, 9)
   **/

  m_nTS = 2336;
  m_nSL = 9;
  m_nAxial = 41;
  m_nStereo = 32;
  m_nPrio = 3;

  m_nOmega = 40;
  m_nPhi = 384;
  m_nTheta = 9;

  m_phiGeo = 32;
  m_parcels = 7;
  m_parcelsExp = 11;

  m_nPhiSector = m_nPhi / m_phiGeo; // 12
  m_nPhiComp = 15;
  m_nPhiUse =  m_parcels * m_nPhiSector; // 84
  m_nPhiExp =  m_parcelsExp * m_nPhiSector; // 132

  m_compAxialBins.omega = m_nOmega; // 40
  m_compAxialBins.phi = m_nPhiComp; // 15
  m_compAxialBins.theta = 1;
  m_compAxialBins.hitID = m_nAxial; // 41
  m_compAxialBins.priorityWire = m_nPrio; // 3

  m_compStereoBins.omega = m_nOmega; // 40
  m_compStereoBins.phi = m_nPhiComp; // 15
  m_compStereoBins.theta = m_nTheta; // 9
  m_compStereoBins.hitID = m_nStereo; // 32
  m_compStereoBins.priorityWire = m_nPrio; // 3

  m_axialBins.omega = m_nOmega; // 40
  m_axialBins.phi = m_nPhiUse; // 84
  m_axialBins.theta = m_nTheta; // 9
  m_axialBins.hitID = m_nAxial; // 41
  m_axialBins.priorityWire = m_nPrio; // 3

  m_stereoBins.omega = m_nOmega; // 40
  m_stereoBins.phi = m_nPhiUse; // 84
  m_stereoBins.theta = m_nTheta; // 9
  m_stereoBins.hitID = m_nStereo; // 32
  m_stereoBins.priorityWire = m_nPrio; // 3

  m_expAxialBins.omega = m_nOmega; // 40
  m_expAxialBins.phi = m_nPhiExp; // 132
  m_expAxialBins.theta = m_nTheta; // 9
  m_expAxialBins.hitID = m_nAxial; // 41
  m_expAxialBins.priorityWire = m_nPrio; // 3

  m_expStereoBins.omega = m_nOmega; // 40
  m_expStereoBins.phi = m_nPhiExp; // 132
  m_expStereoBins.theta = m_nTheta; // 9
  m_expStereoBins.hitID = m_nStereo; // 32
  m_expStereoBins.priorityWire = m_nPrio; // 3

  m_fullBins.omega = m_nOmega; // 40
  m_fullBins.phi = m_nPhi; // 384
  m_fullBins.theta = m_nTheta; // 9
  m_fullBins.hitID = m_nTS; // 2336
  m_fullBins.priorityWire = m_nPrio; // 3

  /* Shapes of the arrays holding the hit patterns */
  std::array<c2index, 2> shapeHitToSectorIDs = {{ m_nTS, m_nPrio }};
  std::array<c5index, 5> shapeCompAxial = {{ m_nAxial, m_nPrio, m_nOmega, m_nPhiComp, 1 }};
  std::array<c5index, 5> shapeCompStereo = {{ m_nStereo, m_nPrio, m_nOmega, m_nPhiComp, m_nTheta }};
  std::array<c5index, 5> shapeAxial = {{ m_nAxial, m_nPrio, m_nOmega, m_nPhiUse, m_nTheta }};
  std::array<c5index, 5> shapeStereo = {{ m_nStereo, m_nPrio, m_nOmega, m_nPhiUse, m_nTheta }};
  std::array<c3index, 3> shapeHough = {{ m_nOmega, m_nPhi, m_nTheta }};
  std::array<c5index, 5> shapeExpAxial = {{ m_nAxial, m_nPrio, m_nOmega, m_nPhiExp, m_nTheta }};
  std::array<c5index, 5> shapeExpStereo = {{ m_nStereo, m_nPrio, m_nOmega, m_nPhiExp, m_nTheta }};

  m_hitToSectorIDs          = new c2array(shapeHitToSectorIDs);
  m_compAxialHitReps        = new c5array(shapeCompAxial);
  m_compStereoHitReps       = new c5array(shapeCompStereo);
  m_parrayAxial             = new c5array(shapeAxial);
  m_parrayStereo            = new c5array(shapeStereo);
  m_phoughSpace             = new c3array(shapeHough);
  m_parrayAxialExp          = new c5array(shapeExpAxial);
  m_parrayStereoExp         = new c5array(shapeExpStereo);

  /* Acceptance Ranges (for getBinToVal method) */
  std::vector<float> omegaRange = { -5., 5.};
  std::vector<float> phiRange = {0., 11.25};
  std::vector<float> thetaRange = {19., 140.};
  float ssOmega = (omegaRange[1] - omegaRange[0]) / m_nOmega; // 40;
  float ssPhi = (phiRange[1] - phiRange[0]) / m_nPhiSector; // 12;
  float ssTheta = (thetaRange[1] - thetaRange[0]) / m_nTheta; // 9;
  m_acceptRanges.push_back(omegaRange);
  m_acceptRanges.push_back(phiRange);
  m_acceptRanges.push_back(thetaRange);
  m_slotSizes.push_back(ssOmega);
  m_slotSizes.push_back(ssPhi);
  m_slotSizes.push_back(ssTheta);
}

/* Fills the m_hitToSectorIDs array */
void NDFinder::initHitToSectorMap(c2array& mapArray)
{
  /* Number of first priority wires in each super layer (TS per SL) */
  constexpr std::array<int, 9> nWires = {160, 160, 192, 224, 256, 288, 320, 352, 384};
  /* Number of priority wires (= number of TS) per SL in a single (1/32) phi sector */
  std::vector<int> wiresPerSector;
  /* Lookup table: Maps the TS id to the SL number */
  std::vector<int> hitToSuperLayer;
  /* Integrated number of priority wires for each SL */
  std::vector<int> cumulativeWires = {0};
  /* Integrated number of sector priority wires for each SL (Axial even, Stereo odd) */
  std::vector<int> cumulativeSectorWires = {0, 0};
  for (int sl = 0; sl < m_nSL; sl++) {
    wiresPerSector.push_back(nWires[sl] / m_phiGeo);
    for (int _ = 0; _ < nWires[sl]; _++) {
      hitToSuperLayer.push_back(sl);
    }
    cumulativeWires.push_back(cumulativeWires[sl] + nWires[sl]);
    cumulativeSectorWires.push_back(cumulativeSectorWires[sl] + nWires[sl] / m_phiGeo);
  }
  for (int hit = 0; hit < m_nTS; hit++) {
    int sl = hitToSuperLayer[hit];
    bool isAxial = (sl % 2 == 0);
    int wireIDinSL = hit - cumulativeWires[sl];
    int wireIDinSector = wireIDinSL % wiresPerSector[sl];
    int relativeWireIDinSector = cumulativeSectorWires[sl] + wireIDinSector;
    int relativeSectorIDinSuperLayer = (int) floor(wireIDinSL / wiresPerSector[sl]);
    mapArray[hit][0] = (int)(isAxial);
    mapArray[hit][1] = relativeWireIDinSector;
    mapArray[hit][2] = relativeSectorIDinSuperLayer;
  }
}


void NDFinder::loadHitRepresentations(const std::string& fileName, const SectorBinning& bins, c5array& hitsToWeights)
{
  std::vector<c5elem> flatArray; // Array of the entries in trg/cdc/data/ndFinderArray*Comp.txt.gz
  std::ifstream arrayFileGZ(fileName, std::ios_base::in | std::ios_base::binary);
  if (!arrayFileGZ.is_open()) {
    B2ERROR("could not open array file: " << fileName);
    return;
  }
  boost::iostreams::filtering_istream arrayStream;
  arrayStream.push(boost::iostreams::gzip_decompressor());
  arrayStream.push(arrayFileGZ);
  c5elem uline;
  while (arrayStream >> uline) {
    flatArray.push_back(uline);
  }
  arrayFileGZ.close();
  B2DEBUG(25, "loaded array from file " << fileName);
  unsigned long icount = 0;
  for (c5index ihit = 0; ihit < bins.hitID; ihit++) {
    for (c5index priorityIndex = 0; priorityIndex < bins.priorityWire; priorityIndex++) {
      for (c5index iomega = 0; iomega < bins.omega; iomega++) {
        for (c5index iphi = 0; iphi < bins.phi; iphi++) {
          for (c5index itheta = 0; itheta < bins.theta; itheta++) {
            hitsToWeights[ihit][priorityIndex][iomega][iphi][itheta] = flatArray[icount];
            icount++;
          }
        }
      }
    }
  }
}


void NDFinder::squeezeOne(c5array& writeArray, c5array& readArray, int outparcels, int inparcels, c5index ihit,
                          c5index priorityIndex, c5index itheta, c5elem nomega)
{
  int outnphi = (int)(12 * outparcels);
  c5index trafstart = (c5index)((inparcels - outparcels) / 2 * 12);
  c5index trafend = (c5index)(trafstart + outnphi);
  for (c5index iomega = 0; iomega < nomega; iomega++) {
    for (c5index iphi = 0; iphi < outnphi; iphi++) {
      c5index readPhi = trafstart + iphi;
      writeArray[ihit][priorityIndex][iomega][iphi][itheta] = readArray[ihit][priorityIndex][iomega][readPhi][itheta];
    }
    for (c5index iphi = 0; iphi < trafstart; iphi++) {
      c5index writePhi = (c5index)(outnphi - trafstart + iphi);
      writeArray[ihit][priorityIndex][iomega][writePhi][itheta] += readArray[ihit][priorityIndex][iomega][iphi][itheta];
    }
    for (c5index iphi = 0; iphi < trafstart; iphi++) {
      c5index readPhi = trafend + iphi;
      writeArray[ihit][priorityIndex][iomega][iphi][itheta] += readArray[ihit][priorityIndex][iomega][readPhi][itheta];
    }
  }
}


void NDFinder::squeezeAll(SectorBinning writebins, c5array& writeArray, c5array& readArray, int outparcels, int inparcels)
{
  for (c5index ihit = 0; ihit < writebins.hitID; ihit++) {
    for (c5index priorityIndex = 0; priorityIndex < writebins.priorityWire; priorityIndex++) {
      for (c5index itheta = 0; itheta < writebins.theta; itheta++) {
        squeezeOne(writeArray, readArray, outparcels, inparcels, ihit, priorityIndex, itheta, writebins.omega);
      }
    }
  }
}


void NDFinder::restoreZeros(SectorBinning zerobins, SectorBinning compbins, c5array& expArray, const c5array& compArray)
{
  B2DEBUG(25, "restoreZeros: zerobins.theta " << zerobins.theta << ", combins.theta " << compbins.theta);
  for (c5index ihit = 0; ihit < compbins.hitID; ihit++) {
    for (c5index priorityIndex = 0; priorityIndex < compbins.priorityWire; priorityIndex++) {
      for (c5index iomega = 0; iomega < compbins.omega; iomega++) {
        for (c5index itheta = 0; itheta < compbins.theta; itheta++) {
          c5elem phiStart = compArray[ihit][priorityIndex][iomega][0][itheta];
          c5elem phiWidth = compArray[ihit][priorityIndex][iomega][1][itheta];
          for (c5index iphi = 0; iphi < phiWidth; iphi++) {
            c5elem phiCur = phiStart + iphi;
            expArray[ihit][priorityIndex][iomega][phiCur][itheta] = compArray[ihit][priorityIndex][iomega][iphi + 2][itheta];
            if (compbins.theta == 1) { // case axial, expand in theta
              for (c5index jtheta = 1; jtheta < zerobins.theta; jtheta++) {
                expArray[ihit][priorityIndex][iomega][phiCur][jtheta] = compArray[ihit][priorityIndex][iomega][iphi + 2][itheta];
              }
            }
          }
        }
      }
    }
  }
}

/* Writes the hit to the Hough space and creates the inverse lookup table */
void NDFinder::addLookup(unsigned short ihit)
{
  c2array& arrayHitMod = *m_hitToSectorIDs;

  c2elem orient = arrayHitMod[m_hitIDs[ihit]][0];
  c2elem relativeWireID   = arrayHitMod[m_hitIDs[ihit]][1];
  c2elem relativeSectorID =  arrayHitMod[m_hitIDs[ihit]][2];
  unsigned short priorityWire = m_priorityWirePos[ihit];

  // Get hit contribution to cluster:
  // 7 of 32 phi parcels: center = 3
  short DstartShort = ((short)relativeSectorID - 3) % m_phiGeo * m_nPhiSector;
  if (DstartShort < 0) {DstartShort = m_nPhi + DstartShort;}
  // Add hit to hough plane
  // 11 of 32 phi parcels: center = 5
  short DstartComp = ((short)relativeSectorID - 5) % m_phiGeo * m_nPhiSector;
  if (DstartComp < 0) {DstartComp = m_nPhi + DstartComp;}

  m_vecDstart.push_back(DstartShort);
  m_hitOrients.push_back(orient);

  HitInfo hitInfo = {relativeWireID, priorityWire, DstartComp, (orient == 1) ? m_compAxialBins : m_compStereoBins};

  if (orient == 1) {
    addC3Comp(hitInfo, *m_compAxialHitReps);
  } else {
    addC3Comp(hitInfo, *m_compStereoHitReps);
  }
}


void NDFinder::addC3Comp(const HitInfo& hitInfo, const c5array& hitsToWeights)
{
  unsigned short ntheta = 0;
  c3array& houghSpace = *m_phoughSpace;
  for (unsigned short itheta = 0; itheta < m_nTheta; itheta++) {
    if (hitInfo.bins.theta > 1) { // stereo
      ntheta = itheta;
    }
    for (unsigned short iomega = 0; iomega < m_nOmega; iomega++) {
      unsigned short phiStart = hitsToWeights[hitInfo.relativeWireID][hitInfo.priorityWire][iomega][0][ntheta];
      unsigned short nPhiEntries = hitsToWeights[hitInfo.relativeWireID][hitInfo.priorityWire][iomega][1][ntheta];
      for (unsigned short entry = 0; entry < nPhiEntries; entry++) {
        unsigned short iHoughPhi = (entry + phiStart + hitInfo.Dstart) % m_nPhi;
        houghSpace[iomega][iHoughPhi][itheta] += hitsToWeights[hitInfo.relativeWireID][hitInfo.priorityWire][iomega][entry + 2][ntheta];
      }
    }
  }
}


void NDFinder::findTracks()
{
  /* Build the Houghplane by summing up all single hit contributions */
  for (unsigned short ihit = 0; ihit < m_nHits; ihit++) {
    addLookup(ihit);
  }
  getCM();
}


std::vector<std::vector<unsigned short>> NDFinder::getHitsVsClusters(std::vector<SimpleCluster>& clusters)
{
  unsigned short nClusters = clusters.size();
  unsigned short defaultValue = 0;
  std::vector<unsigned short> hitElem(m_nHits, defaultValue);
  std::vector<std::vector<unsigned short>> hitsVsClusters(nClusters, hitElem);

  for (unsigned long iclus = 0; iclus < clusters.size(); iclus++) {
    SimpleCluster cli = clusters[iclus];
    std::vector<cell_index> entries = cli.getEntries();
    cell_index maxid = getMax(entries);
    for (unsigned long ihit = 0; ihit < m_hitIDs.size(); ihit++) {
      unsigned short contrib = hitContrib(maxid, ihit);
      hitsVsClusters[iclus][ihit] = contrib;
    }
  }
  return hitsVsClusters;
}

// New hit-to-cluster relation, replaces relateHitsToClusters
std::vector<SimpleCluster>
NDFinder::allHitsToClusters(std::vector<std::vector<unsigned short>>& hitsVsClusters, std::vector<SimpleCluster>& clusters)
{
  std::vector<SimpleCluster> useClusters;
  if (hitsVsClusters.size() > 0) {
    // Iteration over the number of clusters
    for (unsigned long iclus = 0; iclus < hitsVsClusters.size(); iclus++) {
      std::vector<std::vector<long>> superLayerNumbers;
      // Iteration over all track segment hits
      for (unsigned long ihit = 0; ihit < m_hitIDs.size(); ihit++) {
        unsigned short contribution = hitsVsClusters[iclus][ihit];
        if (contribution > 0) {
          superLayerNumbers.push_back({static_cast<long>(ihit), contribution, m_hitSLIDs[ihit], m_priorityWireTime[ihit]});
        }
      }
      // Iteration over all super layers
      for (unsigned short sl = 0; sl < 9; sl++) {
        std::vector<std::vector<long>> oneSuperLayerContributions;
        for (unsigned long nTS = 0; nTS < superLayerNumbers.size(); nTS++) {
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
        for (size_t index = 0; index < oneSuperLayerContributions.size(); index++) {
          // The maximum weight contribution gets identified
          if (oneSuperLayerContributions[index][1] > maxContribution) {
            maxContribution = oneSuperLayerContributions[index][1];
            maxHit = oneSuperLayerContributions[index][0];
          }
        }
        clusters[iclus].addHit(maxHit, maxContribution, m_hitOrients[maxHit]);
      }
      SimpleCluster& clu = clusters[iclus];
      // The hits of the current cluster get extracted
      std::vector<unsigned short> clusterHits = clu.getHits();
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


void NDFinder::getCM()
{
  c3array& houghSpace = *m_phoughSpace;
  m_clusterer.setNewPlane(houghSpace);
  std::vector<SimpleCluster> allClusters = m_clusterer.makeClusters();
  std::vector<std::vector<unsigned short>> hitsVsClusters = getHitsVsClusters(allClusters);
  // New hit to cluster relation
  std::vector<SimpleCluster> validClusters = allHitsToClusters(hitsVsClusters, allClusters);

  for (const SimpleCluster& cluster : validClusters) {
    std::vector<cell_index> entries = cluster.getEntries();
    cell_index maxid = getMax(entries);
    unsigned short maxval = houghSpace[maxid[0]][maxid[1]][maxid[2]];
    float cutoff = m_params.thresh * maxval;
    std::vector<CellWeight> highWeight = getHighWeight(entries, cutoff);
    std::vector<double> result = getWeightedMean(highWeight);
    std::vector<double> estimate = getBinToVal(result);
    std::vector<double> transformed = transform(estimate);

    /* Readouts for the 3DFinderInfo class for analysis (Hough space + cluster info) */
    std::vector<ROOT::Math::XYZVector> readoutHoughSpace;
    std::vector<ROOT::Math::XYZVector> readoutCluster;

    if (m_params.storeAdditionalReadout) {
      /* Readout of the complete Hough space */
      for (c3index i = 0; i < static_cast<c3index>(houghSpace.shape()[0]); ++i) {
        for (c3index j = 0; j < static_cast<c3index>(houghSpace.shape()[1]); ++j) {
          for (c3index k = 0; k < static_cast<c3index>(houghSpace.shape()[2]); ++k) {
            c3elem element = houghSpace[i][j][k];
            readoutHoughSpace.push_back(ROOT::Math::XYZVector(element, 0, 0));
          }
        }
      }
      /* Readout of the peak weight */
      readoutCluster.push_back(ROOT::Math::XYZVector(maxval, 0, 0));
      /* Readout of the total weight */
      unsigned long totalWeight = 0;
      for (const cell_index& cellIndex : entries) {
        totalWeight += houghSpace[cellIndex[0]][cellIndex[1]][cellIndex[2]];
      }
      readoutCluster.push_back(ROOT::Math::XYZVector(totalWeight, 0, 0));
      /* Readout of the number of cluster cells */
      int ncells = entries.size();
      readoutCluster.push_back(ROOT::Math::XYZVector(ncells, 0, 0));
      /* Readout of the cluster center of gravity */
      readoutCluster.push_back(ROOT::Math::XYZVector(result[0], result[1], result[2]));
      /* Readout of the cluster weights */
      for (const cell_index& cellIndex : entries) {
        c3elem element = houghSpace[cellIndex[0]][cellIndex[1]][cellIndex[2]];
        readoutCluster.push_back(ROOT::Math::XYZVector(element, 0, 0));
      }
      /* Readout of the cluster cell indices */
      for (const cell_index& cellIndex : entries) {
        readoutCluster.push_back(ROOT::Math::XYZVector(cellIndex[0], cellIndex[1], cellIndex[2]));
      }
    }
    m_NDFinderTracks.push_back(NDFinderTrack(transformed, cluster, readoutHoughSpace, readoutCluster));
  }
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
  for (int idx = 0; idx < 3; idx++) {
    transformed.push_back(transformVar(estimate[idx], idx));
  }
  return transformed;
}


std::vector<double> NDFinder::getBinToVal(std::vector<double> thisAv)
{
  std::vector<double> estimate;
  for (unsigned short idim = 0; idim < 3; idim++) {
    double trafd = m_acceptRanges[idim][0] + (thisAv[idim] + 0.5) * m_slotSizes[idim];
    estimate.push_back(trafd);
  }
  return estimate;
}


std::vector<double> NDFinder::getWeightedMean(std::vector<CellWeight> highWeight)
{
  double axomega = 0.;
  double axphi = 0.;
  double axtheta = 0.;
  int ncells = 0;
  long weightSum = 0;
  for (CellWeight& elem : highWeight) {
    axomega += elem.index[0] * elem.weight;
    axphi += elem.index[1] * elem.weight;
    axtheta += elem.index[2] * elem.weight;
    ncells++;
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
  const c3array& houghSpace = *m_phoughSpace;

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
  const c3array& houghSpace = *m_phoughSpace;
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


unsigned short NDFinder::hitContrib(cell_index peak, unsigned short ihit)
{
  unsigned short contrib = 0;
  unsigned short iHoughPhi = peak[1];
  short Dstart = m_vecDstart[ihit];
  short iphi_signed = iHoughPhi - Dstart;
  if (iphi_signed < 0) {
    iphi_signed += m_nPhi;  // Wrap phi properly
  }
  unsigned short iphi = (unsigned short)iphi_signed;
  if (Dstart > iHoughPhi && Dstart > 300) {
    iphi = m_nPhi - Dstart + iHoughPhi;
  }
  unsigned short iomega = peak[0];
  unsigned short itheta = peak[2];
  unsigned short orient = m_hitOrients[ihit];

  const c2array& arrayHitMod = *m_hitToSectorIDs;
  c2elem relativeWireID = arrayHitMod[m_hitIDs[ihit]][1];
  unsigned short priorityWire = m_priorityWirePos[ihit];
  if (Dstart > m_nPhi) {
    B2ERROR("phi overflow: iHoughPhi = " << iHoughPhi << ", Dstart = " << Dstart << ", iphi=" << iphi);
  }
  if (iphi < m_nPhiUse) { // hit covers current phi area, get contribution
    if (orient == 1) { // axial
      contrib = (*m_parrayAxial)[relativeWireID][priorityWire][iomega][iphi][itheta];
    } else { // stereo
      contrib = (*m_parrayStereo)[relativeWireID][priorityWire][iomega][iphi][itheta];
    }
  }
  return contrib;
}
