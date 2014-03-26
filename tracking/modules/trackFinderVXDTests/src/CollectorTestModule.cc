/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Ferstl                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/trackFinderVXDTests/CollectorTestModule.h"
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CollectorTest)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CollectorTestModule::CollectorTestModule() : Module()
{
  //Set module properties
  //setDescription("simply highlights current event");
  //setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  //addParam("stepSize", m_stepSize, "e.g. 100 will highlight every 100th event", int(100));
  setDescription("simply tests the Collector of the TrackFinder");
}


CollectorTestModule::~CollectorTestModule()
{

}


void CollectorTestModule::initialize()
{
  // StoreArray<PXDCluster>::optional();
  // StoreArray<SVDCluster>::optional();

// m_collector.initPersistent();

}


void CollectorTestModule::beginRun()
{
  m_eventCounter = 0;
  m_pxdClusterCounter = 0;
  m_svdClusterCounter = 0;
  B2DEBUG(100, "################## collectortest enabled");
}


void CollectorTestModule::event()
{

  B2DEBUG(100, "################## Event Run CollectorTestModule");
  /*
    m_eventCounter++;

    StoreArray<PXDCluster> aPxdClusterArray;
    m_pxdClusterCounter += aPxdClusterArray.getEntries();

    StoreArray<SVDCluster> aSvdClusterArray;
    m_svdClusterCounter += aSvdClusterArray.getEntries();

    if (m_eventCounter % m_stepSize == 0) {
      B2DEBUG(100, "CollectorTestModule - Event: " << m_eventCounter);
    }


    testSilentKill();
    testOverlapped();
    testAllInformationLoop();
    testAllInformationStandard();
    m_collector.safeInformation();

    // Test safeInformation
    StoreArray<SectorTFInfo> sectorTFInfo("");
    int nTFClusters = sectorTFInfo.getEntries();
    if (nTFClusters == 0) {B2DEBUG(100, "VXDTF: Display: Sector is empty!");}

    B2DEBUG(100, "XXX VXDTF: Display: Sector Size: " << nTFClusters);

    for (auto & akt_sector : sectorTFInfo) {
      B2DEBUG(100, "((( Sector ID: " << akt_sector.getSectorID() << "; active: " << akt_sector.getActive() << ", died_at: " << akt_sector.getDiedAt() << "; is only friend: " <<
             akt_sector.getIsOnlyFriend() << "; Friends - size: "
             << akt_sector.getFriends().size() << "; use counter: " << akt_sector.getUseCounter());
    }

    // Filename of Hit - Save - File
    std::ostringstream oss;
    oss << "all_my_hits_event_" << m_eventCounter << ".txt";

    //  m_collector.storeHitInformation(oss.str());

    Start Kommentar  Information for Sector Friends
    [INFO] pass_index: 0; Sector akt: 1, friend sector: 1, sector id (akt/friend): 1 / 1
    [INFO] pass_index: 0; Sector akt: 1, friend sector: 3, sector id (akt/friend): 1 / 3
    [INFO] pass_index: 0; Sector akt: 1, friend sector: 6, sector id (akt/friend): 1 / 6
    [INFO] pass_index: 0; Sector akt: 1, friend sector: 12, sector id (akt/friend): 1 / 17

    [INFO] pass_index: 0; Sector akt: 2, friend sector: 2, sector id (akt/friend): 2 / 2
    [INFO] pass_index: 0; Sector akt: 2, friend sector: 10, sector id (akt/friend): 2 / 12
    [INFO] pass_index: 0; Sector akt: 2, friend sector: 11, sector id (akt/friend): 2 / 15

    1:
      sectors_display_friends.push_back (1);
      sectors_display_friends.push_back (3);
      sectors_display_friends.push_back (6);
      sectors_display_friends.push_back (17);
    2:
      sectors_display_friends.push_back (2);
      sectors_display_friends.push_back (12);
      sectors_display_friends.push_back (15);

      */

}


