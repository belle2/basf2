/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Ferstl                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/AnalizerCollectorTFInfo.h>

#include <framework/gearbox/Const.h>

#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>

#include "tracking/vxdCaTracking/FilterID.h"


//C++ std lib
#include <utility>
#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include <array>

using namespace std;
using namespace Belle2;

/**
 * Information about the Methodes in AnalizerCollectorTFInfo.h
 */

ClassImp(AnalizerCollectorTFInfo)

const string AnalizerCollectorTFInfo::m_fileSeparator = ",";


AnalizerCollectorTFInfo::AnalizerCollectorTFInfo()
{
  //m_output_flag = 0;

}

AnalizerCollectorTFInfo::~AnalizerCollectorTFInfo()
{
}


/** Sets all Particle IDs and real-Information from all clusters */
void AnalizerCollectorTFInfo::setAllParticleIDs(double boarder)
{

  // The Information about the ParticleID is stored in the svd & pxd Clusters
  // so to get to the Information the relative Position (stored in the TF-Clusters)
  // in this StoreArray is used
  StoreArray<ClusterTFInfo> clusterTFInfo("");
  int nCount = clusterTFInfo.getEntries();
  if (nCount == 0) {B2DEBUG(100, "VXDTF: Display: clusterTFInfo is empty!");}
  B2DEBUG(100, "setAllParticleIDs -nCount: " << nCount);

  StoreArray<SVDCluster> svdClusters("");
  int nsvdclusters = svdClusters.getEntries();
  if (nsvdclusters == 0) {B2DEBUG(100, "AnalizerCollectorTFInfo: svdClusters is empty!");}
  B2DEBUG(100, "setAllParticleIDs - nsvdclusters: " << nsvdclusters);

  StoreArray<PXDCluster> pxdClusters("");
  int npxdclusters = pxdClusters.getEntries();
  if (npxdclusters == 0) {B2DEBUG(100, "AnalizerCollectorTFInfo: pxdClusters is empty!");}
  B2DEBUG(100, "setAllParticleIDs - npxdclusters: " << npxdclusters);


  for (auto & currentCluster : clusterTFInfo) {

    // Detector Type for svd/pxd difference
    int particleID = -1;
    bool isReal = 0;
    int pdgCode = 0;

    int detectorType = currentCluster.getDetectorType();
    int passIndex = currentCluster.getPassIndex();
    int clusterId = currentCluster.getRealClusterID();
    int relativeClusterId = currentCluster.getRelativeClusterID();

    B2DEBUG(100, "setAllParticleIDs - init: passIndex: " << passIndex << ", ClusterID: " << clusterId << "; Detector Type: " << detectorType << "; Relative Cluster ID: " << relativeClusterId);


    if (relativeClusterId == -1) {
      B2DEBUG(100, "setAllParticleIDs - No Relative Cluster Information found");
      continue;
    }

    if (detectorType == Const::SVD)  {

      if (nsvdclusters != 0) {

        if (relativeClusterId >= nsvdclusters) {
          B2DEBUG(100, "setAllParticleIDs - No valid Cluster ID in svd: " << relativeClusterId);
          continue;
        }

        const SVDCluster* aCluster = svdClusters[relativeClusterId];

        RelationVector<MCParticle> mcParticleRelations = aCluster->getRelationsTo<MCParticle>();

        // Same procedure VXDSimpleClusterizerModule => first particle (should not have a second particleID)
        particleID = mcParticleRelations[0]->getIndex();
        isReal = mcParticleRelations[0]->hasStatus(MCParticle::c_PrimaryParticle);
        pdgCode = mcParticleRelations[0]->getPDG();

        // should not have a second particleID
        if (mcParticleRelations.size() > 1) {
          B2DEBUG(100, "2nd Particle ID !!!: " << particleID);
        }

      }
    }

    // If PXD
    if (detectorType == Const::PXD) {

      StoreArray<PXDCluster> pxdClusters("");
      int npxdclusters = pxdClusters.getEntries();
      if (npxdclusters != 0) {

        if (relativeClusterId >= npxdclusters) {
          B2DEBUG(100, "setAllParticleIDs - No valid Cluster ID in pxd: " << relativeClusterId);
          continue;
        }

        // ID of PXD Clusters is clusterId - Start of cluster ids for pxd
        const PXDCluster* aCluster = pxdClusters[relativeClusterId];

        RelationVector<MCParticle> mcParticleRelations = aCluster->getRelationsTo<MCParticle>();

        // Same procedure VXDSimpleClusterizerModule => first particle (should not have a second particleID)
        particleID = mcParticleRelations[0]->getIndex();
        isReal = mcParticleRelations[0]->hasStatus(MCParticle::c_PrimaryParticle);
        pdgCode = mcParticleRelations[0]->getPDG();

        // should not have a second particleID
        if (mcParticleRelations.size() > 1) {
          B2DEBUG(100, "2nd Particle ID !!!: " << particleID);
        }

      }

    }

    B2DEBUG(100, "setAllParticleIDs - particleID : " << particleID << "; is real: " << isReal);
    currentCluster.setParticleID(particleID);
    currentCluster.setIsReal(isReal);
    currentCluster.setPDG(pdgCode);

  }

  // Hits, Cells & TCand have to used seperated from each other to get the Information for all Objects

  // Hits real or not real
  StoreArray<HitTFInfo> hitTFInfo("");
  nCount = hitTFInfo.getEntries();
  if (nCount == 0) {B2DEBUG(100, "VXDTF: Display: hitTFInfo is empty!");}

  for (int i = 0; i < hitTFInfo.getEntries(); i++) {
    std::array<int, 2 > currentIsReal {{0, 0}};
    std::vector<int> particleIds;

    for (auto & currentCluster : hitTFInfo[i]->getAssignedCluster()) {
      currentIsReal.at(clusterTFInfo[currentCluster]->getIsReal())++;
      particleIds.push_back(clusterTFInfo[currentCluster]->getParticleID());
    }

    // 0 = false, 1 = true, 2 = part true
    int isReal = 0;
    if (currentIsReal.at(0) > 0) {
      if (currentIsReal.at(1) == 0) {
        isReal = 0;  // false
      } else {
        isReal = 2;  // part
      }
    } else if (currentIsReal.at(1) > 0) {
      isReal = 1;    // true
    } else {
      isReal = 2;  // part
    }

    hitTFInfo[i]->setIsReal(isReal);

    // Particle List with purity
    for (uint z = 0; z < particleIds.size(); z++) {
      // if not already added to the particle list
      if (std::count(particleIds.begin(), particleIds.begin() + z, particleIds.at(z)) == 0) {
        int countParticleId = std::count(particleIds.begin(), particleIds.end(), particleIds.at(z));
        hitTFInfo[i]->push_back_UsedParticles(std::make_pair(particleIds.at(z), ((double)countParticleId / particleIds.size())));
        B2DEBUG(100, "ParticleID: " << particleIds.at(z) << "; purity: " << ((double)countParticleId / particleIds.size()));
      }
    }

    B2DEBUG(100, "setAllParticleIDs - hitID : " << i << "; is real: " << hitTFInfo[i]->getIsReal());
  }


  // Cell real or not real
  StoreArray<CellTFInfo> cellTFInfo("");
  nCount = cellTFInfo.getEntries();
  if (nCount == 0) {B2DEBUG(100, "VXDTF: Display: cellTFInfo is empty!");}

  for (int i = 0; i < cellTFInfo.getEntries(); i++) {
    std::array<int, 2 > currentIsReal {{0, 0}};
    std::vector<int> particleIds;

    for (auto & currentHits : cellTFInfo[i]->getAssignedHits()) {
      if (currentHits != -1) {

        for (auto & currentCluster : hitTFInfo[currentHits]->getAssignedCluster()) {
          currentIsReal.at(clusterTFInfo[currentCluster]->getIsReal())++;
          particleIds.push_back(clusterTFInfo[currentCluster]->getParticleID());

//    B2INFO ("* " << clusterTFInfo.at(currentCluster)->getParticleID());
        }

      }
    }

    // 0 = false, 1 = true, 2 = part true
    uint isReal = 0;
    if (currentIsReal.at(0) > 0) {
      if (currentIsReal.at(1) == 0) {
        isReal = 0;  // false
      } else {
        isReal = 2;  // part
      }
    } else if (currentIsReal.at(1) > 0) {
      isReal = 1;    // true
    } else {
      isReal = 2;  // part
    }
    cellTFInfo[i]->setIsReal(isReal);

    // Particle List with purity
    for (uint z = 0; z < particleIds.size(); z++) {
      // if not already added to the particle list
//       B2INFO ("ParticleID ? " << particleIds.at(z));
      if (std::count(particleIds.begin(), particleIds.begin() + z, particleIds.at(z)) == 0) {
        int countParticleId = std::count(particleIds.begin(), particleIds.end(), particleIds.at(z));
        cellTFInfo[i]->push_back_UsedParticles(std::make_pair(particleIds.at(z), ((double)countParticleId / particleIds.size())));
        B2DEBUG(100, "ParticleID: " << particleIds.at(z) << "; purity: " << ((double)countParticleId / particleIds.size()));
      }
    }

    B2DEBUG(100, "setAllParticleIDs - CellID : " << i << "; is real: " << cellTFInfo[i]->getIsReal());
  }


  // TCand. ghost, clean or contaminated
  StoreArray<TrackCandidateTFInfo> tfcandTFInfo("");
  nCount = tfcandTFInfo.getEntries();
  if (nCount == 0) {B2DEBUG(100, "VXDTF: Display: tfcandTFInfo is empty!");}

  for (int i = 0; i < tfcandTFInfo.getEntries(); i++) {
    std::array<int, 2 > currentIsReal {{0, 0}};
    std::vector<int> particleIds;

    for (auto & currentCells : tfcandTFInfo[i]->getAssignedCell()) {
      if (currentCells != -1) {

        for (auto & currentHits : cellTFInfo[currentCells]->getAssignedHits()) {

          if (currentHits != -1) {

            for (auto & currentCluster : hitTFInfo[currentHits]->getAssignedCluster()) {
              currentIsReal.at(clusterTFInfo[currentCluster]->getIsReal())++;
              particleIds.push_back(clusterTFInfo[currentCluster]->getParticleID());
            }
          }

        }

      }
    }

    // Particle List with purity
    for (uint z = 0; z < particleIds.size(); z++) {
      // if not already added to the particle list
      if (std::count(particleIds.begin(), particleIds.begin() + z, particleIds.at(z)) == 0) {
        int countParticleId = std::count(particleIds.begin(), particleIds.end(), particleIds.at(z));
        tfcandTFInfo[i]->push_back_UsedParticles(std::make_pair(particleIds.at(z), ((double)countParticleId / particleIds.size())));

        B2DEBUG(100, "ParticleID: " << particleIds.at(z) << "; purity: " << ((double)countParticleId / particleIds.size()));
      }
    }

    // 0 = ghost TC
    // 1 = clean TC
    // 2 = contaminated TC
    uint realState = 3;

    if (currentIsReal.at(0) > 0) {
      if (currentIsReal.at(1) == 0) {
        realState = 0;   // ghost
      }
    } else if (particleIds.size() == 1) {
      realState = 1;   // clean
    } else if (tfcandTFInfo[i]->getMainParticle().second > boarder) {
      realState = 2;   // contaminated
    } else {
      realState = 0;   // ghost
    }

    tfcandTFInfo[i]->setIsReal(realState);

    B2DEBUG(100, "setAllParticleIDs - TfCand : " << i << "; is real: " << tfcandTFInfo[i]->getIsReal());
  }

}


