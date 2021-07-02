/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/dataobjects/TauPairDecay.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/MCParticle.h>

namespace Belle2 {
  /**
   * Module to identify generated tau pair decays, using MCParticle information. Each tau lepton decay channel
   * is numbered following the order in the default KKMC decay table. Using this module,
   * the channel number will be stored in the variables `tauPlusMCMode`, and `tauMinusMCMode`.
   * Further details and usage can be found at `TauDecayMCModes`.
   *
   */
  class TauDecayMarkerModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TauDecayMarkerModule();

    /** Initializes the module. */
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;

  private:

    /** pointer to tau pair decay objects */
    StoreObjPtr<TauPairDecay> m_tauDecay;
    /** StoreArray of MCParticles */
    StoreArray<MCParticle> m_MCParticles;
    /** True if the generated event is a tau pair event. */
    bool tauPair;
    /** Number of positive tau leptons in the event */
    int numOfTauPlus;
    /** Number of negative tau leptons in the event */
    int numOfTauMinus;
    /** Index of the generated positive tau */
    int idOfTauPlus;
    /** Index of the generated negative tau */
    int idOfTauMinus;
    /** PDG codes accepted as charged final state particles in generation: {e, mu, pi, K, p} */
    const int finalStatePDGs[5] = { 11, 13, 211, 321, 2212 };
    /** ID of the decay channel of positive tau */
    Int_t m_pmode;
    /** ID of the decay channel of negative tau*/
    Int_t m_mmode;
    /** Prong of the decay channel of positive tau */
    Int_t m_pprong;
    /** Prong of the decay channel of negative tau*/
    Int_t m_mprong;

    /** Identifies if the event is a generated tau pair */
    void IdentifyTauPair();
    /** Gets the id of the decay channel */
    int getDecayChannelOfTau(int s = 0);
    /** Count the number of daughers of the generated tau except gammas */
    int getNumDaughterOfTauExceptGamma(int s = 0, int id = 0, int sign = 0);
    /** Count the number of daughers of the generated tau */
    int getNumDaughterOfTau(int s = 0, int id = 0, int sign = 0);
    /** Get the prong of the generated tau decay */
    int getProngOfDecay(const MCParticle& mc);

  protected:
    /** If true, prints ID and prong of each tau lepton in the event.*/
    bool m_printDecayInfo;

  };

}
