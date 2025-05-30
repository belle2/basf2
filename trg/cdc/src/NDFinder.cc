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


void NDFinder::init(unsigned short minWeight, unsigned char minPts,
                    bool diagonal, unsigned char minSuperAxial, unsigned char minSuperStereo,
                    float thresh,
                    unsigned char minCells, bool dbscanning, unsigned short minTotalWeight, unsigned short minPeakWeight, unsigned char iterations,
                    unsigned char omegaTrim, unsigned char phiTrim, unsigned char thetaTrim,
                    bool verbose,
                    std::string& axialFile, std::string& stereoFile)
{
  m_params.minSuperAxial = (unsigned char) minSuperAxial;
  m_params.minSuperStereo = (unsigned char) minSuperStereo;
  m_params.thresh = (float) thresh;
  m_params.minCells = (unsigned char) minCells;
  m_params.dbscanning = (bool) dbscanning;
  m_params.axialFile = axialFile;
  m_params.stereoFile = stereoFile;
  m_clustererParams.minWeight = (unsigned short) minWeight;
  m_clustererParams.minPts = (unsigned char) minPts;
  m_clustererParams.diagonal = diagonal;
  m_clustererParams.minTotalWeight = (unsigned short) minTotalWeight;
  m_clustererParams.minPeakWeight = (unsigned short) minPeakWeight;
  m_clustererParams.iterations = (unsigned char) iterations;
  m_clustererParams.omegaTrim = (unsigned char) omegaTrim;
  m_clustererParams.phiTrim = (unsigned char) phiTrim;
  m_clustererParams.thetaTrim = (unsigned char) thetaTrim;
  m_verbose = verbose;


  initBins();
  B2DEBUG(25, "initialized binnings");

  initHitModAxSt(*m_parrayHitMod);
  B2DEBUG(25, "initialized HitMod, a map of tsid to (orient, relid, letter).");

  /** Load the axial and stereo track to hit relations from file.*/
  loadArray(m_params.axialFile, m_compAxBins, *m_pcompAxial);
  B2DEBUG(25, "loaded zero suppressed axial array ");
  loadArray(m_params.stereoFile, m_compStBins, *m_pcompStereo);
  B2DEBUG(25, "loaded zero suppressed stereo array ");

  /** Unpack zero suppressed track to hit relations.*/
  restoreZeros(m_expAxBins, m_compAxBins, *m_parrayAxialExp, *m_pcompAxial);
  B2DEBUG(25, "restored expanded axial array (11/32 phi) ");
  restoreZeros(m_expStBins, m_compStBins, *m_parrayStereoExp, *m_pcompStereo);
  B2DEBUG(25, "restored expanded stereo array (11/32 phi) ");
  squeezeAll(m_axBins, *m_parrayAxial, *m_parrayAxialExp, m_params.parcels, m_params.parcelsExp);
  B2DEBUG(25, "squeezed axial array (11/32 phi) --> (7/32 phi): ");
  squeezeAll(m_stBins, *m_parrayStereo, *m_parrayStereoExp, m_params.parcels, m_params.parcelsExp);
  B2DEBUG(25, "squeezed stereo array (11/32 phi) --> (7/32 phi)");

  reset();
  m_clusterer = Clusterizend(m_clustererParams);
  m_clusterer.setPlaneShape(m_planeShape);
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
   * */

  m_nTS = 2336;
  m_nSL = 9;
  m_params.phigeo = 32;

  m_nAx = 41;
  m_nSt = 32;
  m_nPrio = 3;

  m_nOmega = 40;
  m_nPhiFull = 384;
  m_nTheta = 9;
  m_nPhiOne = m_nPhiFull / m_params.phigeo; // 384/32  = 12
  /** compressed phi: phi_start, phi_width, phi_0, ..., phi_12 */
  m_nPhiComp = 15;
  /** use phi bins: 84 = 7 * (384/32) */
  m_nPhiUse =  m_params.parcels * m_nPhiOne;
  /** expanded phi: 132 = 11 * (384/32) */
  m_nPhiExp =  m_params.parcelsExp * m_nPhiOne;

  m_axBins.hitid = m_nAx;
  m_stBins.hitid = m_nSt;
  m_axBins.phi = m_nPhiUse; //84
  m_stBins.phi = m_nPhiUse; //84
  m_compAxBins.hitid = m_nAx;
  m_compStBins.hitid = m_nSt;
  m_compAxBins.phi = m_nPhiComp;
  m_compStBins.phi = m_nPhiComp;
  m_compAxBins.theta = 1;
  m_expAxBins.hitid = m_nAx;
  m_expStBins.hitid = m_nSt;
  m_expAxBins.phi = m_nPhiExp; //132
  m_expStBins.phi = m_nPhiExp; //132

  m_fullBins.hitid = m_nTS;
  m_fullBins.phi = m_nPhiFull;

  m_pc5shapeax = {{ m_nAx, m_nPrio, m_nOmega, m_nPhiUse, m_nTheta }};
  m_pc5shapest =  {{ m_nSt, m_nPrio, m_nOmega, m_nPhiUse, m_nTheta }};
  m_pc3shape =  {{ m_nOmega, m_nPhiFull, m_nTheta }};
  m_pc2shapeHitMod =  {{ m_nTS, m_nPrio}};
  m_pc5shapeCompAx = {{ m_nAx, m_nPrio, m_nOmega, m_nPhiComp, 1 }};
  m_pc5shapeCompSt =  {{ m_nSt, m_nPrio, m_nOmega, m_nPhiComp, m_nTheta }};
  m_pc5shapeExpAx = {{ m_nAx, m_nPrio, m_nOmega, m_nPhiExp, m_nTheta }};
  m_pc5shapeExpSt =  {{ m_nSt, m_nPrio, m_nOmega, m_nPhiExp, m_nTheta }};

  m_parrayAxial     = new c5array(m_pc5shapeax);
  m_parrayStereo    = new c5array(m_pc5shapest);
  m_phoughPlane     = new c3array(m_pc3shape);
  m_parrayHitMod    = new c2array(m_pc2shapeHitMod);
  m_pcompAxial      = new c5array(m_pc5shapeCompAx);
  m_pcompStereo     = new c5array(m_pc5shapeCompSt);
  m_parrayAxialExp  = new c5array(m_pc5shapeExpAx);
  m_parrayStereoExp = new c5array(m_pc5shapeExpSt);

  m_nWires = {160, 160, 192, 224, 256, 288, 320, 352, 384};

  m_planeShape = {m_nOmega, m_nPhiFull, m_nTheta}; //{40, 384, 9};

  /** Acceptance Ranges */
  std::vector<float> omegaRange = { -5., 5.};
  std::vector<float> phiRange = {0., 11.25};
  std::vector<float> thetaRange = {19., 140.};
  float ssOmega = (omegaRange[1] - omegaRange[0]) / m_nOmega; //40;
  float ssPhi = (phiRange[1] - phiRange[0]) / m_nPhiOne; //12;
  float ssTheta = (thetaRange[1] - thetaRange[0]) / m_nTheta; //9;
  m_acceptRanges.push_back(omegaRange);
  m_acceptRanges.push_back(phiRange);
  m_acceptRanges.push_back(thetaRange);
  m_slotSizes.push_back(ssOmega);
  m_slotSizes.push_back(ssPhi);
  m_slotSizes.push_back(ssTheta);
}



