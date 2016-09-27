#pragma once
// ******************************************************************
// RestOfEvent utility
// authors: M. Lubej (matic.lubej@ijs.si)
// ******************************************************************

#include <framework/dataobjects/Helix.h>
#include <TVector3.h>

namespace Belle2 {

  /**
   * Functions that are used in calculating the distance from an ECLCluster to the nearest track at the ECL cylindrical radius.
   */
  namespace C2TDistanceUtility {

    /**
     * Clip the ECLCluster position vector from the point inside the ECL region to the inside surface of the ECL region
     */
    TVector3 clipECLClusterPosition(TVector3 v1);

    /**
     * Calculate the position on the track at the ECL cylindrical radius in the barrel region or at the cylindrical radius of the point in the endcap regions.
     */
    TVector3 getECLTrackHitPosition(const Helix& helix, const TVector3& v1);

  }
}
