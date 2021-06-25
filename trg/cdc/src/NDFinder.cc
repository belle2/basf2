/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sebastian Skambraks                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include "framework/logging/Logger.h"
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "trg/cdc/NDFinder.h"

using namespace Belle2;
using namespace std;


void NDFinder::init(int minweight, int minpts,
                    bool diagonal, int minhits, int minhits_axial,
                    double thresh,
                    double minassign,
                    int mincells, bool verbose,
                    string axialFile, string stereoFile)
{
  m_params.minhits = (long unsigned int) minhits;
  m_params.minhits_axial = (long unsigned int) minhits_axial;
  m_params.thresh = (float) thresh;
  m_params.minassign = (float) minassign;
  m_params.mincells = (long unsigned int) mincells;
  m_params.axialFile = axialFile;
  m_params.stereoFile = stereoFile;
  m_clusterer_params.minweight = (unsigned short) minweight;
  m_clusterer_params.minpts = (unsigned short) minpts;
  m_clusterer_params.diagonal = diagonal;
  m_verbose = verbose;


  initBins();
  B2DEBUG(55, "initialized binnings");

  initHitModAxSt(*m_parrayHitMod);
  B2DEBUG(55, "initialized HitMod, a map of tsid to (orient, relid, letter).");

  /** Load the axial and stereo track to hit relations from file.*/
  loadArray(m_params.axialFile, m_compaxbins, *m_pcompAxial);
  B2DEBUG(55, "loaded zero suppressed axial array ");
  loadArray(m_params.stereoFile, m_compstbins, *m_pcompStereo);
  B2DEBUG(55, "loaded zero suppressed stereo array ");

  /** Unpack zero suppresed track to hit relations.*/
  restoreZeros(m_expaxbins, m_compaxbins, *m_parrayAxialExp, *m_pcompAxial);
  B2DEBUG(55, "restored expanded axial array (11/32 phi) ");
  restoreZeros(m_expstbins, m_compstbins, *m_parrayStereoExp, *m_pcompStereo);
  B2DEBUG(55, "restored expanded stereo array (11/32 phi) ");
  squeezeAll(m_axbins, *m_parrayAxial, *m_parrayAxialExp, m_params.parcels, m_params.parcelsExp);
  B2DEBUG(55, "squeezed axial array (11/32 phi) --> (7/32 phi): ");
  squeezeAll(m_stbins, *m_parrayStereo, *m_parrayStereoExp, m_params.parcels, m_params.parcelsExp);
  B2DEBUG(55, "squeezed stereo array (11/32 phi) --> (7/32 phi)");

  reset();
  m_clusterer = Clusterizend(m_clusterer_params);
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

  m_axbins.hitid = m_nAx;
  m_stbins.hitid = m_nSt;
  m_axbins.phi = m_nPhiUse; //84
  m_stbins.phi = m_nPhiUse; //84
  m_compaxbins.hitid = m_nAx;
  m_compstbins.hitid = m_nSt;
  m_compaxbins.phi = m_nPhiComp;
  m_compstbins.phi = m_nPhiComp;
  m_compaxbins.theta = 1;
  m_expaxbins.hitid = m_nAx;
  m_expstbins.hitid = m_nSt;
  m_expaxbins.phi = m_nPhiExp; //132
  m_expstbins.phi = m_nPhiExp; //132

  m_fullbins.hitid = m_nTS;
  m_fullbins.phi = m_nPhiFull;

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
  vector<float> omegaRange = { -5., 5.};
  vector<float> phiRange = {0., 11.25};
  vector<float> thetaRange = {19., 140.};
  float ssOmega = (omegaRange[1] - omegaRange[0]) / m_nOmega; //40;
  float ssPhi = (phiRange[1] - phiRange[0]) / m_nPhiOne; //12;
  float ssTheta = (thetaRange[1] - thetaRange[0]) / m_nTheta; //9;
  m_acceptRanges.push_back(omegaRange);
  m_acceptRanges.push_back(phiRange);
  m_acceptRanges.push_back(thetaRange);
  m_slotsizes.push_back(ssOmega);
  m_slotsizes.push_back(ssPhi);
  m_slotsizes.push_back(ssTheta);
}



