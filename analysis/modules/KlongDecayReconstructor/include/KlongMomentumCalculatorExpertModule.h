/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/ParticleCombiner/ParticleCombiner.h>

#include <string>

namespace Belle2 {

  /**
   * reco missing module
   */
  class KlongMomentumCalculatorExpertModule : public Module {

  public:

    /**
     * Constructor
     */
    KlongMomentumCalculatorExpertModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    StoreObjPtr<ParticleList> m_koutputList; /**< Klong output particle list */

    std::string m_decayString;   /**< Input DecayString specifying the decay being reconstructed */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the decay being reconstructed */
    std::unique_ptr<ParticleGenerator> m_generator; /**< Generates the combinations */

    std::string m_klistName;   /**< output K_L0 particle list name */

    bool m_writeOut;  /**< toggle output particle list btw. transient/writeOut */

    std::string m_recoList; /**< Suffix attached to the output K_L list, if not defined it is set to '_reco'  */

    int m_maximumNumberOfCandidates; /**< drop all candidates if more candidates than this parameter are produced */

  };

} // Belle2 namespace

//#endif
