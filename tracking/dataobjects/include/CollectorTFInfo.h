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
   *    m_outputFlag (int) = flag for kind and sum of output data  (not in use at the moment)
   *  m_sectorTFAll (map: KeySectors, SectorTFInfo) = all Sectors (init. at initSectors)
   *  m_sectorTF (map: KeySectors, SectorTFInfo) = only activated (used) Sectors
   *  m_clustersTF (vector: ClusterTFInfo) = Clusters
   *  m_hitTF (vector: HitTFInfo) = Hits
   *  m_cellTF (vector: CellTFInfo) = Cells
   *  m_tfCandTF (vector: TrackCandidateTFInfo) = Track Finder Candidates
   *
   *  Important Methodes:
   *  initSectors = all Sectors saved for all events in m_sectorTFAll
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
    int getOutputFlag()  { return m_outputFlag; }

    /** setter - output Flag */
    void setOutputFlag(int value) { m_outputFlag = value; }

    /** Sectors safe for all events */
    virtual void initSectors(const std::vector< std::pair< std::pair<unsigned int, unsigned int>, std::vector<unsigned int> > >& sectors, const std::vector<double>& secConfigU, const std::vector<double>& secConfigV);

    /** registerPersistence (StoreArrays & RelationArray) for the Collector */
    virtual void initPersistent();  // Init Persistence

    /** Clear all vectors */
    virtual void intEvent();

    /** Import of a cluster */
    virtual int importCluster(int passIndex, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, int detectorType, int relativePosition);

    /** Sectors update after update / import Hit
    Reload Sector if not in current Sectors
    Also checks Friends sectors (possible reload) */
    virtual void updateSectors(unsigned int sectorID, int passIndex, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter);  //Sector update

    /** Cluster Update */
    virtual void updateClusters(int clusterId, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, int deltaUseCounter);   //Cluster Update

    /** Import Hit (return hitID) */
    virtual int importHit(int passIndex, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assignedClusterIDs, int secId, TVector3 hitPosition, TVector3 hitSigma);  // Hit import

    /** Hit Update */
    virtual void updateHit(int hitId, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, int addTCID, int removeTCID, std::vector<int> deltaUseCounterCell);  // Hit update

    /** Cell Import */
    virtual int importCell(int passIndex, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, std::vector<int> assignedHitIDs);  // Cell Import

    /** Update Cell */
    virtual void updateCell(int cellID, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, int addTCID, int removeTCID, int changeCellState, std::vector<int> neighbours);  // Cell Update

    /** TC Import, return = tc id */
    virtual int importTC(int passIndex, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected, const std::vector<std::pair<int, unsigned int>> assignedCellIDs);  // TC Import

    /** update TCand */
    virtual void updateTC(int tcid, std::string diedAt, int diedId, std::vector<int> accepted, std::vector<int> rejected);  // TC Update

    /** Update Fit Information (fitSuccessful, probabilityValue, assignedGTFC) of TC */
    virtual void updateTCFitInformation(int tcid, bool fitSuccessful, double probabilityValue, int assignedGTFC);

    /** Information stored in StoreArrays and RelationArrays */
    virtual void safeInformation();

    /** Checks if there are not connected Clusters, Cells and Hits and deactivates them */
    virtual void silentKill();

    /** Check if the Hit is overlapped */
    virtual bool isHitOverlapped(int hitId);

    /** Check if the Cell is overlapped */
    virtual bool isCellOverlapped(int cellID);

    /** Check if the TC is overlapped */
    virtual bool isTCOverlapped(int tcid);

    int m_outputFlag;    /**< Flag = kind and sum of output data  (not in use at the moment) */

    // => private (only public for testing)
    //protected:

    std::map<KeySectors, SectorTFInfo> m_sectorTFAll;    /**< all Sectors */

    std::map<KeySectors, SectorTFInfo> m_sectorTF;    /**< only activated Sectors */

    std::vector<ClusterTFInfo> m_clustersTF;      /**< used Clusters */
    std::vector<HitTFInfo> m_hitTF;       /**< used Hits */
    std::vector<CellTFInfo> m_cellTF;       /**< used Cells */
    std::vector<TrackCandidateTFInfo> m_tfCandTF;   /**< used TCand */

    // IDs & Strings for diedAt
    const static std::string m_nameHitFinder;   /**< string name of HitFinder*/
    const static std::string m_nameCellFinder;    /**< string name of CellFinder*/
    const static std::string m_nameNbFinder;    /**< string name of NbFinder*/
    const static std::string m_nameCA;      /**< string name of CA*/
    const static std::string m_nameTCC;     /**< string name of TCC*/
    const static std::string m_nameOverlap;   /**< string name of Overlap*/
    const static std::string m_nameQI;      /**< string name of QI*/
    const static std::string m_nameHopfield;    /**< string name of Hopfield*/

    const static int m_idHitFinder;   /**< id of HitFinder*/
    const static int m_idCellFinder;  /**< id of CellFinder*/
    const static int m_idNbFinder;  /**< id of NbFinder*/
    const static int m_idCA;    /**< id of CA*/
    const static int m_idTCC;   /**< id of TCC*/
    const static int m_idOverlap; /**< id of Overlap*/
    const static int m_idQI;    /**< id of QI*/
    const static int m_idHopfield;  /**< id of Hopfield*/
    const static int m_idAlive;   /**< id of Alive*/


  private:


    ClassDef(CollectorTFInfo, 1)
  };
}
