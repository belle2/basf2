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


    //************************************
    //
    // PID variables for release 01-00-00
    //
    //************************************


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

    Manager::FunctionPtr particleLogLikelihoodValue(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 2) {
        B2ERROR("Need at least two arguments to particleLogLikelihoodValue");
        return nullptr;
      }
      int pdgCode;
      try {
        pdgCode = std::stoi(arguments[0]);
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of particleLogLikelihoodValue must be a PDG code");
        return nullptr;
      }
      std::vector<std::string> detectors(arguments.begin() + 1, arguments.end());

      Const::PIDDetectorSet detectorSet = parseDetectors(detectors);
      Const::ChargedStable hypType = Const::ChargedStable(abs(pdgCode));

      auto func = [hypType, detectorSet](const Particle * part) -> double {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid) return -999;
        return pid->getLogL(hypType, detectorSet);
      };
      return func;
    }


    Manager::FunctionPtr particlePairProbability(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 3) {
        B2ERROR("Need at least three arguments to particlePairProbability");
        return nullptr;
      }
      int pdgCodeHyp, pdgCodeTest;
      try {
        pdgCodeHyp = std::stoi(arguments[0]);
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of particlePairProbability must be PDG code");
        return nullptr;
      }
      try {
        pdgCodeTest = std::stoi(arguments[1]);
      } catch (std::invalid_argument& e) {
        B2ERROR("Second argument of particlePairProbability must be PDG code");
        return nullptr;
      }

      std::vector<std::string> detectors(arguments.begin() + 2, arguments.end());

      Const::PIDDetectorSet detectorSet = parseDetectors(detectors);
      Const::ChargedStable hypType = Const::ChargedStable(abs(pdgCodeHyp));
      Const::ChargedStable testType = Const::ChargedStable(abs(pdgCodeTest));
      auto func = [hypType, testType, detectorSet](const Particle * part) -> double {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid) return -999;
        return pid->getProbability(hypType, testType, detectorSet);
      };
      return func;
    }


    Manager::FunctionPtr particleProbability(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 2) {
        B2ERROR("Need at least two arguments to particleProbability");
        return nullptr;
      }
      int pdgCodeHyp;
      try {
        pdgCodeHyp = std::stoi(arguments[0]);
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of particleProbability must be PDG code");
        return nullptr;
      }

      std::vector<std::string> detectors(arguments.begin() + 1, arguments.end());
      Const::PIDDetectorSet detectorSet = parseDetectors(detectors);
      Const::ChargedStable hypType = Const::ChargedStable(abs(pdgCodeHyp));

      // Placeholder for the priors
      const unsigned int n = Const::ChargedStable::c_SetSize;
      double frac[n];
      for (unsigned int i = 0; i < n; ++i) frac[i] = 1.0; // flat priors


      auto func = [hypType, frac, detectorSet](const Particle * part) -> double {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid) return -999;
        return pid->getProbability(hypType, frac, detectorSet);
      };
      return func;
    }


    Manager::FunctionPtr particleMissingProbability(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 1) {
        B2ERROR("Need at least one argument to particleMissingProbability");
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


    double particleElectronId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      return pid->getProbability(Const::electron, Const::pion);
    }

    double particleMuonId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      return pid->getProbability(Const::muon, Const::pion);
    }

    double particlePionId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      return pid->getProbability(Const::pion, Const::kaon);
    }

    double particleKaonId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      return pid->getProbability(Const::kaon, Const::pion);
    }


    double particleProtonId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      return pid->getProbability(Const::proton, Const::pion);
    }

    double particleDeuteronId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      return pid->getProbability(Const::deuteron, Const::pion);
    }


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

    double muIDBelle(const Particle* particle)
    {
      const PIDLikelihood* pid = particle->getPIDLikelihood();
      if (!pid) return 0.5;

      if (pid->isAvailable(Const::KLM))
        return exp(pid->getLogL(Const::muon, Const::KLM));
      else
        return 0;
    }

    double muIDBelleQuality(const Particle* particle)
    {
      const PIDLikelihood* pid = particle->getPIDLikelihood();
      if (!pid) return 0;

      return pid->isAvailable(Const::KLM);
    }

    double atcPIDBelle(const Particle* particle,  const std::vector<double>& sigAndBkgHyp)
    {
      int sigHyp = int(std::lround(sigAndBkgHyp[0]));
      int bkgHyp = int(std::lround(sigAndBkgHyp[1]));

      const PIDLikelihood* pid = particle->getPIDLikelihood();
      if (!pid) return 0.5;

      // ACC = ARICH
      Const::PIDDetectorSet set = Const::ARICH;
      double acc_sig = exp(pid->getLogL(hypothesisConversion(sigHyp), set));
      double acc_bkg = exp(pid->getLogL(hypothesisConversion(bkgHyp), set));
      double acc = 0.5;
      if (acc_sig + acc_bkg  > 0.0)
        acc = acc_sig / (acc_sig + acc_bkg);

      // TOF = TOP
      set = Const::TOP;
      double tof_sig = exp(pid->getLogL(hypothesisConversion(sigHyp), set));
      double tof_bkg = exp(pid->getLogL(hypothesisConversion(bkgHyp), set));
      double tof = 0.5;
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
      double cdc = 0.5;
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











    //************************************
    //
    // Legacy variables, to be removed for release 01-00-00
    //
    //************************************


    double deltaLogL(const Particle* part, const Const::ChargedStable& otherHypothesis)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.0;

      Const::ChargedStable thisType = Const::ChargedStable(abs(part->getPDGCode()));

      return pid->getLogL(thisType) - pid->getLogL(otherHypothesis);
    }

    double particleDeltaLogLPion(const Particle* part)
    {
      B2WARNING("Deprecated variable DLLPion. Please use the particle probability PID_piID instead.");
      return deltaLogL(part, Const::pion);
    }

    double particleDeltaLogLKaon(const Particle* part)
    {
      B2WARNING("Deprecated variable DLLKaon. Please use the particle probability PID_KID instead.");
      return deltaLogL(part, Const::kaon);
    }

    double particleDeltaLogLProton(const Particle* part)
    {
      B2WARNING("Deprecated variable DLLProton. Please use the particle probability PID_prID instead.");
      return deltaLogL(part, Const::proton);
    }

    double particleDeltaLogLElectron(const Particle* part)
    {
      B2WARNING("Deprecated variable DLLElectron. Please use the particle probability PID_eID instead.");
      return deltaLogL(part, Const::electron);
    }

    double particleDeltaLogLMuon(const Particle* part)
    {
      B2WARNING("Deprecated variable DLLMuon. Please use the particle probability PID_muID instead.");
      return deltaLogL(part, Const::muon);
    }

    double particleDeltaLogLDeuteron(const Particle* part)
    {
      B2WARNING("Deprecated variable DLLDeuteron. Please use the particle probability PID_dID instead.");
      return deltaLogL(part, Const::deuteron);
    }


    double particlePionvsElectronId(const Particle* part)
    {
      B2WARNING("Deprecated variable pi_vs_eid. Please replace it with PID_pairProbabilityExpert(211, 11, ALL)");
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      return pid->getProbability(Const::pion, Const::electron);
    }

    double particlePionvsElectrondEdxId(const Particle* part)
    {
      B2WARNING("Deprecated variable pi_vs_edEdxid. Please replace it with PID_pairProbabilityExpert(211, 11, SVD, CDC) ");
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::pion, Const::electron, set);
    }

    double particleElectrondEdxId(const Particle* part)
    {
      B2WARNING("Deprecated variable eid_dEdx. Please replace it with PID_pairProbabilityExpert(11, 211, SVD, CDC)");
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::electron, Const::pion, set);
    }

    double particleElectronTOPId(const Particle* part)
    {
      B2WARNING("Deprecated variable eid_TOP. Please replace it with PID_pairProbabilityExpert(11, 211, TOP)");
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::electron, Const::pion, set);
    }

    double particleElectronARICHId(const Particle* part)
    {
      B2WARNING("Deprecated variable eid_ARICH. Please replace it with PID_pairProbabilityExpert(11, 211, ARICH)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::electron, Const::pion, set);
    }

    double particleElectronECLId(const Particle* part)
    {
      B2WARNING("Deprecated variable eid_ECL. Please replace it with PID_pairProbabilityExpert(11, 211, ECL)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ECL;
      return pid->getProbability(Const::electron, Const::pion, set);
    }

    double particleMuondEdxId(const Particle* part)
    {
      B2WARNING("Deprecated variable muid_dEdx. Please replace it with PID_pairProbabilityExpert(13, 211, SVD, CDC)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::muon, Const::pion, set);

    }

    double particleMuonTOPId(const Particle* part)
    {
      B2WARNING("Deprecated variable muid_TOP. Please replace it with PID_pairProbabilityExpert(13, 211, TOP)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::muon, Const::pion, set);
    }

    double particleMuonARICHId(const Particle* part)
    {
      B2WARNING("Deprecated variable muid_ARICH. Please replace it with PID_pairProbabilityExpert(13, 211, ARICH)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::muon, Const::pion, set);
    }

    double particleMuonKLMId(const Particle* part)
    {
      B2WARNING("Deprecated variable muid_KLM. Please replace it with PID_pairProbabilityExpert(13, 211, KLM)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::KLM;
      return pid->getProbability(Const::muon, Const::pion, set);
    }

    double particlePiondEdxId(const Particle* part)
    {
      B2WARNING("Deprecated variable piid_dEdx. Please replace it with PID_pairProbabilityExpert(211, 321, SVD, CDC)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::pion, Const::kaon, set);
    }

    double particlePionTOPId(const Particle* part)
    {
      B2WARNING("Deprecated variable piid_TOP. Please replace it with PID_pairProbabilityExpert(211, 321, TOP)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::pion, Const::kaon, set);
    }

    double particlePionARICHId(const Particle* part)
    {
      B2WARNING("Deprecated variable piid_ARICH. Please replace it with PID_pairProbabilityExpert(211, 321, ARICH)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::pion, Const::kaon, set);
    }

    double particleKaondEdxId(const Particle* part)
    {
      B2WARNING("Deprecated variable Kid_dEdx. Please replace it with PID_pairProbabilityExpert(321, 211, SVD, CDC)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::kaon, Const::pion, set);
    }

    double particleKaonTOPId(const Particle* part)
    {
      B2WARNING("Deprecated variable Kid_TOP. Please replace it with PID_pairProbabilityExpert(321, 211, TOP)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::kaon, Const::pion, set);
    }

    double particleKaonARICHId(const Particle* part)
    {
      B2WARNING("Deprecated variable Kid_ARICH. Please replace it with PID_pairProbabilityExpert(321, 211, ARICH)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::kaon, Const::pion, set);
    }

    double particleProtondEdxId(const Particle* part)
    {
      B2WARNING("Deprecated variable prid_dEdx. Please replace it with PID_pairProbabilityExpert(2212, 211, SVD, CDC)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::proton, Const::pion, set);
    }

    double particleProtonTOPId(const Particle* part)
    {
      B2WARNING("Deprecated variable prid_TOP. Please replace it with PID_pairProbabilityExpert(2212, 211, TOP)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::proton, Const::pion, set);
    }

    double particleProtonARICHId(const Particle* part)
    {
      B2WARNING("Deprecated variable prid_ARICH. Please replace it with PID_pairProbabilityExpert(2212, 211, ARICH)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::proton, Const::pion, set);
    }

    double particleMissingARICHId(const Particle* part)
    {
      B2WARNING("Deprecated variable missing_ARICH. Please replace it with PID_missingProbability(ARICH)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid)
        return 1.0;

      Const::PIDDetectorSet set = Const::ARICH;
      if (not pid->isAvailable(set))
        return 1.0;

      return 0;
    }

    double particleMissingTOPId(const Particle* part)
    {
      B2WARNING("Deprecated variable missing_TOP. Please replace it with PID_missingProbability(TOP)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid)
        return 1.0;

      Const::PIDDetectorSet set = Const::TOP;
      if (not pid->isAvailable(set))
        return 1.0;

      return 0;
    }

    double particleMissingECLId(const Particle* part)
    {
      B2WARNING("Deprecated variable missing_ECL. Please replace it with PID_missingProbability(ECL)");

      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid)
        return 1.0;

      Const::PIDDetectorSet set = Const::ECL;
      if (not pid->isAvailable(set))
        return 1.0;

      return 0;
    }

    double particleKaonIdBelle(const Particle* part)
    {
      float accs = particleKaonARICHId(part);
      float tofs = particleKaonTOPId(part);
      float cdcs = particleKaondEdxId(part);

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



    //************************************
    //
    // new PID variables, default starting from release 01-00-00
    //
    //************************************

    // PID variables to be used for analysis
    REGISTER_VARIABLE("PID_eID", particleElectronId, "electron identification probability");
    REGISTER_VARIABLE("PID_muID", particleMuonId, "muon identification probability");
    REGISTER_VARIABLE("PID_piID", particlePionId, "pion identification probability");
    REGISTER_VARIABLE("PID_KID", particleKaonId, "kaon identification probability");
    REGISTER_VARIABLE("PID_prID", particleProtonId, "proton identification probability");
    REGISTER_VARIABLE("PID_dID", particleDeuteronId, "deuteron identification probability");


    // Metafunctions for experts to access the basic PID quantities
    REGISTER_VARIABLE("PID_logLikelihoodValueExpert(pdgCode, detectorList)", particleLogLikelihoodValue,
                      "returns the likelihood value of for a specific mass hypothesis and  set of detectors. Not to be used in physics analyses, but only by experts doing performance studies.");
    REGISTER_VARIABLE("PID_pairProbabilityExpert(pdgCodeHyp, pdgCodeTest, detectorList)", particlePairProbability,
                      "probability for the pdgCodeHyp mass hypothesis respect to the pdgCodeTest one, using an arbitrary set of detectors.  Not to be used in physics analyses, but only by experts doing performance studies.");
    REGISTER_VARIABLE("PID_probabilityExpert(pdgCodeHyp, detectorList)", particleProbability,
                      "probability for the pdgCodeHyp mass hypothesis respect to all the other ones, using an arbitrary set of detectors.  Not to be used in physics analyses, but only by experts doing performance studies.");
    REGISTER_VARIABLE("PID_missingProbability(detectorList)", particleMissingProbability,
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
    REGISTER_VARIABLE("eIDBelle", particleElectronECLId,
                      "returns Belle's electron ID (eid(3,-1,5).prob()) variable.\n"
                      "To be used only when analysing converted Belle samples.");




    //************************************
    //
    // Old and deprecated variables, to be removed in release 01-00-00
    //
    //************************************

    REGISTER_VARIABLE("DLLPion", particleDeltaLogLPion,     "DEPRECATED. Delta Log L = L(particle's hypothesis) - L(pion)");
    REGISTER_VARIABLE("DLLKaon", particleDeltaLogLKaon,     "DEPRECATED. Delta Log L = L(particle's hypothesis) - L(kaon)");
    REGISTER_VARIABLE("DLLProt", particleDeltaLogLProton,   "DEPRECATED. Delta Log L = L(particle's hypothesis) - L(proton)");
    REGISTER_VARIABLE("DLLDeut", particleDeltaLogLDeuteron, "DEPRECATED. Delta Log L = L(particle's hypothesis) - L(deuteron)");
    REGISTER_VARIABLE("DLLElec", particleDeltaLogLElectron, "DEPRECATED. Delta Log L = L(particle's hypothesis) - L(electron)");
    REGISTER_VARIABLE("DLLMuon", particleDeltaLogLMuon,     "DEPRECATED. Delta Log L = L(particle's hypothesis) - L(muon)");


    REGISTER_VARIABLE("eid", particleElectronId,
                      "DEPRECATED. electron ID variable used before release 01-00-00. To be replaced by PID_eID");
    REGISTER_VARIABLE("muid", particleMuonId,
                      "DEPRECATED. muon ID variable used before release 01-00-00. To be replaced by PID_muID");
    REGISTER_VARIABLE("piid", particlePionId,
                      "DEPRECATED. Pion ID variable used before release 01-00-00. To be replaced by PID_piID");
    REGISTER_VARIABLE("Kid", particleKaonId,
                      "DEPRECATED. Kaon ID variable used before release 01-00-00. To be replaced by PID_KID");
    REGISTER_VARIABLE("prid", particleProtonId,
                      "DEPRECATED. Proton ID variable used before release 01-00-00. To be replaced by PID_prID");
    REGISTER_VARIABLE("did", particleDeuteronId,
                      "DEPRECATED. Deuteron ID variable used before release 01-00-00. To be replaced by PID_dID");

    REGISTER_VARIABLE("PIDKid_belle", particleKaonIdBelle, "DEPRECATED. kaon identification probability bellestyle");

    REGISTER_VARIABLE("K_vs_piid", particleKaonId, "DEPRECATED. kaon vs pion identification probability");
    REGISTER_VARIABLE("pi_vs_eid", particlePionvsElectronId, "DEPRECATED. pion vs electron identification probability");
    REGISTER_VARIABLE("pi_vs_edEdxid", particlePionvsElectrondEdxId,
                      "DEPRECATED. pion vs electron identification probability from dEdx measurement");

    REGISTER_VARIABLE("eid_dEdx", particleElectrondEdxId, "DEPRECATED. electron identification probability from dEdx measurement");
    REGISTER_VARIABLE("muid_dEdx", particleMuondEdxId, "DEPRECATED. muon identification probability from dEdx measurement");
    REGISTER_VARIABLE("piid_dEdx", particlePiondEdxId, "DEPRECATED. pion identification probability from dEdx measurement");
    REGISTER_VARIABLE("Kid_dEdx", particleKaondEdxId, "DEPRECATED. kaon identification probability from dEdx measurement");
    REGISTER_VARIABLE("prid_dEdx", particleProtondEdxId, "DEPRECATED. proton identification probability from dEdx measurement");

    REGISTER_VARIABLE("eid_TOP", particleElectronTOPId, "DEPRECATED. electron identification probability from TOP");
    REGISTER_VARIABLE("muid_TOP", particleMuonTOPId, "DEPRECATED. muon identification probability from TOP");
    REGISTER_VARIABLE("piid_TOP", particlePionTOPId, "DEPRECATED. pion identification probability from TOP");
    REGISTER_VARIABLE("Kid_TOP", particleKaonTOPId, "DEPRECATED. kaon identification probability from TOP");
    REGISTER_VARIABLE("prid_TOP", particleProtonTOPId, "DEPRECATED. proton identification probability from TOP");

    REGISTER_VARIABLE("eid_ARICH", particleElectronARICHId, "DEPRECATED. electron identification probability from ARICH");
    REGISTER_VARIABLE("muid_ARICH", particleMuonARICHId, "DEPRECATED. muon identification probability from ARICH");
    REGISTER_VARIABLE("piid_ARICH", particlePionARICHId, "DEPRECATED. pion identification probability from ARICH");
    REGISTER_VARIABLE("Kid_ARICH", particleKaonARICHId, "DEPRECATED. kaon identification probability from ARICH");
    REGISTER_VARIABLE("prid_ARICH", particleProtonARICHId, "DEPRECATED. proton identification probability from ARICH");

    REGISTER_VARIABLE("muid_KLM", particleMuonKLMId, "DEPRECATED. muon identification probability from KLM");

    REGISTER_VARIABLE("eid_ECL", particleElectronECLId, "DEPRECATED. electron identification probability from ECL");

    // These variables are not used anywhere in the committed code
    REGISTER_VARIABLE("missing_TOP", particleMissingTOPId, "DEPRECATED. 1.0 if identification probability from TOP is missing");
    REGISTER_VARIABLE("missing_ECL", particleMissingECLId, "DEPRECATED. 1.0 if identification probability from ECL is missing");
    REGISTER_VARIABLE("missing_ARICH", particleMissingARICHId, "DEPRECATED. 1.0 if identification probability from ARICH is missing");


  }
}
