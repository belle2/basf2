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

// utilities
#include <analysis/utility/EvtPDLUtil.h>

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
    setDescription("Selects from StoreArray<Particles> according to given decay and selection criteria and fills them into a ParticleList.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("decayString", m_decayString, "Input DecayDescriptor string (see https://belle2.cc.kek.jp/~twiki/bin/view/Physics/DecayString).");

    vector<string> defaultSelection;
    addParam("select", m_selection, "selection criteria", defaultSelection);
    addParam("persistent", m_persistent,
             "toggle newly created particle list btw. transient/persistent", false);

    // initializing the rest of private memebers
    m_pdgCode   = 0;
    m_isSelfConjugatedParticle = 0;
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

    // obtain the input and output particle lists from the decay string
    bool valid = m_decaydescriptor.init(m_decayString);
    if (!valid)
      B2ERROR("ParticleSelectorModule::initialize Invalid input DecayString: " << m_decayString);

    int nProducts = m_decaydescriptor.getNDaughters();
    if (nProducts > 0)
      B2ERROR("ParticleSelectorModule::initialize Invalid input DecayString " << m_decayString
              << ". DecayString should not contain any daughters, only the mother particle.");

    // Mother particle
    const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();

    m_pdgCode  = mother->getPDGCode();
    m_listName = mother->getFullName();

    m_isSelfConjugatedParticle = !(Belle2::EvtPDLUtil::hasAntiParticle(m_pdgCode));
    m_antiListName             = Belle2::EvtPDLUtil::antiParticleListName(m_pdgCode, mother->getLabel());


    if (m_persistent) {
      StoreObjPtr<ParticleList>::registerPersistent(m_listName, DataStore::c_Event, false);
      if (!m_isSelfConjugatedParticle)
        StoreObjPtr<ParticleList>::registerPersistent(m_antiListName, DataStore::c_Event, false);
    } else {
      StoreObjPtr<ParticleList>::registerTransient(m_listName, DataStore::c_Event, false);
      if (!m_isSelfConjugatedParticle)
        StoreObjPtr<ParticleList>::registerTransient(m_antiListName, DataStore::c_Event, false);
    }

    for (unsigned int i = 0; i < m_selection.size(); i++) {
      m_pSelector.addSelection(m_selection[i]);
    }

    std::string cuts;
    m_pSelector.listCuts(cuts);
    if (cuts.empty()) cuts = "(all)";
    B2INFO("ParticleSelector: " << m_listName << " (" << m_antiListName << ") " << cuts);
  }

  void ParticleSelectorModule::beginRun()
  {
  }

  void ParticleSelectorModule::event()
  {
    StoreObjPtr<ParticleList> plist(m_listName);
    bool existingList = plist.isValid();

    if (!existingList) { // new particle list: fill selected
      plist.create();
      plist->initialize(m_pdgCode, m_listName);

      if (!m_isSelfConjugatedParticle) {
        StoreObjPtr<ParticleList> antiPlist(m_antiListName);
        antiPlist.create();
        antiPlist->initialize(-1 * m_pdgCode, m_antiListName);

        antiPlist->bindAntiParticleList(*(plist));
      }

      StoreArray<Particle> Particles;
      for (int i = 0; i < Particles.getEntries(); i++) {
        const Particle* part = Particles[i];
        if (abs(part->getPDGCode()) != abs(m_pdgCode)) continue;
        if (m_pSelector.select(part)) {
          plist->addParticle(part);
        }
      }
    } else { // existing particle list: apply selections and remove unselected
      if (m_selection.size() > 0) {
        // loop over list only if cuts should be applied
        std::vector<unsigned int> toRemove;
        for (unsigned i = 0; i < plist->getListSize(); i++) {
          const Particle* part = plist->getParticle(i);
          if (!m_pSelector.select(part)) toRemove.push_back(part->getArrayIndex());
        }

        plist->removeParticles(toRemove);
      }
    }


    /*
    B2INFO("ParticleSelector: " << m_pdgCode << " " << m_listName  << " (" << m_antiListName << ") "<< " size="
           << plist->getNumOf(ParticleList::c_Particle,false)
           << "+" << plist->getNumOf(ParticleList::c_SelfConjugatedParticle,false)
           << " (" << plist->getNumOf(ParticleList::c_Particle,true)
           << "+" << plist->getNumOf(ParticleList::c_SelfConjugatedParticle,true) << ")");
    */
    //plist->print();

  }


  void ParticleSelectorModule::endRun()
  {
  }

  void ParticleSelectorModule::terminate()
  {
  }


} // end Belle2 namespace

