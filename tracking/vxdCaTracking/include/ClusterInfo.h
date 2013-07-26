/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef CLUSTERINFO_H
#define CLUSTERINFO_H

#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>

// #include <vector>
#include <list>
#include "VXDTFTrackCandidate.h"



namespace Belle2 {

  namespace Tracking {


    /** allows pass-independent overbooking check used by the VXDTFModule.
    * since especially 1D-SVD-Clusters are combined to 2D-Hits and the possibility to use several passes in the VXDTF, it is usefull to store
    * some linking information for each real cluster used (and a Pointer to the cluster)
    */
    class ClusterInfo {
    public:
//      typedef std::vector<VXDTFTrackCandidate*> TcContainer; /**< contains attached track candidates */
      typedef std::list<VXDTFTrackCandidate*> TcContainer; /**< contains attached track candidates */

      /** Empty constructor. */
      ClusterInfo():
        m_bossTC(NULL),
        m_clusterIndex(-1),
        m_ownPositionInIndex(-1),
        m_isPXD(false),
        m_reserved(false),
        m_pxdCluster(NULL),
        m_svdCluster(NULL) {}

      /** Constructor. use this one, when having a sectormap (e.g. during track finding), use ThreeHitFilters when no sectormap is available */
      ClusterInfo(int clusterIndex, int ownIndex, bool isPXD, const PXDCluster* pCluster, const SVDCluster* sCluster):
        m_bossTC(NULL),
        m_clusterIndex(clusterIndex),
        m_ownPositionInIndex(ownIndex),
        m_isPXD(isPXD),
        m_reserved(false),
        m_pxdCluster(pCluster),
        m_svdCluster(sCluster) {}


      /** Destructor. */
      ~ClusterInfo() {}

      /** removes a track candidate from container */
      void removeTrackCandidate(VXDTFTrackCandidate* aTC) {
        TcContainer::iterator tcIt = m_attachedTCs.begin();
        for (; tcIt != m_attachedTCs.end(); ++tcIt) {
          if (isSameTC((*tcIt), aTC) == true) { tcIt = m_attachedTCs.erase(tcIt); return; }
        }
      }

      /** adds a pointer to a track candidate using this cluster */
      void addTrackCandidate(VXDTFTrackCandidate* aTC);

      /** returns the index of the Real cluster this intermediate class is attached to */
      int getRealIndex() const { return m_clusterIndex; }

      /** returns the index number of this intermediate class in the container which stores all the ClusterInfos */
      int getOwnIndex() const { return m_ownPositionInIndex; }

      /** returns boolean wwhich says whether this intermediate class is attached to a PXD- or SVDCluster */
      bool isPXD() const { return m_isPXD; }

      /** returns pointer to the PXDCluster, is NULL if ->this is a SVDCluster */
      const PXDCluster* getPXDCluster() const { return m_pxdCluster; }

      /** returns pointer to the SVDCluster, is NULL if ->this is a SVDCluster */
      const SVDCluster* getSVDCluster() const { return m_svdCluster; }

      /** checks each TC whether it's alive or not. If there is more than one TC alive, it's overbooked and returned boolean is True*/
      bool isOverbooked();

      /** ClusterInfo shall be set reserved, if a final TC for a pass is using it (a pointer to that TC is inputParameter), returning parameter tells whether this was successfull or not */
      bool setReserved(VXDTFTrackCandidate* newBossTC);

      /** is true, if an accepted TC of a pass is using it. This means that no other TC is allowed to use it any more */
      bool isReserved() const { return m_reserved; }

      /** is true, if the TC has already reserved this Cluster. This means that no other TC is allowed to use it any more */
      bool checkReserved(VXDTFTrackCandidate* newBossTC) const;

    protected:
      /** internal shortcut for comparing 2 track candidates */
      bool isSameTC(const VXDTFTrackCandidate* a1, const VXDTFTrackCandidate* a2) {
        if (a1 == a2) { return true; } else { return false; }
      }

      TcContainer m_attachedTCs; /**< contains pointers to all attached TCs added with addTrackCandidate-memberfunction */
      VXDTFTrackCandidate* m_bossTC; /**< if Cluster is reserved, here is the pointer to it stored. Is NULL, if not reserved */
      int m_clusterIndex; /**< real index number of Cluster */
      int m_ownPositionInIndex; /**< index number of its own in the container which stores all the ClusterInfos */
      bool m_isPXD; /**< is true if it's attached to an PXDCluster */
      bool m_reserved; /**< means that an accepted TC uses this cluster and therefore no other TC is allowed to use it anymore */
      const PXDCluster* m_pxdCluster; /**< if isPXD = true, this member contains the pointer to the parenting PXDCluster */
      const SVDCluster* m_svdCluster; /**< if isPXD = false, this member contains the pointer to the parenting SVDCluster */

    }; //end class ClusterInfo
  } // end namespace Tracking
} //end namespace Belle2

#endif //CLUSTERINFO