void NDFinder::loadArray(const std::string& filename, ndbinning bins, c5array& hitsToTracks)
{
  std::vector<c5elem> flatArray;
  std::ifstream arrayFileGZ(filename, std::ios_base::in | std::ios_base::binary);
  boost::iostreams::filtering_istream arrayStream;
  arrayStream.push(boost::iostreams::gzip_decompressor());
  arrayStream.push(arrayFileGZ);
  c5elem uline;
  if (arrayFileGZ.is_open()) {
    while (arrayStream >> uline) {
      flatArray.push_back(uline);
    }
    arrayFileGZ.close();
  } else {
    B2ERROR("could not open array file: " << filename);
  }
  B2DEBUG(25, "loaded array from file " << filename);
  unsigned long icount = 0;
  for (c5index ihit = 0; ihit < bins.hitid; ihit++) {
    for (c5index iprio = 0; iprio < bins.prio; iprio++) {
      for (c5index iomega = 0; iomega < bins.omega; iomega++) {
        for (c5index iphi = 0; iphi < bins.phi; iphi++) {
          for (c5index itheta = 0; itheta < bins.theta; itheta++) {
            hitsToTracks[ihit][iprio][iomega][iphi][itheta] = flatArray[icount];
            icount++;
          }
        }
      }
    }
  }
}


