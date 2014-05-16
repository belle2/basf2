/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Ferstl                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/CollectorTFInfo.h>

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

#include "tracking/dataobjects/FilterID.h"
#include "tracking/dataobjects/FullSecID.h"
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/sectorMapTools/SectorTools.h>


//C++ std lib
#include <utility>
// #include <string>
#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;
using namespace Belle2;

const string CollectorTFInfo::m_nameHitFinder = "HitFinder";
const string CollectorTFInfo::m_nameCellFinder = "CellFinder";
const string CollectorTFInfo::m_nameNbFinder = "NbFinder";
const string CollectorTFInfo::m_nameCA = "CA";
const string CollectorTFInfo::m_nameTCC = "TCC";
const string CollectorTFInfo::m_nameOverlap = "Overlap";
const string CollectorTFInfo::m_nameQI = "QI";
const string CollectorTFInfo::m_nameHopfield = "Hopfield";

const int CollectorTFInfo::m_idHitFinder = 0;
const int CollectorTFInfo::m_idCellFinder = 1;
const int CollectorTFInfo::m_idNbFinder = 2;
const int CollectorTFInfo::m_idCA = 3;
const int CollectorTFInfo::m_idTCC = 4;
const int CollectorTFInfo::m_idOverlap = 6;
const int CollectorTFInfo::m_idQI = 5;
const int CollectorTFInfo::m_idHopfield = 7;
const int CollectorTFInfo::m_idAlive = -1;

/**
 * Information about the Methodes in CollectorTFInfo.h
 */

ClassImp(CollectorTFInfo)


CollectorTFInfo::CollectorTFInfo()
{
  //m_outputFlag = 0;

}

CollectorTFInfo::~CollectorTFInfo()
{
}

