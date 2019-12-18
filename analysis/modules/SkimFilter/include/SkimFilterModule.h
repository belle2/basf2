/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /** This module filters events based on presence of candidates in a list of ParticleLists.
  For detailed Particle List statistics use the ParticleStats module. */

  class SkimFilterModule : public Module {
  private:
    /** Number of events with Particle candidates**/
    int m_nPass;
    /** Name of the lists */
    std::vector<std::string> m_strParticleLists;

  public:
    /** Constructor. */
    SkimFilterModule();

    /** Initialises the module.
     */
    virtual void initialize() override;
    /** Method called for each event. */
    virtual void event() override;
    /** Write TTree to file, and close file if necessary. */
    virtual void terminate() override;
  };
} // end namespace Belle2


