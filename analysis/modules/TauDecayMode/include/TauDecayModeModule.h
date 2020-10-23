/**************************************************************************
    * BASF2 (Belle Analysis Framework 2)                                     *
    * Copyright(C) 2020 - Belle II Collaboration                             *
    *                                                                        *
    * Author: The Belle II Collaboration                                     *
    * Contributors: Leonardo Salinas                                         *
    *                                                                        *
    * This software is provided "as is" without any warranty.                *
    **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/VariableManager/Utility.h>
#include <TVector3.h>
#include "TFile.h"
#include "TBox.h"
#include "TDirectory.h"
#include <iostream>
#include <set>
#include <vector>
#include <algorithm>
#include <map>
#include <fstream>
#include <string>
namespace Belle2 {
  class TauDecayModeModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TauDecayModeModule();

    /** Initializes the module. */
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;
    //! Terminate at the end of job
    virtual void terminate();

  private:

    int EventNumber;

    std::map<std::string, int> mode_decay;

    std::string m_tauminusdecaymode;
    std::string m_tauplusdecaymode;



    std::vector<int> vec_em, vec_ep, vec_nue, vec_anue;
    std::vector<int> vec_mum, vec_mup, vec_numu, vec_anumu;
    std::vector<int> vec_nut, vec_anut;
    std::vector<int> vec_pim, vec_pip, vec_km, vec_kp, vec_apro, vec_pro;
    std::vector<int> vec_pi0, vec_k0s, vec_k0l, vec_gam;
    std::vector<int> vec_eta, vec_omega, vec_kstarp, vec_kstarm;
    std::vector<int> vec_lambda, vec_lmb_br, vec_kstar, vec_kstar_br, vec_etapr;//new particle vector
    std::vector<int> vec_a0p, vec_a0m, vec_b1m, vec_b1p, vec_phi, vec_f1, vec_a1m, vec_a1p, vec_rhom, vec_rhop ;
    std::vector<int> vec_K0, vec_K0_br,  vec_rho0, vec_f0;
    std::vector<int> vec_dau_tauminus;
    std::vector<int> vec_dau_tauplus;


    double getEnergyTauRestFrame(const MCParticle* mc, const int ichg);
    int getRecursiveMotherCharge(const MCParticle* mc);
    int getRecursiveMother(const MCParticle* mc);
    int TauBBBmode(std::string s, std::map<std::string, int> tau_map);

  protected:

    int m_printmode;


  };

}
