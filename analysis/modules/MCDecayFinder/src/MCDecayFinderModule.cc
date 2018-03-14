/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Marko Staric, Phillip Urquijo          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/MCDecayFinder/MCDecayFinderModule.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/utility/EvtPDLUtil.h>

#include <string>
#include <memory>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(MCDecayFinder)

MCDecayFinderModule::MCDecayFinderModule() : Module()
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
  B2WARNING("MCDecayFinder is not yet configured to deal with matches that require intermediate resonances to be ignored.");

  m_decaydescriptor.init(m_strDecay);

  m_antiListName = ParticleListName::antiParticleListName(m_listName);
  m_isSelfConjugatedParticle = (m_listName == m_antiListName);

  B2DEBUG(10, "particle list name: " << m_listName);
  B2DEBUG(10, "antiparticle list name: " << m_antiListName);


  // Register output particle list, particle store and relation to MCParticles
  StoreObjPtr<ParticleList> particleList(m_listName);
  StoreArray<Particle> particles;
  StoreObjPtr<ParticleExtraInfoMap> extraInfoMap;
  StoreArray<MCParticle> mcparticles;
  mcparticles.isRequired();

  DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
  particleList.registerInDataStore(flags);
  particles.registerInDataStore(flags);
  extraInfoMap.registerInDataStore();
  particles.registerRelationTo(mcparticles, DataStore::c_Event, flags);

  if (!m_isSelfConjugatedParticle) {
    StoreObjPtr<ParticleList> antiParticleList(m_antiListName);
    antiParticleList.registerInDataStore(flags);
  }
}

