/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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


