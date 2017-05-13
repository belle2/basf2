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
   *
   */
  class CDCTrackSpacePointStoreArrayHandler : public StoreArrayHandler<CKFCDCToVXDStateObject> {
    using Super = StoreArrayHandler<CKFCDCToVXDStateObject>;

  public:
    /**
     */
    void apply(const std::vector<CKFCDCToVXDStateObject::ResultObject>& cdcTracksWithMatchedSpacePoints) override;
  };
}