// Run at End of beginn-Run / 1 x RUN
/** Sectors safe for all events */
void CollectorTFInfo::initSectors(const std::vector< std::pair <std::pair<unsigned int, unsigned int>, std::vector<unsigned int> > >& sectors, const std::vector<double>& secConfigU, const std::vector<double>& secConfigV)
{

  B2DEBUG(100, "CollectorTFInfo: initSectors");

  // Parameters to get the different corners
  std::pair<double, double> aRelCoor_corner1 = {0, 0};
  std::pair<double, double> aRelCoor_corner2 = {0, 1};
  std::pair<double, double> aRelCoor_corner3 = {1, 0};
  std::pair<double, double> aRelCoor_corner4 = {1, 1};

  SectorTools aTool = SectorTools();

  int sectorSize = sectors.size();

  B2DEBUG(100, "CollectorTFInfo: initSectors, sectorSize: " << sectorSize);

  //in loop all sectors are initialized and their coordinates are set
  for (auto & currentSector : sectors)  {

    // NO Sector 0 => CHANGE with the new Secmap !!!
    if (currentSector.first.second == 0) { continue; }

    SectorTFInfo newsector(currentSector.first.first, currentSector.first.second);

    // Friends store
    newsector.setAllFriends(currentSector.second);

    B2DEBUG(100, "CollectorTFInfo: initSectors, currentSector.first.second: " << currentSector.first.second);


    // SectorID => Corners of the Sector
    FullSecID currentFullSectorID = FullSecID(currentSector.first.second);

    B2DEBUG(100, "CollectorTFInfo: initSectors, search for: " << currentFullSectorID);

    VxdID aVxdID = currentFullSectorID.getVxdID();
    unsigned short aSecID = currentFullSectorID.getSecID();

    B2DEBUG(100, "CollectorTFInfo: initSectors, aSecID: " << aSecID);

    std::pair<float, float> aRelCoor; // reusage of temporal value
//     // 1. Corner Calculate
//     std::pair<float, float> aRelCoor = aTool.calcNormalizedSectorPoint(secConfigU, secConfigV, aSecID, aRelCoor_corner1);
//
// //     B2DEBUG(100, "CollectorTFInfo: initSectors, calcNormalizedSectorPoints: " << aSecID);
//
//     aRelCoor = SpacePoint::convertToLocalCoordinatesNormalized(aRelCoor, aVxdID);
//
// //     B2DEBUG(100, "CollectorTFInfo: initSectors, convertToLocalCoordinatesNormalized: " << aSecID);
//
//     TVector3 corner1Global = SpacePoint::getGlobalCoordinates(aRelCoor, aVxdID);
//
// //     B2DEBUG(100, "CollectorTFInfo: initSectors, getGlobalCoordinates: " << aSecID);
//
//     newsector.setPoint(0, corner1Global);
//
//     // 2. Corner Calculate
//     aRelCoor = aTool.calcNormalizedSectorPoint(secConfigU, secConfigV, aSecID, aRelCoor_corner2);
//     aRelCoor = SpacePoint::convertToLocalCoordinatesNormalized(aRelCoor, aVxdID);
//     TVector3 corner2Global = SpacePoint::getGlobalCoordinates(aRelCoor, aVxdID);
//     newsector.setPoint(1, corner2Global);
//
//     // 3. Corner Calculate
//     aRelCoor = aTool.calcNormalizedSectorPoint(secConfigU, secConfigV, aSecID, aRelCoor_corner3);
//     aRelCoor = SpacePoint::convertToLocalCoordinatesNormalized(aRelCoor, aVxdID);
//     TVector3 corner3Global = SpacePoint::getGlobalCoordinates(aRelCoor, aVxdID);
//     newsector.setPoint(2, corner3Global);
//
//     // 4. Corner Calculate
//     aRelCoor = aTool.calcNormalizedSectorPoint(secConfigU, secConfigV, aSecID, aRelCoor_corner4);
//     aRelCoor = SpacePoint::convertToLocalCoordinatesNormalized(aRelCoor, aVxdID);
//
    // 1. Corner Calculate
    aRelCoor = SectorTools::calcNormalizedSectorPoint(secConfigU, secConfigV, aSecID, aRelCoor_corner1);
    pair<double, double> localCorner00 = SpacePoint::convertNormalizedToLocalCoordinates(aRelCoor, aSecID);
    TVector3 corner1Global = SpacePoint::getGlobalCoordinates(localCorner00, aVxdID);
    newsector.setPoint(0, corner1Global);

    // 2. Corner Calculate
    aRelCoor = SectorTools::calcNormalizedSectorPoint(secConfigU, secConfigV, aSecID, aRelCoor_corner2);
    pair<double, double> localCorner01 = SpacePoint::convertNormalizedToLocalCoordinates(aRelCoor, aVxdID);
    TVector3 corner2Global = SpacePoint::getGlobalCoordinates(localCorner01, aVxdID);
    newsector.setPoint(1, corner2Global);

    // 3. Corner Calculate
    aRelCoor = SectorTools::calcNormalizedSectorPoint(secConfigU, secConfigV, aSecID, aRelCoor_corner3);
    pair<double, double> localCorner10 = SpacePoint::convertNormalizedToLocalCoordinates(aRelCoor, aVxdID);
    TVector3 corner3Global = SpacePoint::getGlobalCoordinates(localCorner10, aVxdID);
    newsector.setPoint(2, corner3Global);

    // 4. Corner Calculate
    aRelCoor = SectorTools::calcNormalizedSectorPoint(secConfigU, secConfigV, aSecID, aRelCoor_corner4);
    pair<double, double> localCorner11 = SpacePoint::convertNormalizedToLocalCoordinates(aRelCoor, aVxdID);
    TVector3 corner4Global = SpacePoint::getGlobalCoordinates(localCorner11, aVxdID);
    newsector.setPoint(3, corner4Global);


    B2DEBUG(100, "CollectorTFInfo: initSectors, all Corners set: " << aSecID);

    // OLD CALCULATION
    /*
    double vSize1 = 0.5 * aSensorInfo.getVSize();

    for (uint j = 0; j != secConfigU.size() - 1; ++j) {
      for (uint k = 0; k != secConfigV.size() - 1; ++k) {
    B2DEBUG(100, "vor secid : " << aSecID);
        aSecID = k + 1 + j * (secConfigV.size() - 1);
    B2DEBUG(100, "vor secid : " << aSecID);

        if (aSecID != uint(currentFullSectorID.getSecID())) { continue; }

        B2DEBUG(100, "initSectors aSecID: " << aSecID << ", k: " << k << ", j: " << j << ", vxdid: " << currentFullSectorID.getVxdID());

        sectorEdgeV1 = secConfigV.at(k) * vSize1 * 2 - vSize1;
        sectorEdgeV2 = secConfigV.at(k + 1) * vSize1 * 2 - vSize1;
        uSizeAtv1 = 0.5 * aSensorInfo.getUSize(sectorEdgeV1);
        uSizeAtv2 = 0.5 * aSensorInfo.getUSize(sectorEdgeV2);
        sectorEdgeU1OfV1 = secConfigU.at(j) * uSizeAtv1 * 2 - uSizeAtv1;
        sectorEdgeU1OfV2 = secConfigU.at(j) * uSizeAtv2 * 2 - uSizeAtv2;
        sectorEdgeU2OfV1 = secConfigU.at(j + 1) * uSizeAtv1 * 2 - uSizeAtv1;
        sectorEdgeU2OfV2 = secConfigU.at(j + 1) * uSizeAtv2 * 2 - uSizeAtv2;

        // sectorCorners
        TVector3 corner1Local = TVector3(sectorEdgeV1, sectorEdgeU1OfV1, 0);
        TVector3 corner2Local = TVector3(sectorEdgeV1, sectorEdgeU2OfV1, 0);
        TVector3 corner3Local = TVector3(sectorEdgeV2, sectorEdgeU1OfV2, 0);
        TVector3 corner4Local = TVector3(sectorEdgeV2, sectorEdgeU2OfV2, 0);

        TVector3 corner1Global = aSensorInfo.pointToGlobal(corner1Local);
        TVector3 corner2Global = aSensorInfo.pointToGlobal(corner2Local);
        TVector3 corner3Global = aSensorInfo.pointToGlobal(corner3Local);
        TVector3 corner4Global = aSensorInfo.pointToGlobal(corner4Local);

        newsector.setPoint(0, corner1Global);
        newsector.setPoint(1, corner2Global);
        newsector.setPoint(2, corner3Global);
        newsector.setPoint(3, corner4Global);

        break;
      }
    }
    */
//   B2DEBUG(100, "Sector von init: " << newsector.getDisplayInformation());

    m_sectorTFAll.insert(make_pair(KeySectors(currentSector.first.first, currentSector.first.second), newsector));

  }

}


