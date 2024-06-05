/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/modules/TrackingSystematics/TrackingEfficiency.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/ParticleList.h>

#include <map>
#include <TRandom.h>
#include <Math/Vector4D.h>

using namespace Belle2;

REG_MODULE(TrackingEfficiency);

TrackingEfficiencyModule::TrackingEfficiencyModule() : Module()
{
  setDescription(
    R"DOC(Module to remove tracks from the lists at random. Include in your code as

.. code:: python

    mypath.add_module("TrackingEfficiency", particleLists=['pi+:cut'], frac=0.01)

The module modifies the input particleLists by randomly removing tracks with the probability frac.
		     
		     )DOC");
  // Parameter definitions
  addParam("particleLists", m_ParticleLists, "input particle lists");
  addParam("frac", m_frac, "probability to remove the particle", 0.0);
}

void TrackingEfficiencyModule::event()
{
  // map from mdstIndex to decision
  std::map <unsigned, bool> indexToRemove;

  // determine list of mdst tracks:
  for (auto& iList : m_ParticleLists) {
    StoreObjPtr<ParticleList> particleList(iList);
    //check particle List exists and has particles
    if (!particleList) {
      B2ERROR("ParticleList " << iList << " not found");
      continue;
    }

    if (!Const::chargedStableSet.contains(Const::ParticleType(abs(particleList->getPDGCode())))) {
      B2ERROR("The provided particlelist " << iList << " does not contain track-based particles.");
    }

    std::vector<unsigned int> toRemove;
    size_t nPart = particleList->getListSize();
    for (size_t iPart = 0; iPart < nPart; iPart++) {
      auto particle = particleList->getParticle(iPart);
      unsigned mdstIndex = particle->getMdstArrayIndex();
      bool remove;
      if (indexToRemove.find(mdstIndex) !=  indexToRemove.end()) {
        // found, use entry
        remove = indexToRemove.at(mdstIndex);
      } else {
        // not found, generate and store it
        auto prob = gRandom->Uniform();
        remove = prob < m_frac;
        indexToRemove.insert(std::pair{mdstIndex, remove});
      }
      if (remove) toRemove.push_back(particle->getArrayIndex());
    }
    particleList->removeParticles(toRemove);
  }
}

