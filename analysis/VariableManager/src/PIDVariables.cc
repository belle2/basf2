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
#include <analysis/VariableManager/PIDVariables.h>
#include <analysis/VariableManager/Variable.h>

#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/PIDLikelihood.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <algorithm>
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
        if (val == "default") return Const::SVD + Const::CDC + Const::TOP + Const::ARICH;
        else if (val == "all") return Const::SVD + Const::CDC + Const::TOP + Const::ARICH + Const::ECL + Const:: KLM;
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
      Const::ChargedStable hypType = Const::ChargedStable(pdgCode);

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
      Const::ChargedStable hypType = Const::ChargedStable(pdgCodeHyp);
      Const::ChargedStable testType = Const::ChargedStable(pdgCodeTest);

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
      int pdgCodeHyp, pdgCodeTest;
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
      Const::ChargedStable hypType = Const::ChargedStable(pdgCodeHyp);
      Const::ChargedStable testType = Const::ChargedStable(pdgCodeTest);
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
      int pdgCodeHyp;
      try {
        pdgCodeHyp = std::stoi(arguments[0]);
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidProbabilityExpert must be PDG code");
        return nullptr;
      }

      std::vector<std::string> detectors(arguments.begin() + 1, arguments.end());
      Const::PIDDetectorSet detectorSet = parseDetectors(detectors);
      Const::ChargedStable hypType = Const::ChargedStable(pdgCodeHyp);

      // Placeholder for the priors
      const unsigned int n = Const::ChargedStable::c_SetSize;
      double frac[n];
      for (unsigned int i = 0; i < n; ++i) frac[i] = 1.0; // flat priors

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
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return std::numeric_limits<float>::quiet_NaN();

      const Const::ChargedStable partDef = (part->getCharge() < 0) ? Const::electron : Const::antielectron;
      const Const::ChargedStable partDefOther = (part->getCharge() < 0) ? Const::antipion : Const::pion;
      return pid->getProbability(partDef, partDefOther);
    }

    double muonID(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return std::numeric_limits<float>::quiet_NaN();

      const Const::ChargedStable partDef = (part->getCharge() < 0) ? Const::muon : Const::antimuon;
      const Const::ChargedStable partDefOther = (part->getCharge() < 0) ? Const::antipion : Const::pion;
      return pid->getProbability(partDef, partDefOther);
    }

    double pionID(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return std::numeric_limits<float>::quiet_NaN();

      const Const::ChargedStable partDef = (part->getCharge() > 0) ? Const::pion : Const::antipion;
      const Const::ChargedStable partDefOther = (part->getCharge() > 0) ? Const::kaon : Const::antikaon;
      return pid->getProbability(partDef, partDefOther);
    }

    double kaonID(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return std::numeric_limits<float>::quiet_NaN();

      const Const::ChargedStable partDef = (part->getCharge() > 0) ? Const::kaon : Const::antikaon;
      const Const::ChargedStable partDefOther = (part->getCharge() > 0) ? Const::pion : Const::antipion;
      return pid->getProbability(partDef, partDefOther);
    }

    double protonID(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return std::numeric_limits<float>::quiet_NaN();

      const Const::ChargedStable partDef = (part->getCharge() > 0) ? Const::proton : Const::antiproton;
      const Const::ChargedStable partDefOther = (part->getCharge() > 0) ? Const::pion : Const::antipion;
      return pid->getProbability(partDef, partDefOther);
    }

    double deuteronID(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return std::numeric_limits<float>::quiet_NaN();

      const Const::ChargedStable partDef = (part->getCharge() > 0) ? Const::deuteron : Const::antideuteron;
      const Const::ChargedStable partDefOther = (part->getCharge() > 0) ? Const::pion : Const::antipion;
      return pid->getProbability(partDef, partDefOther);
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


    // Needed by the flavor tagger algorithm
    double kIDBelle(const Particle* part)
    {
      //default values Belle style
      float accs = 0.5;
      float tofs = 0.5;
      float cdcs = 0.5;

      const PIDLikelihood* pid = part->getPIDLikelihood();

      if (pid) {
        Const::PIDDetectorSet set = Const::ARICH;
        accs = pid->getProbability(Const::kaon, Const::pion, set);
        set = Const::TOP;
        tofs = pid->getProbability(Const::kaon, Const::pion, set);
        set = Const::TOP + Const::SVD;
        cdcs = pid->getProbability(Const::kaon, Const::pion, set);
      }

      if (tofs > 0.999) tofs = 0.999;
      if (tofs < 0.001) tofs = 0.001;
      if (cdcs > 0.999) cdcs = 0.999;
      if (cdcs < 0.001) cdcs = 0.001;

      float s = accs * tofs * cdcs;
      float b = (1. - accs) * (1. - tofs) * (1. - cdcs);

      float r = s / (b + s);

      return r;
    }


    VARIABLE_GROUP("PID");


    // PID variables to be used for analysis
    REGISTER_VARIABLE("electronID", electronID, "electron identification probability");
    REGISTER_VARIABLE("muonID", muonID, "muon identification probability");
    REGISTER_VARIABLE("pionID", pionID, "pion identification probability");
    REGISTER_VARIABLE("kaonID", kaonID, "kaon identification probability");
    REGISTER_VARIABLE("protonID", protonID, "proton identification probability");
    REGISTER_VARIABLE("deuteronID", deuteronID, "deuteron identification probability");

    // Metafunctions for experts to access the basic PID quantities
    REGISTER_VARIABLE("pidLogLikelihoodValueExpert(pdgCode, detectorList)", pidLogLikelihoodValueExpert,
                      "returns the likelihood value of for a specific mass and charge hypothesis (using PDG conventions) and set of detectors. Not to be used in physics analyses, but only by experts doing performance studies.");
    REGISTER_VARIABLE("pidDeltaLogLikelihoodValueExpert(pdgCode1, pdgCode2, detectorList)", pidDeltaLogLikelihoodValueExpert,
                      "returns LogL(hyp1) - LogL(hyp2). Not to be used in physics analyses, but only by experts doing performance studies.");
    REGISTER_VARIABLE("pidPairProbabilityExpert(pdgCodeHyp, pdgCodeTest, detectorList)", pidPairProbabilityExpert,
                      "probability for the pdgCodeHyp mass and charge hypothesis (using PDG conventions) with respect to the pdgCodeTest one, using an arbitrary set of detectors.  Not to be used in physics analyses, but only by experts doing performance studies.");
    REGISTER_VARIABLE("pidProbabilityExpert(pdgCodeHyp, detectorList)", pidProbabilityExpert,
                      "probability for the pdgCodeHyp mass and charge hypothesis (using PDG conventions) with respect to all the other ones, using an arbitrary set of detectors.  Not to be used in physics analyses, but only by experts doing performance studies.");
    REGISTER_VARIABLE("pidMissingProbabilityExpert(detectorList)", pidMissingProbabilityExpert,
                      "returns 1 if the PID probabiliy is missing for the provided detector list, otherwise 0. ");

    // B2BII PID
    REGISTER_VARIABLE("atcPIDBelle(i,j)", atcPIDBelle,
                      "returns Belle's PID atc variable: atc_pid(3,1,5,i,j).prob().\n"
                      "To be used only when analysing converted Belle samples.");
    REGISTER_VARIABLE("muIDBelle", muIDBelle,
                      "returns Belle's PID Muon_likelihood() variable.\n"
                      "To be used only when analysing converted Belle samples.");
    REGISTER_VARIABLE("muIDBelleQuality", muIDBelleQuality,
                      "returns true if Belle's PID Muon_likelihood() is usable (reliable).\n"
                      "To be used only when analysing converted Belle samples.");
    REGISTER_VARIABLE("eIDBelle", eIDBelle,
                      "returns Belle's electron ID (eid(3,-1,5).prob()) variable.\n"
                      "To be used only when analysing converted Belle samples.");
    REGISTER_VARIABLE("kIDBelle", kIDBelle, "kaon identification probability bellestyle.");


  }
}
