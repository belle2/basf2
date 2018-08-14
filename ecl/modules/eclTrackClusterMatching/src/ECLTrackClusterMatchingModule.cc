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

ECLTrackClusterMatchingModule::ECLTrackClusterMatchingModule() : Module(),
  m_tracksToECLClustersRelationArray(m_tracks, m_eclClusters, "newTracksToECLClusters")
{
  setDescription("Match Tracks to ECLCluster");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("useOptimizedMatchingConsistency", m_useOptimizedMatchingConsistency,
           "set false if you want to set the matching criterion on your own", bool(true));
  addParam("matchingConsistency", m_matchingConsistency,
           "the 2D consistency of Delta theta and Delta phi has to exceed this value for a track to be matched to an ECL cluster", 1e-6);
  addParam("rerunOldMatching", m_rerunOldMatching,
           "run old track cluster matching (again)", bool(false));
}

ECLTrackClusterMatchingModule::~ECLTrackClusterMatchingModule()
{
}

void ECLTrackClusterMatchingModule::initialize()
{
  // Check dependencies
  m_tracks.isRequired();
  m_eclClusters.isRequired();
  m_eclShowers.isRequired();
  m_eclCalDigits.isRequired();
  m_extHits.isRequired();
  m_trackFitResults.isRequired();

  m_tracks.registerRelationTo(m_eclShowers);
  m_tracksToECLClustersRelationArray.registerInDataStore("newTracksToECLClusters");
  m_tracks.registerRelationTo(m_eclClusters);
}

