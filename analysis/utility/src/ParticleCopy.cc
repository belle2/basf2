// ******************************************************************
// Particle copy utility
// authors: A. Zupanc (anze.zupanc@ijs.si)
// ******************************************************************

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
  // internally stores daughter indicies of original.
  // Copy daughters as well.
  unsigned nDaughters = original->getNDaughters();

  for (unsigned iOld = 0; iOld < nDaughters; iOld++) {
    const Particle* originalDaughter = original->getDaughter(iOld);

    Particle* daughterCopy = copyParticle(originalDaughter);

    // If the particle has undergone Bremsstrahlung correction, removing its
    // daughters (the original lepton and potential photons) and then appending
    // the copied versions should not change its source type.
    bool updateType = true;
    if (copy->hasExtraInfo("bremsCorrected")) updateType = false;
    // remove original daughter
    copy->removeDaughter(originalDaughter, updateType);
    // append copied daughter instead
    copy->appendDaughter(daughterCopy, updateType);
  }

  return copy;
}

void ParticleCopy::copyDaughters(Belle2::Particle* mother)
{
  unsigned nDaughters = mother->getNDaughters();
  for (unsigned iOld = 0; iOld < nDaughters; iOld++) {
    const Particle* originalDaughter = mother->getDaughter(0);
    Particle* daughterCopy = copyParticle(originalDaughter);

    // remove original daughter
    mother->removeDaughter(originalDaughter);
    // append copied daughter instead
    mother->appendDaughter(daughterCopy);
  }
}
