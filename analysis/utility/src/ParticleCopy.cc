/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/utility/ParticleCopy.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/datastore/StoreArray.h>
#include <iostream>

using namespace Belle2;
using namespace std;

Particle* ParticleCopy::copyParticle(const Particle* original)
{
  StoreArray<Particle> array(original->getArrayName());

  // make a copy of a particle
  Particle* copy = array.appendNew(*original);
  copy->copyRelations(original);

  // Copy its daughters as well.
  // At this stage the copy of the particle
  // internally stores daughter indices of original.
  // Copy daughters as well.
  unsigned nDaughters = original->getNDaughters();

  // If the particle has undergone Bremsstrahlung correction, removing its
  // daughters (the original lepton and potential photons) and then appending
  // the copied versions should not change its source type.
  // Or, if the particle's source is V0, it should be kept as V0 rather than changing to Composite.
  bool updateType = true;
  if (copy->hasExtraInfo("bremsCorrected") ||
      copy->getParticleSource() == Particle::EParticleSourceObject::c_V0)
    updateType = false;

  for (unsigned iOld = 0; iOld < nDaughters; iOld++) {
    const Particle* originalDaughter = original->getDaughter(iOld);

    Particle* daughterCopy = copyParticle(originalDaughter);

    // remove original daughter
    copy->removeDaughter(originalDaughter, updateType);
    // append copied daughter instead
    copy->appendDaughter(daughterCopy, updateType);
  }

  return copy;
}

void ParticleCopy::copyDaughters(Belle2::Particle* mother)
{
  // If the particle has undergone Bremsstrahlung correction, removing its
  // daughters (the original lepton and potential photons) and then appending
  // the copied versions should not change its source type.
  // Or, if the particle's source is V0, it should be kept as V0 rather than changing to Composite.
  bool updateType = true;
  if (mother->hasExtraInfo("bremsCorrected") ||
      mother->getParticleSource() == Particle::EParticleSourceObject::c_V0)
    updateType = false;

  unsigned nDaughters = mother->getNDaughters();
  for (unsigned iOld_neverUsed = 0; iOld_neverUsed < nDaughters; iOld_neverUsed++) {
    // always accessing the first daughter of mother that is being updated
    const Particle* originalDaughter = mother->getDaughter(0);
    Particle* daughterCopy = copyParticle(originalDaughter);

    // remove original daughter from the begging of the daughters vector
    mother->removeDaughter(originalDaughter, updateType);
    // append copied daughter instead at the end of the daughters vector
    mother->appendDaughter(daughterCopy, updateType);
  }
}
