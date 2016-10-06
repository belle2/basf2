/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleCopier/ParticleCopierModule.h>


// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

// utilities
#include <analysis/utility/ParticleCopy.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticleCopier)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticleCopierModule::ParticleCopierModule() : Module()
  {
    setDescription("Replaces each Particle in the ParticleList with its copy.\n"
                   "Particle's (grand)^n-daughter Particles are copied as well.\n"
                   "The existing relations of the original Particle (or it's (grand-)^n-daughters)\n"
                   "are copied as well.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    vector<string> defaultList;
    addParam("inputListNames", m_inputListNames,
             "list of input ParticleList names", defaultList);
  }

  void ParticleCopierModule::initialize()
  {
    // Input lists
    for (const std::string& listName : m_inputListNames) {
      StoreObjPtr<ParticleList>().isRequired(listName);
    }
  }

  void ParticleCopierModule::event()
  {
    const StoreArray<Particle> particles;

    // copy all particles from input lists that pass selection criteria into plist
    for (unsigned iList = 0; iList < m_inputListNames.size(); iList++) {
      const StoreObjPtr<ParticleList> plist(m_inputListNames[iList]);

      const unsigned int origSize = plist->getListSize();
      std::vector<Particle*> copies(origSize);

      for (unsigned i = 0; i < origSize; i++) {
        const Particle* origP = plist->getParticle(i);

        // copy the particle
        Particle* copyP = ParticleCopy::copyParticle(origP);
        copies.at(i) = copyP;
      }

      // clear the original list and fill it with copies
      plist->clear();
      for (unsigned i = 0; i < origSize; i++)
        plist->addParticle(copies[i]);

      unsigned int copySize = plist->getListSize();

      if (copySize != origSize)
        B2FATAL("Size of the ParticleList " << m_inputListNames[iList]
                << " has changed while copying the Particles! original size = "
                << origSize << " vs. new size = " << copySize);

    }
  }
} // end Belle2 namespace

