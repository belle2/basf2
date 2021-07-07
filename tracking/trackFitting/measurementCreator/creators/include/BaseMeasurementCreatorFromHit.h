/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFitting/measurementCreator/creators/BaseMeasurementCreator.h>
#include <tracking/dataobjects/RecoHitInformation.h>

#include <genfit/MeasurementFactory.h>

#include <framework/gearbox/Const.h>
#include <vector>

namespace genfit {
  class AbsMeasurement;
}

namespace Belle2 {
  class RecoTrack;

  /**
   * Base Class to create measurements based on a given hit related to the RecoTrack.
   * Use this class as a base class if you want to add TrackPoints with measurements
   * that are created for each svd/pxd/vtx/cdc hit.
   * If you do not relate to a specific hit from the track finders (for example for ECL energy
   * measurements as a TrackPoint) you should not use this class as a base class but rather use
   * BaseMeasurementCreator directly.
   *
   * TODO: It would be better to skip the genfit::MeasurementFactory completely and just implement the conversion here directly!
   */
  template <class HitType, Const::EDetector detector>
  class BaseMeasurementCreatorFromHit : public BaseMeasurementCreator {
  public:
    /** Creates a MeasurementCreator which handles the creation of measurements of a given kind. **/
    explicit BaseMeasurementCreatorFromHit(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory);

    /** Destructor **/
    virtual ~BaseMeasurementCreatorFromHit() { }

    /** Overload this method to create measurement track points from a given hit. **/
    virtual std::vector<genfit::TrackPoint*> createMeasurementPoints(HitType* hit, RecoTrack& recoTrack,
        const RecoHitInformation& recoHitInformation) const = 0;

  protected:
    /**
     * You probably need a coordinate measurement on which you can base your measurements in createMeasurements.
     * This function uses the measurementFactory to create one.
     * Please be aware that this creates two new objects on the heap: the measurement and the track point. If you do not plan to
     * use those two in a track, please delete them! The track point can be deleted by accessing the
     * measurements GetTrackPoints function.
     * @param hit
     * @param recoHitInformation
     * @return a coordinate AbsMeasurement as pointer.
     */
    genfit::AbsMeasurement* createCoordinateMeasurement(HitType* hit, const RecoHitInformation& recoHitInformation) const;

    /** Helper: Create a TrackPoint from a measurement with a given RecoHitInformation. */
    genfit::TrackPoint* createTrackPointWithRecoHitInformation(genfit::AbsMeasurement* coordinateMeasurement,
                                                               RecoTrack& recoTrack,
                                                               const RecoHitInformation& recoHitInformation) const;

  private:
    /** A reference to the prefilled measurement factory. */
    const genfit::MeasurementFactory<genfit::AbsMeasurement>& m_measurementFactory;

    /** We do not need this method in this overload. */
    std::vector<genfit::TrackPoint*> createMeasurementPoints(RecoTrack&) const override final
    {
      return {};
    }
  };

  /** Needed for templating. */
  /// Standard base class for CDC measurement creators.
  using CDCBaseMeasurementCreator = BaseMeasurementCreatorFromHit<RecoHitInformation::UsedCDCHit, Const::CDC>;
  /// Standard base class for SVD measurement creators.
  using SVDBaseMeasurementCreator = BaseMeasurementCreatorFromHit<RecoHitInformation::UsedSVDHit, Const::SVD>;
  /// Standard base class for PXD measurement creators.
  using PXDBaseMeasurementCreator = BaseMeasurementCreatorFromHit<RecoHitInformation::UsedPXDHit, Const::PXD>;
  /// Standard base class for VTX measurement creators.
  using VTXBaseMeasurementCreator = BaseMeasurementCreatorFromHit<RecoHitInformation::UsedVTXHit, Const::VTX>;
  /// Standard base class for BKLM measurement creators.
  using BKLMBaseMeasurementCreator = BaseMeasurementCreatorFromHit<RecoHitInformation::UsedBKLMHit, Const::BKLM>;
  /// Standard base class for EKLM measurement creators.
  using EKLMBaseMeasurementCreator = BaseMeasurementCreatorFromHit<RecoHitInformation::UsedEKLMHit, Const::EKLM>;

  /** CDC hits are a special case, as they must be WireTrackCandHits, not TrackCandHits. **/
  template<>
  genfit::AbsMeasurement*
  CDCBaseMeasurementCreator::createCoordinateMeasurement(RecoHitInformation::UsedCDCHit* cdcHit,
                                                         const RecoHitInformation& recoHitInformation) const;
}
