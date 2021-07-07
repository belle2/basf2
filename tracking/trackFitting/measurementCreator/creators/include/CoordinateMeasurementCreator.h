/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFitting/measurementCreator/creators/BaseMeasurementCreatorFromHit.h>
#include <tracking/dataobjects/RecoHitInformation.h>

namespace Belle2 {
  class RecoTrack;

  /** A measurement creator for normal coordinate measurements out of cdc/svd/pxd/vtx hits. */
  template <class HitType, Const::EDetector detector>
  class CoordinateMeasurementCreator : public BaseMeasurementCreatorFromHit<HitType, detector> {
  public:
    /** Constructor */
    explicit CoordinateMeasurementCreator(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory);

    /** Create measurements based on coordinate measurements. */
    std::vector<genfit::TrackPoint*> createMeasurementPoints(HitType* hit, RecoTrack& recoTrack,
                                                             const RecoHitInformation& recoHitInformation) const override;
    /** Destructor. */
    virtual ~CoordinateMeasurementCreator() { }
  };

  /** Needed for templating. */
  /// Hit to reco hit measurement creator for the CDC.
  using CDCCoordinateMeasurementCreator = CoordinateMeasurementCreator<RecoHitInformation::UsedCDCHit, Const::CDC>;
  /// Hit to reco hit measurement creator for the SVD.
  using SVDCoordinateMeasurementCreator = CoordinateMeasurementCreator<RecoHitInformation::UsedSVDHit, Const::SVD>;
  /// Hit to reco hit measurement creator for the PXD.
  using PXDCoordinateMeasurementCreator = CoordinateMeasurementCreator<RecoHitInformation::UsedPXDHit, Const::PXD>;
  /// Hit to reco hit measurement creator for the VTX.
  using VTXCoordinateMeasurementCreator = CoordinateMeasurementCreator<RecoHitInformation::UsedVTXHit, Const::VTX>;
  /// Hit to reco hit measurement creator for the BKLM.
  using BKLMCoordinateMeasurementCreator = CoordinateMeasurementCreator<RecoHitInformation::UsedBKLMHit, Const::BKLM>;
  /// Hit to reco hit measurement creator for the EKLM.
  using EKLMCoordinateMeasurementCreator = CoordinateMeasurementCreator<RecoHitInformation::UsedEKLMHit, Const::EKLM>;
}
