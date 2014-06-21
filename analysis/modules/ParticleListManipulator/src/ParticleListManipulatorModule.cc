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
#include <analysis/modules/ParticleListManipulator/ParticleListManipulatorModule.h>


#include <framework/core/ModuleManager.h>

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
#include <analysis/utility/EvtPDLUtil.h>
#include <analysis/utility/VariableManager.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticleListManipulator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticleListManipulatorModule::ParticleListManipulatorModule() : Module()

  {
    // set module description (e.g. insert text)
    setDescription("Manipulates ParticleLists: copies/mergesPerforms particle selection");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("outputListName", m_outputListName, "Output ParticleList name", string(""));

    vector<string> defaultList;
    addParam("inputListNames", m_inputListNames,
             "list of input ParticleList names", defaultList);

    std::map<std::string, std::tuple<double, double>> defaultMap;
    addParam("cuts", m_selection, "Selection criteria to be applied", defaultMap);
    addParam("persistent", m_persistent,
             "toggle newly created particle list btw. transient/persistent", false);

    // initializing the rest of private memebers
    m_pdgCode   = 0;
    m_isSelfConjugatedParticle = 0;
  }

  ParticleListManipulatorModule::~ParticleListManipulatorModule()
  {
  }

  void ParticleListManipulatorModule::initialize()
  {
    if (m_outputListName.empty())
      B2ERROR("Please provide output ParticleList names!");

    if (!m_outputListName.empty()) {
      m_pdgCode  = 0;

      // check the validity of output ParticleList name
      bool valid = m_decaydescriptor.init(m_outputListName);
      if (!valid)
        B2ERROR("ParticleListManipulatorModule::initialize Invalid output ParticleList name: " << m_outputListName);

      // Output particle
      const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();

      m_pdgCode  = mother->getPDGCode();

      m_isSelfConjugatedParticle = !(Belle2::EvtPDLUtil::hasAntiParticle(m_pdgCode));
      m_outputAntiListName       = Belle2::EvtPDLUtil::antiParticleListName(m_pdgCode, mother->getLabel());
    } else {
      B2ERROR("Please provide output ParticleList name!");
    }

    // Input lists
    unsigned nProducts = m_inputListNames.size();
    for (unsigned i = 0; i < nProducts; ++i) {
      bool valid = m_decaydescriptor.init(m_inputListNames[i]);
      if (!valid)
        B2ERROR("Invalid input ParticleList name: " << m_inputListNames[i]);
    }

    if (m_persistent) {
      StoreObjPtr<ParticleList>::registerPersistent(m_outputListName, DataStore::c_Event, false);
      if (!m_isSelfConjugatedParticle)
        StoreObjPtr<ParticleList>::registerPersistent(m_outputAntiListName, DataStore::c_Event, false);
    } else {
      StoreObjPtr<ParticleList>::registerTransient(m_outputListName, DataStore::c_Event, false);
      if (!m_isSelfConjugatedParticle)
        StoreObjPtr<ParticleList>::registerTransient(m_outputAntiListName, DataStore::c_Event, false);
    }

    for (unsigned i = 0; i < m_inputListNames.size(); i++) {
      StoreObjPtr<ParticleList>::required(m_inputListNames[i]);
    }
  }

  void ParticleListManipulatorModule::beginRun()
  {
  }

  void ParticleListManipulatorModule::event()
  {
    StoreArray<Particle> particles;
    StoreObjPtr<ParticleList> plist(m_outputListName);
    bool existingList = plist.isValid();

    if (!existingList) {
      // new particle list: create it, and copy all particles from input lists that pass selection criteria to this one
      plist.create();
      plist->initialize(m_pdgCode, m_outputListName);

      if (!m_isSelfConjugatedParticle) {
        StoreObjPtr<ParticleList> antiPlist(m_outputAntiListName);
        antiPlist.create();
        antiPlist->initialize(-1 * m_pdgCode, m_outputAntiListName);

        antiPlist->bindAntiParticleList(*(plist));
      }
    }

    for (unsigned i = 0; i < m_inputListNames.size(); i++) {
      StoreObjPtr<ParticleList> inPList(m_inputListNames[i]);

      std::vector<int> fsParticles     = inPList->getList(ParticleList::EParticleType::c_FlavorSpecificParticle,               false);
      std::vector<int> scParticles     = inPList->getList(ParticleList::EParticleType::c_SelfConjugatedParticle, false);
      std::vector<int> fsAntiParticles = inPList->getList(ParticleList::EParticleType::c_FlavorSpecificParticle,               true);

      fsParticles.insert(fsParticles.end(), scParticles.begin(), scParticles.end());
      fsParticles.insert(fsParticles.end(), fsAntiParticles.begin(), fsAntiParticles.end());

      for (unsigned i = 0; i < fsParticles.size(); i++) {
        const Particle* part = particles[fsParticles[i]];

        if (checkCuts(part))
          plist->addParticle(part);
      }
    }
  }


  bool ParticleListManipulatorModule::checkCuts(const Particle* particle)
  {

    VariableManager& manager = VariableManager::Instance();

    for (auto & cut : m_selection) {
      auto var = manager.getVariable(cut.first);
      if (var == nullptr) {
        B2INFO(
          "ParticleCombiner: VariableManager doesn't have variable" << cut.first)
        return false;
      }
      double value = var->function(particle);
      if (value < std::get < 0 > (cut.second)
          || value > std::get < 1 > (cut.second))
        return false;
    }
    return true;
  }


  void ParticleListManipulatorModule::endRun()
  {
  }

  void ParticleListManipulatorModule::terminate()
  {
  }

  void ParticleListManipulatorModule::printModuleParams() const
  {
  }


} // end Belle2 namespace

