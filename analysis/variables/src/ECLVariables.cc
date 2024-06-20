/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/variables/ECLVariables.h>

//framework
#include <framework/logging/Logger.h>

//analysis
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ECLEnergyCloseToTrack.h>
#include <analysis/utility/ReferenceFrame.h>
#include <analysis/ClusterUtility/ClusterUtils.h>
#include <analysis/VariableManager/Utility.h>

//MDST
#include <mdst/dataobjects/KlId.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/EventLevelClusteringInfo.h>

#include <Math/Vector4D.h>

#include <cmath>
#include <stack>



namespace Belle2 {
  namespace Variable {
    double distanceToMcKl(const Particle* particle)
    {
      if (particle->hasExtraInfo("mcdistanceKL")) {
        return particle->getExtraInfo("mcdistanceKL");
      } else {
        B2WARNING("The extraInfo mcdistanceKL is not registered! \n"
                  "This variable is only available for ECL based lists, and you have to run the function getNeutralHadronGeomMatches to use it");
        return Const::doubleNaN;
      }
    }

    double distanceToMcNeutron(const Particle* particle)
    {
      if (particle->hasExtraInfo("mcdistanceNeutron")) {
        return particle->getExtraInfo("mcdistanceNeutron");
      } else {
        B2WARNING("The extraInfo mcdistanceNeutron is not registered! \n"
                  "This variable is only available for ECL based lists, and you have to run the function getNeutralHadronGeomMatches to use it");
        return Const::doubleNaN;
      }
    }

    int mdstIndexMcKl(const Particle* particle)
    {
      if (particle->hasExtraInfo("mdstIndexTruthKL")) {
        return int(particle->getExtraInfo("mdstIndexTruthKL") + 0.1);
      } else {
        B2WARNING("The extraInfo mdstIndexTruthKL is not registered! \n"
                  "This variable is only available for ECL based lists, and you have to run the function getNeutralHadronGeomMatches to use it");
        return -1;
      }
    }

    int mdstIndexMcNeutron(const Particle* particle)
    {
      if (particle->hasExtraInfo("mdstIndexTruthNeutron")) {
        return int(particle->getExtraInfo("mdstIndexTruthNeutron") + 0.1);
      } else {
        B2WARNING("The extraInfo mdstIndexTruthNeutron is not registered! \n"
                  "This variable is only available for ECL based lists, and you have to run the function getNeutralHadronGeomMatches to use it");
        return -1;
      }
    }


    double beamBackgroundSuppression(const Particle* particle)
    {
      if (particle->hasExtraInfo("beamBackgroundSuppression")) {
        return particle->getExtraInfo("beamBackgroundSuppression");
      } else {
        B2WARNING("The extraInfo beamBackgroundSuppression is not registered! \n"
                  "This variable is only available for photons, and you either have to use the standard particle lists (stdPhotons or stdPi0s) or run getBeamBackgroundProbability on a photon list.");
        return Const::doubleNaN;
      }
    }

    double fakePhotonSuppression(const Particle* particle)
    {
      if (particle->hasExtraInfo("fakePhotonSuppression")) {
        return particle->getExtraInfo("fakePhotonSuppression");
      } else {
        B2WARNING("The extraInfo fakePhotonSuppression is not registered! \n"
                  "This variable is only available for photons, and you either have to use the standard particle lists (stdPhotons or stdPi0s) or run getFakePhotonProbability on a photon list.");
        return Const::doubleNaN;
      }
    }

    double hadronicSplitOffSuppression(const Particle* particle)
    {
      B2WARNING("This variable has been deprecated since light-2302-genetta and is no longer maintained with up to date weights. Please use the variable fakePhotonSuppression instead.");
      return fakePhotonSuppression(particle);
    }

    double eclClusterKlId(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (!cluster) {
        return Const::doubleNaN;
      }
      const KlId* klid = cluster->getRelatedTo<KlId>();
      if (!klid) {
        return Const::doubleNaN;
      }
      return klid->getKlId();
    }


    double eclPulseShapeDiscriminationMVA(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        if (eclClusterHasPulseShapeDiscrimination(particle)) {
          return cluster->getPulseShapeDiscriminationMVA();
        } else {
          return Const::doubleNaN;
        }
      }
      return Const::doubleNaN;
    }

