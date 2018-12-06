#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/trackFindingVXD/displayInterfaceTF/CollectorTFInfo.h>
#include <gtest/gtest.h>

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class CollectorTFInfoTest : public ::testing::Test {

    /**
     * Documentation Comment Jakob Lettenbichler:
     * this was written by a student and will be removed after finishing redesign of VXDTF.
     * Therefore some documentation-entries are not very useful - all of them are marked with "dummy comment".
     */
  public:
    void import_sectors_loop(); /**< dummy comment: import_sectors_loop */
    void import_sectors_standard(); /**< dummy comment: import_sectors_standard */

    void import_clusters_loop(); /**< dummy comment: import_clusters_loop */
    void import_clusters_standard(); /**< dummy comment: import_clusters_standard */

    void import_hit_loop(); /**< dummy comment: import_hit_loop */
    void import_hit_standard(); /**< dummy comment: import_hit_standard */

    void import_cell_loop(); /**< dummy comment: import_cell_loop */
    void import_cell_standard(); /**< dummy comment: import_cell_standard */

    void import_tfc_loop(); /**< dummy comment: import_tfc_loop */
    void import_tfc_standard(); /**< dummy comment: import_tfc_standard */

    void getAllCells(); /**< Output of all interesting Information of Cells */
    void getAllHits(); /**< Output of all interesting Information of Hits */
    void getAllClusters(); /**< Output of all interesting Information of Clusters */
    void getAllTC(); /**< Output of all interesting Information of TC */
    void getAllSectors(); /**< Output of all interesting Information of Sectors */


  protected:
    CollectorTFInfo m_collector = CollectorTFInfo();
    // Pass Ids
    std::vector<int> pass_sector_ids = {0, 1, 2};
    int pass_sector_id_single = 0;


  };


//**************  METHODES of the Collector
//       virtual void initSectors (std::map<std::pair<unsigned int, unsigned int>, std::vector<int>> sectors);
//     // Beginn Run, safe sectors here
//
//     virtual void initPersistent (); // Init Persistence
//     virtual void intEvent ();  // Clusters import, Sector fill
//
//     virtual int importCluster (int pass_index, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, int detector_type);
//
//     virtual void updateSectors (int sector_ID, int pass_index, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter); //Sector update
//     virtual void updateClusters (int cluster_id, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter);  //Cluster Update
//
//     virtual int importHit (int pass_index, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assigned_Cluster_IDs, int sec_id, TVector3 hit_position); // Hit import
//     virtual void updateHit (int hit_id, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter_TCID, std::vector<int> deltaUseCounter_cell); // Hit update
//
//     int getParticleIDfromRelations (int pass_index, int cluster_id);
//
//     virtual int importCell (int pass_index, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assigned_Hit_IDs); // Cell Import
//     virtual void updateCell (int cellID, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter_TCID, int changeCellState, std::vector<int> neighbours); // Cell Update
//
//     virtual int importTC (int pass_index, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assigned_Cell_IDs); // TC Import
//     virtual void updateTC (int tcid, std::string died_at, std::vector<int> accepted, std::vector<int> rejected); // TC Update
//
//     virtual void silentKill (); // Für alle folgenden Updates
//     virtual void safeInformation ();
//
//




  void CollectorTFInfoTest::import_sectors_standard()
  {


    uint sector_map_size = 40;

    // Create Test Sector
    std::map<std::pair<unsigned int, unsigned int>, std::vector<int>> sectors_display_all_pass;
    std::vector<int> sectors_display_friends;

//       B2INFO ("PassNr. " << pass_sector_id_single << "Size of Sector Map: " << sector_map_size );

    sectors_display_friends.clear();
    sectors_display_all_pass.insert(std::make_pair(std::make_pair(pass_sector_id_single, 0), sectors_display_friends));

    sectors_display_friends.clear();
    sectors_display_friends.push_back(0);
    sectors_display_all_pass.insert(std::make_pair(std::make_pair(pass_sector_id_single, 1), sectors_display_friends));

    sectors_display_friends.clear();
    sectors_display_friends.push_back(1);
    sectors_display_all_pass.insert(std::make_pair(std::make_pair(pass_sector_id_single, 2), sectors_display_friends));

    // Sector 3 - 40
    sectors_display_friends.clear();
    for (uint u = 3; u < sector_map_size; u++) {
      sectors_display_all_pass.insert(std::make_pair(std::make_pair(pass_sector_id_single, u), sectors_display_friends));
    }




    m_collector.initPersistent();
    m_collector.intEvent();
    // m_collector.initSectors (sectors_display_all_pass, std::vector<double>(), std::vector<double>());

  }


  void CollectorTFInfoTest::import_clusters_standard()
  {

    uint anz_clusters_svd = 15;
    uint anz_clusters_pxd = 15;


    for (uint i = 0; i <  anz_clusters_svd; i++) {
      // importCluster (int pass_index, std::string died_at, int accepted, int rejected, int detector_type)
      /*int akt_id =*/ m_collector.importCluster(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), Const::SVD, i);

//      B2INFO ("AKT ID SVD: " << akt_id);

    }

    for (uint i = 0; i <  anz_clusters_pxd; i++) {
      // importCluster (int pass_index, std::string died_at, int accepted, int rejected, int detector_type)
      /*int akt_id = */m_collector.importCluster(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), Const::PXD, i);

