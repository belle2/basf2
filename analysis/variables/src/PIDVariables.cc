/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc, Umberto Tamponi               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/variables/PIDVariables.h>

#include <analysis/VariableManager/Manager.h>

#include <mdst/dataobjects/PIDLikelihood.h>

// framework aux
#include <framework/logging/Logger.h>

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {



    //*************
    // Utilities
    //*************

    // converts Belle numbering scheme for charged final state particles
    // to Belle II ChargedStable
    Const::ChargedStable hypothesisConversion(const int hypothesis)
    {
      switch (hypothesis) {
        case 0:
          return Const::electron;
        case 1:
          return Const::muon;
        case 2:
          return Const::pion;
        case 3:
          return Const::kaon;
        case 4:
          return Const::proton;
      }

      return Const::pion;
    }

    // Parses the detector list for the PID metafunctions
    Const::PIDDetectorSet parseDetectors(const std::vector<std::string>& arguments)
    {
      Const::PIDDetectorSet result;
      for (std::string val : arguments) {
        boost::to_lower(val);
        if (val == "all") return Const::SVD + Const::CDC + Const::TOP + Const::ARICH + Const::ECL + Const::KLM;
        else if (val == "svd") result += Const::SVD;
        else if (val == "cdc") result += Const::CDC;
        else if (val == "top") result += Const::TOP;
        else if (val == "arich") result += Const::ARICH;
        else if (val == "ecl") result += Const::ECL;
        else if (val == "klm") result += Const::KLM;
        else B2ERROR("Unknown detector component: " << val);
      }
      return result;
    }



    //*************
    // Belle II
    //*************

    // a "smart" variable:
    // finds the global probability based on the PDG code of the input particle
    double particleID(const Particle* p)
    {
      int pdg = abs(p->getPDGCode());
      if (pdg == Const::electron.getPDGCode())      return electronID(p);
      else if (pdg == Const::muon.getPDGCode())     return muonID(p);
      else if (pdg == Const::pion.getPDGCode())     return pionID(p);
      else if (pdg == Const::kaon.getPDGCode())     return kaonID(p);
      else if (pdg == Const::proton.getPDGCode())   return protonID(p);
      else if (pdg == Const::deuteron.getPDGCode()) return deuteronID(p);
      else return std::numeric_limits<float>::quiet_NaN();
    }

    Manager::FunctionPtr pidLogLikelihoodValueExpert(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 2) {
        B2ERROR("Need at least two arguments to pidLogLikelihoodValueExpert");
        return nullptr;
      }
      int pdgCode;
      try {
        pdgCode = std::stoi(arguments[0]);
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidLogLikelihoodValueExpert must be a PDG code");
        return nullptr;
      }
      std::vector<std::string> detectors(arguments.begin() + 1, arguments.end());

      Const::PIDDetectorSet detectorSet = parseDetectors(detectors);
      auto hypType = Const::ChargedStable(abs(pdgCode));

      auto func = [hypType, detectorSet](const Particle * part) -> double {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid)
          return std::numeric_limits<float>::quiet_NaN();
        // No informaiton form any subdetector in the list
        if (pid->getLogL(hypType, detectorSet) == 0)
          return std::numeric_limits<float>::quiet_NaN();

        return pid->getLogL(hypType, detectorSet);
      };
      return func;
    }



    Manager::FunctionPtr pidDeltaLogLikelihoodValueExpert(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 3) {
        B2ERROR("Need at least three arguments to pidDeltaLogLikelihoodValueExpert");
        return nullptr;
      }
      int pdgCodeHyp, pdgCodeTest;
      try {
        pdgCodeHyp = std::stoi(arguments[0]);
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidDeltaLogLikelihoodValueExpert must be a PDG code");
        return nullptr;
      }
      try {
        pdgCodeTest = std::stoi(arguments[1]);
      } catch (std::invalid_argument& e) {
        B2ERROR("Second argument of pidDeltaLogLikelihoodValueExpert must be a PDG code");
        return nullptr;
      }

      std::vector<std::string> detectors(arguments.begin() + 2, arguments.end());
      Const::PIDDetectorSet detectorSet = parseDetectors(detectors);
      auto hypType = Const::ChargedStable(abs(pdgCodeHyp));
      auto testType = Const::ChargedStable(abs(pdgCodeTest));

      auto func = [hypType, testType, detectorSet](const Particle * part) -> double {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid) return std::numeric_limits<float>::quiet_NaN();
        // No informaiton form any subdetector in the list
        if (pid->getLogL(hypType, detectorSet) == 0)
          return std::numeric_limits<float>::quiet_NaN();

        return (pid->getLogL(hypType, detectorSet) - pid->getLogL(testType, detectorSet));
      };
      return func;
    }


    Manager::FunctionPtr pidPairProbabilityExpert(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 3) {
        B2ERROR("Need at least three arguments to pidPairProbabilityExpert");
        return nullptr;
      }
      int pdgCodeHyp = 0, pdgCodeTest = 0;
      try {
        pdgCodeHyp = std::stoi(arguments[0]);
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidPairProbabilityExpert must be PDG code");
        return nullptr;
      }
      try {
        pdgCodeTest = std::stoi(arguments[1]);
      } catch (std::invalid_argument& e) {
        B2ERROR("Second argument of pidPairProbabilityExpert must be PDG code");
        return nullptr;
      }

      std::vector<std::string> detectors(arguments.begin() + 2, arguments.end());

      Const::PIDDetectorSet detectorSet = parseDetectors(detectors);
      auto hypType = Const::ChargedStable(abs(pdgCodeHyp));
      auto testType = Const::ChargedStable(abs(pdgCodeTest));
      auto func = [hypType, testType, detectorSet](const Particle * part) -> double {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid) return std::numeric_limits<float>::quiet_NaN();
        // No informaiton form any subdetector in the list
        if (pid->getLogL(hypType, detectorSet) == 0)
          return std::numeric_limits<float>::quiet_NaN();

        return pid->getProbability(hypType, testType, detectorSet);
      };
      return func;
    }


    Manager::FunctionPtr pidProbabilityExpert(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 2) {
        B2ERROR("Need at least two arguments for pidProbabilityExpert");
        return nullptr;
      }
      int pdgCodeHyp = 0;
      try {
        pdgCodeHyp = std::stoi(arguments[0]);
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidProbabilityExpert must be PDG code");
        return nullptr;
      }

      std::vector<std::string> detectors(arguments.begin() + 1, arguments.end());
      Const::PIDDetectorSet detectorSet = parseDetectors(detectors);
      auto hypType = Const::ChargedStable(abs(pdgCodeHyp));

      // Placeholder for the priors
      const unsigned int n = Const::ChargedStable::c_SetSize;
      double frac[n];
      for (double& i : frac) i = 1.0;  // flat priors

      auto func = [hypType, frac, detectorSet](const Particle * part) -> double {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid) return std::numeric_limits<float>::quiet_NaN();
        // No informaiton form any subdetector in the list
        if (pid->getLogL(hypType, detectorSet) == 0)
          return std::numeric_limits<float>::quiet_NaN();

        return pid->getProbability(hypType, frac, detectorSet);
      };
      return func;
    }


    Manager::FunctionPtr pidMissingProbabilityExpert(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 1) {
        B2ERROR("Need at least one argument to pidMissingProbabilityExpert");
        return nullptr;
      }

      std::vector<std::string> detectors(arguments.begin(), arguments.end());
      Const::PIDDetectorSet detectorSet = parseDetectors(detectors);

      auto func = [detectorSet](const Particle * part) -> int {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (not pid->isAvailable(detectorSet))
          return 1;
        else return 0;
      };
      return func;
    }

    double electronID(const Particle* part)
    {
      return Manager::Instance().getVariable("pidProbabilityExpert(11, ALL)")->function(part);
    }

    double muonID(const Particle* part)
    {
      return Manager::Instance().getVariable("pidProbabilityExpert(13, ALL)")->function(part);
    }

    double pionID(const Particle* part)
    {
      return Manager::Instance().getVariable("pidProbabilityExpert(211, ALL)")->function(part);
    }

    double kaonID(const Particle* part)
    {
      return Manager::Instance().getVariable("pidProbabilityExpert(321, ALL)")->function(part);
    }

    double protonID(const Particle* part)
    {
      return Manager::Instance().getVariable("pidProbabilityExpert(2212, ALL)")->function(part);
    }

    double deuteronID(const Particle* part)
    {
      return Manager::Instance().getVariable("pidProbabilityExpert(1000010020, ALL)")->function(part);
    }

    Manager::FunctionPtr pidChargedBDTScore(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1) {
        B2ERROR("Need exactly one argument for pidChargedBDTScore: pdgCodeHyp");
        return nullptr;
      }

      auto pdgCodeHyp(arguments.at(0));

      auto func = [pdgCodeHyp](const Particle * part) -> double {
        auto name = "pidChargedBDTScore_" + pdgCodeHyp;
        return (part->hasExtraInfo(name)) ? part->getExtraInfo(name) : std::numeric_limits<float>::quiet_NaN();
      };
      return func;
    }

    Manager::FunctionPtr pidPairChargedBDTScore(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 2) {
        B2ERROR("Need exactly two arguments for pidPairChargedBDTScore: pdgCodeHyp, pdgCodeTest");
        return nullptr;
      }

      auto pdgCodeHyp(arguments.at(0));
      auto pdgCodeTest(arguments.at(1));

      auto func = [pdgCodeHyp, pdgCodeTest](const Particle * part) -> double {
        auto name = "pidPairChargedBDTScore_" + pdgCodeHyp + "_VS_" + pdgCodeTest;
        return (part->hasExtraInfo(name)) ? part->getExtraInfo(name) : std::numeric_limits<float>::quiet_NaN();
      };
      return func;
    }

    double mostLikelyPDG(const Particle* part)
    {
      auto* pid = part->getPIDLikelihood();
      if (!pid) return std::numeric_limits<double>::quiet_NaN();
      return pid->getMostLikely().getPDGCode();
    }

    double isMostLikely(const Particle* part)
    {
      return mostLikelyPDG(part) == abs(part->getPDGCode());
    }

    //*************
    // B2BII
    //*************

    double muIDBelle(const Particle* particle)
    {
      const PIDLikelihood* pid = particle->getPIDLikelihood();
      if (!pid) return 0.5; // Belle standard

      if (pid->isAvailable(Const::KLM))
        return exp(pid->getLogL(Const::muon, Const::KLM));
      else
        return 0; // Belle standard
    }

    double muIDBelleQuality(const Particle* particle)
    {
      const PIDLikelihood* pid = particle->getPIDLikelihood();
      if (!pid) return 0;// Belle standard

      return pid->isAvailable(Const::KLM);
    }

    double atcPIDBelle(const Particle* particle,  const std::vector<double>& sigAndBkgHyp)
    {
      int sigHyp = int(std::lround(sigAndBkgHyp[0]));
      int bkgHyp = int(std::lround(sigAndBkgHyp[1]));

      const PIDLikelihood* pid = particle->getPIDLikelihood();
      if (!pid) return 0.5; // Belle standard

      // ACC = ARICH
      Const::PIDDetectorSet set = Const::ARICH;
      double acc_sig = exp(pid->getLogL(hypothesisConversion(sigHyp), set));
      double acc_bkg = exp(pid->getLogL(hypothesisConversion(bkgHyp), set));
      double acc = 0.5; // Belle standard
      if (acc_sig + acc_bkg  > 0.0)
        acc = acc_sig / (acc_sig + acc_bkg);

      // TOF = TOP
      set = Const::TOP;
      double tof_sig = exp(pid->getLogL(hypothesisConversion(sigHyp), set));
      double tof_bkg = exp(pid->getLogL(hypothesisConversion(bkgHyp), set));
      double tof = 0.5; // Belle standard
      double tof_all = tof_sig + tof_bkg;
      if (tof_all != 0) {
        tof = tof_sig / tof_all;
        if (tof < 0.001) tof = 0.001;
        if (tof > 0.999) tof = 0.999;
      }

      // dE/dx = CDC
      set = Const::CDC;
      double cdc_sig = exp(pid->getLogL(hypothesisConversion(sigHyp), set));
      double cdc_bkg = exp(pid->getLogL(hypothesisConversion(bkgHyp), set));
      double cdc = 0.5; // Belle standard
      double cdc_all = cdc_sig + cdc_bkg;
      if (cdc_all != 0) {
        cdc = cdc_sig / cdc_all;
        if (cdc < 0.001) cdc = 0.001;
        if (cdc > 0.999) cdc = 0.999;
      }

      // Combined
      double pid_sig = acc * tof * cdc;
      double pid_bkg = (1. - acc) * (1. - tof) * (1. - cdc);

      return pid_sig / (pid_sig + pid_bkg);
    }


    double eIDBelle(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5; // Belle standard

      Const::PIDDetectorSet set = Const::ECL;
      return pid->getProbability(Const::electron, Const::pion, set);
    }


    // PID variables to be used for analysis
    VARIABLE_GROUP("PID");
    REGISTER_VARIABLE("particleID", particleID, "the particle identification probability under the particle's own hypothesis");

    REGISTER_VARIABLE("electronID", electronID,
                      "electron identification probability defined as :math:`\\mathcal{L}_e/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors");
    REGISTER_VARIABLE("muonID", muonID,
                      "muon identification probability defined as :math:`\\mathcal{L}_\\mu/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors");
    REGISTER_VARIABLE("pionID", pionID,
                      "pion identification probability defined as :math:`\\mathcal{L}_\\pi/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors");
    REGISTER_VARIABLE("kaonID", kaonID,
                      "kaon identification probability defined as :math:`\\mathcal{L}_K/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors");
    REGISTER_VARIABLE("protonID", protonID,
                      "proton identification probability defined as :math:`\\mathcal{L}_p/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors");
    REGISTER_VARIABLE("deuteronID", deuteronID,
                      "deuteron identification probability defined as :math:`\\mathcal{L}_d/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors");

    // Metafunctions for experts to access the basic PID quantities
    VARIABLE_GROUP("PID_expert");
    REGISTER_VARIABLE("pidLogLikelihoodValueExpert(pdgCode, detectorList)", pidLogLikelihoodValueExpert,
                      "returns the log likelihood value of for a specific mass hypothesis and  set of detectors.");
    REGISTER_VARIABLE("pidDeltaLogLikelihoodValueExpert(pdgCode1, pdgCode2, detectorList)", pidDeltaLogLikelihoodValueExpert,
                      "returns LogL(hyp1) - LogL(hyp2) (aka DLL) for two mass hypoteses and a set of detectors.");
    REGISTER_VARIABLE("pidPairProbabilityExpert(pdgCodeHyp, pdgCodeTest, detectorList)", pidPairProbabilityExpert,
                      "Pair (or binary) probability for the pdgCodeHyp mass hypothesis respect to the pdgCodeTest one, using an arbitrary set of detectors. :math:`\\mathcal{L}_{hyp}/(\\mathcal{L}_{test}+\\mathcal{L}_{hyp}`");
    REGISTER_VARIABLE("pidProbabilityExpert(pdgCodeHyp, detectorList)", pidProbabilityExpert,
                      "probability for the pdgCodeHyp mass hypothesis respect to all the other ones, using an arbitrary set of detectors :math:`\\mathcal{L}_{hyp}/(\\Sigma_{\\text{all~hyp}}\\mathcal{L}_{i}`. ");
    REGISTER_VARIABLE("pidMissingProbabilityExpert(detectorList)", pidMissingProbabilityExpert,
                      "returns 1 if the PID probabiliy is missing for the provided detector list, otherwise 0. ");
    REGISTER_VARIABLE("pidChargedBDTScore(pdgCodeHyp)", pidChargedBDTScore,
                      "returns the charged Pid BDT score for a certain mass hypothesis with respect to all other charged stable particle hypotheses.");
    REGISTER_VARIABLE("pidPairChargedBDTScore(pdgCodeHyp, pdgCodeTest)", pidPairChargedBDTScore,
                      "returns the charged Pid BDT score for a certain mass hypothesis with respect to an alternative hypothesis.");
    REGISTER_VARIABLE("pidMostLikelyPDG", mostLikelyPDG,
                      "Returns PDG code of the largest PID likelihood, or NaN if PID information is not available.");
    REGISTER_VARIABLE("pidIsMostLikely", isMostLikely,
                      "Returns 1 if the PID likelihood for the particle given its PID is the largest one");

    // B2BII PID
    VARIABLE_GROUP("PID_belle");
    REGISTER_VARIABLE("atcPIDBelle(i,j)", atcPIDBelle,
                      "returns Belle's PID atc variable: atc_pid(3,1,5,i,j).prob().\n"
                      "Parameters i,j are signal and backgroud hypotesis: (0 = electron, 1 = muon, 2 = pion, 3 = kaon, 4 = proton)");
    REGISTER_VARIABLE("muIDBelle", muIDBelle,
                      "returns Belle's PID Muon_likelihood() variable.");
    REGISTER_VARIABLE("muIDBelleQuality", muIDBelleQuality,
                      "returns true if Belle's PID Muon_likelihood() is usable (reliable).");
    REGISTER_VARIABLE("eIDBelle", eIDBelle,
                      "returns Belle's electron ID (eid(3,-1,5).prob()) variable.");

  }
}