void NDFinder::loadArray(const std::string& filename, ndbinning bins, c5array& hitsToTracks)
{
  vector<c5elem> flatArray;
  ifstream arrayFileGZ(filename, ios_base::in | ios_base::binary);
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
  B2DEBUG(55, "loaded array from file " << filename);
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


void NDFinder::restoreZeros(ndbinning zerobins, ndbinning compbins, c5array& expArray, c5array& compArray)
{
  B2DEBUG(55, "restoreZeros: zerobins.theta " << zerobins.theta << ", combins.theta " << compbins.theta);
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


void NDFinder::printArray3D(c3array& hitsToTracks, ndbinning bins, ushort phi_inc = 1, ushort om_inc = 1, ushort divide = 4,
                            ushort minweightShow = 1)
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

  B2DEBUG(55, "printArray3D, phistart = " << phistart << ", phiend = " << phiend);
  auto d3shp = hitsToTracks.shape();
  B2DEBUG(55, "printArray shape: " << d3shp[0] << ", " << d3shp[1] << ", " << d3shp[2]);

  if (phiend == phistart) {
    return;
  }
  c3index itheta = 4; // only print itheta = 4
  for (c3index iomega = 0; iomega < bins.omega; iomega += om_inc) {
    for (c3index iphi = phistart; iphi < phiend; iphi += phi_inc) {
      // reduce printed weight
      ushort valRed = (ushort)((hitsToTracks[iomega][iphi][itheta]) / divide);
      if (valRed <= minweightShow) // skip small weights
        cout << " ";
      else
        cout << valRed;
    }
    cout << "|" << endl;
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
    addC3Comp(hitr, prio, *m_pcompAxial, DstartComp, m_compaxbins);
  } else {
    addC3Comp(hitr, prio, *m_pcompStereo, DstartComp, m_compstbins);
  }
}


void NDFinder::addC3Comp(ushort hitr,
                         ushort prio, c5array& hitsToTracks,
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
    B2DEBUG(55, "m_houghPlane, 2");
    printArray3D(*m_phoughPlane, m_fullbins);
  }
  getCM();
}


std::vector<std::vector<unsigned short>> NDFinder::getHitsVsClusters(std::vector<SimpleCluster>& clusters)
{
  unsigned short nClusters = clusters.size();
  unsigned short default_value = 0;
  std::vector<unsigned short> hitElem(m_nHits, default_value);
  std::vector<std::vector<unsigned short>> hitsVsClusters(nClusters, hitElem);

  for (unsigned long iclus = 0; iclus < clusters.size(); iclus++) {
    SimpleCluster cli = clusters[iclus];
    vector<cell_index> entries = cli.getEntries();
    cell_index maxid = getMax(entries);
    for (unsigned long ihit = 0; ihit < m_hitIds.size(); ihit++) {
      ushort contrib = hitContrib(maxid, ihit);
      hitsVsClusters[iclus][ihit] = contrib;
    }
  }
  return hitsVsClusters;
}

