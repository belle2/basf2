/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

namespace Belle2 {
  /**
   * Calculates 4-momentum of a neutral hadron in a given decay chain e.g. B0 -> J/Psi K_L0, or anti-B0 -> p+ K- anti-n0.
   */
  class NeutralHadron4MomentumCalculatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    NeutralHadron4MomentumCalculatorModule();

    /** Initialization */
    void initialize() override;

    /** Action to perform for each event */
    void event() override;

  private:

    std::string m_decayString;  /**< Decay string for which one wants to perform the calculation */
    DecayDescriptor m_decayDescriptor; /**< Decay Descriptor to be initialized with m_decayString */
    int m_iNeutral = 0; /**< Index of the neutral hadron in the decay string */
    StoreObjPtr<ParticleList> m_plist;  /**< ParticleList that one wants to perform the calculation */
    bool m_fAllowNonNeutralHadron;  /** <  Whether to check the name of the selected particle */
  };
}
