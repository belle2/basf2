/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/PIDVariables.h>

#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/PIDLikelihood.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

    double deltaLogL(const Particle* part, const Const::ChargedStable& otherHypothesis)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.0;

      Const::ChargedStable thisType = Const::ChargedStable(abs(part->getPDGCode()));

      return pid->getLogL(thisType) - pid->getLogL(otherHypothesis);
    }

    double particleDeltaLogLPion(const Particle* part)
    {
      return deltaLogL(part, Const::pion);
    }

    double particleDeltaLogLKaon(const Particle* part)
    {
      return deltaLogL(part, Const::kaon);
    }

    double particleDeltaLogLProton(const Particle* part)
    {
      return deltaLogL(part, Const::proton);
    }

    double particleDeltaLogLElectron(const Particle* part)
    {
      return deltaLogL(part, Const::electron);
    }

    double particleDeltaLogLMuon(const Particle* part)
    {
      return deltaLogL(part, Const::muon);
    }

    double particleElectronId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      return pid->getProbability(Const::electron, Const::pion);
    }

    double particlePionvsElectronId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      return pid->getProbability(Const::pion, Const::electron);
    }

    double particlePionvsElectrondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::pion, Const::electron, set);
    }

    double particleElectrondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::electron, Const::pion, set);
    }

    double particleElectronTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::electron, Const::pion, set);
    }

    double particleElectronARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::electron, Const::pion, set);
    }

    double particleElectronECLId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ECL;
      return pid->getProbability(Const::electron, Const::pion, set);
    }

    double particleMuonId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      return pid->getProbability(Const::muon, Const::pion);
    }

    double particleMuondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::muon, Const::pion, set);

    }

    double particleMuonTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::muon, Const::pion, set);
    }

    double particleMuonARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::muon, Const::pion, set);
    }

    double particleMuonKLMId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::KLM;
      return pid->getProbability(Const::muon, Const::pion, set);
    }

    double particlePionId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      return pid->getProbability(Const::pion, Const::kaon);
    }

    double particlePiondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::pion, Const::kaon, set);
    }

    double particlePionTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::pion, Const::kaon, set);
    }

    double particlePionARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::pion, Const::kaon, set);
    }

    double particleKaonId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      return pid->getProbability(Const::kaon, Const::pion);
    }

    double particleKaondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::kaon, Const::pion, set);
    }

    double particleKaonTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::kaon, Const::pion, set);
    }

    double particleKaonARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::kaon, Const::pion, set);
    }

    double particleProtonId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      return pid->getProbability(Const::proton, Const::pion);
    }

    double particleProtondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::proton, Const::pion, set);
    }

    double particleProtonTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::proton, Const::pion, set);
    }

    double particleProtonARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::proton, Const::pion, set);
    }

    double particleMissingARICHId(const Particle* part)
    {
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

    VARIABLE_GROUP("PID");
    REGISTER_VARIABLE("DLLPion", particleDeltaLogLPion,     "Delta Log L = L(particle's hypothesis) - L(pion)");
    REGISTER_VARIABLE("DLLKaon", particleDeltaLogLKaon,     "Delta Log L = L(particle's hypothesis) - L(kaon)");
    REGISTER_VARIABLE("DLLProt", particleDeltaLogLProton,   "Delta Log L = L(particle's hypothesis) - L(proton)");
    REGISTER_VARIABLE("DLLElec", particleDeltaLogLElectron, "Delta Log L = L(particle's hypothesis) - L(electron)");
    REGISTER_VARIABLE("DLLMuon", particleDeltaLogLMuon,     "Delta Log L = L(particle's hypothesis) - L(muon)");

    REGISTER_VARIABLE("eid", particleElectronId, "electron identification probability");
    REGISTER_VARIABLE("muid", particleMuonId, "muon identification probability");
    REGISTER_VARIABLE("piid", particlePionId, "pion identification probability");
    REGISTER_VARIABLE("Kid", particleKaonId, "kaon identification probability");
    REGISTER_VARIABLE("Kid_belle", particleKaonIdBelle, "kaon identification probability bellestyle");
    REGISTER_VARIABLE("prid", particleProtonId, "proton identification probability");

    REGISTER_VARIABLE("K_vs_piid", particleKaonId, "kaon vs pion identification probability");
    REGISTER_VARIABLE("pi_vs_eid", particlePionvsElectronId, "pion vs electron identification probability");
    REGISTER_VARIABLE("pi_vs_edEdxid", particlePionvsElectrondEdxId,
                      "pion vs electron identification probability from dEdx measurement");

    REGISTER_VARIABLE("eid_dEdx", particleElectrondEdxId, "electron identification probability from dEdx measurement");
    REGISTER_VARIABLE("muid_dEdx", particleMuondEdxId, "muon identification probability from dEdx measurement");
    REGISTER_VARIABLE("piid_dEdx", particlePiondEdxId, "pion identification probability from dEdx measurement");
    REGISTER_VARIABLE("Kid_dEdx", particleKaondEdxId, "kaon identification probability from dEdx measurement");
    REGISTER_VARIABLE("prid_dEdx", particleProtondEdxId, "proton identification probability from dEdx measurement");

    REGISTER_VARIABLE("eid_TOP", particleElectronTOPId, "electron identification probability from TOP");
    REGISTER_VARIABLE("muid_TOP", particleMuonTOPId, "muon identification probability from TOP");
    REGISTER_VARIABLE("piid_TOP", particlePionTOPId, "pion identification probability from TOP");
    REGISTER_VARIABLE("Kid_TOP", particleKaonTOPId, "kaon identification probability from TOP");
    REGISTER_VARIABLE("prid_TOP", particleProtonTOPId, "proton identification probability from TOP");
    REGISTER_VARIABLE("missing_TOP", particleMissingTOPId, "1.0 if identification probability from TOP is missing");

    REGISTER_VARIABLE("eid_ARICH", particleElectronARICHId, "electron identification probability from ARICH");
    REGISTER_VARIABLE("muid_ARICH", particleMuonARICHId, "muon identification probability from ARICH");
    REGISTER_VARIABLE("piid_ARICH", particlePionARICHId, "pion identification probability from ARICH");
    REGISTER_VARIABLE("Kid_ARICH", particleKaonARICHId, "kaon identification probability from ARICH");
    REGISTER_VARIABLE("prid_ARICH", particleProtonARICHId, "proton identification probability from ARICH");
    REGISTER_VARIABLE("missing_ARICH", particleMissingARICHId, "1.0 if identification probability from ARICH is missing");

    REGISTER_VARIABLE("muid_KLM", particleMuonKLMId, "muon identification probability from KLM");

    REGISTER_VARIABLE("eid_ECL", particleElectronECLId, "electron identification probability from ECL");
    REGISTER_VARIABLE("missing_ECL", particleMissingECLId, "1.0 if identification probability from ECL is missing");

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
  }
}
