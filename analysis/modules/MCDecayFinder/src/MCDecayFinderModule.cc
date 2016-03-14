/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Marko Staric                           *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/MCDecayFinder/MCDecayFinderModule.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <string>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(MCDecayFinder)

MCDecayFinderModule::MCDecayFinderModule() : Module()
{
  //Set module properties
  setDescription("Find decays in MCParticle list matching a given DecayString.");
  //Parameter definition
  addParam("strDecayString", m_strDecay, "DecayDescriptor string.", string(""));
  addParam("strListName", m_strListName, "Name of the output particle list", string(""));
  addParam("persistent", m_persistent,
           "toggle output particle list btw. transient/persistent", false);
}

void MCDecayFinderModule::initialize()
{
  B2WARNING("This is an untested prototype. Do not use for any production purposes.");

  if (m_strDecay.empty()) B2ERROR("No decay descriptor string provided.");
  m_decaydescriptor.init(m_strDecay);
  if (m_strListName.empty()) B2ERROR("No name of output particle list provided.");

  m_particleStore = std::string("FoundParticles") + m_strListName;
  B2INFO("particle store used: " << m_particleStore);

  // Register output particle list, particle store and relation to MCParticles

  if (m_persistent) {
    StoreObjPtr<ParticleList>::registerPersistent(m_strListName);
    StoreArray<Particle>::registerPersistent(m_particleStore);
    StoreObjPtr<ParticleExtraInfoMap>::registerPersistent("", DataStore::c_Event, false); //allow reregistration
    RelationArray::registerPersistent<Particle, MCParticle>(m_particleStore, "");
  } else {
    StoreObjPtr<ParticleList>::registerTransient(m_strListName);
    StoreArray<Particle>::registerTransient(m_particleStore);
    StoreObjPtr<ParticleExtraInfoMap>::registerTransient("", DataStore::c_Event, false); //allow reregistration
    RelationArray::registerTransient<Particle, MCParticle>(m_particleStore, "");
  }
}

void MCDecayFinderModule::event()
{
  // particle store (working space)
  StoreArray<Particle> particles(m_particleStore);
  particles.create();

  // Get output particle list
  StoreObjPtr<ParticleList> outputList(m_strListName);
  outputList.create();
  outputList->setPDG(m_decaydescriptor.getMother()->getPDGCode());
  outputList->setParticleCollectionName(m_particleStore);

  // retrieve list of MCParticles
  StoreArray<MCParticle> mcparticles;
  if (!mcparticles) {
    B2ERROR("MCParticle list not found!");
    return;
  }


  // loop over all MCParticles
  int nMCParticles = mcparticles.getEntries();
  for (int i = 0; i < nMCParticles; i++) {
    for (int iCC = 0; iCC < 2; iCC++) {
      DecayTree<MCParticle>* decay = match(mcparticles[i], m_decaydescriptor, iCC);
      if (decay->getObj()) {
        B2INFO("Match!");
        int iIndex = write(decay);
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
  if (!isCC && iPDGD != iPDGP) return decay;
  else if (isCC && iPDGD != -iPDGP) return decay;
  B2INFO("PDG code matched: " << iPDGP);

  // Get number of daughters in the decay descriptor.
  // If no daughters in decay descriptor, no more checks needed.
  int nDaughtersD = d->getNDaughters();
  if (nDaughtersD == 0) {
    B2INFO("DecayDescriptor has no Daughters, everything OK!");
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
    B2INFO("DecayDescriptor has more daughters than MCParticle!");
    return decay;
  } else if (!isIgnorePhotons && nDaughtersD != nDaughtersP) {
    B2INFO("DecayDescriptor must have same number of daughters as MCParticle!");
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
    B2INFO("Decay is not inclusive, check for left over MCParticles!\n")
    for (vector<int>::iterator itDP = daughtersPIndex.begin(); itDP != daughtersPIndex.end(); ++itDP) {
      // Check if additional photons are to be ignored
      B2INFO("Found a " << daughtersP[*itDP]->getPDG());
      if (isIgnorePhotons && daughtersP[*itDP]->getPDG() == 22) continue;
      B2INFO("There was an additional particle left which is not a photon. Something is going wrong here!");
      return decay;
    }
  } else {
    B2INFO("Inclusive decay, no need to check for additional unmatched particles!");
  }
  decay->setObj(const_cast<MCParticle*>(mcp));
  B2INFO("Match found!");
  return decay;
}

int MCDecayFinderModule::write(DecayTree<MCParticle>* decay)
{
  // Particle array for output
  StoreArray<Particle> particles(m_particleStore);
  // Input MCParticle array
  StoreArray<MCParticle> mcparticles;
  // Relation between particles and MCParticles
  RelationArray particle2mcparticle(particles, mcparticles);

  // Create new Particle in particles array
  new(particles.nextFreeAddress()) Particle(decay->getObj());

  // set relation between the created Particle and the MCParticle
  int iIndex = particles.getEntries() - 1;
  particle2mcparticle.add(iIndex, decay->getObj()->getArrayIndex());

  // Now save also daughters of this MCParticle and set the daughter relation
  vector< DecayTree<MCParticle>* > daughters = decay->getDaughters();
  int nDaughers = daughters.size();
  for (int i = 0; i < nDaughers; ++i) {
    int iIndexDaughter = write(daughters[i]);
    particles[iIndex]->appendDaughter(iIndexDaughter);
  }
  return iIndex;
}