/** Stores All Hit Information in a file */
void AnalizerCollectorTFInfo::storeAllHitInformation(std::string filename)
{
  storeHitInformation(filename, -1);
}


/** Stores Hit Information of 1 Particle in a file */
// Information of the Hits:
// 0  hitid   int
// 1  passIndex  int
// 2  sector_id int
// 3  diet_at   string
// 4  diet_at_ID  int
// 5  real    int
// 6  MainParticleID  int
// 7  Purity    double
// 8  Hit_Position  double / double / double
// 9  Hit_Sigma double / double / double
// 10   FilterID::anglesRZ,
// 11 FilterID::anglesXY,
// 12 FilterID::distance3D,
// 13 FilterID::distanceXY,
// 14 FilterID::distanceZ,
// 15 FilterID::helixFit,
// 16 FilterID::slopeRZ,
// 17   FilterID::deltaSlopeRZ,
// 18 FilterID::pT,
// 19 FilterID::deltapT,
// 20 FilterID::normedDistance3D,
// 21 FilterID::distance2IP,
// 22 FilterID::deltaDistance2IP
// 23 FilterID::silentSegFinder
// 24   FilterID::silentTcc
// 24   Clusters size int
// 24   ClusterID 1 int
// 24   ClusterID 2 int

void AnalizerCollectorTFInfo::storeHitInformation(std::string filename, int particleIdFilter)
{
  std::vector<int> searchfilters = { FilterID::angles3D,
                                     FilterID::anglesRZ,
                                     FilterID::anglesXY,
                                     FilterID::distance3D,
                                     FilterID::distanceXY,
                                     FilterID::distanceZ,
                                     FilterID::helixFit,
                                     FilterID::slopeRZ,
                                     FilterID::deltaSlopeRZ,
                                     FilterID::pT,
                                     FilterID::deltapT,
                                     FilterID::normedDistance3D,
                                     FilterID::distance2IP,
                                     FilterID::deltaDistance2IP,
                                     FilterID::silentSegFinder,
                                     FilterID::silentTcc
                                   };

  StoreArray<HitTFInfo> hitTFInfo("");
  uint nCount = hitTFInfo.getEntries();
  if (nCount == 0) {B2DEBUG(100, "VXDTF: Display: hitTFInfo is empty!");}

  // Info for Debug
  uint counterNotRealHits = 0, counterRealHits = 0;

  ofstream myfile(filename);

  string particleText;
  if (particleIdFilter == -1) {
    particleText = "MainParticleID";
  } else {
    particleText = "ParticleID_param";
  }

  B2DEBUG(100, "Filename (storeHitInformation): " << filename);

  myfile << "hitid" << m_fileSeparator << "passIndex" << m_fileSeparator << "sector_id" << m_fileSeparator << "diet_at" << m_fileSeparator << "diet_ID" << m_fileSeparator << "real" << m_fileSeparator << particleText <<
         m_fileSeparator << "Purity" << m_fileSeparator << "Hit_Position" << m_fileSeparator << "Hit_Sigma" << m_fileSeparator << "FilterID::anglesRZ" << m_fileSeparator << "FilterID::anglesXY" << m_fileSeparator << "FilterID::distance3D" << m_fileSeparator << "FilterID::distanceXY" << m_fileSeparator << "FilterID::distanceZ" << m_fileSeparator << "FilterID::helixFit" << m_fileSeparator << "FilterID::slopeRZ" << m_fileSeparator << "FilterID::deltaSlopeRZ" << m_fileSeparator <<  "FilterID::pT" << m_fileSeparator << "FilterID::deltapT" << m_fileSeparator << "FilterID::normedDistance3D" << m_fileSeparator << "FilterID::distance2IP" << m_fileSeparator << "FilterID::deltaDistance2IP" << m_fileSeparator << "FilterID::silentSegFinder" << m_fileSeparator << "FilterID::silentTcc" << m_fileSeparator << "Clusters_Size" << m_fileSeparator << "ClusterID_1" << m_fileSeparator << "ClusterID_2" << endl;


  for (int i = 0; i <  hitTFInfo.getEntries(); i++) {

    // particleIdFilter == -1 => all Entries
    if (particleIdFilter == -1 || hitTFInfo[i]->containsParticle(particleIdFilter)) {

      myfile << i << m_fileSeparator << hitTFInfo[i]->getPassIndex() << m_fileSeparator << hitTFInfo[i]->getSectorID() << m_fileSeparator << hitTFInfo[i]->getDiedAt() << m_fileSeparator << hitTFInfo[i]->getDiedID() << m_fileSeparator << hitTFInfo[i]->getIsReal();

      if (hitTFInfo[i]->getIsReal() == 0) { counterNotRealHits++; } else { counterRealHits++; }

      // Main Particle & Purity
      if (particleIdFilter == -1) {
        myfile << m_fileSeparator << hitTFInfo[i]->getMainParticle().first << m_fileSeparator << hitTFInfo[i]->getMainParticle().second;
      } else {
        myfile << m_fileSeparator << hitTFInfo[i]->getInfoParticle(particleIdFilter).first << m_fileSeparator << hitTFInfo[i]->getInfoParticle(particleIdFilter).second;
      }

      myfile << m_fileSeparator << hitTFInfo[i]->getPosition().X() << "/" << hitTFInfo[i]->getPosition().Y() << "/" << hitTFInfo[i]->getPosition().Z();

      myfile << m_fileSeparator << hitTFInfo[i]->getHitSigma().X() << "/" << hitTFInfo[i]->getHitSigma().Y() << "/" << hitTFInfo[i]->getHitSigma().Z();

      std::vector<int> accepted = hitTFInfo[i]->getAccepted();
      std::vector<int> rejected = hitTFInfo[i]->getRejected();

      B2DEBUG(100, "SaveHitInformation: i: " << i << ", pass index: " << hitTFInfo[i]->getPassIndex() << ", size accepted: " << accepted.size() << ", size rejected: " << rejected.size());

      /*    for (auto &currentFilter: rejected) {
          B2DEBUG(100,"rejected: " << currentFilter);
          }
      */
      for (uint u = 0; u < searchfilters.size(); u++) {
        //B2DEBUG(100,"Filter ?: " << searchfilters.at(u));
        myfile << m_fileSeparator;
        //B2DEBUG(100,"searchfilters: " << searchfilters.at(u));

        if (std::find(accepted.begin(), accepted.end(), searchfilters.at(u)) != accepted.end()) {
          myfile << 1;      //true = accepted
        } else if (std::find(rejected.begin(), rejected.end(), searchfilters.at(u)) != rejected.end()) {
          myfile << 0;      //false = rejected
        } else {
          //myfile << 2;     //not found
        }

      }


      std::vector<int> assignedclusters = hitTFInfo[i]->getAssignedCluster();

      myfile << m_fileSeparator << assignedclusters.size();

      // all the time max 2 Cluster IDs
      for (uint m = 0; m < 2; m++) {

        if (m < assignedclusters.size()) {
          myfile << m_fileSeparator << assignedclusters.at(m);
        } else {
          myfile << m_fileSeparator;
        }

      }



      myfile << endl;
    }
  }

// B2INFO("RealHits: " << counterRealHits << "/ notRealHits: " << counterNotRealHits);

  myfile.close();
}


