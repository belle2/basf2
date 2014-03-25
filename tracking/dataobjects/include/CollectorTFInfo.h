/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Ferstl                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <TObject.h>
#include <TVector3.h>
#include <map>
#include <vector>
#include <algorithm>

#include <tracking/dataobjects/SectorTFInfo.h>
#include <tracking/dataobjects/ClusterTFInfo.h>
#include <tracking/dataobjects/HitTFInfo.h>
#include <tracking/dataobjects/CellTFInfo.h>
#include <tracking/dataobjects/TrackCandidateTFInfo.h>

#include <string>

namespace Belle2 {
  /** Analysis TF Info Class
   *
   *  This class is needed for information transport between the VXD Track Finder and the display.
   *
   *  Members:
   *    m_output_flag (int) = flag for kind and sum of output data  (not in use at the moment)
   *  m_sectorTF_all (map: KeySectors, SectorTFInfo) = all Sectors (init. at initSectors)
   *  m_sectorTF (map: KeySectors, SectorTFInfo) = only activated (used) Sectors
   *  m_cluster_start_info (map: KeyClusters, int) = Information for the different kinds of clusters (Startposition)
   *  m_clustersTF (vector: ClusterTFInfo) = Clusters
   *  m_hitTF (vector: HitTFInfo) = Hits
   *  m_cellTF (vector: CellTFInfo) = Cells
   *  m_tfCandTF (vector: TrackCandidateTFInfo) = Track Finder Candidates
   *
   *  Important Methodes:
   *  initSectors = all Sectors saved for all events in m_sectorTF_all
   *  initPersistent = registerPersistent for StoreArrays and Relations
   *  intEvent = clear Vectors for the Event
   *
   *  importCluster = Imports 1 Cluster, also checks for new startinformation of Clusters
   *    updateSectors = Updates 1 Sector, reloads Sector (if necessary) from all to current Sectors, also checks
   *        Friends sectors also with reload option
   *  updateClusters = Updates 1 Cluster
   *  importHit = Imports 1 Hit, also updates assigned Sector and Clusters
   *  updateHit = Updates 1 Hit, also updates assigned Sector and Clusters
   *  importCell = Imports 1 Cell, also updates assigned Hits
   *  updateCell = Updates 1 cell (also possible neighbours-insert) also updates assigned Hits
   *  importTC = Imports 1 TC, also updates assigned Cells
   *  updateTC = Updates 1 TC, also updates assigned Cells
   *  updateTCFitInformation = Updates 1 TC with Informations of Fit, Probability, and assigned GFTC
   *
   *  silentKill = Checks if there are not connected Clusters, Cells and Hits and deactivates them
   *  safeInformation = Stores the Informations of an Event in StoreArrays and RelationArrays
   *
   *  isHitOverlapped = Checks if the Hit is overlapped (overlapped = true)
   *  isCellOverlapped = Checks if the Cell is overlapped (overlapped = true)
   *  isTCOverlapped = Checks if the TC is overlapped (overlapped = true)
   */

  class CollectorTFInfo: public TObject {

    friend class CollectorTFInfoTest;

  public:

    typedef std::pair<unsigned int, unsigned int> KeySectors;     /**< Key to Sectors */
    typedef std::pair<unsigned int, unsigned int> KeyClusters;  /**< Key to Cluster */

    /** Default constructor for the ROOT IO. */
    CollectorTFInfo();
    virtual ~CollectorTFInfo();

    /** getter - output Flag */
    int getOutputFlag()  { return m_output_flag; }

    /** setter - output Flag */
    void setOutputFlag(int value) { m_output_flag = value; }

    /** Sectors safe for all events */
    virtual void initSectors(std::map < std::pair<unsigned int, unsigned int>, std::vector<int> > sectors, std::vector<double> secConfigU, std::vector<double> secConfigV);

    /** registerPersistence (StoreArrays & RelationArray) for the Collector */
    virtual void initPersistent();  // Init Persistence

    /** Clear all vectors */
    virtual void intEvent();

