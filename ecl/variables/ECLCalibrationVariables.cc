/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <ecl/variables/ECLCalibrationVariables.h>

// analysis
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ECLEnergyCloseToTrack.h>
#include <analysis/dataobjects/ECLTRGInformation.h>
#include <analysis/dataobjects/ECLTriggerCell.h>
#include <analysis/utility/ReferenceFrame.h>
#include <analysis/ClusterUtility/ClusterUtils.h>
#include <analysis/VariableManager/Utility.h>

// framework
#include <framework/core/Module.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>

// dataobjects
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>

using namespace std;

namespace Belle2 {

  namespace Variable {

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

    double eclNumberOfTCsForCluster(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 4) {
        B2FATAL("Need exactly four parameters (minthetaid, maxthetaid, minhitwindow, maxhitwindow).");
      }

      // if we did not run the ECLTRGInformation module, return NaN
      // cppcheck-suppress unassignedVariable
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

      // if everything else is fine, but we don't have a cluster, return 0
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
      // cppcheck-suppress unassignedVariable
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

      // if everything else is fine, but we don't have a cluster, return 0
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

      // if everything else is fine, but we don't have a cluster, return 0
      if (cluster) {
        auto relationsTCs = cluster->getRelationsTo<ECLTriggerCell>();
        for (unsigned int idxTC = 0; idxTC < relationsTCs.size(); ++idxTC) {
          const auto tc = relationsTCs.object(idxTC);
          if (tc->isHighestFADC()) result = 1.0;
        }
      }
      return result;
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
          return std::numeric_limits<float>::quiet_NaN();
        }
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double getEnergyTC(const Particle*, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameter (tcid).");
      }

      StoreObjPtr<ECLTRGInformation> tce;
      const int tcid = int(std::lround(vars[0]));

      if (!tce) return std::numeric_limits<double>::quiet_NaN();
      return tce->getEnergyTC(tcid);
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

    double getTimingTC(const Particle*, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameter (tcid).");
      }

      StoreObjPtr<ECLTRGInformation> tce;
      const int tcid = int(std::lround(vars[0]));

