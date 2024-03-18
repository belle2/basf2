/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/core/Module.h>
//#include <string>

namespace Belle2 {

  /** This module replaces the mass hypotheses of the particles inside the given particleList with the given pdgCode. */

  class ParticleMassHypothesesUpdaterModule : public Module {
  private:
    /** PDG code for mass reference **/
    int m_pdgCode;

    /** Name of the list */
    std::string m_particleList;

    /** Name of the new created list */
    std::string m_newParticleList;

  public:
    /** Constructor. */
    ParticleMassHypothesesUpdaterModule();

    /** Initialises the module.
     */
    virtual void initialize() override;
    /** Method called for each event. */
    virtual void event() override;
    /** Write TTree to file, and close file if necessary. */
    virtual void terminate() override;
  };
} // end namespace Belle2


