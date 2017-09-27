/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Ferstl                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingVXD/displayInterfaceTF/AnalizerCollectorTFInfo.h>

#include <framework/gearbox/Const.h>

#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>

#include "tracking/dataobjects/FullSecID.h"

#include "tracking/dataobjects/FilterID.h"


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

const string AnalizerCollectorTFInfo::m_fileSeparator = ",";

const int AnalizerCollectorTFInfo::m_idAlive = -1;

//Ohter Alive ID for output
const int AnalizerCollectorTFInfo::m_idAlive_output = 100;


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


  for (ClusterTFInfo& currentCluster : clusterTFInfo) {

    // Detector Type for svd/pxd difference
    int particleID = -1;
    bool isReal = 0;
    int pdgCode = 0;

    int detectorType = currentCluster.getDetectorType();
    int passIndex = currentCluster.getPassIndex();
    int clusterId = currentCluster.getRealClusterID();
    int relativeClusterId = currentCluster.getRelativeClusterID();

    B2DEBUG(100, "setAllParticleIDs - init: passIndex: " << passIndex << ", ClusterID: " << clusterId << "; Detector Type: " <<
            detectorType << "; Relative Cluster ID: " << relativeClusterId);


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
        if (mcParticleRelations.size() > 0) {
          particleID = mcParticleRelations[0]->getIndex();
          isReal = mcParticleRelations[0]->hasStatus(MCParticle::c_PrimaryParticle);
          pdgCode = mcParticleRelations[0]->getPDG();

          // should not have a second particleID
          if (mcParticleRelations.size() > 1) {
            B2DEBUG(100, "2nd Particle ID !!!: " << particleID);
          }

        }

      }
    }

    // If PXD
    if (detectorType == Const::PXD) {

      if (npxdclusters != 0) {

        if (relativeClusterId >= npxdclusters) {
          B2DEBUG(100, "setAllParticleIDs - No valid Cluster ID in pxd: " << relativeClusterId);
          continue;
        }

        // ID of PXD Clusters is clusterId - Start of cluster ids for pxd
        const PXDCluster* aCluster = pxdClusters[relativeClusterId];

        RelationVector<MCParticle> mcParticleRelations = aCluster->getRelationsTo<MCParticle>();

        // Same procedure VXDSimpleClusterizerModule => first particle (should not have a second particleID)
        if (mcParticleRelations.size() > 0) {
          particleID = mcParticleRelations[0]->getIndex();
          isReal = mcParticleRelations[0]->hasStatus(MCParticle::c_PrimaryParticle);
          pdgCode = mcParticleRelations[0]->getPDG();

          // should not have a second particleID
          if (mcParticleRelations.size() > 1) {
            B2DEBUG(100, "2nd Particle ID !!!: " << particleID);
          }

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

    for (auto& currentCluster : hitTFInfo[i]->getAssignedCluster()) {
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

    for (auto& currentHits : cellTFInfo[i]->getAssignedHits()) {
      if (currentHits != -1) {

        for (auto& currentCluster : hitTFInfo[currentHits]->getAssignedCluster()) {
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

    for (auto& currentCells : tfcandTFInfo[i]->getAssignedCell()) {
      if (currentCells != -1) {

        for (auto& currentHits : cellTFInfo[currentCells]->getAssignedHits()) {

          if (currentHits != -1) {

            for (auto& currentCluster : hitTFInfo[currentHits]->getAssignedCluster()) {
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
    // 2 = contaminated
    // 3 = real and not real particles in TC

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
// 3  died_at   string
// 4  died_at_ID  int
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
// 15 FilterID::helixParameterFit,
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
  std::vector<int> searchfilters = { FilterID::overHighestAllowedLayer,
                                     FilterID::outOfSectorRange,
                                     FilterID::angles3D,
                                     FilterID::anglesRZ,
                                     FilterID::anglesXY,
                                     FilterID::distance3D,
                                     FilterID::distanceXY,
                                     FilterID::distanceZ,
                                     FilterID::helixParameterFit,
                                     FilterID::deltaSOverZ,
                                     FilterID::deltaSlopeZOverS,
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


  // TEMP
  StoreArray<SectorTFInfo> sectorTFInfo("");
  nCount = sectorTFInfo.getEntries();
  if (nCount == 0) {B2DEBUG(100, "AnalizerCollectorTFInfo: Display: sectorTFInfo is empty!");}
  // TEMP

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


  myfile << "hitid" << m_fileSeparator << "passIndex" << m_fileSeparator << "sector_ID" << m_fileSeparator << "died_at" <<
         m_fileSeparator << "died_ID" << m_fileSeparator << "real" << m_fileSeparator << particleText << m_fileSeparator << "Purity" <<
         m_fileSeparator << "Hit_Position" << m_fileSeparator << "Hit_Sigma" << m_fileSeparator << "overHighestAllowedLayer" <<
         m_fileSeparator << "outOfSectorRange" << m_fileSeparator << FilterID::nameAngles3D << m_fileSeparator << FilterID::nameAnglesRZ <<
         m_fileSeparator << FilterID::nameAnglesXY << m_fileSeparator << FilterID::nameDistance3D << m_fileSeparator <<
         FilterID::nameDistanceXY << m_fileSeparator << FilterID::nameDistanceZ << m_fileSeparator << FilterID::nameHelixParameterFit;

  myfile << m_fileSeparator << FilterID::nameDeltaSOverZ << m_fileSeparator <<  FilterID::nameDeltaSlopeZOverS;


  myfile << m_fileSeparator << FilterID::nameSlopeRZ << m_fileSeparator << FilterID::nameDeltaSlopeRZ << m_fileSeparator <<
         FilterID::namePT << m_fileSeparator << FilterID::nameDeltapT << m_fileSeparator << FilterID::nameNormedDistance3D << m_fileSeparator
         << FilterID::nameDistance2IP << m_fileSeparator << FilterID::nameDeltaDistance2IP << m_fileSeparator <<
         FilterID::nameSilentSegFinder << m_fileSeparator << FilterID::nameSilentTcc << m_fileSeparator <<
         "Clusters_Size" << m_fileSeparator << "ClusterID_1" << m_fileSeparator << "ClusterID_2" << endl;


  for (int i = 0; i <  hitTFInfo.getEntries(); i++) {

    // particleIdFilter == -1 => all Entries
    if (particleIdFilter > -1 and hitTFInfo[i]->containsParticle(particleIdFilter) == false) { continue; }

    /*const */HitTFInfo* aHit = hitTFInfo[i];

    myfile << i << m_fileSeparator << aHit->getPassIndex() << m_fileSeparator << aHit->getSectorID() << m_fileSeparator <<
           aHit->getDiedAt() << m_fileSeparator;


    // Died ID for output change
    if (aHit->getDiedID() != m_idAlive) {
      myfile << aHit->getDiedID();
    } else {
      myfile << m_idAlive_output;
    }


    myfile << m_fileSeparator << aHit->getIsReal();

    if (aHit->getIsReal() == 0) { counterNotRealHits++; } else { counterRealHits++; }

    // Main Particle & Purity
    if (particleIdFilter == -1) {
      myfile << m_fileSeparator << aHit->getMainParticle().first << m_fileSeparator << aHit->getMainParticle().second;
    } else {
      myfile << m_fileSeparator << aHit->getInfoParticle(particleIdFilter).first << m_fileSeparator << aHit->getInfoParticle(
               particleIdFilter).second;
    }

    myfile << m_fileSeparator << aHit->getPosition().X() << "/" << aHit->getPosition().Y() << "/" << aHit->getPosition().Z();

    myfile << m_fileSeparator << aHit->getHitSigma().X() << "/" << aHit->getHitSigma().Y() << "/" << aHit->getHitSigma().Z();

    std::vector<int> accepted = aHit->getAccepted();
    std::vector<int> rejected = aHit->getRejected();

    B2DEBUG(100, "SaveHitInformation: i: " << i << ", pass index: " << aHit->getPassIndex() << ", size accepted: " << accepted.size() <<
            ", size rejected: " << rejected.size());

    B2DEBUG(100, "SaveHitInformation: Real: " << aHit->getIsReal() << ", Died At: " << aHit->getDiedAt());




    // Tests
    // FilterID::pT: 16
//     B2DEBUG(100,"FilterID::pT: " << FilterID::pT);

//      for (auto &currentFilter: rejected) {
//   B2DEBUG(100,"rejected: " << currentFilter);
//   }

//      for (auto &currentFilter: accepted) {
//   B2DEBUG(100,"accepted: " << currentFilter);
//   }

    for (uint u = 0; u < searchfilters.size(); u++) {
      //B2DEBUG(100,"Filter ?: " << searchfilters.at(u));
      myfile << m_fileSeparator;
      // B2DEBUG(100,"searchfilters: " << searchfilters.at(u));

      if (std::find(accepted.begin(), accepted.end(), searchfilters.at(u)) != accepted.end()) {
        myfile << "1";      //true = accepted
        //B2DEBUG(100,"FOUND accepted");
      } else if (std::find(rejected.begin(), rejected.end(), searchfilters.at(u)) != rejected.end()) {
        myfile << "0";      //false = rejected
        //B2DEBUG(100,"FOUND rejected");
      } else {


        // SilentKill sets the 1 here - only for better reading
        if (searchfilters.at(u) == FilterID::silentSegFinder || searchfilters.at(u) == FilterID::silentTcc) {
          myfile << "1";
        }


        //myfile << 2;     //not found
      }

    }


    std::vector<int> assignedclusters = aHit->getAssignedCluster();

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

    //TEMP
    /*
    uint currentSectorID = aHit->getSectorID();

    if (aHit->getPassIndex() == 0) {
      B2DEBUG(100, "COR - HIT: " << i << " Coordinate: " << aHit->getPosition().X() << "/" << aHit->getPosition().Y() << "/" << aHit->getPosition().Z());

      for (auto & currentSector : sectorTFInfo) {




        if (currentSector.getSectorID() == currentSectorID && currentSector.getPassIndex() == aHit->getPassIndex()) {

    B2DEBUG(100, "COR - HIT: currentSectorID: " << currentSectorID );

          int countX = 0, countY = 0, countZ = 0; // counts cases where difference of hit - sectorEdge was positive. If value is != 2, there was something wrong
          for (int nEdge = 0 ; nEdge < 4 ; ++nEdge) {

            if ((aHit->getPosition().X() - currentSector.getPoint(nEdge).X()) > 0) countX++;
            if ((aHit->getPosition().Y() - currentSector.getPoint(nEdge).Y()) > 0) countY++;
            if ((aHit->getPosition().Z() - currentSector.getPoint(nEdge).Z()) > 0) countZ++;

      if (aHit->getPosition().Y() == currentSector.getPoint(nEdge).Y()) {countY = 2; }

      B2DEBUG(100, "Edge: " << nEdge << " Position: " << currentSector.getPoint(nEdge).X() << "/" << currentSector.getPoint(nEdge).Y() << "/" << currentSector.getPoint(nEdge).Z());
          }

          if (countX != 2 or countY != 2 or countZ != 2) {
            B2ERROR("ERRORCASE: sector-edges of sector " << FullSecID(currentSectorID) << " do not match to hit-position! x/y/z: " << countX << "/" << countY << "/" << countZ);
          } else {
            B2WARNING("GOODCASE: sector-edges of sector " << FullSecID(currentSectorID) << " do match to hit-position! x/y/z: " << countX << "/" << countY << "/" << countZ);
          }

          TString tempString = currentSector.getDisplayInformation();
          B2DEBUG(100, "COR - SectorID: " << FullSecID(currentSectorID) << ": " << tempString);
        }
      }
    }
    */
    //TEMP

//     }
  }

  B2DEBUG(100, "RealHits: " << counterRealHits << "/ notRealHits: " << counterNotRealHits);

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
// 3  died_at   string
// 4  died_at_ID  int
// 5  real    int
// 6  MainParticleID  int
// 7  Purity    double
// 8  FilterID::distance3D,
// 9  FilterID::distanceZ  (add)
// 10 FilterID::anglesXY,
// 11 FilterID::anglesRZ,
// 12 FilterID::slopeRZ (add)
// 13 FilterID::distance2IP,
// 14 FilterID::deltaSlopeRZ,
// 15 FilterID::normedDistance3D (add)
// 16 FilterID::pT,
// 17 FilterID::helixParameterFit

//             FilterID::deltaSOverZ,
//             FilterID::deltaSlopeZOverS,

// 18 FilterID::nbFinderLost,
// 19 FilterID::cellularAutomaton,
// 20 FilterID::silentTcc

// neue filter
//                                   FilterID::angles3D,
//              FilterID::distanceXY,

// 21   outer hit
// 22   inner Hit




void AnalizerCollectorTFInfo::storeCellInformation(std::string filename, int particleIdFilter)
{
  // nbfinder, CA
  std::vector<int> searchfilters = { FilterID::distance3D,
                                     FilterID::distanceZ,
                                     FilterID::anglesXY,
                                     FilterID::anglesRZ,
                                     FilterID::slopeRZ,
                                     FilterID::distance2IP,
                                     FilterID::deltaSlopeRZ,
                                     FilterID::normedDistance3D,
                                     FilterID::pT,
                                     FilterID::helixParameterFit,
                                     FilterID::deltaSOverZ,
                                     FilterID::deltaSlopeZOverS,
                                     FilterID::nbFinderLost,
                                     FilterID::cellularAutomaton,
                                     FilterID::silentTcc,
                                     FilterID::angles3D,
                                     FilterID::distanceXY,
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


  myfile << "cellid" << m_fileSeparator << "passIndex" << m_fileSeparator << "state" << m_fileSeparator << "died_at" <<
         m_fileSeparator << "died_ID" << m_fileSeparator << "real" << m_fileSeparator << particleText << m_fileSeparator << "Purity" <<
         m_fileSeparator;

  myfile << FilterID::nameDistance3D << m_fileSeparator <<  FilterID::nameDistanceZ << m_fileSeparator << FilterID::nameAnglesXY <<
         m_fileSeparator << FilterID::nameAnglesRZ << m_fileSeparator << FilterID::nameSlopeRZ << m_fileSeparator <<
         FilterID::nameDistance2IP << m_fileSeparator << FilterID::nameDeltaSlopeRZ << m_fileSeparator << FilterID::nameNormedDistance3D <<
         m_fileSeparator << FilterID::namePT << m_fileSeparator << FilterID::nameHelixParameterFit;

  myfile << m_fileSeparator << FilterID::nameDeltaSOverZ << m_fileSeparator <<  FilterID::nameDeltaSlopeZOverS;

  myfile << m_fileSeparator << FilterID::nameNbFinderLost << m_fileSeparator <<  FilterID::nameCellularAutomaton << m_fileSeparator <<
         FilterID::nameSilentTcc;

  // new Filters
  myfile << m_fileSeparator << FilterID::nameAngles3D << m_fileSeparator << FilterID::nameDistanceXY;

  myfile << m_fileSeparator << "outer_Hit" << m_fileSeparator << "inner_Hit" << endl;

  for (int i = 0; i <  cellTFInfo.getEntries(); i++) {

    // particleIdFilter == -1 => all Entries
    if (particleIdFilter == -1 || cellTFInfo[i]->containsParticle(particleIdFilter)) {

      myfile << i << m_fileSeparator << cellTFInfo[i]->getPassIndex() << m_fileSeparator << cellTFInfo[i]->getState() << m_fileSeparator
             << cellTFInfo[i]->getDiedAt() << m_fileSeparator;

      // Died ID for output change
      if (cellTFInfo[i]->getDiedID() != m_idAlive) {
        myfile << cellTFInfo[i]->getDiedID();
      } else {
        myfile << m_idAlive_output;
      }

      myfile << m_fileSeparator << cellTFInfo[i]->getIsReal();

      // Main Particle & Purity
      if (particleIdFilter == -1) {
        myfile << m_fileSeparator << cellTFInfo[i]->getMainParticle().first << m_fileSeparator << cellTFInfo[i]->getMainParticle().second;
      } else {
        myfile << m_fileSeparator << cellTFInfo[i]->getInfoParticle(particleIdFilter).first << m_fileSeparator <<
               cellTFInfo[i]->getInfoParticle(particleIdFilter).second;
      }

      std::vector<int> accepted = cellTFInfo[i]->getAccepted();
      std::vector<int> rejected = cellTFInfo[i]->getRejected();

      B2DEBUG(100, "SaveCellInformation: i: " << i << ", pass index: " << cellTFInfo[i]->getPassIndex() << ", size accepted: " <<
              accepted.size() <<  ", size rejected: " << rejected.size() << ", sizeNeighbours: " << cellTFInfo[i]->sizeNeighbours());

      B2DEBUG(100, "SaveHitInformation: Real: " << cellTFInfo[i]->getIsReal() << ", Died At: " << cellTFInfo[i]->getDiedAt());


      // TESTS
      B2DEBUG(100, "FilterID::pT: " << FilterID::pT);

      for (auto& currentFilter : rejected) {
        if (FilterID::pT == currentFilter) {
          B2DEBUG(100, "rejected: " << currentFilter << ", Real: " << cellTFInfo[i]->getIsReal() << ", i: " << i << ", Died At: " <<
                  cellTFInfo[i]->getDiedAt());
        }
      }

      for (auto& currentFilter : accepted) {
        if (FilterID::pT == currentFilter) {
          B2DEBUG(100, "accepted: " << currentFilter << ", Real: " << cellTFInfo[i]->getIsReal() << ", i: " << i << ", Died At: " <<
                  cellTFInfo[i]->getDiedAt());
        }
      }


      for (uint u = 0; u < searchfilters.size(); u++) {
        //B2DEBUG(100,"Filter ?: " << searchfilters.at(u));
        myfile << m_fileSeparator;

        if (std::find(accepted.begin(), accepted.end(), searchfilters.at(u)) != accepted.end()) {
          myfile << "1";      //true = accepted
        } else if (std::find(rejected.begin(), rejected.end(), searchfilters.at(u)) != rejected.end()) {
          myfile << "0";      //false = rejected
        } else {

          // SilentKill sets the 1 here - only for better reading
          if (searchfilters.at(u) == FilterID::silentTcc) {
            myfile << "1";
          }

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

      for (auto& currentCoordinate : cellTFInfo[i]->getCoordinates()) {
        B2DEBUG(100, "Coor-Cell " << i << " Coordinate: " << currentCoordinate.X() << "/" << currentCoordinate.Y() << "/" <<
                currentCoordinate.Z());
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
// 3  died_at   string
// 4  died_at_id  int
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

  myfile << "tcid" << m_fileSeparator << "passIndex" << m_fileSeparator << "ownid" << m_fileSeparator << "died_at" << m_fileSeparator
         << "died_ID" << m_fileSeparator << "real" << m_fileSeparator << particleText <<
         m_fileSeparator << "Purity" << m_fileSeparator << FilterID::nameHopfield << m_fileSeparator << FilterID::nameGreedy <<
         m_fileSeparator << FilterID::nameTcDuel << m_fileSeparator << FilterID::nameTcFinderCurr << m_fileSeparator <<
         FilterID::nameZiggZaggXY << m_fileSeparator << FilterID::nameDeltapT << m_fileSeparator << FilterID::nameDeltaDistance2IP <<
         m_fileSeparator << FilterID::nameZiggZaggRZ << m_fileSeparator <<  FilterID::nameCalcQIbyKalman << m_fileSeparator <<
         FilterID::nameOverlapping << m_fileSeparator << FilterID::nameCirclefit << m_fileSeparator << "Count_Asso_Cell_IDs" <<
         m_fileSeparator << "Cell_ID_1" << m_fileSeparator << "Cell_ID_2" << m_fileSeparator << "Cell_ID_3" << m_fileSeparator << "Cell_ID_4"
         << m_fileSeparator << "Cell_ID_5" << m_fileSeparator << "Cell_ID_6" << m_fileSeparator
         << "Cell_ID_7" << m_fileSeparator << "Cell_ID_8" << m_fileSeparator << "Cell_ID_9" << m_fileSeparator << "Cell_ID_10" << endl;

  for (int i = 0; i <  tfcandTFInfo.getEntries(); i++) {

    // particleIdFilter == -1 => all Entries
    if (particleIdFilter == -1 || tfcandTFInfo[i]->containsParticle(particleIdFilter)) {

      myfile << i << m_fileSeparator << tfcandTFInfo[i]->getPassIndex() << m_fileSeparator << tfcandTFInfo[i]->getOwnID() <<
             m_fileSeparator << tfcandTFInfo[i]->getDiedAt() << m_fileSeparator;

      // Died ID for output change
      if (tfcandTFInfo[i]->getDiedID() != m_idAlive) {
        myfile << tfcandTFInfo[i]->getDiedID();
      } else {
        myfile << m_idAlive_output;
      }

      myfile << m_fileSeparator << tfcandTFInfo[i]->getIsReal();

      // Main Particle & Purity
      if (particleIdFilter == -1) {
        myfile << m_fileSeparator << tfcandTFInfo[i]->getMainParticle().first << m_fileSeparator <<
               tfcandTFInfo[i]->getMainParticle().second;
      } else {
        myfile << m_fileSeparator << tfcandTFInfo[i]->getInfoParticle(particleIdFilter).first << m_fileSeparator <<
               tfcandTFInfo[i]->getInfoParticle(particleIdFilter).second;
      }

      std::vector<int> accepted = tfcandTFInfo[i]->getAccepted();
      std::vector<int> rejected = tfcandTFInfo[i]->getRejected();

      B2DEBUG(100, "SaveTCInformation: i: " << i << ", pass index: " << tfcandTFInfo[i]->getPassIndex() << ", size accepted: " <<
              accepted.size() << ", size rejected: " << rejected.size());

      for (uint u = 0; u < searchfilters.size(); u++) {
        //B2DEBUG(100,"Filter ?: " << searchfilters.at(u));
        myfile << m_fileSeparator;

        if (std::find(accepted.begin(), accepted.end(), searchfilters.at(u)) != accepted.end()) {
          myfile << "1";      //true = accepted
        } else if (std::find(rejected.begin(), rejected.end(), searchfilters.at(u)) != rejected.end()) {
          myfile << "0";      //false = rejected
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

      for (auto& currentCoordinate : tfcandTFInfo[i]->getCoordinates()) {
        B2DEBUG(100, "Coor-TC " << i << " Coordinate: " << currentCoordinate.X() << "/" << currentCoordinate.Y() << "/" <<
                currentCoordinate.Z());
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
// 3  died_at     string
// 4  died_at_id    int
// 5  real      int
// 6  Detector Type   int
// 7  ParticleID    int

void AnalizerCollectorTFInfo::storeClustersInformation(std::string filename)
{

  ofstream myfile(filename);

  StoreArray<ClusterTFInfo> clusterTFInfo("");
  uint nCount = clusterTFInfo.getEntries();
  if (nCount == 0) {B2DEBUG(100, "AnalizerCollectorTFInfo: Display: clusterTFInfo is empty!");}

  myfile << "ClusterID" << m_fileSeparator << "passIndex" << m_fileSeparator << "Relative_Cluster_ID" << m_fileSeparator << "died_at"
         << m_fileSeparator << "died_ID" << m_fileSeparator << "real" << m_fileSeparator << "Detector_Type" << m_fileSeparator <<
         "Particle_ID" << m_fileSeparator << "PDG" << endl;

  for (int i = 0; i <  clusterTFInfo.getEntries(); i++) {

    myfile << i << m_fileSeparator << clusterTFInfo[i]->getPassIndex() << m_fileSeparator << clusterTFInfo[i]->getRelativeClusterID() <<
           m_fileSeparator << clusterTFInfo[i]->getDiedAt() << m_fileSeparator;

    // Died ID for output change
    if (clusterTFInfo[i]->getDiedID() != m_idAlive) {
      myfile << clusterTFInfo[i]->getDiedID();
    } else {
      myfile << m_idAlive_output;
    }

    myfile << m_fileSeparator << clusterTFInfo[i]->getIsReal() << m_fileSeparator <<  clusterTFInfo[i]->getDetectorType() <<
           m_fileSeparator << clusterTFInfo[i]->getParticleID() << m_fileSeparator << clusterTFInfo[i]->getPDG() << endl;

  }

  myfile.close();
}



/** Stores Sector Information in a file */
// Information of the Sector:
// 0  SectorID    uint
// 1  passIndex    int
// 2  Real Sector ID    int
// 3  died_at     string
// 4  died_at_id    int
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

  myfile << "SectorID" << m_fileSeparator << "passIndex" << m_fileSeparator << "Real_Sector_ID" << m_fileSeparator << "died_at" <<
         m_fileSeparator << "died_ID" << m_fileSeparator << "Friends_only" << m_fileSeparator << "Point_1" << m_fileSeparator << "Point_2" <<
         m_fileSeparator << "Point_3" << m_fileSeparator << "Point_4" << m_fileSeparator << "Friends_Sector_IDs";

  for (uint m = 0; m < 15; m++) {
    myfile << m_fileSeparator << "Sector_ID_" << (m + 1);
  }

  myfile << endl;

  for (int i = 0; i <  sectorTFInfo.getEntries(); i++) {

    if (withFriends || (!sectorTFInfo[i]->getIsOnlyFriend())) {

      myfile << i << m_fileSeparator << sectorTFInfo[i]->getPassIndex() << m_fileSeparator << sectorTFInfo[i]->getSectorID() <<
             m_fileSeparator << sectorTFInfo[i]->getDiedAt() << m_fileSeparator;

      // Died ID for output change
      if (sectorTFInfo[i]->getDiedID() != m_idAlive) {
        myfile << sectorTFInfo[i]->getDiedID();
      } else {
        myfile << m_idAlive_output;
      }

      myfile << m_fileSeparator << sectorTFInfo[i]->getIsOnlyFriend() << m_fileSeparator <<  sectorTFInfo[i]->getPoint(
               0).X() << "/" << sectorTFInfo[i]->getPoint(0).Y() << "/" << sectorTFInfo[i]->getPoint(0).Z() << m_fileSeparator <<
             sectorTFInfo[i]->getPoint(1).X() << "/" << sectorTFInfo[i]->getPoint(1).Y() << "/" << sectorTFInfo[i]->getPoint(
               1).Z() << m_fileSeparator <<  sectorTFInfo[i]->getPoint(2).X() << "/" << sectorTFInfo[i]->getPoint(
               2).Y() << "/" << sectorTFInfo[i]->getPoint(2).Z() << m_fileSeparator << sectorTFInfo[i]->getPoint(
               3).X() << "/" << sectorTFInfo[i]->getPoint(3).Y() << "/" << sectorTFInfo[i]->getPoint(3).Z();

      std::vector<unsigned int> friendSectors = sectorTFInfo[i]->getFriends();

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

      for (auto& currentCoordinate : sectorTFInfo[i]->getCoordinates()) {
        B2DEBUG(100, "Coor-Sector " << i << " Coordinate: " << currentCoordinate.X() << "/" << currentCoordinate.Y() << "/" <<
                currentCoordinate.Z());
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

  for (auto& currentTC : tfcandTFInfo) {

    //Search for connected TrackCandidate
    if (currentTC.getAssignedGFTC() == gfIndex) {
      tcIDCurrent = currentTC.getOwnID();
      continue;
    }
  }

  return tcIDCurrent;

}





