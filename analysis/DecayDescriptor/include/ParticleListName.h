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
  /** Helper to deal with ParticleList names. Own namespace because of CINT. */
  namespace ParticleListName {
    /** Returns name of anti-particle-list corresponding to listName.
     *
     * Throws std::runtime_error for invalid particle lists.
     * If listName refers to a self-conjugated list, simply returns listName.
     *
     * Ex: antiParticleListName("B+:somelabel") -> "B-:somelabel"
     *     antiParticleListName("gamma") -> "gamma"
     *
     * User documentation is located at analysis/doc/DecayDescriptor.rst
     * Please modify in accordingly to introduced changes.
     */
    std::string antiParticleListName(const std::string& listName);

  }
}
