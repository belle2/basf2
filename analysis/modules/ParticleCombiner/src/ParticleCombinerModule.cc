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
    addParam("MassCut", m_massCut, "[GeV] tuple of lower and upper mass cut", std::make_tuple(0.0, 100.0));
    std::map<std::string, std::tuple<double, double>> defaultMap;
    addParam("cutsOnProduct", m_productCuts, "Map of Variable and Cut Values. Cuts are performed on the product of the variable value of all daughter particles.", defaultMap);
    addParam("cutsOnSum", m_sumCuts, "Map of Variable and Cut Values. Cuts are performed on the sum of the variable value of all daughter particles.", defaultMap);
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

    double massCutLow = 0.0;
    double massCutHigh = 0.0;
    std::tie(massCutLow, massCutHigh) = m_massCut;

    ParticleCombiner combiner(m_inputListNames, isParticleSelfConjugated(outputList));
    while (combiner.loadNext()) {

      vector<Particle*> particleStack = combiner.getCurrentParticles();
      vector<int> indices = combiner.getCurrentIndices();

      TLorentzVector vec(0., 0., 0., 0.);
      for (unsigned i = 0; i < particleStack.size(); i++) {
        vec = vec + particleStack[i]->get4Vector();
      }
      double mass = vec.M();
      if (mass < massCutLow || mass > massCutHigh) continue;

      if (!checkCuts(particleStack)) continue;

      ParticleList::EParticleType outputType = combiner.getCurrentType();
      new(Particles.nextFreeAddress()) Particle(vec,
                                                outputType == ParticleList::c_AntiParticle ? outputList->getPDGbar() :  outputList->getPDG(),
                                                outputType == ParticleList::c_SelfConjugatedParticle ? Particle::c_Unflavored : Particle::c_Flavored,
                                                indices);
      int iparticle = Particles.getEntries() - 1;
      outputList->addParticle(iparticle, outputType);

    }

    // printout with B2INFO
    std::string decay;
    for (unsigned i = 0; i < m_inputListNames.size(); i++) {
      decay = decay + " " + m_inputListNames[i];
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

  bool ParticleCombinerModule::checkCuts(const std::vector<Particle*>& particleStack)
  {

    VariableManager& manager = VariableManager::Instance();

    for (auto & cut : m_productCuts) {
      // Now calculate the value by multiplying all decay product values,
      double value = 1.0;
      auto var = manager.getVariable(cut.first);
      if (var == nullptr) {
        B2INFO("ParticleCombiner: VariableManager doesn't have variable" <<  cut.first)
        return false;
      }
      for (unsigned i = 0; i < particleStack.size(); i++) {
        value *= var->function(particleStack[i]);
      }
      if (value < std::get<0>(cut.second) || value > std::get<1>(cut.second)) return false;
    }

    for (auto & cut : m_sumCuts) {
      // Now calculate the value by adding all decay product values,
      double value = 0.0;
      auto var = manager.getVariable(cut.first);
      if (var == nullptr) {
        B2INFO("ParticleCombiner: VariableManager doesn't have variable" <<  cut.first)
        return false;
      }
      for (unsigned i = 0; i < particleStack.size(); i++) {
        value += var->function(particleStack[i]);
      }
      if (value < std::get<0>(cut.second) || value > std::get<1>(cut.second)) return false;
    }

    return true;

  }



} // end Belle2 namespace

