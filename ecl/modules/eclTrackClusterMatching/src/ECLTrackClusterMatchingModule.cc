/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ecl/modules/eclTrackClusterMatching/ECLTrackClusterMatchingModule.h>

#include <ecl/dbobjects/ECLTrackClusterMatchingParameterizations.h>
#include <ecl/dbobjects/ECLTrackClusterMatchingThresholds.h>
#include <ecl/utility/utilityFunctions.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/HitPatternCDC.h>

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
  addParam("minimalCDCHits", m_minimalCDCHits,
           "bad VXD-standalone tracks cause (too) low photon efficiency in end caps, temporarily fixed by requiring minimal number of CDC hits",
           -1);
  addParam("skipZeroChargeTracks", m_skipZeroChargeTracks,
           "switch to exclude tracks with zero charge from track-cluster matching", bool(false));
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
    m_tracks.registerRelationTo(m_eclShowers);
    m_tracks.registerRelationTo(m_eclClusters);
    m_tracks.registerRelationTo(m_eclShowers, DataStore::c_Event, DataStore::c_WriteOut, "AngularDistance");
    m_tracks.registerRelationTo(m_eclClusters, DataStore::c_Event, DataStore::c_WriteOut, "AngularDistance");

    // function to update parameterization functions if the payload changes
    auto updateParameterizationFunctions = [this]() {
      const auto& map = m_matchingParameterizations->getRMSParameterizations();

      f_phiRMSFWDCROSS = map.at("PhiFWDCROSS");
      f_phiRMSFWDDL = map.at("PhiFWDDL");
      f_phiRMSFWDNEAR = map.at("PhiFWDNEAR");
      f_phiRMSBRLCROSS = map.at("PhiBRLCROSS");
      f_phiRMSBRLDL = map.at("PhiBRLDL");
      f_phiRMSBRLNEAR = map.at("PhiBRLNEAR");
      f_phiRMSBWDCROSS = map.at("PhiBWDCROSS");
      f_phiRMSBWDDL = map.at("PhiBWDDL");
      f_phiRMSBWDNEAR = map.at("PhiBWDNEAR");
      f_thetaRMSFWDCROSS = map.at("ThetaFWDCROSS");
      f_thetaRMSFWDDL = map.at("ThetaFWDDL");
      f_thetaRMSFWDNEAR = map.at("ThetaFWDNEAR");
      f_thetaRMSBRLCROSS = map.at("ThetaBRLCROSS");
      f_thetaRMSBRLDL = map.at("ThetaBRLDL");
      f_thetaRMSBRLNEAR = map.at("ThetaBRLNEAR");
      f_thetaRMSBWDCROSS = map.at("ThetaBWDCROSS");
      f_thetaRMSBWDDL = map.at("ThetaBWDDL");
      f_thetaRMSBWDNEAR = map.at("ThetaBWDNEAR");
    };

    // function to update matching threshold functions if the payload changes
    auto updateMatchingThresholds = [this]() {
      m_matchingThresholdValuesFWD = m_matchingThresholds->getFWDMatchingThresholdValues();
      m_matchingThresholdValuesBRL = m_matchingThresholds->getBRLMatchingThresholdValues();
      m_matchingThresholdValuesBWD = m_matchingThresholds->getBWDMatchingThresholdValues();
    };

    // Update once right away
    updateParameterizationFunctions();
    updateMatchingThresholds();
    // And register to be called every time the payloads change
    m_matchingParameterizations.addCallback(updateParameterizationFunctions);
    m_matchingThresholds.addCallback(updateMatchingThresholds);
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
    // TEMPORARY FIX: require minimal number of CDC hits, otherwise exclude tracks from track-cluster matching procedure
    if (!(fitResult->getHitPatternCDC().getNHits() > m_minimalCDCHits)) continue;
    if (m_skipZeroChargeTracks && fitResult->getChargeSign() == 0) continue;
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
            track.addRelationTo(shower);
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
              track.addRelationTo(cluster);
              track.addRelationTo(cluster, 1.0, "AngularDistance");
            } else {
              track.addRelationTo(cluster, 1.0, "EnterCrystal");
            }
          }
        }
      } // end loop on hypothesis IDs
    }
    if (m_angularDistanceMatching) {
      // tracks should never be matched to more than one cluster
      if (track.getRelationsTo<ECLCluster>("", "AngularDistance").size() > 0) continue;
      // never match tracks pointing towards gaps or adjacent part of barrel using angular distance
      if (trackTowardsGap(theta)) continue;
      // for low-pt tracks matching based on the angular distance is only applied if track points towards the FWD
      ECL::DetectorRegion trackDetectorRegion = ECL::getDetectorRegion(theta);
      if (pt < m_matchingPTThreshold && trackDetectorRegion != ECL::DetectorRegion::FWD) continue;

      map<int, pair<double, int>> hypothesisIdBestQualityCROSSArrayIndexMap;
      map<int, pair<double, int>> hypothesisIdBestQualityDLArrayIndexMap;
      map<int, pair<double, int>> hypothesisIdBestQualityNEARArrayIndexMap;
      set<int> uniqueHypothesisIds;

      // Find extrapolated track hits in the ECL, considering only hit points
      // that either are on the sphere, closest to, or on radial direction of an
      // ECLCluster.
      for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
        if (!isECLHit(extHit)) continue;
        ECLCluster* eclCluster = extHit.getRelatedFrom<ECLCluster>();
        if (!eclCluster) continue;
        ECLShower* eclShower = eclCluster->getRelatedTo<ECLShower>();
        if (eclShower != nullptr) {
          // accept only shower from region matching track direction, exception for gaps
          int eclDetectorRegion = eclShower->getDetectorRegion();
          if (abs(eclDetectorRegion - trackDetectorRegion) == 1) continue;
          // never match low-pt tracks with showers in the barrel
          if (pt < m_matchingPTThreshold && eclDetectorRegion == ECL::DetectorRegion::BRL) continue;
          double phiHit = extHit.getPosition().Phi();
          double phiShower = eclShower->getPhi();
          double deltaPhi = phiHit - phiShower;
          if (deltaPhi > M_PI) {
            deltaPhi = deltaPhi - 2 * M_PI;
          } else if (deltaPhi < -M_PI) {
            deltaPhi = deltaPhi + 2 * M_PI;
          }
          double thetaHit = extHit.getPosition().Theta();
          double thetaShower = eclShower->getTheta();
          double deltaTheta = thetaHit - thetaShower;
          ExtHitStatus extHitStatus = extHit.getStatus();
          double quality = showerQuality(deltaPhi, deltaTheta, pt, eclDetectorRegion, extHitStatus);
          int hypothesisId = eclShower->getHypothesisId();
          bool inserted = (uniqueHypothesisIds.insert(hypothesisId)).second;
          if (inserted) {
            hypothesisIdBestQualityCROSSArrayIndexMap.insert(make_pair(hypothesisId, make_pair(0, -1)));
            hypothesisIdBestQualityDLArrayIndexMap.insert(make_pair(hypothesisId, make_pair(0, -1)));
            hypothesisIdBestQualityNEARArrayIndexMap.insert(make_pair(hypothesisId, make_pair(0, -1)));
          }
          if (extHitStatus == EXT_ECLCROSS) {
            if (quality > hypothesisIdBestQualityCROSSArrayIndexMap.at(hypothesisId).first) {
              hypothesisIdBestQualityCROSSArrayIndexMap[hypothesisId] = make_pair(quality, eclShower->getArrayIndex());
            }
          } else if (extHitStatus == EXT_ECLDL) {
            if (quality > hypothesisIdBestQualityDLArrayIndexMap.at(hypothesisId).first) {
              hypothesisIdBestQualityDLArrayIndexMap[hypothesisId] = make_pair(quality, eclShower->getArrayIndex());
            }
          } else {
            if (quality > hypothesisIdBestQualityNEARArrayIndexMap.at(hypothesisId).first) {
              hypothesisIdBestQualityNEARArrayIndexMap[hypothesisId] = make_pair(quality, eclShower->getArrayIndex());
            }
          }
        }
      } // end loop on ExtHits related to Track

      vector<map<int, pair<double, int>>> hypothesisIdBestQualityArrayIndexMaps;
      hypothesisIdBestQualityArrayIndexMaps.push_back(hypothesisIdBestQualityCROSSArrayIndexMap);
      hypothesisIdBestQualityArrayIndexMaps.push_back(hypothesisIdBestQualityDLArrayIndexMap);
      hypothesisIdBestQualityArrayIndexMaps.push_back(hypothesisIdBestQualityNEARArrayIndexMap);
      if (m_useOptimizedMatchingConsistency) optimizedPTMatchingConsistency(theta, pt);
      for (const auto& uniqueHypothesisId : uniqueHypothesisIds) {
        for (const auto& hypothesisIdBestQualityArrayIndexMap : hypothesisIdBestQualityArrayIndexMaps) {
          if (hypothesisIdBestQualityArrayIndexMap.at(uniqueHypothesisId).first > m_matchingConsistency
              && hypothesisIdBestQualityArrayIndexMap.at(uniqueHypothesisId).second > -1) {
            auto shower = m_eclShowers[hypothesisIdBestQualityArrayIndexMap.at(uniqueHypothesisId).second];
            shower->setIsTrack(true);
            track.addRelationTo(shower);
            track.addRelationTo(shower, 1.0, "AngularDistance");
            ECLCluster* cluster = shower->getRelatedFrom<ECLCluster>();
            if (cluster != nullptr) {
              cluster->setIsTrack(true);
              track.addRelationTo(cluster);
              track.addRelationTo(cluster, 1.0, "AngularDistance");
            }
            break;
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

double ECLTrackClusterMatchingModule::showerQuality(double deltaPhi, double deltaTheta, double pt,
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
