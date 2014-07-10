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
    addParam("decayString", m_decayString, "Input DecayDescriptor string (see https://belle2.cc.kek.jp/~twiki/bin/view/Physics/DecayString).");

    Variable::Cut::Parameter emptyCut;
    addParam("cut", m_cutParameter, "Selection criteria to be applied", emptyCut);

    addParam("persistent", m_persistent,
             "toggle output particle list btw. transient/persistent", false);

    // initializing the rest of private memebers
    m_pdgCode   = 0;
    m_isSelfConjugatedParticle = 0;
    m_generator = 0;
  }

  ParticleCombinerModule::~ParticleCombinerModule()
  {
  }

  void ParticleCombinerModule::initialize()
  {
    // clear everything
    m_pdgCode = 0;
    m_listName = "";

    // obtain the input and output particle lists from the decay string
    bool valid = m_decaydescriptor.init(m_decayString);
    if (!valid)
      B2ERROR("Invalid input DecayString: " << m_decayString);

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
      StoreObjPtr<ParticleList>::required(daughter->getFullName());
    }
    m_generator = new ParticleGenerator(m_decayString);

    if (m_persistent) {
      StoreObjPtr<ParticleList>::registerPersistent(m_listName);
      if (!m_isSelfConjugatedParticle)
        StoreObjPtr<ParticleList>::registerPersistent(m_antiListName);
    } else {
      StoreObjPtr<ParticleList>::registerTransient(m_listName);
      if (!m_isSelfConjugatedParticle)
        StoreObjPtr<ParticleList>::registerTransient(m_antiListName);
    }

    m_cut.init(m_cutParameter);

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

    m_generator->init();
    while (m_generator->loadNext()) {

      const Particle& particle = m_generator->getCurrentParticle();
      if (!m_cut.check(&particle))
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

} // end Belle2 namespace