vector<SimpleCluster>
NDFinder::relateHitsToClusters(std::vector<std::vector<unsigned short>>& hitsVsClusters, std::vector<SimpleCluster>& clusters)
{
  vector<SimpleCluster> useclusters;
  vector<unsigned long> restHits;
  if (hitsVsClusters.size() > 0) {
    for (unsigned long ihit = 0; ihit < m_hitIds.size(); ihit++) {
      int cid = hitToCluster(hitsVsClusters, ihit);
      if (cid != -1) { // add hit to cluster
        clusters[cid].add_hit(ihit, hitsVsClusters[cid][ihit], m_hitOrients[ihit]);
      } else {
        restHits.push_back(ihit);
      }
    }

    // select clusters, try to reassign rest hits
    vector<bool> processed(hitsVsClusters.size(), false);
    for (long unsigned int iround = 0; iround <= (m_params.minhits - 2); iround++) {
      if (iround > 0) {
        vector<unsigned long> stillRest;
        for (unsigned long irhit = 0; irhit < restHits.size(); irhit++) {
          unsigned long ihit = restHits[irhit];
          int cid = hitToCluster(hitsVsClusters, ihit);
          if (cid != -1) { // add hit to cluster
            clusters[cid].add_hit(ihit, hitsVsClusters[cid][ihit], m_hitOrients[ihit]);
          } else {
            stillRest.push_back(ihit);
          }
        }
        restHits = stillRest;
      }
      stringstream msg;
      for (long unsigned int iclus = 0; iclus < hitsVsClusters.size(); iclus++) {
        if (processed[iclus])
          continue;
        SimpleCluster& clu = clusters[iclus];
        //if (clu.get_hits().size() >= m_params.minhits) {
        if (clu.get_hits().size() >= m_params.minhits && clu.get_naxial() >= m_params.minhits_axial) {
          useclusters.push_back(clusters[iclus]);
          processed[iclus] = true;
          msg << ", add iclu=" << iclus << "(nHits=" << clu.get_hits().size() << ",round=" << iround << ")";
        } else if (clu.get_hits().size() < ((m_params.minhits - 2) + iround)) { //remove small clusters
          msg << ", skip iclu=" << iclus << "(nHits=" << clu.get_hits().size() << ",round=" << iround << ")";
          for (unsigned long int ihics = 0; ihics < hitsVsClusters[0].size(); ihics++) {
            hitsVsClusters[iclus][ihics] = 0;
          }
          processed[iclus] = true;
          for (unsigned short ihirs : clu.get_hits()) {
            restHits.push_back(ihirs);
          }
        }
      }
      if (m_verbose) {
        /** hit assignment */
        B2DEBUG(55, "iround=" << iround <<
                ", restHits: " << restHits.size() << ", useclusters: " <<
                useclusters.size() << ", msg=" << msg.str());
      }
    }
  }
  return useclusters;
}


void NDFinder::getCM()
{
  c3array& houghPlane = *m_phoughPlane;
  m_clusterer.setNewPlane(houghPlane);
  std::vector<SimpleCluster> allClusters = m_clusterer.dbscan();
  std::vector<SimpleCluster> clusters;
  for (SimpleCluster& clu : allClusters) {
    if (clu.getEntries().size() > m_params.mincells) {
      clusters.push_back(clu);
    }
  }

  std::vector<std::vector<unsigned short>> hitsVsClusters = getHitsVsClusters(clusters);
  vector<SimpleCluster> useclusters = relateHitsToClusters(hitsVsClusters, clusters);

  for (unsigned long iclus = 0; iclus < useclusters.size(); iclus++) {
    SimpleCluster cli = useclusters[iclus];
    vector<cell_index> entries = cli.getEntries();
    cell_index maxid = getMax(entries);
    ushort maxval = houghPlane[maxid[0]][maxid[1]][maxid[2]];
    float cutoff = m_params.thresh * maxval;
    vector<cellweight> highWeight = getHighWeight(entries, cutoff);
    vector<vector<long int>> flatHW;
    for (auto cx : highWeight) {
      flatHW.push_back({cx.index[0], cx.index[1], cx.index[2], (unsigned short int)cx.weight});
    }
    vector<double> result = getWeightedMean(highWeight);
    vector<double> estimate = getBinToVal(result);
    vector<double> transformed = transform(estimate);
    /** pt, phiRad, cotTheta, cluster */
    m_NDFinderTracks.push_back(NDFinderTrack(transformed, cli));
  }
}


