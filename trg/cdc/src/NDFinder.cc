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
                    bool diagonal, int minhits, double thresh,
                    double minassign,
                    int mincells)
{
  m_params.minhits = (long unsigned int) minhits;
  m_params.thresh = (float) thresh;
  m_params.minassign = (float) minassign;
  m_params.mincells = (long unsigned int) mincells;
  m_clusterer_params.minweight = (unsigned short) minweight;
  m_clusterer_params.minpts = (unsigned short) minpts;
  m_clusterer_params.diagonal = diagonal;


  /** Create the arrays of the track to hit relations for axial and stereo hits.
   * Since the CDC wire pattern is repeated 32 times,
   * the hit ids are stored for 1/32 of the CDC only.
   * The total number of 2336 TS corresponds to (41 + 32) * 32 */

  /** set default binnings */
  m_nbinsPhi32 = 384;
  m_nbinsPhi1 = m_nbinsPhi32 / m_params.phigeo; // 384/32  = 12

  m_axbins.hitid = 41;
  m_stbins.hitid = 32;
  m_compaxbins.hitid = 41;
  m_compstbins.hitid = 32;
  m_compaxbins.phi = 15;
  m_compstbins.phi = 15;
  m_compaxbins.theta = 1;


  m_fullbins.hitid = 2336;
  m_fullbins.phi = m_nbinsPhi32; //384;

  /** Load the axial and stereo track to hit relations from file.*/
  bool successAx = loadArray(m_params.arrayAxialFile, m_axbins, m_arrayAxial);
  B2DEBUG(78, "loaded axial array: " << successAx);
  bool successSt = loadArray(m_params.arrayStereoFile, m_stbins, m_arrayStereo);
  B2DEBUG(78, "loaded stereo array: " << successSt);
  bool successHitMod = loadHitModAxSt(m_params.hitModAxStFile, m_fullbins, m_arrayHitMod);
  B2DEBUG(78, "loaded HitMod: " << successHitMod);

  bool sucCompAx = loadArray(m_params.compAxialFile, m_compaxbins, m_compAxial);
  B2DEBUG(78, "loaded axial array: " << sucCompAx);
  bool sucCompSt = loadArray(m_params.compStereoFile, m_compstbins, m_compStereo);
  B2DEBUG(78, "loaded stereo array: " << sucCompSt);

  reset();
  m_clusterer = Clusterizend(m_clusterer_params);
  vector<ushort> planeShape = {40, m_nbinsPhi32, 9}; //{40, 384, 9};
  m_clusterer.setPlaneShape(planeShape);

  /** Acceptance Ranges */
  vector<float> omegaRange = { -5., 5.};
  vector<float> phiRange = {0., 11.25};
  vector<float> thetaRange = {19., 140.};
  float ssOmega = (omegaRange[1] - omegaRange[0]) / 40;
  float ssPhi = (phiRange[1] - phiRange[0]) / 12;
  float ssTheta = (thetaRange[1] - thetaRange[0]) / 9;
  m_acceptRanges.push_back(omegaRange);
  m_acceptRanges.push_back(phiRange);
  m_acceptRanges.push_back(thetaRange);
  m_slotsizes.push_back(ssOmega);
  m_slotsizes.push_back(ssPhi);
  m_slotsizes.push_back(ssTheta);
}

bool NDFinder::loadArray(const std::string& filename, binning bins, c5array& hitsToTracks)
{
  B2DEBUG(13, "loadArray( " << filename << " ... ");
  B2DEBUG(13, "binning: hitid=" << bins.hitid << ", prio=" << bins.prio <<
          ", omega=" << bins.omega << ", phi=" << bins.phi <<
          ", theta=" << bins.theta);
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
    return false;
  }
  B2DEBUG(35, "loaded array from file " << filename << ". size is " << flatArray.size());
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
  return true;
}



