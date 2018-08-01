/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 * Contributors: Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclTrackClusterMatching/ECLTrackClusterMatchingModule.h>
#include <framework/datastore/RelationVector.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ECLTrackClusterMatching)

ECLTrackClusterMatchingModule::ECLTrackClusterMatchingModule() : Module()
{
  setDescription("Match Tracks to ECLCluster");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("useOptimizedMatchingConsistency", m_useOptimizedMatchingConsistency,
           "set false if you want to set the matching criterion on your own", bool(true));
  addParam("matchingConsistency", m_matchingConsistency,
           "the 2D consistency of Delta theta and Delta phi has to exceed this value for a track to be matched to an ECL cluster", 1e-6);
}

ECLTrackClusterMatchingModule::~ECLTrackClusterMatchingModule()
{
}

void ECLTrackClusterMatchingModule::initialize()
{
  // Check dependencies
  m_tracks.isRequired();
  m_eclClusters.isRequired();
  m_tracks.registerRelationTo(m_eclClusters);
  m_extHits.isRequired();
  m_trackFitResults.isRequired();
}

void ECLTrackClusterMatchingModule::beginRun()
{
}

void ECLTrackClusterMatchingModule::event()
{
  for (auto& eclCluster : m_eclClusters) {
    bool matchedWithHighPTTrack = true;
    const auto& relatedTracks = eclCluster.getRelationsFrom<Track>();
    for (unsigned int index = 0; index < relatedTracks.size() && matchedWithHighPTTrack; ++index) {
      const Track* relatedTrack = relatedTracks.object(index);
      const TrackFitResult* fitResult = relatedTrack->getTrackFitResultWithClosestMass(Const::muon);
      if (fitResult->getTransverseMomentum() < 0.3 && getDetectorRegion(TMath::ACos(fitResult->getMomentum().CosTheta())) == 2) {
        matchedWithHighPTTrack = false;
      }
    }
    if (matchedWithHighPTTrack) {
      eclCluster.setIsTrack(false);
    }
  }
  for (const Track& track : m_tracks) {
    ECLCluster* cluster_best = nullptr;
    double quality_best = 0;
    const TrackFitResult* fitResult = track.getTrackFitResultWithClosestMass(Const::muon);
    double pt = fitResult->getTransverseMomentum();
    double theta = TMath::ACos(fitResult->getMomentum().CosTheta());
    int trackDetectorRegion = getDetectorRegion(theta);
    if (m_useOptimizedMatchingConsistency) optimizedMatchingConsistency(theta);
    // Find extrapolated track hits in the ECL, considering only hit points
    // that either are on the sphere, closest to, or on radial direction of an
    // ECLCluster.
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (!isECLHit(extHit)) continue;
      ECLCluster* eclCluster = extHit.getRelatedFrom<ECLCluster>();
      if (eclCluster != nullptr) {
        if (eclCluster->getHypothesisId() != 5) continue;
        int eclDetectorRegion = eclCluster->getDetectorRegion();
        // accept only cluster from region matching track direction, exception for gaps
        if (abs(eclDetectorRegion - trackDetectorRegion) == 1) continue;
        // decline low-pt tracks unless the tested cluster is in endcaps
        if (pt < 0.3 && eclDetectorRegion == 2) continue;
        double phiHit = extHit.getPosition().Phi();
        double phiCluster = eclCluster->getPhi();
        double deltaPhi = phiHit - phiCluster;
        if (deltaPhi > M_PI) {
          deltaPhi = deltaPhi - 2 * M_PI;
        } else if (deltaPhi < -M_PI) {
          deltaPhi = deltaPhi + 2 * M_PI;
        }
        double thetaHit = extHit.getPosition().Theta();
        double thetaCluster = eclCluster->getTheta();
        double deltaTheta = thetaHit - thetaCluster;
        double quality = clusterQuality(deltaPhi, deltaTheta, pt, eclDetectorRegion);
        if (quality > quality_best) {
          quality_best = quality;
          cluster_best = eclCluster;
        }
      }
    } // end loop on ExtHits related to Track
    if (cluster_best != nullptr) {
      // optimizedFakeRateMatchingConsistency(cluster_best->getTheta());
      if (quality_best > m_matchingConsistency) {
        cluster_best->setIsTrack(true);
        track.addRelationTo(cluster_best);
      }
    }
  } // end loop on Tracks
} // end event loop

