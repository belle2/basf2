/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Sam Cunliffe, Martin Heck, Torben Ferber    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/ECLCluster.h>

// framework aux
#include <framework/logging/Logger.h>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

namespace {

  Belle2::TrackFitResult const* getTrackFitResultFromParticle(Belle2::Particle const* particle)
  {
    const Belle2::Track* track = particle->getTrack();
    if (!track) {
      return nullptr;
    }

    const Belle2::TrackFitResult* trackFit = track->getTrackFitResultWithClosestMass(Belle2::Const::ChargedStable(abs(
                                               particle->getPDGCode())));
    return trackFit;
  }
}

namespace Belle2 {
  namespace Variable {

    double trackNHits(const Particle* part, const Const::EDetector& det)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }

      if (det == Const::EDetector::CDC) {
        return trackFit->getHitPatternCDC().getNHits();
      } else if (det == Const::EDetector::SVD) {
        return trackFit->getHitPatternVXD().getNSVDHits();
      } else if (det == Const::EDetector::PXD) {
        return trackFit->getHitPatternVXD().getNPXDHits();
      } else {
        return std::numeric_limits<double>::quiet_NaN();
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

    double trackFirstSVDLayer(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getHitPatternVXD().getFirstSVDLayer();
    }

    double trackFirstPXDLayer(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getHitPatternVXD().getFirstPXDLayer(HitPatternVXD::PXDMode::normal);
    }

    double trackLastCDCLayer(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getHitPatternCDC().getLastLayer();
    }

    double trackD0(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getD0();
    }

    double trackPhi0(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getPhi0();
    }

    double trackOmega(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getOmega();
    }

    double trackZ0(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getZ0();
    }

    double trackTanLambda(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getTanLambda();
    }

    double trackD0Error(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }

      double errorSquared = trackFit->getCovariance5()[0][0];
      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return std::numeric_limits<double>::quiet_NaN();
    }

    double trackPhi0Error(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }

