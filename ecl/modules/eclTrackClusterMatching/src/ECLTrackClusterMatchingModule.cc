/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 * Contributors: Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclTrackClusterMatching/ECLTrackClusterMatchingModule.h>

#include <ecl/dbobjects/ECLTrackClusterMatchingParameterizations.h>
#include <ecl/dbobjects/ECLTrackClusterMatchingThresholds.h>
#include <ecl/utility/utilityFunctions.h>

#include <framework/datastore/RelationVector.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ECLTrackClusterMatching)

ECLTrackClusterMatchingModule::ECLTrackClusterMatchingModule() : Module(),
  m_matchingParameterizations("ECLTrackClusterMatchingParameterizations"),
  m_matchingThresholds("ECLTrackClusterMatchingThresholds")
{
  setDescription("Creates and saves a Relation between Tracks and ECLCluster in the DataStore. It uses the existing Relation between Tracks and ExtHit as well as the Relation between ECLCluster and ExtHit.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("useAngularDistanceMatching", m_angularDistanceMatching,
           "if true use track cluster matching based on angular distance, if false use matching based on entered crystals", bool(true));
  addParam("useOptimizedMatchingConsistency", m_useOptimizedMatchingConsistency,
           "set false if you want to set the matching criterion on your own", bool(true));
  addParam("matchingConsistency", m_matchingConsistency,
           "the 2D consistency of Delta theta and Delta phi has to exceed this value for a track to be matched to an ECL cluster", 1e-6);
  addParam("matchingPTThreshold", m_matchingPTThreshold,
           "tracks with pt greater than this value will exclusively be matched based on angular distance", 0.3);
  addParam("brlEdgeTheta", m_brlEdgeTheta,
           "distance of polar angle from gaps where crystal-entering based matching is applied (in rad)", 0.1);
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

  if (m_angularDistanceMatching) {
    m_tracks.registerRelationTo(m_eclShowers, DataStore::c_Event, DataStore::c_WriteOut, "AngularDistance");
    m_tracks.registerRelationTo(m_eclClusters, DataStore::c_Event, DataStore::c_WriteOut, "AngularDistance");

    f_phiRMSFWDCROSS = m_matchingParameterizations->getPhiFWDCROSSRMSParameterization();
    f_phiRMSFWDDL = m_matchingParameterizations->getPhiFWDDLRMSParameterization();
    f_phiRMSFWDNEAR = m_matchingParameterizations->getPhiFWDNEARRMSParameterization();
    f_phiRMSBRLCROSS = m_matchingParameterizations->getPhiBRLCROSSRMSParameterization();
    f_phiRMSBRLDL = m_matchingParameterizations->getPhiBRLDLRMSParameterization();
    f_phiRMSBRLNEAR = m_matchingParameterizations->getPhiBRLNEARRMSParameterization();
    f_phiRMSBWDCROSS = m_matchingParameterizations->getPhiBWDCROSSRMSParameterization();
    f_phiRMSBWDDL = m_matchingParameterizations->getPhiBWDDLRMSParameterization();
    f_phiRMSBWDNEAR = m_matchingParameterizations->getPhiBWDNEARRMSParameterization();
    f_thetaRMSFWDCROSS = m_matchingParameterizations->getThetaFWDCROSSRMSParameterization();
    f_thetaRMSFWDDL = m_matchingParameterizations->getThetaFWDDLRMSParameterization();
    f_thetaRMSFWDNEAR = m_matchingParameterizations->getThetaFWDNEARRMSParameterization();
    f_thetaRMSBRLCROSS = m_matchingParameterizations->getThetaBRLCROSSRMSParameterization();
    f_thetaRMSBRLDL = m_matchingParameterizations->getThetaBRLDLRMSParameterization();
    f_thetaRMSBRLNEAR = m_matchingParameterizations->getThetaBRLNEARRMSParameterization();
    f_thetaRMSBWDCROSS = m_matchingParameterizations->getThetaBWDCROSSRMSParameterization();
    f_thetaRMSBWDDL = m_matchingParameterizations->getThetaBWDDLRMSParameterization();
    f_thetaRMSBWDNEAR = m_matchingParameterizations->getThetaBWDNEARRMSParameterization();

    m_matchingThresholdValuesFWD = m_matchingThresholds->getFWDMatchingThresholdValues();
    m_matchingThresholdValuesBRL = m_matchingThresholds->getBRLMatchingThresholdValues();
    m_matchingThresholdValuesBWD = m_matchingThresholds->getBWDMatchingThresholdValues();
  } else {
    m_tracks.registerRelationTo(m_eclShowers, DataStore::c_Event, DataStore::c_WriteOut, "EnterCrystal");
    m_tracks.registerRelationTo(m_eclClusters, DataStore::c_Event, DataStore::c_WriteOut, "EnterCrystal");
  }
}