// Persistence 1 x initialize
/** registerPersistence (StoreArrays & RelationArray) for the Collector */
void CollectorTFInfo::initPersistent()
{
  B2DEBUG(100, "CollectorTFInfo: initPersistent");

  // Store Arrays for output
  StoreArray<ClusterTFInfo>::registerPersistent();
  StoreArray<SectorTFInfo>::registerPersistent();
  StoreArray<HitTFInfo>::registerPersistent();
  StoreArray<CellTFInfo>::registerPersistent();
  StoreArray<TrackCandidateTFInfo>::registerPersistent();

  RelationArray::registerPersistent<SectorTFInfo, SectorTFInfo>("", "");
  RelationArray::registerPersistent<CellTFInfo, CellTFInfo>("", "");
}


// Run the_real_event => 1x per event
/** Clear all vectors */
void CollectorTFInfo::intEvent()  // Clear Vectors
{
  B2DEBUG(100, "CollectorTFInfo: intEvent");

  // Clear for new event
  m_clustersTF.clear();
  m_sectorTF.clear();
  m_hitTF.clear();
  m_cellTF.clear();
  m_tfCandTF.clear();

}


/** Import of a cluster */
int CollectorTFInfo::importCluster(int passIndex, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, int detectorType, int relativePosition)
{
  B2DEBUG(100, "CollectorTFInfo: importCluster, passIndex: " << passIndex << ", diet_at: " << diedAt << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", Detector Type: " << detectorType);

  int clusterId = m_clustersTF.size();

  ClusterTFInfo newcluster(passIndex, clusterId, detectorType);

  newcluster.setUseCounter(0);

  newcluster.setDiedAt(diedAt);
  newcluster.setDiedID(diedId);

  newcluster.insert_Accepted(accepted);
  newcluster.insert_Rejected(rejected);

  newcluster.setRelativeClusterID(relativePosition);

  m_clustersTF.push_back(newcluster);

  // B2DEBUG(100,"CollectorTFInfo: importCluster, startPositionID: " << newcluster.getStartPositionID() );
  B2DEBUG(100, "CollectorTFInfo: importCluster ClusterID: " << clusterId);


  return clusterId;
}



/** Sectors update after update / import Hit
 Reload Sector if not in current Sectors
 Also checks Friends sectors (possible reload) */
void CollectorTFInfo::updateSectors(unsigned int sectorID, int passIndex, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter)
{
  B2DEBUG(100, "CollectorTFInfo: updateSectors, sectorID: " << sectorID << ", Pass Index: " << passIndex << ", diet_at: " << diedAt << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", deltaUseCounter: " << deltaUseCounter);

  //std::map<KeySectors, SectorTFInfo>::iterator it, itFriend;

  // Key of the current Sector
  KeySectors myKey(passIndex, sectorID);

  auto itCurrentSector = m_sectorTF.find(myKey);

  // Sector Search in current Sectors
  if (itCurrentSector == m_sectorTF.end()) {
    // Not found => Search in all Sectors
    itCurrentSector = m_sectorTFAll.find(myKey);

    if (itCurrentSector == m_sectorTFAll.end()) {
      // Not found in all Sectors
      B2DEBUG(100, "CollectorTFInfo: updateSectors - Sector not found !");
      return;

    } else {

      // Found => Sector reload
      B2DEBUG(100, "Sector reload: sectorID: " << sectorID);

      // + Updates
      //Copy of sector => use for this event
      SectorTFInfo currentSector = itCurrentSector->second;
      currentSector.setIsOnlyFriend(false);

      currentSector.changeUseCounter(deltaUseCounter);

      // DiedAt = add to string
      if (diedAt.size() != 0 && currentSector.getUseCounter() == 0) {
        currentSector.setDiedAt(diedAt);
        currentSector.setDiedID(diedId);
      }

      currentSector.insert_Accepted(accepted);
      currentSector.insert_Rejected(rejected);

      B2DEBUG(100, "CollectorTFInfo: updateSectors - akt deltaUseCounter : " << currentSector.getUseCounter());

      m_sectorTF.insert(make_pair(myKey, currentSector));

      // Reload all friends of current Sector
      for (auto & currentFriendSector : currentSector.getFriends()) {

        // Key of Friend Sector
        KeySectors friendKey(passIndex, currentFriendSector);

        // Search in current Sectors (if loaded => nothing)
        if (m_sectorTF.find(friendKey) ==  m_sectorTF.end()) {

          // not found => search in all Sectors
          auto itFriend = m_sectorTFAll.find(friendKey);

          // Reload Friend Sector
          if (itFriend == m_sectorTFAll.end()) {
            // Sector not found
            B2DEBUG(100, "NIX DA Sector-Friend");
          } else {

            B2DEBUG(100, "Sector-Friend reload: sectorID: " << currentFriendSector);

            //Copy of sector => use for this event
            SectorTFInfo currentFriendSectorObj = itFriend->second;

            // Friend reload => not active
            currentFriendSectorObj.setIsOnlyFriend(true);
            m_sectorTF.insert(make_pair(friendKey, currentFriendSectorObj));

          }
        }
      }

    }

  } else {

    // Reference to change Sector Information
    SectorTFInfo& currentSector = itCurrentSector->second;

    //in Case it only was a sector friend before
    currentSector.setIsOnlyFriend(false);

    // Only update active Friends ???

    currentSector.changeUseCounter(deltaUseCounter);

    // DiedAt = add to string
    if (diedAt.size() != 0 && currentSector.getUseCounter() == 0) {
      currentSector.setDiedAt(diedAt);
      currentSector.setDiedID(diedId);
    }

    currentSector.insert_Accepted(accepted);
    currentSector.insert_Rejected(rejected);

    B2DEBUG(100, "CollectorTFInfo: updateSectors - akt deltaUseCounter : " << currentSector.getUseCounter());

  }


}


