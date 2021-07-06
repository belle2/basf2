/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/SelectDaughters/SelectDaughtersModule.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

// Magnetic field
#include <framework/geometry/BFieldManager.h>

using namespace std;

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SelectDaughters)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SelectDaughtersModule::SelectDaughtersModule() : Module()
{
  // Set module properties
  setDescription("SelectDaughters");

  //Parameter definitions
  addParam("listName", m_listName, "name of particle list", string(""));
  addParam("decayString", m_decayString, "specifies which daughter particles will remain", string(""));

}

void SelectDaughtersModule::initialize()
{
  if (m_decayString != "") {
    m_decaydescriptor.init(m_decayString);
  }
}

void SelectDaughtersModule::event()
{

  StoreObjPtr<ParticleList> plist(m_listName);
  if (!plist) {
    B2ERROR("ParticleList " << m_listName << " not found");
    return;
  }
  if (m_decayString == "") {
    B2ERROR("decay descriptor cannot be empty");
    return;
  }

  unsigned int n = plist->getListSize();
  for (unsigned i = 0; i < n; i++) {
    Particle* particle = plist->getParticle(i);

    std::vector<Particle*> daughters = particle->getDaughters();
    std::vector<const Particle*> selParticles = m_decaydescriptor.getSelectionParticles(particle);

    for (auto& daughter : daughters) {
      bool isSel = false;
      for (auto& selParticle : selParticles) {
        if (daughter == selParticle) isSel = true;
      }
      if (!isSel) particle->removeDaughter(daughter);
    }
  }
}