void ECLTrackClusterMatchingModule::event()
{
  for (auto& eclCluster : m_eclClusters) {
    eclCluster.setIsTrack(false);
  }
  for (const Track& track : m_tracks) {
    const TrackFitResult* fitResult = track.getTrackFitResultWithClosestMass(Const::pion);
    double theta = TMath::ACos(fitResult->getMomentum().CosTheta());
    double pt = fitResult->getTransverseMomentum();
    if (!m_angularDistanceMatching || pt < m_matchingPTThreshold || trackTowardsGap(theta)) {

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

        // if we find a shower, take that one by directly accessing the store array
        if (arrayindex > -1) {
          auto shower = m_eclShowers[arrayindex];
          shower->setIsTrack(true);
          if (m_angularDistanceMatching) {
            track.addRelationTo(shower, 1.0, "AngularDistance");
          } else {
            track.addRelationTo(shower, 1.0, "EnterCrystal");
          }
          B2DEBUG(29, shower->getArrayIndex() << " "  << shower->getIsTrack());

          // there is a 1:1 relation, just set the relation for the corresponding cluster as well
          ECLCluster* cluster = shower->getRelatedFrom<ECLCluster>();
          if (cluster != nullptr) {
            cluster->setIsTrack(true);
            if (m_angularDistanceMatching) {
              track.addRelationTo(cluster, 1.0, "AngularDistance");
            } else {
              track.addRelationTo(cluster, 1.0, "EnterCrystal");
            }
          }
        }
      } // end loop on hypothesis IDs
    }
    if (m_angularDistanceMatching) {
      // never match tracks pointing towards gaps or adjacent part of barrel using angular distance
      if (trackTowardsGap(theta)) continue;
      ECL::DetectorRegion trackDetectorRegion = ECL::getDetectorRegion(theta);
      // for low-pt tracks matching based on the angular distance is only applied if track points towards the FWD
      if (pt < m_matchingPTThreshold && trackDetectorRegion != ECL::DetectorRegion::FWD) continue;
      ECLCluster* cluster_best_cross = nullptr;
      ECLCluster* cluster_best_dl = nullptr;
      ECLCluster* cluster_best_near = nullptr;
      double quality_best_cross = 0, quality_best_dl = 0, quality_best_near = 0;
      // Find extrapolated track hits in the ECL, considering only hit points
      // that either are on the sphere, closest to, or on radial direction of an
      // ECLCluster.
      for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
        if (!isECLHit(extHit)) continue;
        ECLCluster* eclCluster = extHit.getRelatedFrom<ECLCluster>();
        if (eclCluster != nullptr) {
          if (eclCluster->getHypothesisId() != ECLCluster::c_nPhotons) continue;
          int eclDetectorRegion = eclCluster->getDetectorRegion();
          // accept only cluster from region matching track direction, exception for gaps
          if (abs(eclDetectorRegion - trackDetectorRegion) == 1) continue;
          // never match low-pt tracks with clusters in the barrel
          if (pt < m_matchingPTThreshold && eclDetectorRegion == ECL::DetectorRegion::BRL) continue;
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
          ExtHitStatus extHitStatus = extHit.getStatus();
          double quality = clusterQuality(deltaPhi, deltaTheta, pt, eclDetectorRegion, extHitStatus);
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
          track.addRelationTo(cluster_best_cross, 1.0, "AngularDistance");
          ECLShower* shower_cross = cluster_best_cross->getRelatedFrom<ECLShower>();
          if (shower_cross != nullptr) {
            shower_cross->setIsTrack(true);
            track.addRelationTo(shower_cross, 1.0, "AngularDistance");
          }
        } else if (cluster_best_dl != nullptr && quality_best_dl > m_matchingConsistency) {
          cluster_best_dl->setIsTrack(true);
          track.addRelationTo(cluster_best_dl, 1.0, "AngularDistance");
          ECLShower* shower_dl = cluster_best_dl->getRelatedFrom<ECLShower>();
          if (shower_dl != nullptr) {
            shower_dl->setIsTrack(true);
            track.addRelationTo(shower_dl, 1.0, "AngularDistance");
          }
        } else if (cluster_best_near != nullptr && quality_best_near > m_matchingConsistency) {
          cluster_best_near->setIsTrack(true);
          track.addRelationTo(cluster_best_near, 1.0, "AngularDistance");
          ECLShower* shower_near = cluster_best_near->getRelatedFrom<ECLShower>();
          if (shower_near != nullptr) {
            shower_near->setIsTrack(true);
            track.addRelationTo(shower_near, 1.0, "AngularDistance");
          }
        }
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
                                                     int eclDetectorRegion, int hitStatus) const
{
  double phi_consistency = phiConsistency(deltaPhi, pt, eclDetectorRegion, hitStatus);
  double theta_consistency = thetaConsistency(deltaTheta, pt, eclDetectorRegion, hitStatus);
  return phi_consistency * theta_consistency * (1 - log(phi_consistency * theta_consistency));
}

double ECLTrackClusterMatchingModule::phiConsistency(double deltaPhi, double pt, int eclDetectorRegion, int hitStatus) const
{
  double phi_RMS;
  if (eclDetectorRegion == ECL::DetectorRegion::FWD || eclDetectorRegion == ECL::DetectorRegion::FWDGap) {
    if (hitStatus == EXT_ECLCROSS) {
      phi_RMS = f_phiRMSFWDCROSS.Eval(pt);
    } else if (hitStatus == EXT_ECLDL) {
      phi_RMS = f_phiRMSFWDDL.Eval(pt);
    } else {
      phi_RMS = f_phiRMSFWDNEAR.Eval(pt);
    }
  } else if (eclDetectorRegion == ECL::DetectorRegion::BRL) {
    if (hitStatus == EXT_ECLCROSS) {
      phi_RMS = f_phiRMSBRLCROSS.Eval(pt);
    } else if (hitStatus == EXT_ECLDL) {
      phi_RMS = f_phiRMSBRLDL.Eval(pt);
    } else {
      phi_RMS = f_phiRMSBRLNEAR.Eval(pt);
    }
  } else if (eclDetectorRegion == ECL::DetectorRegion::BWD || eclDetectorRegion == ECL::DetectorRegion::BWDGap) {
    if (hitStatus == EXT_ECLCROSS) {
      phi_RMS = f_phiRMSBWDCROSS.Eval(pt);
    } else if (hitStatus == EXT_ECLDL) {
      phi_RMS = f_phiRMSBWDDL.Eval(pt);
    } else {
      phi_RMS = f_phiRMSBWDNEAR.Eval(pt);
    }
  } else { /* ECL cluster below acceptance */
    return 0;
  }
  return erfc(abs(deltaPhi) / phi_RMS);
}

double ECLTrackClusterMatchingModule::thetaConsistency(double deltaTheta, double pt, int eclDetectorRegion, int hitStatus) const
{
  double theta_RMS;
  if (eclDetectorRegion == ECL::DetectorRegion::FWD || eclDetectorRegion == ECL::DetectorRegion::FWDGap) {
    if (hitStatus == EXT_ECLCROSS) {
      theta_RMS = f_thetaRMSFWDCROSS.Eval(pt);
    } else if (hitStatus == EXT_ECLDL) {
      theta_RMS = f_thetaRMSFWDDL.Eval(pt);
    } else {
      theta_RMS = f_thetaRMSFWDNEAR.Eval(pt);
    }
  } else if (eclDetectorRegion == ECL::DetectorRegion::BRL) {
    if (hitStatus == EXT_ECLCROSS) {
      theta_RMS = f_thetaRMSBRLCROSS.Eval(pt);
    } else if (hitStatus == EXT_ECLDL) {
      theta_RMS = f_thetaRMSBRLDL.Eval(pt);
    } else {
      theta_RMS = f_thetaRMSBRLNEAR.Eval(pt);
    }
  } else if (eclDetectorRegion == ECL::DetectorRegion::BWD || eclDetectorRegion == ECL::DetectorRegion::BWDGap) {
    if (hitStatus == EXT_ECLCROSS) {
      theta_RMS = f_thetaRMSBWDCROSS.Eval(pt);
    } else if (hitStatus == EXT_ECLDL) {
      theta_RMS = f_thetaRMSBWDDL.Eval(pt);
    } else {
      theta_RMS = f_thetaRMSBWDNEAR.Eval(pt);
    }
  } else { /* ECL cluster below acceptance */
    return 0;
  }
  return erfc(abs(deltaTheta) / theta_RMS);
}

bool ECLTrackClusterMatchingModule::trackTowardsGap(double theta) const
{
  if (ECL::getDetectorRegion(theta) == ECL::DetectorRegion::BRL) {
    if (ECL::getDetectorRegion(theta - m_brlEdgeTheta) != ECL::DetectorRegion::BRL) return true;
    else if (ECL::getDetectorRegion(theta + m_brlEdgeTheta) != ECL::DetectorRegion::BRL) return true;
    else return false;
  } else return false;
}

void ECLTrackClusterMatchingModule::optimizedPTMatchingConsistency(double theta, double pt)
{
  if (ECL::getDetectorRegion(theta) == ECL::DetectorRegion::FWD || ECL::getDetectorRegion(theta) == ECL::DetectorRegion::FWDGap) {
    for (const auto& matchingThresholdPair : m_matchingThresholdValuesFWD) {
      if (pt < matchingThresholdPair.first) {
        m_matchingConsistency = matchingThresholdPair.second;
        break;
      }
    }
  } else if (ECL::getDetectorRegion(theta) == ECL::DetectorRegion::BRL) {
    for (const auto& matchingThresholdPair : m_matchingThresholdValuesBRL) {
      if (theta < matchingThresholdPair.first && pt < matchingThresholdPair.second.first) {
        m_matchingConsistency = matchingThresholdPair.second.second;
        break;
      }
    }
  } else if (ECL::getDetectorRegion(theta) == ECL::DetectorRegion::BWD
             || ECL::getDetectorRegion(theta) == ECL::DetectorRegion::BWDGap) {
    for (const auto& matchingThresholdPair : m_matchingThresholdValuesBWD) {
      if (pt < matchingThresholdPair.first) {
        m_matchingConsistency = matchingThresholdPair.second;
        break;
      }
    }
  }
}