void NDFinder::initHitModAxSt(c2array& hitMod)
{
  unsigned short phigeo = m_params.phigeo; // 32
  std::vector<int> modSLs; //nWires per SL in 1/32 sector (9)
  std::vector<int> toslid; //tsid to sl map (2336)
  std::vector<int> sloffsets = {0}; // integrated number of wires in inner SLs (9);
  std::vector<int> modoffsetsAxSt = {0, 0}; // integrated number of wires in inner SLs in 1/32 sector, separate for axial and stereo SLs (9);
  for (int isl = 0; isl < m_nSL; isl++) {
    modSLs.push_back(m_nWires[isl] / phigeo);
    for (int itsrel = 0; itsrel < m_nWires[isl]; itsrel++) {
      toslid.push_back(isl);
    }
    sloffsets.push_back(sloffsets[isl] + m_nWires[isl]);
    int last = modoffsetsAxSt[isl];
    modoffsetsAxSt.push_back(last + m_nWires[isl] / phigeo);
  }
  for (int its = 0; its < m_nTS; its++) { // 2336
    int sl = toslid[its];
    bool isAxial = (sl % 2 == 0);
    int idInSL = its - sloffsets[sl];
    int modSL = modSLs[sl];
    int modId = idInSL % modSL;
    int letter = (int) floor(idInSL / modSL);
    int relId = (int)(modoffsetsAxSt[sl] + modId);
    hitMod[its][0] = (int)(isAxial);
    hitMod[its][1] = relId;
    hitMod[its][2] = letter;
  }
}


void NDFinder::squeezeOne(c5array& writeArray, c5array& readArray, int outparcels, int inparcels, c5index ihit, c5index iprio,
                          c5index itheta, c5elem nomega)
{
  int outnphi = (int)(12 * outparcels);
  c5index trafstart = (c5index)((inparcels - outparcels) / 2 * 12);
  c5index trafend = (c5index)(trafstart + outnphi);
  for (c5index iomega = 0; iomega < nomega; iomega++) {
    for (c5index iphi = 0; iphi < outnphi; iphi++) {
      c5index readPhi = trafstart + iphi;
      writeArray[ihit][iprio][iomega][iphi][itheta] = readArray[ihit][iprio][iomega][readPhi][itheta];
    }
    for (c5index iphi = 0; iphi < trafstart; iphi++) {
      c5index writePhi = (c5index)(outnphi - trafstart + iphi);
      writeArray[ihit][iprio][iomega][writePhi][itheta] += readArray[ihit][iprio][iomega][iphi][itheta];
    }
    for (c5index iphi = 0; iphi < trafstart; iphi++) {
      c5index readPhi = trafend + iphi;
      writeArray[ihit][iprio][iomega][iphi][itheta] += readArray[ihit][iprio][iomega][readPhi][itheta];
    }
  }
}


void NDFinder::squeezeAll(ndbinning writebins, c5array& writeArray, c5array& readArray, int outparcels, int inparcels)
{
  for (c5index ihit = 0; ihit < writebins.hitid; ihit++) {
    for (c5index iprio = 0; iprio < writebins.prio; iprio++) {
      for (c5index itheta = 0; itheta < writebins.theta; itheta++) {
        squeezeOne(writeArray, readArray, outparcels, inparcels, ihit, iprio, itheta, writebins.omega);
      }
    }
  }
}


void NDFinder::restoreZeros(ndbinning zerobins, ndbinning compbins, c5array& expArray, const c5array& compArray)
{
  B2DEBUG(25, "restoreZeros: zerobins.theta " << zerobins.theta << ", combins.theta " << compbins.theta);
  for (c5index ihit = 0; ihit < compbins.hitid; ihit++) {
    for (c5index iprio = 0; iprio < compbins.prio; iprio++) {
      for (c5index iomega = 0; iomega < compbins.omega; iomega++) {
        for (c5index itheta = 0; itheta < compbins.theta; itheta++) {
          c5elem phiStart = compArray[ihit][iprio][iomega][0][itheta];
          c5elem phiWidth = compArray[ihit][iprio][iomega][1][itheta];
          for (c5index iphi = 0; iphi < phiWidth; iphi++) {
            c5elem phiCur = phiStart + iphi;
            expArray[ihit][iprio][iomega][phiCur][itheta] = compArray[ihit][iprio][iomega][iphi + 2][itheta];
            if (compbins.theta == 1) { // case axial, expand in theta
              for (c5index jtheta = 1; jtheta < zerobins.theta; jtheta++) {
                expArray[ihit][iprio][iomega][phiCur][jtheta] = compArray[ihit][iprio][iomega][iphi + 2][itheta];
              }
            }
          }
        }
      }
    }
  }
}


