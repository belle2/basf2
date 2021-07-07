/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/datastore/SelectSubset.h>
#include <analysis/dataobjects/Particle.h>

namespace Belle2 {
  /** Specialised SelectSubset<Particle> that also fixes daughter indices and all ParticleLists. Relations are already handled in SelectSubset.
   *
   * Can be used like SelectSubset via select(), or by calling removeParticlesNotInLists() instead.
   */
  class ParticleSubset : public SelectSubset<Particle> {
  public:
    /** Removes all Particles that are not in one of the given ParticleLists (or daughters of Particles in the lists).
     *
     * Removal is done immediately, there is no need to call select() afterwards.
     */
    void removeParticlesNotInLists(const std::vector<std::string>& listNames);

    /** select Particles for which f returns true, discard others */
    void select(std::function<bool (const Particle*)> f)
    {
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
    static void fixVector(std::vector<int>& vec, const std::map<int, int>& oldToNewMap);

  };
}