/** Cluster Update */
void CollectorTFInfo::updateClusters(int clusterId, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter)   //Cluster Update
{
  B2DEBUG(100, "CollectorTFInfo: updateClusters, clusterId: " << clusterId << ", diet_at: " << diedAt << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", deltaUseCounter: " << deltaUseCounter);

  if (clusterId >= int(m_clustersTF.size())) {
    B2DEBUG(100, "CollectorTFInfo: updateClusters - Cluster not found !");
    return;
  } else {

    //Reference => change cluster
    ClusterTFInfo& currentCluster = m_clustersTF.at(clusterId);

    currentCluster.changeUseCounter(deltaUseCounter);

    // DiedAt = add to string
    if (diedAt.size() != 0 && currentCluster.getUseCounter() <= 0) {
      currentCluster.setDiedAt(diedAt);
      currentCluster.setDiedID(diedId);
    }

    currentCluster.insert_Accepted(accepted);
    currentCluster.insert_Rejected(rejected);

    B2DEBUG(100, "CollectorTFInfo: updateClusters - akt deltaUseCounter : " << currentCluster.getUseCounter());
  }



}



/** Import Hit (return hitID) */
int CollectorTFInfo::importHit(int passIndex, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assignedClusterIDs, int secId, TVector3 hitPosition, TVector3 hitSigma)  // Hit import
{
  B2DEBUG(100, "CollectorTFInfo: importHit, passIndex: " << passIndex << ", diet_at: " << diedAt << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", secId: " << secId);

  int hitId = m_hitTF.size();

  HitTFInfo newhit(passIndex, secId, hitPosition, hitSigma);

  //newhit.setUseCounter(0);

  newhit.setDiedAt(diedAt);
  newhit.setDiedID(diedId);

  newhit.insert_Accepted(accepted);
  newhit.insert_Rejected(rejected);

  // 1 = deltaUseCounter, +1 use
  updateSectors(secId, passIndex, diedAt, diedId, accepted, rejected, 1);

  for (auto & currentId : assignedClusterIDs) {

    newhit.push_back_AssignedCluster(currentId);

    // 1 = delausecounter, +1 use
    updateClusters(currentId, diedAt, diedId, accepted, rejected, 1);

    // take NO PARTICLE ID from Relations !!!  => moved to TFAnalizerModule
    //  => disabled
    // newhit.setParticleID(getParticleIDfromRelations(passIndex, currentId));
  }

  m_hitTF.push_back(newhit);

  return hitId;
}


/** Hit Update */
// addTCID, removeTCID if -1 => no deltaUseCounter
void CollectorTFInfo::updateHit(int hitId, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, int addTCID, int removeTCID, std::vector<int> deltaUseCounterCell)
{
  B2DEBUG(100, "CollectorTFInfo: updateHit, hitId: " << hitId << ", diet_at: " << diedAt << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", deltaUseCounterCell - size: " << deltaUseCounterCell.size());

  if (hitId >= int(m_hitTF.size()) || hitId == -1) {
    B2DEBUG(100, "CollectorTFInfo: updateHit - Hit not found !");
    return;
  } else {

    //Reference => change hit
    HitTFInfo& currentHit = m_hitTF.at(hitId);

    int deltaUseCounter = currentHit.changeUseCounterCell(deltaUseCounterCell);

    // Changed Cells = deltaUseCounter

    currentHit.changeUseCounter(deltaUseCounter);

    //B2DEBUG(100,"CollectorTFInfo: akt deltaUseCounter: " << deltaUseCounter);

    string diedAtCurrent;
    int diedIdCurrent = 0;

    int deltaUseHit = 0;

    // DiedAt = add to string
    if (currentHit.getUseCounter() == 0 && currentHit.getActive()) {
      diedAtCurrent = diedAt;
      diedIdCurrent = diedId;

      currentHit.setDiedAt(diedAtCurrent);
      currentHit.setDiedID(diedIdCurrent);

      // Delta Use Count for Sectors & Clusters
      if (diedAt.size() != 0) {
        deltaUseHit = -1;
      }
    }

    for (auto & currentCluster : currentHit.getAssignedCluster()) {
      // 0 = only update for cluster (already used by this hit)
      updateClusters(currentCluster, diedAtCurrent, diedIdCurrent, accepted, rejected, deltaUseHit);
    }

    // 0 = only update for sector (already used by this hit)
    updateSectors(currentHit.getSectorID(), currentHit.getPassIndex(), diedAtCurrent, diedIdCurrent, accepted, rejected, deltaUseHit);

    //Only Add TCID to List if not already in it
    if (addTCID != -1) {
      currentHit.push_back_UseCounterTCIDs(addTCID);
      currentHit.push_back_AllCounterTCIDs(addTCID);
    }

    if (removeTCID != -1) {
      currentHit.remove_UseCounterTCIDs(removeTCID);
    }

    B2DEBUG(100, "CollectorTFInfo: INFO Hit id " << hitId << ", akt UseCounter: " << currentHit.getUseCounter() << ", died at: " << diedAt);



    currentHit.insert_Accepted(accepted);
    currentHit.insert_Rejected(rejected);
  }

}



