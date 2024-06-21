/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/core/Module.h>

namespace Belle2 {

  /** This module creates a new particle list with the desired pdg code using tracks from an input particle list (basically updates the mass hypotheses of the input list). */

  class ParticleMassHypothesesUpdaterModule : public Module {
  private:
    /** PDG code for mass reference **/
    int m_newPdgCode;

    /** Name of the list */
    std::string m_particleList;

    /** Writeout flag */
    bool m_writeOut;

    /** Name of the new created list */
    std::string m_newParticleList;
    /** Name of the new created anti-list */
    std::string m_newAntiParticleList;

  public:
    /** Constructor. */
    ParticleMassHypothesesUpdaterModule();

    /** Initialises the module. */
    virtual void initialize() override;
    /** Method called for each event. */
    virtual void event() override;
    /** Write TTree to file, and close file if necessary. */
    virtual void terminate() override;
  };
} // end namespace Belle2


