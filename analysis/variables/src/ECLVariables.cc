/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *               Alon Hershenhorn                                         *
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
#include <analysis/utility/ReferenceFrame.h>
#include <analysis/ClusterUtility/ClusterUtils.h>

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

    double eclPulseShapeDiscriminationMVA(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        if (eclClusterHasPulseShapeDiscrimination(particle)) {
          return cluster->getPulseShapeDiscriminationMVA();
        } else {
          return -1.0;
        }
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
      double energy = eclClusterE(particle);
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
      const auto& frame = ReferenceFrame::GetCurrent();

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        ClusterUtils clutls;
        TLorentzVector p4Cluster = clutls.Get4MomentumFromCluster(cluster, particle->getECLClusterEHypothesisBit());

        return frame.getMomentum(p4Cluster).E();
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

    double eclClusterCellId(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getMaxECellId();
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
      const auto& frame = ReferenceFrame::GetCurrent();

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        ClusterUtils clutls;
        TLorentzVector p4Cluster = clutls.Get4MomentumFromCluster(cluster, particle->getECLClusterEHypothesisBit());

        return frame.getMomentum(p4Cluster).Theta();
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
      const auto& frame = ReferenceFrame::GetCurrent();

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        ClusterUtils clutls;
        TLorentzVector p4Cluster = clutls.Get4MomentumFromCluster(cluster, particle->getECLClusterEHypothesisBit());

        return frame.getMomentum(p4Cluster).Phi();
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
        const Track* track = cluster->getRelatedFrom<Track>();

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
      // Hypothesis ID is deprecated, this function should be removed in release-05.
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        if (cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)
            and cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron))
          return 56.0;
        else if (cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
          return 5.0;
        else if (cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron))
          return 6.0;
        else
          return -1.0;
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterHasNPhotonsHypothesis(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double eclClusterHasNeutralHadronHypothesis(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron);
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

        auto* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

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

        auto* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

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

        auto* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

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

        auto* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

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

        auto* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

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

        auto* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

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

        auto* eclinfo = track->getRelatedTo<ECLEnergyCloseToTrack>();

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
      int nDaughters = particle->getNDaughters();
      if (nDaughters < 1) {
        B2WARNING("The provided particle has no daughters!");
        return std::numeric_limits<float>::quiet_NaN();
      }

      double numer = 0, denom = 0;
      double time, deltatime;
      int numberOfClusterDaughters = 0;

      /*
                                      ** TODO !!! **
       Use Martin Ritter's 1337 Particle::forEachDaughter once pull-request #2119 is merged.
      */
      std::stack<const Particle*> stacked;
      stacked.push(particle);
      while (!stacked.empty()) {
        const Particle* current = stacked.top();
        stacked.pop();

        const ECLCluster* cluster = current->getECLCluster();
        if (cluster) {
          numberOfClusterDaughters ++;

          time = cluster->getTime();
          B2DEBUG(10, "time[" << numberOfClusterDaughters << "] = " << time);
          deltatime = cluster->getDeltaTime99();
          B2DEBUG(10, "deltatime[" << numberOfClusterDaughters << "] = " << deltatime);
          numer += time / pow(deltatime, 2);
          B2DEBUG(11, "numer[" << numberOfClusterDaughters << "] = " << numer);
          denom += 1 / pow(deltatime, 2);
          B2DEBUG(11, "denom[" << numberOfClusterDaughters << "] = " << denom);
        } else {
          const std::vector<Particle*> daughters = current->getDaughters();
          nDaughters = current->getNDaughters();
          for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
            stacked.push(daughters[iDaughter]);
          }
        }
      }
      if (numberOfClusterDaughters < 1) {
        B2WARNING("There are no clusters or cluster matches amongst the daughters of the provided particle!");
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
      int nDaughters = particle->getNDaughters();
      if (nDaughters < 1) {
        B2WARNING("The provided particle has no daughters!");
        return std::numeric_limits<float>::quiet_NaN();
      }

      double averageECLTime, maxTimeDiff = -1;
      double time, deltatime, maxTimeDiff_temp;
      int numberOfClusterDaughters = 0;

      averageECLTime = weightedAverageECLTime(particle);

      std::stack<const Particle*> stacked;
      stacked.push(particle);
      while (!stacked.empty()) {
        const Particle* current = stacked.top();
        stacked.pop();

        const ECLCluster* cluster = current->getECLCluster();
        if (cluster) {
          numberOfClusterDaughters ++;

          time = cluster->getTime();
          B2DEBUG(10, "time[" << numberOfClusterDaughters << "] = " << time);
          deltatime = cluster->getDeltaTime99();
          B2DEBUG(10, "deltatime[" << numberOfClusterDaughters << "] = " << deltatime);
          maxTimeDiff_temp = fabs((time - averageECLTime) / deltatime);
          B2DEBUG(11, "maxTimeDiff_temp[" << numberOfClusterDaughters << "] = " << maxTimeDiff_temp);
          if (maxTimeDiff_temp > maxTimeDiff)
            maxTimeDiff = maxTimeDiff_temp;
          B2DEBUG(11, "maxTimeDiff[" << numberOfClusterDaughters << "] = " << maxTimeDiff);
        } else {
          const std::vector<Particle*> daughters = current->getDaughters();
          nDaughters = current->getNDaughters();
          for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
            stacked.push(daughters[iDaughter]);
          }
        }
      }
      if (numberOfClusterDaughters < 1) {
        B2WARNING("There are no clusters or cluster matches amongst the daughters of the provided particle!");
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

    double eclClusterMdstIndex(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getArrayIndex();
      } else return std::numeric_limits<double>::quiet_NaN();

      return std::numeric_limits<double>::quiet_NaN();
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
        for (const auto& tc : ecltcs) {
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

    double eclClusterOnlyInvariantMass(const Particle* part)
    {
      int nDaughters = part->getNDaughters();
      TLorentzVector sum;

      if (nDaughters < 1) {
        return part->getMass();
      } else {
        int nClusterDaughters = 0;
        std::stack<const Particle*> stacked;
        stacked.push(part);
        while (!stacked.empty()) {
          const Particle* current = stacked.top();
          stacked.pop();

          const ECLCluster* cluster = current->getECLCluster();
          if (cluster) {
            const ECLCluster::EHypothesisBit clusterBit = current->getECLClusterEHypothesisBit();
            nClusterDaughters ++;
            ClusterUtils clutls;
            TLorentzVector p4Cluster = clutls.Get4MomentumFromCluster(cluster, clusterBit);
            sum += p4Cluster;
          } else {
            const std::vector<Particle*> daughters = current->getDaughters();
            nDaughters = current->getNDaughters();
            for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
              stacked.push(daughters[iDaughter]);
            }
          }
        }

        if (nClusterDaughters < 1) {
          B2WARNING("There are no clusters amongst the daughters of the provided particle!");
          return std::numeric_limits<double>::quiet_NaN();
        }
        B2DEBUG(10, "Number of daughters with cluster associated = " << nClusterDaughters);
        return sum.M();
      }
    }


    VARIABLE_GROUP("ECL Cluster related");
    REGISTER_VARIABLE("clusterEoP", eclClusterEoP, R"DOC(
Returns ratio of uncorrelated energy E over momentum p, a convenience
alias for (clusterE / p).
)DOC");
    REGISTER_VARIABLE("clusterReg", eclClusterDetectionRegion, R"DOC(
Returns an integer code for the ECL region of a cluster.

    - 1: forward, 2: barrel, 3: backward,
    - 11: between FWD and barrel, 13: between BWD and barrel,
    - 0: otherwise
)DOC");
    REGISTER_VARIABLE("clusterDeltaLTemp", eclClusterDeltaL, R"DOC(
| Returns DeltaL for the shower shape.
| A cluster comprises the energy depositions of several crystals. All these crystals have slightly
  different orientations in space. A shower direction can be constructed by calculating the weighted
  average of these orientations using the corresponding energy depositions as weights. The intersection
  (more precisely the point of closest approach) of the vector with this direction originating from the
  cluster center and an extrapolated track can be used as reference for the calculation of the shower
  depth. It is defined as the distance between this intersection and the cluster center.

.. warning::
    This distance is calculated on the reconstructed level and is temporarily
    included to the ECL cluster MDST data format for studying purposes. If it is found
    that it is not crucial for physics analysis then this variable will be removed
    in future releases.
    Therefore, keep in mind that this variable might be removed in the future!

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`-250.0`
    | Upper limit: :math:`250.0`
    | Precision: :math:`10` bit
)DOC");
    REGISTER_VARIABLE("minC2TDist", eclClusterIsolation, R"DOC(
Returns distance between ECL cluster and nearest track hitting the ECL.

A cluster comprises the energy depositions of several crystals. All these crystals have slightly
different orientations in space. A shower direction can be constructed by calculating the weighted
average of these orientations using the corresponding energy depositions as weights. The intersection
(more precisely the point of closest approach) of the vector with this direction originating from the
cluster center and an extrapolated track can be used as reference for the calculation of the track depth.
It is defined as the distance between this intersection and the track hit position on the front face of the ECL.

.. note::
    This distance is calculated on the reconstructed level.

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`0.0`
    | Upper limit: :math:`250.0`
    | Precision: :math:`10` bit
)DOC");
    REGISTER_VARIABLE("clusterE", eclClusterE, R"DOC(
Returns ECL cluster's energy corrected for leakage and background.

The raw photon energy is given by the weighted sum of all ECL crystal energies within the ECL cluster. 
The weights per crystals are :math:`\leq 1` after cluster energy splitting in the case of overlapping 
clusters. The number of crystals that are included in the sum depends on a initial energy estimation 
and local beam background levels at the highest energy crystal position. It is optimized to minimize 
the core width (resolution) of true photons. Photon energy distributions always show a low energy tail 
due to unavoidable longitudinal and transverse leakage that can be further modified by the clustering
algorithm and beam backgrounds.The peak position of the photon energy distributions are corrected to
match the true photon energy in MC:

    - Leakage correction: Using large MC samples of mono-energetic single photons, a correction factor
      :math:`f` as function of reconstructed detector position, reconstructed photon energy and beam backgrounds
      is determined via :math:`f = \frac{\text{peak_reconstructed}}{\text{energy_true}}`.

    - Cluster energy calibration (data only): To reach the target precision of :math:`< 1.8\%` energy
      resolution for high energetic photons, the remaining difference between MC and data must be calibrated
      using kinematically fit muon pairs. This calibration is only applied to data and not to MC and will
      take time to develop.

It is important to note that after perfect leakage correction and cluster energy calibration,
the :math:`\pi^{0}` mass peak will be shifted slightly to smaller values than the PDG average
due to the low energy tails of photons. The :math:`\pi^{0}` mass peak must not be corrected
to the PDG value by adjusting the reconstructed photon energies. Selection criteria based on
the mass for :math:`\pi^{0}` candidates must be based on the biased value. Most analysis
will used mass constrained :math:`\pi^{0}` s anyhow.

.. warning::
    We only store clusters with :math:`E > 20\,` MeV.

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`-5` (:math:`e^{-5} = 0.00674\,` GeV)
    | Upper limit: :math:`3.0` (:math:`e^3 = 20.08553\,` GeV)
    | Precision: :math:`18` bit
    | This value can be changed to a different reference frame with :b2:var:`useCMSFrame`.
)DOC");
    REGISTER_VARIABLE("clusterErrorE", eclClusterErrorE, R"DOC(
Returns ECL cluster's uncertainty on energy
(from background level and energy dependent tabulation).
)DOC");
    REGISTER_VARIABLE("clusterErrorPhi", eclClusterErrorPhi, R"DOC(
Returns ECL cluster's uncertainty on :math:`\phi`
(from background level and energy dependent tabulation).
)DOC");
    REGISTER_VARIABLE("clusterErrorTheta", eclClusterErrorTheta, R"DOC(
Returns ECL cluster's uncertainty on :math:`\theta`
(from background level and energy dependent tabulation).
)DOC");

    REGISTER_VARIABLE("clusterR", eclClusterR, R"DOC(
Returns ECL cluster's centroid distance from :math:`(0,0,0)`.
)DOC");
    REGISTER_VARIABLE("clusterPhi", eclClusterPhi, R"DOC(
Returns ECL cluster's azimuthal angle :math:`\phi`
(this is not generally equal to a photon azimuthal angle).

| The direction of a cluster is given by the connecting line of :math:`\,(0,0,0)\,` and
  cluster centroid position in the ECL.
| The cluster centroid position is calculated using up to 21 crystals (5x5 excluding corners)
  after cluster energy splitting in the case of overlapping clusters.
| The centroid position is the logarithmically weighted average of all crystals evaluated at
  the crystal centers. Cluster centroids are generally biased towards the centers of the
  highest energetic crystal. This effect is larger for low energetic photons.
| Beam backgrounds slightly decrease the position resolution, mainly for low energetic photons.

.. note::
    Radius of a cluster is almost constant in the barrel and should not be used directly in any selection.

Unlike for charged tracks, the uncertainty (covariance) of the photon directions is not determined
based on individual cluster properties but taken from on MC-based parametrizations of the resolution
as function of true photon energy, true photon direction and beam background level.

.. warning::
    Users must use the actual particle direction (done automatically in the modularAnalysis using the average
    IP position (can be changed if needed)) and not the ECL Cluster direction (position in the ECL measured
    from :math:`(0,0,0)`) for particle kinematics.

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`-\pi`
    | Upper limit: :math:`\pi`
    | Precision: :math:`16` bit
)DOC");
    REGISTER_VARIABLE("clusterConnectedRegionID", eclClusterConnectedRegionID, R"DOC(
Returns ECL cluster's connected region ID.
)DOC");
    REGISTER_VARIABLE("clusterTheta", eclClusterTheta, R"DOC(
Returns ECL cluster's polar angle :math:`\theta`
(this is not generally equal to a photon polar angle).

| The direction of a cluster is given by the connecting line of :math:`\,(0,0,0)\,` and
  cluster centroid position in the ECL.
| The cluster centroid position is calculated using up to 21 crystals (5x5 excluding corners)
  after cluster energy splitting in the case of overlapping clusters.
| The centroid position is the logarithmically weighted average of all crystals evaluated at
  the crystal centers. Cluster centroids are generally biased towards the centers of the
  highest energetic crystal. This effect is larger for low energetic photons.
| Beam backgrounds slightly decrease the position resolution, mainly for low energetic photons.

.. note::
    Radius of a cluster is almost constant in the barrel and should not be used directly in any selection.

Unlike for charged tracks, the uncertainty (covariance) of the photon directions is not determined
based on individual cluster properties but taken from on MC-based parametrizations of the resolution
as function of true photon energy, true photon direction and beam background level.

.. warning::
    Users must use the actual particle direction (done automatically in the modularAnalysis using the average
    IP position (can be changed if needed)) and not the ECL Cluster direction (position in the ECL measured
    from :math:`(0,0,0)`) for particle kinematics.

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`0.0`
    | Upper limit: :math:`\pi`
    | Precision: :math:`16` bit
)DOC");
    REGISTER_VARIABLE("clusterTiming", eclClusterTiming, R"DOC(
Returns ECL cluster's timing. Photon timing is given by the fitted time
of the recorded waveform of the highest energetic crystal in a cluster.
After all corrections (including Time-Of-Flight) and calibrations, photons from the interaction point (IP)
should have a time that corresponds to the event trigger time :math:`t_{0}`
(For MC, this is currently not simulated and such photons are designed to have a time of :math:`t = 0\,` nano second).

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`-1000.0`
    | Upper limit: :math:`1000.0`
    | Precision: :math:`12` bit
)DOC");
    REGISTER_VARIABLE("clusterErrorTiming", eclClusterErrorTiming, R"DOC(
Returns ECL cluster's timing uncertainty that contains :math:`99\%` of true photons (dt99).

The photon timing uncertainty is currently determined using MC. The resulting parametrization depends on
the true energy deposition in the highest energetic crystal and the local beam background level in that crystal.
The resulting timing distribution is non-Gaussian and for each photon the value dt99 is stored,
where :math:`|\text{timing}| / \text{dt99} < 1` is designed to give a :math:`99\%`
timing efficiency for true photons from the IP.
The resulting efficiency is approximately flat in energy and independent of beam background levels.

Very large values of dt99 are an indication of failed waveform fits in the ECL.
We remove such clusters in most physics photon lists.

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`0.0`
    | Upper limit: :math:`1000.0`
    | Precision: :math:`12` bit

.. warning::
    In real data there will be a sizeable number of high energetic Bhabha events
    (from previous or later bunch collisions) that can easily be rejected by timing cuts.
    However, these events create large ECL clusters that can overlap with other ECL clusters
    and it is not clear that a simple rejection is the correction strategy.
)DOC");
    REGISTER_VARIABLE("clusterHighestE", eclClusterHighestE, R"DOC(
Returns energy of the highest energetic crystal in the ECL cluster after reweighting.

.. warning::
    This variable must be used carefully since it can bias shower selection
    towards photons that hit crystals in the center and hence have a large energy
    deposition in the highest energy crystal.

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`-5` (:math:`e^{-5} = 0.00674\,` GeV)
    | Upper limit: :math:`3.0` (:math:`e^3 = 20.08553\,` GeV)
    | Precision: :math:`18` bit
)DOC");
    REGISTER_VARIABLE("clusterCellID", eclClusterCellId,
                      "Returns cellId of the crystal with highest energy in the ECLCluster.");
    REGISTER_VARIABLE("clusterE1E9", eclClusterE1E9, R"DOC(
Returns ratio of energies of the central crystal, E1, and 3x3 crystals, E9, around the central crystal.
Since :math:`E1 \leq E9`, this ratio is :math:`\leq 1` and tends towards larger values for photons
and smaller values for hadrons.

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`0.0`
    | Upper limit: :math:`1.0`
    | Precision: :math:`10` bit
)DOC");
    REGISTER_VARIABLE("clusterE9E25", eclClusterE9E25, R"DOC(
Deprecated - kept for backwards compatibility - returns clusterE9E21.
)DOC");
    REGISTER_VARIABLE("clusterE9E21", eclClusterE9E21, R"DOC(
Returns ratio of energies in inner 3x3 crystals, E9, and 5x5 crystals around the central crystal without corners.
Since :math:`E9 \leq E21`, this ratio is :math:`\leq 1` and tends towards larger values for photons
and smaller values for hadrons.

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`0.0`
    | Upper limit: :math:`1.0`
    | Precision: :math:`10` bit
)DOC");
    REGISTER_VARIABLE("clusterAbsZernikeMoment40", eclClusterAbsZernikeMoment40, R"DOC(
Returns absolute value of Zernike moment 40 (:math:`|Z_{40}|`). (shower shape variable).

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`0.0`
    | Upper limit: :math:`1.7`
    | Precision: :math:`10` bit
)DOC");
    REGISTER_VARIABLE("clusterAbsZernikeMoment51", eclClusterAbsZernikeMoment51, R"DOC(
Returns absolute value of Zernike moment 51 (:math:`|Z_{51}|`). (shower shape variable).

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`0.0`
    | Upper limit: :math:`1.2`
    | Precision: :math:`10` bit
)DOC");
    REGISTER_VARIABLE("clusterZernikeMVA", eclClusterZernikeMVA, R"DOC(
Returns output of a MVA using eleven Zernike moments of the cluster. Zernike moments are calculated per
shower in a plane perpendicular to the shower direction via

.. math::
    |Z_{nm}| = \frac{n+1}{\pi} \frac{1}{\sum_{i} w_{i} E_{i}} \left|\sum_{i} R_{nm}(\rho_{i}) e^{-im\alpha_{i}} w_{i} E_{i} \right|

where n, m are the integers, :math:`i` runs over the crystals in the shower,
:math:`E_{i}` is the energy of the i-th crystal in the shower,
:math:`R_{nm}` is a polynomial of degree :math:`n`,
:math:`\rho_{i}` is the radial distance of the :math:`i`-th crystal in the perpendicular plane, 
and :math:`\alpha_{i}` is the polar angle of the :math:`i`-th crystal in the perpendicular plane.
As a crystal can be related to more than one shower, :math:`w_{i}` is the fraction of the
energy of the :math:`i`-th crystal associated with the shower.

More details about the implementation can be found in `BELLE2-NOTE-TE-2017-001 <https://docs.belle2.org/record/454?ln=en>`_ .

More details about Zernike polynomials can be found in `Wikipedia <https://en.wikipedia.org/wiki/Zernike_polynomials>`_ .

| For cluster with hypothesisId==N1: raw MVA output.
| For cluster with hypothesisId==N2: 1 - prod{clusterZernikeMVA}, where the product is on all N1 showers
  belonging to the same connected region (shower shape variable).

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`0.0`
    | Upper limit: :math:`1.0`
    | Precision: :math:`10` bit
)DOC");
    REGISTER_VARIABLE("clusterSecondMoment", eclClusterSecondMoment, R"DOC(
Returns second moment :math:`S`. It is defined as:

.. math::
    S = \frac{\sum_{i=0}^{n} w_{i} E_{i} r^2_{i}}{\sum_{i=0}^{n} w_{i} E_{i}}

where :math:`E_{i} = (E_0, E_1, ...)` are the single crystal energies sorted by energy, :math:`w_{i}` is
the crystal weight, and :math:`r_{i}` is the distance of the :math:`i`-th digit to the shower center projected
to a plane perpendicular to the shower axis.

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`0.0`
    | Upper limit: :math:`40.0`
    | Precision: :math:`10` bit
)DOC");
    REGISTER_VARIABLE("clusterLAT", eclClusterLAT, R"DOC(
Returns lateral energy distribution (shower variable). It is defined as following:

.. math::
    S = \frac{\sum_{i=3}^{n} w_{i} E_{i} r^2_{i}}{\sum_{i=3}^{n} w_{i} E_{i} r^2_{i} + w_{0} E_{0} r^2_{0} + w_{1} E_{1} r^2_{0}}

where :math:`E_{i} = (E_0, E_1, ...)` are the single crystal energies sorted by energy, :math:`w_{i}` is
the crystal weight, :math:`r_{i}` is the distance of the :math:`i`-th digit to the shower center projected to
a plane perpendicular to the shower axis, and :math:`r_{0} \approx 5\,cm` is the distance between two crystals.

clusterLAT peaks around 0.3 for radially symmetrical electromagnetic showers and is larger for hadronic events,
and electrons with a close-by radiative or Bremsstrahlung photon.

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`0.0`
    | Upper limit: :math:`1.0`
    | Precision: :math:`10` bit
)DOC");
    REGISTER_VARIABLE("clusterNHits", eclClusterNHits, R"DOC(
Returns sum of weights :math:`w_{i}` (:math:`w_{i} \leq 1`) of all crystals in an ECL cluster.
For non-overlapping clusters this is equal to the number of crystals in the cluster.
In case of energy splitting among nearby clusters, this can be a non-integer value.

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`0.0`
    | Upper limit: :math:`200.0`
    | Precision: :math:`10` bit
    | If fractional weights are not of interest, this value should be cast to the nearest integer. 
)DOC");
    REGISTER_VARIABLE("clusterTrackMatch", eclClusterTrackMatched, R"DOC(
Returns 1.0 if at least one reconstructed charged track is matched to the ECL cluster.

Every reconstructed charged track is extrapolated into the ECL.
Every ECL crystal that is crossed by the track extrapolation is marked.
Each ECL cluster that contains any marked crystal is matched to the track.
Multiple tracks can be matched to one cluster and multiple clusters can be matched to one track.
It is conceptually correct to have two tracks matched to the same cluster.
)DOC");
    REGISTER_VARIABLE("nECLClusterTrackMatches", nECLClusterTrackMatches, R"DOC(
Returns number of charged tracks matched to this cluster.

.. note::
    Sometimes (perfectly correctly) two tracks are extrapolated into the same cluster.

        - For charged particles, this should return at least 1 (but sometimes 2 or more).
        - For neutrals, this should always return 0.
        - Returns NaN if there is no cluster.
)DOC");
    REGISTER_VARIABLE("clusterCRID", eclClusterConnectedRegionId, R"DOC(
| Returns ECL cluster's connected region ID.
| This can be used to find potentially overlapping ECL clusters.
)DOC");
    REGISTER_VARIABLE("clusterHasPulseShapeDiscrimination", eclClusterHasPulseShapeDiscrimination, R"DOC(
Status bit to indicate if cluster has digits with waveforms that passed energy and :math:`\chi^2`
thresholds for computing PSD variables.
)DOC");
    REGISTER_VARIABLE("clusterPulseShapeDiscriminationMVA", eclPulseShapeDiscriminationMVA, R"DOC(
Returns MVA classifier that uses pulse shape discrimination to identify electromagnetic vs hadronic showers.
    
    - 1 for electromagnetic showers
    - 0 for hadronic showers
)DOC");
    REGISTER_VARIABLE("clusterNumberOfHadronDigits", eclClusterNumberOfHadronDigits, R"DOC(
Returns ECL cluster's number of hadron digits in cluster (pulse shape discrimination variable).
Weighted sum of digits in cluster with significant scintillation emission (:math:`> 3\,` MeV)
in the hadronic scintillation component.
Computed only using cluster digits with energy :math:`> 50\,` MeV and good offline waveform fit :math:`\chi^2`.

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`0.0`
    | Upper limit: :math:`255.0`
    | Precision: :math:`18` bit
)DOC");
    REGISTER_VARIABLE("clusterClusterID", eclClusterId, R"DOC(
Returns ECL cluster ID of this ECL cluster within the connected region (CR) to which it belongs to.
)DOC");
    REGISTER_VARIABLE("clusterHypothesis", eclClusterHypothesisId, R"DOC(
Emulates the deprecated hypothesis ID of this ECL cluster in as-backward-compatible way as possible.

Returns 5 for the nPhotons hypothesis, 6 for the neutralHadron hypothesis.
Since release-04-00-00, it will be possible for a cluster to have both hypotheses so if both are set it will return 56.

.. warning::
   This variable is a legacy variable and will be removed in release-05-00-00. 
   You probably want to use :b2:var:`clusterHasNPhotons` and :b2:var:`clusterHasNeutralHadron` instead of this variable.
)DOC");
    REGISTER_VARIABLE("clusterHasNPhotons", eclClusterHasNPhotonsHypothesis, R"DOC(
Returns 1.0 if cluster has the 'N photons' hypothesis (historically called 'N1'),
0.0 if not, and NaN if no cluster is associated to the particle.
)DOC");
    REGISTER_VARIABLE("clusterHasNeutralHadron", eclClusterHasNeutralHadronHypothesis, R"DOC(
Returns 1.0 if the cluster has the 'neutral hadrons' hypothesis (historically called 'N2'),
0.0 if not, and NaN if no cluster is associated to the particle.
)DOC");
    REGISTER_VARIABLE("eclExtTheta", eclExtTheta, R"DOC(
Returns extrapolated :math:`\theta`.
)DOC");
    REGISTER_VARIABLE("eclExtPhi", eclExtPhi, R"DOC(
Returns extrapolated :math:`\phi`.
)DOC");
    REGISTER_VARIABLE("eclExtPhiId", eclExtPhiId, R"DOC(
Returns extrapolated :math:`\phi` ID.
)DOC");
    REGISTER_VARIABLE("weightedAverageECLTime", weightedAverageECLTime, R"DOC(
Returns ECL weighted average time of all clusters (neutrals) and matched clusters (charged) of daughters
(of any generation) of the provided particle.
)DOC");
    REGISTER_VARIABLE("maxWeightedDistanceFromAverageECLTime", maxWeightedDistanceFromAverageECLTime, R"DOC(
Returns maximum weighted distance between time of the cluster of a photon and the ECL average time, amongst
the clusters (neutrals) and matched clusters (charged) of daughters (of all generations) of the provided particle.
)DOC");
    REGISTER_VARIABLE("clusterMdstIndex", eclClusterMdstIndex, R"DOC(
StoreArray index(0 - based) of the MDST ECLCluster (useful for track-based particles matched to a cluster).
)DOC");

    REGISTER_VARIABLE("nECLOutOfTimeCrystals", nECLOutOfTimeCrystals, R"DOC(
[Eventbased] Returns the number of crystals (ECLCalDigits) that are out of time.
)DOC");

    REGISTER_VARIABLE("nECLOutOfTimeCrystalsFWDEndcap", nECLOutOfTimeCrystalsFWDEndcap, R"DOC(
[Eventbased] Returns the number of crystals (ECLCalDigits) that are out of time in the forward endcap.
)DOC");

    REGISTER_VARIABLE("nECLOutOfTimeCrystalsBarrel", nECLOutOfTimeCrystalsBarrel, R"DOC(
[Eventbased] Returns the number of crystals (ECLCalDigits) that are out of time in the barrel.
)DOC");

    REGISTER_VARIABLE("nECLOutOfTimeCrystalsBWDEndcap", nECLOutOfTimeCrystalsBWDEndcap, R"DOC(
[Eventbased] Returns the number of crystals (ECLCalDigits) that are out of time in the backward endcap.
)DOC");

    REGISTER_VARIABLE("nRejectedECLShowers", nRejectedECLShowers, R"DOC(
[Eventbased] Returns the number of showers in the ECL that do not become clusters.
)DOC");

    REGISTER_VARIABLE("nRejectedECLShowersFWDEndcap", nRejectedECLShowersFWDEndcap, R"DOC(
[Eventbased] Returns the number of showers in the ECL that do not become clusters, from the forward endcap.
)DOC");

    REGISTER_VARIABLE("nRejectedECLShowersBarrel", nRejectedECLShowersBarrel, R"DOC(
[Eventbased] Returns the number of showers in the ECL that do not become clusters, from the barrel.
)DOC");

    REGISTER_VARIABLE("nRejectedECLShowersBWDEndcap", nRejectedECLShowersBWDEndcap, R"DOC(
[Eventbased] Returns the number of showers in the ECL that do not become clusters, from the backward endcap.
)DOC");

    REGISTER_VARIABLE("eclClusterOnlyInvariantMass", eclClusterOnlyInvariantMass, R"DOC(
[Expert] The invariant mass calculated from all ECLCluster daughters (i.e. photons) and
cluster-matched tracks using the cluster 4-momenta.

Used for ECL-based dark sector physics and debugging track-cluster matching.
)DOC");

    VARIABLE_GROUP("Belle Variables");
    REGISTER_VARIABLE("goodBelleGamma", goodBelleGamma, R"DOC(
[Legacy] Returns 1.0 if photon candidate passes simple region dependent
energy selection for Belle data and MC (50/100/150 MeV).
)DOC");
    REGISTER_VARIABLE("clusterBelleQuality", eclClusterDeltaL, R"DOC(
[Legacy] Returns ECL cluster's quality indicating a good cluster in GSIM (stored in deltaL of ECL cluster object).
Belle analysis typically used clusters with quality == 0 in their :math:`E_{\text{extra ECL}}` (Belle only).
)DOC");

    // These variables require cDST inputs and the eclTrackCalDigitMatch module run first
    VARIABLE_GROUP("ECL calibration");

    REGISTER_VARIABLE("clusterUncorrE", eclClusterUncorrectedE, R"DOC(
[Expert] [Calibration] Returns ECL cluster's uncorrected energy. That is, before leakage corrections.
This variable should only be used for study of the ECL. Please see :b2:var:`clusterE`.
)DOC");

    REGISTER_VARIABLE("eclEnergy3FWDBarrel", eclEnergy3FWDBarrel, R"DOC(
[Calibration] Returns energy sum of three crystals in forward barrel.
)DOC");

    REGISTER_VARIABLE("eclEnergy3FWDEndcap", eclEnergy3FWDEndcap, R"DOC(
[Calibration] Returns energy sum of three crystals in forward endcap.
)DOC");

    REGISTER_VARIABLE("eclEnergy3BWDBarrel", eclEnergy3BWDBarrel, R"DOC(
[Calibration] Returns energy sum of three crystals in backward barrel.
)DOC");

    REGISTER_VARIABLE("eclEnergy3BWDEndcap", eclEnergy3BWDEndcap, R"DOC(
[Calibration] Returns energy sum of three crystals in backward endcap.
)DOC");

    // These variables require cDST inputs and the eclTRGInformation module run first
    VARIABLE_GROUP("ECL trigger calibration");
    REGISTER_VARIABLE("clusterNumberOfTCs(i, j, k, l)", eclNumberOfTCsForCluster, R"DOC(
[Calibration] Returns the number of TCs for this ECL cluster for a given TC theta ID range
:math:`(i, j)` and hit window :math:`(k, l)`.
)DOC");
    REGISTER_VARIABLE("clusterTCFADC(i, j, k, l)", eclTCFADCForCluster, R"DOC(
[Calibration] Returns the total FADC sum related to this ECL cluster for a given TC theta ID
range :math:`(i, j)` and hit window :math:`(k, l)`.
)DOC");
    REGISTER_VARIABLE("clusterTCIsMaximum", eclTCIsMaximumForCluster, R"DOC(
[Calibration] Returns True if cluster is related to maximum TC.
)DOC");
    REGISTER_VARIABLE("clusterTrigger", eclClusterTrigger, R"DOC(
[Calibration] Returns 1.0 if ECL cluster is matched to a trigger cluster (requires to run eclTriggerClusterMatcher
(which requires TRGECLClusters in the input file)) and 0 otherwise. Returns -1 if the matching code was not run.
NOT FOR PHASE2 DATA!
)DOC");
    REGISTER_VARIABLE("eclEnergyTC(i)", getEnergyTC, R"DOC(
[Eventbased][Calibration] Returns the energy (in FADC counts) for the :math:`i`-th trigger cell (TC), 1 based (1..576).
)DOC");
    REGISTER_VARIABLE("eclEnergyTCECLCalDigit(i)", getEnergyTCECLCalDigit, R"DOC(
[Eventbased][Calibration] Returns the energy (in GeV) for the :math:`i`-th trigger cell (TC)
based on ECLCalDigits, 1 based (1..576).
)DOC");
    REGISTER_VARIABLE("eclTimingTC(i)", getTimingTC, R"DOC(
[Eventbased][Calibration] Returns the time (in ns) for the :math:`i`-th trigger cell (TC), 1 based (1..576).
)DOC");
    REGISTER_VARIABLE("eclHitWindowTC(i)", eclHitWindowTC, R"DOC(
[Eventbased][Calibration] Returns the hit window for the :math:`i`-th trigger cell (TC), 1 based (1..576).
)DOC");
    REGISTER_VARIABLE("eclEventTimingTC", getEvtTimingTC, R"DOC(
[Eventbased][Calibration] Returns the ECL TC event time (in ns).
)DOC");
    REGISTER_VARIABLE("eclMaximumTCId", getMaximumTCId, R"DOC(
[Eventbased][Calibration] Returns the TC ID with maximum FADC value.
)DOC");


    REGISTER_VARIABLE("eclTimingTCECLCalDigit(i)", getTimingTCECLCalDigit, R"DOC(
[Eventbased][Calibration] Returns the time (in ns) for the :math:`i`-th trigger cell (TC) based
on ECLCalDigits, 1 based (1..576)
)DOC");

    REGISTER_VARIABLE("eclNumberOfTCs(i, j, k)", getNumberOfTCs, R"DOC(
[Eventbased][Calibration] Returns the number of TCs above threshold (i=FADC counts) for this event
for a given theta range (j-k)
)DOC");
    REGISTER_VARIABLE("eclEnergySumTC(i, j)", eclEnergySumTC, R"DOC(
[Eventbased][Calibration] Returns energy sum (in FADC counts) of all TC cells between two
theta ids i<=thetaid<=j, 1 based (1..17)
)DOC");
    REGISTER_VARIABLE("eclEnergySumTCECLCalDigit(i, j, k, l)", eclEnergySumTCECLCalDigit, R"DOC(
[Eventbased][Calibration] Returns energy sum (in GeV) of all TC cells between two theta ids i<=thetaid<=j,
1 based (1..17). k is the sum option: 0 (all), 1 (those with actual TC entries), 2 (sum of ECLCalDigit energy
in this TC above threshold). l is the threshold parameter for the option k 2.
)DOC");
    REGISTER_VARIABLE("eclEnergySumTCECLCalDigitInECLCluster", eclEnergySumTCECLCalDigitInECLCluster, R"DOC(
[Eventbased][Calibration] Returns energy sum (in GeV) of all ECLCalDigits if TC is above threshold
that are part of an ECLCluster above eclEnergySumTCECLCalDigitInECLClusterThreshold within TC thetaid 2-15.
)DOC");
    REGISTER_VARIABLE("eclEnergySumECLCalDigitInECLCluster", eclEnergySumECLCalDigitInECLCluster, R"DOC(
[Eventbased][Calibration] Returns energy sum (in GeV) of all ECLCalDigits that are part of an ECL cluster
above eclEnergySumTCECLCalDigitInECLClusterThreshold within TC thetaid 2-15.
)DOC");
    REGISTER_VARIABLE("eclEnergySumTCECLCalDigitInECLClusterThreshold", eclEnergySumTCECLCalDigitInECLClusterThreshold, R"DOC(
[Eventbased][Calibration] Returns threshold used to calculate eclEnergySumTCECLCalDigitInECLCluster.
)DOC");

  }
}
