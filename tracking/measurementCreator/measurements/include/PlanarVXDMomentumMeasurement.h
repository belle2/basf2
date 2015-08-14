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

#include <tracking/measurementCreator/measurements/PlanarMomentumMeasurement.h>
#include <tracking/vxdMomentumEstimation/VXDMomentumEstimation.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/dataobjects/Helix.h>


namespace Belle2 {


  /** @brief Measurement class implementing a planar hit geometry (1 or 2D) with only a momentum measurement.
   *
   */
  template <class HitType>
  class PlanarVXDMomentumMeasurement : public PlanarMomentumMeasurement {

  public:
    PlanarVXDMomentumMeasurement(const genfit::PlanarMeasurement& parentElement, HitType* hit,
                                 const typename VXDMomentumEstimation<HitType>::FitParameters& fitParameters,
                                 const typename VXDMomentumEstimation<HitType>::CorrectionFitParameters& correctionFitParameters,
                                 bool useTrackFinderSeeds, bool useThickness) :
      PlanarMomentumMeasurement(parentElement),
      m_fitParameters(fitParameters),
      m_correctionFitParameters(correctionFitParameters),
      m_useTrackFinderSeeds(useTrackFinderSeeds),
      m_useThickness(useThickness),
      m_hit(hit)
    {
      rawHitCoords_.ResizeTo(1);
      rawHitCov_.ResizeTo(1, 1);
    }

    virtual genfit::AbsMeasurement* clone() const {return new PlanarVXDMomentumMeasurement(*this);}

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
    /** Underlaying hit/cluster */
    HitType* m_hit;
  };

  template <class HitType>
  std::vector<genfit::MeasurementOnPlane*> PlanarVXDMomentumMeasurement<HitType>::constructMeasurementsOnPlane(
    const genfit::StateOnPlane& state) const
  {
    const VXDMomentumEstimation<HitType>& momentumEstimation = VXDMomentumEstimation<HitType>::getInstance();

    TVectorD rawHitCoordinates(1);
    TMatrixDSym rawHitCovariance(1);

    const TVector3& position(state.getPos());
    const TVector3& momentum(state.getMom());
    short charge = state.getCharge();

    Helix helix(position, momentum, charge, 1.5);

    if (m_useThickness) {
      rawHitCoordinates(0) = momentumEstimation.estimateQOverPWithThickness(*m_hit, charge, m_fitParameters, m_correctionFitParameters);
    } else {
      if (m_useTrackFinderSeeds) {
        rawHitCoordinates(0) = momentumEstimation.estimateQOverP(*m_hit, helix.getMomentum(1.5), helix.getPerigee(), charge,
                                                                 m_fitParameters,
                                                                 m_correctionFitParameters);
      } else {
        MCParticle* relatedMCParticle = m_hit->template getRelated<MCParticle>("MCParticles");

        if (relatedMCParticle == nullptr) {
          B2WARNING("Hit has no related MCParticle!")
          rawHitCoordinates(0) = momentumEstimation.estimateQOverP(*m_hit, helix.getMomentum(1.5), helix.getPerigee(), charge,
                                                                   m_fitParameters,
                                                                   m_correctionFitParameters);
        } else {
          const TVector3& momentumMC = relatedMCParticle->getMomentum();
          const TVector3& positionMC = relatedMCParticle->getProductionVertex();
          const short chargeMC = relatedMCParticle->getCharge();

          rawHitCoordinates(0) = momentumEstimation.estimateQOverP(*m_hit, momentumMC, positionMC, chargeMC, m_fitParameters,
                                                                   m_correctionFitParameters);
        }
      }
    }

    // TODO
    rawHitCovariance(0, 0) = 0.23;

    genfit::MeasurementOnPlane* mop = new genfit::MeasurementOnPlane(rawHitCoordinates, rawHitCovariance,
        state.getPlane(), state.getRep(), constructHMatrix(state.getRep()));
    return {mop};
  }

}