    double eclClusterNumberOfHadronDigits(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        if (eclClusterHasPulseShapeDiscrimination(particle)) {
          return cluster->getNumberOfHadronDigits();
        } else
          return Const::doubleNaN;
      }
      return Const::doubleNaN;
    }

    double eclClusterDetectionRegion(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster)
        return cluster->getDetectorRegion();

      return Const::doubleNaN;
    }

    double eclClusterIsolation(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        auto minDist = cluster->getMinTrkDistance();
        if (minDist > 0)
          return minDist;
      }
      return Const::doubleNaN;
    }

    double eclClusterIsolationID(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster)
        return cluster->getMinTrkDistanceID();

      return Const::doubleNaN;
    }

    Manager::FunctionPtr eclClusterIsolationVar(const std::vector<std::string>& arguments)
    {
      if (arguments.size() > 2 or arguments.size() == 0)
        B2FATAL("Wrong number of arguments (2 required) for meta variable minC2TDistVar");
      std::string listName = "pi-:all";
      std::string variableName = arguments[0];
      if (arguments.size() == 2)
        listName = arguments[1];


      auto func = [listName, variableName](const Particle * particle) -> double {
        StoreObjPtr<ParticleList> particleList(listName);
        if (!(particleList.isValid()))
        {
          B2FATAL("Invalid Listname " << listName << " given to minC2TDistVar!");
        }
        const Variable::Manager::Var* var = Manager::Instance().getVariable(variableName);
        const ECLCluster* cluster = particle->getECLCluster();
        if (!cluster)
          return Const::doubleNaN;
        auto trackID = cluster->getMinTrkDistanceID();
        double result = Const::doubleNaN;
        // Find particle with that track ID:
        for (unsigned int i = 0; i < particleList->getListSize(); i++)
        {
          const Particle* listParticle = particleList->getParticle(i);
          if (listParticle and listParticle->getTrack() and listParticle->getTrack()->getArrayIndex() == trackID) {
            result = std::get<double>(var->function(listParticle));
            break;
          }
        }
        return result;
      };
      return func;
    }

    double eclClusterConnectedRegionID(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster)
        return  cluster->getConnectedRegionId();

      return Const::doubleNaN;
    }

    double eclClusterDeltaL(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster)
        return cluster->getDeltaL();

      return Const::doubleNaN;
    }

    double eclClusterErrorE(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        ClusterUtils clutls;
        const auto EPhiThetaCov = clutls.GetCovarianceMatrix3x3FromCluster(cluster);
        return sqrt(fabs(EPhiThetaCov[0][0]));
      }
      return Const::doubleNaN;
    }

    double eclClusterUncorrectedE(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getEnergyRaw();
      }
      return Const::doubleNaN;
    }

    double eclClusterE(const Particle* particle)
    {
      const auto& frame = ReferenceFrame::GetCurrent();

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        ClusterUtils clutls;
        ROOT::Math::PxPyPzEVector p4Cluster = clutls.GetCluster4MomentumFromCluster(cluster, particle->getECLClusterEHypothesisBit());

        return frame.getMomentum(p4Cluster).E();
      }
      return Const::doubleNaN;
    }

    double eclClusterHighestE(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getEnergyHighestCrystal();
      }
      return Const::doubleNaN;
    }

    double eclClusterCellId(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getMaxECellId();
      }
      return Const::doubleNaN;
    }

    // An array with each number representing the last number of the cellID per thetaID. There are 69 thetaIDs in total.
    const std::array<int, 69> lastCellIDperThetaID{48,   96,  160,  224,  288,  384,  480,  576,  672,  768,  864,
            1008, 1152, 1296, 1440, 1584, 1728, 1872, 2016, 2160, 2304, 2448,
            2592, 2736, 2880, 3024, 3168, 3312, 3456, 3600, 3744, 3888, 4032,
            4176, 4320, 4464, 4608, 4752, 4896, 5040, 5184, 5328, 5472, 5616,
            5760, 5904, 6048, 6192, 6336, 6480, 6624, 6768, 6912, 7056, 7200,
            7344, 7488, 7632, 7776, 7920, 8064, 8160, 8256, 8352, 8448, 8544,
            8608, 8672, 8736};

    double eclClusterThetaId(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        int cellID = cluster->getMaxECellId();
        return std::distance(lastCellIDperThetaID.begin(), std::lower_bound(lastCellIDperThetaID.begin(), lastCellIDperThetaID.end(),
                             cellID));
      }
      return Const::doubleNaN;
    }

    double eclClusterPhiId(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        int cellID = cluster->getMaxECellId();
        if (cellID <= 48) {
          return cellID - 1;
        } else {
          int closestinlist = lastCellIDperThetaID[std::distance(lastCellIDperThetaID.begin(), std::lower_bound(lastCellIDperThetaID.begin(),
                                                                                                             lastCellIDperThetaID.end(), cellID)) - 1];
          return cellID - closestinlist - 1;
        }
      }
      return Const::doubleNaN;
    }

    double eclClusterTiming(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getTime();
      }
      return Const::doubleNaN;
    }

    double eclClusterHasFailedTiming(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->hasFailedFitTime();
      }
      return Const::doubleNaN;
    }

    double eclClusterErrorTiming(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getDeltaTime99();
      }
      return Const::doubleNaN;
    }

    double eclClusterHasFailedErrorTiming(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->hasFailedTimeResolution();
      }
      return Const::doubleNaN;
    }

    double eclClusterTheta(const Particle* particle)
    {
      const auto& frame = ReferenceFrame::GetCurrent();

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        ClusterUtils clutls;
        ROOT::Math::PxPyPzEVector p4Cluster = clutls.Get4MomentumFromCluster(cluster, particle->getECLClusterEHypothesisBit());

        return frame.getMomentum(p4Cluster).Theta();
      }
      return Const::doubleNaN;
    }

    double eclClusterErrorTheta(const Particle* particle)
    {

      const ECLCluster* cluster  = particle->getECLCluster();
      if (cluster) {
        ClusterUtils clutls;
        const auto EPhiThetaCov = clutls.GetCovarianceMatrix3x3FromCluster(cluster);
        return sqrt(fabs(EPhiThetaCov[2][2]));
      }
      return Const::doubleNaN;
    }

    double eclClusterErrorPhi(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        ClusterUtils clutls;
        const auto EPhiThetaCov = clutls.GetCovarianceMatrix3x3FromCluster(cluster);
        return sqrt(fabs(EPhiThetaCov[1][1]));
      }
      return Const::doubleNaN;
    }

    double eclClusterPhi(const Particle* particle)
    {
      const auto& frame = ReferenceFrame::GetCurrent();

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        ClusterUtils clutls;
        ROOT::Math::PxPyPzEVector p4Cluster = clutls.Get4MomentumFromCluster(cluster, particle->getECLClusterEHypothesisBit());

        return frame.getMomentum(p4Cluster).Phi();
      }
      return Const::doubleNaN;
    }

    double eclClusterR(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getR();
      }
      return Const::doubleNaN;
    }

    double eclClusterE1E9(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getE1oE9();
      }
      return Const::doubleNaN;
    }

    double eclClusterE9E21(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getE9oE21();
      }
      return Const::doubleNaN;
    }

    double eclClusterAbsZernikeMoment40(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getAbsZernike40();
      }
      return Const::doubleNaN;
    }

    double eclClusterAbsZernikeMoment51(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getAbsZernike51();
      }
      return Const::doubleNaN;
    }

    double eclClusterZernikeMVA(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getZernikeMVA();
      }
      return Const::doubleNaN;
    }

    double eclClusterSecondMoment(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getSecondMoment();
      }
      return Const::doubleNaN;
    }

    double eclClusterLAT(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getLAT();
      }
      return Const::doubleNaN;
    }

    double eclClusterNHits(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getNumberOfCrystals();
      }
      return Const::doubleNaN;
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
      return Const::doubleNaN;
    }

    double nECLClusterTrackMatches(const Particle* particle)
    {
      // if no ECL cluster then nan
      const ECLCluster* cluster = particle->getECLCluster();
      if (!cluster)
        return Const::doubleNaN;

      // one or more tracks may be matched to charged particles
      size_t out = cluster->getRelationsFrom<Track>().size();
      return double(out);
    }

    double eclClusterId(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->getClusterId();
      }
      return Const::doubleNaN;
    }

    double eclClusterHasNPhotonsHypothesis(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      }
      return Const::doubleNaN;
    }

    double eclClusterHasNeutralHadronHypothesis(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron);
      }
      return Const::doubleNaN;
    }

    double eclClusterHasPulseShapeDiscrimination(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        return cluster->hasPulseShapeDiscrimination();
      }
      return Const::doubleNaN;
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
          return Const::doubleNaN;
        }
      }

      return Const::doubleNaN;
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
          return Const::doubleNaN;
        }
      }

      return Const::doubleNaN;
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
          return Const::doubleNaN;
        }
      }

      return Const::doubleNaN;
    }

    double weightedAverageECLTime(const Particle* particle)
    {
      int nDaughters = particle->getNDaughters();
      if (nDaughters < 1) {
        B2WARNING("The provided particle has no daughters!");
        return Const::doubleNaN;
      }

      double numer = 0, denom = 0;
      int numberOfClusterDaughters = 0;

      auto weightedECLTimeAverage = [&numer, &denom, &numberOfClusterDaughters](const Particle * p) {
        const ECLCluster* cluster = p->getECLCluster();
        if (cluster and not cluster->hasFailedFitTime()) {
          numberOfClusterDaughters ++;

          double time = cluster->getTime();
          B2DEBUG(10, "time[" << numberOfClusterDaughters << "] = " << time);
          double deltatime = cluster->getDeltaTime99();
          B2DEBUG(10, "deltatime[" << numberOfClusterDaughters << "] = " << deltatime);
          numer += time / pow(deltatime, 2);
          B2DEBUG(11, "numer[" << numberOfClusterDaughters << "] = " << numer);
          denom += 1 / pow(deltatime, 2);
          B2DEBUG(11, "denom[" << numberOfClusterDaughters << "] = " << denom);
        }
        return false;
      };

      particle->forEachDaughter(weightedECLTimeAverage, true, true);

      if (numberOfClusterDaughters < 1) {
        B2WARNING("There are no clusters or cluster matches amongst the daughters of the provided particle!");
        return Const::doubleNaN;
      }

      if (denom == 0) {
        B2WARNING("The denominator of the weighted mean is zero!");
        return Const::doubleNaN;
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
        return Const::doubleNaN;
      }

      double maxTimeDiff = -DBL_MAX;
      int numberOfClusterDaughters = 0;

      double averageECLTime = weightedAverageECLTime(particle);

      auto maxTimeDifference = [&maxTimeDiff, &numberOfClusterDaughters, &averageECLTime](const Particle * p) {

        const ECLCluster* cluster = p->getECLCluster();
        if (cluster) {
          numberOfClusterDaughters ++;

          double time = cluster->getTime();
          B2DEBUG(10, "time[" << numberOfClusterDaughters << "] = " << time);
          double deltatime = cluster->getDeltaTime99();
          B2DEBUG(10, "deltatime[" << numberOfClusterDaughters << "] = " << deltatime);
          double maxTimeDiff_temp = fabs((time - averageECLTime) / deltatime);
          B2DEBUG(11, "maxTimeDiff_temp[" << numberOfClusterDaughters << "] = " << maxTimeDiff_temp);
          if (maxTimeDiff_temp > maxTimeDiff)
            maxTimeDiff = maxTimeDiff_temp;
          B2DEBUG(11, "maxTimeDiff[" << numberOfClusterDaughters << "] = " << maxTimeDiff);
        }
        return false;
      };

      particle->forEachDaughter(maxTimeDifference, true, true);

      if (numberOfClusterDaughters < 1) {
        B2WARNING("There are no clusters or cluster matches amongst the daughters of the provided particle!");
        return Const::doubleNaN;
      }

      if (maxTimeDiff < 0) {
        B2WARNING("The max time difference is negative!");
        return Const::doubleNaN;
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
      } else return Const::doubleNaN;

      return Const::doubleNaN;
    }


    /*************************************************************
     * Event-based ECL clustering information
     */
    double nECLOutOfTimeCrystalsFWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double)elci->getNECLCalDigitsOutOfTimeFWD();
    }

    double nECLOutOfTimeCrystalsBarrel(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double)elci->getNECLCalDigitsOutOfTimeBarrel();
    }

    double nECLOutOfTimeCrystalsBWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double)elci->getNECLCalDigitsOutOfTimeBWD();
    }

    double nECLOutOfTimeCrystals(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double)elci->getNECLCalDigitsOutOfTime();
    }

    double nRejectedECLShowersFWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double)elci->getNECLShowersRejectedFWD();
    }

    double nRejectedECLShowersBarrel(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double)elci->getNECLShowersRejectedBarrel();
    }

    double nRejectedECLShowersBWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double)elci->getNECLShowersRejectedBWD();
    }

    double nRejectedECLShowers(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNECLShowersRejected();
    }

    double nKLMMultistripHitsFWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNKLMDigitsMultiStripFWD();
    }

    double nKLMMultistripHitsBarrel(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNKLMDigitsMultiStripBarrel();
    }

    double nKLMMultistripHitsBWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNKLMDigitsMultiStripBWD();
    }

    double nKLMMultistripHits(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNKLMDigitsMultiStrip();
    }

    double nECLShowersFWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNECLShowersFWD();
    }

    double nECLShowersBarrel(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNECLShowersBarrel();
    }

    double nECLShowersBWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNECLShowersBWD();
    }

    double nECLShowers(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNECLShowers();
    }

    double nECLLocalMaximumsFWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNECLLocalMaximumsFWD();
    }

    double nECLLocalMaximumsBarrel(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNECLLocalMaximumsBarrel();
    }

    double nECLLocalMaximumsBWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNECLLocalMaximumsBWD();
    }

    double nECLLocalMaximums(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNECLLocalMaximums();
    }

    double nECLTriggerCellsFWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNECLTriggerCellsFWD();
    }

    double nECLTriggerCellsBarrel(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNECLTriggerCellsBarrel();
    }

    double nECLTriggerCellsBWDEndcap(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNECLTriggerCellsBWD();
    }

    double nECLTriggerCells(const Particle*)
    {
      StoreObjPtr<EventLevelClusteringInfo> elci;
      if (!elci) return Const::doubleNaN;
      return (double) elci->getNECLTriggerCells();
    }

    double eclClusterEoP(const Particle* part)
    {
      double E = eclClusterE(part);
      if (part->hasExtraInfo("bremsCorrectedPhotonEnergy")) {
        E += part->getExtraInfo("bremsCorrectedPhotonEnergy");
      }
      const double p =  part->getMomentumMagnitude();
      if (0 == p) { return Const::doubleNaN;}
      return E / p;
    }

    double eclClusterOnlyInvariantMass(const Particle* part)
    {
      int nDaughters = part->getNDaughters();
      ROOT::Math::PxPyPzEVector sum;

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
            ROOT::Math::PxPyPzEVector p4Cluster = clutls.Get4MomentumFromCluster(cluster, clusterBit);
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
          return Const::doubleNaN;
        }
        B2DEBUG(10, "Number of daughters with cluster associated = " << nClusterDaughters);
        return sum.M();
      }
    }

    Manager::FunctionPtr photonHasOverlap(const std::vector<std::string>& arguments)
    {
      std::string cutString = "";
      if (arguments.size() > 0) {
        cutString = arguments[0];
      }
      std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(cutString));

      std::string photonlistname = "gamma:all";
      if (arguments.size() > 1) {
        photonlistname = arguments[1];
      }

      std::string tracklistname = "e-:all";
      if (arguments.size() > 2) {
        tracklistname = arguments[2];
      }

      auto func = [cut, photonlistname, tracklistname](const Particle * particle) -> double {

        if (particle->getPDGCode() != Const::photon.getPDGCode())
        {
          B2WARNING("The variable photonHasOverlap is supposed to be calculated for photons. Returning NaN.");
          return Const::doubleNaN;
        }

        StoreObjPtr<ParticleList> photonlist(photonlistname);
        if (!(photonlist.isValid()))
        {
          B2WARNING("The provided particle list " << photonlistname << " does not exist."
                    " Therefore, the variable photonHasOverlap can not be calculated. Returning NaN.");
          return Const::doubleNaN;
        }
        if (photonlist->getPDGCode() != Const::photon.getPDGCode())
        {
          B2WARNING("The list " << photonlistname << " does not contain photons."
                    " Therefore, the variable photonHasOverlap can not be calculated reliably. Returning NaN.");
          return Const::doubleNaN;
        }

        StoreObjPtr<ParticleList> tracklist(tracklistname);
        if (!(tracklist.isValid()))
        {
          B2WARNING("The provided particle list " << tracklistname << " does not exist."
                    " Therefore, the variable photonHasOverlap can not be calculated. Returning NaN.");
          return Const::doubleNaN;
        }
        if (!Const::chargedStableSet.contains(Const::ParticleType(abs(tracklist->getPDGCode()))))
        {
          B2WARNING("The list " << tracklistname << " does not contain charged final state particles."
                    " Therefore, the variable photonHasOverlap can not be calculated reliably. Returning NaN.");
          return Const::doubleNaN;
        }

        double connectedRegionID = eclClusterConnectedRegionID(particle);
        unsigned mdstArrayIndex = particle->getMdstArrayIndex();

        for (unsigned int i = 0; i < photonlist->getListSize(); i++)
        {
          const Particle* part = photonlist->getParticle(i);

          // skip the particle itself
          if (part->getMdstArrayIndex() == mdstArrayIndex) {
            continue;
          }

          // skip photons that do not fulfill the provided criteria
          if (!cut->check(part)) {
            continue;
          }

          if (connectedRegionID == eclClusterConnectedRegionID(part)) {
            return 1;
          }
        }

        for (unsigned int i = 0; i < tracklist->getListSize(); i++)
        {
          const Particle* part = tracklist->getParticle(i);

          // skip tracks that do not fulfill the provided criteria
          if (!cut->check(part)) {
            continue;
          }

          if (connectedRegionID == eclClusterConnectedRegionID(part)) {
            return 1;
          }
        }

        return 0;
      };
      return func;
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
..

)DOC","cm");

    REGISTER_VARIABLE("minC2TDist", eclClusterIsolation, R"DOC(
Returns the distance between the ECL cluster and its nearest track. 

For all tracks in the event, the distance between each of their extrapolated hits in the ECL and the ECL shower 
position is calculated, and the overall smallest distance is returned. The track array index of the track that is 
closest to the ECL cluster can be retrieved using `minC2TDistID`. 

If the calculated distance is greater than :math:`250.0`, the returned distance will be capped at :math:`250.0`. 
If there are no extrapolated hits found in the ECL for the event, NaN will be returned. 

.. note::
    This distance is calculated on the reconstructed level.

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`0.0`
    | Upper limit: :math:`250.0`
    | Precision: :math:`10` bit
..

)DOC","cm");
    REGISTER_VARIABLE("minC2TDistID", eclClusterIsolationID, R"DOC(
Returns the track array index of the nearest track to the ECL cluster. The nearest track is calculated
using the `minC2TDist` variable. 
)DOC");
    REGISTER_METAVARIABLE("minC2TDistVar(variable,particleList=pi-:all)", eclClusterIsolationVar, R"DOC(
Returns the variable value of the nearest track to the given ECL cluster as calculated by `minC2TDist`. The 
first argument is the variable name, e.g. `nCDCHits`, while the second (optional) argument is the particle list name which 
will be used to pick up the nearest track in the calculation of `minC2TDist`. The default particle list used 
is ``pi-:all``. 
)DOC", Manager::VariableDataType::c_double);
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