//      B2INFO ("AKT ID PXD: " << akt_id);
    }
  }


  void CollectorTFInfoTest::import_sectors_loop()
  {
    // Create Test Sector
    std::map<unsigned int, std::vector<int>> sector_map;
    uint size_of_sectors = 10;

    for (uint i = 0; i < size_of_sectors; i++) {
      sector_map.insert(std::make_pair(i, std::vector<int>()));
    }

    // 1. Sectors init
    //KeySectors dosn't function => so pair Int int
    std::map<std::pair<unsigned int, unsigned int>, std::vector<int>> sectors_display_all_pass;
    std::vector<int> sectors_display_friends;

    for (uint i = 0; i < pass_sector_ids.size(); i++) {

      B2DEBUG(1, "PassNr. " << i << "Size of Sector Map: " << sector_map.size());

      for (auto& akt_sector : sector_map) {
        sectors_display_friends.clear();

        // Friends read and store in second vector
        for (auto& akt_friend : akt_sector.second) {
          sectors_display_friends.push_back(akt_friend);
        }
        sectors_display_all_pass.insert(std::make_pair(std::make_pair(i, akt_sector.first), sectors_display_friends));
      }
    }
    m_collector.initPersistent();
    m_collector.intEvent();
    //  m_collector.initSectors (sectors_display_all_pass, std::vector<double>(), std::vector<double>());
  }


  void CollectorTFInfoTest::import_clusters_loop()
  {
    uint anz_clusters_svd = 10;
    uint anz_clusters_pxd = 10;

    for (uint index = 0; index < pass_sector_ids.size(); index++) {
      for (uint i = 0; i <  anz_clusters_svd; i++) {
        // importCluster (int pass_index, std::string died_at, int accepted, int rejected, int detector_type)
        /*int akt_id =*/
        m_collector.importCluster(index, "", -1, std::vector<int>(), std::vector<int>(), Const::SVD, i);
      }
      for (uint i = 0; i <  anz_clusters_pxd; i++) {
        // importCluster (int pass_index, std::string died_at, int accepted, int rejected, int detector_type)
        /*int akt_id =*/
        m_collector.importCluster(index, "", -1, std::vector<int>(), std::vector<int>(), Const::PXD, i);
      }
    }
  }

  void CollectorTFInfoTest::import_hit_standard()
  {
    //     virtual int importHit (int pass_index, std::string died_at, std::vector<int> accepted,
    //                            std::vector<int> rejected, std::vector<int> assigned_Cluster_IDs,
    //                            int sec_id, TVector3 hit_position); // Hit import_clusters

    // ID 0 - 4
    // Sector Overlap => Hitid 3,4
    // Cluster Overlap => Hitid 7,8
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({1, 2, 3}), 0,
                          TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({4}), 2, TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({7}), 4, TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({9}), 5, TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({10}), 5, TVector3(),
                          TVector3());

    // ID 5 - 9
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({11, 12, 13}), 6,
                          TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({14, 15}), 7,
                          TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({17}), 8, TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({17}), 9, TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({18}), 1, TVector3(),
                          TVector3());

    // ID 10 - 14
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>(), -1, TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>(), -1, TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>(), -1, TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>(), -1, TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>(), -1, TVector3(),
                          TVector3());

    // ID 15 - 19
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>(), 19, TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>(), 20, TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>(), 21, TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>(), 22, TVector3(),
                          TVector3());
    m_collector.importHit(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>(), 23, TVector3(),
                          TVector3());
  }


  void CollectorTFInfoTest::import_hit_loop()
  {
    uint anz_hits = 10;

    // std::vector<int> clusters = {7, 8, 9};
    for (uint index = 0; index < pass_sector_ids.size(); index++) {
      for (uint i = 0; i <  anz_hits; i++) {
        //     virtual int importHit (int pass_index, std::string died_at, std::vector<int> accepted,
        //                            std::vector<int> rejected, std::vector<int> assigned_Cluster_IDs,
        //                            int sec_id, TVector3 hit_position); // Hit import_clusters

        /*int akt_id =*/ m_collector.importHit(index, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>(), 1, TVector3(),
                                               TVector3());
      }
    }
  }


  void CollectorTFInfoTest::import_cell_standard()
  {
    // ID 0 - 9
    // Allowed Overlap => Cellid 2, 3 (same hit)
    // Not allowed overlap => Cellid 7, 8 (other hit)

    // Cellid 1 => overlapped Sector
    m_collector.importCell(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({1, 2}));
    m_collector.importCell(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({4, 5}));
    m_collector.importCell(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({11, 9}));
    m_collector.importCell(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({9, 10}));
    m_collector.importCell(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>());
    m_collector.importCell(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>());
    m_collector.importCell(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>());
    m_collector.importCell(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({15, 16}));
    m_collector.importCell(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>({16, 17}));
    m_collector.importCell(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), std::vector<int>());

    //     virtual int importCell (int pass_index, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assigned_Hit_IDs); // Cell Import
  }


  void CollectorTFInfoTest::import_cell_loop()
  {
    uint anz_cell = 10;
    std::vector<int> hits = {1, 2, 3};

    for (uint index = 0; index < pass_sector_ids.size(); index++) {
      for (uint i = 0; i <  anz_cell; i++) {
        //     virtual int importCell (int pass_index, std::string died_at, std::vector<int> accepted,
        //                             std::vector<int> rejected, std::vector<int> assigned_Hit_IDs); // Cell Import
        /*int akt_id =*/ m_collector.importCell(index, "", -1, std::vector<int>(), std::vector<int>(), hits);
      }
    }
  }

  void CollectorTFInfoTest::import_tfc_standard()
  {
    std::vector<std::pair<int, unsigned int>>  cells0 = {std::make_pair(0, 0)};
    std::vector<std::pair<int, unsigned int>>  cells1 = {std::make_pair(1, 1)};
    std::vector<std::pair<int, unsigned int>>  cells23 = {std::make_pair(2, 2), std::make_pair(3, 3)};
    std::vector<std::pair<int, unsigned int>>  cells79 = {std::make_pair(7, 7), std::make_pair(9, 9)};
    std::vector<std::pair<int, unsigned int>>  cells8 = {std::make_pair(8, 8)};

    // ID 1 = Sector overlapped
    // ID 2 = allowed overlapping (Cells)
    // ID 3, 4 = not allowed overlapping (Cells)
    m_collector.importTC(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), cells0);
    m_collector.importTC(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), cells1);
    m_collector.importTC(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), cells23);
    m_collector.importTC(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), cells79);
    m_collector.importTC(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(), cells8);

    m_collector.importTC(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(),
                         std::vector<std::pair<int, unsigned int>>({}));
    m_collector.importTC(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(),
                         std::vector<std::pair<int, unsigned int>>({}));
    m_collector.importTC(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(),
                         std::vector<std::pair<int, unsigned int>>({}));
    m_collector.importTC(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(),
                         std::vector<std::pair<int, unsigned int>>({}));
    m_collector.importTC(pass_sector_id_single, "", -1, std::vector<int>(), std::vector<int>(),
                         std::vector<std::pair<int, unsigned int>>({}));
  }


  void CollectorTFInfoTest::import_tfc_loop()
  {
    uint anz_tfc = 10;
    std::vector<std::pair<int, unsigned int>>  cells = {std::make_pair(1, 1), std::make_pair(2, 2), std::make_pair(3, 3)};

    for (uint index = 0; index < pass_sector_ids.size(); index++) {
      for (uint i = 0; i <  anz_tfc; i++) {
        // int passIndex, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected,
        // const std::vector<std::pair<int, unsigned int>> assignedCellIDs
        // TC Import
        /*int akt_id =*/ m_collector.importTC(index, "", -1, std::vector<int>(), std::vector<int>(), cells);
      }
    }
  }


  // Output of all interesting Information of Cells
  void CollectorTFInfoTest::getAllCells()
  {
    for (uint i = 0; i <  m_collector.m_cellTF.size(); i++) {
      B2INFO("* Cell ID: " << i << "; active: " << m_collector.m_cellTF[i].getActive() << ", died_at: " <<
             m_collector.m_cellTF[i].getDiedAt() << "; m_assigned_hits_ids - size: " << m_collector.m_cellTF[i].getAssignedHits().size() <<
             "; Neighbours - size: " << m_collector.m_cellTF[i].getNeighbours().size() << "; State: " << m_collector.m_cellTF[i].getState() <<
             "; use counter: " << m_collector.m_cellTF[i].getUseCounter());
    }
  }


  // Output of all interesting Information of Hits
  void CollectorTFInfoTest::getAllHits()
  {
    for (uint i = 0; i <  m_collector.m_hitTF.size(); i++) {
      B2INFO("* Hit ID: " << i << "; active: " << m_collector.m_hitTF[i].getActive() << ", died_at: " <<
             m_collector.m_hitTF[i].getDiedAt() << "; m_assigned_cluster - size: " << m_collector.m_hitTF[i].getAssignedCluster().size() <<
             "; UseTC IDs - size: " << m_collector.m_hitTF[i].getUseCounterTCIDs().size() <<
             "; use counter: " << m_collector.m_hitTF[i].getUseCounter() << "; SectorID: " << m_collector.m_hitTF[i].getSectorID());
    }
  }

  // Output of all interesting Information of Clusters
  void CollectorTFInfoTest::getAllClusters()
  {
    for (uint i = 0; i <  m_collector.m_clustersTF.size(); i++) {
      B2INFO("* Cluster ID: " << i << "; active: " << m_collector.m_clustersTF[i].getActive() << ", died_at: " <<
             m_collector.m_clustersTF[i].getDiedAt() << "; Real Cluster ID: " <<
             m_collector.m_clustersTF[i].getRealClusterID() << "; Detector Type: "
             << m_collector.m_clustersTF[i].getDetectorType() << "; use counter: " << m_collector.m_clustersTF[i].getUseCounter());
    }
  }


  // Output of all interesting Information of TC
  void CollectorTFInfoTest::getAllTC()
  {
    for (uint i = 0; i <  m_collector.m_tfCandTF.size(); i++) {
      B2INFO("* TC ID: " << i << "; active: " << m_collector.m_tfCandTF[i].getActive() << ", died_at: " <<
             m_collector.m_tfCandTF[i].getDiedAt() << "; Own ID: " <<
             m_collector.m_tfCandTF[i].getOwnID() << "; AssignedCells - size: " <<
             m_collector.m_tfCandTF[i].getAssignedCell().size());
    }
  }


  // Output of all interesting Information of Sectors
  void CollectorTFInfoTest::getAllSectors()
  {
    for (auto& akt_sector : m_collector.m_sectorTF) {
      B2INFO("* Sector ID: " << akt_sector.second.getSectorID() << "; active: " << akt_sector.second.getActive() << ", died_at: " <<
             akt_sector.second.getDiedAt() << "; is only friend: " <<
             akt_sector.second.getIsOnlyFriend() << "; Friends - size: "
             << akt_sector.second.getFriends().size() << "; use counter: " << akt_sector.second.getUseCounter());
    }
  }


  /*
  //TEST SILENT KILL
  TEST_F(CollectorTFInfoTest, testSilentKill){

    import_sectors_standard();

    import_clusters_standard();

    import_hit_standard ();

    //     virtual void updateHit (int hit_id, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter_TCID, std::vector<int> deltaUseCounter_cell); // Hit updateCell

    import_cell_standard();

  //     for (int i = 0; i <  m_collector.m_cellTF.size(); i++) {
  //  B2INFO ("cell => hits size: " << m_collector.m_cellTF[i].getAssignedHits().size() );
  //
  //     }


    //     virtual void updateCell (int cellID, std::string died_at, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter_TCID, int changeCellState, std::vector<int> neighbours); // Cell Updates


    import_tfc_standard();

  //    for (int i = 0; i <  m_collector.m_tfCandTF.size(); i++) {
  //       B2INFO ("TfCand: " << i << "; active: " << m_collector.m_tfCandTF[i].getActive() << ", ass_cells - size: " << m_collector.m_tfCandTF[i].getAssignedCell().size() );
  //    }
  //

  //     for (int i = 0; i <  m_collector.m_cellTF.size(); i++) {
  //       B2INFO ("Before Silent Kill Cell ID: " << i << "; active: " << m_collector.m_cellTF[i].getActive() );
  //    }
  //
  //    for (int i = 0; i <  m_collector.m_hitTF.size(); i++) {
  //       B2INFO ("Before Silent Kill Hit ID: " << i << "; active: " << m_collector.m_hitTF[i].getActive());
  //    }

  //   for (int i = 0; i <  m_collector.m_clustersTF.size(); i++) {
  //      B2INFO ("Before Silent Kill Cluster ID: " << i << "; active: " << m_collector.m_clustersTF[i].getActive() );
  //   }

    m_collector.silentKill();

    int cells_live = 0;

     for (uint i = 0; i <  m_collector.m_cellTF.size(); i++) {
       cells_live += m_collector.m_cellTF[i].getActive();
       // B2INFO ("After Silent Kill  Cell ID: " << i << "; active: " << m_collector.m_cellTF[i].getActive() );
    }

    int hit_live = 0;
    for (uint i = 0; i <  m_collector.m_hitTF.size(); i++) {
      hit_live += m_collector.m_hitTF[i].getActive();
     // B2INFO ("After Silent Kill  Hit ID: " << i << "; active: " << m_collector.m_hitTF[i].getActive() << ", diet_at: " << m_collector.m_hitTF[i].getDiedAt() );
   }

   B2INFO ("hit_live " << hit_live);
   B2INFO ("cells_live: " << cells_live);

    // Active Hits after Silent Kill
    ASSERT_EQ(10, hit_live );

    // Active Cells after Silent Kill
    ASSERT_EQ(7, cells_live );

  //     for (int i = 0; i <  m_collector.m_clustersTF.size(); i++) {
  //       B2INFO ("After Silent Kill  Cluster ID: " << i << "; active: " << m_collector.m_clustersTF[i].getActive() );
  //    }


  }

  */