void CollectorTestModule::endRun()
{
  /*
   if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
   double invEvents = 1. / m_eventCounter;
   B2DEBUG(100, "CollectorTestModule: after " << m_eventCounter << " events there were " << m_pxdClusterCounter << "/" << m_svdClusterCounter << " pxd/svdClusters total and " << double(m_pxdClusterCounter)*invEvents << "/" << double(m_svdClusterCounter)*invEvents << " pxd/svdClusters per event");

   */
}


void CollectorTestModule::terminate()
{

}




void CollectorTestModule::import_sectors_standard()
{


  int sector_map_size = 40;

  // Create Test Sector
  std::map<std::pair<unsigned int, unsigned int>, std::vector<int>> sectors_display_all_pass;
  std::vector<int> sectors_display_friends;

  B2DEBUG(100, "PassNr. " << pass_sector_id_single << "Size of Sector Map: " << sector_map_size);

  sectors_display_friends.clear();
  sectors_display_all_pass.insert(std::make_pair(std::make_pair(pass_sector_id_single, 0), sectors_display_friends));

  sectors_display_friends.clear();
  sectors_display_friends.push_back(1);
  sectors_display_friends.push_back(3);
  sectors_display_friends.push_back(6);
  sectors_display_friends.push_back(17);
  sectors_display_all_pass.insert(std::make_pair(std::make_pair(pass_sector_id_single, 1), sectors_display_friends));

  sectors_display_friends.clear();
  sectors_display_friends.push_back(2);
  sectors_display_friends.push_back(12);
  sectors_display_friends.push_back(15);
  sectors_display_all_pass.insert(std::make_pair(std::make_pair(pass_sector_id_single, 2), sectors_display_friends));

  // Sector 3 - 40
  sectors_display_friends.clear();
  for (int u = 3; u < sector_map_size; u++) {
    sectors_display_all_pass.insert(std::make_pair(std::make_pair(pass_sector_id_single, u), sectors_display_friends));
  }


  // m_collector.initSectors (sectors_display_all_pass, vector<double>(),  vector<double>());
  m_collector.intEvent();

}


void CollectorTestModule::import_clusters_standard()
{

  int anz_clusters_svd = 15;
  int anz_clusters_pxd = 15;


  for (int i = 0; i <  anz_clusters_svd; i++) {
    // importCluster (int pass_index, std::string died_at, int accepted, int rejected, int detector_type)
    m_collector.importCluster(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), Const::SVD, i);

//     B2DEBUG ("AKT ID SVD: " << akt_id);

  }

  for (int i = 0; i <  anz_clusters_pxd; i++) {
    // importCluster (int pass_index, std::string died_at, int accepted, int rejected, int detector_type)
    m_collector.importCluster(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), Const::PXD, i);

//     B2DEBUG ("AKT ID PXD: " << akt_id);
  }



}



void CollectorTestModule::import_sectors_loop()
{


  // Create Test Sector
  std::map<unsigned int, std::vector<int>> sector_map;
  int size_of_sectors = 10;

  for (int i = 0; i < size_of_sectors; i++) {
    sector_map.insert(std::make_pair(i, std::vector<int>()));
  }


  // 1. Sectors init

  //KeySectors dosn't function => so pair Int int
  std::map<std::pair<unsigned int, unsigned int>, std::vector<int>> sectors_display_all_pass;
  std::vector<int> sectors_display_friends;

  for (uint i = 0; i < pass_sector_ids.size(); i++) {

    B2DEBUG(100, "PassNr. " << i << "Size of Sector Map: " << sector_map.size());

    for (auto & akt_sector : sector_map) {
      sectors_display_friends.clear();

      // Friends read and store in second vector
      for (auto & akt_friend : akt_sector.second) {
        sectors_display_friends.push_back(akt_friend);
      }

      sectors_display_all_pass.insert(std::make_pair(std::make_pair(i, akt_sector.first), sectors_display_friends));

    }

  }


  m_collector.intEvent();
  // m_collector.initSectors (sectors_display_all_pass, vector<double>(), vector<double>());

}