void ECLTrackClusterMatchingModule::event()
{
  if (m_rerunOldMatching) {
    for (auto& eclCluster : m_eclClusters) {
      eclCluster.setIsTrack(false);
    }
    Const::ChargedStable hypothesis = Const::pion;
    int pdgCode = abs(hypothesis.getPDGCode());

    for (const Track& track : m_tracks) {

      const TrackFitResult* fitResult = track.getTrackFitResultWithClosestMass(hypothesis);
      double theta = TMath::ACos(fitResult->getMomentum().CosTheta());
      if (fitResult->getTransverseMomentum() > 0.3 && !trackTowardsGap(theta)) continue;

      // Unique shower ids related to this track
      set<int> uniqueShowerIds;

      // Need to make sure that we match one shower at most
      set<int> uniquehypothesisIds;
      vector<int> hypothesisIds;
      vector<double> energies;
      vector<int> arrayIndexes;

      // Find extrapolated track hits in the ECL, considering
      // only hit points where the track enters the crystal
      // note that more than one crystal belonging to more than one shower
      // can be found
      for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
        if (abs(extHit.getPdgCode()) != pdgCode) continue;
        if (!isECLEnterHit(extHit)) continue;
        const int cell = extHit.getCopyID() + 1;

        // Find ECLCalDigit with same cell ID as ExtHit
        const auto idigit = find_if(m_eclCalDigits.begin(), m_eclCalDigits.end(),
        [&](const ECLCalDigit & d) { return d.getCellId() == cell; }
                                   );
        // Couldn't find ECLCalDigit with same cell ID as the ExtHit
        if (idigit == m_eclCalDigits.end()) continue;

        // Save all unique shower IDs of the showers related to idigit
        for (auto& shower : idigit->getRelationsFrom<ECLShower>()) {
          bool inserted = (uniqueShowerIds.insert(shower.getUniqueId())).second;

          // If this track <-> shower relation hasn't been set yet, set it for the shower and the ECLCLuster
          if (!inserted) continue;

          hypothesisIds.push_back(shower.getHypothesisId());
          energies.push_back(shower.getEnergy());
          arrayIndexes.push_back(shower.getArrayIndex());
          uniquehypothesisIds.insert(shower.getHypothesisId());

          B2DEBUG(29, shower.getArrayIndex() << " "  << shower.getHypothesisId() << " " << shower.getEnergy() << " " <<
                  shower.getConnectedRegionId());

        } // end loop on shower related to idigit
      } // end loop on ExtHit

      // only set the relation for the highest energetic shower per hypothesis
      for (auto hypothesisId : uniquehypothesisIds) {
        double highestEnergy = 0.0;
        int arrayindex = -1;

        for (unsigned ix = 0; ix < energies.size(); ix++) {
          if (hypothesisIds[ix] == hypothesisId and energies[ix] > highestEnergy) {
            highestEnergy = energies[ix];
            arrayindex = arrayIndexes[ix];
          }
        }

        // if we find a shower, take that one by directly acessing the store array
        if (arrayindex > -1) {
          auto shower = m_eclShowers[arrayindex];
          shower->setIsTrack(true);
          track.addRelationTo(shower);
          B2DEBUG(29, shower->getArrayIndex() << " "  << shower->getIsTrack());

          // there is a 1:1 relation, just set the relation for the corresponding cluster as well
          ECLCluster* cluster = shower->getRelatedFrom<ECLCluster>();
          if (cluster != nullptr) {
            cluster->setIsTrack(true);
            track.addRelationTo(cluster);
          }
        }
      }
    } // end loop on Tracks
  } else {
    for (auto& eclCluster : m_eclClusters) {
      bool matchedWithHighPTTrack = true;
      const auto& relatedTracks = eclCluster.getRelationsFrom<Track>();
      for (unsigned int index = 0; index < relatedTracks.size() && matchedWithHighPTTrack; ++index) {
        const Track* relatedTrack = relatedTracks.object(index);
        const TrackFitResult* fitResult = relatedTrack->getTrackFitResultWithClosestMass(Const::pion);
        if (fitResult->getTransverseMomentum() < 0.3 && getDetectorRegion(TMath::ACos(fitResult->getMomentum().CosTheta())) == 2) {
          matchedWithHighPTTrack = false;
        }
      }
      if (matchedWithHighPTTrack) {
        eclCluster.setIsTrack(false);
      }
    }
  }

  for (const Track& track : m_tracks) {
    ECLCluster* cluster_best_cross = nullptr;
    ECLCluster* cluster_best_dl = nullptr;
    ECLCluster* cluster_best_near = nullptr;
    double quality_best_cross = 0, quality_best_dl = 0, quality_best_near = 0;
    const TrackFitResult* fitResult = track.getTrackFitResultWithClosestMass(Const::pion);
    double pt = fitResult->getTransverseMomentum();
    double theta = TMath::ACos(fitResult->getMomentum().CosTheta());
    // never match tracks pointing towards gaps or adjacent part of barrel using angular distance
    if (trackTowardsGap(theta)) continue;
    int trackDetectorRegion = getDetectorRegion(theta);
    // for low-pt tracks pointing towards the barrel never match based on the angular distance
    if (pt < 0.3 && trackDetectorRegion == 2) continue;
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
        // never match low-pt tracks with clusters in the barrel
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
        ExtHitStatus extHitStatus = extHit.getStatus();
        if (extHitStatus == EXT_ECLCROSS) {
          if (quality > quality_best_cross) {
            quality_best_cross = quality;
            cluster_best_cross = eclCluster;
          }
        } else if (extHitStatus == EXT_ECLDL) {
          if (quality > quality_best_dl) {
            quality_best_dl = quality;
            cluster_best_dl = eclCluster;
          }
        } else if (quality > quality_best_near) {
          quality_best_near = quality;
          cluster_best_near = eclCluster;
        }
      }
    } // end loop on ExtHits related to Track
    if (cluster_best_cross != nullptr || cluster_best_dl != nullptr || cluster_best_near != nullptr) {
      if (m_useOptimizedMatchingConsistency) optimizedPTMatchingConsistency(theta, pt);
      if (cluster_best_cross != nullptr && quality_best_cross > m_matchingConsistency) {
        cluster_best_cross->setIsTrack(true);
        m_tracksToECLClustersRelationArray.add(track.getArrayIndex(), cluster_best_cross->getArrayIndex());
        track.addRelationTo(cluster_best_cross);
      } else if (cluster_best_dl != nullptr && quality_best_dl > m_matchingConsistency) {
        cluster_best_dl->setIsTrack(true);
        m_tracksToECLClustersRelationArray.add(track.getArrayIndex(), cluster_best_dl->getArrayIndex());
        track.addRelationTo(cluster_best_dl);
      } else if (cluster_best_near != nullptr && quality_best_near > m_matchingConsistency) {
        cluster_best_near->setIsTrack(true);
        m_tracksToECLClustersRelationArray.add(track.getArrayIndex(), cluster_best_near->getArrayIndex());
        track.addRelationTo(cluster_best_near);
      }
    }
  } // end loop on Tracks
} // end event loop

void ECLTrackClusterMatchingModule::terminate()
{
}

bool ECLTrackClusterMatchingModule::isECLEnterHit(const ExtHit& extHit) const
{
  if ((extHit.getDetectorID() != Const::EDetector::ECL)) return false;
  if ((extHit.getStatus() != EXT_ENTER)) return false;
  if (extHit.getCopyID() == -1) return false;
  else return true;
}

bool ECLTrackClusterMatchingModule::isECLHit(const ExtHit& extHit) const
{
  if ((extHit.getDetectorID() != Const::EDetector::ECL)) return false;
  ExtHitStatus extHitStatus = extHit.getStatus();
  if (extHitStatus == EXT_ECLCROSS || extHitStatus == EXT_ECLDL || extHitStatus == EXT_ECLNEAR) return true;
  else return false;
}

