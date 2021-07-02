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

  /** This module replaces the mass of the particles inside the given particleLists
  with the invariant mass of the particle corresponding to the given pdgCode. */

  class ParticleMassUpdaterModule : public Module {
  private:
    /** PDG code for mass reference **/
    int m_pdgCode;

    /** Name of the lists */
    std::vector<std::string> m_strParticleLists;

  public:
    /** Constructor. */
    ParticleMassUpdaterModule();

    /** Initialises the module.
     */
    virtual void initialize() override;
    /** Method called for each event. */
    virtual void event() override;
    /** Write TTree to file, and close file if necessary. */
    virtual void terminate() override;
  };
} // end namespace Belle2


