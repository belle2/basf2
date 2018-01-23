/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/ckf/svd/findlets/CKFToSVDFindlet.h>
#include <tracking/ckf/pxd/findlets/CKFToPXDFindlet.h>
#include <tracking/ckf/svd/findlets/CKFToSVDSeedFindlet.h>

#include <tracking/ckf/general/utilities/ClassMnemomics.h>
#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

namespace Belle2 {
  /**
   * Combinatorical Kalman Filter used for extrapolating CDC tracks into SVD and create merged tracks.
   * All implementation is done in the corresponding findlet.
   */
  class CDCToSVDSpacePointCKFModule : public TrackFindingCDC::FindletModule<CKFToSVDFindlet> {

  public:
    /// Set description
    CDCToSVDSpacePointCKFModule()
    {
      setDescription("Combinatorical Kalman Filter used for extracting CDC tracks into "
                     "SVD and create merged tracks.");
    }
  };

  class CDCToSVDSeedCKFModule : public TrackFindingCDC::FindletModule<CKFToSVDSeedFindlet> {

  public:
    /// Set description
    CDCToSVDSeedCKFModule()
    {
      setDescription("Combinatorical Kalman Filter used for extracting CDC tracks into "
                     "SVD and create merged tracks.");
    }
  };

  class ToPXDCKFModule : public TrackFindingCDC::FindletModule<CKFToPXDFindlet> {

  public:
    /// Set description
    ToPXDCKFModule()
    {
      setDescription("Combinatorical Kalman Filter used for extracting SVD/CDC tracks into "
                     "PXD and create merged tracks.");
    }
  };

}