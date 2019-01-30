/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/AllParticlesCombiner/AllParticlesCombinerModule.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <framework/datastore/StoreArray.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(AllParticlesCombiner)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

AllParticlesCombinerModule::AllParticlesCombinerModule() : Module()
{
  // Set module properties
  setDescription(R"DOC("This module combines all particles of the provided list to one mother particle.
  )DOC");

  // Parameter definitions
  addParam("inputListName", m_inputListName, "List of particles which are supposed to be combined", std::string(""));
  addParam("cut", m_cutString, "Selection criteria", std::string(""));
  addParam("writeOut", m_writeOut,
           "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);
  addParam("outputListName", m_outputListName,
           "Name of the output list created by the combination of all particles in the input list.", std::string(""));

}

void AllParticlesCombinerModule::initialize()
{
  StoreArray<Particle>().isRequired();
  m_inputList.isRequired(m_inputListName);

  bool valid = m_decaydescriptor.init(m_outputListName);
  if (!valid)
    B2ERROR("Invalid output ListName: " << m_outputListName);

  // Mother particle
  const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();

  m_pdgCode = mother->getPDGCode();
  m_antiListName = ParticleListName::antiParticleListName(mother->getFullName());
  m_isSelfConjugatedParticle = (m_outputListName == m_antiListName);

  StoreObjPtr<ParticleList> particleList(m_outputListName);
  DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
  particleList.registerInDataStore(flags);
  if (!m_isSelfConjugatedParticle) {
    StoreObjPtr<ParticleList> antiParticleList(m_antiListName);
    antiParticleList.registerInDataStore(flags);
  }

  m_cut = Variable::Cut::compile(m_cutString);
}

void AllParticlesCombinerModule::event()
{
  StoreArray<Particle> particles;

  StoreObjPtr<ParticleList> outputList(m_outputListName);
  outputList.create();
  outputList->initialize(m_pdgCode, m_outputListName);

  if (!m_isSelfConjugatedParticle) {
    StoreObjPtr<ParticleList> outputAntiList(m_antiListName);
    outputAntiList.create();
    outputAntiList->initialize(-1 * m_pdgCode, m_antiListName);

    outputList->bindAntiParticleList(*(outputAntiList));
  }

  StoreObjPtr<ParticleList> plist(m_inputListName);

  double px = 0;
  double py = 0;
  double pz = 0;
  double E = 0;
  std::vector<int> daughterIndices(plist->getListSize(), 0);
  for (unsigned int i = 0; i < plist->getListSize(); ++i) {
    Particle* particle = plist->getParticle(i, true);
    daughterIndices[i] = particle->getArrayIndex();
    px += particle->getPx();
    py += particle->getPy();
    pz += particle->getPz();
    E += particle->getEnergy();
  }
  const TLorentzVector vec(px, py, pz, E);

  Particle combinedParticle = Particle(vec, m_pdgCode, m_isSelfConjugatedParticle ? Particle::c_Unflavored : Particle::c_Flavored,
                                       daughterIndices, particles.getPtr());

  Particle* newParticle = particles.appendNew(combinedParticle);
  if (m_cut->check(newParticle)) {
    outputList->addParticle(newParticle);
  }
}
