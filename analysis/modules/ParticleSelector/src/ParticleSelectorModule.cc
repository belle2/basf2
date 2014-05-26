/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleSelector/ParticleSelectorModule.h>

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>


using namespace std;

namespace Belle2 {

  bool ParticleSelectorModule::m_printVariables(true);

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticleSelector)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticleSelectorModule::ParticleSelectorModule() : Module()

  {
    // set module description (e.g. insert text)
    setDescription("Performs particle selection");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("PDG", m_pdg,
             "PDG code. If set to zero, particle list assumed to exist", 0);
    vector<string> defaultOtherLists;
    addParam("fromOtherLists", m_otherLists,
             "Select all particle from the given lists instead of the Particle StoreArray", defaultOtherLists);
    addParam("ListName", m_listName, "name of particle list", string(""));
    vector<string> defaultSelection;
    addParam("Select", m_selection, "selection criteria", defaultSelection);
    addParam("persistent", m_persistent,
             "toggle newly created particle list btw. transient/persistent", false);

  }

  ParticleSelectorModule::~ParticleSelectorModule()
  {
  }

  void ParticleSelectorModule::initialize()
  {

    if (m_printVariables) {
      cout << "--------------------------------------------------------\n";
      cout << "Modular analysis: list of available selection variables:\n";
      cout << "--------------------------------------------------------\n";
      m_pSelector.listVariables();
      cout << "--------------------------------------------------------\n";
      m_printVariables = false;
    }

    if (m_pdg != 0) {
      if (m_persistent) {
        StoreObjPtr<ParticleList>::registerPersistent(m_listName);
      } else {
        StoreObjPtr<ParticleList>::registerTransient(m_listName);
      }
    }

    for (unsigned int i = 0; i < m_otherLists.size(); i++) {
      StoreObjPtr<ParticleList>::required(m_otherLists[i]);
    }

    for (unsigned int i = 0; i < m_selection.size(); i++) {
      m_pSelector.addSelection(m_selection[i]);
    }
    std::string cuts;
    m_pSelector.listCuts(cuts);
    if (cuts.empty()) cuts = "(all)";
    B2INFO("ParticleSelector: " << m_listName << " " << cuts);
  }

  void ParticleSelectorModule::beginRun()
  {
  }

  void ParticleSelectorModule::event()
  {
    StoreObjPtr<ParticleList> plist(m_listName);

    if (m_otherLists.size() > 0) { // use given lists instead of store array
      plist.create();
      plist->setPDG(m_pdg);
      for (unsigned int i = 0; i < m_otherLists.size(); i++) {
        StoreObjPtr<ParticleList> otherList(m_otherLists[i]);
        if (abs(otherList->getPDG()) != abs(m_pdg)) continue;
        for (unsigned j = 0; j < otherList->getListSize(); j++) {
          const Particle* part = otherList->getParticle(j);
          if (m_pSelector.select(part)) {
            plist->addParticle(part);
          }
        }
      }
    } else if (m_pdg != 0) { // new particle list: fill selected
      plist.create();
      plist->setPDG(m_pdg);
      StoreArray<Particle> Particles;
      for (int i = 0; i < Particles.getEntries(); i++) {
        const Particle* part = Particles[i];
        if (abs(part->getPDGCode()) != abs(m_pdg)) continue;
        if (m_pSelector.select(part)) {
          plist->addParticle(part);
        }
      }
    } else { // existing particle list: apply selections and remove unselected
      std::vector<unsigned int> toRemove;
      for (unsigned i = 0; i < plist->getListSize(); i++) {
        const Particle* part = plist->getParticle(i);
        if (!m_pSelector.select(part)) toRemove.push_back(i);
      }
      plist->removeParticles(toRemove);
    }


    B2INFO("ParticleSelector: " << m_pdg << " " << m_listName << " size="
           << plist->getList(ParticleList::c_Particle).size()
           << "+" << plist->getList(ParticleList::c_AntiParticle).size()
           << "+" << plist->getList(ParticleList::c_SelfConjugatedParticle).size());

  }


  void ParticleSelectorModule::endRun()
  {
  }

  void ParticleSelectorModule::terminate()
  {
  }

  void ParticleSelectorModule::printModuleParams() const
  {
  }


} // end Belle2 namespace