/** Cell Import */
int CollectorTFInfo::importCell(int passIndex, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assignedHitIDs)
{
  B2DEBUG(100, "CollectorTFInfo: importCell, passIndex: " << passIndex << ", diet_at: " << diedAt << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", assignedHitIDs - size: " << assignedHitIDs.size());

  int cellID = m_cellTF.size();

  CellTFInfo newcell(passIndex);

  newcell.setUseCounter(0);

  newcell.setDiedAt(diedAt);
  newcell.setDiedID(diedId);

  newcell.insert_Accepted(accepted);
  newcell.insert_Rejected(rejected);

  // standard = 2 cells, but so its varible
  std::vector<int> cellUpdate(assignedHitIDs.size());

  for (auto & indexCell : cellUpdate) {
    indexCell = 0;
  }

  // standard = 2 Hits
  for (auto & currentHit : assignedHitIDs) {
    // outer hit  = Index 0, inner hit = Index 1
    // Index = Posistion of currentHit
    cellUpdate.at(&currentHit - &assignedHitIDs.at(0)) = 1;

    // no ID from TC => -1; deltaUseCounter for cell = 1
    updateHit(currentHit, diedAt, diedId, accepted, rejected, -1, -1, cellUpdate);

    // Index = Posistion of currentHit
    cellUpdate.at((&currentHit - &assignedHitIDs.at(0))) = 0;

    // Assigned Hits store
    // Coordinates for assigned Hits store
    if (currentHit < int(m_hitTF.size()) && currentHit != -1) {
      newcell.push_back_AssignedHits(currentHit, m_hitTF[currentHit].getPosition());
    }

  }

  m_cellTF.push_back(newcell);

  return cellID;
}





/** Update Cell */
void CollectorTFInfo::updateCell(int cellID, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, int addTCID, int removeTCID, int cellstate, std::vector<int> neighbours)
{
  B2DEBUG(100, "CollectorTFInfo: updateCell, cellID: " << cellID << ", diet_at: " << diedAt << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", addTCID: " << addTCID << ", removeTCID: " << removeTCID << ", diedId: " << diedId);

  if (cellID >= int(m_cellTF.size()) || cellID == -1) {
    B2DEBUG(100, "CollectorTFInfo: updateCell - cell not found !");
    return;

  } else {
    // Reference to change Cell
    CellTFInfo& currentCell = m_cellTF.at(cellID);

    // set State (if cellstate = -1 => no change in CellState)
    if (cellstate != -1) {
      currentCell.setState(cellstate);
    }

    // If the from the nb-Finder: Add neighbours
    if (neighbours.size() > 0) {
      currentCell.insert_Neighbours(neighbours);
    }

    currentCell.insert_Accepted(accepted);
    currentCell.insert_Rejected(rejected);

    int deltaUseCounter = 0;

    // addTCID/removeTCID if -1 => no deltaUseCounter
    if (addTCID != -1) {
      deltaUseCounter++;
    }

    if (removeTCID != -1) {
      deltaUseCounter--;
    }

    currentCell.changeUseCounter(deltaUseCounter);

    std::string currentDiedAt;

    // DiedAt = add to string
    // removeTCID == -1 => died from Cell - Update
    // remove TCID == -2 => silent Kill
    // removeTCID != -1 && currentCell.getUseCounter() == 0 => died from TC - Update
    if (removeTCID == -1 || removeTCID == -2  || (removeTCID != -1 && currentCell.getUseCounter() == 0)) {
      currentDiedAt = diedAt;
      if (diedId != -1) {    currentCell.setDiedID(diedId);  }
    }

    currentCell.setDiedAt(currentDiedAt);





    // standard = 2 cells, but so its varible
    std::vector<int> cellUpdate(currentCell.getAssignedHits().size());

    for (auto & indexCell : cellUpdate) {
      indexCell = 0;
    }

    // TC add => no Change in Counter for Hits
    // remove TC ID ????
    if (addTCID != -1) {
      deltaUseCounter = 0;
    }

    for (auto & currentHit : currentCell.getAssignedHits()) {

      // outer hit  = Index 0, inner hit = Index 1
      // Index = Posistion of currentHit
      cellUpdate.at(&currentHit - &currentCell.getAssignedHits().at(0)) = deltaUseCounter;

      // outer hit  = Index 0, inner hit = Index 1
      updateHit(currentHit, currentDiedAt, diedId, accepted, rejected, addTCID, removeTCID, cellUpdate);

      // Index = Posistion of currentHit
      cellUpdate.at(&currentHit - &currentCell.getAssignedHits().at(0)) = 0;


    }

  }
}