void ECLTrackClusterMatchingModule::endRun()
{
}

void ECLTrackClusterMatchingModule::terminate()
{
}

bool ECLTrackClusterMatchingModule::isECLHit(const ExtHit& extHit) const
{
  if ((extHit.getDetectorID() != Const::EDetector::ECL)) return false;
  ExtHitStatus extHitStatus = extHit.getStatus();
  if (extHitStatus == EXT_ECLCROSS || extHitStatus == EXT_ECLDL || extHitStatus == EXT_ECLNEAR) return true;
  else return false;
}

double ECLTrackClusterMatchingModule::clusterQuality(double deltaPhi, double deltaTheta, double transverseMomentum,
                                                     int eclDetectorRegion) const
{
  double phi_consistency = phiConsistency(deltaPhi, transverseMomentum, eclDetectorRegion);
  double theta_consistency = thetaConsistency(deltaTheta, transverseMomentum, eclDetectorRegion);
  return phi_consistency * theta_consistency * (1 - log(phi_consistency * theta_consistency));
}

double ECLTrackClusterMatchingModule::phiConsistency(double deltaPhi, double transverseMomentum, int eclDetectorRegion) const
{
  double phi_RMS;
  if (eclDetectorRegion == 1 || eclDetectorRegion == 11) { /* RMS for FWD and FWDG */
    phi_RMS = exp(-4.057 - 0.346 * transverseMomentum) + exp(-1.712 - 8.05 * transverseMomentum);
  } else if (eclDetectorRegion == 2) { /* RMS for barrel */
    if (transverseMomentum < 0.24) {
      phi_RMS = 0.0356 + exp(1.1 - 33.3 * transverseMomentum);
    } else if (transverseMomentum < 0.3) {
      phi_RMS = 0.0591 * exp(-0.5 * pow((transverseMomentum - 0.2692) / 0.0276, 2));
    } else if (transverseMomentum < 0.52) {
      phi_RMS = -0.0534 + 0.422 * transverseMomentum - 0.503 * transverseMomentum * transverseMomentum;
    } else {
      phi_RMS = exp(-4.020 - 0.287 * transverseMomentum) + exp(1.29 - 10.41 * transverseMomentum);
    }
  } else if (eclDetectorRegion == 3 || eclDetectorRegion == 13) { /* RMS for BWD and BWDG */
    phi_RMS = exp(-4.06 - 0.19 * transverseMomentum) + exp(-2.187 - 5.83 * transverseMomentum);
  } else { /* ECL cluster below acceptance */
    return 0;
  }
  return erfc(abs(deltaPhi) / phi_RMS);
}

double ECLTrackClusterMatchingModule::thetaConsistency(double deltaTheta, double transverseMomentum, int eclDetectorRegion) const
{
  double theta_RMS;
  if (eclDetectorRegion == 1 || eclDetectorRegion == 11) { /* RMS for FWD and FWDG */
    theta_RMS = 0.00761 + exp(-3.52 - 6.65 * transverseMomentum); // valid for pt > 0.11
  } else if (eclDetectorRegion == 2) { /* RMS for barrel */
    theta_RMS = 0.011041 + exp(-2.830 - 5.78 * transverseMomentum); // valid for pt > 0.3
  } else if (eclDetectorRegion == 3 || eclDetectorRegion == 13) { /* RMS for BWD and BWDG */
    theta_RMS = 0.01090 + exp(-3.38 - 4.65 * transverseMomentum); // valid for pt > 0.14
  } else { /* ECL cluster below acceptance */
    return 0;
  }
  return erfc(abs(deltaTheta) / theta_RMS);
}

int ECLTrackClusterMatchingModule::getDetectorRegion(double theta) const
{
  if (theta < 0.2164208) return 0;   // < 12.4 deg
  // if (theta < 0.5480334) return 1;   // < 31.4 deg
  if (theta < 0.45) return 1;   // smaller FWD
  // if (theta < 0.561996) return 11;   // < 32.2 deg
  if (theta < 0.65) return 11;   // enlarged gap
  // if (theta < 2.2462387) return 2;   // < 128.7 deg
  if (theta < 2.15) return 2;   // smaller barrel
  // if (theta < 2.2811453) return 13;   // < 130.7 deg
  if (theta < 2.35) return 13;   // enlarged gap
  if (theta < 2.7070057) return 3;   // < 155.1 deg
  else return 0;
}

