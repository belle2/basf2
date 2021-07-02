/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/MCDecayFinder/MCDecayFinderModule.h>
#include <framework/gearbox/Const.h>
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/utility/EvtPDLUtil.h>
#include <analysis/utility/MCMatching.h>

#include <string>
#include <memory>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(MCDecayFinder)

MCDecayFinderModule::MCDecayFinderModule() : Module(), m_isSelfConjugatedParticle(false)
{
  //Set module properties
  setDescription("Find decays in MCParticle list matching a given DecayString and create Particles from them.");
  //Parameter definition
  addParam("decayString", m_strDecay, "DecayDescriptor string.");
  addParam("listName", m_listName, "Name of the output particle list");
  addParam("writeOut", m_writeOut,
           "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);
}

void MCDecayFinderModule::initialize()
{
  m_decaydescriptor.init(m_strDecay);

  m_antiListName = ParticleListName::antiParticleListName(m_listName);
  m_isSelfConjugatedParticle = (m_listName == m_antiListName);

  B2DEBUG(10, "particle list name: " << m_listName);
  B2DEBUG(10, "antiparticle list name: " << m_antiListName);


  // Register output particle list, particle store and relation to MCParticles
  m_mcparticles.isRequired();

  DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
  m_outputList.registerInDataStore(m_listName, flags);
  m_particles.registerInDataStore(flags);
  m_extraInfoMap.registerInDataStore();
  m_particles.registerRelationTo(m_mcparticles, DataStore::c_Event, flags);

  if (!m_isSelfConjugatedParticle) {
    m_antiOutputList.registerInDataStore(m_antiListName, flags);
  }
}

void MCDecayFinderModule::event()
{
  // Create output particle list
  m_outputList.create();
  m_outputList->initialize(m_decaydescriptor.getMother()->getPDGCode(), m_listName);

  if (!m_isSelfConjugatedParticle) {
    m_antiOutputList.create();
    m_antiOutputList->initialize(-1 * m_decaydescriptor.getMother()->getPDGCode(), m_antiListName);
    m_outputList->bindAntiParticleList(*(m_antiOutputList));
  }

  // loop over all MCParticles
  int nMCParticles = m_mcparticles.getEntries();
  for (int i = 0; i < nMCParticles; i++) {
    for (int iCC = 0; iCC < 2; iCC++) {
      std::unique_ptr<DecayTree<MCParticle>> decay(match(m_mcparticles[i], m_decaydescriptor, iCC));
      if (decay->getObj()) {
        B2DEBUG(19, "Match!");
        int iIndex = write(decay.get());
        m_outputList->addParticle(m_particles[iIndex]);
      }
    }
  }
}

DecayTree<MCParticle>* MCDecayFinderModule::match(const MCParticle* mcp, const DecayDescriptor* d, bool isCC)
{
  // Suffixes used in this method:
  // P = Information from MCParticle
  // D = Information from DecayDescriptor

  // Create empty DecayTree as return value
  auto* decay = new DecayTree<MCParticle>();

  // Load PDG codes and compare,
  int iPDGD = d->getMother()->getPDGCode();
  int iPDGP = mcp->getPDG();

  bool isSelfConjugatedParticle = !(EvtPDLUtil::hasAntiParticle(iPDGD));

  if (!isCC && iPDGD != iPDGP) return decay;
  else if (isCC && (iPDGD != -iPDGP && !isSelfConjugatedParticle)) return decay;
  else if (isCC && (iPDGD !=  iPDGP &&  isSelfConjugatedParticle)) return decay;
  B2DEBUG(19, "PDG code matched: " << iPDGP);

  // Get number of daughters in the decay descriptor.
  // If no daughters in decay descriptor, no more checks needed.
  int nDaughtersD = d->getNDaughters();
  if (nDaughtersD == 0) {
    B2DEBUG(19, "DecayDescriptor has no Daughters, everything OK!");
    decay->setObj(const_cast<MCParticle*>(mcp));
    return decay;
  }
  // Get number of daughter recursively if missing intermediate states are accepted.
  int nDaughtersRecursiveD = nDaughtersD;
  if (d->isIgnoreIntermediate()) {
    nDaughtersRecursiveD = getNDaughtersRecursive(d);
  }

  // Get daughters of MCParticle
  vector<const MCParticle*> daughtersP;
  if (d->isIgnoreIntermediate()) {
    appendParticles(mcp, daughtersP);
  } else {
    const vector<MCParticle*>& tmpDaughtersP = mcp->getDaughters();
    for (auto daug : tmpDaughtersP)
      daughtersP.push_back(daug);
  }

  // The MCParticle must have at least as many daughters as the decaydescriptor
  int nDaughtersP = daughtersP.size();
  if (nDaughtersRecursiveD > nDaughtersP) {
    B2DEBUG(10, "DecayDescriptor has more daughters than MCParticle!");
    return decay;
  }

  // loop over all daughters of the decay descriptor
  for (int iDD = 0; iDD < nDaughtersD; iDD++) {
    // check if there is an unmatched particle daughter matching this decay descriptor daughter
    bool isMatchDaughter = false;
    auto itDP = daughtersP.begin();
    while (itDP != daughtersP.end()) {
      const MCParticle* daugP = *itDP;
      DecayTree<MCParticle>* daughter = match(daugP, d->getDaughter(iDD), isCC);
      if (!daughter->getObj()) {
        ++itDP;
        continue;
      }
      // Matching daughter found, remove it from list of unmatched particle daughters
      decay->append(daughter);
      isMatchDaughter = true;
      itDP = daughtersP.erase(itDP);

      // if the matched daughter has daughters, they are also removed from the list
      if (d->isIgnoreIntermediate() and d->getDaughter(iDD)->getNDaughters() != 0) {
        vector<const MCParticle*> grandDaughtersP;
        if (d->getDaughter(iDD)->isIgnoreIntermediate()) {
          appendParticles(daugP, grandDaughtersP);
        } else {
          const vector<MCParticle*>& tmpGrandDaughtersP = daugP->getDaughters();
          for (auto grandDaugP : tmpGrandDaughtersP)
            grandDaughtersP.push_back(grandDaugP);
        }

        for (auto grandDaugP : grandDaughtersP) {
          auto jtDP = itDP;
          while (jtDP != daughtersP.end()) {
            const MCParticle* daugP_j = *jtDP;
            // if a grand-daughter matched a daughter, remove it.
            if (grandDaugP == daugP_j)
              jtDP = daughtersP.erase(jtDP);
            else
              ++jtDP;
          }
        }
      }

      break;
    }
    if (!isMatchDaughter) {
      return decay;
    }
  }

  // Ok, it seems that everything from the DecayDescriptor could be matched.
  // If the decay is NOT INCLUSIVE,  no unmatched MCParticles should be left
  bool isInclusive = (d->isIgnoreMassive() and d->isIgnoreNeutrino() and d->isIgnoreGamma() and d->isIgnoreRadiatedPhotons());
  if (!isInclusive) {
    B2DEBUG(10, "Check for left over MCParticles!\n");
    for (auto daugP : daughtersP) {
      if (daugP->getPDG() == Const::photon.getPDGCode()) { // gamma
        // if gamma is FSR or produced by PHOTOS
        if (MCMatching::isFSR(daugP) or daugP->hasStatus(MCParticle::c_IsPHOTOSPhoton)) {
          // if the radiated photons are ignored, we can skip the particle
          if (d->isIgnoreRadiatedPhotons()) continue;
        }
        // else if missing gamma is ignored, we can skip the particle
        else if (d->isIgnoreGamma()) continue;
      } else if ((abs(daugP->getPDG()) == 12 or abs(daugP->getPDG()) == 14 or abs(daugP->getPDG()) == 16)) { // neutrino
        if (d->isIgnoreNeutrino()) continue;
      } else if (MCMatching::isFSP(daugP->getPDG())) { // other final state particle
        if (d->isIgnoreMassive()) continue;
      } else { // intermediate
        if (d->isIgnoreIntermediate()) continue;
      }
      B2DEBUG(10, "There was an additional particle left. Found " << daugP->getPDG());
      return decay;
    }
  }
  decay->setObj(const_cast<MCParticle*>(mcp));
  B2DEBUG(19, "Match found!");
  return decay;
}

int MCDecayFinderModule::write(DecayTree<MCParticle>* decay)
{
  // Create new Particle in particles array
  Particle* newParticle = m_particles.appendNew(decay->getObj());

  // set relation between the created Particle and the MCParticle
  newParticle->addRelationTo(decay->getObj());
  const int iIndex = m_particles.getEntries() - 1;

  // Now save also daughters of this MCParticle and set the daughter relation
  vector< DecayTree<MCParticle>* > daughters = decay->getDaughters();
  int nDaughers = daughters.size();
  for (int i = 0; i < nDaughers; ++i) {
    int iIndexDaughter = write(daughters[i]);
    newParticle->appendDaughter(iIndexDaughter);
  }
  return iIndex;
}

void MCDecayFinderModule::appendParticles(const MCParticle* gen, vector<const MCParticle*>& children)
{
  if (MCMatching::isFSP(gen->getPDG()))
    return; //stop at the bottom of the MC decay tree (ignore secondaries)

  const vector<MCParticle*>& genDaughters = gen->getDaughters();
  for (auto daug : genDaughters) {
    children.push_back(daug);
    appendParticles(daug, children);
  }
}

int MCDecayFinderModule::getNDaughtersRecursive(const DecayDescriptor* d)
{
  const int nDaughter = d->getNDaughters();
  if (nDaughter == 0) return nDaughter;

  int nDaughterRecursive = nDaughter;
  for (int iDaug = 0; iDaug < nDaughter; iDaug++) {
    const DecayDescriptor* dDaug = d->getDaughter(iDaug);

    nDaughterRecursive += getNDaughtersRecursive(dDaug);
  }

  return nDaughterRecursive;
}