    /** Import of a cluster */
    virtual int importCluster(int pass_index, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, int detector_type, int relative_position);

    /** Sectors update after update / import Hit
    Reload Sector if not in current Sectors
    Also checks Friends sectors (possible reload) */
    virtual void updateSectors(int sector_ID, int pass_index, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter);  //Sector update

    /** Cluster Update */
    virtual void updateClusters(int cluster_id, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter);   //Cluster Update

    /** Import Hit (return hitID) */
    virtual int importHit(int pass_index, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assigned_Cluster_IDs, int sec_id, TVector3 hit_position, TVector3 hit_sigma);  // Hit import

    /** Hit Update */
    virtual void updateHit(int hit_id, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, int add_TCID, int remove_TCID, std::vector<int> deltaUseCounter_cell);  // Hit update

    /** Cell Import */
    virtual int importCell(int pass_index, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assigned_Hit_IDs);  // Cell Import

    /** Update Cell */
    virtual void updateCell(int cellID, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, int add_TCID, int remove_TCID, int changeCellState, std::vector<int> neighbours);  // Cell Update

    /** TC Import, return = tc id */
    virtual int importTC(int pass_index, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected, std::vector<std::pair<int, unsigned int>> assigned_Cell_IDs);  // TC Import

    /** update TCand */
    virtual void updateTC(int tcid, std::string died_at, int died_id, std::vector<int> accepted, std::vector<int> rejected);  // TC Update

    /** Update Fit Information (fit_successful, probability_value, assigned_GTFC) of TC */
    virtual void updateTCFitInformation(int tcid, bool fit_successful, double probability_value, int assigned_GTFC);

    /** Information stored in StoreArrays and RelationArrays */
    virtual void safeInformation();

    /** Checks if there are not connected Clusters, Cells and Hits and deactivates them */
    virtual void silentKill();

    /** Check if the Hit is overlapped */
    virtual bool isHitOverlapped(int hit_id);

    /** Check if the Cell is overlapped */
    virtual bool isCellOverlapped(int cellID);

    /** Check if the TC is overlapped */
    virtual bool isTCOverlapped(int tcid);

    int m_output_flag;    /**< Flag = kind and sum of output data  (not in use at the moment) */

    // => private (only public for testing)
    //protected:

    std::map<KeySectors, SectorTFInfo> m_sectorTF_all;    /**< all Sectors */

    std::map<KeySectors, SectorTFInfo> m_sectorTF;    /**< only activated Sectors */

    std::vector<ClusterTFInfo> m_clustersTF;      /**< used Clusters */
    std::vector<HitTFInfo> m_hitTF;       /**< used Hits */
    std::vector<CellTFInfo> m_cellTF;       /**< used Cells */
    std::vector<TrackCandidateTFInfo> m_tfCandTF;   /**< used TCand */

    // IDs & Strings for died_at
    const static std::string nameHitFinder;   /**< string name of HitFinder*/
    const static std::string nameCellFinder;    /**< string name of CellFinder*/
    const static std::string nameNbFinder;    /**< string name of NbFinder*/
    const static std::string nameCA;      /**< string name of CA*/
    const static std::string nameTCC;     /**< string name of TCC*/
    const static std::string nameOverlap;   /**< string name of Overlap*/
    const static std::string nameQI;      /**< string name of QI*/
    const static std::string nameHopfield;    /**< string name of Hopfield*/

    const static int idHitFinder;   /**< id of HitFinder*/
    const static int idCellFinder;  /**< id of CellFinder*/
    const static int idNbFinder;  /**< id of NbFinder*/
    const static int idCA;    /**< id of CA*/
    const static int idTCC;   /**< id of TCC*/
    const static int idOverlap; /**< id of Overlap*/
    const static int idQI;    /**< id of QI*/
    const static int idHopfield;  /**< id of Hopfield*/
    const static int idAlive;   /**< id of Alive*/


  private:


    ClassDef(CollectorTFInfo, 1)
  };
}