void MCDecayFinderModule::event()
{
  // particle store (working space)
  StoreArray<Particle> particles(m_particleStore);

  // Get output particle list
  StoreObjPtr<ParticleList> outputList(m_listName);
  outputList.create();
  outputList->initialize(m_decaydescriptor.getMother()->getPDGCode(), m_listName);

  if (!m_isSelfConjugatedParticle) {
    StoreObjPtr<ParticleList> antiOutputList(m_antiListName);
    antiOutputList.create();
    antiOutputList->initialize(-1 * m_decaydescriptor.getMother()->getPDGCode(), m_antiListName);
    outputList->bindAntiParticleList(*(antiOutputList));
  }

  // loop over all MCParticles
  StoreArray<MCParticle> mcparticles;
  int nMCParticles = mcparticles.getEntries();
  for (int i = 0; i < nMCParticles; i++) {
    for (int iCC = 0; iCC < 2; iCC++) {
      std::unique_ptr<DecayTree<MCParticle>> decay(match(mcparticles[i], m_decaydescriptor, iCC));
      if (decay->getObj()) {
        B2DEBUG(19, "Match!");
        int iIndex = write(decay.get());
        outputList->addParticle(particles[iIndex]);
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
  DecayTree<MCParticle>* decay = new DecayTree<MCParticle>();

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

  // Get daughters of MCParticle
  const vector<MCParticle*> daughtersP = mcp->getDaughters();
  int nDaughtersP = daughtersP.size();
  // Create index for MCParticle daughter list
  // The index of matched daughters will be then removed later from this list.
  vector<int> daughtersPIndex;
  for (int i = 0; i < nDaughtersP; i++) daughtersPIndex.push_back(i);

  // 1) if radiated photons are to be ignored, the MCParticle must
  // have at least as many daughters as the decaydescriptor
  // 2) if radiated photons are included the number of daughters has to be equal!
  bool isIgnorePhotons = d->isIgnorePhotons();
  if (isIgnorePhotons && nDaughtersD > nDaughtersP) {
    B2DEBUG(10, "DecayDescriptor has more daughters than MCParticle!");
    return decay;
  } else if (!isIgnorePhotons && nDaughtersD != nDaughtersP) {
    B2DEBUG(10, "DecayDescriptor must have same number of daughters as MCParticle!");
    return decay;
  }

  // loop over all daughters of the decay descriptor
  for (int iDD = 0; iDD < nDaughtersD; iDD++) {
    // check if there is an unmatched particle daughter matching this decay descriptor daughter
    bool isMatchDaughter = false;
    for (vector<int>::iterator itDP = daughtersPIndex.begin(); itDP != daughtersPIndex.end(); ++itDP) {
      DecayTree<MCParticle>* daughter = match(daughtersP[*itDP], d->getDaughter(iDD), isCC);
      if (!daughter->getObj()) continue;
      // Matching daughter found, remove it from list of unmatched particle daughters
      decay->append(daughter);
      isMatchDaughter = true;
      daughtersPIndex.erase(itDP);
      break;
    }
    // Could this daughter from the DecayDescriptor be matched?
    if (!isMatchDaughter) {
      return decay;
      /*
      // if allowed, try to skip intermediate resonance!
      bool isIgnoreResonances = false;
      if (!isIgnoreResonances) return decay;
      for (vector<int>::iterator itDP1 = daughtersPIndex.begin(); itDP1 != daughtersPIndex.end(); ++itDP1) {
          const vector<MCParticle*> daughtersP2 = daughtersP[*itDP1]->getDaughters();
          int nDaughtersP2 = daughtersP2.size();
          // Create index for MCParticle daughter list
          // The index of matched daughters will be then removed later from this list.
          vector<int> daughtersPIndex2;
          for (int i=0; i<nDaughtersP2; i++) daughtersPIndex.push_back(i);
          for (vector<int>::iterator itDP2 = daughtersPIndex.begin(); itDP2 != daughtersPIndex.end(); ++itDP2) {
              DecayTree<MCParticle>* daughter = match(daughtersP2[*itDP2], d.getDaughter(iDD), isCC);
              if (!daughter->getObj()) continue;
              // Matching daughter found, remove it from list of unmatched particle daughters
              decay->append(daughter);
              isMatchDaughter = true;
              daughtersPIndex2.erase(itDP2);
              break;
          }
      */
    }

  }
  // Ok, it seems that everything from the DecayDescriptor could be matched.
  // If the decay is NOT INCLUSIVE,  no unmatched MCParticles should be left
  bool isInclusive = d->isInclusive();
  if (!isInclusive) {
    B2DEBUG(10, "Decay is not inclusive, check for left over MCParticles!\n");
    for (vector<int>::iterator itDP = daughtersPIndex.begin(); itDP != daughtersPIndex.end(); ++itDP) {
      // Check if additional photons are to be ignored
      if (isIgnorePhotons && daughtersP[*itDP]->getPDG() == 22) continue;
      B2DEBUG(10, "There was an additional particle left which is not a photon. Found " << daughtersP[*itDP]->getPDG());
      return decay;
    }
  } else {
    B2DEBUG(19, "Inclusive decay, no need to check for additional unmatched particles!");
  }
  decay->setObj(const_cast<MCParticle*>(mcp));
  B2DEBUG(19, "Match found!");
  return decay;
}

int MCDecayFinderModule::write(DecayTree<MCParticle>* decay)
{
  // Particle array for output
  StoreArray<Particle> particles;
  // Input MCParticle array
  StoreArray<MCParticle> mcparticles;

  // Create new Particle in particles array
  Particle* newParticle = particles.appendNew(decay->getObj());

  // set relation between the created Particle and the MCParticle
  newParticle->addRelationTo(decay->getObj());
  const int iIndex = particles.getEntries() - 1;

  // Now save also daughters of this MCParticle and set the daughter relation
  vector< DecayTree<MCParticle>* > daughters = decay->getDaughters();
  int nDaughers = daughters.size();
  for (int i = 0; i < nDaughers; ++i) {
    int iIndexDaughter = write(daughters[i]);
    newParticle->appendDaughter(iIndexDaughter);
  }
  return iIndex;
}