/** Stores All Cell Information in a file */
void AnalizerCollectorTFInfo::storeAllCellInformation(std::string filename)
{
  storeCellInformation(filename, -1);
}



/** Stores Cell Information (with one particleID) in a file */
// Information of the Hits:
// 0  cellid    int
// 1  passIndex  int
// 2  state   int
// 3  diet_at   string
// 4  diet_at_ID  int
// 5  real    int
// 6  MainParticleID  int
// 7  Purity    double
// 8  FilterID::distance3D,
// 9  FilterID::anglesXY,
// 10   FilterID::anglesRZ,
// 11 FilterID::distance2IP,
// 12   FilterID::deltaSlopeRZ,
// 13 FilterID::pT,
// 14 FilterID::helixFit
// 15 FilterID::nbFinderLost,
// 16 FilterID::cellularAutomaton,
// 17 FilterID::silentTcc
// 18   outer hit
// 19   inner Hit
//

void AnalizerCollectorTFInfo::storeCellInformation(std::string filename, int particleIdFilter)
{
  // nbfinder, CA
  std::vector<int> searchfilters = { FilterID::angles3D,
                                     FilterID::anglesXY,
                                     FilterID::anglesRZ,
                                     FilterID::distance2IP,
                                     FilterID::deltaSlopeRZ,
                                     FilterID::pT,
                                     FilterID::helixFit,
                                     FilterID::nbFinderLost,
                                     FilterID::cellularAutomaton,
                                     FilterID::silentTcc
                                   };

  ofstream myfile(filename);

  StoreArray<CellTFInfo> cellTFInfo("");
  uint nCount = cellTFInfo.getEntries();
  if (nCount == 0) {B2DEBUG(100, "VXDTF: Display: cellTFInfo is empty!");}

  string particleText;
  if (particleIdFilter == -1) {
    particleText = "MainParticleID";
  } else {
    particleText = "ParticleID_param";
  }


  myfile << "cellid" << m_fileSeparator << "passIndex" << m_fileSeparator << "state" << m_fileSeparator << "diet_at" << m_fileSeparator << "diet_ID" << m_fileSeparator << " real" << m_fileSeparator << particleText <<
         m_fileSeparator << "Purity" << m_fileSeparator << "FilterID::distance3D" << m_fileSeparator << "FilterID::anglesXY" << m_fileSeparator << "FilterID::anglesRZ" << m_fileSeparator << "FilterID::distance2IP" << m_fileSeparator << "FilterID::deltaSlopeRZ" << m_fileSeparator << "FilterID::pT" << m_fileSeparator << "FilterID::helixFit" << m_fileSeparator << "FilterID::nbFinderLost" << m_fileSeparator <<  "FilterID::cellularAutomaton" << m_fileSeparator << "FilterID::silentTcc" << m_fileSeparator << "outer hit" << m_fileSeparator << "inner Hit" << endl;

  for (int i = 0; i <  cellTFInfo.getEntries(); i++) {

    // particleIdFilter == -1 => all Entries
    if (particleIdFilter == -1 || cellTFInfo[i]->containsParticle(particleIdFilter)) {

      myfile << i << m_fileSeparator << cellTFInfo[i]->getPassIndex() << m_fileSeparator << cellTFInfo[i]->getState() << m_fileSeparator << cellTFInfo[i]->getDiedAt() << m_fileSeparator << cellTFInfo[i]->getDiedID() << m_fileSeparator << cellTFInfo[i]->getIsReal();

      // Main Particle & Purity
      if (particleIdFilter == -1) {
        myfile << m_fileSeparator << cellTFInfo[i]->getMainParticle().first << m_fileSeparator << cellTFInfo[i]->getMainParticle().second;
      } else {
        myfile << m_fileSeparator << cellTFInfo[i]->getInfoParticle(particleIdFilter).first << m_fileSeparator << cellTFInfo[i]->getInfoParticle(particleIdFilter).second;
      }

      std::vector<int> accepted = cellTFInfo[i]->getAccepted();
      std::vector<int> rejected = cellTFInfo[i]->getRejected();

      B2DEBUG(100, "SaveCellInformation: i: " << i << ", pass index: " << cellTFInfo[i]->getPassIndex() << ", size accepted: " <<
              accepted.size() << ", size rejected: " << rejected.size());

      for (uint u = 0; u < searchfilters.size(); u++) {
        //B2DEBUG(100,"Filter ?: " << searchfilters.at(u));
        myfile << m_fileSeparator;

        if (std::find(accepted.begin(), accepted.end(), searchfilters.at(u)) != accepted.end()) {
          myfile << 1;      //true = accepted
        } else if (std::find(rejected.begin(), rejected.end(), searchfilters.at(u)) != rejected.end()) {
          myfile << 0;      //false = rejected
        } else {
          //myfile << 2;     //not found
        }

      }

      myfile << m_fileSeparator;

      // outer hit  = Index 0, inner hit = Index 1
      if (cellTFInfo[i]->getAssignedHits().size() > 0) {
        myfile << cellTFInfo[i]->getAssignedHits().at(0);
      }

      myfile << m_fileSeparator;

      if (cellTFInfo[i]->getAssignedHits().size() > 1) {
        myfile << cellTFInfo[i]->getAssignedHits().at(1);
      }

      myfile << endl;

      for (auto & currentCoordinate : cellTFInfo[i]->getCoordinates()) {
        B2DEBUG(100, "Cell " << i << " Coordinate: " << currentCoordinate.X() << "/" << currentCoordinate.Y() << "/" << currentCoordinate.Z());
      }

    }
  }

  myfile.close();
}


