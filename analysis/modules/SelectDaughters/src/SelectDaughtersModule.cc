/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/SelectDaughters/SelectDaughtersModule.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/dbobjects/BeamParameters.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

// utilities
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ParticleCopy.h>

// Magnetic field
#include <framework/geometry/BFieldManager.h>

#include <TMath.h>



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

SelectDaughtersModule::~SelectDaughtersModule()
{
}

void SelectDaughtersModule::initialize()
{
  if (m_decayString != "") {
    m_decaydescriptor.init(m_decayString);
  }
}

void SelectDaughtersModule::beginRun()
{
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
    std::vector<std::string> selParticlesName = m_decaydescriptor.getSelectionNames();

    for (unsigned idau = 0; idau < daughters.size(); idau++) {
      bool isSel = false;
      for (unsigned isel = 0; isel < selParticles.size(); isel++) {
        if (daughters[idau] == selParticles[isel]) isSel = true;
      }
      if (!isSel) particle->removeDaughter(daughters[idau]);
    }
  }
}

void SelectDaughtersModule::endRun()
{
}

void SelectDaughtersModule::terminate()
{
}


