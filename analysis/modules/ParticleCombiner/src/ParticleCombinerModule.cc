/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleCombiner/ParticleCombinerModule.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/utility/VariableManager.h>

// decay descriptor
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>

// utilities
#include <analysis/utility/EvtPDLUtil.h>

#include <algorithm>

using namespace std;

namespace Belle2 {

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

  REG_MODULE(ParticleCombiner)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

  ParticleCombinerModule::ParticleCombinerModule() :
    Module()

  {
    // set module description (e.g. insert text)
    setDescription("Makes particle combinations");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("decayString", m_strDecay, "Input DecayDescriptor string (see https://belle2.cc.kek.jp/~twiki/bin/view/Physics/DecayString).",
             string(""));

    std::map<std::string, std::tuple<double, double>> defaultMap;
    addParam("cuts", m_cuts, "Map of Variable and Cut Values.", defaultMap);
    addParam("persistent", m_persistent,
             "toggle output particle list btw. transient/persistent", false);

    // legacy parameters (can be used instead of strDecayString parameter)
    addParam("PDG", m_pdgCode, "PDG code", 0);
    addParam("ListName", m_listName, "name of the output particle list",
             string(""));
    vector<string> defaultList;
    addParam("InputListNames", m_inputListNames,
             "list of input particle list names", defaultList);
  }

  ParticleCombinerModule::~ParticleCombinerModule()
  {
  }

  void ParticleCombinerModule::initialize()
  {
    if (m_strDecay.empty() && m_inputListNames.size() == 0)
      B2ERROR("No decay descriptor string and no input lists provided.");

    if (!m_strDecay.empty()) {
      // clear everything
      m_pdgCode = 0;
      m_listName = "";
      m_inputListNames.clear();
      //m_daughterPDGCodes.clear();

      // obtain the input and output particle lists from the decay string
      bool valid = m_decaydescriptor.init(m_strDecay);
      if (!valid)
        B2ERROR("Invalid input DecayString: " << m_strDecay);

      // Mother particle
      const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();

      m_pdgCode = mother->getPDGCode();
      m_listName = mother->getFullName();

      m_isSelfConjugatedParticle = !(Belle2::EvtPDLUtil::hasAntiParticle(
                                       m_pdgCode));
      m_antiListName = Belle2::EvtPDLUtil::antiParticleListName(m_pdgCode,
                                                                mother->getLabel());

      // Daughters
      int nProducts = m_decaydescriptor.getNDaughters();
      for (int i = 0; i < nProducts; ++i) {
        const DecayDescriptorParticle* daughter =
          m_decaydescriptor.getDaughter(i)->getMother();
        m_inputListNames.push_back(daughter->getFullName());
        //m_daughterPDGCodes.push_back(daughter->getPDGCode());
      }
      m_generator = new ParticleGenerator(m_strDecay);
    } else {
      // check if the mother and daughter list names are valid names
      bool valid = m_decaydescriptor.init(m_listName);
      if (!valid)
        B2ERROR("Invalid output (mother) list name: " << m_listName);

      // Mother particle
      const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();
      if (m_pdgCode != mother->getPDGCode())
        B2WARNING(
          "Inconsistent list name and pdg code! Will use the pdg code determined from the list name.");

      m_pdgCode = mother->getPDGCode();

      m_isSelfConjugatedParticle = !(Belle2::EvtPDLUtil::hasAntiParticle(
                                       m_pdgCode));
      m_antiListName = Belle2::EvtPDLUtil::antiParticleListName(m_pdgCode,
                                                                mother->getLabel());

      // Daughters
      unsigned nProducts = m_inputListNames.size();
      for (unsigned i = 0; i < nProducts; ++i) {
        valid = m_decaydescriptor.init(m_inputListNames[i]);
        if (!valid)
          B2ERROR("Invalid input (daughter) list name: " << m_inputListNames[i]);
      }
      m_generator = new ParticleGenerator(m_listName); //TODO Create correct decayString here, this won't work
    }

    if (m_persistent) {
      StoreObjPtr<ParticleList>::registerPersistent(m_listName);
      if (!m_isSelfConjugatedParticle)
        StoreObjPtr<ParticleList>::registerPersistent(m_antiListName);
    } else {
      StoreObjPtr<ParticleList>::registerTransient(m_listName);
      if (!m_isSelfConjugatedParticle)
        StoreObjPtr<ParticleList>::registerTransient(m_antiListName);
    }

    for (unsigned i = 0; i < m_inputListNames.size(); i++) {
      StoreObjPtr<ParticleList>::required(m_inputListNames[i]);
    }


  }

  void ParticleCombinerModule::beginRun()
  {
  }

  void ParticleCombinerModule::event()
  {
    StoreArray<Particle> particles;

    StoreObjPtr<ParticleList> outputList(m_listName);
    outputList.create();
    outputList->initialize(m_pdgCode, m_listName);

    if (!m_isSelfConjugatedParticle) {
      StoreObjPtr<ParticleList> outputAntiList(m_antiListName);
      outputAntiList.create();
      outputAntiList->initialize(-1 * m_pdgCode, m_antiListName);

      outputList->bindAntiParticleList(*(outputAntiList));
    }

    int pdg    = outputList->getPDGCode();
    int pdgbar = outputList->getAntiParticlePDGCode();

    //B2INFO("[ParticleCombinerModule::event] OutputListName = " << m_listName << "(" << m_antiListName << ")" << "[" << pdg << "/" << pdgbar << "]");

    //int counter = 1;
    m_generator->init();
    while (m_generator->loadNext()) {
      //B2INFO("[ParticleCombinerModule::event] loaded Particle #" << counter++);

      const Particle& particle = m_generator->getCurrentParticle();
      if (!checkCuts(&particle))
        continue;

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      outputList->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }
  }

  void ParticleCombinerModule::endRun()
  {
  }

  void ParticleCombinerModule::terminate()
  {
    delete m_generator;
  }

  bool ParticleCombinerModule::checkCuts(const Particle* particle)
  {

    VariableManager& manager = VariableManager::Instance();

    for (auto & cut : m_cuts) {
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

} // end Belle2 namespace

