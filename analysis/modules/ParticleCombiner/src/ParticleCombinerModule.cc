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
#include <analysis/modules/ParticleCombiner/ParticleCombinerModule.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/utility/VariableManager.h>

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

  ParticleCombinerModule::ParticleCombinerModule() : Module()

  {
    // set module description (e.g. insert text)
    setDescription("Makes particle combinations");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("PDG", m_pdg, "PDG code", 0);
    addParam("ListName", m_listName, "name of the output particle list", string(""));
    vector<string> defaultList;
    addParam("InputListNames", m_inputListNames, "list of input particle list names",
             defaultList);
    std::map<std::string, std::tuple<double, double>> defaultMap;
    addParam("cuts", m_cuts, "Map of Variable and Cut Values.", defaultMap);
    addParam("persistent", m_persistent,
             "toggle output particle list btw. transient/persistent", false);

  }

  ParticleCombinerModule::~ParticleCombinerModule()
  {
  }

  void ParticleCombinerModule::initialize()
  {
    if (m_persistent) {
      StoreObjPtr<ParticleList>::registerPersistent(m_listName);
    } else {
      StoreObjPtr<ParticleList>::registerTransient(m_listName);
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
    StoreArray<Particle> Particles;

    StoreObjPtr<ParticleList> outputList(m_listName);
    outputList.create();
    outputList->setPDG(m_pdg);

    ParticleCombiner combiner(m_inputListNames, isParticleSelfConjugated(outputList));
    int pdg = outputList->getPDG();
    int pdgbar = outputList->getPDGbar();

    while (combiner.loadNext()) {

      const Particle particle = combiner.getCurrentParticle(pdg, pdgbar);
      if (!checkCuts(&particle)) continue;

      new(Particles.nextFreeAddress()) Particle(particle);
      int iparticle = Particles.getEntries() - 1;
      outputList->addParticle(iparticle, combiner.getCurrentType());

    }

    B2INFO("ParticleCombiner: " << m_pdg << " " << m_listName << " size="
           << outputList->getList(ParticleList::c_Particle).size()
           << "+" << outputList->getList(ParticleList::c_AntiParticle).size()
           << "+" << outputList->getList(ParticleList::c_SelfConjugatedParticle).size());

  }


  bool ParticleCombinerModule::isParticleSelfConjugated(StoreObjPtr<ParticleList>& outputList)
  {
    return outputList->getPDG() == outputList->getPDGbar();
  }

  void ParticleCombinerModule::endRun()
  {
  }

  void ParticleCombinerModule::terminate()
  {
  }

  bool ParticleCombinerModule::checkCuts(const Particle* particle)
  {

    VariableManager& manager = VariableManager::Instance();

    for (auto & cut : m_cuts) {
      auto var = manager.getVariable(cut.first);
      if (var == nullptr) {
        B2INFO("ParticleCombiner: VariableManager doesn't have variable" <<  cut.first)
        return false;
      }
      double value = var->function(particle);
      if (value < std::get<0>(cut.second) || value > std::get<1>(cut.second)) return false;
    }
    return true;
  }



} // end Belle2 namespace

