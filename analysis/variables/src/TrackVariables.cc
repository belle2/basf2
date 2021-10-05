/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own includes
#include <analysis/variables/TrackVariables.h>
#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/Helix.h>

// dataobjects from the MDST
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/ECLCluster.h>

// framework aux
#include <framework/logging/Logger.h>

#include <cmath>

namespace Belle2 {
  namespace Variable {

    static const double realNaN = std::numeric_limits<double>::quiet_NaN();
    static const TVector3 vecNaN(realNaN, realNaN, realNaN);

    double trackNHits(const Particle* part, const Const::EDetector& det)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;

      // Before release-05 (MC13 + proc 11 and older) the hit patterns of TrackFitResults for V0s from the V0Finder were set to 0.
      // Then, we have to take the detour via the related track to access the number of track hits.
      if (trackFit->getHitPatternCDC().getNHits() + trackFit->getHitPatternVXD().getNdf() < 1) {
        trackFit = part->getTrack()->getTrackFitResultWithClosestMass(Const::ChargedStable(std::abs(part->getPDGCode())));
      }
      if (det == Const::EDetector::CDC) {
        return trackFit->getHitPatternCDC().getNHits();
      } else if (det == Const::EDetector::SVD) {
        return trackFit->getHitPatternVXD().getNSVDHits();
      } else if (det == Const::EDetector::PXD) {
        return trackFit->getHitPatternVXD().getNPXDHits();
      } else {
        return realNaN;
      }
    }

    double trackNCDCHits(const Particle* part)
    {
      return trackNHits(part, Const::EDetector::CDC);
    }

    double trackNSVDHits(const Particle* part)
    {
      return trackNHits(part, Const::EDetector::SVD);
    }

    double trackNPXDHits(const Particle* part)
    {
      return trackNHits(part, Const::EDetector::PXD);
    }

    double trackNVXDHits(const Particle* part)
    {
      return trackNPXDHits(part) + trackNSVDHits(part);
    }

    double trackNDF(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;
      return trackFit->getNDF();
    }

    double trackChi2(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;
      return trackFit->getChi2();
    }

    double trackFirstSVDLayer(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;
      // Before release-05 (MC13 + proc 11 and older) the hit patterns of TrackFitResults for V0s from the V0Finder were set to 0.
      // Then, we have to take the detour via the related track to access the real pattern and get the first SVD layer if available.
      if (trackFit->getHitPatternCDC().getNHits() + trackFit->getHitPatternVXD().getNdf() < 1) {
        trackFit = part->getTrack()->getTrackFitResultWithClosestMass(Const::ChargedStable(std::abs(part->getPDGCode())));
      }
      return trackFit->getHitPatternVXD().getFirstSVDLayer();
    }

    double trackFirstPXDLayer(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;
      // Before release-05 (MC13 + proc 11 and older) the hit patterns of TrackFitResults for V0s from the V0Finder were set to 0.
      // Then, we have to take the detour via the related track to access the real pattern and get the first PXD layer if available.
      if (trackFit->getHitPatternCDC().getNHits() + trackFit->getHitPatternVXD().getNdf() < 1) {
        trackFit = part->getTrack()->getTrackFitResultWithClosestMass(Const::ChargedStable(std::abs(part->getPDGCode())));
      }
      return trackFit->getHitPatternVXD().getFirstPXDLayer(HitPatternVXD::PXDMode::normal);
    }

    double trackFirstCDCLayer(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;
      // Before release-05 (MC13 + proc 11 and older) the hit patterns of TrackFitResults for V0s from the V0Finder were set to 0.
      // Then, we have to take the detour via the related track to access the real pattern and get the first CDC layer if available.
      if (trackFit->getHitPatternCDC().getNHits() + trackFit->getHitPatternVXD().getNdf() < 1) {
        trackFit = part->getTrack()->getTrackFitResultWithClosestMass(Const::ChargedStable(std::abs(part->getPDGCode())));
      }
      return trackFit->getHitPatternCDC().getFirstLayer();
    }

