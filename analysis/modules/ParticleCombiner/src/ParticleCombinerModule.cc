/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

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
#include <generators/dataobjects/MCParticle.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

// Own include
#include <analysis/modules/ParticleCombiner/ParticleCombinerModule.h>
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
    setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

    // Add parameters
    addParam("PDG", m_pdg, "PDG code", 0);
    addParam("ListName", m_listName, "name of the output particle list", string(""));
    vector<string> defaultList;
    addParam("InputListNames", m_inputListNames, "list of input particle list names",
             defaultList);
    addParam("MassCutLow", m_massCutLow, "[GeV] lower mass cut", 0.0);
    addParam("MassCutHigh", m_massCutHigh, "[GeV] upper mass cut", 100.0);
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
    StoreObjPtr<ParticleList> outputList(m_listName);
    outputList.create();
    outputList->setPDG(m_pdg);

    vector<StoreObjPtr<ParticleList> > plists;
    for (unsigned i = 0; i < m_inputListNames.size(); i++) {
      StoreObjPtr<ParticleList> list(m_inputListNames[i]);
      if (!list) {
        B2ERROR("ParticleList " << m_inputListNames[i] << " not found");
        return;
      }
      if (list->getParticleCollectionName() != std::string("Particles")) {
        B2ERROR("ParticleList " << m_inputListNames[i] <<
                " does not refer to the default Particle collection");
        return;
      }
      plists.push_back(list);
    }

    if (outputList->getFlavorType() == 1) {
      vector<int> decay, decaybar;
      for (unsigned i = 0; i < plists.size(); i++) {
        decay.push_back(plists[i]->getPDG());
        decaybar.push_back(plists[i]->getPDGbar());
      }
      std::sort(decay.begin(), decay.end());
      std::sort(decaybar.begin(), decaybar.end());
      if (decay == decaybar) outputList->swapFlavorType();
    }

    combination(outputList, plists, 0);
    if (outputList->getFlavorType() == 1) combination(outputList, plists, 1);

    // printout with B2INFO
    std::string decay;
    for (unsigned i = 0; i < m_inputListNames.size(); i++) {
      decay = decay + " " + m_inputListNames[i];
    }
    switch (outputList->getFlavorType()) {
      case 0:
        B2INFO("ParticleCombiner: " << m_pdg << " " << m_listName << "->" << decay
               << " size=" << outputList->getList(0).size());
        break;
      case 1:
        B2INFO("ParticleCombiner: " << m_pdg << " " << m_listName << "->" << decay
               << " size="
               << outputList->getList(0).size() << "+" << outputList->getList(1).size());
        break;
      default:
        B2ERROR("ParticleCombiner: " << m_pdg << " " << m_listName << "->" << decay
                << " ***invalid FlavorType: "
                << outputList->getFlavorType());
    }

  }


  void ParticleCombinerModule::endRun()
  {
  }

  void ParticleCombinerModule::terminate()
  {
  }

  void ParticleCombinerModule::printModuleParams() const
  {
  }

  void ParticleCombinerModule::combination(StoreObjPtr<ParticleList>& outputList,
                                           vector<StoreObjPtr<ParticleList> >& plists,
                                           unsigned chargeState)
  {
    StoreArray<Particle> Particles;

    // arrays needed to construct N nested loops

    int N = plists.size(); // number of particle lists to combine
    int n[N]; // array of list sizes
    int k[N]; // array of list element indices

    // initialize arrays and list indices

    int nLoop = 1;
    for (int i = 0; i < N; i++) {
      n[i] = plists[i]->getList(chargeState).size();
      if (n[i] == 0) return;
      k[i] = 0;
      nLoop *= n[i];
    }

    // stack needed to check for unique combination

    unordered_set<boost::dynamic_bitset<> > indexStack;

    // N nested loops

    for (int loop = 0; loop < nLoop; loop++) {

      // increment indices first
      for (int i = 0; i < N; i++) {
        k[i]++;
        if (k[i] < n[i]) break;
        k[i] = 0;
      }

      // check if particle sources are all different
      vector<Particle*> particleStack;
      vector<int> indices;
      for (int i = 0; i < N; i++) {
        int ii = plists[i]->getList(chargeState)[k[i]];
        particleStack.push_back(Particles[ii]);
        indices.push_back(ii);
      }
      if (!differentSources(particleStack, Particles)) continue;

      // apply invariant mass cut
      TLorentzVector vec(0., 0., 0., 0.);
      for (unsigned i = 0; i < particleStack.size(); i++) {
        vec = vec + particleStack[i]->get4Vector();
      }
      double mass = vec.M();
      if (mass < m_massCutLow) continue;
      if (mass > m_massCutHigh) continue;

      // check if the combination is unique (e.g. it's permutation is not on the stack)
      if (!uniqueCombination(indexStack, indices)) continue;

      // store combination
      new(Particles.nextFreeAddress()) Particle(vec,
                                                outputList->getPDG(chargeState),
                                                outputList->getFlavorType(),
                                                indices);
      int iparticle = Particles.getEntries() - 1;
      outputList->addParticle(iparticle, chargeState);

    }

  }


  bool ParticleCombinerModule::differentSources(std::vector<Particle*> stack,
                                                StoreArray<Particle>& Particles)
  {
    vector<int> sources; // stack for particle sources

    while (!stack.empty()) {
      Particle* p = stack.back();
      stack.pop_back();
      vector<int> childs = p->getDaughterIndices();
      if (childs.empty()) {
        int source = p->getMdstSource();
        for (unsigned i = 0; i < sources.size(); i++) {
          if (source == sources[i]) return false;
        }
        sources.push_back(source);
      } else {
        for (unsigned i = 0; i < childs.size(); i++) stack.push_back(Particles[childs[i]]);
      }
    }
    return true;
  }


  bool ParticleCombinerModule::uniqueCombination(
    std::unordered_set<boost::dynamic_bitset<> >& indexStack,
    const std::vector<int>& indices)
  {
    int max_element = *std::max_element(indices.begin(), indices.end());
    boost::dynamic_bitset<> indicesBits(max_element + 1); //all zeroes
    for (int idx : indices) {
      indicesBits[idx] = 1;
    }

    if (indexStack.find(indicesBits) != indexStack.end())
      return false;

    indexStack.insert(indicesBits);
    return true;
  }


} // end Belle2 namespace

