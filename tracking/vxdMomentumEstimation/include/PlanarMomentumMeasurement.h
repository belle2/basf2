/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <genfit/PlanarMeasurement.h>
#include <tracking/vxdMomentumEstimation/VXDMomentumEstimation.h>
#include <genfit/Exception.h>
#include <genfit/RKTrackRep.h>
#include <tracking/vxdMomentumEstimation/HMatrixQP.h>
#include <cassert>

#include <framework/logging/Logger.h>


namespace Belle2 {


  /** @brief Measurement class implementing a planar hit geometry (1 or 2D) with only a momentum measurement.
   *
   */
  template <class HitType>
  class PlanarMomentumMeasurement : public genfit::PlanarMeasurement {

  public:
    PlanarMomentumMeasurement(const genfit::PlanarMeasurement& parentElement, HitType* hit,
                              const typename VXDMomentumEstimation<HitType>::FitParameters& fitParameters,
                              const typename VXDMomentumEstimation<HitType>::CorrectionFitParameters& correctionFitParameters,
                              bool useTrackFinderSeeds, bool useThickness) :
      genfit::PlanarMeasurement(parentElement),
      m_fitParameters(fitParameters),
      m_correctionFitParameters(correctionFitParameters),
      m_useTrackFinderSeeds(useTrackFinderSeeds),
      m_useThickness(useThickness),
      m_hit(hit)
    {
      rawHitCoords_.ResizeTo(1);
      rawHitCov_.ResizeTo(1, 1);
    }

    virtual genfit::AbsMeasurement* clone() const {return new PlanarMomentumMeasurement(*this);}

    virtual const genfit::AbsHMatrix* constructHMatrix(const genfit::AbsTrackRep*) const;

    virtual std::vector<genfit::MeasurementOnPlane*> constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const override;

  private:
    /** Parameters for the main function */
    typename VXDMomentumEstimation<HitType>::FitParameters m_fitParameters;
    /** Parameters for the correction function. Set them to zero to not use a correction function */
    typename VXDMomentumEstimation<HitType>::CorrectionFitParameters m_correctionFitParameters;
    /** Use the seeds of the track finder or the seeds of the MC particles */
    bool m_useTrackFinderSeeds = true;
    /** Use the thickness of the clusters of the path length for estimating dX */
    bool m_useThickness = false;
    /** Minimal value for the momentum below the estimation is used */
    double m_minimumMomentum = 0.1;
    /** Underlaying hit/cluster */
    HitType* m_hit;
  };

  template <class HitType>
  const genfit::AbsHMatrix* PlanarMomentumMeasurement<HitType>::constructHMatrix(const genfit::AbsTrackRep* rep) const
  {
    if (dynamic_cast<const genfit::RKTrackRep*>(rep) == NULL) {
      B2FATAL("PlanarMomentumMeasurement default implementation can only handle state vectors of type RKTrackRep!");
    }

    return new HMatrixQP();
  }

  template <class HitType>
  std::vector<genfit::MeasurementOnPlane*> PlanarMomentumMeasurement<HitType>::constructMeasurementsOnPlane(
    const genfit::StateOnPlane& state) const
  {
    const VXDMomentumEstimation<HitType>& momentumEstimation = VXDMomentumEstimation<HitType>::getInstance();

    TVectorD rawHitCoordinates(1);
    TMatrixDSym rawHitCovariance(1);

    const TVector3& position(state.getPos());
    const TVector3& momentum(state.getMom());
    short charge = state.getCharge();

    if (momentum.Mag() > m_minimumMomentum) {
      return {};
    }

    if (m_useThickness) {
      rawHitCoordinates(0) = momentumEstimation.estimateQOverPWithThickness(*m_hit, charge, m_fitParameters, m_correctionFitParameters);
    } else {
      if (m_useTrackFinderSeeds) {
        rawHitCoordinates(0) = momentumEstimation.estimateQOverP(*m_hit, momentum, position, charge, m_fitParameters,
                                                                 m_correctionFitParameters);
      } else {
        rawHitCoordinates(0) = momentumEstimation.estimateQOverP(*m_hit, momentum, position, charge, m_fitParameters,
                                                                 m_correctionFitParameters);
      }
    }

    // TODO
    rawHitCovariance(0, 0) = 0.23;

    if (rawHitCoordinates(0) > m_minimumMomentum) {
      return {};
    }

    genfit::MeasurementOnPlane* mop = new genfit::MeasurementOnPlane(rawHitCoordinates, rawHitCovariance,
        state.getPlane(), state.getRep(), constructHMatrix(state.getRep()));
    return {mop};
  }

}