double ECLTrackClusterMatchingModule::clusterQuality(double deltaPhi, double deltaTheta, double pt,
                                                     int eclDetectorRegion) const
{
  double phi_consistency = phiConsistency(deltaPhi, pt, eclDetectorRegion);
  double theta_consistency = thetaConsistency(deltaTheta, pt, eclDetectorRegion);
  return phi_consistency * theta_consistency * (1 - log(phi_consistency * theta_consistency));
}

double ECLTrackClusterMatchingModule::phiConsistency(double deltaPhi, double pt, int eclDetectorRegion) const
{
  double phi_RMS;
  if (eclDetectorRegion == 1 || eclDetectorRegion == 11) { /* RMS for FWD and FWDG */
    if (pt < 0.3) {
      phi_RMS = 0.0795 - 0.037 * pt - 0.404 * pt * pt;
    } else if (pt < 1) {
      phi_RMS = exp(-3.548 - 0.977 * pt) + exp(-0.62 - 17.4 * pt);
    } else {
      phi_RMS = 0.0069 + exp(-3.8 - 1.7 * pt);
    }
  } else if (eclDetectorRegion == 2) { /* RMS for barrel */
    if (pt < 0.385) { // only valid for pt > 0.3
      phi_RMS = -4.03546 + 38.8294 * pt - 115.813 * pt * pt + 115.127 * pt * pt * pt;
    } else {
      phi_RMS = exp(-4.020 - 0.287 * pt) + exp(1.29 - 10.41 * pt);
    }
  } else if (eclDetectorRegion == 3 || eclDetectorRegion == 13) { /* RMS for BWD and BWDG */
    if (pt < 0.3) {
      phi_RMS = exp(-5.23 + 6.2 * pt) + exp(-1.09 - 12.1 * pt);
    } else {
      phi_RMS = 0.01338 + exp(-1.736 - 6.02 * pt);
    }
  } else { /* ECL cluster below acceptance */
    return 0;
  }
  return erfc(abs(deltaPhi) / phi_RMS);
}

double ECLTrackClusterMatchingModule::thetaConsistency(double deltaTheta, double pt, int eclDetectorRegion) const
{
  double theta_RMS;
  if (eclDetectorRegion == 1 || eclDetectorRegion == 11) { /* RMS for FWD and FWDG */
    if (pt < 0.3) {
      theta_RMS = 0.01175 + 0.0478 * pt - 0.163 * pt * pt;
    } else {
      theta_RMS = 0.00517 + exp(-4.601 - 2.09 * pt);
    }
  } else if (eclDetectorRegion == 2) { /* RMS for barrel */
    if (pt < 1) {
      theta_RMS = 0.010649 + exp(-2.795 - 5.612 * pt); // valid for pt > 0.3
    } else if (pt < 2) {
      theta_RMS = 0.01846 - 0.01022 * pt + 0.00264 * pt * pt;
    } else if (pt < 2.2) {
      theta_RMS = -0.0046 + 0.0066 * pt;
    } else {
      theta_RMS = 0.10017;
    }
  } else if (eclDetectorRegion == 3 || eclDetectorRegion == 13) { /* RMS for BWD and BWDG */
    if (pt < 0.21) {
      theta_RMS = 0.0325 - 0.0536 * pt; // valid for pt > 0.14
    } else if (pt < 0.27) {
      theta_RMS = 0.0088 + 0.061 * pt;
    } else {
      theta_RMS = 0.01128 + exp(-2.954 - 5.1 * pt);
    }
  } else { /* ECL cluster below acceptance */
    return 0;
  }
  return erfc(abs(deltaTheta) / theta_RMS);
}

int ECLTrackClusterMatchingModule::getDetectorRegion(double theta) const
{
  if (theta < 0.2164208) return 0;   // < 12.4 deg
  if (theta < 0.5480334) return 1;   // < 31.4 deg
  if (theta < 0.561996) return 11;   // < 32.2 deg
  if (theta < 2.2462387) return 2;   // < 128.7 deg
  if (theta < 2.2811453) return 13;   // < 130.7 deg
  if (theta < 2.7070057) return 3;   // < 155.1 deg
  else return 0;
}

bool ECLTrackClusterMatchingModule::trackTowardsGap(double theta) const
{
  if (theta > 0.55 && theta < 0.65) return true;
  else if (theta > 2.15 && theta < 2.25) return true;
  else return false;
}

