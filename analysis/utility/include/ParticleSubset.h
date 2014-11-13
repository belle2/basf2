#pragma once

#include <framework/datastore/SelectSubset.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

namespace Belle2 {
  /** Specialised SelectSubset<Particle> that also fixes daughter indices and all ParticleLists. */
  class ParticleSubset : public SelectSubset<Particle> {
  public:
    /** select Particles for which f returns true, discard others */
    void select(std::function<bool (const Particle*)> f) {
      const std::map<int, int>& oldToNewMap = copySetWithRelations(f);

      if (m_inheritToSelf) {
        copyRelationsToSelf();
      }

      if (m_reduceExistingSet) {
        swapSetsAndDestroyOriginal();
      }

      fixParticles(oldToNewMap);

      fixParticleLists(oldToNewMap);
    }

  protected:
    /** fix daughter indices, reset m_arrayPointer */
    void fixParticles(const std::map<int, int>& oldToNewMap);

    /** fix contents of particlelists by updating indices or removing entries. */
    void fixParticleLists(const std::map<int, int>& oldToNewMap);

    /** replace entries in vec via oldToNewMap, removing those not found. */
    void fixVector(std::vector<int>& vec, const std::map<int, int>& oldToNewMap);
  };
}
