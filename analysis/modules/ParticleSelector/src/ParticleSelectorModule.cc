/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/modules/ParticleSelector/ParticleSelectorModule.h>

// framework aux
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(ParticleSelector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ParticleSelectorModule::ParticleSelectorModule() : Module()
{
  setDescription("Removes Particles from given ParticleList that do not pass specified selection criteria.");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("decayString", m_decayString,
           "Input ParticleList name (see :ref:`DecayString`).");

  addParam("cut", m_cutParameter,
           "Selection criteria to be applied, see `cut_strings_selections`",
           std::string(""));
}

void ParticleSelectorModule::initialize()
{
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

  const int pdgCode = mother->getPDGCode();
  string listLabel = mother->getLabel();
  m_listName = mother->getFullName();
  // Some labels are reserved for the particle loader which loads all particles of the corresponding type.
  // If people applied cuts on these particle lists, very dangerous bugs could be introduced.
  // An exception is made for the gamma:all list. This can be limited to photons from the ECL only.
  if (Const::finalStateParticlesSet.contains(Const::ParticleType(abs(pdgCode))) and listLabel == "all") {
    if (abs(pdgCode) == Const::photon.getPDGCode() and m_cutParameter == "isFromECL")
      m_exceptionForGammaAll = true;
    else
      B2FATAL("You are trying to apply a cut on the list " << m_listName <<
              " but the label 'all' is protected for lists of final-state particles." <<
              " It could introduce *very* dangerous bugs.");
  } else if (listLabel == "V0") {
    // the label V0 is also protected
    B2FATAL("You are trying to apply a cut on the list " << m_listName <<
            " but the label " << listLabel << " is protected and can not be reduced.");
  }

  m_particleList.isRequired(m_listName);

  m_cut = Variable::Cut::compile(m_cutParameter);

  B2INFO("ParticleSelector: " << m_listName);
  B2INFO("   -> With cuts  : " << m_cutParameter);
}

void ParticleSelectorModule::event()
{
  if (m_exceptionForGammaAll)
    m_particleList->setEditable(true);

  // loop over list only if cuts should be applied
  if (!m_cutParameter.empty()) {
    std::vector<unsigned int> toRemove;
    unsigned int n = m_particleList->getListSize();
    for (unsigned i = 0; i < n; i++) {
      const Particle* part = m_particleList->getParticle(i);
      if (!m_cut->check(part)) toRemove.push_back(part->getArrayIndex());
    }
    m_particleList->removeParticles(toRemove);
  }

  if (m_exceptionForGammaAll)
    m_particleList->setEditable(false);
}
