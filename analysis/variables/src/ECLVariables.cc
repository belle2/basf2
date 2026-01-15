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
#include <framework/database/DBObjPtr.h>
#include <framework/core/Environment.h>

//analysis
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ECLEnergyCloseToTrack.h>
#include <analysis/utility/ReferenceFrame.h>
#include <analysis/ClusterUtility/ClusterUtils.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/dbobjects/ECLTimingNormalization.h>

//MDST
#include <mdst/dataobjects/KlId.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/EventLevelClusteringInfo.h>

#include <Math/Vector4D.h>
#include <TRandom.h>
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

    double eclClusterTimeNorm90(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      StoreObjPtr<EventLevelClusteringInfo> elci;

      //..Get the appropriate (data or MC) payload
      std::string payloadName = "ECLTimingNormalization_data";
      if (Environment::Instance().isMC()) {payloadName = "ECLTimingNormalization_MC";}
      static DBObjPtr<ECLTimingNormalization> ECLTimingNormalization(payloadName);
      if (!ECLTimingNormalization.isValid()) {
        B2FATAL(payloadName << " payload is not available");
      }

      if (elci and cluster) {

        //..Only valid for crystals in CDC acceptance
        unsigned short cellID = cluster->getMaxECellId(); // [1, 8736]
        const unsigned short firstCellID = 161; // first cellID in CDC acceptance
        const unsigned short lastCellID = 8608; // last cellID in CDC acceptance
        if (cellID < firstCellID or cellID > lastCellID) {return Const::doubleNaN;}
        int iCrys = cellID - 1; // [0, 8735]

        //..clusterTiming. Dither to remove artifacts from compression
        double rawTime = cluster->getTime();
        const double timingBit = 2000. / 4096.;
        double tSmear = timingBit * (gRandom->Uniform() - 0.5);
        double time = rawTime + tSmear;

        //..Uncorrected single crystal energy
        double clusterECorrected = cluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
        double clusterERaw = cluster->getEnergyRaw();
        double singleECorrected = cluster->getEnergyHighestCrystal();
        double singleCrystalE = singleECorrected * clusterERaw / clusterECorrected;
        double basicEScale = 0.1 / singleCrystalE;

        //..Background level for the region for this cellID
        const int firstBarrel = 1153; // first cellID in barrel
        const int lastBarrel = 7776; // last cellID in barrel
        double ootCrys = elci->getNECLCalDigitsOutOfTimeBarrel();
        if (cellID < firstBarrel) {ootCrys = elci->getNECLCalDigitsOutOfTimeFWD();}
        if (cellID > lastBarrel) {ootCrys = elci->getNECLCalDigitsOutOfTimeBWD();}

        //..Time walk correction (7 parameters)
        //  E0 / bias at E0 / lowE slope / highE slope / curvature / Emin / Emax
        std::array< std::array<float, 7>,  8736> par7 = ECLTimingNormalization->getTimeWalkPar();

        float EForCor = singleCrystalE;
        if (EForCor < par7[iCrys][5]) {EForCor = par7[iCrys][5];}
        if (EForCor > par7[iCrys][6]) {EForCor = par7[iCrys][6];}
        float dE = EForCor - par7[iCrys][0];
        double timeWalkCor = par7[iCrys][1] + par7[iCrys][2] * dE;
        if (dE > 0) {
          timeWalkCor = par7[iCrys][1] + par7[iCrys][3] * dE + par7[iCrys][4] * dE * dE;
        }

        //..Dependence on regional background level (5 parameters)
        //  intercept / slope / p2 / min ootCrys / max ootCrys
        std::array< std::array<float, 5>,  8736> bPar = ECLTimingNormalization->getBackgroundPar();

        float oot = ootCrys;
        if (oot < bPar[iCrys][3]) {oot = bPar[iCrys][3];}
        if (oot > bPar[iCrys][4]) {oot = bPar[iCrys][4];}
        double backgroundNorm = bPar[iCrys][0] + bPar[iCrys][1] * oot + bPar[iCrys][2] * oot * oot;

        //..Dependence on single crystal energy (7 parameters)
        //  E0 / res at E0 / lowE slope / highE slope / curvature / Emin / Emax
        par7 = ECLTimingNormalization->getEnergyPar();

        EForCor = singleCrystalE;
        if (EForCor < par7[iCrys][5]) {EForCor = par7[iCrys][5];}
        if (EForCor > par7[iCrys][6]) {EForCor = par7[iCrys][6];}
        dE = EForCor - par7[iCrys][0];
        double energyNorm = par7[iCrys][1] + par7[iCrys][2] * dE;
        if (dE > 0) {
          energyNorm = par7[iCrys][1] + par7[iCrys][3] * dE  + par7[iCrys][4] * dE * dE;
        }

        //..Overall normalization. Cannot be too small.
        double minTNorm = (double)ECLTimingNormalization->getMinTNormalization();
        double tNormalization = basicEScale * backgroundNorm * energyNorm;
        if (tNormalization < minTNorm) {tNormalization = minTNorm;}
        double tNorm90 = (time - timeWalkCor) / tNormalization;
        return tNorm90;

      } else {
        return Const::doubleNaN;

      }
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
        int mdstSource = particle->getMdstSource();

        for (unsigned int i = 0; i < photonlist->getListSize(); i++)
        {
          const Particle* part = photonlist->getParticle(i);

          // skip the particle itself
          if (part->getMdstSource() == mdstSource) {
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

    VARIABLE_GROUP("ECL cluster related");
    REGISTER_VARIABLE("clusterEoP", eclClusterEoP, R"DOC(
Returns ratio of the cluster energy :math:`E` over momentum :math:`p`.

)DOC");
    REGISTER_VARIABLE("clusterReg", eclClusterDetectionRegion, R"DOC(
Returns an integer code representing the ECL region for the cluster:

- 1: forward, 2: barrel, 3: backward
- 11: between forward endcap and barrel, 13: between backward endcap and barrel
- 0: outside the ECL acceptance region

)DOC");
    REGISTER_VARIABLE("clusterDeltaLTemp", eclClusterDeltaL, R"DOC(
Returns the :math:`\Delta L` for the cluster shape as defined below. 

First, the cluster direction is constructed by calculating the weighted average of the orientation 
for the crystals in the cluster. The POCA of the vector with this direction originating from the
cluster center and an extrapolated track can be used to the calculate the shower
depth. :math:`\Delta L` is then defined as the distance between this intersection and the cluster center. 

.. attention::
    This distance is calculated on the reconstruction level and is temporarily
    included in mdst for investigation purposes. If it is found
    that it is not crucial for physics analyses then this variable will be removed
    in future releases. So keep in mind that this variable might be removed in the future.

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`-250.0`
    | - Upper limit: :math:`250.0`
    | - Precision: :math:`10` bit
..

)DOC","cm");
    REGISTER_VARIABLE("minC2TDist", eclClusterIsolation, R"DOC(
Returns the distance between the cluster and its nearest track.  

For all tracks in the event, the distance between each of their extrapolated hits in the ECL and the ECL shower 
position is calculated, and the overall smallest distance is returned. If there are no extrapolated hits found in the ECL 
for the event, ``NaN`` will be returned. The track array index of the track that is closest to the cluster can be 
retrieved using `minC2TDistID`. 

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`0.0`
    | - Upper limit: :math:`250.0`
    | - Precision: :math:`10` bit
..

)DOC","cm");
    REGISTER_VARIABLE("minC2TDistID", eclClusterIsolationID, R"DOC(
Returns the track array index of the nearest track to the cluster. The nearest track is calculated
using the `minC2TDist` variable. 

)DOC");
    REGISTER_METAVARIABLE("minC2TDistVar(variable,particleList=pi-:all)", eclClusterIsolationVar, R"DOC(
Returns the value of your chosen variable for the track nearest to the given cluster as calculated by 
`minC2TDist`. 

The first parameter ``variable`` is the variable name e.g. `nCDCHits`, while the second (optional) parameter ``particleList`` 
is the particle list name which will be used in the calculation of `minC2TDist`. The default particle list used 
is ``pi-:all``. 

)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("clusterE", eclClusterE, R"DOC(
Returns the cluster energy corrected for leakage and background.

.. attention::
    We only store clusters with :math:`E > 20\,` MeV.

.. topic:: Calculating Photon Energy

    The raw photon energy is given by the weighted sum of all crystal energies within the cluster.
    The weights per crystals are :math:`\leq 1` after cluster energy splitting in the case of overlapping
    clusters. The number of crystals that are included in the sum depends on a initial energy estimation
    and local beam background levels for the highest energy crystal. The crystal number is optimized to minimize
    the resolution of photons. Photon energy distributions always show a low energy tail
    due to unavoidable longitudinal and transverse leakage. The peak position of the photon energy distributions are 
    corrected to match the true photon energy in MC. The corrections applied include: 

    - Leakage correction: using  MC samples of mono-energetic single photon, a correction factor
      :math:`f` as function of the reconstructed detector position, photon energy and beam background level 
      is determined via :math:`f = \frac{\text{peak_reconstructed}}{\text{energy_true}}`

    - Cluster energy calibration (data only): to reach the target precision of :math:`< 1.8\%` energy
      resolution for high energetic photons, the remaining difference between MC and data is calibrated
      using kinematically fits to muon pairs

    It is important to note that after perfect leakage corrections and cluster energy calibrations,
    the :math:`\pi^{0}` mass peak will be shifted slightly to smaller values than the PDG average
    due to the low energy tails of photons. 

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`-5` (:math:`e^{-5} = 0.00674\,` GeV in the lab frame)
    | - Upper limit: :math:`3.0` (:math:`e^3 = 20.08553\,` GeV in the lab frame)
    | - Precision: :math:`18` bit

)DOC","GeV");
    REGISTER_VARIABLE("clusterErrorE", eclClusterErrorE, R"DOC(
Returns the uncertainty on the cluster energy. It is derived from 
a background level and energy-dependent error tabulation. 

.. danger::
    This variable should not be used a selection variable or in an MVA for photon identification as it is not 
    a directly-reconstructed quantity. For more information please see the 
    `ECL Cluster Recommendations <https://indico.belle2.org/event/13722/contributions/84645/attachments/31512/51585/ecl_recommendation.pdf>`_ 
    slides.

)DOC","GeV");
    REGISTER_VARIABLE("clusterErrorPhi", eclClusterErrorPhi, R"DOC(
Returns the uncertainty on the phi angle of the cluster. It is derived from 
a background level and energy-dependent error tabulation. 

.. danger::
    This variable should not be used a selection variable or in an MVA for photon identification as it is not 
    a directly-reconstructed quantity. For more information please see the 
    `ECL Cluster Recommendations <https://indico.belle2.org/event/13722/contributions/84645/attachments/31512/51585/ecl_recommendation.pdf>`_ 
    slides.

)DOC","rad");
    REGISTER_VARIABLE("clusterErrorTheta", eclClusterErrorTheta, R"DOC(
Returns the uncertainty on the theta angle of the cluster. It is derived from 
a background level and energy-dependent error tabulation. 

.. danger::
    This variable should not be used a selection variable or in an MVA for photon identification as it is not 
    a directly-reconstructed quantity. For more information please see the 
    `ECL Cluster Recommendations <https://indico.belle2.org/event/13722/contributions/84645/attachments/31512/51585/ecl_recommendation.pdf>`_ 
    slides.

)DOC","rad");
    REGISTER_VARIABLE("clusterR", eclClusterR, R"DOC(
Returns the distance of the cluster centroid from :math:`(0,0,0)`.

.. warning::
    This variable is not recommended for use in analyses as they are difficult to interpret. For more information please see the 
    `ECL Cluster Recommendations <https://indico.belle2.org/event/13722/contributions/84645/attachments/31512/51585/ecl_recommendation.pdf>`_ 
    slides.

)DOC","cm");
    REGISTER_VARIABLE("clusterPhi", eclClusterPhi, R"DOC(
Returns the azimuthal angle :math:`\phi` of the cluster. This is generally not equal 
to the azimuthal angle of the photon. 

.. topic:: Calculating Cluster Direction

    The direction of a cluster is given by the connecting line from :math:`\,(0,0,0)\,` to 
    cluster centroid position in the ECL. The centroid position is calculated using up to 21 crystals 
    (as 5x5 grid excluding corners) after the crystal energies are split in the case of overlapping clusters. 
    The centroid position is the logarithmically weighted average of all crystals evaluated at the 
    crystal centers. The centroid is generally biased towards the centers of the highest energetic 
    crystal. This effect is larger for low energy photons. Beam backgrounds slightly decrease the 
    position resolution, and mainly effects the low energy photons. 
    
    Unlike for charged tracks, the uncertainty (covariance) of the photon direction is not determined 
    based on individual cluster properties but taken from MC-based parametrizations of the resolution 
    as function of true photon energy, true photon direction and beam background level. 

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`-\pi`
    | - Upper limit: :math:`\pi`
    | - Precision: :math:`16` bit
..

)DOC","rad");
    REGISTER_VARIABLE("clusterConnectedRegionID", eclClusterConnectedRegionID, R"DOC(
Returns connected region ID for the cluster. 
)DOC");
    REGISTER_VARIABLE("clusterTheta", eclClusterTheta, R"DOC(
Returns the polar angle :math:`\theta` of the cluster. This is generally not equal 
to the polar angle of the photon. 

.. topic:: Calculating Cluster Direction

    The direction of a cluster is given by the connecting line from :math:`\,(0,0,0)\,` to 
    cluster centroid position in the ECL. The centroid position is calculated using up to 21 crystals 
    (as 5x5 grid excluding corners) after the crystal energies are split in the case of overlapping clusters. 
    The centroid position is the logarithmically weighted average of all crystals evaluated at the 
    crystal centers. The centroid is generally biased towards the centers of the highest energetic 
    crystal. This effect is larger for low energy photons. Beam backgrounds slightly decrease the 
    position resolution, and mainly effects the low energy photons. 
    
    Unlike for charged tracks, the uncertainty (covariance) of the photon direction is not determined 
    based on individual cluster properties but taken from MC-based parametrizations of the resolution 
    as function of true photon energy, true photon direction and beam background level. 

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`0.0`
    | - Upper limit: :math:`\pi`
    | - Precision: :math:`16` bit
..

)DOC","rad");
    REGISTER_VARIABLE("clusterTiming", eclClusterTiming, R"DOC(
Returns the time of the cluster. This is calculated **differently** in Belle and Belle II. Please 
read their definitions below. 

.. topic:: In Belle II

    It is calculated as the cluster time minus the `eventT0`. The cluster time is obtained by a fit to 
    the recorded waveform of the highest energy crystal in the cluster. For a cluster produced by a 
    particle from the IP, the cluster time should be consistent with `eventT0` within the uncertainties 
    following all calibrations and corrections. For MC, note that the calibrations and corrections are not 
    fully simulated. In order to see if the waveform fit fails, see `clusterHasFailedTiming`.    

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`-1000.0`
    | - Upper limit: :math:`1000.0`
    | - Precision: :math:`12` bit
..

.. topic:: In Belle

    It is equal to the trigger cell (TC) time corresponding to the cluster. This information is only 
    available in Belle data since experiment 31, and not available in Belle MC. Clusters produced at the IP 
    in time with the event have a TC time in the range of 9000 - 11000. 

.. note::
    | In case this variable is obtained from Belle data that is stored in Belle II mdst/udst format, it will be truncated to:
    | - Lower limit: :math:`-1000.0`
    | - Upper limit: :math:`1000.0`
    | - Precision: :math:`12` bit
..

)DOC","ns");
    REGISTER_VARIABLE("clusterHasFailedTiming", eclClusterHasFailedTiming, R"DOC(
Status bit indicating if the waveform fit for the `clusterTiming` calculation has failed. 
)DOC");
    REGISTER_VARIABLE("clusterErrorTiming", eclClusterErrorTiming, R"DOC(
Returns the cluster timing uncertainty which is equal to the :math:`dt99` value as defined below. Very large 
values of :math:`dt99` are an indication of a failed waveform fit for the cluster.  

The timing distribution for each cluster is non-Gaussian and so the value :math:`dt99` is stored where 
:math:`|\text{timing}| / \text{dt99} < 1` is designed to give a :math:`99\%` timing efficiency for 
true photons from the IP. (This definition results in an efficiency that is approximately flat in energy 
and independent of the beam background level). The :math:`dt99` value stored is determined using MC with a 
parametrisation that depends on the true energy deposition in the highest energetic crystal and the 
local beam background level in that crystal.  

.. warning::
    This variable should should only be used for relative timing selections as it is not 
    a directly-reconstructed quantity. For more information please see the 
    `ECL Cluster Recommendations <https://indico.belle2.org/event/13722/contributions/84645/attachments/31512/51585/ecl_recommendation.pdf>`_ 
    slides.

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`0.0`
    | - Upper limit: :math:`1000.0`
    | - Precision: :math:`12` bit

)DOC","ns");
    REGISTER_VARIABLE("clusterHasFailedErrorTiming", eclClusterHasFailedErrorTiming, R"DOC(
Status bit indicating if the calculation of `clusterErrorTiming` has failed due to a failed 
waveform fit. 
)DOC");
    REGISTER_VARIABLE("clusterHighestE", eclClusterHighestE, R"DOC(
Returns the energy of the highest energetic crystal in the cluster after reweighting. 

.. warning::
    This variable is not recommended for use in analyses as they are difficult to interpret. For more information please see the 
    `ECL Cluster Recommendations <https://indico.belle2.org/event/13722/contributions/84645/attachments/31512/51585/ecl_recommendation.pdf>`_ 
    slides.

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`-5` (:math:`e^{-5} = 0.00674\,` GeV)
    | - Upper limit: :math:`3.0` (:math:`e^3 = 20.08553\,` GeV)
    | - Precision: :math:`18` bit
..

)DOC","GeV");
    REGISTER_VARIABLE("clusterCellID", eclClusterCellId,
                      "Returns the cell ID of the crystal with highest energy in the cluster.");
    REGISTER_VARIABLE("clusterThetaID", eclClusterThetaId, R"DOC(
Returns the :math:`\theta` ID of the crystal with highest energy in the cluster. There are 
69 :math:`\theta` IDs in total covering the entire ECL acceptance. The mapping between crystal number 
and :math:`\theta` ID can be found in the 
`code definition <https://software.belle2.org/development/doxygen/ECLVariables_8cc_source.html#l00299>`_. 
)DOC");
    REGISTER_VARIABLE("clusterPhiID", eclClusterPhiId, R"DOC(
Returns the :math:`\phi` ID of the crystal with highest energy in the cluster.
)DOC");
    REGISTER_VARIABLE("clusterE1E9", eclClusterE1E9, R"DOC(
Returns the ratio of the energy in the central crystal (:math:`E_1`) to the total energy in the 
3x3 crystal grid around the central crystal (:math:`E_9`). Since :math:`E_1 \leq E_9`, this ratio is 
:math:`\leq 1` and tends towards larger values for photons and smaller values for hadrons. 

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`0.0`
    | - Upper limit: :math:`1.0`
    | - Precision: :math:`10` bit

)DOC");
    REGISTER_VARIABLE("clusterE9E25", eclClusterE9E25, R"DOC(
.. deprecated:: release-XX-XX-XX

    Returns `clusterE9E21`. Kept for backwards compatibility.   
)DOC");
    REGISTER_VARIABLE("clusterE9E21", eclClusterE9E21, R"DOC(
Returns the ratio of the total energy in the 3x3 crystal grid around the central 
crystal (:math:`E_9`) to the total energy in the 5x5 crystal grid around the central crystal 
excluding the corners (:math:`E_21`). Since :math:`E_9 \leq E_21`, this ratio is :math:`\leq 1` and tends towards larger 
values for photons and smaller values for hadrons. 

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`0.0`
    | - Upper limit: :math:`1.0`
    | - Precision: :math:`10` bit
    
)DOC");
    REGISTER_VARIABLE("clusterAbsZernikeMoment40", eclClusterAbsZernikeMoment40, R"DOC(
Returns absolute value of the 40th Zernike moment :math:`|Z_{40}|`. An explanation on this shower 
shape variable is in the description for the `clusterZernikeMVA` variable.

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`0.0`
    | - Upper limit: :math:`1.7`
    | - Precision: :math:`10` bit
)DOC");
    REGISTER_VARIABLE("clusterAbsZernikeMoment51", eclClusterAbsZernikeMoment51, R"DOC(
Returns absolute value of the 51st Zernike moment :math:`|Z_{51}|`. An explanation on this shower 
shape variable is in the description for the `clusterZernikeMVA` variable.

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`0.0`
    | - Upper limit: :math:`1.2`
    | - Precision: :math:`10` bit
)DOC");
    REGISTER_VARIABLE("clusterZernikeMVA", eclClusterZernikeMVA, R"DOC(
Returns output of an MVA trained to use eleven Zernike moments of the cluster. 

Zernike moments are calculated for each shower in the plane perpendicular to 
the shower direction via 

.. math::
    |Z_{nm}| = \frac{n+1}{\pi} \frac{1}{\sum_{i} w_{i} E_{i}} \left|\sum_{i} R_{nm}(\rho_{i}) e^{-im\alpha_{i}} w_{i} E_{i} \right|

where :math:`n, m` are the integers, :math:`i` runs over each crystal in the shower,
:math:`E_{i}` is the energy of the i-th crystal in the shower,
:math:`R_{nm}` is a polynomial of degree :math:`n`,
:math:`\rho_{i}` is the radial distance of the :math:`i`-th crystal in the perpendicular plane,
and :math:`\alpha_{i}` is the polar angle of the :math:`i`-th crystal in the perpendicular plane.
As a crystal can be related to more than one shower, :math:`w_{i}` is the fraction of the
energy of the :math:`i`-th crystal associated with the shower.

.. caution::
    This variable is sensitive to other nearby particles and so cluster isolation properties shoud 
    always be checked. For more information please see the 
    `ECL Cluster Recommendations <https://indico.belle2.org/event/13722/contributions/84645/attachments/31512/51585/ecl_recommendation.pdf>`_ 
    slides.

.. seealso::
    - More details about the implementation can be found in `BELLE2-NOTE-TE-2017-001 <https://docs.belle2.org/record/454?ln=en>`_ .
    - More details about Zernike polynomials can be found on `Wikipedia <https://en.wikipedia.org/wiki/Zernike_polynomials>`_ .

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`0.0`
    | - Upper limit: :math:`1.0`
    | - Precision: :math:`10` bit
)DOC");
    REGISTER_VARIABLE("clusterSecondMoment", eclClusterSecondMoment, R"DOC(
Returns the second moment :math:`S` of the cluster. This is mainly implemented for reconstructing high a 
energy :math:`\pi^0` originating from merged ECL clusters. 

It is defined as:

.. math::
    S = \frac{1}{S_{0}(\theta)}\frac{\sum_{i=0}^{n} w_{i} E_{i} r^2_{i}}{\sum_{i=0}^{n} w_{i} E_{i}}

where :math:`E_{i} = (E_0, E_1, ...)` are the crystal energies sorted by descending energy, :math:`w_{i}` is
the fraction of the crystal energy associated with the shower, and :math:`r_{i}` is the distance of 
the :math:`i`-th digit to the shower center projected onto a plane perpendicular to the 
shower axis. :math:`S_{0}(\theta)` normalizes :math:`S` to 1 for isolated photons.

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`0.0`
    | - Upper limit: :math:`40.0`
    | - Precision: :math:`10` bit
..

)DOC","dimensionless");
    REGISTER_VARIABLE("clusterLAT", eclClusterLAT, R"DOC(
Returns lateral energy distribution of the shower. For radially-symmetric electromagnetic showers, this 
variable peaks at :math:`\approx 0.3`. It is larger for hadronic particles and electrons with a nearby relative 
or Bremsstrahlung photon. 

It is defined as following:

.. math::
    S = \frac{\sum_{i=2}^{n} w_{i} E_{i} r^2_{i}}{(w_{0} E_{0} + w_{1} E_{1}) r^2_{0} + \sum_{i=2}^{n} w_{i} E_{i} r^2_{i}}

where :math:`E_{i} = (E_0, E_1, ...)` are the crystal energies sorted by descending energy, :math:`w_{i}` is
the fraction of the crystal energy associated with the shower, :math:`r_{i}` is the distance of 
the :math:`i`-th digit to the shower center projected onto a plane perpendicular to the shower axis,
and :math:`r_{0} \approx 5\,cm` is the average distance between two crystal centres.

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`0.0`
    | - Upper limit: :math:`1.0`
    | - Precision: :math:`10` bit
)DOC");
    REGISTER_VARIABLE("clusterNHits", eclClusterNHits, R"DOC(
Returns sum of weights :math:`w_{i}` (:math:`w_{i} \leq 1`) of all crystals in the cluster.
For non-overlapping clusters this is equal to the number of crystals in the cluster. However, in the case 
of overlapping clusters where individual crystal energies are split among them, this can be a non-integer value. 

.. tip::
    If fractional weights are not of interest, this value should be cast to the nearest integer

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`0.0`
    | - Upper limit: :math:`200.0`
    | - Precision: :math:`10` bit

)DOC");
    REGISTER_VARIABLE("clusterTrackMatch", eclClusterTrackMatched, R"DOC(
Returns 1.0 if at least one reconstructed charged track is matched to the cluster. Track-cluster 
matching is briefly described below. 

Every reconstructed track is extrapolated to the ECL. Every ECL crystal that is crossed by 
the extrapolated track is marked. Following this, every cluster that contains a marked crystal 
becomes associated with the original track. A cluster can only have one matched track 
but multiple tracks can be matched to the same cluster. In the latter case, all tracks matched 
to the same cluster will return the same cluster variables e.g. all will have the same `clusterE`. 
)DOC");
    REGISTER_VARIABLE("nECLClusterTrackMatches", nECLClusterTrackMatches, R"DOC(
Returns the number of tracks matched to the cluster. For more information on track-cluster matching 
please see the description for `clusterTrackMatch`. 

For the clusters of neutral particles, this should always return 0. If there is no cluster ``NaN`` is 
returned. 
)DOC");
    REGISTER_VARIABLE("clusterHasPulseShapeDiscrimination", eclClusterHasPulseShapeDiscrimination, R"DOC(
Status bit to indicate if the waveforms from the cluster pass the requirements for computing pulse 
shape discrimination variables such as `clusterPulseShapeDiscriminationMVA`. 

.. topic:: Pulse Shape Information Requirements
    
    The exact requirements for a cluster have pulse shape information are as follows:
    - there is at least one crystal with energy above 30 MeV for phase 2 data or 50 MeV for phase 3 data 
    (this is the energy threshold requirement for saving waveforms offline)
    - one of the waveforms for the cluster must have a good :math:`\chi^2` fit 

)DOC");
    REGISTER_VARIABLE("beamBackgroundSuppression", beamBackgroundSuppression, R"DOC(
Returns the output of an MVA classifier that uses shower-related variables to distinguish true photon clusters from beam background clusters.
Class 1 is for true photon clusters while class 0 is for beam background clusters.

The MVA has been trained using MC and the features used, in decreasing importance, are:

- `clusterTiming`
- `clusterPulseShapeDiscriminationMVA`
- `clusterE`
- `clusterTheta`
- `clusterZernikeMVA` (this has been removed starting from the MC16 training)

.. seealso::

    For the correct usage, please see 
    the `Performance Recommendations Webpage <https://belle2.pages.desy.de/performance/recommendations/>`_.

.. important::

    Please cite `this proceeding <https://inspirehep.net/literature/2785196>`_ if using this tool. 
)DOC");
    REGISTER_VARIABLE("fakePhotonSuppression", fakePhotonSuppression, R"DOC(
Returns the output of an MVA classifier that uses shower-related variables to distinguish true photon clusters from fake photon clusters. 
Class 1 is for true photon clusters while class 0 is for fake photon clusters. 

The MVA has been trained using MC and the features are:

- `clusterPulseShapeDiscriminationMVA`
- `minC2TDist`
- `clusterE`
- `clusterTiming`
- `clusterTheta`
- `clusterZernikeMVA` (this has been removed starting from the MC16 training)

.. seealso::
    For the correct usage, please see 
    the `Performance Recommendations Webpage <https://belle2.pages.desy.de/performance/recommendations/>`_.

.. important::
    Please cite `this proceeding <https://inspirehep.net/literature/2785196>`_ if using this tool. 

)DOC");
    REGISTER_VARIABLE("hadronicSplitOffSuppression", hadronicSplitOffSuppression, R"DOC(
.. deprecated:: light-2302-genetta

    The training for this variable has been not been updated since MC14. 
    Please use `fakePhotonSuppression` instead which is an improved and up-to-date version of this MVA. 

)DOC");
    REGISTER_VARIABLE("clusterPulseShapeDiscriminationMVA", eclPulseShapeDiscriminationMVA, R"DOC(
Returns the output of an MVA classifier that uses pulse shape information to discriminate between electromagnetic 
and hadronic showers. Class 1 is for electromagnetic showers while class 0 is for hadronic showers. 

.. important::
    Please cite `this paper <https://inspirehep.net/literature/1807894>`_ if using this tool. 

)DOC");
    REGISTER_VARIABLE("clusterNumberOfHadronDigits", eclClusterNumberOfHadronDigits, R"DOC(
Returns the number of hadron digits in the cluster based on pulse shape information. This is the weight sum of cluster digits 
that have :math:`> 3\,` MeV for the hadronic scintillation component. The digits must have pulse shape information available 
(see `clusterHasPulseShapeDiscrimination` for more information). 

.. warning::
    This is a purely technical flag and should not be used for any physics-level selection

.. note::
    | Please read `this <importantNoteECL>` first.
    | - Lower limit: :math:`0.0`
    | - Upper limit: :math:`255.0`
    | - Precision: :math:`18` bit
)DOC");
    REGISTER_VARIABLE("clusterClusterID", eclClusterId, R"DOC(
Returns the ID the cluster within the connected region to which it belongs. 
)DOC");
    REGISTER_VARIABLE("clusterHasNPhotons", eclClusterHasNPhotonsHypothesis, R"DOC(
Returns 1.0 if the cluster has the "N photons" hypothesis (historically called "N1"),
0.0 if not, and ``NaN`` if no cluster is associated to the particle.
)DOC");
    REGISTER_VARIABLE("clusterHasNeutralHadron", eclClusterHasNeutralHadronHypothesis, R"DOC(
Returns 1.0 if the cluster has the "neutral hadrons" hypothesis (historically called "N2"),
0.0 if not, and ``NaN`` if no cluster is associated to the particle.
)DOC");
    REGISTER_VARIABLE("eclExtTheta", eclExtTheta, R"DOC(
Returns the :math:`\theta` angle of the extrapolated track associated to the cluster (if any). 

.. warning::
    This requires the ``ECLTrackCalDigitMatch`` module to be executed.

)DOC","rad");
    REGISTER_VARIABLE("eclExtPhi", eclExtPhi, R"DOC(
Returns the :math:`\phi` angle of the extrapolated track associated to the cluster (if any). 

.. warning::
    This requires the ``ECLTrackCalDigitMatch`` module to be executed.

)DOC","rad");
    REGISTER_VARIABLE("eclExtPhiId", eclExtPhiId, R"DOC(
Returns the :math:`\phi` ID of the extrapolated track associated to the cluster (if any). 

.. warning::
    This requires the ``ECLTrackCalDigitMatch`` module to be executed.

)DOC");
    REGISTER_VARIABLE("weightedAverageECLTime", weightedAverageECLTime, R"DOC(
Returns the weighted average time of all clusters corresponding to daughter particles of the provided particle. 

)DOC", "ns");
    REGISTER_VARIABLE("maxWeightedDistanceFromAverageECLTime", maxWeightedDistanceFromAverageECLTime, R"DOC(
Returns maximum weighted distance between time of the cluster of a photon and the ECL average time, amongst
the clusters (neutrals) and matched clusters (charged) of daughters (of all generations) of the provided particle.

)DOC", "ns");
    REGISTER_VARIABLE("clusterMdstIndex", eclClusterMdstIndex, R"DOC(
Returns the ``StoreArray`` index of the ECL cluster mDST object. This can be useful for track-based particles that are matched to an ECL cluster. 
)DOC");

    REGISTER_VARIABLE("nECLOutOfTimeCrystals", nECLOutOfTimeCrystals, R"DOC(
**[Event-based]** Returns the number of crystals that are out of time with the `eventT0` by more than 110.0 ns. Only crystals with an energy greater than 
7 MeV are are counted.  
)DOC");

    REGISTER_VARIABLE("nECLOutOfTimeCrystalsFWDEndcap", nECLOutOfTimeCrystalsFWDEndcap, R"DOC(
**[Event-based]** Returns the number of crystals in the forward endcap that are out of time with the `eventT0` by more than 110.0 ns. Only crystals with an energy greater than 
7 MeV are are counted.  
)DOC");

    REGISTER_VARIABLE("nECLOutOfTimeCrystalsBarrel", nECLOutOfTimeCrystalsBarrel, R"DOC(
**[Event-based]** Returns the number of crystals in the barrel that are out of time with the `eventT0` by more than 110.0 ns. Only crystals with an energy greater than 
7 MeV are are counted.  
)DOC");

    REGISTER_VARIABLE("nECLOutOfTimeCrystalsBWDEndcap", nECLOutOfTimeCrystalsBWDEndcap, R"DOC(
**[Event-based]** Returns the number of crystals in the backward endcap that are out of time with the `eventT0` by more than 110.0 ns. Only crystals with an energy greater than 
7 MeV are are counted.  
)DOC");

    REGISTER_VARIABLE("nRejectedECLShowers", nRejectedECLShowers, R"DOC(
**[Event-based]** Returns the number of ECL showers that do not become clusters. If the number exceeds 255, the variable is set to 255.
)DOC");

    REGISTER_VARIABLE("nRejectedECLShowersFWDEndcap", nRejectedECLShowersFWDEndcap, R"DOC(
**[Event-based]** Returns the number of ECL showers in the forward endcap that do not become clusters.
If the number exceeds 255, the variable is set to 255.
)DOC");

    REGISTER_VARIABLE("nRejectedECLShowersBarrel", nRejectedECLShowersBarrel, R"DOC(
**[Event-based]** Returns the number of ECL showers in the barrel that do not become clusters.
If the number exceeds 255, the variable is set to 255.
)DOC");

    REGISTER_VARIABLE("nRejectedECLShowersBWDEndcap", nRejectedECLShowersBWDEndcap, R"DOC(
**[Event-based]** Returns the number of ECL showers in the backward endcap that do not become clusters.
If the number exceeds 255, the variable is set to 255.
)DOC");
  
  REGISTER_VARIABLE("nKLMMultistripHitsFWDEndcap", nKLMMultistripHitsFWDEndcap, R"DOC(
**[Event-based]** Returns the number of multi-strip hits in the KLM forward endcap associated with the ECL cluster.
)DOC");
  
  REGISTER_VARIABLE("nKLMMultistripHitsBarrel", nKLMMultistripHitsBarrel, R"DOC(
**[Event-based]** Returns the number of multi-strip hits in the KLM barrel associated with the ECL cluster.
)DOC");
  
  REGISTER_VARIABLE("nKLMMultistripHitsBWDEndcap", nKLMMultistripHitsBWDEndcap, R"DOC(
**[Event-based]** Returns the number of multi-strip hits in the KLM backward endcap associated with the ECL cluster.
)DOC");
  
  REGISTER_VARIABLE("nKLMMultistripHits", nKLMMultistripHits, R"DOC(
**[Event-based]** Returns the number of multi-strip hits in the KLM associated with the ECL cluster.
)DOC");
  
  REGISTER_VARIABLE("nECLShowersFWDEndcap", nECLShowersFWDEndcap, R"DOC(
**[Event-based]** Returns the number of ECL showers in the forward endcap.
)DOC");
  
  REGISTER_VARIABLE("nECLShowersBarrel", nECLShowersBarrel, R"DOC(
**[Event-based]** Returns the number of ECL showers in the barrel.
)DOC");
  
  REGISTER_VARIABLE("nECLShowersBWDEndcap", nECLShowersBWDEndcap, R"DOC(
**[Event-based]** Returns the number of ECL showers in the backward endcap.
)DOC");

  REGISTER_VARIABLE("nECLShowers", nECLShowers, R"DOC(
**[Event-based]** Returns the number of ECL showers.
)DOC");
  
  REGISTER_VARIABLE("nECLLocalMaximumsFWDEndcap", nECLLocalMaximumsFWDEndcap, R"DOC(
**[Event-based]** Returns the number of Local Maximums in the ECL forward endcap.
)DOC");
  
  REGISTER_VARIABLE("nECLLocalMaximumsBarrel", nECLLocalMaximumsBarrel, R"DOC(
**[Event-based]** Returns the number of Local Maximums in the ECL barrel.
)DOC");
  
  REGISTER_VARIABLE("nECLLocalMaximumsBWDEndcap", nECLLocalMaximumsBWDEndcap, R"DOC(
**[Event-based]** Returns the number of Local Maximums in the ECL backward endcap.
)DOC");

  REGISTER_VARIABLE("nECLLocalMaximums", nECLLocalMaximums, R"DOC(
**[Event-based]** Returns the number of Local Maximums in the ECL.
)DOC");
  
  REGISTER_VARIABLE("nECLTriggerCellsFWDEndcap", nECLTriggerCellsFWDEndcap, R"DOC(
**[Event-based]** Returns the number of ECL trigger cells above 100 MeV in the forward endcap.
)DOC");
  
  REGISTER_VARIABLE("nECLTriggerCellsBarrel", nECLTriggerCellsBarrel, R"DOC(
**[Event-based]** Returns the number of ECL trigger cells above 100 MeV in the barrel.
)DOC");
  
  REGISTER_VARIABLE("nECLTriggerCellsBWDEndcap", nECLTriggerCellsBWDEndcap, R"DOC(
**[Event-based]** Returns the number of ECL trigger cells above 100 MeV in the backward endcap.
)DOC");

  REGISTER_VARIABLE("nECLTriggerCells", nECLTriggerCells, R"DOC(
**[Event-based]** Returns the number of ECL trigger cells above 100 MeV.
)DOC");

  REGISTER_VARIABLE("eclClusterOnlyInvariantMass", eclClusterOnlyInvariantMass, R"DOC(
Returns the invariant mass calculated from all neutral and track-matched cluster daughters using the cluster 4-momenta. 
This is primarily used for ECL-based dark sector physics and debugging track-cluster matching.

)DOC","GeV/:math:`\\text{c}^2`");

  REGISTER_METAVARIABLE("photonHasOverlap(cutString, photonlistname, tracklistname)", photonHasOverlap, R"DOC(
Returns ``True`` if the connection region of the cluster is shared by another cluster. Both neutral and track-matched clusters 
are considered. 

A ``cutString`` can be provided in order to ignore clusters that do not satisfy a given criteria. By default, the particle lists  
``gamma:all`` and ``e-:all`` are used to check for neutral and track-matched clusters respectively. However, this can be customised 
by using the ``photonlistname`` and ``tracklistname`` parameters. If ``gamma:all`` or ``e-:all`` does not exist or if this variable is applied 
to a particle that is not a photon, ``NaN`` will be returned. 
)DOC", Manager::VariableDataType::c_double);

  REGISTER_VARIABLE("clusterUncorrE", eclClusterUncorrectedE, R"DOC(
Returns the uncorrected energy of the cluster. 

.. danger:: 
    This variable should not be used for any physics analysis but only for ECL- or calibration-based studies. 

)DOC","GeV");

  REGISTER_VARIABLE("distanceToMcKl",distanceToMcKl,R"DOC(
Returns the distance to the nearest truth-matched :math:`K_L^0` particle that has been extrapolated to the cluster. 

.. warning::
    This requires the `getNeutralHadronGeomMatches` function to be used.

)DOC", "cm");

  REGISTER_VARIABLE("distanceToMcNeutron",distanceToMcNeutron,R"DOC(
Returns the distance to the nearest truth-matched (anti)neutron particle that has been extrapolated to the cluster. 

.. warning::
    This requires the `getNeutralHadronGeomMatches` function to be used.

)DOC", "cm");

  REGISTER_VARIABLE("mdstIndexMcKl",mdstIndexMcKl,R"DOC(
Returns the ``StoreArray`` index of the mDST object corresponding to the nearest truth-matched :math:`K_L^0` particle as determined 
during the calculation of the `distanceToMcKl` variable. 

.. warning::
    This requires the `getNeutralHadronGeomMatches` function to be used.

)DOC");

  REGISTER_VARIABLE("mdstIndexMcNeutron",mdstIndexMcNeutron,R"DOC(
Returns the ``StoreArray`` index of the mDST object corresponding to the nearest truth-matched (anti)neutron particle as determined 
during the calculation of the `distanceToMcKl` variable. 

.. warning::
    This requires the `getNeutralHadronGeomMatches` function to be used.
)DOC");
  REGISTER_VARIABLE("clusterTimeNorm90", eclClusterTimeNorm90,
  R"DOC(
  Returns ECL cluster's timing normalized such that :math:`90\%` of real photons will 
  have :math:`|\text{clusterTimeNorm90}| < 1`. Normalization depends on energy, background
  level, and cellID, and differs for data and MC. Valid only for crystals within the CDC acceptance, :math:`161 <= |\text{clusterCellID}| <= 8608`. Note: the required payloads are stored in the neutrals global tag. Please find the latest recommendation using :ref:`b2help-recommendation`.)DOC",
                      "dimensionless");

  }
}
