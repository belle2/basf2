/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:N Sushree Ipsita,Vishal Bhardwaj                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>


namespace Belle2 {
  namespace Variable {
    double Pi0_Prob(double mass, double energy, double theta);
    double Eta_Prob(double mass, double energy, double theta);
    double Belle_Koppenberg_Pi0Veto(const Particle* particle);
    double Belle_Koppenberg_EtaVeto(const Particle* particle);



  }
}