void CollectorTestModule::import_clusters_loop()
{

  int anz_clusters_svd = 10;
  int anz_clusters_pxd = 10;

  for (uint index = 0; index < pass_sector_ids.size(); index++) {

    for (int i = 0; i <  anz_clusters_svd; i++) {
      // importCluster (int pass_index, std::string died_at, int accepted, int rejected, int detector_type)
      m_collector.importCluster(index, "", -1, vector<int>(), vector<int>(), Const::SVD, i);

//     B2DEBUG ("AKT ID SVD: " << akt_id);

    }

    for (int i = 0; i <  anz_clusters_pxd; i++) {
      // importCluster (int pass_index, std::string died_at, int accepted, int rejected, int detector_type)
      m_collector.importCluster(index, "", -1, vector<int>(), vector<int>(), Const::PXD, i);

//     B2DEBUG ("AKT ID PXD: " << akt_id);
    }

  }


}





void CollectorTestModule::import_hit_standard()
{

  //     virtual int importHit (int pass_index, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assigned_Cluster_IDs, int sec_id, TVector3 hit_position); // Hit import_clusters

  // ID 0 - 4
  // Sector Overlap => Hitid 3,4
  // Cluster Overlap => Hitid 7,8
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({1, 2, 3}), 0, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({4}), 2, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({7}), 4, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({9}), 5, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({10}), 5, TVector3(), TVector3());

  // ID 5 - 9
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({11, 12, 13}), 6, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({14, 15}), 7, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({17}), 8, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({17}), 9, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({18}), 1, TVector3(), TVector3());

  // ID 10 - 14
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>(), -1, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>(), -1, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>(), -1, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>(), -1, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>(), -1, TVector3(), TVector3());

  // ID 15 - 19
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>(), 19, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>(), 20, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>(), 21, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>(), 22, TVector3(), TVector3());
  m_collector.importHit(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>(), 23, TVector3(), TVector3());

}



void CollectorTestModule::import_hit_loop()
{

  int anz_hits = 10;

  // vector<int> clusters = {7, 8, 9};

  for (uint index = 0; index < pass_sector_ids.size(); index++) {

    for (int i = 0; i <  anz_hits; i++) {

      //     virtual int importHit (int pass_index, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assigned_Cluster_IDs, int sec_id, TVector3 hit_position); // Hit import_clusters

      m_collector.importHit(index, "", -1, vector<int>(), vector<int>(), vector<int>(), 1, TVector3(), TVector3());

//     B2DEBUG ("HIT ID: " << akt_id);
    }
  }
}



void CollectorTestModule::import_cell_standard()
{

  // ID 0 - 9
  // Allowed Overlap => Cellid 2, 3 (same hit)
  // Not allowed overlap => Cellid 7, 8 (other hit)

  // Cellid 1 => overlapped Sector
  m_collector.importCell(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({1, 2}));
  m_collector.importCell(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({4, 5}));
  m_collector.importCell(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({11, 9}));
  m_collector.importCell(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({9, 10}));
  m_collector.importCell(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>());
  m_collector.importCell(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>());
  m_collector.importCell(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>());
  m_collector.importCell(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({15, 16}));
  m_collector.importCell(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>({16, 17}));
  m_collector.importCell(pass_sector_id_single, "", -1, vector<int>(), vector<int>(), vector<int>());

  //     virtual int importCell (int pass_index, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assigned_Hit_IDs); // Cell Import

}


void CollectorTestModule::import_cell_loop()
{

  int anz_cell = 10;

  vector<int> hits = {1, 2, 3};

  for (uint index = 0; index < pass_sector_ids.size(); index++) {

    for (int i = 0; i <  anz_cell; i++) {

      //     virtual int importCell (int pass_index, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assigned_Hit_IDs); // Cell Import

      m_collector.importCell(index, "", -1, vector<int>(), vector<int>(), hits);

//     B2DEBUG ("Cell ID: " << akt_id);
    }
  }
}