/** Stores All TC Information in a file */
void AnalizerCollectorTFInfo::storeAllTCInformation(std::string filename)
{
  storeTCInformation(filename, -1);
}



/** Stores TCand Information in a file (with one particle_id) */
// Information of the Hits:
// 0  tcid    int
// 1  passIndex  int
// 2  ownid   int
// 3  diet_at   string
// 4  diet_at_id  int
// 5  real    int
// 6  MainParticleID  int
// 7  Purity    double
// 8  FilterID::hopfield,
// 9    FilterID::greedy,
// 10   FilterID::tcDuel,
// 11   FilterID::tcFinderCurr,
// 12   FilterID::ziggZaggXY,
// 13   FilterID::deltapT,
// 14   FilterID::deltaDistance2IP,
// 15   FilterID::ziggZaggRZ,
// 16   FilterID::calcQIbyKalman,
// 17   FilterID::overlapping,
// 18   FilterID::circlefit
// 19   Count Asso. Cell IDs
// 20-29 ... Cell IDs

void AnalizerCollectorTFInfo::storeTCInformation(std::string filename, int particleIdFilter)
{
  //B2INFO("storeTCInformation");

  // 10 filters?
  std::vector<int> searchfilters = {
    FilterID::hopfield,
    FilterID::greedy,
    FilterID::tcDuel,
    FilterID::tcFinderCurr,
    FilterID::ziggZaggXY,
    FilterID::deltapT,
    FilterID::deltaDistance2IP,
    FilterID::ziggZaggRZ,
    FilterID::calcQIbyKalman,
    FilterID::overlapping,
    FilterID::circlefit
  };

  ofstream myfile(filename);

  StoreArray<TrackCandidateTFInfo> tfcandTFInfo("");
  uint nCount = tfcandTFInfo.getEntries();
  if (nCount == 0) {B2DEBUG(100, "VXDTF: Display: tfcandTFInfo is empty!");}

  string particleText;
  if (particleIdFilter == -1) {
    particleText = "MainParticleID";
  } else {
    particleText = "ParticleID_param";
  }

  myfile << "tcid" << m_fileSeparator << "passIndex" << m_fileSeparator << "ownid" << m_fileSeparator << "diet_at" << m_fileSeparator << "diet_ID" << m_fileSeparator << "real" << m_fileSeparator << particleText <<
         m_fileSeparator << "Purity" << m_fileSeparator << "FilterID::hopfield" << m_fileSeparator << "FilterID::greedy" << m_fileSeparator << "FilterID::tcDuel" << m_fileSeparator << "FilterID::tcFinderCurr" << m_fileSeparator << "FilterID::ziggZaggXY" << m_fileSeparator << "FilterID::deltapT" << m_fileSeparator << "FilterID::deltaDistance2IP" << m_fileSeparator << "FilterID::ziggZaggRZ" << m_fileSeparator <<  "FilterID::calcQIbyKalman" << m_fileSeparator << "FilterID::overlapping" << m_fileSeparator << "FilterID::circlefit" << m_fileSeparator << "Count Asso. Cell IDs" << m_fileSeparator << "Cell ID 1" << m_fileSeparator << "Cell ID 2" << m_fileSeparator << "Cell ID 3" << m_fileSeparator << "Cell ID 4" << m_fileSeparator << "Cell ID 5" << m_fileSeparator << "Cell ID 6" << m_fileSeparator
         << "Cell ID 7" << m_fileSeparator << "Cell ID 8" << m_fileSeparator << "Cell ID 9" << m_fileSeparator << "Cell ID 10" << endl;

  for (int i = 0; i <  tfcandTFInfo.getEntries(); i++) {

    // particleIdFilter == -1 => all Entries
    if (particleIdFilter == -1 || tfcandTFInfo[i]->containsParticle(particleIdFilter)) {

      myfile << i << m_fileSeparator << tfcandTFInfo[i]->getPassIndex() << m_fileSeparator << tfcandTFInfo[i]->getOwnID() << m_fileSeparator << tfcandTFInfo[i]->getDiedAt() << m_fileSeparator << tfcandTFInfo[i]->getDiedID() << m_fileSeparator << tfcandTFInfo[i]->getIsReal();

      // Main Particle & Purity
      if (particleIdFilter == -1) {
        myfile << m_fileSeparator << tfcandTFInfo[i]->getMainParticle().first << m_fileSeparator << tfcandTFInfo[i]->getMainParticle().second;
      } else {
        myfile << m_fileSeparator << tfcandTFInfo[i]->getInfoParticle(particleIdFilter).first << m_fileSeparator << tfcandTFInfo[i]->getInfoParticle(particleIdFilter).second;
      }

      std::vector<int> accepted = tfcandTFInfo[i]->getAccepted();
      std::vector<int> rejected = tfcandTFInfo[i]->getRejected();

      B2DEBUG(100, "SaveTCInformation: i: " << i << ", pass index: " << tfcandTFInfo[i]->getPassIndex() << ", size accepted: " <<
              accepted.size() << ", size rejected: " << rejected.size());

      for (uint u = 0; u < searchfilters.size(); u++) {
        //B2DEBUG(100,"Filter ?: " << searchfilters.at(u));
        myfile << m_fileSeparator;

        if (std::find(accepted.begin(), accepted.end(), searchfilters.at(u)) != accepted.end()) {
          myfile << 1;      //true = accepted
        } else if (std::find(rejected.begin(), rejected.end(), searchfilters.at(u)) != rejected.end()) {
          myfile << 0;      //false = rejected
        } else {
          //myfile << 2;     //not found
        }

      }

      std::vector<int> assignedcells = tfcandTFInfo[i]->getAssignedCell();

      myfile << m_fileSeparator << assignedcells.size();

      // all the time first 10 Cell IDs
      for (uint m = 0; m < 10; m++) {

        if (m < assignedcells.size()) {
          myfile << m_fileSeparator << assignedcells.at(m);
        } else {
          myfile << m_fileSeparator;
        }

      }

      myfile << endl;

      for (auto & currentCoordinate : tfcandTFInfo[i]->getCoordinates()) {
        B2DEBUG(100, "TC " << i << " Coordinate: " << currentCoordinate.X() << "/" << currentCoordinate.Y() << "/" << currentCoordinate.Z());
      }

    }
  }

  myfile.close();
}




