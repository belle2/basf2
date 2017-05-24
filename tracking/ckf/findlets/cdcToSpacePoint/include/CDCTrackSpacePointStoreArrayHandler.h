/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/ckf/findlets/base/StoreArrayHandler.h>
#include <tracking/ckf/states/CKFCDCToVXDStateObject.h>

namespace Belle2 {
  /**
   * Derived store array handler for CDC RecoTrack seeds and SpacePoints.
   *
   * This findlet is responsible for the interface between the DataStore and the CKF modules:
   *  * to fetch the reco tracks and the space points from the StoreArray (fetch)
   *  * to write back the found VXD tracks only and the merged tracks (CDC + VXD) (apply)
   */
  class CDCTrackSpacePointStoreArrayHandler : public StoreArrayHandler<CKFCDCToVXDStateObject> {
    using Super = StoreArrayHandler<CKFCDCToVXDStateObject>;

  public:
    /**
     * Write back the found tracks (VXD only and the merged ones).
     */
    void apply(const std::vector<CKFCDCToVXDStateObject::ResultObject>& cdcTracksWithMatchedSpacePoints) override;
  };
}
