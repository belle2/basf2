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

#include <vector>
#include "VXDTFTrackCandidate.h"



namespace Belle2 {


  /** allows pass-independent overbooking check used by the VXDTFModule.
   * since especially 1D-SVD-Clusters are combined to 2D-Hits and the possibility to use several passes in the VXDTF, it is usefull to store
   * some linking information for each real cluster used.
   */
  class ClusterInfo {
  public:

    /** Empty constructor. */
    ClusterInfo():
      m_clusterIndex(-1),
      m_isPXD(false) {}

    /** Constructor. use this one, when having a sectormap (e.g. during track finding), use ThreeHitFilters when no sectormap is available */
    ClusterInfo(int clusterIndex, bool isPXD):
      m_clusterIndex(clusterIndex),
      m_isPXD(isPXD) {}


    /** Destructor. */
    ~ClusterInfo() {}

    /** adds a pointer to a track candidate using this cluster */
    void addTrackCandidate(VXDTFTrackCandidate* aTC);

    /** returns the index of the Real cluster this intermediate class is attached to */
    int getIndex() { return m_clusterIndex; }

    /** returns boolean wwhich says whether this intermediate class is attached to a PXD- or SVDCluster */
    bool isPXD() { return m_isPXD; }

    /** checks each TC whether it's alive or not. If there is more than one TC alive, it's overbooked and returned boolean is True*/
    bool isOverbooked();


  protected:
    /** internal shortcut for comparing 2 track candidates */
    bool isSameTC(const VXDTFTrackCandidate* a1, const VXDTFTrackCandidate* a2);

    std::vector<VXDTFTrackCandidate*> m_attachedTCs; /**< contains pointers to all attached TCs added with addTrackCandidate-memberfunction */
    int m_clusterIndex; /**< real index number of Cluster */
    bool m_isPXD; /**< is true if it's attached to an PXDCluster */

  }; //end class ClusterInfo
} //end namespace Belle2

#endif //CLUSTERINFO