/** Stores Clusters Information in a file */
// Information of the Clusters:
// 0  ClusterID   int
// 1  passIndex    int
// 2  Relative Cluster ID int
// 3  diet_at     string
// 4  diet_at_id    int
// 5  real      int
// 6  Detector Type   int
// 7  ParticleID    int

void AnalizerCollectorTFInfo::storeClustersInformation(std::string filename)
{

  ofstream myfile(filename);

  StoreArray<ClusterTFInfo> clusterTFInfo("");
  uint nCount = clusterTFInfo.getEntries();
  if (nCount == 0) {B2DEBUG(100, "AnalizerCollectorTFInfo: Display: clusterTFInfo is empty!");}

  myfile << "ClusterID" << m_fileSeparator << "passIndex" << m_fileSeparator << "Relative_Cluster_ID" << m_fileSeparator << "diet_at" << m_fileSeparator << "diet_ID" << m_fileSeparator << "real" << m_fileSeparator << "Detector_Type" << m_fileSeparator << "Particle_ID" << m_fileSeparator << "PDG" << endl;

  for (int i = 0; i <  clusterTFInfo.getEntries(); i++) {

    myfile << i << m_fileSeparator << clusterTFInfo[i]->getPassIndex() << m_fileSeparator << clusterTFInfo[i]->getRelativeClusterID() << m_fileSeparator << clusterTFInfo[i]->getDiedAt() << m_fileSeparator << clusterTFInfo[i]->getDiedID() << m_fileSeparator << clusterTFInfo[i]->getIsReal() << m_fileSeparator <<  clusterTFInfo[i]->getDetectorType() << m_fileSeparator << clusterTFInfo[i]->getParticleID() << m_fileSeparator << clusterTFInfo[i]->getPDG() << endl;

  }

  myfile.close();
}



