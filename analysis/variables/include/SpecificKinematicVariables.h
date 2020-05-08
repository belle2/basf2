/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sviat Bilokin                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <analysis/VariableManager/Manager.h>
#include <TLorentzVector.h>
#include <string>
#include <vector>

namespace Belle2 {
  class Particle;

  namespace Variable {
    /**
    * Returns the momentum transfer squared, q^2, calculated in CMS as q^2 = (p_B - p_h)^2,
    * where p_h is the CMS momentum of all hadrons in the decay B -> H_1 ... H_n ell nu_ell
    * The B meson momentum in CMS is assumed to be 0.
    */
    double REC_q2BhSimple(const Particle* particle);

    /**
     * Returns the momentum transfer squared, q^2, calculated in CMS as q^2 = (p_B - p_h)^2,
     * where p_h is the CMS momentum of all hadrons in the decay B -> H_1 ... H_n ell nu_ell
     * This calculation uses a weighted average of the B meson around the reco B cone
     */
    double REC_q2Bh(const Particle* particle);


    /**
     * Returns the invariant mass squared of the missing momentum calculated
     * assumings the reco B is at rest and calculating the neutrino ("missing") momentum from p_nu = pB - p_had - p_lep.
     */
    double REC_MissM2(const Particle* particle);

  }
}