void NDFinder::printArray3D(c3array& hitsToTracks, ndbinning bins, ushort phiInc = 1, ushort omInc = 1, ushort divide = 4,
                            ushort minWeightShow = 1)
{
  ushort phistart = 0;
  ushort phiend = bins.phi;
  bool started = false;
  unsigned long phiSlice = 0;
  for (c3index iphi = 0; iphi < bins.phi; iphi++) {
    for (c3index itheta = 0; itheta < bins.theta; itheta++) {
      for (c3index iomega = 0; iomega < bins.omega; iomega++) {
        phiSlice += hitsToTracks[iomega][iphi][itheta];
      }
    }
    if (phiSlice == 0 and not started) {
      phistart = iphi + 1;
    } else if (phiSlice != 0 and not started) {
      started = true;
    } else if (phiSlice != 0 and started) {
      phiend = iphi;
    } else if (phiSlice == 0 and started) {
      phiend = iphi;
      break;
    }
  }

  B2DEBUG(25, "printArray3D, phistart = " << phistart << ", phiend = " << phiend);
  auto d3shp = hitsToTracks.shape();
  B2DEBUG(25, "printArray shape: " << d3shp[0] << ", " << d3shp[1] << ", " << d3shp[2]);

  if (phiend == phistart) {
    return;
  }
  c3index itheta = 4; // only print itheta = 4
  for (c3index iomega = 0; iomega < bins.omega; iomega += omInc) {
    for (c3index iphi = phistart; iphi < phiend; iphi += phiInc) {
      // reduce printed weight
      ushort valRed = (ushort)((hitsToTracks[iomega][iphi][itheta]) / divide);
      if (valRed <= minWeightShow) // skip small weights
        std::cout << " ";
      else
        std::cout << valRed;
    }
    std::cout << "|" << std::endl;
  }
}


/** orient: {1 : axial, 0 : stereo} */
void NDFinder::addLookup(unsigned short ihit)
{
  c2array& arrayHitMod = *m_parrayHitMod;

  c2elem orient = arrayHitMod[m_hitIds[ihit]][0];
  c2elem hitr   = arrayHitMod[m_hitIds[ihit]][1];
  c2elem letter =  arrayHitMod[m_hitIds[ihit]][2];

  unsigned short prio = m_prioPos[ ihit ];
  short letterShort = (short) letter;

  // Get hit contribution to cluster:
  // 7 of 32 phi parcels: center = 3
  short DstartShort = (letterShort - 3) % m_params.phigeo * m_nPhiOne;
  if (DstartShort < 0) {DstartShort = m_nPhiFull + DstartShort;}
  // Add hit to hough plane
  // 11 of 32 phi parcels: center = 5
  short DstartComp = (letterShort - 5) % m_params.phigeo * m_nPhiOne;
  if (DstartComp < 0) {DstartComp = m_nPhiFull + DstartComp;}

  m_vecDstart.push_back(DstartShort);
  m_hitOrients.push_back(orient);

  if (orient == 1) {
    addC3Comp(hitr, prio, *m_pcompAxial, DstartComp, m_compAxBins);
  } else {
    addC3Comp(hitr, prio, *m_pcompStereo, DstartComp, m_compStBins);
  }
}


void NDFinder::addC3Comp(ushort hitr,
                         ushort prio, const c5array& hitsToTracks,
                         short Dstart, ndbinning bins)
{
  ushort ntheta = 0;
  c3array& houghPlane = *m_phoughPlane;
  for (ushort itheta = 0; itheta < m_nTheta; itheta++) { //9
    if (bins.theta > 1) { //stereo
      ntheta = itheta;
    }
    for (ushort iomega = 0; iomega < m_nOmega; iomega++) { //40
      ushort startfield = 0;
      ushort lenfield = 1;
      ushort xstart = hitsToTracks[hitr][prio][iomega][startfield][ntheta];
      ushort xlen = hitsToTracks[hitr][prio][iomega][lenfield][ntheta];
      for (ushort iphiz = 0; iphiz < xlen; iphiz++) {
        ushort iphix = iphiz + 2;
        ushort iphi = iphiz + xstart;
        ushort iHoughPhi = (iphi + Dstart) % m_nPhiFull;
        houghPlane[iomega][iHoughPhi][itheta] += hitsToTracks[hitr][prio][iomega][iphix][ntheta];
      }
    }
  }
}