void ECLTrackClusterMatchingModule::optimizedMatchingConsistency(double theta)
{
  if (theta < 0.375) m_matchingConsistency = 0.01;
  else if (theta < 0.4) m_matchingConsistency = 1e-3;
  else if (theta < 0.55) m_matchingConsistency = 1e-4;
  else if (theta < 0.65) m_matchingConsistency = 1e-2;
  else if (theta < 0.9) m_matchingConsistency = 1e-3;
  else if (theta < 1.05) m_matchingConsistency = 1e-4;
  else if (theta < 1.175) m_matchingConsistency = 1e-5;
  else if (theta < 1.3) m_matchingConsistency = 1e-6;
  else if (theta < 1.35) m_matchingConsistency = 1e-7;
  else if (theta < 1.5) m_matchingConsistency = 1e-8;
  else if (theta < 1.7) m_matchingConsistency = 1e-7;
  else if (theta < 1.85) m_matchingConsistency = 1e-6;
  else if (theta < 2) m_matchingConsistency = 1e-5;
  else if (theta < 2.15) m_matchingConsistency = 1e-4;
  else if (theta < 2.4) m_matchingConsistency = 0.01;
  else m_matchingConsistency = 0.1;
}

void ECLTrackClusterMatchingModule::optimizedFakeRateMatchingConsistency(double theta)
{
  if (theta < 0.3) m_matchingConsistency = 1e-15;
  else if (theta < 0.325) m_matchingConsistency = 1e-12;
  else if (theta < 0.55) m_matchingConsistency = 1e-15;
  else if (theta < 0.575) m_matchingConsistency = 1e-3;
  else if (theta < 0.6) m_matchingConsistency = 1e-2;
  else if (theta < 0.65) m_matchingConsistency = 1e-3;
  else if (theta < 0.675) m_matchingConsistency = 1e-5;
  else if (theta < 0.7) m_matchingConsistency = 1e-7;
  else if (theta < 0.725) m_matchingConsistency = 1e-5;
  else if (theta < 0.75) m_matchingConsistency = 1e-4;
  else if (theta < 0.775) m_matchingConsistency = 1e-8;
  else if (theta < 0.85) m_matchingConsistency = 1e-9;
  else if (theta < 2.075) m_matchingConsistency = 1e-15;
  else if (theta < 2.1) m_matchingConsistency = 1e-9;
  else if (theta < 2.125) m_matchingConsistency = 1e-12;
  else if (theta < 2.15) m_matchingConsistency = 1e-9;
  else if (theta < 2.175) m_matchingConsistency = 1e-3;
  else if (theta < 2.2) m_matchingConsistency = 1e-5;
  else if (theta < 2.225) m_matchingConsistency = 1e-6;
  else if (theta < 2.25) m_matchingConsistency = 1e-12;
  else if (theta < 2.275) m_matchingConsistency = 1e-15;
  else if (theta < 2.3) m_matchingConsistency = 1e-2;
  else if (theta < 2.325) m_matchingConsistency = 1e-12;
  else if (theta < 2.35) m_matchingConsistency = 1e-5;
  else if (theta < 2.375) m_matchingConsistency = 1e-3;
  else if (theta < 2.425) m_matchingConsistency = 1e-4;
  else if (theta < 2.45) m_matchingConsistency = 1e-9;
  else if (theta < 2.475) m_matchingConsistency = 1e-2;
  else if (theta < 2.5) m_matchingConsistency = 1e-7;
  else if (theta < 2.525) m_matchingConsistency = 1e-4;
  else if (theta < 2.55) m_matchingConsistency = 1e-2;
  else if (theta < 2.575) m_matchingConsistency = 1e-12;
  else if (theta < 2.6) m_matchingConsistency = 1e-15;
  else if (theta < 2.625) m_matchingConsistency = 1e-7;
  else if (theta < 2.65) m_matchingConsistency = 1e-6;
  else if (theta < 2.675) m_matchingConsistency = 1e-4;
  else m_matchingConsistency = 0.001;
}
