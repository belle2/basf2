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

    /** free memory */
    virtual ~TauDecayMarkerModule();

    /** Initializes the module. */
    virtual void initialize();

    /** Method is called for each event. */
    virtual void event();

    /** Define run parameters. */
    virtual void beginRun();

    /** Finish the run. */
    virtual void endRun();

    /** finish the execution  */
    virtual void terminate();


  private:
    std::string m_particleList;
    std::vector<int> isr_list;
    bool tau_pair;
    int no_of_tau;
    int no_of_tau_plus;
    int no_of_tau_minus, no_of_ISR;
    int id_of_tau_plus;
    int id_of_tau_minus;
    int size_of_gen_hepevt;
    Int_t m_pmode;
    Int_t m_mmode;



    void my_tau_pair();
    int get_no_of_decay_channel_of_tau(int s = 0);
    int get_no_of_daughter_of_tau_except_gamma(int s = 0, int id = 0, int sign = 0);
    int get_no_of_daughter_of_tau(int s = 0, int id = 0, int sign = 0);


  };

}

#endif // TAUDECAYMARKERMODULE_H
