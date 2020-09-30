/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yo Sato                                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleCombinerFromMC/ParticleCombinerFromMCModule.h>

// framework aux
#include <framework/logging/Logger.h>

// decay descriptor
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>

// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/MCMatching.h>

#include <memory>

using namespace std;

namespace Belle2 {

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

  REG_MODULE(ParticleCombinerFromMC)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

  ParticleCombinerFromMCModule::ParticleCombinerFromMCModule() :
    Module()

  {
    // set module description (e.g. insert text)
    setDescription("Makes particle combinations");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("decayString", m_decayString,
             "Input DecayDescriptor string (see :ref:`DecayString`).");
    addParam("cut", m_cutParameter, "Selection criteria to be applied", std::string(""));
    addParam("decayMode", m_decayModeID, "User-specified decay mode identifier (saved in 'decayModeID' extra-info for each Particle)",
             0);

    addParam("writeOut", m_writeOut,
             "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);
    addParam("chargeConjugation", m_chargeConjugation,
             "If true, the charge-conjugated mode will be reconstructed as well", true);

    // initializing the rest of private members
    m_pdgCode   = 0;
    m_isSelfConjugatedParticle = false;
    m_generator = nullptr;
    m_cut = nullptr;
  }

  void ParticleCombinerFromMCModule::initialize()
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

    m_antiListName = ParticleListName::antiParticleListName(m_listName);
    m_isSelfConjugatedParticle = (m_listName == m_antiListName);

    m_cut = Variable::Cut::compile(m_cutParameter);

    // register particles which have (sub-)decay recursively
    registerParticleRecursively(m_decaydescriptor);

  }

  void ParticleCombinerFromMCModule::event()
  {
    B2DEBUG(10, "event() started !!!");

    // combine particles recursively
    combineRecursively(m_decaydescriptor);

    StoreObjPtr<ParticleList> plist(m_listName);
    bool existingList = plist.isValid();

    if (!existingList) {
      B2WARNING("Output list " << m_listName << " was not created");
      return;
    }

    // loop over list only if cuts should be applied
    if (!m_cutParameter.empty()) {
      std::vector<unsigned int> toRemove;
      unsigned int n = plist->getListSize();
      for (unsigned i = 0; i < n; i++) {
        const Particle* part = plist->getParticle(i);

        MCMatching::setMCTruth(part);
        MCMatching::getMCErrors(part);

        if (!m_cut->check(part)) toRemove.push_back(part->getArrayIndex());
      }
      plist->removeParticles(toRemove);
    }
  }


  void ParticleCombinerFromMCModule::registerParticleRecursively(const DecayDescriptor& decaydescriptor)
  {
    int nProducts = decaydescriptor.getNDaughters();

    for (int i = 0; i < nProducts; ++i) {
      if (decaydescriptor.getDaughter(i)->getNDaughters() == 0) {
        // if daughter does not have daughters, it must be already registered
        const DecayDescriptorParticle* daughter = decaydescriptor.getDaughter(i)->getMother();
        StoreObjPtr<ParticleList>().isRequired(daughter->getFullName());
      } else {
        // if daughter has daughters, call the function recursively
        registerParticleRecursively(*(decaydescriptor.getDaughter(i)));
      }
    }

    // Mother particle
    const DecayDescriptorParticle* mother = decaydescriptor.getMother();
    std::string listName = mother->getFullName();
    StoreObjPtr<ParticleList> particleList(listName);

    // if particleList already exists
    auto existList = std::find(m_vector_listName.begin(), m_vector_listName.end(), listName);
    if (existList != m_vector_listName.end()) {
      B2ERROR(listName << " already exist ! You cannot write same sub-decay twice !!!");
      return;
    }

    std::string antiListName = ParticleListName::antiParticleListName(listName);
    bool isSelfConjugatedParticle = (listName == antiListName);

    DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
    particleList.registerInDataStore(flags);
    if (!isSelfConjugatedParticle && m_chargeConjugation) {
      StoreObjPtr<ParticleList> antiParticleList(antiListName);
      antiParticleList.registerInDataStore(flags);
    }

    m_vector_listName.push_back(listName);

  }

  void ParticleCombinerFromMCModule::combineRecursively(const DecayDescriptor& decaydescriptor)
  {
    // Mother particle
    const DecayDescriptorParticle* mother = decaydescriptor.getMother();
    int pdgCode = mother->getPDGCode();
    std::string listName = mother->getFullName();
    std::string antiListName = ParticleListName::antiParticleListName(listName);
    bool isSelfConjugatedParticle = (listName == antiListName);

    StoreObjPtr<ParticleList> outputList(listName);
    outputList.create();
    outputList->initialize(pdgCode, listName);

    if (!isSelfConjugatedParticle && m_chargeConjugation) {
      StoreObjPtr<ParticleList> outputAntiList(antiListName);
      outputAntiList.create();
      outputAntiList->initialize(-1 * pdgCode, antiListName);

      outputList->bindAntiParticleList(*(outputAntiList));
    }

    unsigned int numberOfLists = decaydescriptor.getNDaughters();

    for (unsigned int i = 0; i < numberOfLists; ++i) {
      const DecayDescriptor* dDaughter = decaydescriptor.getDaughter(i);

      if (dDaughter->getNDaughters() == 0) {
        // if daughter does not have daughters, check if it is not reconstructed particle.
        const DecayDescriptorParticle* daughter = decaydescriptor.getDaughter(i)->getMother();
        StoreObjPtr<ParticleList> plist(daughter->getFullName());
        // if daughter is not created, returns error
        if (!plist.isValid())
          B2ERROR(daughter->getFullName() << " is not created");
        // if daughter contains reconstructed particles, returns error.
        unsigned nPart = plist->getListSize();
        for (unsigned iPart = 0; iPart < nPart; iPart++) {
          const Particle* part = plist->getParticle(iPart);
          Particle::EParticleSourceObject particleType = part->getParticleSource();
          if (particleType == Particle::c_Track or
              particleType == Particle::c_ECLCluster or
              particleType == Particle::c_KLMCluster)
            B2ERROR(daughter->getFullName() << " contains a reconstructed particle! It is not accepted in ParticleCombinerFromMCModule!");
        }
        // if not, do nothing.
      } else {
        // if daughter has daughter, call the function recursively
        combineRecursively(*dDaughter);
      }
    }

    // initialize the generator
    m_generator = std::make_unique<ParticleGenerator>(decaydescriptor, "");
    m_generator->init();

    while (m_generator->loadNext(m_chargeConjugation)) {
      Particle&& particle = m_generator->getCurrentParticle();

      Particle* newParticle = m_particles.appendNew(particle);
      // append to the created particle the user specified decay mode ID
      newParticle->addExtraInfo("decayModeID", m_decayModeID);

      int iparticle = m_particles.getEntries() - 1;
      outputList->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }

    // select only signal particles
    std::unique_ptr<Variable::Cut> cutIsSignal = Variable::Cut::compile("isSignal");

    std::vector<unsigned int> toRemove;
    unsigned int n = outputList->getListSize();
    for (unsigned i = 0; i < n; i++) {
      const Particle* part = outputList->getParticle(i);

      MCMatching::setMCTruth(part);
      MCMatching::getMCErrors(part);

      if (!cutIsSignal->check(part)) toRemove.push_back(part->getArrayIndex());
    }
    outputList->removeParticles(toRemove);

  }

} // end Belle2 namespace

