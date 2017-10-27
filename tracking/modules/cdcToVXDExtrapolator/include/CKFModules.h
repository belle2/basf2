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
#include <tracking/ckf/findlets/cdcToSVDSpacePoint/CDCToSVDSpacePointCKFFindlet.h>
#include <tracking/ckf/findlets/pxdSpacePoint/PXDSpacePointCKFFindlet.h>
#include <tracking/ckf/findlets/vxdToCDC/VXDToCDCCKFFindlet.h>

#include <tracking/ckf/utilities/ClassMnemomics.h>
#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

namespace Belle2 {
  /**
   * Combinatorical Kalman Filter used for extrapolating CDC tracks into SVD and create merged tracks.
   * All implementation is done in the corresponding findlet.
   */
  class CDCToSVDSpacePointCKFModule : public TrackFindingCDC::FindletModule<CDCToSVDSpacePointCKFFindlet> {

  public:
    /// Set description
    CDCToSVDSpacePointCKFModule()
    {
      setDescription("Combinatorical Kalman Filter used for extracting CDC tracks into "
                     "SVD and create merged tracks.");
    }
  };

  /**
   * Combinatorical Kalman Filter used for extrapolating all tracks into PXD and create merged tracks.
   * All implementation is done in the corresponding findlet.
   */
  class PXDSpacePointCKFModule : public TrackFindingCDC::FindletModule<PXDSpacePointCKFFindlet> {

  public:
    /// Set description
    PXDSpacePointCKFModule()
    {
      setDescription("Combinatorical Kalman Filter used for extracting tracks into "
                     "PXD and create merged tracks.");
    }
  };

  /**
   * Combinatorical Kalman Filter used for extrapolating VXD tracks into CDC and create merged tracks.
   * All implementation is done in the corresponding findlet.
   */
  class VXDToCDCWireHitCKFModule : public TrackFindingCDC::FindletModule<VXDToCDCCKFFindlet> {

  public:
    /// Set description
    VXDToCDCWireHitCKFModule() : TrackFindingCDC::FindletModule<VXDToCDCCKFFindlet>( {"CDCWireHitVector"})
    {
      setDescription("Combinatorical Kalman Filter used for extracting VXD tracks into "
                     "CDC and create merged tracks.");
    }
  };
}