      double errorSquared = trackFit->getCovariance5()[1][1];
      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return std::numeric_limits<double>::quiet_NaN();
    }

    double trackOmegaError(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }

      double errorSquared = trackFit->getCovariance5()[2][2];
      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return std::numeric_limits<double>::quiet_NaN();
    }

    double trackZ0Error(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }

      double errorSquared = trackFit->getCovariance5()[3][3];
      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return std::numeric_limits<double>::quiet_NaN();
    }

    double trackTanLambdaError(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }

      double errorSquared = trackFit->getCovariance5()[4][4];
      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return std::numeric_limits<double>::quiet_NaN();
    }

    double trackPValue(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return -1;
      }

      return trackFit->getPValue();
    }

    double trackFitHypothesisPDG(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return 0.0;
      }

      return trackFit->getParticleType().getPDGCode();
    }

    double trackNECLClusters(const Particle* part)
    {
      const Track* track = part->getTrack();
      if (!track)
        return std::numeric_limits<double>::quiet_NaN();

      // count the number of nPhotons hypothesis ecl clusters
      int count = 0;
      for (const ECLCluster& cluster : track->getRelationsTo<ECLCluster>())
        if (cluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
          count++;
      return double(count);
    }

    // returns extrapolated theta position based on helix parameters
    double trackHelixExtTheta(const Particle* part, const std::vector<double>& pars)
    {
      if (pars.size() != 3) {
        B2WARNING("Exactly three parameters (r, zfwd, zbwd) required.");
        return std::numeric_limits<double>::quiet_NaN();
      }

      const double r = pars[0];
      const double zfwd = pars[1];
      const double zbwd = pars[2];

      // get the track fit
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) return std::numeric_limits<double>::quiet_NaN();

      // get helix and paremeters
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

      // pick smalles arclength
      const double l = std::min(std::min(lHelixRadius, lFWD), lBWD);

      return atan2(h.getPositionAtArcLength2D(l).Perp(), h.getPositionAtArcLength2D(l).Z());
    }

    // returns extrapolated phi position based on helix parameters
    double trackHelixExtPhi(const Particle* part, const std::vector<double>& pars)
    {
      if (pars.size() != 3) {
        B2WARNING("Exactly three parameters (r, zfwd, zbwd) required.");
        return std::numeric_limits<double>::quiet_NaN();
      }

      const double r = pars[0];
      const double zfwd = pars[1];
      const double zbwd = pars[2];

      // get the track fit
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) return std::numeric_limits<double>::quiet_NaN();

      // get helix and paremeters
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

      // pick smalles arclength
      const double l = std::min(std::min(lHelixRadius, lFWD), lBWD);

      return atan2(h.getPositionAtArcLength2D(l).Y(), h.getPositionAtArcLength2D(l).X());
    }


    /***************************************************
     * Event level tracking quantities
     */

    // The number of CDC hits in the event not assigned to any track
    double nExtraCDCHits(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<double>::quiet_NaN();
      return elti->getNCDCHitsNotAssigned();
    }

    // The number of CDC hits in the event not assigned to any track nor very
    // likely beam background (i.e. hits that survive a cleanup selection)
    double nExtraCDCHitsPostCleaning(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<double>::quiet_NaN();
      return elti->getNCDCHitsNotAssignedPostCleaning();
    }

    // Check for the presence of a non-assigned hit in the specified CDC layer
    double hasExtraCDCHitsInLayer(const Particle*, const std::vector<double>& layer)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<double>::quiet_NaN();
      int ilayer = int(std::lround(layer[0]));
      return elti->hasCDCLayer(ilayer);
    }

    // Check for the presence of a non-assigned hit in the specified CDC SuperLayer
    double hasExtraCDCHitsInSuperLayer(const Particle*, const std::vector<double>& layer)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<double>::quiet_NaN();
      int ilayer = int(std::lround(layer[0]));
      return elti->hasCDCSLayer(ilayer);
    }

    // The number of segments that couldn't be assigned to any track
    double nExtraCDCSegments(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<double>::quiet_NaN();
      return elti->getNCDCSegments();
    }

    //  The number of VXD hits not assigned to any track in the specified layer
    double nExtraVXDHitsInLayer(const Particle*, const std::vector<double>& layer)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<double>::quiet_NaN();
      int ilayer = int(std::lround(layer[0]));
      return elti->getNVXDClustersInLayer(ilayer);
    }

    //  The number of VXD hits not assigned to any track
    double nExtraVXDHits(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<double>::quiet_NaN();
      double out = 0.0;
      for (uint16_t ilayer = 1; ilayer < 7; ilayer++)
        out += elti->getNVXDClustersInLayer(ilayer);
      return out;
    }

    // time of first SVD sample relatvie to event T0
    double svdFirstSampleTime(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<double>::quiet_NaN();
      return elti->getSVDFirstSampleTime();
    }

    // A flag set by the tracking if there is reason to assume there was a track
    // in the event missed by the tracking or the track finding was (partly) aborted
    // for this event. Further information about this can be obtained from the flagBlock
    // of the EventLevelTrackingInfo object.
    double trackFindingFailureFlag(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<double>::quiet_NaN();
      return elti->hasAnErrorFlag();
    }

    double getHelixParameterPullAtIndex(const Particle* particle, const int index)
    {

      if (!particle) { return std::numeric_limits<double>::quiet_NaN(); }

      const MCParticle* mcparticle = particle->getRelatedTo<MCParticle>();
      if (!mcparticle) { return std::numeric_limits<double>::quiet_NaN(); }

      const Belle2::Track* track = particle->getTrack();
      if (!track) { return std::numeric_limits<double>::quiet_NaN(); }

      const Belle2::TrackFitResult* trackfit =  track->getTrackFitResultWithClosestMass(Belle2::Const::ChargedStable(std::abs(
                                                  particle->getPDGCode())));
      if (!trackfit) { return std::numeric_limits<double>::quiet_NaN(); }

      const Belle2::UncertainHelix measHelix = trackfit->getUncertainHelix();
      const TMatrixDSym measCovariance = measHelix.getCovariance();
      const TVector3 mcProdVertex = mcparticle->getVertex();
      const TVector3 mcMomentum = mcparticle->getMomentum();

      const double BzAtProdVertex = Belle2::BFieldManager::getField(TVector3(mcProdVertex.X(), mcProdVertex.Y(),
                                    mcProdVertex.Z())).Z() / Belle2::Unit::T;
      const double mcParticleCharge = mcparticle->getCharge();
      const Belle2::Helix mcHelix = Belle2::Helix(mcProdVertex, mcMomentum, mcParticleCharge, BzAtProdVertex);

      const std::vector<double> mcHelixPars = {mcHelix.getD0(), mcHelix.getPhi0(), mcHelix.getOmega(), mcHelix.getZ0(), mcHelix.getTanLambda()};
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
    REGISTER_VARIABLE("d0Pull", getHelixD0Pull,     "mc-meas/err_meas for d0");
    REGISTER_VARIABLE("phi0Pull", getHelixPhi0Pull,     "mc-meas/err_meas for phi0");
    REGISTER_VARIABLE("omegaPull", getHelixOmegaPull,     "mc-meas/err_meas for omega");
    REGISTER_VARIABLE("z0Pull", getHelixZ0Pull,     "mc-meas/err_meas for z0");
    REGISTER_VARIABLE("tanLambdaPull", getHelixTanLambdaPull,     "mc-meas/err_meas for tanLambda");

    REGISTER_VARIABLE("nCDCHits", trackNCDCHits,     "Number of CDC hits associated to the track");
    REGISTER_VARIABLE("nSVDHits", trackNSVDHits,     "Number of SVD hits associated to the track");
    REGISTER_VARIABLE("nPXDHits", trackNPXDHits,     "Number of PXD hits associated to the track");
    REGISTER_VARIABLE("nVXDHits", trackNVXDHits,     "Number of PXD and SVD hits associated to the track");
    REGISTER_VARIABLE("firstSVDLayer", trackFirstSVDLayer,     "First activated SVD layer associated to the track");
    REGISTER_VARIABLE("firstPXDLayer", trackFirstPXDLayer,     "First activated PXD layer associated to the track");
    REGISTER_VARIABLE("lastCDCLayer", trackLastCDCLayer, "Last CDC layer associated to the track");
    REGISTER_VARIABLE("d0",        trackD0,        "Signed distance to the POCA in the r-phi plane");
    REGISTER_VARIABLE("phi0",      trackPhi0,      "Angle of the transverse momentum in the r-phi plane");
    REGISTER_VARIABLE("omega",     trackOmega,     "Curvature of the track");
    REGISTER_VARIABLE("z0",        trackZ0,        "z coordinate of the POCA");
    REGISTER_VARIABLE("tanlambda", trackTanLambda, "Slope of the track in the r-z plane");
    REGISTER_VARIABLE("d0Err",        trackD0Error,        "Error of signed distance to the POCA in the r-phi plane");
    REGISTER_VARIABLE("phi0Err",      trackPhi0Error,      "Error of angle of the transverse momentum in the r-phi plane");
    REGISTER_VARIABLE("omegaErr",     trackOmegaError,     "Error of curvature of the track");
    REGISTER_VARIABLE("z0Err",        trackZ0Error,        "Error of z coordinate of the POCA");
    REGISTER_VARIABLE("tanlambdaErr", trackTanLambdaError, "Error of slope of the track in the r-z plane");
    REGISTER_VARIABLE("pValue", trackPValue, "chi2 probalility of the track fit");
    REGISTER_VARIABLE("trackFitHypothesisPDG", trackFitHypothesisPDG, "PDG code of the track hypothesis actually used for the fit");
    REGISTER_VARIABLE("trackNECLClusters", trackNECLClusters,
                      "Number ecl clusters matched to the track. This is always 0 or 1 with newer versions of ECL reconstruction.");
    REGISTER_VARIABLE("helixExtTheta", trackHelixExtTheta,
                      "Returns theta of extrapolated helix parameters (parameters (in cm): radius, z fwd, z bwd)");
    REGISTER_VARIABLE("helixExtPhi", trackHelixExtPhi,
                      "Returns phi of extrapolated helix parameters (parameters (in cm): radius, z fwd, z bwd)");

    REGISTER_VARIABLE("nExtraCDCHits", nExtraCDCHits, "[Eventbased] The number of CDC hits in the event not assigned to any track");
    REGISTER_VARIABLE("nExtraCDCHitsPostCleaning", nExtraCDCHitsPostCleaning,
                      "[Eventbased] The number of CDC hits in the event not assigned to any track nor very likely beam background (i.e. hits that survive a cleanup selection)");
    REGISTER_VARIABLE("hasExtraCDCHitsInLayer(i)", hasExtraCDCHitsInLayer,
                      "[Eventbased] Returns 1 if a non-assigned hit exists in the specified CDC layer");
    REGISTER_VARIABLE("hasExtraCDCHitsInSuperLayer(i)", hasExtraCDCHitsInSuperLayer,
                      "[Eventbased] Returns 1 if a non-assigned hit exists in the specified CDC SuperLayer");
    REGISTER_VARIABLE("nExtraCDCSegments", nExtraCDCSegments, "[Eventbased] The number of CDC segments not assigned to any track");
    // TODO: once the Tracking group fill the dataobject these can be
    // uncommented - at the moment they are not filled, so leave out
    //REGISTER_VARIABLE("nExtraVXDHitsInLayer(i)", nExtraVXDHitsInLayer,
    //"[Eventbased] The number VXD hits not assigned in the specified VXD layer");
    //REGISTER_VARIABLE("nExtraVXDHits", nExtraVXDHits, "[Eventbased] The number of VXD hits not assigned to any track");
    //REGISTER_VARIABLE("svdFirstSampleTime", svdFirstSampleTime, "[Eventbased] The time of first SVD sample relatvie to event T0");
    REGISTER_VARIABLE("trackFindingFailureFlag", trackFindingFailureFlag,
                      "[Eventbased] A flag set by the tracking if there is reason to assume there was a track in the event missed by the tracking, "
                      "or the track finding was (partly) aborted for this event.");


  }
}
