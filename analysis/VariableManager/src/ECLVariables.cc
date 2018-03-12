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
#include <analysis/utility/C2TDistanceUtility.h>
#include <analysis/dataobjects/ECLEnergyCloseToTrack.h>

//MDST
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>

//ROOT
#include <TVector3.h>

#include <cmath>

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
      int result = -999;
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

    double minCluster2HelixDistance(const Particle* particle)
    {
      // Needed StoreArrays
      StoreArray<ECLCluster> eclClusters;
      StoreArray<Track> tracks;

      // Initialize variables
      ECLCluster* ecl = nullptr;
      Track* track = nullptr;

      // If neutral particle, 'getECLCluster'; if charged particle, 'getTrack->getECLCluster'; if no ECLCluster, return -1.0
      if (particle->getCharge() == 0)
        ecl = eclClusters[particle->getMdstArrayIndex()];
      else {
        track = tracks[particle->getMdstArrayIndex()];
        if (track)
          ecl = track->getRelatedTo<ECLCluster>();
      }

      if (!ecl)
        return -1.0;

      TVector3 v1raw = ecl->getClusterPosition();
      TVector3 v1 = C2TDistanceUtility::clipECLClusterPosition(v1raw);

      // Get closest track from Helix
      float minDistHelix = 999.9;

      for (int iTrack = 0; iTrack < tracks.getEntries(); iTrack++) {

        //TODO: expand use to all ChargeStable particles
        const TrackFitResult* tfr = tracks[iTrack]->getTrackFitResultWithClosestMass(Const::pion);
        Helix helix = tfr->getHelix();

        TVector3 tempv2helix = C2TDistanceUtility::getECLTrackHitPosition(helix, v1);
        if (tempv2helix.Mag() == 999.9)
          continue;

        double tempDistHelix = (tempv2helix - v1).Mag();

        if (tempDistHelix < minDistHelix) {
          minDistHelix = tempDistHelix;
        }
      }

      return minDistHelix;
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

    VARIABLE_GROUP("ECL Cluster related");
    REGISTER_VARIABLE("clusterReg", eclClusterDetectionRegion,
                      "Returns an integer code for the ECL region of a cluster:\n"
                      "1 - forward, 2 - barrel, 3 - backward, 11 - between FWD and barrel, 13 - between BWD and barrel, 0 - otherwise)");
    REGISTER_VARIABLE("clusterDeltaLTemp", eclClusterDeltaL,
                      "Returns DeltaL for the shower shape.\n"
                      "NOTE : this distance is calculated on the reconstructed level and is temporarily\n"
                      "included to the ECLCLuster MDST data format for studying purposes. If it is found\n"
                      "that it is not crucial for physics analysis then this variable will be removed in future releases.\n"
                      "Therefore, keep in mind that this variable might be removed in the future!");
    REGISTER_VARIABLE("minC2TDistTemp", eclClusterIsolation,
                      "Return distance from eclCluster to nearest track hitting the ECL.\n"
                      "NOTE : this distance is calculated on the reconstructed level and is temporarily\n"
                      "included to the ECLCLuster MDST data format for studying purposes. If it is found\n"
                      "to be effectively replaced by the \'minC2HDist\', which can be calculated\n"
                      "on the analysis level then this variable will be removed in future releases.\n"
                      "Therefore, keep in mind that this variable might be removed in the future!");
    REGISTER_VARIABLE("minC2HDist", minCluster2HelixDistance,
                      "Returns distance from eclCluster to nearest point on nearest Helix at the ECL cylindrical radius.");
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
                      "Returns ECL cluster's hadron scintillation component intensity.");
    REGISTER_VARIABLE("ClusterNumberOfHadronDigits", eclClusterNumberOfHadronDigits,
                      "Returns ECL cluster's weighted sum of hadron digits (current weights are all 1.0).");
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
  }
}
