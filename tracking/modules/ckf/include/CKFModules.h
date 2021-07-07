/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/ckf/svd/findlets/CKFToSVDFindlet.h>
#include <tracking/ckf/pxd/findlets/CKFToPXDFindlet.h>
#include <tracking/ckf/vtx/findlets/CKFToVTXFindlet.h>
#include <tracking/ckf/vtx/findlets/CKFToVTXSeedFindlet.h>
#include <tracking/ckf/cdc/findlets/CKFToCDCFindlet.h>
#include <tracking/ckf/svd/findlets/CKFToSVDSeedFindlet.h>
#include <tracking/ckf/cdc/findlets/CKFToCDCFromEclFindlet.h>

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
      setDescription("Combinatorical Kalman Filter used for extrapolating CDC tracks into "
                     "SVD and create merged tracks.");
    }
  };

  /**
   * Seed-finding combinatorical Kalman Filter that combines every RecoTrack with every
   * SVD track, then filters the combinations.
   * All implementation is done in the corresponding findlet.
   */
  class CDCToSVDSeedCKFModule : public TrackFindingCDC::FindletModule<CKFToSVDSeedFindlet> {

  public:
    /// Set description
    CDCToSVDSeedCKFModule()
    {
      setDescription("Combinatorical Kalman Filter used for merging existing CDC tracks and SVD tracks.");
    }
  };

  /**
   * Combinatorical Kalman Filter that extrapolates every RecoTrack into the VTX
   * and collects space points.
   * All implementation is done in the corresponding findlet.
   */
  class CDCToVTXSpacePointCKFModule : public TrackFindingCDC::FindletModule<CKFToVTXFindlet> {

  public:
    /// Set description
    CDCToVTXSpacePointCKFModule()
    {
      setDescription("Combinatorical Kalman Filter used for extrapolating CDC tracks into "
                     "VTX, adding VTX hits, and create merged tracks.");
    }
  };

  /**
   * Seed-finding combinatorical Kalman Filter that combines every RecoTrack with every
   * VTX track, then filters the combinations.
   * All implementation is done in the corresponding findlet.
   */
  class CDCToVTXSeedCKFModule : public TrackFindingCDC::FindletModule<CKFToVTXSeedFindlet> {

  public:
    /// Set description
    CDCToVTXSeedCKFModule()
    {
      setDescription("Combinatorical Kalman Filter used for extrapolating CDC tracks into "
                     "VTX and create merged tracks.");
    }
  };

  /**
   * Combinatorical Kalman Filter that extrapolates every RecoTrack into the PXD
   * and collects space points.
   * All implementation is done in the corresponding findlet.
   */
  class ToPXDCKFModule : public TrackFindingCDC::FindletModule<CKFToPXDFindlet> {

  public:
    /// Set description
    ToPXDCKFModule()
    {
      setDescription("Combinatorical Kalman Filter used for extrapolating SVD/CDC tracks into "
                     "PXD and create merged tracks.");
    }
  };

  /**
   * Combinatorical Kalman Filter that extrapolates every RecoTrack into the CDC
   * and collects wire hits.
   * All implementation is done in the corresponding findlet.
   */
  class ToCDCCKFModule : public TrackFindingCDC::FindletModule<CKFToCDCFindlet> {

  public:
    /// Set description
    ToCDCCKFModule() : TrackFindingCDC::FindletModule<CKFToCDCFindlet>( {"CDCWireHitVector"})
    {
      setDescription("Combinatorical Kalman Filter used for extrapolating SVD tracks into "
                     "CDC and create merged tracks.");
    }
  };

  /**
   * Combinatorical Kalman Filter that extrapolates every ECLShower into the CDC
   * and collects wire hits.
   * All implementation is done in the corresponding findlet.
   */
  class ToCDCFromEclCKFModule : public TrackFindingCDC::FindletModule<CKFToCDCFromEclFindlet> {

  public:
    /// Set description
    ToCDCFromEclCKFModule() : TrackFindingCDC::FindletModule<CKFToCDCFromEclFindlet>( {"CDCWireHitVector"})
    {
      setDescription("Combinatorical Kalman Filter used for extrapolating ECL showers into "
                     "CDC and create merged tracks.");
    }
  };
}