      if (!tce) return std::numeric_limits<double>::quiet_NaN();
      return tce->getTimingTC(tcid);
    }

    double eclHitWindowTC(const Particle*, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameter (tcid).");
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

    // These variables require cDST inputs and the eclTrackCalDigitMatch module run first
    VARIABLE_GROUP("ECL calibration");

    REGISTER_VARIABLE("eclEnergy3FWDBarrel", eclEnergy3FWDBarrel, R"DOC(
[Calibration] Returns energy sum of three crystals in forward barrel.
)DOC", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclEnergy3FWDEndcap", eclEnergy3FWDEndcap, R"DOC(
[Calibration] Returns energy sum of three crystals in forward endcap.
)DOC", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclEnergy3BWDBarrel", eclEnergy3BWDBarrel, R"DOC(
[Calibration] Returns energy sum of three crystals in backward barrel.
)DOC", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclEnergy3BWDEndcap", eclEnergy3BWDEndcap, R"DOC(
[Calibration] Returns energy sum of three crystals in backward endcap.
)DOC", Manager::VariableDataType::c_double);

    // These variables require cDST inputs and the eclTRGInformation module run first
    VARIABLE_GROUP("ECL trigger calibration");

    REGISTER_VARIABLE("clusterNumberOfTCs(i, j, k, l)", eclNumberOfTCsForCluster, R"DOC(
[Calibration] Returns the number of TCs for this ECL cluster for a given TC theta ID range
:math:`(i, j)` and hit window :math:`(k, l)`.
)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("clusterTCFADC(i, j, k, l)", eclTCFADCForCluster, R"DOC(
[Calibration] Returns the total FADC sum related to this ECL cluster for a given TC theta ID
range :math:`(i, j)` and hit window :math:`(k, l)`.
)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("clusterTCIsMaximum", eclTCIsMaximumForCluster, R"DOC(
[Calibration] Returns True if cluster is related to maximum TC.
)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("clusterTrigger", eclClusterTrigger, R"DOC(
[Calibration] Returns 1.0 if ECL cluster is matched to a trigger cluster (requires to run eclTriggerClusterMatcher
(which requires TRGECLClusters in the input file)) and 0 otherwise. Returns -1 if the matching code was not run.
NOT FOR PHASE2 DATA!
)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclEnergyTC(i)", getEnergyTC, R"DOC(
[Eventbased][Calibration] Returns the energy (in FADC counts) for the :math:`i`-th trigger cell (TC), 1 based (1..576).
)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclEnergyTCECLCalDigit(i)", getEnergyTCECLCalDigit, R"DOC(
[Eventbased][Calibration] Returns the energy (in GeV) for the :math:`i`-th trigger cell (TC)
based on ECLCalDigits, 1 based (1..576).
)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclTimingTC(i)", getTimingTC, R"DOC(
[Eventbased][Calibration] Returns the time (in ns) for the :math:`i`-th trigger cell (TC), 1 based (1..576).
)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclHitWindowTC(i)", eclHitWindowTC, R"DOC(
[Eventbased][Calibration] Returns the hit window for the :math:`i`-th trigger cell (TC), 1 based (1..576).
)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclEventTimingTC", getEvtTimingTC, R"DOC(
[Eventbased][Calibration] Returns the ECL TC event time (in ns).
)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclMaximumTCId", getMaximumTCId, R"DOC(
[Eventbased][Calibration] Returns the TC ID with maximum FADC value.
)DOC", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclTimingTCECLCalDigit(i)", getTimingTCECLCalDigit, R"DOC(
[Eventbased][Calibration] Returns the time (in ns) for the :math:`i`-th trigger cell (TC) based
on ECLCalDigits, 1 based (1..576)
)DOC", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclNumberOfTCs(i, j, k)", getNumberOfTCs, R"DOC(
[Eventbased][Calibration] Returns the number of TCs above threshold (i=FADC counts) for this event
for a given theta range (j-k)
)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclEnergySumTC(i, j)", eclEnergySumTC, R"DOC(
[Eventbased][Calibration] Returns energy sum (in FADC counts) of all TC cells between two
theta ids i<=thetaid<=j, 1 based (1..17)
)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclEnergySumTCECLCalDigit(i, j, k, l)", eclEnergySumTCECLCalDigit, R"DOC(
[Eventbased][Calibration] Returns energy sum (in GeV) of all TC cells between two theta ids i<=thetaid<=j,
1 based (1..17). k is the sum option: 0 (all), 1 (those with actual TC entries), 2 (sum of ECLCalDigit energy
in this TC above threshold). l is the threshold parameter for the option k 2.
)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclEnergySumTCECLCalDigitInECLCluster", eclEnergySumTCECLCalDigitInECLCluster, R"DOC(
[Eventbased][Calibration] Returns energy sum (in GeV) of all ECLCalDigits if TC is above threshold
that are part of an ECLCluster above eclEnergySumTCECLCalDigitInECLClusterThreshold within TC thetaid 2-15.
)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclEnergySumECLCalDigitInECLCluster", eclEnergySumECLCalDigitInECLCluster, R"DOC(
[Eventbased][Calibration] Returns energy sum (in GeV) of all ECLCalDigits that are part of an ECL cluster
above eclEnergySumTCECLCalDigitInECLClusterThreshold within TC thetaid 2-15.
)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclEnergySumTCECLCalDigitInECLClusterThreshold", eclEnergySumTCECLCalDigitInECLClusterThreshold, R"DOC(
[Eventbased][Calibration] Returns threshold used to calculate eclEnergySumTCECLCalDigitInECLCluster.
)DOC", Manager::VariableDataType::c_double);

  }
}
