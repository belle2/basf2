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
#include <analysis/variables/ECLVariables.h>

//framework
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>

//analysis
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ECLEnergyCloseToTrack.h>
#include <analysis/dataobjects/ECLTRGInformation.h>
#include <analysis/dataobjects/ECLTriggerCell.h>

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
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        if (eclClusterHasPulseShapeDiscrimination(particle)) {
          return cluster->getClusterHadronIntensity();
        } else
          return -1.0;
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterNumberOfHadronDigits(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        if (eclClusterHasPulseShapeDiscrimination(particle)) {
          return cluster->getNumberOfHadronDigits();
        } else
          return -1.0;
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterDetectionRegion(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster)
        return cluster->getDetectorRegion();

      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterIsolation(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster)
        return cluster->getMinTrkDistance();

      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterConnectedRegionID(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster)
        return  cluster->getConnectedRegionId();

      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterDeltaL(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster)
        return cluster->getDeltaL();

      return std::numeric_limits<float>::quiet_NaN();
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

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getUncertaintyEnergy();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterUncorrectedE(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getEnergyRaw();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterE(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getEnergy();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterHighestE(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getEnergyHighestCrystal();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterTiming(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getTime();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterErrorTiming(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getDeltaTime99();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterTheta(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getTheta();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterErrorTheta(const Particle* particle)
    {

      const ECLCluster* cluster  = particle->getECLCluster();
      if (cluster) {
        return cluster->getUncertaintyTheta();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterErrorPhi(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getUncertaintyPhi();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterPhi(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getPhi();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterR(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getR();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterE1E9(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getE1oE9();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterE9E21(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getE9oE21();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterAbsZernikeMoment40(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getAbsZernike40();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterAbsZernikeMoment51(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getAbsZernike51();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterZernikeMVA(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getZernikeMVA();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterSecondMoment(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getSecondMoment();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterLAT(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getLAT();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterNHits(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getNumberOfCrystals();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterTrackMatched(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        const Track* track = cluster->getRelated<Track>();

        if (track)
          return 1.0;
        else
          return 0.0;
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double nECLClusterTrackMatches(const Particle* particle)
    {
      // if no ECL cluster then nan
      const ECLCluster* cluster = particle->getECLCluster();
      if (!cluster)
        return std::numeric_limits<double>::quiet_NaN();

      // one or more tracks may be matched to charged particles
      size_t out = cluster->getRelationsFrom<Track>().size();
      return double(out);
    }

    double eclClusterConnectedRegionId(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getConnectedRegionId();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterUniqueId(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getUniqueId();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterId(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getClusterId();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterHypothesisId(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getHypothesisId();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterHasPulseShapeDiscrimination(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->hasPulseShapeDiscrimination();
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterTrigger(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        const bool matcher = cluster->hasTriggerClusterMatching();

        if (matcher) {
          return cluster->isTriggerCluster();
        } else {
          B2WARNING("Particle has an associated ECLCluster but the ECLTriggerClusterMatcher module has not been run!");
          return -1.;
        }
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclExtTheta(const Particle* particle)
    {

      const Track* track = particle->getTrack();
      if (track) {

        ECLEnergyCloseToTrack* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

        if (eclinfo) {
          return eclinfo->getExtTheta();
        } else {
          B2WARNING("Relation to ECLEnergyCloseToTrack not found, did you forget to run ECLTrackCalDigitMatchModule?");
          return std::numeric_limits<float>::quiet_NaN();
        }
      }

      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclExtPhi(const Particle* particle)
    {

      const Track* track = particle->getTrack();
      if (track) {

        ECLEnergyCloseToTrack* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

        if (eclinfo) {
          return eclinfo->getExtPhi();
        } else {
          B2WARNING("Relation to ECLEnergyCloseToTrack not found, did you forget to run ECLTrackCalDigitMatchModule?");
          return std::numeric_limits<float>::quiet_NaN();
        }
      }

      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclExtPhiId(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (track) {

        ECLEnergyCloseToTrack* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

        if (eclinfo) {
          return eclinfo->getExtPhiId();
        } else {
          B2WARNING("Relation to ECLEnergyCloseToTrack not found, did you forget to run ECLTrackCalDigitMatchModule?");
          return std::numeric_limits<float>::quiet_NaN();
        }
      }

      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclEnergy3FWDBarrel(const Particle* particle)
    {

      const Track* track = particle->getTrack();
      if (track) {

        ECLEnergyCloseToTrack* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

        if (eclinfo) {
          return eclinfo->getEnergy3FWDBarrel();
        } else {
          B2WARNING("Relation to ECLEnergyCloseToTrack not found, did you forget to run ECLTrackCalDigitMatchModule?");
          return std::numeric_limits<float>::quiet_NaN();
        }
      }

      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclEnergy3FWDEndcap(const Particle* particle)
    {

      const Track* track = particle->getTrack();
      if (track) {

        ECLEnergyCloseToTrack* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

        if (eclinfo) {
          return eclinfo->getEnergy3FWDEndcap();
        } else {
          B2WARNING("Relation to ECLEnergyCloseToTrack not found, did you forget to run ECLTrackCalDigitMatchModule?");
          return std::numeric_limits<float>::quiet_NaN();
        }
      }

      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclEnergy3BWDEndcap(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (track) {

        ECLEnergyCloseToTrack* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

        if (eclinfo) {
          return eclinfo->getEnergy3BWDEndcap();
        } else {
          B2WARNING("Relation to ECLEnergyCloseToTrack not found, did you forget to run ECLTrackCalDigitMatchModule?");
          return std::numeric_limits<float>::quiet_NaN();
        }
      }

      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclEnergy3BWDBarrel(const Particle* particle)
    {

      const Track* track = particle->getTrack();
      if (track) {

        ECLEnergyCloseToTrack* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

        if (eclinfo) {
          return eclinfo->getEnergy3BWDBarrel();
        } else {
          B2WARNING("Relation to ECLEnergyCloseToTrack not found, did you forget to run ECLTrackCalDigitMatchModule?");
          return std::numeric_limits<float>::quiet_NaN();
        }
      }

      return std::numeric_limits<float>::quiet_NaN();
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
      return (double) elci->getNECLShowersRejected();
    }

    double eclClusterEoP(const Particle* part)
    {
      const double E = eclClusterE(part);
      const double p =  part->getMomentumMagnitude();
      if (0 == p) { return std::numeric_limits<float>::quiet_NaN();}
      return E / p;
    }

    double getEnergyTC(const Particle*, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (tcid).");
      }

      StoreObjPtr<ECLTRGInformation> tce;
      const int tcid = int(std::lround(vars[0]));

      if (!tce) return std::numeric_limits<double>::quiet_NaN();
      return tce->getEnergyTC(tcid);
    }

    double getTimingTC(const Particle*, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (tcid).");
      }

      StoreObjPtr<ECLTRGInformation> tce;
      const int tcid = int(std::lround(vars[0]));

      if (!tce) return std::numeric_limits<double>::quiet_NaN();
      return tce->getTimingTC(tcid);
    }

    double eclHitWindowTC(const Particle*, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (tcid).");
      }

      StoreObjPtr<ECLTRGInformation> tce;
      const int tcid = int(std::lround(vars[0]));

      if (!tce) return std::numeric_limits<double>::quiet_NaN();
      return tce->getHitWinTC(tcid);
    }

    double getEvtTimingTC(const Particle*)
    {
      StoreObjPtr<ECLTRGInformation> tce;
      if (!tce) return std::numeric_limits<double>::quiet_NaN();
      return tce->getEvtTiming();
    }

    double getMaximumTCId(const Particle*)
    {
      StoreObjPtr<ECLTRGInformation> tce;
      if (!tce) return std::numeric_limits<double>::quiet_NaN();
      return tce->getMaximumTCId();
    }


    double getNumberOfTCs(const Particle*, const std::vector<double>& vars)
    {
      if (vars.size() != 3) {
        B2FATAL("Need exactly three parameters (fadccut, minthetaid, maxthetaid).");
      }

      StoreArray<ECLTriggerCell> ecltcs;
      const int fadccut = int(std::lround(vars[0]));

      if (!ecltcs) return std::numeric_limits<double>::quiet_NaN();
      if (fadccut == 0) return ecltcs.getEntries();
      else {
        int minTheta = int(std::lround(vars[1]));
        int maxTheta = int(std::lround(vars[2]));

        unsigned nTCs = 0;
        for (const auto tc : ecltcs) {
          if (tc.getFADC() >= fadccut and
              tc.getThetaId() >= minTheta and
              tc.getThetaId() <= maxTheta) nTCs++;
        }
        return nTCs;
      }
      return 0.0;
    }

    double getEnergyTCECLCalDigit(const Particle*, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameter (tcid).");
      }

      StoreObjPtr<ECLTRGInformation> tce;
      const int tcid = int(std::lround(vars[0]));

      if (!tce) return std::numeric_limits<double>::quiet_NaN();
      return tce->getEnergyTCECLCalDigit(tcid);
    }

    double getTimingTCECLCalDigit(const Particle*, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameter (tcid).");
      }

      StoreObjPtr<ECLTRGInformation> tce;
      const int tcid = int(std::lround(vars[0]));

      if (!tce) return std::numeric_limits<double>::quiet_NaN();
      return tce->getTimingTCECLCalDigit(tcid);
    }

    double eclEnergySumTC(const Particle*, const std::vector<double>& vars)
    {
      if (vars.size() != 3) {
        B2FATAL("Need exactly three parameters (fadccut, minthetaid, maxthetaid).");
      }

      StoreObjPtr<ECLTRGInformation> tce;
      if (!tce) return std::numeric_limits<double>::quiet_NaN();

      const int fadccut = int(std::lround(vars[0]));
      const int minTheta = int(std::lround(vars[1]));
      const int maxTheta = int(std::lround(vars[2]));

      if (maxTheta < minTheta) {
        B2WARNING("minTheta i (vars[1]) must be equal or less than maxTheta j (vars[2]).");
        return std::numeric_limits<double>::quiet_NaN();
      }

      double energySum = 0.;
      for (unsigned idx = 1; idx <= 576; idx++) {
        if (tce->getThetaIdTC(idx) >= minTheta and tce->getThetaIdTC(idx) <= maxTheta and tce->getEnergyTC(idx) >= fadccut) {
          energySum += tce->getEnergyTC(idx);
        }
      }

      return energySum;
    }

    double eclEnergySumTCECLCalDigit(const Particle*, const std::vector<double>& vars)
    {
      if (vars.size() != 3) {
        B2FATAL("Need exactly three parameters (minthetaid, maxthetaid, option).");
      }

      StoreObjPtr<ECLTRGInformation> tce;
      if (!tce) return std::numeric_limits<double>::quiet_NaN();

      int minTheta = int(std::lround(vars[0]));
      int maxTheta = int(std::lround(vars[1]));
      int option = int(std::lround(vars[2]));
      double par = vars[3];

      if (maxTheta < minTheta) {
        B2WARNING("minTheta i (vars[0]) must be equal or less than maxTheta j (vars[1]).");
        return std::numeric_limits<double>::quiet_NaN();
      }
      if (option < 0 or option > 2) {
        B2WARNING("Third parameters k (vars[2]) must be >=0 and <=2.");
        return std::numeric_limits<double>::quiet_NaN();
      }

      double energySum = 0.;
      for (unsigned idx = 1; idx <= 576; idx++) {
        if (tce->getThetaIdTC(idx) >= minTheta and
            tce->getThetaIdTC(idx) <= maxTheta) {

          if (option == 0) {
            energySum += tce->getEnergyTCECLCalDigit(idx);
          } else if (option == 1 and tce->getEnergyTC(idx)) {
            energySum += tce->getEnergyTCECLCalDigit(idx);
          } else if (option == 2 and tce->getEnergyTCECLCalDigit(idx) > par) { // TCECLCalDigits > par
            energySum += tce->getEnergyTCECLCalDigit(idx);
          }
        }
      }

      return energySum;
    }

    double eclEnergySumTCECLCalDigitInECLCluster(const Particle*)
    {
      StoreObjPtr<ECLTRGInformation> tce;
      if (!tce) return std::numeric_limits<double>::quiet_NaN();
      return tce->getSumEnergyTCECLCalDigitInECLCluster();
    }

    double eclEnergySumECLCalDigitInECLCluster(const Particle*)
    {
      StoreObjPtr<ECLTRGInformation> tce;
      if (!tce) return std::numeric_limits<double>::quiet_NaN();
      return tce->getSumEnergyECLCalDigitInECLCluster();
    }

    double eclEnergySumTCECLCalDigitInECLClusterThreshold(const Particle*)
    {
      StoreObjPtr<ECLTRGInformation> tce;
      if (!tce) return std::numeric_limits<double>::quiet_NaN();
      return tce->getClusterEnergyThreshold();
    }

    double eclNumberOfTCsForCluster(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 4) {
        B2FATAL("Need exactly two parameters (minthetaid, maxthetaid, minhitwindow, maxhitwindow).");
      }

      // if we did not run the ECLTRGInformation module, return NaN
      StoreArray<ECLTriggerCell> ecltc;
      if (!ecltc) return std::numeric_limits<double>::quiet_NaN();

      // if theta range makes no sense, return NaN
      const int minTheta = int(std::lround(vars[0]));
      const int maxTheta = int(std::lround(vars[1]));
      if (maxTheta < minTheta) {
        B2WARNING("minTheta i (vars[0]) must be equal or less than maxTheta j (vars[1]).");
        return std::numeric_limits<double>::quiet_NaN();
      }
      // if hitwin range makes no sense, return NaN
      const int minHitWin = int(std::lround(vars[2]));
      const int maxHitWin = int(std::lround(vars[3]));
      if (maxHitWin < minHitWin) {
        B2WARNING("minHitWin k (vars[2]) must be equal or less than maxHitWin l (vars[3]).");
        return std::numeric_limits<double>::quiet_NaN();
      }

      double result = 0.;
      const ECLCluster* cluster = particle->getECLCluster();

      // if everything else is fine, but we dont have a cluster, return 0
      if (cluster) {
        auto relationsTCs = cluster->getRelationsWith<ECLTriggerCell>();
        for (unsigned int idxTC = 0; idxTC < relationsTCs.size(); ++idxTC) {
          const auto tc = relationsTCs.object(idxTC);
          if (tc->getThetaId() >= minTheta and tc->getThetaId() <= maxTheta
              and tc->getHitWin() >= minHitWin and tc->getHitWin() <= maxHitWin) result += 1.0;
        }
      }
      return result;
    }

    double eclTCFADCForCluster(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 4) {
        B2FATAL("Need exactly four parameters (minthetaid, maxthetaid, minhitwindow, maxhitwindow).");
      }

      // if we did not run the ECLTRGInformation module, return NaN
      StoreArray<ECLTriggerCell> ecltc;
      if (!ecltc) return std::numeric_limits<double>::quiet_NaN();

      // if theta range makes no sense, return NaN
      const int minTheta = int(std::lround(vars[0]));
      const int maxTheta = int(std::lround(vars[1]));
      if (maxTheta < minTheta) {
        B2WARNING("minTheta i (vars[0]) must be equal or less than maxTheta j (vars[1]).");
        return std::numeric_limits<double>::quiet_NaN();
      }

      // if hitwin range makes no sense, return NaN
      const int minHitWin = int(std::lround(vars[2]));
      const int maxHitWin = int(std::lround(vars[3]));
      if (maxHitWin < minHitWin) {
        B2WARNING("minHitWin k (vars[2]) must be equal or less than maxHitWin l (vars[3]).");
        return std::numeric_limits<double>::quiet_NaN();
      }

      double result = 0.;
      const ECLCluster* cluster = particle->getECLCluster();

      // if everything else is fine, but we dont have a cluster, return 0
      if (cluster) {
        auto relationsTCs = cluster->getRelationsTo<ECLTriggerCell>();
        for (unsigned int idxTC = 0; idxTC < relationsTCs.size(); ++idxTC) {
          const auto tc = relationsTCs.object(idxTC);
          if (tc->getThetaId() >= minTheta and tc->getThetaId() <= maxTheta
              and tc->getHitWin() >= minHitWin and tc->getHitWin() <= maxHitWin) result += tc->getFADC();
        }
      }
      return result;
    }

    double eclTCIsMaximumForCluster(const Particle* particle)
    {

      // if we did not run the ECLTRGInformation module, return NaN
      StoreArray<ECLTriggerCell> ecltc;
      if (!ecltc) return std::numeric_limits<double>::quiet_NaN();

      double result = 0.;
      const ECLCluster* cluster = particle->getECLCluster();

      // if everything else is fine, but we dont have a cluster, return 0
      if (cluster) {
        auto relationsTCs = cluster->getRelationsTo<ECLTriggerCell>();
        for (unsigned int idxTC = 0; idxTC < relationsTCs.size(); ++idxTC) {
          const auto tc = relationsTCs.object(idxTC);
          if (tc->isHighestFADC()) result = 1.0;
        }
      }
      return result;
    }


    VARIABLE_GROUP("ECL Cluster related");
    REGISTER_VARIABLE("clusterEoP", eclClusterEoP, "uncorrelated E over P, a convenience alias for ( clusterE / p )");
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
    REGISTER_VARIABLE("clusterE", eclClusterE, "Returns ECL cluster's energy corrected for leakage and background.");
    REGISTER_VARIABLE("clusterErrorE", eclClusterErrorE,
                      "Returns ECL cluster's uncertainty on energy (from background level and energy dependent tabulation).");
    REGISTER_VARIABLE("clusterErrorPhi", eclClusterErrorPhi,
                      "Returns ECL cluster's uncertainty on phi (from background level and energy dependent tabulation).");
    REGISTER_VARIABLE("clusterErrorTheta", eclClusterErrorTheta,
                      "Returns ECL cluster's uncertainty on theta (from background level and energy dependent tabulation).");

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
    REGISTER_VARIABLE("clusterNHits", eclClusterNHits,
                      "Returns sum of crystal weights sum(w_i) with w_i<=1  associated to this cluster. for non-overlapping clusters this is equal to the number of crystals in the cluster.");
    REGISTER_VARIABLE("clusterTrackMatch", eclClusterTrackMatched,
                      "Returns 1.0 if at least one charged track is matched to this ECL cluster.");
    REGISTER_VARIABLE("nECLClusterTrackMatches", nECLClusterTrackMatches,
                      "Return the number of charged tracks matched to this cluster. "
                      "Note that sometimes (perfectly correctly) two tracks are extrapolated "
                      "into the same cluster so for charged particles, this should return at "
                      "least 1 (but sometimes 2 or more). For neutrals, this should always "
                      "return zero. Returns NAN if there is no cluster.");
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

    // These variables require cDST inputs and the eclTrackCalDigitMatch module run first
    VARIABLE_GROUP("ECL calibration");

    REGISTER_VARIABLE("clusterUncorrE", eclClusterUncorrectedE,
                      "[Expert] [Calibration] Returns ECL cluster's uncorrected energy. That is, before leakage corrections. This variable should only be used for study of the ECL. Please see clusterE.");
    REGISTER_VARIABLE("eclEnergy3FWDBarrel", eclEnergy3FWDBarrel, "[Calibration] Returns energy sum of three crystals in FWD barrel");
    REGISTER_VARIABLE("eclEnergy3FWDEndcap", eclEnergy3FWDEndcap, "[Calibration] Returns energy sum of three crystals in FWD endcap");
    REGISTER_VARIABLE("eclEnergy3BWDBarrel", eclEnergy3BWDBarrel, "[Calibration] Returns energy sum of three crystals in BWD barrel");
    REGISTER_VARIABLE("eclEnergy3BWDEndcap", eclEnergy3BWDEndcap, "[Calibration] Returns energy sum of three crystals in BWD endcap");

    // These variables require cDST inputs and the eclTRGInformation module run first
    VARIABLE_GROUP("ECL trigger calibration");
    REGISTER_VARIABLE("clusterNumberOfTCs(i, j, k, l)", eclNumberOfTCsForCluster,
                      "[Calibration] return the number of TCs for this ECLCluster for a given TC theta Id range (i, j) and hit window (k, l)");
    REGISTER_VARIABLE("clusterTCFADC(i, j, k, l)", eclTCFADCForCluster,
                      "[Calibration] return the total FADC sum related to this ECLCluster for a given TC theta Id range (i, j) and hit window (k, l)");
    REGISTER_VARIABLE("clusterTCIsMaximum", eclTCIsMaximumForCluster,
                      "[Calibration] return true if cluster is related to maximum TC");

    REGISTER_VARIABLE("eclEnergyTC(i)", getEnergyTC,
                      "[Eventbased][Calibration] return the energy (in FADC counts) for the i-th trigger cell (TC), 1 based (1..576)");
    REGISTER_VARIABLE("eclEnergyTCECLCalDigit(i)", getEnergyTCECLCalDigit,
                      "[Eventbased][Calibration] return the energy (in GeV) for the i-th trigger cell (TC) based on ECLCalDigits, 1 based (1..576)");
    REGISTER_VARIABLE("eclTimingTC(i)", getTimingTC,
                      "[Eventbased][Calibration] return the time (in ns) for the i-th trigger cell (TC), 1 based (1..576)");
    REGISTER_VARIABLE("eclHitWindowTC(i)", eclHitWindowTC,
                      "[Eventbased][Calibration] return the hit window for the i-th trigger cell (TC), 1 based (1..576)");
    REGISTER_VARIABLE("eclEventTimingTC", getEvtTimingTC,
                      "[Eventbased][Calibration] return the ECL TC event time (in ns)");
    REGISTER_VARIABLE("eclMaximumTCId", getMaximumTCId,
                      "[Eventbased][Calibration] return the TC Id with maximum FADC value");


    REGISTER_VARIABLE("eclTimingTCECLCalDigit(i)", getTimingTCECLCalDigit,
                      "[Eventbased][Calibration] return the time (in ns) for the i-th trigger cell (TC) based on ECLCalDigits, 1 based (1..576)");

    REGISTER_VARIABLE("eclNumberOfTCs(i, j, k)", getNumberOfTCs,
                      "[Eventbased][Calibration] return the number of TCs above threshold (i=FADC counts) for this event for a given theta range (j-k)");
    REGISTER_VARIABLE("eclEnergySumTC(i, j)", eclEnergySumTC,
                      "[Eventbased][Calibration] return the energy sum (in FADC counts) of all TC cells between two theta ids i<=thetaid<=j, 1 based (1..17)");
    REGISTER_VARIABLE("eclEnergySumTCECLCalDigit(i, j, k, l)", eclEnergySumTCECLCalDigit,
                      "[Eventbased][Calibration] return the energy sum (in GeV) of all TC cells between two theta ids  i<=thetaid<=j, 1 based (1..17). k is the sum option: 0 (all), 1 (those with actual TC entries), 2 (sum of ECLCalDigit energy in this TC above threshold). l is the threshold parameter for the option k 2.");
    REGISTER_VARIABLE("eclEnergySumTCECLCalDigitInECLCluster", eclEnergySumTCECLCalDigitInECLCluster,
                      "[Eventbased][Calibration] return the energy sum (in GeV) of all ECLCalDigits if TC is above threshold that are part of an ECLCluster above eclEnergySumTCECLCalDigitInECLClusterThreshold within TC thetaid 2-15");
    REGISTER_VARIABLE("eclEnergySumECLCalDigitInECLCluster", eclEnergySumECLCalDigitInECLCluster,
                      "[Eventbased][Calibration] return the energy sum (in GeV) of all ECLCalDigits that are part of an ECLCluster above eclEnergySumTCECLCalDigitInECLClusterThreshold within TC thetaid 2-15");
    REGISTER_VARIABLE("eclEnergySumTCECLCalDigitInECLClusterThreshold", eclEnergySumTCECLCalDigitInECLClusterThreshold,
                      "[Eventbased][Calibration] return threshold used to calculate eclEnergySumTCECLCalDigitInECLCluster");

  }
}











