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

  /** allows pass-independent overbooking check */
  class ClusterInfo {
  public:

    /** Empty constructor. */
    ClusterInfo() {}

    /** Constructor. use this one, when having a sectormap (e.g. during track finding), use ThreeHitFilters when no sectormap is available */
    ClusterInfo(int clusterIndex):
      m_clusterIndex(clusterIndex) {}


    /** Destructor. */
    ~ClusterInfo() {}

    void addTrackCandidate(VXDTFTrackCandidate* aTC);

    int getIndex() { return m_clusterIndex; }
    /*
    clusterInfos werden in nem Vector gespeichert -> clusterInfoIndex
    vxdtfhits bekommen clusterInfoIndex, tcs bekommen über vxdTFhits die Indices (eindeutig, da mit pxdNummer addiert) für Hopfield
    Tcs sagen ihren Clusterinfos, dass sie geowned sind (bekommen pointer auf ihre TCs, welche wissen ob sie noch leben oder nicht)

    */
    bool isOverbooked(); // check alle verlinkten TCs ob sie noch leben. Lebt mehr als einer -> überbucht


  protected:
    bool isSameTC(const VXDTFTrackCandidate* a1, const VXDTFTrackCandidate* a2);

    std::vector<VXDTFTrackCandidate*> m_attachedTCs;
    int m_clusterIndex; /**< index number of SVDCluster */

  }; //end class ClusterInfo
} //end namespace Belle2

#endif //CLUSTERINFO


