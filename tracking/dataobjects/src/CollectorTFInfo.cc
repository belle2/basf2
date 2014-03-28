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

#include "tracking/vxdCaTracking/FilterID.h"
//#include "tracking/modules/VXDTFHelperTools/FilterCalculatorModule.h"
#include "tracking/dataobjects/FullSecID.h"
#include <vxd/geometry/GeoCache.h>

//C++ std lib
#include <utility>
#include <string>
#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;
using namespace Belle2;

const string CollectorTFInfo::nameHitFinder = "HitFinder";
const string CollectorTFInfo::nameCellFinder = "CellFinder";
const string CollectorTFInfo::nameNbFinder = "NbFinder";
const string CollectorTFInfo::nameCA = "CA";
const string CollectorTFInfo::nameTCC = "TCC";
const string CollectorTFInfo::nameOverlap = "Overlap";
const string CollectorTFInfo::nameQI = "QI";
const string CollectorTFInfo::nameHopfield = "Hopfield";

const int CollectorTFInfo::idHitFinder = 0;
const int CollectorTFInfo::idCellFinder = 1;
const int CollectorTFInfo::idNbFinder = 2;
const int CollectorTFInfo::idCA = 3;
const int CollectorTFInfo::idTCC = 4;
const int CollectorTFInfo::idOverlap = 6;
const int CollectorTFInfo::idQI = 5;
const int CollectorTFInfo::idHopfield = 7;
const int CollectorTFInfo::idAlive = -1;

/**
 * Information about the Methodes in CollectorTFInfo.h
 */

ClassImp(CollectorTFInfo)


CollectorTFInfo::CollectorTFInfo()
{
  //m_output_flag = 0;

}

CollectorTFInfo::~CollectorTFInfo()
{
}

