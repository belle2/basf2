/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kiyoshi Hayasaka, Michel Villanueva                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef TAUDECAYMARKERMODULE_H
#define TAUDECAYMARKERMODULE_H

#include <TMath.h>
#include <string>
#include <vector>

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/GearDir.h>

#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/TauPairDecay.h>


namespace Belle2 {
  /**
   * Module to identify and label generated tau decays channels, using MCParticle information.
   *
   *    *
   */
  class TauDecayMarkerModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TauDecayMarkerModule();

    /** Initializes the module. */
    virtual void initialize();

    /** Method is called for each event. */
    virtual void event();

  private:

    /** True if the generated event is a tau pair event. */
    bool tau_pair;
    /** Number of positive tau leptons in the event */
    int no_of_tau_plus;
    /** Number of negative tau leptons in the event */
    int no_of_tau_minus;
    /** Index of the generated positive tau */
    int id_of_tau_plus;
    /** Index of the generated negative tau */
    int id_of_tau_minus;
    /** ID of the decay channel of positive tau */
    Int_t m_pmode;
    /** ID of the decay channel of negative tau*/
    Int_t m_mmode;

    /** Identifies if the event is a generated tau pair */
    void IdentifyTauPair();
    /** Gets the id of the decay channel */
    int getDecayChannelOfTau(int s = 0);
    /** Count the number of daughers of the generated tau except gammas */
    int getNumDaughterOfTauExceptGamma(int s = 0, int id = 0, int sign = 0);
    /** Count the number of daughers of the generated tau */
    int getNumDaughterOfTau(int s = 0, int id = 0, int sign = 0);

  };

}

#endif // TAUDECAYMARKERMODULE_H