void CollectorTestModule::import_tfc_standard()
{

  // ID 1 = Sector overlapped
  // ID 2 = allowed overlapping (Cells)
  // ID 3, 4 = not allowed overlapping (Cells)
//      m_collector.importTC (pass_sector_id_single, "", vector<int>(), vector<int>(), vector<int>({0}));
//      m_collector.importTC (pass_sector_id_single, "", vector<int>(), vector<int>(), vector<int>({1}));
//      m_collector.importTC (pass_sector_id_single, "", vector<int>(), vector<int>(), vector<int>({2, 3}));
//      m_collector.importTC (pass_sector_id_single, "", vector<int>(), vector<int>(), vector<int>({7, 9}));
//      m_collector.importTC (pass_sector_id_single, "", vector<int>(), vector<int>(), vector<int>({8}));
//
//      m_collector.importTC (pass_sector_id_single, "", vector<int>(), vector<int>(), vector<int>({}));
//      m_collector.importTC (pass_sector_id_single, "", vector<int>(), vector<int>(), vector<int>({}));
//      m_collector.importTC (pass_sector_id_single, "", vector<int>(), vector<int>(), vector<int>({}));
//      m_collector.importTC (pass_sector_id_single, "", vector<int>(), vector<int>(), vector<int>({}));
//      m_collector.importTC (pass_sector_id_single, "", vector<int>(), vector<int>(), vector<int>({}));


}


void CollectorTestModule::import_tfc_loop()
{

  int anz_tfc = 10;
// vector<int> cells = {1, 2, 3};


  for (uint index = 0; index < pass_sector_ids.size(); index++) {

    for (int i = 0; i <  anz_tfc; i++) {

//     virtual int importTC (int pass_index, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assigned_Cell_IDs); // TC Import

//     int akt_id = m_collector.importTC (index, "", vector<int>(), vector<int>(), cells);

//     B2DEBUG ("TC ID: " << akt_id);
    }
  }
}


// Output of all interesting Information of Cells
void CollectorTestModule::getAllCells()
{

  for (uint i = 0; i <  m_collector.m_cellTF.size(); i++) {
    B2DEBUG(100, "* Cell ID: " << i << "; active: " << m_collector.m_cellTF[i].getActive() << ", died_at: " << m_collector.m_cellTF[i].getDiedAt() << "; m_assigned_hits_ids - size: " << m_collector.m_cellTF[i].getAssignedHits().size() << "; Neighbours - size: " << m_collector.m_cellTF[i].getNeighbours().size() << "; State: " << m_collector.m_cellTF[i].getState() <<
            "; use counter: " << m_collector.m_cellTF[i].getUseCounter());
  }

}


// Output of all interesting Information of Hits
void CollectorTestModule::getAllHits()
{

  for (uint i = 0; i <  m_collector.m_hitTF.size(); i++) {
    B2DEBUG(100, "* Hit ID: " << i << "; active: " << m_collector.m_hitTF[i].getActive() << ", died_at: " << m_collector.m_hitTF[i].getDiedAt() << "; m_assigned_cluster - size: " << m_collector.m_hitTF[i].getAssignedCluster().size() << "; UseTC IDs - size: " << m_collector.m_hitTF[i].getUseCounterTCIDs().size() <<
            "; use counter: " << m_collector.m_hitTF[i].getUseCounter() << "; SectorID: " << m_collector.m_hitTF[i].getSectorID());
  }

}

// Output of all interesting Information of Clusters
void CollectorTestModule::getAllClusters()
{

  for (uint i = 0; i <  m_collector.m_clustersTF.size(); i++) {
    B2DEBUG(100, "* Cluster ID: " << i << "; active: " << m_collector.m_clustersTF[i].getActive() << ", died_at: " << m_collector.m_clustersTF[i].getDiedAt() << "; Real Cluster ID: " <<
            m_collector.m_clustersTF[i].getRealClusterID() << "; Detector Type: "
            << m_collector.m_clustersTF[i].getDetectorType() << "; use counter: " << m_collector.m_clustersTF[i].getUseCounter()
            << "; ParticleID: " << m_collector.m_clustersTF[i].getParticleID()
           );
  }

}


