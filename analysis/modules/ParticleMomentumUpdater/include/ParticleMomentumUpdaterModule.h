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

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <string>

namespace Belle2 {

  /** This module replaces the momentum of the Particle in the target particle list
  by p(beam) - p(selected daughters). */

  class ParticleMomentumUpdaterModule : public Module {
  private:
    /** name of input particle list. */
    std::string m_particleList;
    /** DecayString specifying the target Particle whose momentum will be updated */
    std::string m_decayStringTarget;
    /** DecayString specifying the daughters used to update the momentum of the target particle */
    std::string m_decayStringDaughters;
    /** Decay descriptor of the daughter particles */
    DecayDescriptor m_pDDescriptorDaughters;
    /** Decay descriptor of the target particles */
    DecayDescriptor m_pDDescriptorTarget;

  public:
    /** Constructor. */
    ParticleMomentumUpdaterModule();

    /** Initialises the module.
     */
    virtual void initialize() override;
    /** Method called for each event. */
    virtual void event() override;
    /** Write TTree to file, and close file if necessary. */
    virtual void terminate() override;
  };
} // end namespace Belle2


