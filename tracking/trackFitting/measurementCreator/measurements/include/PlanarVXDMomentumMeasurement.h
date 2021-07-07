/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFitting/measurementCreator/measurements/PlanarMomentumMeasurement.h>
#include <tracking/vxdMomentumEstimation/VXDMomentumEstimation.h>
#include <tracking/vxdMomentumEstimation/VXDMomentumEstimationTools.h>
#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  /**
   * Measurement class implementing a planar hit geometry (1 or 2D) with a momentum measurement based on the VXD dEdX information
   * with setable parameters (see VXDMomentumEstimationMeasurementCreator).
   */
  template <class HitType>
  class PlanarVXDMomentumMeasurement : public PlanarMomentumMeasurement {

  public:
    /**
     * Initialize the momentum measurement with a planar measurement as a parent, the corresponding hit and the reco track to
     * get information from. The internal matrices are set accordingly.
     */
    PlanarVXDMomentumMeasurement(const genfit::PlanarMeasurement& parentElement, HitType* hit, const Belle2::RecoTrack* recoTrack) :
      PlanarMomentumMeasurement(parentElement), m_hit(hit), m_recoTrack(recoTrack)
    {
      rawHitCoords_.ResizeTo(1);
      rawHitCov_.ResizeTo(1, 1);

      setHitId(hit->getArrayIndex());
      setDetId(getDetId());
    }

    /** Clone the measurement. */
    virtual genfit::AbsMeasurement* clone() const override {return new PlanarVXDMomentumMeasurement(*this);}

    /** Construct the measurement on the plane set in the parent element. */
    virtual std::vector<genfit::MeasurementOnPlane*> constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const override;

    /** Set the correction fit parameters. */
    void setCorrectionFitParameters(const typename VXDMomentumEstimation<HitType>::CorrectionFitParameters& correctionFitParameters)
    {
      m_correctionFitParameters = correctionFitParameters;
    }

    /** Set the measurement sigma. */
    void setSigma(double sigma)
    {
      m_sigma = sigma;
    }

    /** Set the fit parameters. */
    void setFitParameters(const typename VXDMomentumEstimation<HitType>::FitParameters& fitParameters)
    {
      m_fitParameters = fitParameters;
    }

    /** Set whether to use mc information. */
    void setUseMCInformation(bool useMCInformation)
    {
      m_useMCInformation = useMCInformation;
    }

    /** Set whether to use the thickness. */
    void setUseThickness(bool useThickness)
    {
      m_useThickness = useThickness;
    }

    /** Set whether to use tracking seeds not the current state. */
    void setUseTrackingSeeds(bool useTrackingSeeds)
    {
      m_useTrackingSeeds = useTrackingSeeds;
    }

    /** Get the underlaying hit (cluster). */
    HitType* getHit() const
    {
      return m_hit;
    }

  private:
    /** Parameters for the main function. */
    typename VXDMomentumEstimation<HitType>::FitParameters m_fitParameters;
    /** Parameters for the correction function. Set them to zero to not use a correction function. */
    typename VXDMomentumEstimation<HitType>::CorrectionFitParameters m_correctionFitParameters;
    /** Use the thickness of the clusters of the path length for estimating dX. */
    bool m_useThickness = false;
    /** Use the seeds of the track finder or the seeds of the MC particles. */
    bool m_useMCInformation = true;
    /** Use the tracking seeds in the origin for calculating the path length rather than the current state. */
    bool m_useTrackingSeeds = false;
    /** Value of the measurement sigma. */
    double m_sigma = 0.03;

    /** Underlaying hit/cluster. */
    HitType* m_hit = nullptr;
    /** RecoTrack for which the hit is created. */
    const RecoTrack* m_recoTrack = nullptr;

    /// Return the detector ID.
    int getDetId() const
    {
      return -1;
    }
  };

  /** Do the job: construct a measurement based on the estimator chosen by the parameters.
   * TODO: Implement better error handling and a smarter cut.
   */
  template <class HitType>
  std::vector<genfit::MeasurementOnPlane*> PlanarVXDMomentumMeasurement<HitType>::constructMeasurementsOnPlane(
    const genfit::StateOnPlane& state) const
  {
    const VXDMomentumEstimation<HitType>& momentumEstimator = VXDMomentumEstimation<HitType>::getInstance();
    const VXDMomentumEstimationTools<HitType>& momentumEstimationTools = VXDMomentumEstimationTools<HitType>::getInstance();

    TVectorD rawHitCoordinates(1);
    TMatrixDSym rawHitCovariance(1);

    // Copy the information from the current state
    const TVector3& statePosition(state.getPos());
    const TVector3& stateMomentum(state.getMom());
    short stateCharge = state.getCharge();


    // Copy the information from the reco track.
    const TVector3& trackPosition(m_recoTrack->getPositionSeed());
    const TVector3& trackMomentum(m_recoTrack->getMomentumSeed());
    short trackCharge = m_recoTrack->getChargeSeed();

    // Copy the information from the mc particle (if there is one)
    MCParticle* relatedMCParticle = m_hit->template getRelated<MCParticle>("MCParticles");

    TVector3 mcMomentum;
    TVector3 mcPostion;
    short mcCharge;

    if (relatedMCParticle == nullptr) {
      mcMomentum = trackMomentum;
      mcPostion = trackPosition;
      mcCharge = trackCharge;
    } else {
      mcMomentum = relatedMCParticle->getMomentum();
      mcPostion = relatedMCParticle->getProductionVertex();
      mcCharge = relatedMCParticle->getCharge();
    }

    // Copy information from the mc hit
    const TVector3& hitMCMomentum = momentumEstimationTools.getEntryMomentumOfMCParticle(*m_hit);
    const TVector3& hitMCPosition = momentumEstimationTools.getEntryPositionOfMCParticle(*m_hit);


    if (m_useThickness) {
      rawHitCoordinates(0) = momentumEstimator.estimateQOverPWithThickness(*m_hit, stateCharge, m_fitParameters,
                             m_correctionFitParameters);
    } else {
      if (m_useTrackingSeeds) {
        if (m_useMCInformation) {
          rawHitCoordinates(0) = momentumEstimator.estimateQOverP(*m_hit, mcMomentum, mcPostion, mcCharge,
                                                                  m_fitParameters,
                                                                  m_correctionFitParameters);

        } else {
          rawHitCoordinates(0) = momentumEstimator.estimateQOverP(*m_hit, trackMomentum, trackPosition, trackCharge,
                                                                  m_fitParameters,
                                                                  m_correctionFitParameters);
        }
      } else {
        if (m_useMCInformation) {
          rawHitCoordinates(0) = momentumEstimator.estimateQOverP(*m_hit, hitMCMomentum, hitMCPosition, stateCharge,
                                                                  m_fitParameters,
                                                                  m_correctionFitParameters);
        } else {
          rawHitCoordinates(0) = momentumEstimator.estimateQOverP(*m_hit, stateMomentum, statePosition, stateCharge,
                                                                  m_fitParameters,
                                                                  m_correctionFitParameters);
        }
      }
    }

    rawHitCovariance(0, 0) = m_sigma;

    genfit::MeasurementOnPlane* mop = new genfit::MeasurementOnPlane(rawHitCoordinates, rawHitCovariance,
        state.getPlane(), state.getRep(), constructHMatrix(state.getRep()));
    return {mop};
  }

  /** Specialisation for PXD clusters. */
  template<>
  int PlanarVXDMomentumMeasurement<PXDCluster>::getDetId() const;

  /** Specialisation for SVD clusters. */
  template<>
  int PlanarVXDMomentumMeasurement<SVDCluster>::getDetId() const;

}