/** TC Import, return = tc id */
int CollectorTFInfo::importTC(int passIndex, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, const std::vector<std::pair<int, unsigned int>> assignedCellIDs)
{
  B2DEBUG(100, "CollectorTFInfo: importTC, passIndex: " << passIndex << ", diet_at: " << diedAt << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", assignedCellIDs - size: " << assignedCellIDs.size());

  int tfcandID = m_tfCandTF.size();

  TrackCandidateTFInfo newtf(passIndex, tfcandID);

  newtf.setDiedAt(diedAt);
  newtf.setDiedID(diedId);

  newtf.insert_Accepted(accepted);
  newtf.insert_Rejected(rejected);

  for (auto & currentCell : assignedCellIDs) {

    // Assigned Cell store
    // Coordinates for assigned Cells store
    if (currentCell.first < int(m_cellTF.size()) && currentCell.first != -1) {
      newtf.push_back_AssignedCell(currentCell.first, m_cellTF[currentCell.first].getCoordinates());
    }

    // vector<int>() = no new neighbours
    updateCell(currentCell.first, diedAt, diedId, accepted, rejected, tfcandID, -1, currentCell.second, vector<int>());   // ID vom TCCAND.-Filter fehlt
  }

  m_tfCandTF.push_back(newtf);

  return tfcandID;
}


/** update TCand */
void CollectorTFInfo::updateTC(int tcid, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected)  // TC Update
{
  B2DEBUG(100, "CollectorTFInfo: updateTC, tcid: " << tcid << ", diet_at: " << diedAt << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size());

  if (tcid >= int(m_tfCandTF.size()) || tcid == -1) {
    B2DEBUG(100, "CollectorTFInfo: updateTC - TCCand not found !");
    return;

  } else {
    // Reference to change TFCand
    TrackCandidateTFInfo& currentTfCand = m_tfCandTF.at(tcid);

    // DiedAt = add to string
    currentTfCand.setDiedAt(diedAt);
    currentTfCand.setDiedID(diedId);
    currentTfCand.insert_Accepted(accepted);
    currentTfCand.insert_Rejected(rejected);

    int removeTCID = -1;
    if (diedAt.size() > 0) {
      removeTCID = tcid;
    }

    for (auto & currentCell : currentTfCand.getAssignedCell()) {
      // vector<int>() = no new neighbours
      // -1 => only update of the TC so no TC-ID needed
      updateCell(currentCell, diedAt, diedId, accepted, rejected, -1, removeTCID, -1, vector<int>());
    }


  }

}


/** Update Fit Information (fitSuccessful, probabilityValue, assignedGTFC) of TC */
void CollectorTFInfo::updateTCFitInformation(int tcid, bool fitSuccessful, double probabilityValue, int assignedGTFC)  // TC Update
{
  B2DEBUG(100, "CollectorTFInfo: updateTCFitInformation, tcid: " << tcid << ", fitSuccessful: " << fitSuccessful << ", probabilityValue: " << probabilityValue << ", assignedGTFC: " << assignedGTFC);

  if (tcid >= int(m_tfCandTF.size()) || tcid < 0) {
    B2DEBUG(100, "CollectorTFInfo: updateTCFitInformation - TCCand not found !: " << tcid);
    return;

  } else {
    // Reference to change TFCand
    TrackCandidateTFInfo& currentTfCand = m_tfCandTF.at(tcid);

    currentTfCand.fitIsPossible(fitSuccessful);
    currentTfCand.setProbValue(probabilityValue);
    currentTfCand.assignGFTC(assignedGTFC);

  }

}