    double trackLastCDCLayer(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;
      // Before release-05 (MC13 + proc 11 and older) the hit patterns of TrackFitResults for V0s from the V0Finder were set to 0.
      // Then, we have to take the detour via the related track to access the real pattern and get the last CDC layer if available.
      if (trackFit->getHitPatternCDC().getNHits() + trackFit->getHitPatternVXD().getNdf() < 1) {
        trackFit = part->getTrack()->getTrackFitResultWithClosestMass(Const::ChargedStable(std::abs(part->getPDGCode())));
      }
      return trackFit->getHitPatternCDC().getLastLayer();
    }

    double trackD0(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;
      return trackFit->getD0();
    }

    double trackPhi0(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;
      return trackFit->getPhi0();
    }

    double trackOmega(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;
      return trackFit->getOmega();
    }

    double trackZ0(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;
      return trackFit->getZ0();
    }

    double trackTanLambda(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;
      return trackFit->getTanLambda();
    }

    double trackD0Error(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;

      double errorSquared = trackFit->getCovariance5()[0][0];
      if (errorSquared <= 0) return realNaN;
      return sqrt(errorSquared);
    }

    double trackPhi0Error(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;

      double errorSquared = trackFit->getCovariance5()[1][1];
      if (errorSquared <= 0) return realNaN;
      return sqrt(errorSquared);
    }

    double trackOmegaError(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;

      double errorSquared = trackFit->getCovariance5()[2][2];
      if (errorSquared <= 0) return realNaN;
      return sqrt(errorSquared);
    }

    double trackZ0Error(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;

      double errorSquared = trackFit->getCovariance5()[3][3];
      if (errorSquared <= 0) return realNaN;
      return sqrt(errorSquared);
    }

    double trackTanLambdaError(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;

      double errorSquared = trackFit->getCovariance5()[4][4];
      if (errorSquared <= 0) return realNaN;
      return sqrt(errorSquared);
    }

    double trackPValue(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;
      return trackFit->getPValue();
    }

    double trackFitHypothesisPDG(const Particle* part)
    {
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return realNaN;
      return trackFit->getParticleType().getPDGCode();
    }

    double trackNECLClusters(const Particle* part)
    {
      const Track* track = part->getTrack();
      if (!track) return realNaN;

      // count the number of nPhotons hypothesis ecl clusters
      int count = 0;
      for (const ECLCluster& cluster : track->getRelationsTo<ECLCluster>())
        if (cluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
          ++count;
      return count;
    }

    // used in trackHelixExtTheta and trackHelixExtPhi
    TVector3 getPositionOnHelix(const Particle* part, const std::vector<double>& pars)
    {
      if (pars.size() != 3) {
        B2FATAL("Exactly three parameters (r, zfwd, zbwd) required.");
      }

      const double r    = pars[0];
      const double zfwd = pars[1];
      const double zbwd = pars[2];

      // get the track fit
      auto trackFit = part->getTrackFitResult();
      if (!trackFit) return vecNaN;

      // get helix and parameters
      const double z0 = trackFit->getZ0();
      const double tanlambda = trackFit->getTanLambda();
      const Helix h = trackFit->getHelix();

      // extrapolate to radius
      const double arcLength = h.getArcLength2DAtCylindricalR(r);
      const double lHelixRadius = arcLength > 0 ? arcLength : std::numeric_limits<double>::max();

      // extrapolate to FWD z
      const double lFWD = (zfwd - z0) / tanlambda > 0 ? (zfwd - z0) / tanlambda : std::numeric_limits<double>::max();

      // extrapolate to BWD z
      const double lBWD = (zbwd - z0) / tanlambda > 0 ? (zbwd - z0) / tanlambda : std::numeric_limits<double>::max();

      // pick smallest arclength
      const double l = std::min({lHelixRadius, lFWD, lBWD});

      return h.getPositionAtArcLength2D(l);
    }

    // returns extrapolated theta position based on helix parameters
    double trackHelixExtTheta(const Particle* part, const std::vector<double>& pars)
    {
      if (pars.size() != 3) {
        B2FATAL("Exactly three parameters (r, zfwd, zbwd) required for helixExtTheta.");
      }
      TVector3 position = getPositionOnHelix(part, pars);
      if (position == vecNaN) return realNaN;
      return position.Theta();
    }

    // returns extrapolated phi position based on helix parameters
    double trackHelixExtPhi(const Particle* part, const std::vector<double>& pars)
    {
      if (pars.size() != 3) {
        B2FATAL("Exactly three parameters (r, zfwd, zbwd) required for helixExtPhi.");
      }
      TVector3 position = getPositionOnHelix(part, pars);
      if (position == vecNaN) return realNaN;
      return position.Phi();
    }


    /***************************************************
     * Event level tracking quantities
     */

    // The number of CDC hits in the event not assigned to any track
    double nExtraCDCHits(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return realNaN;
      return elti->getNCDCHitsNotAssigned();
    }

    // The number of CDC hits in the event not assigned to any track nor very
    // likely beam background (i.e. hits that survive a cleanup selection)
    double nExtraCDCHitsPostCleaning(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return realNaN;
      return elti->getNCDCHitsNotAssignedPostCleaning();
    }

    // Check for the presence of a non-assigned hit in the specified CDC layer
    double hasExtraCDCHitsInLayer(const Particle*, const std::vector<double>& layer)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return realNaN;
      int ilayer = std::lround(layer[0]);
      return elti->hasCDCLayer(ilayer);
    }