void NDFinder::findTracks()
{
  /** Build the Houghplane by summing up all single hit contributions */
  for (unsigned short ihit = 0; ihit < m_nHits; ihit++) {
    addLookup(ihit);
  }
  if (m_verbose) {
    B2DEBUG(25, "m_houghPlane, 2");
    printArray3D(*m_phoughPlane, m_fullBins);
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
    for (unsigned long ihit = 0; ihit < m_hitIds.size(); ihit++) {
      ushort contrib = hitContrib(maxid, ihit);
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
      for (unsigned long ihit = 0; ihit < m_hitIds.size(); ihit++) {
        unsigned short contribution = hitsVsClusters[iclus][ihit];
        if (contribution > 0) {
          superLayerNumbers.push_back({static_cast<long>(ihit), contribution, m_hitSLIds[ihit], m_prioTime[ihit]});
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
        clusterSLNumbers.push_back(m_hitSLIds[element]);
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
  c3array& houghPlane = *m_phoughPlane;
  std::vector<SimpleCluster> allClusters;
  std::vector<ROOT::Math::XYZVector> houghspace;
  if (m_params.dbscanning) {
    m_clusterer.setNewPlane(houghPlane);
    allClusters = m_clusterer.dbscan();
  } else {
    c3array copiedPlane = houghPlane;
    m_clusterer.setNewPlane(copiedPlane);
    std::vector<SimpleCluster> fixedClusters = m_clusterer.makeClusters();
    allClusters = fixedClusters;
  }
  std::vector<SimpleCluster> clusters;
  for (SimpleCluster& clu : allClusters) {
    if (clu.getEntries().size() > m_params.minCells) {
      clusters.push_back(clu);
    }
  }
  std::vector<std::vector<unsigned short>> hitsVsClusters = getHitsVsClusters(clusters);
  // New hit to cluster relation
  std::vector<SimpleCluster> useClusters = allHitsToClusters(hitsVsClusters, clusters);

  for (unsigned long iclus = 0; iclus < useClusters.size(); iclus++) {
    SimpleCluster cli = useClusters[iclus];
    std::vector<cell_index> entries = cli.getEntries();
    cell_index maxid = getMax(entries);
    ushort maxval = houghPlane[maxid[0]][maxid[1]][maxid[2]];
    float cutoff = m_params.thresh * maxval;
    std::vector<cellweight> highWeight = getHighWeight(entries, cutoff);
    std::vector<std::vector<long int>> flatHW;
    for (auto cx : highWeight) {
      flatHW.push_back({cx.index[0], cx.index[1], cx.index[2], (unsigned short int)cx.weight});
    }
    std::vector<double> result = getWeightedMean(highWeight);
    std::vector<double> estimate = getBinToVal(result);
    std::vector<double> transformed = transform(estimate);

    // READOUTS, uncomment what needed:
    std::vector<ROOT::Math::XYZVector> ndreadout;

    // Readout of the peak weight
    /* ndreadout.push_back(ROOT::Math::XYZVector(maxval, 0, 0)); */

    // Readout of the total weight
    /* unsigned long totalWeight = 0; */
    /* for (const cell_index& cellIndex : entries) { */
    /*   totalWeight += houghPlane[cellIndex[0]][cellIndex[1]][cellIndex[2]]; */
    /* } */
    /* ndreadout.push_back(ROOT::Math::XYZVector(totalWeight, 0, 0)); */

    // Readout of the number of cluster cells
    /* int ncells = entries.size(); */
    /* ndreadout.push_back(ROOT::Math::XYZVector(ncells, 0, 0)); */

    // Readout of the cluster center of gravity
    /* ndreadout.push_back(ROOT::Math::XYZVector(result[0], result[1], result[2])); */

    // Readout of the cluster weights
    /* for (const cell_index& cellIndex : entries) { */
    /*   c3elem element = houghPlane[cellIndex[0]][cellIndex[1]][cellIndex[2]]; */
    /*   ndreadout.push_back(ROOT::Math::XYZVector(element, 0, 0)); */
    /* } */

    // Readout of the cluster cell indices
    /* for (const cell_index& cellIndex : entries) { */
    /*   ndreadout.push_back(ROOT::Math::XYZVector(cellIndex[0], cellIndex[1], cellIndex[2])); */
    /* } */

    // Readout of the complete Hough space:
    /* for (c3index i = 0; i < static_cast<c3index>(houghPlane.shape()[0]); ++i) { */
    /*   for (c3index j = 0; j < static_cast<c3index>(houghPlane.shape()[1]); ++j) { */
    /*     for (c3index k = 0; k < static_cast<c3index>(houghPlane.shape()[2]); ++k) { */
    /*       c3elem element = houghPlane[i][j][k]; */
    /*       houghspace.push_back(ROOT::Math::XYZVector(element, 0, 0)); */
    /*     } */
    /*   } */
    /* } */

    m_NDFinderTracks.push_back(NDFinderTrack(transformed, cli, houghspace, ndreadout));
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
  for (ushort idim = 0; idim < 3; idim++) {
    double trafd = m_acceptRanges[idim][0] + (thisAv[idim] + 0.5) * m_slotSizes[idim];
    estimate.push_back(trafd);
  }
  return estimate;
}


std::vector<double> NDFinder::getWeightedMean(std::vector<cellweight> highWeight)
{
  double axomega = 0.;
  double axphi = 0.;
  double axtheta = 0.;
  long weightSum = 0;
  for (cellweight& elem : highWeight) {
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
  ushort curWeight = 0;
  cell_index curMaxIndex = {0, 0, 0};
  const c3array& houghPlane = *m_phoughPlane;

  for (const cell_index& entry : entries) {
    if (houghPlane[entry[0]][entry[1]][entry[2]] > curWeight) {
      curWeight = houghPlane[entry[0]][entry[1]][entry[2]];
      curMaxIndex = entry;
    }
  }
  return curMaxIndex;
}


std::vector<cellweight> NDFinder::getHighWeight(std::vector<cell_index> entries, float cutoff)
{
  std::vector<cellweight> cellsAndWeight;
  const c3array& houghPlane = *m_phoughPlane;
  for (cell_index& entry : entries) {
    ushort cellWeight = houghPlane[entry[0]][entry[1]][entry[2]];
    if (cellWeight > cutoff) {
      cellweight curElem;
      curElem.index = entry;
      curElem.weight = cellWeight;
      cellsAndWeight.push_back(curElem);
    }
  }
  return cellsAndWeight;
}


ushort NDFinder::hitContrib(cell_index peak, ushort ihit)
{
  ushort contrib = 0;
  ushort iHoughPhi = peak[1];
  short Dstart = m_vecDstart[ihit];
  ushort iphi = iHoughPhi - Dstart;
  if (Dstart > iHoughPhi && Dstart > 300) {
    iphi = m_nPhiFull - Dstart + iHoughPhi;
  }
  ushort iomega = peak[0];
  ushort itheta = peak[2];
  ushort orient = m_hitOrients[ihit];

  const c2array& arrayHitMod = *m_parrayHitMod;
  c2elem hitr = arrayHitMod[m_hitIds[ihit]][1];
  unsigned short prio = m_prioPos[ ihit ];
  if (Dstart > m_nPhiFull) {
    B2ERROR("phi overflow: iHoughPhi = " << iHoughPhi << ", Dstart = " << Dstart << ", iphi=" << iphi);
  }
  if (iphi < m_nPhiUse) { // hit covers current phi area, get contribution
    if (orient == 1) { // axial
      contrib = (*m_parrayAxial)[hitr][prio][iomega][iphi][itheta];
    } else { // stereo
      contrib = (*m_parrayStereo)[hitr][prio][iomega][iphi][itheta];
    }
  }
  return contrib;
}


void
NDFinder::printParams()
{
  clustererParams cpa = m_clusterer.getParams();
  B2DEBUG(25, "clustererParams minWeight=" << cpa.minWeight << ", minPts=" << cpa.minPts << ", diagonal=" << cpa.diagonal);
  B2DEBUG(25, "ndFinderParams minSuperAxial=" << m_params.minSuperAxial <<
          ", minSuperStereo=" << m_params.minSuperStereo << ", thresh=" << m_params.thresh <<
          ", minCells=" << m_params.minCells << ", dbscanning=" << m_params.dbscanning << ", minTotalWeight=" << m_params.minTotalWeight <<
          ", minPeakWeight=" << m_params.minPeakWeight <<
          ", iterations=" << m_params.iterations << ", omegaTrim=" << m_params.omegaTrim << ", phiTrim=" << m_params.phiTrim << ", thetaTrim="
          << m_params.thetaTrim);
}