// Output of all interesting Information of TC
void CollectorTestModule::getAllTC()
{

  for (uint i = 0; i <  m_collector.m_tfCandTF.size(); i++) {
    B2DEBUG(100, "* TC ID: " << i << "; active: " << m_collector.m_tfCandTF[i].getActive() << ", died_at: " << m_collector.m_tfCandTF[i].getDiedAt() << "; Own ID: " <<
            m_collector.m_tfCandTF[i].getOwnID() << "; AssignedCells - size: " <<
            m_collector.m_tfCandTF[i].getAssignedCell().size() <<
            "; isFitPossible: " << m_collector.m_tfCandTF[i].isFitPossible() <<
            "; assigned GTFC: " << m_collector.m_tfCandTF[i].getAssignedGFTC() <<
            "; Prob Value: " << m_collector.m_tfCandTF[i].getProbValue());
  }

}



// Output of all interesting Information of Sectors
void CollectorTestModule::getAllSectors()
{

  B2DEBUG(100, "* getAllSectors - Sectors size: " << m_collector.m_sectorTF.size());

  for (auto & akt_sector : m_collector.m_sectorTF) {
    B2DEBUG(100, "* Sector ID: " << akt_sector.second.getSectorID() << "; active: " << akt_sector.second.getActive() << ", died_at: " << akt_sector.second.getDiedAt() << "; is only friend: " <<
            akt_sector.second.getIsOnlyFriend() << "; Friends - size: "
            << akt_sector.second.getFriends().size() << "; use counter: " << akt_sector.second.getUseCounter());
  }


//          TVector3 m_points[4];
}



//TEST SILENT KILL
void CollectorTestModule::testSilentKill()
{

  import_sectors_standard();

  import_clusters_standard();

  import_hit_standard();

  //     virtual void updateHit (int hit_id, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter_TCID, std::vector<int> deltaUseCounter_cell); // Hit updateCell

  import_cell_standard();

//     for (int i = 0; i <  m_collector.m_cellTF.size(); i++) {
//  B2DEBUG ("cell => hits size: " << m_collector.m_cellTF[i].getAssignedHits().size() );
//
//     }


  //     virtual void updateCell (int cellID, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter_TCID, int changeCellState, std::vector<int> neighbours); // Cell Updates


  import_tfc_standard();

//    for (int i = 0; i <  m_collector.m_tfCandTF.size(); i++) {
//       B2DEBUG ("TfCand: " << i << "; active: " << m_collector.m_tfCandTF[i].getActive() << ", ass_cells - size: " << m_collector.m_tfCandTF[i].getAssignedCell().size() );
//    }
//

//     for (int i = 0; i <  m_collector.m_cellTF.size(); i++) {
//       B2DEBUG ("Before Silent Kill Cell ID: " << i << "; active: " << m_collector.m_cellTF[i].getActive() );
//    }
//
//    for (int i = 0; i <  m_collector.m_hitTF.size(); i++) {
//       B2DEBUG ("Before Silent Kill Hit ID: " << i << "; active: " << m_collector.m_hitTF[i].getActive());
//    }

//   for (int i = 0; i <  m_collector.m_clustersTF.size(); i++) {
//      B2DEBUG ("Before Silent Kill Cluster ID: " << i << "; active: " << m_collector.m_clustersTF[i].getActive() );
//   }

  m_collector.silentKill();

  int cells_live = 0;

  for (uint i = 0; i <  m_collector.m_cellTF.size(); i++) {
    cells_live += m_collector.m_cellTF[i].getActive();
    // B2DEBUG ("After Silent Kill  Cell ID: " << i << "; active: " << m_collector.m_cellTF[i].getActive() );
  }

  int hit_live = 0;
  for (uint i = 0; i <  m_collector.m_hitTF.size(); i++) {
    hit_live += m_collector.m_hitTF[i].getActive();
    // B2DEBUG ("After Silent Kill  Hit ID: " << i << "; active: " << m_collector.m_hitTF[i].getActive() << ", diet_at: " << m_collector.m_hitTF[i].getDiedAt() );
  }

  B2DEBUG(100, "hit_live " << hit_live);
  B2DEBUG(100, "cells_live: " << cells_live);

  // Active Hits after Silent Kill
  //ASSERT_EQ(10, hit_live );

  // Active Cells after Silent Kill
  //ASSERT_EQ(7, cells_live );

//     for (int i = 0; i <  m_collector.m_clustersTF.size(); i++) {
//       B2DEBUG ("After Silent Kill  Cluster ID: " << i << "; active: " << m_collector.m_clustersTF[i].getActive() );
//    }


}



