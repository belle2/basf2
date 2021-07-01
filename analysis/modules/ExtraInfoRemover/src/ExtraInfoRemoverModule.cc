/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/ExtraInfoRemover/ExtraInfoRemoverModule.h>
#include <analysis/dataobjects/ParticleList.h>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(ExtraInfoRemover)

ExtraInfoRemoverModule::ExtraInfoRemoverModule() : Module()
{
  //Set module properties
  setDescription("Deletes the ExtraInfo from each particle in the given ParticleLists.");
  setPropertyFlags(c_ParallelProcessingCertified);
  //Parameter definition
  addParam("particleLists", m_strParticleLists, "List of ParticleLists", vector<string>());
  addParam("removeEventExtraInfo", m_removeEventExtraInfo, "If True, also eventExtraInfo will be removed", false);

}

void ExtraInfoRemoverModule::initialize()
{
}

void ExtraInfoRemoverModule::event()
{

  for (auto& iList :  m_strParticleLists) {

    StoreObjPtr<ParticleList> particleList(iList);
    if (!particleList.isValid()) {
      B2INFO("ParticleList " << iList << " not found");
      continue;
    } else {
      for (unsigned int i = 0; i < particleList->getListSize(); ++i) {
        Particle* iParticle =  particleList ->getParticle(i);
        iParticle->removeExtraInfo();
      }
    }
  }

  if (m_removeEventExtraInfo)  {
    m_eventExtraInfo->removeExtraInfo();
  }

}