// Run at End of beginn-Run / 1 x RUN
/** Sectors safe for all events */
void CollectorTFInfo::initSectors(std::map< std::pair<unsigned int, unsigned int>, std::vector<int> > sectors, std::vector<double> secConfigU, std::vector<double> secConfigV)
{

  B2DEBUG(100, "CollectorTFInfo: initSectors");

  double sectorEdgeV1 = 0, sectorEdgeV2 = 0, uSizeAtv1 = 0, uSizeAtv2 = 0, sectorEdgeU1OfV1 = 0, sectorEdgeU1OfV2 = 0, sectorEdgeU2OfV1 = 0, sectorEdgeU2OfV2 = 0;

  uint aSecID;

  VXD::GeoCache& geometry = VXD::GeoCache::getInstance();

  int sector_size = sectors.size();

  B2DEBUG(100, "CollectorTFInfo: initSectors, sector_size: " << sector_size);

  for (auto & akt_sector : sectors)  {

    // NO Sector 0
    if (akt_sector.first.second != 0) {


      SectorTFInfo newsector(akt_sector.first.first, akt_sector.first.second);

      // Friends store
      newsector.setAllFriends(akt_sector.second);


      FullSecID currentSector = FullSecID(akt_sector.first.second);

      const VXD::SensorInfoBase& aSensorInfo = geometry.getSensorInfo(currentSector.getVxdID());

      double vSize1 = 0.5 * aSensorInfo.getVSize();

      for (uint j = 0; j != secConfigU.size() - 1; ++j) {
        for (uint k = 0; k != secConfigV.size() - 1; ++k) {
          aSecID = k + 1 + j * (secConfigV.size() - 1);
          if (aSecID != (uint) currentSector.getSecID()) { continue; }

          sectorEdgeV1 = secConfigV.at(k) * vSize1 * 2 - vSize1;
          sectorEdgeV2 = secConfigV.at(k + 1) * vSize1 * 2 - vSize1;
          uSizeAtv1 = 0.5 * aSensorInfo.getUSize(sectorEdgeV1);
          uSizeAtv2 = 0.5 * aSensorInfo.getUSize(sectorEdgeV2);
          sectorEdgeU1OfV1 = secConfigU.at(j) * uSizeAtv1 * 2 - uSizeAtv1;
          sectorEdgeU1OfV2 = secConfigU.at(j) * uSizeAtv2 * 2 - uSizeAtv2;
          sectorEdgeU2OfV1 = secConfigU.at(j + 1) * uSizeAtv1 * 2 - uSizeAtv1;
          sectorEdgeU2OfV2 = secConfigU.at(j + 1) * uSizeAtv2 * 2 - uSizeAtv2;

          // sectorEcken
          TVector3 ecke1Lokal = TVector3(sectorEdgeV1, sectorEdgeU1OfV1, 0);
          TVector3 ecke2Lokal = TVector3(sectorEdgeV1, sectorEdgeU2OfV1, 0);
          TVector3 ecke3Lokal = TVector3(sectorEdgeV2, sectorEdgeU1OfV2, 0);
          TVector3 ecke4Lokal = TVector3(sectorEdgeV2, sectorEdgeU2OfV2, 0);

          TVector3 ecke1Global = aSensorInfo.pointToGlobal(ecke1Lokal);
          TVector3 ecke2Global = aSensorInfo.pointToGlobal(ecke2Lokal);
          TVector3 ecke3Global = aSensorInfo.pointToGlobal(ecke3Lokal);
          TVector3 ecke4Global = aSensorInfo.pointToGlobal(ecke4Lokal);

          newsector.setPoint(0, ecke1Global);
          newsector.setPoint(1, ecke2Global);
          newsector.setPoint(2, ecke3Global);
          newsector.setPoint(3, ecke4Global);

          break;
        }
      }

      m_sectorTF_all.insert(make_pair(KeySectors(akt_sector.first.first, akt_sector.first.second), newsector));

    }
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
int CollectorTFInfo::importCluster(int pass_index, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, int detector_type, int relative_position)
{
  B2DEBUG(100, "CollectorTFInfo: importCluster, pass_index: " << pass_index << ", diet_at: " << died_at << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", Detector Type: " << detector_type);

  int cluster_id = m_clustersTF.size();

  ClusterTFInfo newcluster(pass_index, cluster_id, detector_type);

  newcluster.setUseCounter(0);

  newcluster.setDiedAt(died_at);
  newcluster.setDiedID(died_id);

  newcluster.insert_Accepted(accepted);
  newcluster.insert_Rejected(rejected);

  newcluster.setRelativeClusterID(relative_position);

  m_clustersTF.push_back(newcluster);

  // B2DEBUG(100,"CollectorTFInfo: importCluster, startPositionID: " << newcluster.getStartPositionID() );
  B2DEBUG(100, "CollectorTFInfo: importCluster ClusterID: " << cluster_id);


  return cluster_id;
}



/** Sectors update after update / import Hit
 Reload Sector if not in current Sectors
 Also checks Friends sectors (possible reload) */
void CollectorTFInfo::updateSectors(int sector_ID, int pass_index, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter)
{
  B2DEBUG(100, "CollectorTFInfo: updateSectors, sector_ID: " << sector_ID << ", Pass Index: " << pass_index << ", diet_at: " << died_at << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", deltaUseCounter: " << deltaUseCounter);

  std::map<KeySectors, SectorTFInfo>::iterator it, it_friend;

  // Key of the current Sector
  KeySectors my_key(pass_index, sector_ID);

  it = m_sectorTF.find(my_key);

  // Sector Search in current Sectors
  if (it == m_sectorTF.end()) {
    // Not found => Search in all Sectors
    it = m_sectorTF_all.find(my_key);

    if (it == m_sectorTF_all.end()) {
      // Not found in all Sectors
      B2DEBUG(100, "CollectorTFInfo: updateSectors - Sector not found !");
      return;

    } else {

      // Found => Sector reload
      B2DEBUG(100, "Sector reload: sector_ID: " << sector_ID);

      // + Updates
      //Copy of sector => use for this event
      SectorTFInfo akt_sector = it->second;
      akt_sector.setIsOnlyFriend(false);

      akt_sector.changeUseCounter(deltaUseCounter);

      // DiedAt = add to string
      if (died_at.size() != 0 && akt_sector.getUseCounter() == 0) {
        akt_sector.setDiedAt(died_at);
        akt_sector.setDiedID(died_id);
      }

      akt_sector.insert_Accepted(accepted);
      akt_sector.insert_Rejected(rejected);

      B2DEBUG(100, "CollectorTFInfo: updateSectors - akt deltaUseCounter : " << akt_sector.getUseCounter());

      m_sectorTF.insert(make_pair(my_key, akt_sector));

      // Reload all friends of current Sector
      for (auto & akt_friend_sector : akt_sector.getFriends()) {

        // Key of Friend Sector
        KeySectors friend_key(pass_index, akt_friend_sector);

        // Search in current Sectors (if loaded => nothing)
        if (m_sectorTF.find(friend_key) ==  m_sectorTF.end()) {

          // not found => search in all Sectors
          it_friend = m_sectorTF_all.find(friend_key);

          // Reload Friend Sector
          if (it_friend == m_sectorTF_all.end()) {
            // Sector not found
            B2DEBUG(100, "NIX DA Sector-Friend");
          } else {

            B2DEBUG(100, "Sector-Friend reload: sector_ID: " << akt_friend_sector);

            //Copy of sector => use for this event
            SectorTFInfo akt_friend_sector_obj = it_friend->second;

            // Friend reload => not active
            akt_friend_sector_obj.setIsOnlyFriend(true);
            m_sectorTF.insert(make_pair(friend_key, akt_friend_sector_obj));

          }
        }
      }

    }

  } else {

    // Reference to change Sector Information
    SectorTFInfo& akt_sector = it->second;

    //in Case it only was a sector friend before
    akt_sector.setIsOnlyFriend(false);

    // Only update active Friends ???

    akt_sector.changeUseCounter(deltaUseCounter);

    // DiedAt = add to string
    if (died_at.size() != 0 && akt_sector.getUseCounter() == 0) {
      akt_sector.setDiedAt(died_at);
      akt_sector.setDiedID(died_id);
    }

    akt_sector.insert_Accepted(accepted);
    akt_sector.insert_Rejected(rejected);

    B2DEBUG(100, "CollectorTFInfo: updateSectors - akt deltaUseCounter : " << akt_sector.getUseCounter());

  }


}


/** Cluster Update */
void CollectorTFInfo::updateClusters(int cluster_id, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter)   //Cluster Update
{
  B2DEBUG(100, "CollectorTFInfo: updateClusters, cluster_id: " << cluster_id << ", diet_at: " << died_at << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", deltaUseCounter: " << deltaUseCounter);

  if (cluster_id >= int(m_clustersTF.size())) {
    B2DEBUG(100, "CollectorTFInfo: updateClusters - Cluster not found !");
    return;
  } else {

    //Reference => change cluster
    ClusterTFInfo& akt_cluster = m_clustersTF.at(cluster_id);

    akt_cluster.changeUseCounter(deltaUseCounter);

    // DiedAt = add to string
    if (died_at.size() != 0 && akt_cluster.getUseCounter() <= 0) {
      akt_cluster.setDiedAt(died_at);
      akt_cluster.setDiedID(died_id);
    }

    akt_cluster.insert_Accepted(accepted);
    akt_cluster.insert_Rejected(rejected);

    B2DEBUG(100, "CollectorTFInfo: updateClusters - akt deltaUseCounter : " << akt_cluster.getUseCounter());
  }



}



/** Import Hit (return hitID) */
int CollectorTFInfo::importHit(int pass_index, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assigned_Cluster_IDs, int sec_id, TVector3 hit_position, TVector3 hit_sigma)  // Hit import
{
  B2DEBUG(100, "CollectorTFInfo: importHit, pass_index: " << pass_index << ", diet_at: " << died_at << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", sec_id: " << sec_id);

  int hit_id = m_hitTF.size();

  HitTFInfo newhit(pass_index, sec_id, hit_position, hit_sigma);

  //newhit.setUseCounter(0);

  newhit.setDiedAt(died_at);
  newhit.setDiedID(died_id);

  newhit.insert_Accepted(accepted);
  newhit.insert_Rejected(rejected);

  // 1 = deltaUseCounter, +1 use
  updateSectors(sec_id, pass_index, died_at, died_id, accepted, rejected, 1);

  for (auto & akt_id : assigned_Cluster_IDs) {

    newhit.push_back_AssignedCluster(akt_id);

    // 1 = delausecounter, +1 use
    updateClusters(akt_id, died_at, died_id, accepted, rejected, 1);

    // take NO PARTICLE ID from Relations !!!  => moved to TFAnalizerModule
    //  => disabled
    // newhit.setParticleID(getParticleIDfromRelations(pass_index, akt_id));
  }

  m_hitTF.push_back(newhit);

  return hit_id;
}


/** Hit Update */
// add_TCID, remove_TCID if -1 => no deltaUseCounter
void CollectorTFInfo::updateHit(int hit_id, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, int add_TCID, int remove_TCID, std::vector<int> deltaUseCounter_cell)
{
  B2DEBUG(100, "CollectorTFInfo: updateHit, hit_id: " << hit_id << ", diet_at: " << died_at << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", deltaUseCounter_cell - size: " << deltaUseCounter_cell.size());

  if (hit_id >= int(m_hitTF.size()) || hit_id == -1) {
    B2DEBUG(100, "CollectorTFInfo: updateHit - Hit not found !");
    return;
  } else {

    //Reference => change hit
    HitTFInfo& akt_hit = m_hitTF.at(hit_id);

    int deltaUseCounter = akt_hit.changeUseCounterCell(deltaUseCounter_cell);

    // Changed Cells = deltaUseCounter

    akt_hit.changeUseCounter(deltaUseCounter);

    //B2DEBUG(100,"CollectorTFInfo: akt deltaUseCounter: " << deltaUseCounter);

    string died_at_akt;
    int died_id_akt = 0;

    int deltaUseHit = 0;

    // DiedAt = add to string
    if (akt_hit.getUseCounter() == 0 && akt_hit.getActive()) {
      died_at_akt = died_at;
      died_id_akt = died_id;

      akt_hit.setDiedAt(died_at_akt);
      akt_hit.setDiedID(died_id_akt);

      // Delta Use Count for Sectors & Clusters
      if (died_at.size() != 0) {
        deltaUseHit = -1;
      }
    }

    for (auto & akt_cluster : akt_hit.getAssignedCluster()) {
      // 0 = only update for cluster (already used by this hit)
      updateClusters(akt_cluster, died_at_akt, died_id_akt, accepted, rejected, deltaUseHit);
    }

    // 0 = only update for sector (already used by this hit)
    updateSectors(akt_hit.getSectorID(), akt_hit.getPassIndex(), died_at_akt, died_id_akt, accepted, rejected, deltaUseHit);

    //Only Add TCID to List if not already in it
    if (add_TCID != -1) {
      akt_hit.push_back_UseCounterTCIDs(add_TCID);
      akt_hit.push_back_AllCounterTCIDs(add_TCID);
    }

    if (remove_TCID != -1) {
      akt_hit.remove_UseCounterTCIDs(remove_TCID);
    }

    B2DEBUG(100, "CollectorTFInfo: INFO Hit id " << hit_id << ", akt UseCounter: " << akt_hit.getUseCounter() << ", died at: " << died_at);



    akt_hit.insert_Accepted(accepted);
    akt_hit.insert_Rejected(rejected);
  }

}



/** Cell Import */
int CollectorTFInfo::importCell(int pass_index, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assigned_Hit_IDs)
{
  B2DEBUG(100, "CollectorTFInfo: importCell, pass_index: " << pass_index << ", diet_at: " << died_at << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", assigned_Hit_IDs - size: " << assigned_Hit_IDs.size());

  int cellID = m_cellTF.size();

  CellTFInfo newcell(pass_index);

  newcell.setUseCounter(0);

  newcell.setDiedAt(died_at);
  newcell.setDiedID(died_id);

  newcell.insert_Accepted(accepted);
  newcell.insert_Rejected(rejected);

  // standard = 2 cells, but so its varible
  std::vector<int> cell_update(assigned_Hit_IDs.size());

  for (auto & index_cell : cell_update) {
    index_cell = 0;
  }

  // standard = 2 Hits
  for (auto & akt_hit : assigned_Hit_IDs) {
    newcell.push_back_AssignedHits(akt_hit);

    // outer hit  = Index 0, inner hit = Index 1
    // Index = Posistion of akt_hit
    cell_update.at(&akt_hit - &assigned_Hit_IDs.at(0)) = 1;

    // no ID from TC => -1; deltaUseCounter for cell = 1
    updateHit(akt_hit, died_at, died_id, accepted, rejected, -1, -1, cell_update);

    // Index = Posistion of akt_hit
    cell_update.at((&akt_hit - &assigned_Hit_IDs.at(0))) = 0;

  }

  m_cellTF.push_back(newcell);

  return cellID;
}





/** Update Cell */
void CollectorTFInfo::updateCell(int cellID, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, int add_TCID, int remove_TCID, int cellstate, std::vector<int> neighbours)
{
  B2DEBUG(100, "CollectorTFInfo: updateCell, cellID: " << cellID << ", diet_at: " << died_at << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", add_TCID: " << add_TCID << ", remove_TCID: " << remove_TCID << ", died_id: " << died_id);

  if (cellID >= int(m_cellTF.size()) || cellID == -1) {
    B2DEBUG(100, "CollectorTFInfo: updateCell - cell not found !");
    return;

  } else {
    // Reference to change Cell
    CellTFInfo& akt_cell = m_cellTF.at(cellID);

    // set State (if cellstate = -1 => no change in CellState)
    if (cellstate != -1) {
      akt_cell.setState(cellstate);
    }

    // If the from the nb-Finder: Add neighbours
    if (neighbours.size() > 0) {
      akt_cell.insert_Neighbours(neighbours);
    }

    akt_cell.insert_Accepted(accepted);
    akt_cell.insert_Rejected(rejected);

    int deltaUseCounter = 0;

    // add_TCID/remove_TCID if -1 => no deltaUseCounter
    if (add_TCID != -1) {
      deltaUseCounter++;
    }

    if (remove_TCID != -1) {
      deltaUseCounter--;
    }

    akt_cell.changeUseCounter(deltaUseCounter);

    std::string akt_died_at;

    // DiedAt = add to string
    // remove_TCID == -1 => died from Cell - Update
    // remove TCID == -2 => silent Kill
    // remove_TCID != -1 && akt_cell.getUseCounter() == 0 => died from TC - Update
    if (remove_TCID == -1 || remove_TCID == -2  || (remove_TCID != -1 && akt_cell.getUseCounter() == 0)) {
      akt_died_at = died_at;
      if (died_id != -1) {    akt_cell.setDiedID(died_id);  }
    }

    akt_cell.setDiedAt(akt_died_at);





    // standard = 2 cells, but so its varible
    std::vector<int> cell_update(akt_cell.getAssignedHits().size());

    for (auto & index_cell : cell_update) {
      index_cell = 0;
    }

    // TC add => no Change in Counter for Hits
    // remove TC ID ????
    if (add_TCID != -1) {
      deltaUseCounter = 0;
    }

    for (auto & akt_hit : akt_cell.getAssignedHits()) {

      // outer hit  = Index 0, inner hit = Index 1
      // Index = Posistion of akt_hit
      cell_update.at(&akt_hit - &akt_cell.getAssignedHits().at(0)) = deltaUseCounter;

      // outer hit  = Index 0, inner hit = Index 1
      updateHit(akt_hit, akt_died_at, died_id, accepted, rejected, add_TCID, remove_TCID, cell_update);

      // Index = Posistion of akt_hit
      cell_update.at(&akt_hit - &akt_cell.getAssignedHits().at(0)) = 0;


    }

  }
}


/** TC Import, return = tc id */
int CollectorTFInfo::importTC(int pass_index, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, std::vector<std::pair<int, unsigned int>> assigned_Cell_IDs)
{
  B2DEBUG(100, "CollectorTFInfo: importTC, pass_index: " << pass_index << ", diet_at: " << died_at << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size() << ", assigned_Cell_IDs - size: " << assigned_Cell_IDs.size());

  int tfcandID = m_tfCandTF.size();

  TrackCandidateTFInfo newtf(pass_index, tfcandID);

  newtf.setDiedAt(died_at);
  newtf.setDiedID(died_id);

  newtf.insert_Accepted(accepted);
  newtf.insert_Rejected(rejected);

  for (auto & akt_cell : assigned_Cell_IDs) {
    newtf.push_back_AssignedCell(akt_cell.first);

    // vector<int>() = no new neighbours
    updateCell(akt_cell.first, died_at, died_id, accepted, rejected, tfcandID, -1, akt_cell.second, vector<int>());   // ID vom TCCAND.-Filter fehlt
  }

  m_tfCandTF.push_back(newtf);

  return tfcandID;
}


/** update TCand */
void CollectorTFInfo::updateTC(int tcid, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected)  // TC Update
{
  B2DEBUG(100, "CollectorTFInfo: updateTC, tcid: " << tcid << ", diet_at: " << died_at << ", accepted-size: " << accepted.size() << ", rejected-size: " << rejected.size());

  if (tcid >= int(m_tfCandTF.size()) || tcid == -1) {
    B2DEBUG(100, "CollectorTFInfo: updateTC - TCCand not found !");
    return;

  } else {
    // Reference to change TFCand
    TrackCandidateTFInfo& akt_tfcand = m_tfCandTF.at(tcid);

    // DiedAt = add to string
    akt_tfcand.setDiedAt(died_at);
    akt_tfcand.setDiedID(died_id);
    akt_tfcand.insert_Accepted(accepted);
    akt_tfcand.insert_Rejected(rejected);

    int remove_TCID = -1;
    if (died_at.size() > 0) {
      remove_TCID = tcid;
    }

    for (auto & akt_cell : akt_tfcand.getAssignedCell()) {
      // vector<int>() = no new neighbours
      // -1 => only update of the TC so no TC-ID needed
      updateCell(akt_cell, died_at, died_id, accepted, rejected, -1, remove_TCID, -1, vector<int>());
    }


  }

}


/** Update Fit Information (fit_successful, probability_value, assigned_GTFC) of TC */
void CollectorTFInfo::updateTCFitInformation(int tcid, bool fit_successful, double probability_value, int assigned_GTFC)  // TC Update
{
  B2DEBUG(100, "CollectorTFInfo: updateTCFitInformation, tcid: " << tcid << ", fit_successful: " << fit_successful << ", probability_value: " << probability_value << ", assigned_GTFC: " << assigned_GTFC);

  if (tcid >= int(m_tfCandTF.size()) || tcid < 0) {
    B2DEBUG(100, "CollectorTFInfo: updateTCFitInformation - TCCand not found !: " << tcid);
    return;

  } else {
    // Reference to change TFCand
    TrackCandidateTFInfo& akt_tfcand = m_tfCandTF.at(tcid);

    akt_tfcand.fitIsPossible(fit_successful);
    akt_tfcand.setProbValue(probability_value);
    akt_tfcand.assignGFTC(assigned_GTFC);

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

  for (auto & akt_cluster : m_clustersTF) {
    clusterTFInfo.appendNew(akt_cluster);
  }

  B2DEBUG(100, "*******  Cluster size: " << clusterTFInfo.getEntries() << " *******");
  B2DEBUG(100, "*******  akt Cluster size: " << m_clustersTF.size() << " *******");

  // SECTORS
  StoreArray<SectorTFInfo> sectorTFInfo("");
  if (!sectorTFInfo.isValid())  sectorTFInfo.create();


  // Friends of Sectors
  RelationArray relSectorSectorFriend(sectorTFInfo, sectorTFInfo);

  std::map<KeySectors, SectorTFInfo>::iterator it;
  KeySectors friend_key;

  for (auto & akt_sector : m_sectorTF) {
    int index_sector = sectorTFInfo.getEntries();

    sectorTFInfo.appendNew(akt_sector.second);


    for (auto & akt_friend : akt_sector.second.getFriends()) {

      // Pass_id
      friend_key.first = akt_sector.second.getPassIndex();
      friend_key.second = akt_friend;

      it = m_sectorTF.find(friend_key);

      // Sector Search in current Sectors
      if (it != m_sectorTF.end()) {
        relSectorSectorFriend.add(index_sector, std::distance(m_sectorTF.begin(), it));
        B2DEBUG(100, "pass_index: " << friend_key.first << "; Sector akt: " << index_sector << ", friend sector: " << std::distance(m_sectorTF.begin(), it) << ", sector id (akt/friend): " << akt_sector.second.getSectorID() << " / " << akt_friend);
      } else {
        B2DEBUG(100, "Friend-Sector not found: " << akt_friend);
      }

      // same with sectorid; IDs = SectorIDs for akt. Sector and Friend sectors
      //relSectorSectorFriend.add (akt_sector.second.getSectorID(), akt_friend);
    }
  }

  B2DEBUG(100, "*******  SECTORS size: " << sectorTFInfo.getEntries() << " *******");
  B2DEBUG(100, "*******  akt SECTORS size: " << m_sectorTF.size() << " *******");

  B2DEBUG(100, "*******  relSectorSectorFriend: " << relSectorSectorFriend.getEntries() << " *******");


  // HITS
  StoreArray<HitTFInfo> hitTFInfo("");
  if (!hitTFInfo.isValid())  hitTFInfo.create();

  for (auto & akt_hit : m_hitTF) {
    hitTFInfo.appendNew(akt_hit);
  }

  // Cells
  StoreArray<CellTFInfo> cellTFInfo("");
  if (!cellTFInfo.isValid())  cellTFInfo.create();

  RelationArray relCellNBCell(cellTFInfo, cellTFInfo);

  for (auto & akt_cell : m_cellTF) {
    int index_cell = cellTFInfo.getEntries();

    cellTFInfo.appendNew(akt_cell);

    // ID of cell/neighbours = Index of cell
    for (auto & akt_nb : akt_cell.getNeighbours()) {
      relCellNBCell.add(index_cell, akt_nb);
      B2DEBUG(100, "*******  relCellNBCell: index_cell: " << index_cell << ", akt_nb: " << akt_nb);
    }
  }

  B2DEBUG(100, "*******  relCellNBCell: " << relCellNBCell.getEntries() << " *******");

  // TFCAND
  StoreArray<TrackCandidateTFInfo> tfcandTFInfo("");
  if (!tfcandTFInfo.isValid())  tfcandTFInfo.create();

  for (auto & akt_tfcand : m_tfCandTF) {
    tfcandTFInfo.appendNew(akt_tfcand);
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

    std::vector<bool> check_clusters(m_clustersTF.size(), true);

    // Set if the clusters are active / or not active
    for (uint i = 0; i < m_clustersTF.size(); i++) {
      check_clusters.at(i) = m_clustersTF.at(i).getActive();
    }

    // Search for Hits not used
    // => every hit used; check = false
    for (auto & akt_hit : m_hitTF) {

      // Only check active Cells, other Hits should already been deactivated
      if (akt_hit.getActive()) {
        for (auto & akt_cluster : akt_hit.getAssignedCluster()) {
          check_clusters.at(akt_cluster) = false;
        }
      }

    }

    // if there are any clusters left => not used (check_clusters = true) => deactivate
    for (uint i = 0; i < check_clusters.size(); i++) {
      if (check_clusters.at(i)) {
        // Hit died at Hitfinder
        updateClusters(i, CollectorTFInfo::nameHitFinder, CollectorTFInfo::idHitFinder, vector<int>(), {FilterID::silentHitFinder}, -1);
      }
    }

  }



  B2DEBUG(100, "Searching for dead hit");
  // Searching for dead hit
  // only if Hits and Cells are already loaded
  if (m_hitTF.size() > 0 && m_cellTF.size() > 0) {

    std::vector<bool> check_hit(m_hitTF.size(), true);

    // Set if the hits are active / or not active
    for (uint i = 0; i < m_hitTF.size(); i++) {
      check_hit.at(i) = m_hitTF.at(i).getActive();
    }

    // Search for Hits not used
    // => every hit used; check = false
    for (auto & akt_cell : m_cellTF) {

      // Only check active Cells, other Hits should already been deactivated
      if (akt_cell.getActive()) {
        for (auto & akt_hit : akt_cell.getAssignedHits()) {
          if (akt_hit < int(m_hitTF.size()) && akt_hit >= 0) {
            check_hit.at(akt_hit) = false;
          }
        }
      }

    }

    // if there are any hits left => not used (check_hit = true) => deactivate
    for (uint i = 0; i < check_hit.size(); i++) {
      if (check_hit.at(i)) {
        // Hit died at Segfinder
        updateHit(i, CollectorTFInfo::nameCellFinder, CollectorTFInfo::idCellFinder, vector<int>(), {FilterID::silentSegFinder}, -1, -1, vector<int>());
      }
    }

  }



  B2DEBUG(100, "Searching for dead Cells");

  // Searching for dead Cells
  // only if TCs and Cells are already loaded
  if (m_tfCandTF.size() > 0 && m_cellTF.size() > 0) {

    std::vector<bool> check_cell(m_cellTF.size(), true);

    // Set if the Cells are active / or not active
    for (uint i = 0; i < m_cellTF.size(); i++) {
      check_cell.at(i) = m_cellTF.at(i).getActive();
    }

    // Search for Cells not used
    // => every cell used; check = false
    for (auto & akt_tfcand : m_tfCandTF) {

      // Only check active TF, other cells should already been deactivated
      if (akt_tfcand.getActive()) {
        for (auto & akt_cell_id : akt_tfcand.getAssignedCell()) {
          check_cell.at(akt_cell_id) = false;
        }
      }

    }

    // if there are any cells left => not used (check_cell = true) => deactivate
    for (uint i = 0; i < check_cell.size(); i++) {
      if (check_cell.at(i)) {
        // Cell died at TCC
        updateCell(i, CollectorTFInfo::nameTCC, CollectorTFInfo::idTCC, vector<int>(), {FilterID::silentTcc}, -1, -2, -1, vector<int>());
      }
    }

  }


  B2DEBUG(100, "End Silent kill");

}



/** Check if the Hit is overlapped */
// 1 & 2 (Collector, here) If Sector of Cluster is overlapped => also Hit is overlapped
// 3 & 4 (Member, isOverlappedByCellsOrTCs) Other Checks with Cells or TC => Hit is overlapped
bool CollectorTFInfo::isHitOverlapped(int hit_id)
{

  bool is_overlapped = false;

  //B2DEBUG(100,"CollectorTFInfo: isHitOverlapped, hit_id: " << hit_id );

  if (hit_id >= int(m_hitTF.size())) {
    B2DEBUG(100, "CollectorTFInfo: isHitOverlapped - Hit not found !");
    return is_overlapped;
  }

  HitTFInfo& akt_hit = m_hitTF.at(hit_id);

  // 1. Sector overlapped ?
  // Search Sector
  std::map<KeySectors, SectorTFInfo>::iterator it;

  KeySectors my_key(akt_hit.getPassIndex(), akt_hit.getSectorID());

  it = m_sectorTF.find(my_key);

  if (it == m_sectorTF.end()) {
    B2DEBUG(100, "CollectorTFInfo: isHitOverlapped - Sector not found : " << akt_hit.getSectorID());
    return is_overlapped;
  }

  SectorTFInfo& akt_sector = it->second;

  is_overlapped = akt_sector.isOverlapped();
  // if sector is overlapped => other checks not necessary
  if (is_overlapped) {
    B2DEBUG(100, "CollectorTFInfo: isHitOverlapped - Sector overlapped ");
    return is_overlapped;

  }

  // 2. Clusters
  // Check all Clusters of the Hit
  for (auto & cluster_id : akt_hit.getAssignedCluster()) {

    if (cluster_id >= int(m_clustersTF.size())) {
      B2DEBUG(100, "CollectorTFInfo: isHitOverlapped - Cluster not found !");
      return is_overlapped;
    }

    ClusterTFInfo& akt_cluster = m_clustersTF.at(cluster_id);

    is_overlapped = akt_cluster.isOverlapped();

    // if cluster is overlapped => other checks not necessary
    if (is_overlapped) {
      B2DEBUG(100, "CollectorTFInfo: isHitOverlapped - Cluster overlapped ");
      return is_overlapped;
    }
  }

  // 3. & 4. Cells or Hits
  return akt_hit.isOverlappedByCellsOrTCs();
}



/** Check if the Cell is overlapped */
// If one hit is overlapped => Cell is overlapped
// Ohter checks with TC => member (isOverlappedByTC)
bool CollectorTFInfo::isCellOverlapped(int cellID)
{

  bool is_overlapped = false;

  if (cellID >= int(m_cellTF.size())) {
    B2DEBUG(100, "CollectorTFInfo: isCellOverlapped - cell not found !");
    return is_overlapped;
  }

  // Reference to change Cell
  CellTFInfo& akt_cell = m_cellTF.at(cellID);

  for (auto & akt_hit : akt_cell.getAssignedHits()) {

    is_overlapped = isHitOverlapped(akt_hit);

    // if hit is overlapped => other checks not necessary
    if (is_overlapped) { return is_overlapped; }
  }

  return akt_cell.isOverlappedByTC();
}



/** Check if the TC is overlapped */
// IF one cell of the TC is overlapped => TC is overlapped
bool CollectorTFInfo::isTCOverlapped(int tcid)
{

  bool is_overlapped = false;

  if (tcid >= int(m_tfCandTF.size())) {
    B2DEBUG(100, "CollectorTFInfo: isTCOverlapped - TCCand not found !");
    return is_overlapped;
  }

  // Reference to change TFCand
  TrackCandidateTFInfo& akt_tfcand = m_tfCandTF.at(tcid);

  for (auto & akt_cell : akt_tfcand.getAssignedCell()) {

    is_overlapped = isCellOverlapped(akt_cell);

    // if cell is overlapped => other checks not necessary
    if (is_overlapped) { return is_overlapped; }
  }

  return is_overlapped;
}