    // Check for the presence of a non-assigned hit in the specified CDC SuperLayer
    double hasExtraCDCHitsInSuperLayer(const Particle*, const std::vector<double>& layer)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return realNaN;
      int ilayer = std::lround(layer[0]);
      return elti->hasCDCSLayer(ilayer);
    }

    // The number of segments that couldn't be assigned to any track
    double nExtraCDCSegments(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return realNaN;
      return elti->getNCDCSegments();
    }

    //  The number of VXD hits not assigned to any track in the specified layer
    double nExtraVXDHitsInLayer(const Particle*, const std::vector<double>& layer)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return realNaN;
      int ilayer = std::lround(layer[0]);
      return elti->getNVXDClustersInLayer(ilayer);
    }

    //  The number of VXD hits not assigned to any track
    double nExtraVXDHits(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return realNaN;
      double out = 0.0;
      for (uint16_t ilayer = 1; ilayer < 7; ++ilayer)
        out += elti->getNVXDClustersInLayer(ilayer);
      return out;
    }

    // time of first SVD sample relative to event T0
    double svdFirstSampleTime(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return realNaN;
      return elti->getSVDFirstSampleTime();
    }

    // A flag set by the tracking if there is reason to assume there was a track
    // in the event missed by the tracking or the track finding was (partly) aborted
    // for this event. Further information about this can be obtained from the flagBlock
    // of the EventLevelTrackingInfo object.
    double trackFindingFailureFlag(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return realNaN;
      return elti->hasAnErrorFlag();
    }

    double getHelixParameterPullAtIndex(const Particle* particle, const int index)
    {
      if (!particle) return realNaN;

      const MCParticle* mcparticle = particle->getMCParticle();
      if (!mcparticle) return realNaN;

      const Belle2::TrackFitResult* trackfit =  particle->getTrackFitResult();
      if (!trackfit) return realNaN;

      const Belle2::UncertainHelix measHelix = trackfit->getUncertainHelix();
      const TMatrixDSym measCovariance = measHelix.getCovariance();
      const TVector3 mcProdVertex = mcparticle->getVertex();
      const TVector3 mcMomentum = mcparticle->getMomentum();

      const double BzAtProdVertex = Belle2::BFieldManager::getFieldInTesla(mcProdVertex).Z();
      const double mcParticleCharge = mcparticle->getCharge();
      const Belle2::Helix mcHelix = Belle2::Helix(mcProdVertex, mcMomentum, mcParticleCharge, BzAtProdVertex);

      const std::vector<double> mcHelixPars   = {mcHelix.getD0(), mcHelix.getPhi0(), mcHelix.getOmega(), mcHelix.getZ0(), mcHelix.getTanLambda()};
      const std::vector<double> measHelixPars = {measHelix.getD0(), measHelix.getPhi0(), measHelix.getOmega(), measHelix.getZ0(), measHelix.getTanLambda()};
      const std::vector<double> measErrSquare = {measCovariance[0][0], measCovariance[1][1], measCovariance[2][2], measCovariance[3][3], measCovariance[4][4]};

      return (mcHelixPars.at(index) - measHelixPars.at(index)) / std::sqrt(measErrSquare.at(index));
    }

    double getHelixD0Pull(const Particle* part)
    {
      return getHelixParameterPullAtIndex(part, 0);
    }

    double getHelixPhi0Pull(const Particle* part)
    {
      return getHelixParameterPullAtIndex(part, 1);
    }

    double getHelixOmegaPull(const Particle* part)
    {
      return getHelixParameterPullAtIndex(part, 2);
    }

    double getHelixZ0Pull(const Particle* part)
    {
      return getHelixParameterPullAtIndex(part, 3);
    }
    double getHelixTanLambdaPull(const Particle* part)
    {
      return getHelixParameterPullAtIndex(part, 4);
    }


    VARIABLE_GROUP("Tracking");
    REGISTER_VARIABLE("d0Pull", getHelixD0Pull,     R"DOC(
The pull of the tracking parameter :math:`d_0` for the reconstructed
pattern-recognition track, with respect to the MC track. That is:

.. math::

      \frac{d_0^\textrm{MC} - d_0^\textrm{PR}}{\sigma_{d_0; \textrm{PR}}}

.. seealso:: :b2:var:`d0`, :b2:var:`d0Err`

Returns NaN if no MC particle is related or if called on something other than a
track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("phi0Pull", getHelixPhi0Pull, R"DOC(
The pull of the tracking parameter :math:`\phi_0` for the reconstructed
pattern-recognition track, with respect to the MC track.  That is:

.. math::

      \frac{\phi_0^\textrm{MC} - \phi_0^\textrm{PR}}{\sigma_{\phi_0; \textrm{PR}}}

.. seealso:: :b2:var:`phi0`, :b2:var:`phi0Err`

Returns NaN if no MC particle is related or if called on something other than a
track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("omegaPull", getHelixOmegaPull, R"DOC(
The pull of the tracking parameter :math:`\omega` for the reconstructed
pattern-recognition track, with respect to the MC track.  That is:

.. math::

      \frac{\omega^\textrm{MC} - \omega^\textrm{PR}}{\sigma_{\omega; \textrm{PR}}}

.. seealso:: :b2:var:`omega`, :b2:var:`omegaErr`

Returns NaN if no MC particle is related or if called on something other than a
track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("z0Pull", getHelixZ0Pull, R"DOC(
The pull of the tracking parameter :math:`z_0` for the reconstructed
pattern-recognition track, with respect to the MC track.  That is:

.. math::

      \frac{z_0^\textrm{MC} - z_0^\textrm{PR}}{\sigma_{z_0; \textrm{PR}}}

.. seealso:: :b2:var:`z0`, :b2:var:`z0Err`

Returns NaN if no MC particle is related or if called on something other than a
track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("tanLambdaPull", getHelixTanLambdaPull, R"DOC(
The pull of the tracking parameter :math:`\tan\lambda` for the reconstructed
pattern-recognition track, with respect to the MC track.  That is:

.. math::

      \frac{(\tan\lambda)^\textrm{MC} - (\tan\lambda)^\textrm{PR}}{\sigma_{\tan\lambda; \textrm{PR}}}

.. seealso:: :b2:var:`tanLambda`, :b2:var:`tanLambdaErr`

Returns NaN if no MC particle is related or if called on something other than a
track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("nCDCHits", trackNCDCHits,
                      "The number of CDC hits associated to the track. Returns NaN if called for something other than a track-based particle.",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("nSVDHits", trackNSVDHits,
                      "The number of SVD hits associated to the track. Returns NaN if called for something other than a track-based particle.",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("nPXDHits", trackNPXDHits,
                      "The number of PXD hits associated to the track. Returns NaN if called for something other than a track-based particle.",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("nVXDHits", trackNVXDHits,
                      "The number of PXD and SVD hits associated to the track. Returns NaN if called for something other than a track-based particle.",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("ndf",      trackNDF, R"DOC(
Returns the number of degrees of freedom of the track fit. 

.. note:: 

        Note that this is not simply the number of hits -5 due to outlier hit
        rejection.

Returns NaN if called for something other than a track-based particle, or for
mdst files processed with basf2 versions older than ``release-05-01``.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("chi2",      trackChi2, R"DOC(
Returns the :math:`\chi^2` of the track fit.  This is actually computed based on
:b2:var:`pValue` and :b2:var:`ndf`. 

.. note:: Note that for :b2:var:`pValue` exactly equal to 0 it returns infinity. 

Returns NaN if called for something other than a track-based particle, or for
mdst files processed with basf2 versions older than ``release-05-01``.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("firstSVDLayer", trackFirstSVDLayer,
                      "The first activated SVD layer associated to the track. Returns NaN if called for something other than a track-based particle.",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("firstPXDLayer", trackFirstPXDLayer,
                      "The first activated PXD layer associated to the track. Returns NaN if called for something other than a track-based particle.",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("firstCDCLayer", trackFirstCDCLayer,
                      "The first activated CDC layer associated to the track. Returns NaN if called for something other than a track-based particle.",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("lastCDCLayer", trackLastCDCLayer,
                      "The last CDC layer associated to the track. Returns NaN if called for something other than a track-based particle.",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("d0", trackD0, R"DOC(
Returns the tracking parameter :math:`d_0`, the signed distance to the
point-of-closest-approach (POCA) in the :math:`r-\phi` plane. 

.. note::

        Tracking parameters are with respect to the origin (0,0,0).  For the
        POCA with respect to the measured beam interaction point, see
        :b2:var:`dr` (you probably want this unless you're doing a tracking
        study or some debugging).

Returns NaN if called for something other than a track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("phi0", trackPhi0, R"DOC(
Returns the tracking parameter :math:`\phi_0`, the angle of the transverse
momentum in the :math:`r-\phi` plane.

Returns NaN if called for something other than a track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("omega", trackOmega, R"DOC(
Returns the tracking parameter :math:`\omega`, the curvature of the track.

Returns NaN if called for something other than a track-based particle."
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("z0", trackZ0, R"DOC(
Returns the tracking parameter :math:`z_0`, the z-coordinate of the
point-of-closest-approach (POCA).

.. note::

        Tracking parameters are with respect to the origin (0,0,0).  For the
        POCA with respect to the measured beam interaction point, see
        :b2:var:`dz` (you probably want this unless you're doing a tracking
        study or some debugging).

Returns NaN if called for something other than a track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("tanLambda", trackTanLambda, R"DOC(
Returns :math:`\tan\lambda`, the slope of the track in the :math:`r-z` plane.

Returns NaN if called for something other than a track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("d0Err", trackD0Error, R"DOC(
Returns the uncertainty on :math:`d_0`, the signed distance to the
point-of-closest-approach (POCA) in the :math:`r-\phi` plane.

.. seealso:: :b2:var:`d0`, :b2:var:`d0Pull`

Returns NaN if called for something other than a track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("phi0Err", trackPhi0Error, R"DOC(
Returns the uncertainty on :math:`\phi_0`, the angle of the transverse momentum
in the :math:`r-\phi` plane. 

.. seealso:: :b2:var:`phi0`, :b2:var:`phi0Pull`

Returns NaN if called for something other than a track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("omegaErr", trackOmegaError, R"DOC(
Returns the uncertainty on :math:`\omega`, the curvature of the track. 

.. seealso:: :b2:var:`omega`, :b2:var:`omegaPull`

Returns NaN if called for something other than a track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("z0Err", trackZ0Error, R"DOC(
Returns the uncertainty on :math:`z_0`, the z-coordinate of the
point-of-closest-approach (POCA). 

.. seealso:: :b2:var:`z0`, :b2:var:`z0Pull`

Returns NaN if called for something other than a track-based particle."
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("tanLambdaErr", trackTanLambdaError, R"DOC(
Returns the uncertainty on :math:`\tan\lambda`, the slope of the track in the
:math:`r-z` plane.

.. seealso:: :b2:var:`tanLambda`, :b2:var:`tanLambdaPull`

Returns NaN if called for something other than a track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("pValue", trackPValue, R"DOC(
The :math:`\chi^2` probability of the **track** fit.

.. note::

        This is the p-value of the track-fit. It does not get updated after
        vertex fitting or kinematic fitting, and is meaningless for composite
        particles.

        See :b2:var:`chiProb` (you probably want this for high-level analysis).

Returns NaN if called for something other than a track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("trackFitHypothesisPDG", trackFitHypothesisPDG, R"DOC(
Returns the PDG code of the track hypothesis actually used for the fit. 
Returns NaN if called for something other than a track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("trackNECLClusters", trackNECLClusters, R"DOC(
Returns a count of the number of ECLClusters matched to the track.  This is
always 0 or 1 with newer versions of ECL reconstruction.

.. note::

        For high-level analysis it is recommended to require the presence of a
        matched ECL cluster along with a minimum energy requirement.  A
        track-based particle will have a clusterE if it is matched (NaN if
        there is no cluster match for the track.

        .. code-block:: python

              import modularAnalysis as ma
              # minimum energy of 200 MeV
              ma.fillParticleList("e+:clusters", "clusterE > 0.2", path)

              # these two are equivalent
              ma.fillParticleList("e+:unmatched", "isNAN(clusterE) == 1", path)
              ma.fillParticleList("e+:unmatched2", "trackNECLClusters == 0", path)

Returns NaN if called for something other than a track-based particle.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("helixExtTheta", trackHelixExtTheta,
                      "Returns theta of extrapolated helix parameters (parameters (in cm): radius, z fwd, z bwd)", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("helixExtPhi", trackHelixExtPhi,
                      "Returns phi of extrapolated helix parameters (parameters (in cm): radius, z fwd, z bwd)", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("nExtraCDCHits", nExtraCDCHits, R"DOC(
[Eventbased] The number of CDC hits in the event not assigned to any track.

Returns NaN if there is no event-level tracking information available.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("nExtraCDCHitsPostCleaning", nExtraCDCHitsPostCleaning, R"DOC(
[Eventbased] Returns a count of the number of CDC hits in the event not assigned
to any track nor very likely beam background (i.e. hits that survive a cleanup
selection).

Returns NaN if there is no event-level tracking information available.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("hasExtraCDCHitsInLayer(i)", hasExtraCDCHitsInLayer, R"DOC(
[Eventbased] Returns 1 if a non-assigned hit exists in the specified CDC layer,
0 otherwise.

Returns NaN if there is no event-level tracking information available.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("hasExtraCDCHitsInSuperLayer(i)", hasExtraCDCHitsInSuperLayer, R"DOC(
[Eventbased] Returns 1 if a non-assigned hit exists in the specified CDC
SuperLayer, 0 otherwise.

Returns NaN if there is no event-level tracking information available.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("nExtraCDCSegments", nExtraCDCSegments, R"DOC(
[Eventbased] Returns the number of CDC segments not assigned to any track.

Returns NaN if there is no event-level tracking information available.
    )DOC", Manager::VariableDataType::c_double);
    // TODO: once the Tracking group fill the dataobject these can be
    // uncommented - at the moment they are not filled, so leave out
    //REGISTER_VARIABLE("nExtraVXDHitsInLayer(i)", nExtraVXDHitsInLayer,
    //"[Eventbased] The number VXD hits not assigned in the specified VXD layer");
    //REGISTER_VARIABLE("nExtraVXDHits", nExtraVXDHits, "[Eventbased] The number of VXD hits not assigned to any track");
    //REGISTER_VARIABLE("svdFirstSampleTime", svdFirstSampleTime, "[Eventbased] The time of first SVD sample relatvie to event T0");
    REGISTER_VARIABLE("trackFindingFailureFlag", trackFindingFailureFlag, R"DOC(
[Eventbased] Returns a flag set by the tracking if there is reason to assume
there was a track in the event missed by the tracking, or the track finding was
(partly) aborted for this event.

Returns NaN if there is no event-level tracking information available.
    )DOC", Manager::VariableDataType::c_double);
  }
}
