/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Sam Cunliffe, Martin Heck                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own includes
#include <analysis/VariableManager/TrackVariables.h>
#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// dataobjects from the MDST
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>

// framework aux
#include <framework/gearbox/Unit.h>
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
        return 0.0;
      }

      if (det == Const::EDetector::CDC) {
        return trackFit->getHitPatternCDC().getNHits();
      } else if (det == Const::EDetector::SVD) {
        return trackFit->getHitPatternVXD().getNSVDHits();
      } else if (det == Const::EDetector::PXD) {
        return trackFit->getHitPatternVXD().getNPXDHits();
      } else {
        return 0.0;
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
        return 0.0;
      }
      return trackFit->getHitPatternVXD().getFirstSVDLayer();
    }

    double trackFirstPXDLayer(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return 0.0;
      }
      return trackFit->getHitPatternVXD().getFirstPXDLayer(HitPatternVXD::PXDMode::normal);
    }

    double trackD0(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return 0.0;
      }
      return trackFit->getD0();
    }

    double trackPhi0(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return 0.0;
      }
      return trackFit->getPhi0();
    }

    double trackOmega(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return 0.0;
      }
      return trackFit->getOmega();
    }

    double trackZ0(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return 0.0;
      }
      return trackFit->getZ0();
    }

    double trackTanLambda(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return 0.0;
      }
      return trackFit->getTanLambda();
    }

    double trackD0Error(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return 0.0;
      }

      double errorSquared = trackFit->getCovariance5()[0][0];
      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return 0.0;
    }

    double trackPhi0Error(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return 0.0;
      }

      double errorSquared = trackFit->getCovariance5()[1][1];
      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return 0.0;
    }

    double trackOmegaError(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return 0.0;
      }

      double errorSquared = trackFit->getCovariance5()[2][2];
      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return 0.0;
    }

    double trackZ0Error(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return 0.0;
      }

      double errorSquared = trackFit->getCovariance5()[3][3];
      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return 0.0;
    }

    double trackTanLambdaError(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return 0.0;
      }

      double errorSquared = trackFit->getCovariance5()[4][4];
      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return 0.0;
    }

    double trackPValue(const Particle* part)
    {
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return 0.0;
      }

      return trackFit->getPValue();
    }

    /***************************************************
     * Event level tracking quantities
     */

    // The number of CDC hits in the event not assigned to any track
    double nExtraCDCHits(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<float>::quiet_NaN();
      return elti->getNCDCHitsNotAssigned();
    }

    // The number of CDC hits in the event not assigned to any track nor very
    // likely beam background (i.e. hits that survive a cleanup selection)
    double nExtraCDCHitsPostCleaning(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<float>::quiet_NaN();
      return elti->getNCDCHitsNotAssignedPostCleaning();
    }

    // Check for the presence of a non-assigned hit in the specified CDC layer
    double hasExtraCDCHitsInLayer(const Particle*, const std::vector<double>& layer)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<float>::quiet_NaN();
      int ilayer = int(std::lround(layer[0]));
      return elti->hasCDCLayer(ilayer);
    }

    // Check for the presence of a non-assigned hit in the specified CDC SuperLayer
    double hasExtraCDCHitsInSuperLayer(const Particle*, const std::vector<double>& layer)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<float>::quiet_NaN();
      int ilayer = int(std::lround(layer[0]));
      return elti->hasCDCSLayer(ilayer);
    }

    // The number of segments that couldn't be assigned to any track
    double nExtraCDCSegments(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<float>::quiet_NaN();
      return elti->getNCDCSegments();
    }

    //  The number of VXD hits not assigned to any track in the specified layer
    double nExtraVXDHitsInLayer(const Particle*, const std::vector<double>& layer)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<float>::quiet_NaN();
      int ilayer = int(std::lround(layer[0]));
      return elti->getNVXDClustersInLayer(ilayer);
    }

    //  The number of VXD hits not assigned to any track
    double nExtraVXDHits(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<float>::quiet_NaN();
      double out = 0.0;
      for (uint16_t ilayer = 1; ilayer < 7; ilayer++)
        out += elti->getNVXDClustersInLayer(ilayer);
      return out;
    }

    // time of first SVD sample relatvie to event T0
    double svdFirstSampleTime(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<float>::quiet_NaN();
      return elti->getSVDFirstSampleTime();
    }

    // A flag set by the tracking if there is reason to assume there was a track
    // in the event missed by the tracking
    double trackFindingFailureFlag(const Particle*)
    {
      StoreObjPtr<EventLevelTrackingInfo> elti;
      if (!elti) return std::numeric_limits<float>::quiet_NaN();
      return elti->hasAnErrorFlag();
    }

    VARIABLE_GROUP("Tracking");
    REGISTER_VARIABLE("nCDCHits", trackNCDCHits,     "Number of CDC hits associated to the track");
    REGISTER_VARIABLE("nSVDHits", trackNSVDHits,     "Number of SVD hits associated to the track");
    REGISTER_VARIABLE("nPXDHits", trackNPXDHits,     "Number of PXD hits associated to the track");
    REGISTER_VARIABLE("nVXDHits", trackNVXDHits,     "Number of PXD and SVD hits associated to the track");
    REGISTER_VARIABLE("firstSVDLayer", trackFirstSVDLayer,     "First activated SVD layer associated to the track");
    REGISTER_VARIABLE("firstPXDLayer", trackFirstPXDLayer,     "First activated PXD layer associated to the track");

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

    REGISTER_VARIABLE("nExtraCDCHits", nExtraCDCHits, "[Eventbased] The number of CDC hits in the event not assigned to any track");
    REGISTER_VARIABLE("nExtraCDCHitsPostCleaning", nExtraCDCHitsPostCleaning,
                      "[Eventbased] The number of CDC hits in the event not assigned to any track nor very likely beam background (i.e. hits that survive a cleanup selection)");
    REGISTER_VARIABLE("hasExtraCDCHitsInLayer(i)", hasExtraCDCHitsInLayer,
                      "[Eventbased] Returns 1 if a non-assigned hit exists in the specified CDC layer");
    REGISTER_VARIABLE("hasExtraCDCHitsInSuperLayer(i)", hasExtraCDCHitsInSuperLayer,
                      "[Eventbased] Returns 1 if a non-assigned hit exists in the specified CDC SuperLayer");
    REGISTER_VARIABLE("nExtraCDCSegments", nExtraCDCSegments, "[Eventbased] The number of CDC segments not assigned to any track");
    REGISTER_VARIABLE("nExtraVXDHitsInLayer(i)", nExtraVXDHitsInLayer,
                      "[Eventbased] The number VXD hits not assigned in the specified VXD layer");
    REGISTER_VARIABLE("nExtraVXDHits", nExtraVXDHits, "[Eventbased] The number of VXD hits not assigned to any track");
    REGISTER_VARIABLE("svdFirstSampleTime", svdFirstSampleTime, "[Eventbased] The time of first SVD sample relatvie to event T0");
    REGISTER_VARIABLE("trackFindingFailureFlag", trackFindingFailureFlag,
                      "[Eventbased] A flag set by the tracking if there is reason to assume there was a track in the event missed by the tracking");


  }
}