//TESTs the overlapping logic
void CollectorTestModule::testOverlapped()
{

  import_sectors_standard();

  import_clusters_standard();

  import_hit_standard();

  import_cell_standard();

  import_tfc_standard();

  // Hit ID
  // ID 0 - 4
  // Sector Overlap => Hitid 3,4
  // Cluster Overlap => Hitid 7,8

  /*   ASSERT_EQ(false, m_collector.isHitOverlapped(1) );
     ASSERT_EQ(false, m_collector.isHitOverlapped(2) );
     ASSERT_EQ(true, m_collector.isHitOverlapped(3) );
     ASSERT_EQ(true, m_collector.isHitOverlapped(4) );
     ASSERT_EQ(true, m_collector.isHitOverlapped(7) );
     ASSERT_EQ(true, m_collector.isHitOverlapped(8) );
  */
  for (uint i = 0; i <  m_collector.m_hitTF.size(); i++) {
    B2DEBUG(100, "Hit ID: " << i << "; overlapped: " << m_collector.isHitOverlapped(i));
  }

  // Cell IDs
  // Allowed Overlap => Cellid 2, 3 (same hit)
  // Not allowed overlap => Cellid 7, 8 (other hit)
  /*
     ASSERT_EQ(false, m_collector.isCellOverlapped(2) );
     ASSERT_EQ(false, m_collector.isCellOverlapped(3) );
     ASSERT_EQ(true, m_collector.isCellOverlapped(7) );
     ASSERT_EQ(true, m_collector.isCellOverlapped(8) );
  */

  for (uint i = 0; i <  m_collector.m_cellTF.size(); i++) {
    B2DEBUG(100, "Cell ID: " << i << "; overlapped: " << m_collector.isCellOverlapped(i));
  }

  // TfCand
  // ID 1 = Sector overlapped
  // ID 2 = allowed overlapping (Cells)
  // ID 3, 4 = not allowed overlapping (Cells)
  /*
     ASSERT_EQ(true, m_collector.isTCOverlapped(1) );
     ASSERT_EQ(false, m_collector.isTCOverlapped(2) );
     ASSERT_EQ(true, m_collector.isTCOverlapped(3) );
     ASSERT_EQ(true, m_collector.isTCOverlapped(4) );  */

  for (uint i = 0; i <  m_collector.m_tfCandTF.size(); i++) {
    B2DEBUG(100, "TC ID: " << i << "; overlapped: " << m_collector.isTCOverlapped(i));
  }

  getAllSectors();
  getAllClusters();
  getAllHits();
  getAllCells();
  getAllTC();

  /* for (auto& akt_cluster:m_collector.m_cluster_start_info) {
     B2DEBUG ("pass index : " << akt_cluster.first.first << "; Detector ID : " << akt_cluster.first.second << "; start at: " << akt_cluster.second);
   }*/

}