void ECLTrackClusterMatchingModule::optimizedPTMatchingConsistency(double theta, double pt)
{
  if (getDetectorRegion(theta) == 1 || getDetectorRegion(theta) == 11) {
    if (pt < 0.3) m_matchingConsistency = 0.1;
    else if (pt < 0.466) m_matchingConsistency = 1e-5;
    else if (pt < 0.6) m_matchingConsistency = 1e-6;
    else if (pt < 0.666) m_matchingConsistency = 1e-7;
    else if (pt < 0.8) m_matchingConsistency = 1e-9;
    else if (pt < 0.866) m_matchingConsistency = 1e-12;
    else if (pt < 0.9) m_matchingConsistency = 1e-15;
    else m_matchingConsistency = 1e-21;
  } else if (getDetectorRegion(theta) == 2) {
    if (theta < 1) {
      if (pt < 0.4) m_matchingConsistency = 0.05;
      else if (pt < 0.433) m_matchingConsistency = 1e-2;
      else if (pt < 0.566) m_matchingConsistency = 1e-3;
      else if (pt < 0.6) m_matchingConsistency = 1e-4;
      else if (pt < 0.666) m_matchingConsistency = 1e-5;
      else if (pt < 0.733) m_matchingConsistency = 1e-6;
      else if (pt < 0.833) m_matchingConsistency = 1e-7;
      else if (pt < 1) m_matchingConsistency = 1e-8;
      else if (pt < 1.033) m_matchingConsistency = 1e-9;
      else if (pt < 1.066) m_matchingConsistency = 1e-10;
      else if (pt < 1.1) m_matchingConsistency = 1e-11;
      else if (pt < 1.233) m_matchingConsistency = 1e-12;
      else if (pt < 1.366) m_matchingConsistency = 1e-15;
      else m_matchingConsistency = 1e-18;
    } else if (theta < 1.8) {
      if (pt < 0.333) m_matchingConsistency = 0.1;
      else if (pt < 0.366) m_matchingConsistency = 1e-2;
      else if (pt < 0.4) m_matchingConsistency = 1e-3;
      else if (pt < 0.466) m_matchingConsistency = 1e-4;
      else if (pt < 0.5) m_matchingConsistency = 1e-5;
      else if (pt < 0.566) m_matchingConsistency = 1e-6;
      else if (pt < 0.6) m_matchingConsistency = 1e-7;
      else if (pt < 0.633) m_matchingConsistency = 1e-8;
      else if (pt < 0.7) m_matchingConsistency = 1e-9;
      else if (pt < 0.733) m_matchingConsistency = 1e-10;
      else if (pt < 0.766) m_matchingConsistency = 1e-11;
      else if (pt < 0.866) m_matchingConsistency = 1e-12;
      else if (pt < 1.1) m_matchingConsistency = 1e-15;
      else if (pt < 1.2) m_matchingConsistency = 1e-18;
      else if (pt < 1.5) m_matchingConsistency = 1e-21;
      else m_matchingConsistency = 1e-24;
    } else {
      if (pt < 0.366) m_matchingConsistency = 0.05;
      else if (pt < 0.4) m_matchingConsistency = 1e-2;
      else if (pt < 0.433) m_matchingConsistency = 1e-3;
      else if (pt < 0.466) m_matchingConsistency = 1e-4;
      else if (pt < 0.5) m_matchingConsistency = 1e-5;
      else if (pt < 0.533) m_matchingConsistency = 1e-6;
      else if (pt < 0.6) m_matchingConsistency = 1e-7;
      else if (pt < 0.666) m_matchingConsistency = 1e-8;
      else if (pt < 0.7) m_matchingConsistency = 1e-9;
      else if (pt < 0.766) m_matchingConsistency = 1e-10;
      else if (pt < 0.8) m_matchingConsistency = 1e-11;
      else if (pt < 1.033) m_matchingConsistency = 1e-12;
      else if (pt < 1.2) m_matchingConsistency = 1e-15;
      else if (pt < 1.5) m_matchingConsistency = 1e-18;
      else m_matchingConsistency = 1e-21;
    }
  } else if (getDetectorRegion(theta) == 3 || getDetectorRegion(theta) == 13) {
    if (pt < 0.2) m_matchingConsistency = 0.2;
    else if (pt < 0.3) m_matchingConsistency = 0.1;
    else if (pt < 0.4) m_matchingConsistency = 1e-2;
    else if (pt < 0.533) m_matchingConsistency = 1e-3;
    else if (pt < 0.566) m_matchingConsistency = 1e-4;
    else if (pt < 0.6) m_matchingConsistency = 1e-6;
    else if (pt < 0.633) m_matchingConsistency = 1e-7;
    else m_matchingConsistency = 1e-8;
  }
}
