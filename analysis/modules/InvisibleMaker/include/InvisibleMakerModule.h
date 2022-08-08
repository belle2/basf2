/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Lukas Bierwirth                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once
#include <framework/core/Module.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/VariableManager/Manager.h>

#include <string>

namespace Belle2 {

  /** This module replaces the mass of the particles inside the given particleLists
  with the invariant mass of the particle corresponding to the given pdgCode. */

  class InvisibleMakerModule : public Module {
  private:
    /** Name of the mother list */
    std::string m_strMotherParticleList;

    /** DecayString specifying the target Particle which will be made invisible */
    std::string m_decayStringTarget;

    /** DecayDescriptor specifying the daughter which will be made invisible */
    DecayDescriptor m_pDDescriptorTarget;

    /**Value of the diagonal covariance matrix of the target Particle*/
    double m_dummyCovMatrix;

    /**If true, the four-momemtum of the invisible particle will be set to zero*/
    bool m_eraseFourMomentum;

  public:
    /** Constructor. */
    InvisibleMakerModule();

    /** Initialises the module.
     */
    virtual void initialize() override;
    /** Method called for each event. */
    virtual void event() override;
    /** Write TTree to file, and close file if necessary. */
    virtual void terminate() override;
  };
} // end namespace Belle2


