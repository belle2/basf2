/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/RemoveParticlesNotInLists/RemoveParticlesNotInLists.h>
#include <analysis/dataobjects/ParticleList.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>

using namespace std;
using namespace Belle2;


REG_MODULE(RemoveParticlesNotInLists)

RemoveParticlesNotInListsModule::RemoveParticlesNotInListsModule(): m_nRemoved(0), m_nTotal(0)
{
  setDescription("Removes all Particles that are not in one of the given ParticleLists (or daughters of Particles in the lists). All relations from/to Particles, daughter indices, and other ParticleLists are fixed. Note that this does not currently touch any data used to create final state particles, which might make up a large fraction of the total file size.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("particleLists", m_particleLists, "Keep the Particles and their daughters in these ParticleLists.");
}

void RemoveParticlesNotInListsModule::initialize()
{
  m_particles.isRequired();
  m_subset.registerSubset(m_particles);

  for (const auto& l : m_particleLists) {
    StoreObjPtr<ParticleList>(l).isRequired();
  }
}

void RemoveParticlesNotInListsModule::event()
{
  const int nBefore = m_particles.getEntries();

  m_subset.removeParticlesNotInLists(m_particleLists);

  const int nAfter = m_particles.getEntries();

  m_nTotal += nBefore;
  m_nRemoved += nBefore - nAfter;
}

void RemoveParticlesNotInListsModule::terminate()
{
  double perc = double(m_nRemoved) / m_nTotal * 100.0;
  B2INFO(getName() << ": removed " << m_nRemoved << " Particles (" << perc << " % of total amount).");
}