bool NDFinder::loadHitModAxSt(const std::string& filename, binning bins, c2array& hitMod)
{
  vector<c2elem> flatArray;
  ifstream arrayFileGZ(filename, ios_base::in | ios_base::binary);
  boost::iostreams::filtering_istream arrayStream;
  arrayStream.push(boost::iostreams::gzip_decompressor());
  arrayStream.push(arrayFileGZ);
  c2elem uline;
  if (arrayFileGZ.is_open()) {
    while (arrayStream >> uline) {
      flatArray.push_back(uline);
    }
    arrayFileGZ.close();
  } else {
    B2ERROR("could not open array file: " << filename);
    return false;
  }
  B2DEBUG(35, "loaded array from file " << filename << ". size is " << flatArray.size());
  unsigned long icount = 0;
  for (c2index ihit = 0; ihit < bins.hitid; ihit++) { //2336
    for (c2index iorl = 0; iorl < bins.prio; iorl++) { //3
      hitMod[ihit][iorl] = flatArray[icount];
      icount++;
    }
  }
  return true;
}




bool NDFinder::printArray3D(c3array& hitsToTracks, binning bins, ushort phi_inc = 1, ushort om_inc = 1)
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
      phistart = iphi;
    } else if (phiSlice != 0 and not started) {
      started = true;
    } else if (phiSlice != 0 and started) {
      phiend = iphi;
    } else if (phiSlice == 0 and started) {
      phiend = iphi;
      break;
    }
  }
  ///** TODO NOTE DEBUG */
  //if (phiend = 384) {
  //  phiend = 192;
  //}

  B2DEBUG(55, "printArray3D, phistart = " << phistart << ", phiend = " << phiend);
  auto d3shp = hitsToTracks.shape();
  B2DEBUG(78, "printArray shape: " << d3shp[0] << ", " << d3shp[1] << ", " << d3shp[2]);

  ushort cphi = 0;
  //for (c3index itheta = 0; itheta < bins.theta; itheta++) {
  for (c3index itheta = 4; itheta < 5; itheta++) {
    for (c3index iomega = 0; iomega < bins.omega; iomega += om_inc) {
      for (c3index iphi = phistart; iphi < phiend; iphi += phi_inc) {
        cphi++;
        ushort valRed = (ushort)((hitsToTracks[iomega][iphi][itheta]) / 4);
        if (valRed <= 1)
          //if (valRed == 0)
          cout << " ";
        else
          cout << valRed;
      }
      cout << "|" << cphi << endl;
      cphi = 0;
    }
  }
  return true;
}

/** orient: {1 : axial, 0 : stereo} */
void NDFinder::addLookup(unsigned short ihit)
{
  c2elem orient = m_arrayHitMod[m_hitIds[ihit]][0];
  c2elem hitr = m_arrayHitMod[m_hitIds[ihit]][1];
  c2elem letter =  m_arrayHitMod[m_hitIds[ihit]][2];
  unsigned short prio = m_prioPos[ ihit ];
  short letterShort = (short) letter;

  //TODO: get letter center from m_params.parcels
  // required to get hit contribution to cluster.
  // 7 of 32 phi parcels: center = 3 (hit contrib)
  // 11 of 32 phi parcels: center = 5 (comp)
  short DstartShort = (letterShort - 3) % m_params.phigeo * m_nbinsPhi1;
  if (DstartShort < 0) {DstartShort = m_nbinsPhi32 + DstartShort;} // 384;
  short DstartComp = (letterShort - 5) % m_params.phigeo * m_nbinsPhi1;
  if (DstartComp < 0) {DstartComp = m_nbinsPhi32 + DstartComp;} // 384;

  m_vecDstart.push_back(DstartShort);
  m_hitOrients.push_back(orient);

  if (orient == 1) {
    addC3Comp(hitr, prio, m_compAxial, DstartComp, m_compaxbins);
  } else {
    addC3Comp(hitr, prio, m_compStereo, DstartComp, m_compstbins);
  }
}

