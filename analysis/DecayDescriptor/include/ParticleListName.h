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
     */
    std::string antiParticleListName(const std::string& listName);

  }
}