/** Information stored in StoreArrays and RelationArrays */
void CollectorTFInfo::safeInformation()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);

  B2DEBUG(100, "*******  CollectorTFInfo processing event number: " << eventMetaDataPtr->getEvent() << " *******");

  // Create Store Arrays for Output

  // CLUSTERS
  StoreArray<ClusterTFInfo> clusterTFInfo("");
  if (!clusterTFInfo.isValid())  clusterTFInfo.create();

  for (auto & currentCluster : m_clustersTF) {
    clusterTFInfo.appendNew(currentCluster);
  }

  B2DEBUG(100, "*******  Cluster size: " << clusterTFInfo.getEntries() << " *******");
  B2DEBUG(100, "*******  akt Cluster size: " << m_clustersTF.size() << " *******");

  // SECTORS
  StoreArray<SectorTFInfo> sectorTFInfo("");
  if (sectorTFInfo.isValid() == false) { sectorTFInfo.create(); }


  // Friends of Sectors
  RelationArray relSectorSectorFriend(sectorTFInfo, sectorTFInfo);

  //std::map<KeySectors, SectorTFInfo>::iterator it;
  KeySectors friendKey;

  for (auto & currentSector : m_sectorTF) {
    int indexSector = sectorTFInfo.getEntries();

    sectorTFInfo.appendNew(currentSector.second);


    for (auto & currentFriend : currentSector.second.getFriends()) {

      // Pass_id
      friendKey.first = currentSector.second.getPassIndex();
      friendKey.second = currentFriend;

      auto itCurrentSector = m_sectorTF.find(friendKey);

      // Sector Search in current Sectors
      if (itCurrentSector != m_sectorTF.end()) {
        relSectorSectorFriend.add(indexSector, std::distance(m_sectorTF.begin(), itCurrentSector));
        B2DEBUG(100, "passIndex: " << friendKey.first << "; Sector akt: " << indexSector << ", friend sector: " << std::distance(m_sectorTF.begin(), itCurrentSector) << ", sector id (akt/friend): " << currentSector.second.getSectorID() << " / " << currentFriend);
      } else {
        B2DEBUG(100, "Friend-Sector not found: " << currentFriend);
      }

      // same with sectorid; IDs = SectorIDs for akt. Sector and Friend sectors
      //relSectorSectorFriend.add (currentSector.second.getSectorID(), currentFriend);
    }
  }

  B2DEBUG(100, "*******  SECTORS size: " << sectorTFInfo.getEntries() << " *******");
  B2DEBUG(100, "*******  akt SECTORS size: " << m_sectorTF.size() << " *******");

  B2DEBUG(100, "*******  relSectorSectorFriend: " << relSectorSectorFriend.getEntries() << " *******");


  // HITS
  StoreArray<HitTFInfo> hitTFInfo("");
  if (!hitTFInfo.isValid())  hitTFInfo.create();

  for (auto & currentHit : m_hitTF) {
    hitTFInfo.appendNew(currentHit);
  }

  // Cells
  StoreArray<CellTFInfo> cellTFInfo("");
  if (!cellTFInfo.isValid())  cellTFInfo.create();

  RelationArray relCellNBCell(cellTFInfo, cellTFInfo);

  for (auto & currentCell : m_cellTF) {
    int indexCell = cellTFInfo.getEntries();

    cellTFInfo.appendNew(currentCell);

    // ID of cell/neighbours = Index of cell
    for (auto & currentNb : currentCell.getNeighbours()) {
      relCellNBCell.add(indexCell, currentNb);
      B2DEBUG(100, "*******  relCellNBCell: indexCell: " << indexCell << ", currentNb: " << currentNb);
    }
  }

  B2DEBUG(100, "*******  relCellNBCell: " << relCellNBCell.getEntries() << " *******");

  // TFCAND
  StoreArray<TrackCandidateTFInfo> tfcandTFInfo("");
  if (!tfcandTFInfo.isValid())  tfcandTFInfo.create();

  for (auto & currentTfCand : m_tfCandTF) {
    tfcandTFInfo.appendNew(currentTfCand);
  }

}




/** Silent Kill = Checks if there are not connected Clusters, Cells and Hits and deactivates them */
// Deleted where:
//  Clusters = im Hitfinder
//  Cell = TCC
//  Hit = Segfinder

void CollectorTFInfo::silentKill()  // Für alle folgenden Updates
{

  B2DEBUG(100, "Start Silent kill");


  B2DEBUG(100, "Searching for dead clusters");
  // Searching for dead clusters
  // only if Hits and Clusters are already loaded
  if (m_hitTF.size() > 0 && m_clustersTF.size() > 0) {

    std::vector<bool> checkClusters(m_clustersTF.size(), true);

    // Set if the clusters are active / or not active
    for (uint i = 0; i < m_clustersTF.size(); i++) {
      checkClusters.at(i) = m_clustersTF.at(i).getActive();
    }

    // Search for Hits not used
    // => every hit used; check = false
    for (auto & currentHit : m_hitTF) {

      // Only check active Cells, other Hits should already been deactivated
      if (currentHit.getActive()) {
        for (auto & currentCluster : currentHit.getAssignedCluster()) {
          checkClusters.at(currentCluster) = false;
        }
      }

    }

    // if there are any clusters left => not used (checkClusters = true) => deactivate
    for (uint i = 0; i < checkClusters.size(); i++) {
      if (checkClusters.at(i)) {
        // Hit died at Hitfinder
        updateClusters(i, CollectorTFInfo::m_nameHitFinder, CollectorTFInfo::m_idHitFinder, vector<int>(), {FilterID::silentHitFinder}, -1);
      }
    }

  }



  B2DEBUG(100, "Searching for dead hit");
  // Searching for dead hit
  // only if Hits and Cells are already loaded
  if (m_hitTF.size() > 0 && m_cellTF.size() > 0) {

    std::vector<bool> checkHit(m_hitTF.size(), true);

    // Set if the hits are active / or not active
    for (uint i = 0; i < m_hitTF.size(); i++) {
      checkHit.at(i) = m_hitTF.at(i).getActive();
    }

    // Search for Hits not used
    // => every hit used; check = false
    for (auto & currentCell : m_cellTF) {

      // Only check active Cells, other Hits should already been deactivated
      if (currentCell.getActive()) {
        for (auto & currentHit : currentCell.getAssignedHits()) {
          if (currentHit < int(m_hitTF.size()) && currentHit >= 0) {
            checkHit.at(currentHit) = false;
          }
        }
      }

    }

    // if there are any hits left => not used (checkHit = true) => deactivate
    for (uint i = 0; i < checkHit.size(); i++) {
      if (checkHit.at(i)) {
        // Hit died at Segfinder
        updateHit(i, CollectorTFInfo::m_nameCellFinder, CollectorTFInfo::m_idCellFinder, vector<int>(), {FilterID::silentSegFinder}, -1, -1, vector<int>());
      }
    }

  }



  B2DEBUG(100, "Searching for dead Cells");

  // Searching for dead Cells
  // only if TCs and Cells are already loaded
  if (m_tfCandTF.size() > 0 && m_cellTF.size() > 0) {

    std::vector<bool> checkCell(m_cellTF.size(), true);

    // Set if the Cells are active / or not active
    for (uint i = 0; i < m_cellTF.size(); i++) {
      checkCell.at(i) = m_cellTF.at(i).getActive();
    }

    // Search for Cells not used
    // => every cell used; check = false
    for (auto & currentTfCand : m_tfCandTF) {

      // Only check active TF, other cells should already been deactivated
      if (currentTfCand.getActive()) {
        for (auto & currentCellId : currentTfCand.getAssignedCell()) {
          checkCell.at(currentCellId) = false;
        }
      }

    }

    // if there are any cells left => not used (checkCell = true) => deactivate
    for (uint i = 0; i < checkCell.size(); i++) {
      if (checkCell.at(i)) {
        // Cell died at TCC
        updateCell(i, CollectorTFInfo::m_nameTCC, CollectorTFInfo::m_idTCC, vector<int>(), {FilterID::silentTcc}, -1, -2, -1, vector<int>());
      }
    }

  }


  B2DEBUG(100, "End Silent kill");

}