void NDFinder::addC3Comp(ushort hitr,
                         ushort prio, c5array& hitsToTracks,
                         short Dstart, binning bins)
{
  ushort ntheta = 0;
  for (ushort itheta = 0; itheta < 9; itheta++) {
    if (bins.theta > 1) {
      ntheta = itheta;
    }
    for (ushort iomega = 0; iomega < 40; iomega++) {
      ushort startfield = 0;
      ushort lenfield = 1;
      ushort xstart = hitsToTracks[hitr][prio][iomega][startfield][ntheta];
      ushort xlen = hitsToTracks[hitr][prio][iomega][lenfield][ntheta];
      for (ushort iphiz = 0; iphiz < xlen; iphiz++) {
        ushort iphix = iphiz + 2;
        ushort iphi = iphiz + xstart;
        ushort iHoughPhi = (iphi + Dstart) % m_nbinsPhi32;
        m_houghPlane[iomega][iHoughPhi][itheta] += hitsToTracks[hitr][prio][iomega][iphix][ntheta];
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
  B2DEBUG(17, "m_houghPlane, 2");
  printArray3D(m_houghPlane, m_fullbins);// , 2); */
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
    cell_index maxid = getMax(entries); /** TODO */
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
        clusters[cid].add_hit(ihit, hitsVsClusters[cid][ihit]);
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
            clusters[cid].add_hit(ihit, hitsVsClusters[cid][ihit]);
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
        if (clu.get_hits().size() >= m_params.minhits) {
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
      /**DEBUG hit assignment */
      B2DEBUG(15, "iround=" << iround <<
              ", restHits: " << restHits.size() << ", useclusters: " <<
              useclusters.size() << ", msg=" << msg.str());
    }
  }
  return useclusters;
}


void NDFinder::getCM()
{
  m_clusterer.setNewPlane(m_houghPlane);
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
    ushort maxval = m_houghPlane[maxid[0]][maxid[1]][maxid[2]];
    float cutoff = m_params.thresh * maxval;
    vector<cellweight> highWeight = getHighWeight(entries, cutoff);
    vector<vector<long int>> flatHW;
    for (auto cx : highWeight) {
      flatHW.push_back({cx.index[0], cx.index[1], cx.index[2], (unsigned short int)cx.weight});
    }
    vector<double> result = getWeightedMean(highWeight);
    vector<double> estimate = getBinToVal(result);
    vector<double> transformed = transform(estimate);
    m_NDFinderTracks.push_back(NDFinderTrack(transformed, cli)); //pt, phiRad, cotTheta, cluster
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
      float crit1 = fabs((float)candWeights[0][1] - (float)candWeights[1][1]) / ((float)candWeights[0][1]);
      if (crit1 > m_params.minassign) { // 0.2
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

  for (cell_index& entry : entries) {
    if (m_houghPlane[entry[0]][entry[1]][entry[2]] > cur_weight) {
      cur_weight = m_houghPlane[entry[0]][entry[1]][entry[2]];
      cur_max_index = entry;
    }
  }
  return cur_max_index;
}


vector<cellweight> NDFinder::getHighWeight(std::vector<cell_index> entries, float cutoff)
{
  vector<cellweight> cellsAndWeight;
  for (cell_index& entry : entries) {
    ushort cellWeight = m_houghPlane[entry[0]][entry[1]][entry[2]];
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
    iphi = m_nbinsPhi32 - Dstart + iHoughPhi; // 384
  }
  ushort iomega = peak[0];
  ushort itheta = peak[2];
  ushort orient = m_hitOrients[ihit];

  c2elem hitr = m_arrayHitMod[m_hitIds[ihit]][1];
  unsigned short prio = m_prioPos[ ihit ];
  if (Dstart > m_nbinsPhi32) { // 384
    B2ERROR("phi overflow: iHoughPhi = " << iHoughPhi << ", Dstart = " << Dstart << ", iphi=" << iphi);
  }
  if (iphi <= 83) { // hit covers this phi area, get contribution
    if (orient == 1) { // axial
      contrib = m_arrayAxial[hitr][prio][iomega][iphi][itheta];
    } else { // stereo
      contrib = m_arrayStereo[hitr][prio][iomega][iphi][itheta];
    }
  }
  return contrib;
}


void
NDFinder::printParams()
{
  clusterer_params cpa = m_clusterer.getParams();
  B2DEBUG(17, "clusterer_params minweight=" << cpa.minweight << ", minpts=" << cpa.minpts << ", diagonal=" << cpa.diagonal);
  B2DEBUG(17, "ndFinderParams minhits=" << m_params.minhits << ", thresh=" << m_params.thresh << ", minassign=" <<
          m_params.minassign <<
          ", mincells=" << m_params.mincells);
}