//     //TESTs the overlapping logic
//   TEST_F(CollectorTFInfoTest, testOverlapped){
//
//     import_sectors_standard();
//
//     import_clusters_standard();
//
//     import_hit_standard ();
//
//     import_cell_standard();
//
//     import_tfc_standard();
//
//     // Hit ID
//     // ID 0 - 4
//     // Sector Overlap => Hitid 3,4
//     // Cluster Overlap => Hitid 7,8
//
//     bool false_item = false;
//     bool true_item = true;
//
//     ASSERT_EQ(false_item, m_collector.isHitOverlapped(1) );
//     ASSERT_EQ(false_item, m_collector.isHitOverlapped(2) );
//     ASSERT_EQ(true_item, m_collector.isHitOverlapped(3) );
//     ASSERT_EQ(true_item, m_collector.isHitOverlapped(4) );
//     ASSERT_EQ(true_item, m_collector.isHitOverlapped(7) );
//     ASSERT_EQ(true_item, m_collector.isHitOverlapped(8) );
//
//     for (uint i = 0; i <  m_collector.m_hitTF.size(); i++) {
//       B2INFO ("Hit ID: " << i << "; overlapped: " << m_collector.isHitOverlapped(i));
//     }
//
//    // Cell IDs
//    // Allowed Overlap => Cellid 2, 3 (same hit)
//    // Not allowed overlap => Cellid 7, 8 (other hit)
//
//     ASSERT_EQ(false_item, m_collector.isCellOverlapped(2) );
//     ASSERT_EQ(false_item, m_collector.isCellOverlapped(3) );
//     ASSERT_EQ(true_item, m_collector.isCellOverlapped(7) );
//     ASSERT_EQ(true_item, m_collector.isCellOverlapped(8) );
//
//
//     for (uint i = 0; i <  m_collector.m_cellTF.size(); i++) {
//       B2INFO ("Cell ID: " << i << "; overlapped: " << m_collector.isCellOverlapped(i));
//     }
//
//      // TfCand
//      // ID 1 = Sector overlapped
//      // ID 2 = allowed overlapping (Cells)
//      // ID 3, 4 = not allowed overlapping (Cells)
//
//     ASSERT_EQ(true_item, m_collector.isTCOverlapped(1) );
//     ASSERT_EQ(false_item, m_collector.isTCOverlapped(2) );
//     ASSERT_EQ(true_item, m_collector.isTCOverlapped(3) );
//     ASSERT_EQ(true_item, m_collector.isTCOverlapped(4) );
//
//     for (uint i = 0; i <  m_collector.m_tfCandTF.size(); i++) {
//       B2INFO ("TC ID: " << i << "; overlapped: " << m_collector.isTCOverlapped(i));
//     }
//
// //     getAllSectors ();
// //     getAllClusters ();
// //     getAllHits ();
// //     getAllCells();
// //     getAllTC ();
//
//
//
//   }
//

  /** dummy comment: testAllInformationLoop */
  TEST_F(CollectorTFInfoTest, testAllInformationLoop)
  {
    bool false_item = false;
    bool true_item = true;

    import_sectors_loop();

    import_clusters_loop();

    m_collector.updateSectors(4, 1, "geloescht", 0, std::vector<int>(), std::vector<int>(), 0);

    m_collector.updateClusters(3, "geloescht", 0, std::vector<int>(), std::vector<int>(), 0);

    ASSERT_EQ("geloescht", m_collector.m_clustersTF[3].getDiedAt());
    ASSERT_EQ(false_item, m_collector.m_clustersTF[3].getActive());

    import_hit_loop();

    m_collector.updateHit(1, "geloescht", 0, std::vector<int>(), std::vector<int>(), -1, -1, std::vector<int>());

    ASSERT_EQ("geloescht", m_collector.m_hitTF[1].getDiedAt());
    ASSERT_EQ(false_item, m_collector.m_hitTF[1].getActive());
    ASSERT_EQ(true_item, m_collector.m_hitTF[2].getActive());

    import_cell_loop();
    m_collector.updateCell(3, "geloescht", 2, std::vector<int>(), std::vector<int>(), -1, -1, 0, std::vector<int>());

    ASSERT_EQ("geloescht", m_collector.m_cellTF[3].getDiedAt());
    ASSERT_EQ(false_item, m_collector.m_cellTF[3].getActive());
    ASSERT_EQ(true_item, m_collector.m_cellTF[1].getActive());

    import_tfc_loop();

    m_collector.updateTC(2, "geloescht", 4, std::vector<int>(), std::vector<int>());

    ASSERT_EQ("geloescht", m_collector.m_tfCandTF[2].getDiedAt());
    ASSERT_EQ(false_item, m_collector.m_tfCandTF[2].getActive());
    ASSERT_EQ(true_item, m_collector.m_tfCandTF[1].getActive());
  }
}
