/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>

namespace Belle2 {
  /** Utilities for converting PDG codes into particle names.
   *
   * Uses the TDatabasePDG class, which is filled from evt.pdl by basf2.
   */
  namespace EvtPDLUtil {
    /**
     * Checks if the particle with given pdg code has an anti-particle or not.
     *
     * @param pdgCode of the particle
     * @return true/false if anti-particle does/does not exist
     */
    bool hasAntiParticle(int pdgCode);

    /**
    * Returns the name of a particle with given pdg code.
    */
    std::string particleName(int pdgCode);

    /**
     * Returns the name of the anti-particle of a particle with given pdg code.
     * If the anti-particle does not exist the particle's name is returned instead.
     */
    std::string antiParticleName(int pdgCode);

    /**
     * Returns the name of the anti-particle ParticleList for particles with given pdg code and with given label.
     * If the anti-particle does not exist the particle's name is returned instead
     */
    std::string antiParticleListName(int pdgCode, const std::string& label);

    /**
     * Returns the name of the particle ParticleList for particles with given pdg code and with given label.
     */
    std::string particleListName(int pdgCode, const std::string& label);

    /**
     * Returns electric charge of a particle with given pdg code.
     */
    double charge(int pdgCode);

  }  // namespace EvtPDLUtil
}  // namespace Belle2
