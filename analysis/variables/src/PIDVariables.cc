/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/variables/PIDVariables.h>

#include <analysis/VariableManager/Manager.h>

#include <mdst/dataobjects/PIDLikelihood.h>

// framework aux
#include <framework/logging/Logger.h>
#include <framework/utilities/Conversion.h>
#include <framework/gearbox/Const.h>

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


    Const::PIDDetectorSet parseDetectors(const std::vector<std::string>& arguments)
    {
      Const::PIDDetectorSet result;
      for (std::string val : arguments) {
        boost::to_lower(val);
        if (val == "all") return Const::PIDDetectors::set();
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

    // Specialisation of valid detectors parser for ChargedBDT.
    Const::PIDDetectorSet parseDetectorsChargedBDT(const std::vector<std::string>& arguments)
    {
      Const::PIDDetectorSet result;
      for (std::string val : arguments) {
        boost::to_lower(val);
        if (val == "all") return Const::PIDDetectors::set();
        else if (val == "ecl") result += Const::ECL;
        else B2ERROR("Invalid detector component: " << val << " for charged BDT.");
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
        pdgCode = Belle2::convertString<int>(arguments[0]);
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
        // No information form any subdetector in the list
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
        pdgCodeHyp = Belle2::convertString<int>(arguments[0]);
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidDeltaLogLikelihoodValueExpert must be a PDG code");
        return nullptr;
      }
      try {
        pdgCodeTest = Belle2::convertString<int>(arguments[1]);
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
        // No information form any subdetector in the list
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
        pdgCodeHyp = Belle2::convertString<int>(arguments[0]);
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidPairProbabilityExpert must be PDG code");
        return nullptr;
      }
      try {
        pdgCodeTest = Belle2::convertString<int>(arguments[1]);
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
        // No information from any subdetector in the list
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
        pdgCodeHyp = Belle2::convertString<int>(arguments[0]);
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
        // No information from any subdetector in the list
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

      auto func = [detectorSet](const Particle * part) -> double {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid) return std::numeric_limits<double>::quiet_NaN();
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

    double binaryPID(const Particle* part, const std::vector<double>& arguments)
    {
      if (arguments.size() != 2) {
        B2ERROR("The variable binaryPID needs exactly two arguments: the PDG codes of two hypotheses.");
        return std::numeric_limits<float>::quiet_NaN();;
      }
      int pdgCodeHyp = std::abs(int(std::lround(arguments[0])));
      int pdgCodeTest = std::abs(int(std::lround(arguments[1])));
      return Manager::Instance().getVariable("pidPairProbabilityExpert(" + std::to_string(pdgCodeHyp) + ", " + std::to_string(
                                               pdgCodeTest) + ", ALL)")->function(part);
    }

    double electronID_noSVD(const Particle* part)
    {
      // Excluding SVD for electron ID. This variable is temporary. BII-8760
      return Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(part);
    }

    double muonID_noSVD(const Particle* part)
    {
      // Excluding SVD for muon ID. This variable is temporary. BII-8760
      return Manager::Instance().getVariable("pidProbabilityExpert(13, CDC, TOP, ARICH, ECL, KLM)")->function(part);
    }

    double pionID_noSVD(const Particle* part)
    {
      // Excluding SVD for pion ID. This variable is temporary. BII-8760
      return Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(part);
    }

    double kaonID_noSVD(const Particle* part)
    {
      // Excluding SVD for kaon ID. This variable is temporary. BII-8760
      return Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(part);
    }

    double protonID_noSVD(const Particle* part)
    {
      // Excluding SVD for proton ID. This variable is temporary. BII-8760
      return Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(part);
    }

    double deuteronID_noSVD(const Particle* part)
    {
      // Excluding SVD for deuteron ID. This variable is temporary. BII-8760
      return Manager::Instance().getVariable("pidProbabilityExpert(1000010020, CDC, TOP, ARICH, ECL, KLM)")->function(part);
    }

    double binaryPID_noSVD(const Particle* part, const std::vector<double>& arguments)
    {
      // Excluding SVD for binary ID. This variable is temporary. BII-8760
      if (arguments.size() != 2) {
        B2ERROR("The variable binaryPID_noSVD needs exactly two arguments: the PDG codes of two hypotheses.");
        return std::numeric_limits<float>::quiet_NaN();;
      }
      int pdgCodeHyp = std::abs(int(std::lround(arguments[0])));
      int pdgCodeTest = std::abs(int(std::lround(arguments[1])));
      return Manager::Instance().getVariable("pidPairProbabilityExpert(" + std::to_string(pdgCodeHyp) + ", " + std::to_string(
                                               pdgCodeTest) + ", CDC, TOP, ARICH, ECL, KLM)")->function(part);
    }

    double antineutronID(const Particle* particle)
    {
      if (particle->hasExtraInfo("nbarID")) {
        return particle->getExtraInfo("nbarID");
      } else {
        if (particle->getPDGCode() == -Const::neutron.getPDGCode()) {
          B2WARNING("The extraInfo nbarID is not registered! \n"
                    "Please use function getNbarIDMVA in modularAnalysis.");
        }
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    Manager::FunctionPtr pidChargedBDTScore(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 2) {
        B2ERROR("Need exactly two arguments for pidChargedBDTScore: pdgCodeHyp, detector");
        return nullptr;
      }

      int hypPdgId;
      try {
        hypPdgId = Belle2::convertString<int>(arguments.at(0));
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidChargedBDTScore must be an integer (PDG code).");
        return nullptr;
      }
      Const::ChargedStable hypType = Const::ChargedStable(hypPdgId);

      std::vector<std::string> detectors(arguments.begin() + 1, arguments.end());
      Const::PIDDetectorSet detectorSet = parseDetectorsChargedBDT(detectors);

      auto func = [hypType, detectorSet](const Particle * part) -> double {
        auto name = "pidChargedBDTScore_" + std::to_string(hypType.getPDGCode());
        for (size_t iDet(0); iDet < detectorSet.size(); ++iDet)
        {
          auto det = detectorSet[iDet];
          name += "_" + std::to_string(det);
        }
        return (part->hasExtraInfo(name)) ? part->getExtraInfo(name) : std::numeric_limits<float>::quiet_NaN();
      };
      return func;
    }

    Manager::FunctionPtr pidPairChargedBDTScore(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 3) {
        B2ERROR("Need exactly three arguments for pidPairChargedBDTScore: pdgCodeHyp, pdgCodeTest, detector.");
        return nullptr;
      }

      int hypPdgId, testPdgId;
      try {
        hypPdgId = Belle2::convertString<int>(arguments.at(0));
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidPairChargedBDTScore must be an integer (PDG code).");
        return nullptr;
      }
      try {
        testPdgId = Belle2::convertString<int>(arguments.at(1));
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidPairChargedBDTScore must be an integer (PDG code).");
        return nullptr;
      }
      Const::ChargedStable hypType = Const::ChargedStable(hypPdgId);
      Const::ChargedStable testType = Const::ChargedStable(testPdgId);

      std::vector<std::string> detectors(arguments.begin() + 2, arguments.end());
      Const::PIDDetectorSet detectorSet = parseDetectorsChargedBDT(detectors);

      auto func = [hypType, testType, detectorSet](const Particle * part) -> double {
        auto name = "pidPairChargedBDTScore_" + std::to_string(hypType.getPDGCode()) + "_VS_" + std::to_string(testType.getPDGCode());
        for (size_t iDet(0); iDet < detectorSet.size(); ++iDet)
        {
          auto det = detectorSet[iDet];
          name += "_" + std::to_string(det);
        }
        return (part->hasExtraInfo(name)) ? part->getExtraInfo(name) : std::numeric_limits<float>::quiet_NaN();
      };
      return func;
    }

    Manager::FunctionPtr mostLikelyPDG(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 0 and arguments.size() != Const::ChargedStable::c_SetSize) {
        B2ERROR("Need zero or exactly " << Const::ChargedStable::c_SetSize << " arguments for pidMostLikelyPDG");
        return nullptr;
      }
      double prob[Const::ChargedStable::c_SetSize];
      if (arguments.size() == 0) {
        for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) prob[i] = 1. / Const::ChargedStable::c_SetSize;
      }
      if (arguments.size() == Const::ChargedStable::c_SetSize) {
        try {
          int i = 0;
          for (std::string arg : arguments) {
            prob[i++] = Belle2::convertString<float>(arg);
          }
        } catch (std::invalid_argument& e) {
          B2ERROR("All arguments of mostLikelyPDG must be a float number");
          return nullptr;
        }
      }
      auto func = [prob](const Particle * part) -> double {
        auto* pid = part->getPIDLikelihood();
        if (!pid) return std::numeric_limits<double>::quiet_NaN();
        return pid->getMostLikely(prob).getPDGCode();
      };
      return func;
    }

    Manager::FunctionPtr isMostLikely(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 0 and arguments.size() != 6) {
        B2ERROR("Need zero or exactly " << Const::ChargedStable::c_SetSize << " arguments for pidIsMostLikely");
        return nullptr;
      }
      auto func = [arguments](const Particle * part) -> double {
        return mostLikelyPDG(arguments)(part) == abs(part->getPDGCode());
      };
      return func;
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
    REGISTER_VARIABLE("particleID", particleID,
                      "the particle identification probability under the particle's own hypothesis, using info from all available detectors");
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
    REGISTER_VARIABLE("binaryPID(pdgCode1, pdgCode2)", binaryPID,
                      "Returns the binary probability for the first provided mass hypothesis with respect to the second mass hypothesis using all detector components");
    REGISTER_VARIABLE("electronID_noSVD", electronID_noSVD,
                      "(SPECIAL (TEMP) variable) electron identification probability defined as :math:`\\mathcal{L}_e/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors *excluding the SVD*");
    REGISTER_VARIABLE("muonID_noSVD", muonID_noSVD,
                      "(SPECIAL (TEMP) variable) muon identification probability defined as :math:`\\mathcal{L}_\\mu/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors *excluding the SVD*");
    REGISTER_VARIABLE("pionID_noSVD", pionID_noSVD,
                      "(SPECIAL (TEMP) variable) pion identification probability defined as :math:`\\mathcal{L}_\\pi/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors *excluding the SVD*");
    REGISTER_VARIABLE("kaonID_noSVD", kaonID_noSVD,
                      "(SPECIAL (TEMP) variable) kaon identification probability defined as :math:`\\mathcal{L}_K/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors *excluding the SVD*");
    REGISTER_VARIABLE("protonID_noSVD", protonID_noSVD,
                      "(SPECIAL (TEMP) variable) proton identification probability defined as :math:`\\mathcal{L}_p/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors *excluding the SVD*");
    REGISTER_VARIABLE("deuteronID_noSVD", deuteronID_noSVD,
                      "(SPECIAL (TEMP) variable) deuteron identification probability defined as :math:`\\mathcal{L}_d/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors *excluding the SVD*");
    REGISTER_VARIABLE("binaryPID_noSVD(pdgCode1, pdgCode2)", binaryPID_noSVD,
                      "Returns the binary probability for the first provided mass hypothesis with respect to the second mass hypothesis using all detector components, *excluding the SVD*.");
    REGISTER_VARIABLE("nbarID", antineutronID, R"DOC(
Returns MVA classifier for antineutron PID.

    - 1  signal(antineutron) like
    - 0  background like
    - -1 invalid using this PID due to some ECL variables used unavailable

This PID is only for antineutron. Neutron is also considered as background.
The variables used are `clusterPulseShapeDiscriminationMVA`, `clusterE`, `clusterLAT`, `clusterE1E9`, `clusterE9E21`,
`clusterAbsZernikeMoment40`, `clusterAbsZernikeMoment51`, `clusterZernikeMVA`.)DOC");

    // Metafunctions for experts to access the basic PID quantities
    VARIABLE_GROUP("PID_expert");
    REGISTER_VARIABLE("pidLogLikelihoodValueExpert(pdgCode, detectorList)", pidLogLikelihoodValueExpert,
                      "returns the log likelihood value of for a specific mass hypothesis and  set of detectors.");
    REGISTER_VARIABLE("pidDeltaLogLikelihoodValueExpert(pdgCode1, pdgCode2, detectorList)", pidDeltaLogLikelihoodValueExpert,
                      "returns LogL(hyp1) - LogL(hyp2) (aka DLL) for two mass hypotheses and a set of detectors.");
    REGISTER_VARIABLE("pidPairProbabilityExpert(pdgCodeHyp, pdgCodeTest, detectorList)", pidPairProbabilityExpert,
                      "Pair (or binary) probability for the pdgCodeHyp mass hypothesis respect to the pdgCodeTest one, using an arbitrary set of detectors. :math:`\\mathcal{L}_{hyp}/(\\mathcal{L}_{test}+\\mathcal{L}_{hyp}`");
    REGISTER_VARIABLE("pidProbabilityExpert(pdgCodeHyp, detectorList)", pidProbabilityExpert,
                      "probability for the pdgCodeHyp mass hypothesis respect to all the other ones, using an arbitrary set of detectors :math:`\\mathcal{L}_{hyp}/(\\Sigma_{\\text{all~hyp}}\\mathcal{L}_{i}`. ");
    REGISTER_VARIABLE("pidMissingProbabilityExpert(detectorList)", pidMissingProbabilityExpert,
                      "returns 1 if the PID probabiliy is missing for the provided detector list, otherwise 0. ");
    REGISTER_VARIABLE("pidChargedBDTScore(pdgCodeHyp, detector)", pidChargedBDTScore,
                      "returns the charged Pid BDT score for a certain mass hypothesis with respect to all other charged stable particle hypotheses. The second argument specifies which BDT training to use: based on 'ALL' PID detectors (NB: 'SVD' is currently excluded), or 'ECL' only. The choice depends on the ChargedPidMVAMulticlassModule's configuration.");
    REGISTER_VARIABLE("pidPairChargedBDTScore(pdgCodeHyp, pdgCodeTest, detector)", pidPairChargedBDTScore,
                      "returns the charged Pid BDT score for a certain mass hypothesis with respect to an alternative hypothesis. The second argument specifies which BDT training to use: based on 'ALL' PID detectors (NB: 'SVD' is currently excluded), or 'ECL' only. The choice depends on the ChargedPidMVAModule's configuration.");
    REGISTER_VARIABLE("pidMostLikelyPDG", mostLikelyPDG,
                      "Returns PDG code of the largest PID likelihood, or NaN if PID information is not available.");
    REGISTER_VARIABLE("pidIsMostLikely", isMostLikely,
                      "Returns 1 if the PID likelihood for the particle given its PID is the largest one");

    // B2BII PID
    VARIABLE_GROUP("PID_belle");
    REGISTER_VARIABLE("atcPIDBelle(i,j)", atcPIDBelle,
                      "returns Belle's PID atc variable: ``atc_pid(3,1,5,i,j).prob()``.\n"
                      "Parameters i,j are signal and background hypothesis: (0 = electron, 1 = muon, 2 = pion, 3 = kaon, 4 = proton)");
    REGISTER_VARIABLE("muIDBelle", muIDBelle,
                      "returns Belle's PID ``Muon_likelihood()`` variable.");
    REGISTER_VARIABLE("muIDBelleQuality", muIDBelleQuality,
                      "returns true if Belle's PID ``Muon_likelihood()`` is usable (reliable).");
    REGISTER_VARIABLE("eIDBelle", eIDBelle,
                      "returns Belle's electron ID ``eid(3,-1,5).prob()`` variable.");

  }
}