void CollectorTestModule::testAllInformationStandard()
{
  import_sectors_standard();

  import_clusters_standard();

  m_collector.updateSectors(4, 0, "geloescht Sector ", 0, vector<int>(), vector<int>(), 0);

  m_collector.updateClusters(3, "geloescht Cluster", 0, vector<int>(), vector<int>(), 0);

//     ASSERT_EQ("geloescht", m_collector.m_clustersTF[3].getDiedAt() );
//     ASSERT_EQ(false, m_collector.m_clustersTF[3].getActive() );

  import_hit_standard();

  B2DEBUG(100, "*** SECTOR AFTER HIT Import : ");
  getAllSectors();

  m_collector.updateHit(1, "geloescht Hit ", 0, vector<int>(), vector<int>(), -1, -1, vector<int>());

//     ASSERT_EQ("geloescht", m_collector.m_hitTF[1].getDiedAt() );
//     ASSERT_EQ(false, m_collector.m_hitTF[1].getActive() );
//     ASSERT_EQ(true, m_collector.m_hitTF[2].getActive() );

  import_cell_standard();
  m_collector.updateCell(3, "geloescht Cell ", 1, vector<int>(), vector<int>(), -1, -1, 0, vector<int>({1, 2}));

//     ASSERT_EQ("geloescht", m_collector.m_cellTF[3].getDiedAt() );
//     ASSERT_EQ(false, m_collector.m_cellTF[3].getActive() );
//     ASSERT_EQ(true, m_collector.m_cellTF[1].getActive() );

  import_tfc_standard();

  m_collector.updateTC(4, "geloescht TC ", 4, vector<int>(), vector<int>());

  // int tcid, bool fit_successful, double probability_value, int assigned_GTFC
  m_collector.updateTCFitInformation(3, true, 0.5, 12);

//     ASSERT_EQ("geloescht", m_collector.m_tfCandTF[2].getDiedAt() );
//     ASSERT_EQ(false, m_collector.m_tfCandTF[2].getActive() );
//     ASSERT_EQ(true, m_collector.m_tfCandTF[1].getActive() );


  //m_collector.safeInformation ();

  getAllSectors();
  getAllClusters();
  getAllHits();
  getAllCells();
  getAllTC();

}


void CollectorTestModule::testAllInformationLoop()
{
  import_sectors_loop();

  import_clusters_loop();

  m_collector.updateSectors(4, 1, "geloescht", 0, vector<int>(), vector<int>(), 0);

  m_collector.updateClusters(3, "geloescht", 0, vector<int>(), vector<int>(), 0);

//     ASSERT_EQ("geloescht", m_collector.m_clustersTF[3].getDiedAt() );
//     ASSERT_EQ(false, m_collector.m_clustersTF[3].getActive() );

  import_hit_loop();

  m_collector.updateHit(1, "geloescht", 0, vector<int>(), vector<int>(), -1, -1, vector<int>());

//     ASSERT_EQ("geloescht", m_collector.m_hitTF[1].getDiedAt() );
//     ASSERT_EQ(false, m_collector.m_hitTF[1].getActive() );
//     ASSERT_EQ(true, m_collector.m_hitTF[2].getActive() );

  import_cell_loop();
  m_collector.updateCell(3, "geloescht", 1, vector<int>(), vector<int>(), -1, -1, 0, vector<int>({1, 2}));

//     ASSERT_EQ("geloescht", m_collector.m_cellTF[3].getDiedAt() );
//     ASSERT_EQ(false, m_collector.m_cellTF[3].getActive() );
//     ASSERT_EQ(true, m_collector.m_cellTF[1].getActive() );

  import_tfc_loop();

  m_collector.updateTC(2, "geloescht", 4, vector<int>(), vector<int>());

//     ASSERT_EQ("geloescht", m_collector.m_tfCandTF[2].getDiedAt() );
//     ASSERT_EQ(false, m_collector.m_tfCandTF[2].getActive() );
//     ASSERT_EQ(true, m_collector.m_tfCandTF[1].getActive() );


  //m_collector.safeInformation ();

}