/** Stores Sector Information in a file */
// Information of the Sector:
// 0  SectorID    int
// 1  passIndex    int
// 2  Real Sector ID    int
// 3  diet_at     string
// 4  diet_at_id    int
// 5  Friends only    int
// 6  Point 1     double / double / double
// 7  Point 2     double / double / double
// 8  Point 3     double / double / double
// 9  Point 4     double / double / double
// 10 Friend Sectors Count  int
// 11-25 Friend Sector IDs  int

void AnalizerCollectorTFInfo::storeSectorInformation(std::string filename, bool withFriends)
{

  ofstream myfile(filename);

  StoreArray<SectorTFInfo> sectorTFInfo("");
  int nCount = sectorTFInfo.getEntries();
  if (nCount == 0) {B2DEBUG(100, "AnalizerCollectorTFInfo: Display: sectorTFInfo is empty!");}

  myfile << "SectorID" << m_fileSeparator << "passIndex" << m_fileSeparator << "Real_Sector_ID" << m_fileSeparator << "diet_at" << m_fileSeparator << "diet_ID" << m_fileSeparator << "Friends_only" << m_fileSeparator << "Point_1" << m_fileSeparator << "Point_2" << m_fileSeparator << "Point_3" << m_fileSeparator << "Point_4" << m_fileSeparator << "Friends_Sector_IDs";

  for (uint m = 0; m < 15; m++) {
    myfile << m_fileSeparator << "Sector ID " << (m + 1);
  }

  myfile << endl;

  for (int i = 0; i <  sectorTFInfo.getEntries(); i++) {

    if (withFriends || (!sectorTFInfo[i]->getIsOnlyFriend())) {

      myfile << i << m_fileSeparator << sectorTFInfo[i]->getPassIndex() << m_fileSeparator << sectorTFInfo[i]->getSectorID() << m_fileSeparator << sectorTFInfo[i]->getDiedAt() << m_fileSeparator << sectorTFInfo[i]->getDiedID() << m_fileSeparator << sectorTFInfo[i]->getIsOnlyFriend() << m_fileSeparator <<  sectorTFInfo[i]->getPoint(0).X() << "/" << sectorTFInfo[i]->getPoint(0).Y() << "/" << sectorTFInfo[i]->getPoint(0).Z() << m_fileSeparator << sectorTFInfo[i]->getPoint(1).X() << "/" << sectorTFInfo[i]->getPoint(1).Y() << "/" << sectorTFInfo[i]->getPoint(1).Z() << m_fileSeparator <<  sectorTFInfo[i]->getPoint(2).X() << "/" << sectorTFInfo[i]->getPoint(2).Y() << "/" << sectorTFInfo[i]->getPoint(2).Z() << m_fileSeparator << sectorTFInfo[i]->getPoint(3).X() << "/" << sectorTFInfo[i]->getPoint(3).Y() << "/" << sectorTFInfo[i]->getPoint(3).Z();

      std::vector<int> friendSectors = sectorTFInfo[i]->getFriends();

      myfile << m_fileSeparator << friendSectors.size();

      // all the time first 15 Sector Friends
      for (uint m = 0; m < 15; m++) {

        if (m < friendSectors.size()) {
          myfile << m_fileSeparator << friendSectors.at(m);
        } else {
          myfile << m_fileSeparator;
        }

      }

      myfile << endl;

      for (auto & currentCoordinate : sectorTFInfo[i]->getCoordinates()) {
        B2DEBUG(100, "Sector " << i << " Coordinate: " << currentCoordinate.X() << "/" << currentCoordinate.Y() << "/" << currentCoordinate.Z());
      }

    }
  }

  myfile.close();
}



/** getter of the TCand-ID to a given gfIndex */
int AnalizerCollectorTFInfo::getTcIDFromGfIndex(int gfIndex)
{
  int tcIDCurrent = -1;

  // no gfIndex
  if (gfIndex == -1) { return tcIDCurrent; }

  StoreArray<TrackCandidateTFInfo> tfcandTFInfo("");
  int nCount = tfcandTFInfo.getEntries();
  if (nCount == 0) {B2DEBUG(100, "getTcIDFromGfIndex: tfcandTFInfo is empty!");}

  for (auto & currentTC : tfcandTFInfo) {

    //Search for connected TrackCandidate
    if (currentTC.getAssignedGFTC() == gfIndex) {
      tcIDCurrent = currentTC.getOwnID();
      continue;
    }
  }

  return tcIDCurrent;

}