- Energy Bias Correction module, sub-percent correction, is NOT applied on clusterE, but on photon energy
  and momentum. Only applied to data.

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
..

)DOC","GeV");
    REGISTER_VARIABLE("clusterErrorE", eclClusterErrorE, R"DOC(
Returns ECL cluster's uncertainty on energy
(from background level and energy dependent tabulation).

)DOC","GeV");
    REGISTER_VARIABLE("clusterErrorPhi", eclClusterErrorPhi, R"DOC(
Returns ECL cluster's uncertainty on :math:`\phi`
(from background level and energy dependent tabulation).

)DOC","rad");
    REGISTER_VARIABLE("clusterErrorTheta", eclClusterErrorTheta, R"DOC(
Returns ECL cluster's uncertainty on :math:`\theta`
(from background level and energy dependent tabulation).

)DOC","rad");

    REGISTER_VARIABLE("clusterR", eclClusterR, R"DOC(
Returns ECL cluster's centroid distance from :math:`(0,0,0)`.

)DOC","cm");
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
..

)DOC","rad");
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
..

)DOC","rad");
    REGISTER_VARIABLE("clusterTiming", eclClusterTiming, R"DOC(
**In Belle II:**
Returns the time of the ECL cluster. It is calculated as the Photon timing minus the Event t0.
Photon timing is given by the fitted time of the recorded waveform of the highest energy crystal in the
cluster. After all calibrations and corrections (including Time-Of-Flight), photons from the interaction
point (IP) should have a Photon timing that corresponds to the Event t0, :math:`t_{0}`.  The Event t0 is the
time of the event and may be measured by a different sub-detector (see Event t0 documentation). For an ECL
cluster produced at the interaction point in time with the event, the cluster time should be consistent with zero
within the uncertainties. Special values are returned if the fit for the Photon timing fails (see
documentation for `clusterHasFailedTiming`). (For MC, the calibrations and corrections are not fully simulated).

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`-1000.0`
    | Upper limit: :math:`1000.0`
    | Precision: :math:`12` bit
..

**In Belle:**
Returns the trigger cell (TC) time of the ECL cluster (photon).
This information is available only in Belle data since experiment 31, and not available in Belle MC.
Clusters produced at the interaction point in time with the event, have TC time in the range of 9000-11000
Calculated based on the Appendix of Belle note 831.

.. note::
    | In case this variable is obtained from Belle data that is stored in Belle II mdst/udst format, it will be truncated to:
    | Lower limit: :math:`-1000.0`
    | Upper limit: :math:`1000.0`
    | Precision: :math:`12` bit
..

)DOC","ns");
    REGISTER_VARIABLE("clusterHasFailedTiming", eclClusterHasFailedTiming, R"DOC(
Status bit for if the ECL cluster's timing fit failed. Photon timing is given by the fitted time
of the recorded waveform of the highest energetic crystal in a cluster; however, that fit can fail and so
this variable tells the user if that has happened.
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
..

)DOC","ns");
    REGISTER_VARIABLE("clusterHasFailedErrorTiming", eclClusterHasFailedErrorTiming, R"DOC(
Status bit for if the ECL cluster's timing uncertainty calculation failed. Photon timing is given by the fitted time
of the recorded waveform of the highest energetic crystal in a cluster; however, that fit can fail and so
this variable tells the user if that has happened.
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
..

)DOC","GeV");
    REGISTER_VARIABLE("clusterCellID", eclClusterCellId,
                      "Returns cellId of the crystal with highest energy in the ECLCluster.");
    REGISTER_VARIABLE("clusterThetaID", eclClusterThetaId,
                      "Returns thetaId of the crystal with highest energy in the ECLCluster.");
    REGISTER_VARIABLE("clusterPhiID", eclClusterPhiId,
                      "Returns phiId of the crystal with highest energy in the ECLCluster.");
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
    S = \frac{1}{S_{0}(\theta)}\frac{\sum_{i=0}^{n} w_{i} E_{i} r^2_{i}}{\sum_{i=0}^{n} w_{i} E_{i}}

