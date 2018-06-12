/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alon Hershenhorn, Torben Ferber                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/ECLVariables.h>

//framework
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>

//analysis
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ECLEnergyCloseToTrack.h>

//MDST
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/EventLevelClusteringInfo.h>

//ROOT
#include <TVector3.h>

#include <cmath>
#include <stack>

namespace Belle2 {
  namespace Variable {

    double eclClusterHadronIntensity(const Particle* particle)
    {
      double result = -999.0;
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        if (eclClusterHasPulseShapeDiscrimination(particle)) {
          result = cluster->getClusterHadronIntensity();
        }
      }
      return result;
    }

    double eclClusterNumberOfHadronDigits(const Particle* particle)
    {
      double result = -999;
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        if (eclClusterHasPulseShapeDiscrimination(particle)) {
          result = cluster->getNumberOfHadronDigits();
        }
      }
      return result;
    }

    double eclClusterDetectionRegion(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster)
        result = cluster->getDetectorRegion();

      return result;
    }

    double eclClusterIsolation(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster)
        result = cluster->getMinTrkDistance();

      return result;
    }

    double eclClusterConnectedRegionID(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster)
        result = cluster->getConnectedRegionId();

      return result;
    }

    double eclClusterDeltaL(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster)
        result = cluster->getDeltaL();

      return result;
    }

    bool isGoodBelleGamma(int region, double energy)
    {
      bool goodGammaRegion1, goodGammaRegion2, goodGammaRegion3;
      goodGammaRegion1 = region == 1 && energy > 0.100;
      goodGammaRegion2 = region == 2 && energy > 0.050;
      goodGammaRegion3 = region == 3 && energy > 0.150;

      return goodGammaRegion1 || goodGammaRegion2 || goodGammaRegion3;
    }

    double goodBelleGamma(const Particle* particle)
    {
      double energy = particle->getEnergy();
      int region = eclClusterDetectionRegion(particle);

      return (double) isGoodBelleGamma(region, energy);
    }

    double eclClusterErrorE(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getUncertaintyEnergy();
      }
      return result;
    }

    double eclClusterUncorrectedE(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getEnergyRaw();
      }
      return result;
    }

    double eclClusterE(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getEnergy();
      }
      return result;
    }

    double eclClusterHighestE(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getEnergyHighestCrystal();
      }
      return result;
    }

    double eclClusterTiming(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getTime();
      }
      return result;
    }

    double eclClusterErrorTiming(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getDeltaTime99();
      }
      return result;
    }

    double eclClusterTheta(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getTheta();
      }
      return result;
    }

    double eclClusterErrorTheta(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* shower = particle->getECLCluster();
      if (shower) {
        result = shower->getUncertaintyTheta();
      }
      return result;
    }

    double eclClusterErrorPhi(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* shower = particle->getECLCluster();
      if (shower) {
        result = shower->getUncertaintyPhi();
      }
      return result;
    }

    double eclClusterPhi(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getPhi();
      }
      return result;
    }

    double eclClusterR(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getR();
      }
      return result;
    }

    double eclClusterE1E9(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getE1oE9();
      }
      return result;
    }

    double eclClusterE9E21(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getE9oE21();
      }
      return result;
    }

    double eclClusterAbsZernikeMoment40(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getAbsZernike40();
      }
      return result;
    }

    double eclClusterAbsZernikeMoment51(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getAbsZernike51();
      }
      return result;
    }

    double eclClusterZernikeMVA(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getZernikeMVA();
      }
      return result;
    }

    double eclClusterSecondMoment(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getSecondMoment();
      }
      return result;
    }

    double eclClusterLAT(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getLAT();
      }
      return result;
    }

    double eclClusterMergedPi0(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        //result = cluster->getMergedPi0();
      }
      return result;
    }

    double eclClusterNHits(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getNumberOfCrystals();
      }
      return result;
    }

    double eclClusterTrackMatched(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        const Track* track = cluster->getRelated<Track>();

        if (track)
          result = 1.0;
      }
      return result;

    }

    double eclClusterConnectedRegionId(const Particle* particle)
    {
      double result = -1.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getConnectedRegionId();
      }
      return result;
    }

    double eclClusterUniqueId(const Particle* particle)
    {
      double result = -1.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getUniqueId();
      }
      return result;
    }

    double eclClusterId(const Particle* particle)
    {
      double result = -1.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getClusterId();
      }
      return result;
    }

    double eclClusterHypothesisId(const Particle* particle)
    {
      double result = -1.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->getHypothesisId();
      }
      return result;
    }

    double eclClusterHasPulseShapeDiscrimination(const Particle* particle)
    {
      int result = 0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        result = cluster->hasPulseShapeDiscrimination();
      }
      return result;
    }

    double eclClusterTrigger(const Particle* particle)
    {
      double result = -1.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        const bool matcher = cluster->hasTriggerClusterMatching();

        if (matcher) {
          result = cluster->isTriggerCluster();
        } else {
          B2WARNING("Particle has an associated ECLCluster but the ECLTriggerClusterMatcher module has not been run!");
        }
      }
      return result;
    }

    double eclExtTheta(const Particle* particle)
    {
      double result = -1.0;
      const Track* track = particle->getTrack();
      if (track) {

        ECLEnergyCloseToTrack* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

        if (eclinfo) {
          result = eclinfo->getExtTheta();
        } else {
          B2WARNING("Relation to ECLEnergyCloseToTrack not found, did you forget to run ECLTrackCalDigitMatchModule?");
        }
      }

      return result;
    }

    double eclExtPhi(const Particle* particle)
    {
      double result = -1.0;
      const Track* track = particle->getTrack();
      if (track) {

        ECLEnergyCloseToTrack* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

        if (eclinfo) {
          result = eclinfo->getExtPhi();
        } else {
          B2WARNING("Relation to ECLEnergyCloseToTrack not found, did you forget to run ECLTrackCalDigitMatchModule?");
        }
      }

      return result;
    }

    double eclExtPhiId(const Particle* particle)
    {
      double result = -1.0;
      const Track* track = particle->getTrack();
      if (track) {

        ECLEnergyCloseToTrack* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

        if (eclinfo) {
          result = eclinfo->getExtPhiId();
        } else {
          B2WARNING("Relation to ECLEnergyCloseToTrack not found, did you forget to run ECLTrackCalDigitMatchModule?");
        }
      }

      return result;
    }

    double eclEnergy3FWDBarrel(const Particle* particle)
    {
      double result = -1.0;
      const Track* track = particle->getTrack();
      if (track) {

        ECLEnergyCloseToTrack* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

        if (eclinfo) {
          result = eclinfo->getEnergy3FWDBarrel();
        } else {
          B2WARNING("Relation to ECLEnergyCloseToTrack not found, did you forget to run ECLTrackCalDigitMatchModule?");
        }
      }

      return result;
    }

    double eclEnergy3FWDEndcap(const Particle* particle)
    {
      double result = -1.0;
      const Track* track = particle->getTrack();
      if (track) {

        ECLEnergyCloseToTrack* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

        if (eclinfo) {
          result = eclinfo->getEnergy3FWDEndcap();
        } else {
          B2WARNING("Relation to ECLEnergyCloseToTrack not found, did you forget to run ECLTrackCalDigitMatchModule?");
        }
      }

      return result;
    }

    double eclEnergy3BWDEndcap(const Particle* particle)
    {
      double result = -1.0;
      const Track* track = particle->getTrack();
      if (track) {

        ECLEnergyCloseToTrack* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

        if (eclinfo) {
          result = eclinfo->getEnergy3BWDEndcap();
        } else {
          B2WARNING("Relation to ECLEnergyCloseToTrack not found, did you forget to run ECLTrackCalDigitMatchModule?");
        }
      }

      return result;
    }

    double eclEnergy3BWDBarrel(const Particle* particle)
    {
      double result = -1.0;
      const Track* track = particle->getTrack();
      if (track) {

        ECLEnergyCloseToTrack* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

        if (eclinfo) {
          result = eclinfo->getEnergy3BWDBarrel();
        } else {
          B2WARNING("Relation to ECLEnergyCloseToTrack not found, did you forget to run ECLTrackCalDigitMatchModule?");
        }
      }

      return result;
    }

    double weightedAverageECLTime(const Particle* particle)
    {
      int nDaughters = int(particle->getNDaughters());
      if (nDaughters < 1) {
        B2WARNING("The provided particle has no daughters!");
        return std::numeric_limits<float>::quiet_NaN();
      }

      double numer = 0, denom = 0;
      double time, deltatime;
      int numberOfPhotonicDaughters = 0;

      /*
                                      ** TODO !!! **
       Use Martin Ritter's 1337 Particle::forEachDaughter once pull-request #2119 is merged.
      */
      std::stack<const Particle*> stacked;
      stacked.push(particle);
      while (!stacked.empty()) {
        const Particle* current = stacked.top();
        stacked.pop();

        int PDGcode = current->getPDGCode();
        if (PDGcode == 22) {
          numberOfPhotonicDaughters ++;
          const ECLCluster* cluster = current->getECLCluster();
          time = cluster->getTime();
          B2DEBUG(10, "time[" << numberOfPhotonicDaughters << "] = " << time);
          deltatime = cluster->getDeltaTime99();
          B2DEBUG(10, "deltatime[" << numberOfPhotonicDaughters << "] = " << deltatime);
          numer += time / pow(deltatime, 2);
          B2DEBUG(11, "numer[" << numberOfPhotonicDaughters << "] = " << numer);
          denom += 1 / pow(deltatime, 2);
          B2DEBUG(11, "denom[" << numberOfPhotonicDaughters << "] = " << denom);
        } else {
          const std::vector<Particle*> daughters = current->getDaughters();
          for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
            stacked.push(daughters[iDaughter]);
          }
        }
      }
      if (numberOfPhotonicDaughters < 1) {
        B2WARNING("There are no photons amongst the daughters of the provided particle!");
        return std::numeric_limits<float>::quiet_NaN();
      }

      if (denom == 0) {
        B2WARNING("The denominator of the weighted mean is zero!");
        return std::numeric_limits<float>::quiet_NaN();
      } else {
        B2DEBUG(10, "numer/denom = " << numer / denom);
        return numer / denom;
      }
    }

    double maxWeightedDistanceFromAverageECLTime(const Particle* particle)
    {
      int nDaughters = int(particle->getNDaughters());
      if (nDaughters < 1) {
        B2WARNING("The provided particle has no daughters!");
        return std::numeric_limits<float>::quiet_NaN();
      }

      double averageECLTime, maxTimeDiff = -1;
      double time, deltatime, maxTimeDiff_temp;
      int numberOfPhotonicDaughters = 0;

      averageECLTime = weightedAverageECLTime(particle);

      std::stack<const Particle*> stacked;
      stacked.push(particle);
      while (!stacked.empty()) {
        const Particle* current = stacked.top();
        stacked.pop();

        int PDGcode = current->getPDGCode();
        if (PDGcode == 22) {
          numberOfPhotonicDaughters ++;
          const ECLCluster* cluster = current->getECLCluster();
          time = cluster->getTime();
          B2DEBUG(10, "time[" << numberOfPhotonicDaughters << "] = " << time);
          deltatime = cluster->getDeltaTime99();
          B2DEBUG(10, "deltatime[" << numberOfPhotonicDaughters << "] = " << deltatime);
          maxTimeDiff_temp = fabs((time - averageECLTime) / deltatime);
          B2DEBUG(11, "maxTimeDiff_temp[" << numberOfPhotonicDaughters << "] = " << maxTimeDiff_temp);
          if (maxTimeDiff_temp > maxTimeDiff)
            maxTimeDiff = maxTimeDiff_temp;
          B2DEBUG(11, "maxTimeDiff[" << numberOfPhotonicDaughters << "] = " << maxTimeDiff);
        } else {
          const std::vector<Particle*> daughters = current->getDaughters();
          for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
            stacked.push(daughters[iDaughter]);
          }
        }
      }
      if (numberOfPhotonicDaughters < 1) {
        B2WARNING("There are no photons amongst the daughters of the provided particle!");
        return std::numeric_limits<float>::quiet_NaN();
      }

      if (maxTimeDiff < 0) {
        B2WARNING("The max time difference is negative!");
        return std::numeric_limits<float>::quiet_NaN();
      } else {
        B2DEBUG(10, "maxTimeDiff = " << maxTimeDiff);
        return maxTimeDiff;
      }
    }

    /*************************************************************
     * Event-based ECL clustering information
     */
    double nECLOutOfTimeCrystalsFWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return std::numeric_limits<double>::quiet_NaN();
      return (double)elci->getNECLCalDigitsOutOfTimeFWD();
    }

    double nECLOutOfTimeCrystalsBarrel(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return std::numeric_limits<double>::quiet_NaN();
      return (double)elci->getNECLCalDigitsOutOfTimeBarrel();
    }

    double nECLOutOfTimeCrystalsBWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return std::numeric_limits<double>::quiet_NaN();
      return (double)elci->getNECLCalDigitsOutOfTimeBWD();
    }

    double nECLOutOfTimeCrystals(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return std::numeric_limits<double>::quiet_NaN();
      return (double)elci->getNECLCalDigitsOutOfTime();
    }

    double nRejectedECLShowersFWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return std::numeric_limits<double>::quiet_NaN();
      return (double)elci->getNECLShowersRejectedFWD();
    }

    double nRejectedECLShowersBarrel(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return std::numeric_limits<double>::quiet_NaN();
      return (double)elci->getNECLShowersRejectedBarrel();
    }

    double nRejectedECLShowersBWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return std::numeric_limits<double>::quiet_NaN();
      return (double)elci->getNECLShowersRejectedBWD();
    }

    double nRejectedECLShowers(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return std::numeric_limits<double>::quiet_NaN();
      return (double)elci->getNECLShowersRejected();
    }

    double eclClusterEoP(const Particle* part)
    {
      const double E = eclClusterE(part);
      const double p =  part->getMomentumMagnitude();
      if (0 == p) { return std::nan(""); }
      return E / p;
    }


    VARIABLE_GROUP("ECL Cluster related");
    REGISTER_VARIABLE("clusterEoP", eclClusterEoP, "uncorrelated E over P");
    REGISTER_VARIABLE("clusterReg", eclClusterDetectionRegion,
                      "Returns an integer code for the ECL region of a cluster:\n"
                      "1 - forward, 2 - barrel, 3 - backward, 11 - between FWD and barrel, 13 - between BWD and barrel, 0 - otherwise)");
    REGISTER_VARIABLE("clusterDeltaLTemp", eclClusterDeltaL,
                      "Returns DeltaL for the shower shape.\n"
                      "NOTE : this distance is calculated on the reconstructed level and is temporarily\n"
                      "included to the ECLCLuster MDST data format for studying purposes. If it is found\n"
                      "that it is not crucial for physics analysis then this variable will be removed in future releases.\n"
                      "Therefore, keep in mind that this variable might be removed in the future!");
    REGISTER_VARIABLE("minC2TDist", eclClusterIsolation,
                      "Return distance from eclCluster to nearest track hitting the ECL.\n"
                      "NOTE : this distance is calculated on the reconstructed level");
    REGISTER_VARIABLE("goodBelleGamma", goodBelleGamma,
                      "[Legacy] Returns 1.0 if photon candidate passes simple region dependent energy selection for Belle data and MC (50/100/150 MeV).");
    REGISTER_VARIABLE("clusterE", eclClusterE, "Returns ECL cluster's corrected energy.");
    REGISTER_VARIABLE("clusterErrorE", eclClusterErrorE,
                      "Returns ECL cluster's uncertainty on energy (from background level and energy dependent tabulation).");
    REGISTER_VARIABLE("clusterErrorPhi", eclClusterErrorPhi,
                      "Returns ECL cluster's uncertainty on phi (from background level and energy dependent tabulation).");
    REGISTER_VARIABLE("clusterErrorTheta", eclClusterErrorTheta,
                      "Returns ECL cluster's uncertainty on theta (from background level and energy dependent tabulation).");

    REGISTER_VARIABLE("clusterUncorrE", eclClusterUncorrectedE,
                      "Returns ECL cluster's uncorrected energy.");
    REGISTER_VARIABLE("clusterR", eclClusterR,
                      "Returns ECL cluster's centroid distance from (0,0,0).");
    REGISTER_VARIABLE("clusterPhi", eclClusterPhi,
                      "Returns ECL cluster's azimuthal angle (this is not generally equal to a photon azimuthal angle).");
    REGISTER_VARIABLE("clusterConnectedRegionID", eclClusterConnectedRegionID,
                      "Returns ECL cluster's connected region ID.");
    REGISTER_VARIABLE("clusterBelleQuality", eclClusterDeltaL,
                      "Returns ECL cluster's quality indicating a good cluster in GSIM (stored in deltaL of ECL cluster object)."
                      "The Belle people used only clusters with quality == 0 in their E_{extra_ecl} (Belle only).");
    REGISTER_VARIABLE("clusterTheta", eclClusterTheta,
                      "Returns ECL cluster's polar angle (this is not generally equal to a photon polar angle).");
    REGISTER_VARIABLE("clusterTiming", eclClusterTiming,
                      "Returns ECL cluster's timing.");
    REGISTER_VARIABLE("clusterErrorTiming", eclClusterErrorTiming,
                      "Returns ECL cluster's timing uncertainty that contains 99% of true photons.");
    REGISTER_VARIABLE("clusterHighestE", eclClusterHighestE,
                      "Returns energy of the crystal with highest energy in the ECLCluster.");
    REGISTER_VARIABLE("clusterE1E9", eclClusterE1E9,
                      "Returns ratio of energies of the central crystal and 3x3 crystals around the central crystal.");
    REGISTER_VARIABLE("clusterE9E25", eclClusterE9E25,
                      "Deprecated - kept for backwards compatibility - returns clusterE9E21.");
    REGISTER_VARIABLE("clusterE9E21", eclClusterE9E21,
                      "Returns ratio of energies in inner 3x3 and (5x5 cells without corners).");
    REGISTER_VARIABLE("clusterAbsZernikeMoment40", eclClusterAbsZernikeMoment40,
                      "Returns absolute value of Zernike moment 40 (shower shape variable).");
    REGISTER_VARIABLE("clusterAbsZernikeMoment51", eclClusterAbsZernikeMoment51,
                      "Returns absolute value of Zernike moment 51 (shower shape variable).");
    REGISTER_VARIABLE("clusterZernikeMVA", eclClusterZernikeMVA,
                      "Returns output of a MVA using eleven Zernike moments of the cluster.\n"
                      "For cluster with hypothesisId==N1: raw MVA output.\n"
                      "For cluster with hypothesisId==N2: 1 - prod{clusterZernikeMVA}, where the product is on all N1 showers belonging to the same connected region (shower shape variable)");
    REGISTER_VARIABLE("clusterSecondMoment", eclClusterSecondMoment,
                      "Returns second moment.");
    REGISTER_VARIABLE("clusterLAT", eclClusterLAT,
                      "Returns lateral energy distribution (shower variable).");
    REGISTER_VARIABLE("clusterMergedPi0", eclClusterMergedPi0,
                      "Returns high momentum pi0 likelihood (not available yet).");
    REGISTER_VARIABLE("clusterNHits", eclClusterNHits,
                      "Returns sum of crystal weights sum(w_i) with w_i<=1  associated to this cluster. for non-overlapping clusters this is equal to the number of crystals in the cluster.");
    REGISTER_VARIABLE("clusterTrackMatch", eclClusterTrackMatched,
                      "Returns 1.0 if at least one charged track is matched to this ECL cluster.");
    REGISTER_VARIABLE("clusterCRID", eclClusterConnectedRegionId,
                      "Returns ECL cluster's connected region ID.");
    REGISTER_VARIABLE("ClusterHasPulseShapeDiscrimination", eclClusterHasPulseShapeDiscrimination,
                      "Status bit to indicate if cluster has digits with waveforms that passed energy and chi2 thresholds for computing PSD variables.");
    REGISTER_VARIABLE("ClusterHadronIntensity", eclClusterHadronIntensity,
                      "Returns ECL cluster's Cluster Hadron Component Intensity (pulse shape discrimination variable). \n"
                      "Sum of the CsI(Tl) hadron scintillation component emission normalized to the sum of CsI(Tl) total scintillation emission. \n"
                      "Computed only using cluster digits with energy greater than 50 MeV and good offline waveform fit chi2.");
    REGISTER_VARIABLE("ClusterNumberOfHadronDigits", eclClusterNumberOfHadronDigits,
                      "Returns ECL cluster's Number of hadron digits in cluster (pulse shape discrimination variable). \n"
                      "Weighted sum of digits in cluster with significant scintillation emission (> 3 MeV) in the hadronic scintillation component. \n"
                      "Computed only using cluster digits with energy greater than 50 MeV and good offline waveform fit chi2.");
    REGISTER_VARIABLE("clusterClusterID", eclClusterId,
                      "Returns the ECL cluster id of this ECL cluster within the connected region to which it belongs to. Use clusterUniqueID to get an unique ID.");
    REGISTER_VARIABLE("clusterHypothesis", eclClusterHypothesisId,
                      "Returns the hypothesis ID of this ECL cluster.");
    REGISTER_VARIABLE("clusterUniqueID", eclClusterUniqueId,
                      "Returns the unique ID (based on CR, shower in CR and hypothesis) of this ECL cluster.");
    REGISTER_VARIABLE("clusterTrigger", eclClusterTrigger,
                      "Returns 1.0 if the ECLCluster is matched to a trigger cluster (requires to run eclTriggerClusterMatcher (which requires TRGECLClusters in the input file)) and 0 otherwise. Returns -1 if the matching code was not run.");
    REGISTER_VARIABLE("eclExtTheta", eclExtTheta, "Returns extrapolated theta.");
    REGISTER_VARIABLE("eclExtPhi", eclExtPhi, "Returns extrapolated phi.");
    REGISTER_VARIABLE("eclExtPhiId", eclExtPhiId, "Returns extrapolated phi id.");
    REGISTER_VARIABLE("eclEnergy3FWDBarrel", eclEnergy3FWDBarrel, "Returns energy sum of three crystals in FWD barrel");
    REGISTER_VARIABLE("eclEnergy3FWDEndcap", eclEnergy3FWDEndcap, "Returns energy sum of three crystals in FWD endcap");
    REGISTER_VARIABLE("eclEnergy3BWDBarrel", eclEnergy3BWDBarrel, "Returns energy sum of three crystals in BWD barrel");
    REGISTER_VARIABLE("eclEnergy3BWDEndcap", eclEnergy3BWDEndcap, "Returns energy sum of three crystals in BWD endcap");
    REGISTER_VARIABLE("weightedAverageECLTime", weightedAverageECLTime,
                      "Returns the ECL weighted average time of all the photons daughters (of any generation) of the provided particle");
    REGISTER_VARIABLE("maxWeightedDistanceFromAverageECLTime", maxWeightedDistanceFromAverageECLTime,
                      "Returns the maximum weighted distance between the time of the cluster of a photon and the ECL average time, amongst the cluster associated to the photonic daughters (of all generations) of the provided particle");

    REGISTER_VARIABLE("nECLOutOfTimeCrystals", nECLOutOfTimeCrystals,
                      "[Eventbased] return the number of crystals (ECLCalDigits) that are out of time");
    REGISTER_VARIABLE("nECLOutOfTimeCrystalsFWDEndcap", nECLOutOfTimeCrystalsFWDEndcap,
                      "[Eventbased] return the number of crystals (ECLCalDigits) that are out of time in the FWD endcap");
    REGISTER_VARIABLE("nECLOutOfTimeCrystalsBarrel", nECLOutOfTimeCrystalsBarrel,
                      "[Eventbased] return the number of crystals (ECLCalDigits) that are out of time in the barrel");
    REGISTER_VARIABLE("nECLOutOfTimeCrystalsBWDEndcap", nECLOutOfTimeCrystalsBWDEndcap,
                      "[Eventbased] return the number of crystals (ECLCalDigits) that are out of time in the FWD endcap");
    REGISTER_VARIABLE("nRejectedECLShowers", nRejectedECLShowers,
                      "[Eventbased] return the number of showers in the ECL that do not become clusters");
    REGISTER_VARIABLE("nRejectedECLShowersFWDEndcap", nRejectedECLShowersFWDEndcap,
                      "[Eventbased] return the number of showers in the ECL that do not become clusters, from the FWD endcap");
    REGISTER_VARIABLE("nRejectedECLShowersBarrel", nRejectedECLShowersBarrel,
                      "[Eventbased] return the number of showers in the ECL that do not become clusters, from the barrel");
    REGISTER_VARIABLE("nRejectedECLShowersBWDEndcap", nRejectedECLShowersBWDEndcap,
                      "[Eventbased] return the number of showers in the ECL that do not become clusters, from the BWD endcap");

  }
}