std::vector<double> NDFinder::getBinToVal(std::vector<double> thisAv)
{
  vector<double> estimate;
  for (ushort idim = 0; idim < 3; idim++) {
    double trafd = m_acceptRanges[idim][0] + (thisAv[idim] + 0.5) * m_slotsizes[idim];
    estimate.push_back(trafd);
  }
  return estimate;
}

std::vector<double> NDFinder::getWeightedMean(std::vector<cellweight> highWeight)
{
  double axomega = 0.;
  double axphi = 0.;
  double axtheta = 0.;
  int ncells = 0;
  long weightSum = 0;

  for (cellweight& elem : highWeight) {
    axomega += elem.index[0] * elem.weight;
    axphi += elem.index[1] * elem.weight;
    axtheta += elem.index[2] * elem.weight;
    ncells++;
    weightSum += elem.weight;
  }
  axomega /= weightSum;
  axphi /= weightSum;
  axtheta /= weightSum;
  vector<double> result = {axomega, axphi, axtheta};
  return result;
}

int NDFinder::hitToCluster(std::vector<std::vector<unsigned short>>& hitsVsClusters, unsigned short ihit)
{
  int cur_clus = -1;
  std::vector<std::vector<ushort>> candWeights;

  for (unsigned long iclus = 0; iclus < hitsVsClusters.size(); iclus++) {
    std::vector<ushort> cwEntry = {(ushort) iclus, hitsVsClusters[iclus][ihit]};
    candWeights.push_back(cwEntry);
  }
  if (candWeights.size() == 1) { // single cluster case
    if (candWeights[0][1] > 0) { // hit contributes non-zero weight
      cur_clus = 0;
    }
  } else if (candWeights.size() > 1) {
    struct mySortClass {
      bool operator()(vector<ushort> i, vector<ushort> j) {return (i[1] > j[1]);}
    } mySortObject;
    sort(candWeights.begin(), candWeights.end(), mySortObject);

    if (candWeights[0][1] > 0) { // the hit contributes non-zero weight
      float crit = fabs((float)candWeights[0][1] - (float)candWeights[1][1]) / ((float)candWeights[0][1]);
      if (crit > m_params.minassign) {
        cur_clus = candWeights[0][0];
      }
    }
  }
  return cur_clus;
}


cell_index NDFinder::getMax(std::vector<cell_index>& entries)
{
  ushort cur_weight = 0;
  cell_index cur_max_index = {0, 0, 0};
  c3array& houghPlane = *m_phoughPlane;

  for (cell_index& entry : entries) {
    if (houghPlane[entry[0]][entry[1]][entry[2]] > cur_weight) {
      cur_weight = houghPlane[entry[0]][entry[1]][entry[2]];
      cur_max_index = entry;
    }
  }
  return cur_max_index;
}


vector<cellweight> NDFinder::getHighWeight(std::vector<cell_index> entries, float cutoff)
{
  vector<cellweight> cellsAndWeight;
  c3array& houghPlane = *m_phoughPlane;
  for (cell_index& entry : entries) {
    ushort cellWeight = houghPlane[entry[0]][entry[1]][entry[2]];
    if (cellWeight > cutoff) {
      cellweight cur_elem;
      cur_elem.index = entry;
      cur_elem.weight = cellWeight;
      cellsAndWeight.push_back(cur_elem);
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

  c2array& arrayHitMod = *m_parrayHitMod;
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
  clusterer_params cpa = m_clusterer.getParams();
  B2DEBUG(55, "clusterer_params minweight=" << cpa.minweight << ", minpts=" << cpa.minpts << ", diagonal=" << cpa.diagonal);
  B2DEBUG(55, "ndFinderParams minhits=" << m_params.minhits <<
          ", minhits_axial=" << m_params.minhits_axial << ", thresh=" << m_params.thresh << ", minassign=" <<
          m_params.minassign <<
          ", mincells=" << m_params.mincells);
}