where :math:`E_{i} = (E_0, E_1, ...)` are the single crystal energies sorted by energy, :math:`w_{i}` is
the crystal weight, and :math:`r_{i}` is the distance of the :math:`i`-th digit to the shower center projected
to a plane perpendicular to the shower axis. :math:`S_{0}(\theta)` normalizes :math:`S` to 1 for isolated photons.

.. note::
    | Please read `this <importantNoteECL>` first.
    | Lower limit: :math:`0.0`
    | Upper limit: :math:`40.0`
    | Precision: :math:`10` bit
..

)DOC","dimensionless");
    REGISTER_VARIABLE("clusterLAT", eclClusterLAT, R"DOC(
Returns lateral energy distribution (shower variable). It is defined as following:

.. math::
    S = \frac{\sum_{i=2}^{n} w_{i} E_{i} r^2_{i}}{(w_{0} E_{0} + w_{1} E_{1}) r^2_{0} + \sum_{i=2}^{n} w_{i} E_{i} r^2_{i}}

where :math:`E_{i} = (E_{0}, E_{1}, ...)` are the single crystal energies sorted by energy
(:math:`E_{0}` is the highest energy and :math:`E_{1}` the second highest), :math:`w_{i}`
is the crystal weight, :math:`r_{i}` is the distance of the :math:`i`-th digit to the
shower center projected to a plane perpendicular to the shower axis,
and :math:`r_{0} \approx 5\,cm` is the distance between two crystals.

clusterLAT peaks around 0.3 for radially symmetrical electromagnetic showers and is larger
for hadronic events, and electrons with a close-by radiative or Bremsstrahlung photon.

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
    REGISTER_VARIABLE("clusterHasPulseShapeDiscrimination", eclClusterHasPulseShapeDiscrimination, R"DOC(
Status bit to indicate if cluster has digits with waveforms that passed energy and :math:`\chi^2`
thresholds for computing PSD variables.
)DOC");
    REGISTER_VARIABLE("beamBackgroundSuppression", beamBackgroundSuppression, R"DOC(
Returns the output of an MVA classifier that uses shower-related variables to distinguish true photon clusters from beam background clusters.
Class 1 is for true photon clusters while class 0 is for beam background clusters.

The MVA has been trained using MC and the features used are:

- `clusterTiming`
- `clusterPulseShapeDiscriminationMVA`
- `clusterE`
- `clusterTheta`
- `clusterZernikeMVA`

Both run-dependent and run-independent weights are available. For more information on this, and for usage recommendations, please see
the `Neutrals Performance Confluence Page <https://confluence.desy.de/display/BI/Neutrals+Performance>`_.
)DOC");
    REGISTER_VARIABLE("fakePhotonSuppression", fakePhotonSuppression, R"DOC(
Returns the output of an MVA classifier that uses shower-related variables to distinguish true photon clusters from fake photon clusters (e.g. split-offs,
track-cluster matching failures etc.). Class 1 is for true photon clusters while class 0 is for fake photon clusters. 

The MVA has been trained using MC and the features are:

- `clusterPulseShapeDiscriminationMVA`
- `minC2TDist`
- `clusterZernikeMVA`
- `clusterE`
- `clusterTiming`
- `clusterTheta`

This MVA is the same as the one used for `hadronicSplitOffSuppression` but that variable should not be used as it is deprecated and does not use the new weights. 

Both run-dependent and run-independent weights are available. For more information on this, and for usage recommendations, please see
the `Neutrals Performance Confluence Page <https://confluence.desy.de/display/BI/Neutrals+Performance>`_.
)DOC");
    REGISTER_VARIABLE("hadronicSplitOffSuppression", hadronicSplitOffSuppression, R"DOC(
Returns the output of an MVA classifier that uses shower-related variables to distinguish true photon clusters from hadronic splitoff clusters.
The classes are: 

- 1 for true photon clusters
- 0 for hadronic splitoff clusters

The MVA has been trained using samples of signal photons and hadronic splitoff photons coming from MC. The features used are (in decreasing order of significance): 

- `clusterPulseShapeDiscriminationMVA`
- `minC2TDist`
- `clusterZernikeMVA`
- `clusterE`
- `clusterLAT`
- `clusterE1E9`
- `clusterSecondMoment`
)DOC");
    MAKE_DEPRECATED("hadronicSplitOffSuppression", false, "light-2302-genetta", R"DOC(
                     Use the variable `fakePhotonSuppression` instead which is maintained and uses the latest weight files.
)DOC");
    REGISTER_VARIABLE("clusterKlId", eclClusterKlId, R"DOC(
Returns MVA classifier that uses ECL clusters variables to discriminate Klong clusters from em background.
    
- 1 for Kl
- 0 for background
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
    REGISTER_VARIABLE("clusterHasNPhotons", eclClusterHasNPhotonsHypothesis, R"DOC(
Returns 1.0 if cluster has the 'N photons' hypothesis (historically called 'N1'),
0.0 if not, and NaN if no cluster is associated to the particle.
)DOC");
    REGISTER_VARIABLE("clusterHasNeutralHadron", eclClusterHasNeutralHadronHypothesis, R"DOC(
Returns 1.0 if the cluster has the 'neutral hadrons' hypothesis (historically called 'N2'),
0.0 if not, and NaN if no cluster is associated to the particle.
)DOC");
    REGISTER_VARIABLE("eclExtTheta", eclExtTheta, R"DOC(
Returns extrapolated :math:`\theta` of particle track associated to the cluster (if any). Requires module ECLTrackCalDigitMatch to be executed.

)DOC","rad");
    REGISTER_VARIABLE("eclExtPhi", eclExtPhi, R"DOC(
Returns extrapolated :math:`\phi` of particle track associated to the cluster (if any). Requires module ECLTrackCalDigitMatch to be executed..

)DOC","rad");
    REGISTER_VARIABLE("eclExtPhiId", eclExtPhiId, R"DOC(
Returns extrapolated :math:`\phi` ID of particle track associated to the cluster (if any). Requires module ECLTrackCalDigitMatch to be executed..
)DOC");
    REGISTER_VARIABLE("weightedAverageECLTime", weightedAverageECLTime, R"DOC(
Returns ECL weighted average time of all clusters (neutrals) and matched clusters (charged) of daughters
(of any generation) of the provided particle.

)DOC", "ns");
    REGISTER_VARIABLE("maxWeightedDistanceFromAverageECLTime", maxWeightedDistanceFromAverageECLTime, R"DOC(
Returns maximum weighted distance between time of the cluster of a photon and the ECL average time, amongst
the clusters (neutrals) and matched clusters (charged) of daughters (of all generations) of the provided particle.

)DOC", "ns");
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
If the number exceeds 255 (uint8_t maximum value) the variable is set to 255.
)DOC");

    REGISTER_VARIABLE("nRejectedECLShowersBarrel", nRejectedECLShowersBarrel, R"DOC(
[Eventbased] Returns the number of showers in the ECL that do not become clusters, from the barrel.
If the number exceeds 255 (uint8_t maximum value) the variable is set to 255.
)DOC");

    REGISTER_VARIABLE("nRejectedECLShowersBWDEndcap", nRejectedECLShowersBWDEndcap, R"DOC(
[Eventbased] Returns the number of showers in the ECL that do not become clusters, from the backward endcap.
If the number exceeds 255 (uint8_t maximum value) the variable is set to 255.
)DOC");
  
  REGISTER_VARIABLE("nKLMMultistripHitsFWDEndcap", nKLMMultistripHitsFWDEndcap, R"DOC(
[Eventbased] Returns the number of multi-strip hits in the KLM forward endcap.
)DOC");
  
  REGISTER_VARIABLE("nKLMMultistripHitsBarrel", nKLMMultistripHitsBarrel, R"DOC(
[Eventbased] Returns the number of multi-strip hits in the KLM barrel.
)DOC");
  
  REGISTER_VARIABLE("nKLMMultistripHitsBWDEndcap", nKLMMultistripHitsBWDEndcap, R"DOC(
[Eventbased] Returns the number of multi-strip hits in the KLM backward endcap.
)DOC");
  
  REGISTER_VARIABLE("nKLMMultistripHits", nKLMMultistripHits, R"DOC(
[Eventbased] Returns the number of multi-strip hits in the KLM.
)DOC");
  
  REGISTER_VARIABLE("nECLShowersFWDEndcap", nECLShowersFWDEndcap, R"DOC(
[Eventbased] Returns the number of ECLShowers in the forward endcap.
)DOC");
  
  REGISTER_VARIABLE("nECLShowersBarrel", nECLShowersBarrel, R"DOC(
[Eventbased] Returns the number of ECLShowers in the barrel.
)DOC");
  
  REGISTER_VARIABLE("nECLShowersBWDEndcap", nECLShowersBWDEndcap, R"DOC(
[Eventbased] Returns the number of ECLShowers in the backward endcap.
)DOC");

  REGISTER_VARIABLE("nECLShowers", nECLShowers, R"DOC(
[Eventbased] Returns the number of ECLShowers.
)DOC");
  
  REGISTER_VARIABLE("nECLLocalMaximumsFWDEndcap", nECLLocalMaximumsFWDEndcap, R"DOC(
[Eventbased] Returns the number of LocalMaximums in the ECL forward endcap.
)DOC");
  
  REGISTER_VARIABLE("nECLLocalMaximumsBarrel", nECLLocalMaximumsBarrel, R"DOC(
[Eventbased] Returns the number of LocalMaximums in the ECL barrel.
)DOC");
  
  REGISTER_VARIABLE("nECLLocalMaximumsBWDEndcap", nECLLocalMaximumsBWDEndcap, R"DOC(
[Eventbased] Returns the number of LocalMaximums in the ECL backward endcap.
)DOC");

  REGISTER_VARIABLE("nECLLocalMaximums", nECLLocalMaximums, R"DOC(
[Eventbased] Returns the number of LocalMaximums in the ECL.
)DOC");
  
  REGISTER_VARIABLE("nECLTriggerCellsFWDEndcap", nECLTriggerCellsFWDEndcap, R"DOC(
[Eventbased] Returns the number of ECL trigger cells above 100 MeV in the forward endcap.
)DOC");
  
  REGISTER_VARIABLE("nECLTriggerCellsBarrel", nECLTriggerCellsBarrel, R"DOC(
[Eventbased] Returns the number of ECL trigger cells above 100 MeV in the barrel.
)DOC");
  
  REGISTER_VARIABLE("nECLTriggerCellsBWDEndcap", nECLTriggerCellsBWDEndcap, R"DOC(
[Eventbased] Returns the number of ECL trigger cells above 100 MeV in the backward endcap.
)DOC");

  REGISTER_VARIABLE("nECLTriggerCells", nECLTriggerCells, R"DOC(
[Eventbased] Returns the number of ECL trigger cells above 100 MeV.
)DOC");

    REGISTER_VARIABLE("eclClusterOnlyInvariantMass", eclClusterOnlyInvariantMass, R"DOC(
[Expert] The invariant mass calculated from all ECLCluster daughters (i.e. photons) and
cluster-matched tracks using the cluster 4-momenta.

Used for ECL-based dark sector physics and debugging track-cluster matching.

)DOC","GeV/:math:`\\text{c}^2`");

    REGISTER_METAVARIABLE("photonHasOverlap(cutString, photonlistname, tracklistname)", photonHasOverlap, R"DOC(
      Returns true if the connected ECL region of the particle's cluster is shared by another particle's cluster.
      Neutral and charged cluster are considered.
      A cut string can be provided to ignore cluster that do not satisfy the given criteria.
      By default, the ParticleList ``gamma:all`` is used for the check of neutral ECL cluster,
      and the ParticleList ``e-:all`` for the check of charged ECL cluster.
      However, one can customize the name of the ParticleLists via additional arguments.
      If no argument or only a cut string is provided and ``gamma:all`` or ``e-:all`` does not exist
      or if the variable is requested for a particle that is not a photon, NaN is returned.
      )DOC", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("clusterUncorrE", eclClusterUncorrectedE, R"DOC(
[Expert] [Calibration] Returns ECL cluster's uncorrected energy. That is, before leakage corrections.
This variable should only be used for study of the ECL. Please see :b2:var:`clusterE`.

)DOC","GeV");

    REGISTER_VARIABLE("distanceToMcKl",distanceToMcKl,R"DOC(
Returns the distance to the nearest truth KL particle, extrapolated to the cluster radius. To use
this variable, it is required to run getNeutralHadronGeomMatches function. Optionally, it can return 
negative values to indicate that the ECL cluster should be removed from the analysis to correct for data
to MC difference in KL efficiency.

)DOC", "cm");

    REGISTER_VARIABLE("distanceToMcNeutron",distanceToMcNeutron,R"DOC(
Returns the distance to the nearest truth (anti)neutron, extrapolated to the cluster radius. To use
this variable, it is required to run getNeutralHadronGeomMatches function. Optionally, it can return 
negative values to indicate that the ECL cluster should be removed from the analysis to correct for data
to MC difference in KL efficiency.

)DOC", "cm");

    REGISTER_VARIABLE("mdstIndexMcKl",mdstIndexMcKl,R"DOC(
      Returns the mdst index of the nearest truth KL, extrapolated to the cluster radius, if it is
      within the matching cone. To use this variable, it is required to run getNeutralHadronGeomMatches function. 
)DOC");

    REGISTER_VARIABLE("mdstIndexMcNeutron",mdstIndexMcNeutron,R"DOC(
      Returns the mdst index of the nearest truth (anti)neutron, extrapolated to the cluster radius, if it is
      within the matching cone. To use this variable, it is required to run getNeutralHadronGeomMatches function. 
)DOC");

  }
}