/** Check if the Hit is overlapped */
// 1 & 2 (Collector, here) If Sector of Cluster is overlapped => also Hit is overlapped
// 3 & 4 (Member, isOverlappedByCellsOrTCs) Other Checks with Cells or TC => Hit is overlapped
bool CollectorTFInfo::isHitOverlapped(int hitId)
{

  bool isOverlapped = false;

  //B2DEBUG(100,"CollectorTFInfo: isHitOverlapped, hitId: " << hitId );

  if (hitId >= int(m_hitTF.size())) {
    B2DEBUG(100, "CollectorTFInfo: isHitOverlapped - Hit not found !");
    return isOverlapped;
  }

  HitTFInfo& currentHit = m_hitTF.at(hitId);

  // 1. Sector overlapped ?
  // Search Sector
  //std::map<KeySectors, SectorTFInfo>::iterator it;

  KeySectors myKey(currentHit.getPassIndex(), currentHit.getSectorID());

  auto itCurrentSector = m_sectorTF.find(myKey);

  if (itCurrentSector == m_sectorTF.end()) {
    B2DEBUG(100, "CollectorTFInfo: isHitOverlapped - Sector not found : " << currentHit.getSectorID());
    return isOverlapped;
  }

  SectorTFInfo& currentSector = itCurrentSector->second;

  isOverlapped = currentSector.isOverlapped();
  // if sector is overlapped => other checks not necessary
  if (isOverlapped) {
    B2DEBUG(100, "CollectorTFInfo: isHitOverlapped - Sector overlapped ");
    return isOverlapped;

  }

  // 2. Clusters
  // Check all Clusters of the Hit
  for (const auto & clusterId : currentHit.getAssignedCluster()) {

    if (clusterId >= int(m_clustersTF.size())) {
      B2DEBUG(100, "CollectorTFInfo: isHitOverlapped - Cluster not found !");
      return isOverlapped;
    }

    ClusterTFInfo& currentCluster = m_clustersTF.at(clusterId);

    isOverlapped = currentCluster.isOverlapped();

    // if cluster is overlapped => other checks not necessary
    if (isOverlapped) {
      B2DEBUG(100, "CollectorTFInfo: isHitOverlapped - Cluster overlapped ");
      return isOverlapped;
    }
  }

  // 3. & 4. Cells or Hits
  return currentHit.isOverlappedByCellsOrTCs();
}



/** Check if the Cell is overlapped */
// If one hit is overlapped => Cell is overlapped
// Ohter checks with TC => member (isOverlappedByTC)
bool CollectorTFInfo::isCellOverlapped(int cellID)
{

  bool isOverlapped = false;

  if (cellID >= int(m_cellTF.size())) {
    B2DEBUG(100, "CollectorTFInfo: isCellOverlapped - cell not found !");
    return isOverlapped;
  }

  // Reference to change Cell
  CellTFInfo& currentCell = m_cellTF.at(cellID);

  for (auto & currentHit : currentCell.getAssignedHits()) {

    isOverlapped = isHitOverlapped(currentHit);

    // if hit is overlapped => other checks not necessary
    if (isOverlapped) { return isOverlapped; }
  }

  return currentCell.isOverlappedByTC();
}



/** Check if the TC is overlapped */
// IF one cell of the TC is overlapped => TC is overlapped
bool CollectorTFInfo::isTCOverlapped(int tcid)
{

  bool isOverlapped = false;

  if (tcid >= int(m_tfCandTF.size())) {
    B2DEBUG(100, "CollectorTFInfo: isTCOverlapped - TCCand not found !");
    return isOverlapped;
  }

  // Reference to change TFCand
  TrackCandidateTFInfo& currentTfCand = m_tfCandTF.at(tcid);

  for (auto & currentCell : currentTfCand.getAssignedCell()) {

    isOverlapped = isCellOverlapped(currentCell);

    // if cell is overlapped => other checks not necessary
    if (isOverlapped) { return isOverlapped; }
  }

  return isOverlapped;
}



