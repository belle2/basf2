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
REG_MODULE(MCDecayFinder);

MCDecayFinderModule::MCDecayFinderModule() : Module(), m_isSelfConjugatedParticle(false)
{
  //Set module properties
  setDescription("Find decays in MCParticle list matching a given DecayString and create Particles from them.");
  //Parameter definition
  addParam("decayString", m_strDecay, "DecayDescriptor string.");
  addParam("listName", m_listName, "Name of the output particle list");
  addParam("appendAllDaughters", m_appendAllDaughters,
           "If true, all daughters of the matched MCParticle will be added in the order defined at the MCParticle. "
           "If false, only the daughters described in the given decayString will be appended to the output particle.",
           false);
  addParam("skipNonPrimaryDaughters", m_skipNonPrimaryDaughters,
           "If true, the secondary MC daughters will be skipped to append to the output particles. Default: true",
           true);
  addParam("writeOut", m_writeOut,
           "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);
}

void MCDecayFinderModule::initialize()
{
  bool valid = m_decaydescriptor.init(m_strDecay);
  if (!valid)
    B2ERROR("Invalid input DecayString: " << m_strDecay);

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
  if (not m_extraInfoMap)
    m_extraInfoMap.create();

  // Create output particle list
  const int motherPDG = m_decaydescriptor.getMother()->getPDGCode();
  m_outputList.create();
  m_outputList->initialize(motherPDG, m_listName);

  if (!m_isSelfConjugatedParticle) {
    m_antiOutputList.create();
    m_antiOutputList->initialize(-1 * motherPDG, m_antiListName);
    m_outputList->bindAntiParticleList(*(m_antiOutputList));
  }

  // loop over all MCParticles
  int nMCParticles = m_mcparticles.getEntries();
  for (int i = 0; i < nMCParticles; i++) {

    if (abs(m_mcparticles[i]->getPDG()) != abs(motherPDG))
      continue;

    for (int iCC = 0; iCC < 2; iCC++) {
      int arrayIndex = -1;
      std::unique_ptr<DecayTree<MCParticle>> decay(match(m_mcparticles[i], m_decaydescriptor, iCC, arrayIndex));

      MCParticle* mcp = decay->getObj();
      if (!mcp) continue;

      B2DEBUG(19, "Match!");

      if (m_appendAllDaughters) {
        // if m_appendAllDaughters is True, create a new Particle appending all daughters
        Particle* newParticle = m_particles.appendNew(mcp);
        newParticle->addRelationTo(mcp);

        for (auto mcDaughter : mcp->getDaughters()) {
          if (mcDaughter->hasStatus(MCParticle::c_PrimaryParticle) or not m_skipNonPrimaryDaughters
              or abs(mcp->getPDG()) == Const::Lambda.getPDGCode()) {  // Lambda's daughters are not primary but it is not FSP at mcmatching

            auto partDaughter = createParticleRecursively(mcDaughter, m_skipNonPrimaryDaughters);
            newParticle->appendDaughter(partDaughter, /* updateType = */ false); // particleSource remain c_MCParticle
          }
        }
        m_outputList->addParticle(newParticle);

      } else if (arrayIndex == -1) {
        // Particle is not created when no daughter is described in decayString
        Particle* newParticle = m_particles.appendNew(mcp);
        newParticle->addRelationTo(mcp);

        m_outputList->addParticle(newParticle);

      } else {
        // Particle is already created
        m_outputList->addParticle(m_particles[arrayIndex]);
      }

    }
  }
}

DecayTree<MCParticle>* MCDecayFinderModule::match(const MCParticle* mcp, const DecayDescriptor* d, bool isCC, int& arrayIndex)
{
  // Suffixes used in this method:
  // P = Information from MCParticle
  // D = Information from DecayDescriptor

  // Create empty DecayTree as return value
  auto* decay = new DecayTree<MCParticle>();

  // Load PDG codes and compare,
  const int iPDGD = d->getMother()->getPDGCode();
  const int iPDGP = mcp->getPDG();
  const bool isSelfConjugatedParticle = !(EvtPDLUtil::hasAntiParticle(iPDGD));

  if (!isCC && iPDGD != iPDGP) return decay;
  else if (isCC && (iPDGD != -iPDGP && !isSelfConjugatedParticle)) return decay;
  else if (isCC && (iPDGD !=  iPDGP &&  isSelfConjugatedParticle)) return decay;
  B2DEBUG(19, "PDG code matched: " << iPDGP);

  // Get number of daughters in the decay descriptor.
  // If no daughters in decay descriptor, no more checks needed.
  const int nDaughtersD = d->getNDaughters();
  if (nDaughtersD == 0) {
    B2DEBUG(19, "DecayDescriptor has no Daughters, everything OK!");
    decay->setObj(const_cast<MCParticle*>(mcp));
    return decay; // arrayIndex is not set
  }

  // Get daughters of MCParticle
  vector<const MCParticle*> daughtersP;
  int nDaughtersRecursiveD = 0;
  if (d->isIgnoreIntermediate()) {
    appendParticles(mcp, daughtersP);
    // Get number of daughter recursively if missing intermediate states are accepted.
    nDaughtersRecursiveD = getNDaughtersRecursive(d);
  } else {
    const vector<MCParticle*>& tmpDaughtersP = mcp->getDaughters();
    for (auto daug : tmpDaughtersP)
      daughtersP.push_back(daug);
    nDaughtersRecursiveD = nDaughtersD;
  }

  // The MCParticle must have at least as many daughters as the decaydescriptor
  if (nDaughtersRecursiveD > (int)daughtersP.size()) {
    B2DEBUG(10, "DecayDescriptor has more daughters than MCParticle!");
    return decay;
  }

  // nested daughter has to be called at first to avoid overlap
  std::vector<std::pair<int, int>> daughtersDepthMapD; // first: -1*depth, second:iDD
  for (int iDD = 0; iDD < nDaughtersD; iDD++) {
    int depth = 1; // to be incremented
    countMaxDepthOfNest(d->getDaughter(iDD), depth);

    daughtersDepthMapD.push_back({-1 * depth, iDD});
  }
  // sorted in ascending order = the deepest nested daughter will come first
  std::sort(daughtersDepthMapD.begin(), daughtersDepthMapD.end());

  // loop over all daughters of the decay descriptor
  for (auto pair : daughtersDepthMapD) {
    int iDD = pair.second;
    // check if there is an unmatched particle daughter matching this decay descriptor daughter
    bool isMatchDaughter = false;
    auto itDP = daughtersP.begin();
    while (itDP != daughtersP.end()) {
      const MCParticle* daugP = *itDP;

      int tmp; // array index of a created daughter Particle (not to be used)
      DecayTree<MCParticle>* daughter = match(daugP, d->getDaughter(iDD), isCC, tmp);
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
  decay->setObj(const_cast<MCParticle*>(mcp));

  Particle* newParticle = buildParticleFromDecayTree(decay, d);
  int status = MCMatching::getMCErrors(newParticle);
  if (status != MCMatching::c_Correct) { // isSignal != 1
    B2DEBUG(10, "isSignal is not True. There was an additional particle left.");
    decay->setObj(nullptr);
    return decay;
  }

  arrayIndex = newParticle->getArrayIndex();

  B2DEBUG(19, "Match found!");
  return decay;
}

void MCDecayFinderModule::appendParticles(const MCParticle* gen, vector<const MCParticle*>& children)
{
  if (MCMatching::isFSP(gen->getPDG())) {
    if (gen->getPDG() != Const::Kshort.getPDGCode()) // exception for K_S0
      return; //stop at the bottom of the MC decay tree (ignore secondaries)

    // Currently the decay of "FSP" cannot be specified except for K_S0,
    // e.g. photon-conversion: gamma -> e+ e-, decay-in-flight: K+ -> mu+ nu_mu
  }

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

void MCDecayFinderModule::countMaxDepthOfNest(const DecayDescriptor* d, int& depth)
{
  int maxDepth = 0;
  for (int i = 0; i < d->getNDaughters(); i++) {
    int tmp_depth = 1;
    countMaxDepthOfNest(d->getDaughter(i), tmp_depth);

    if (tmp_depth > maxDepth)
      maxDepth = tmp_depth;
  }

  depth += maxDepth;
}

bool MCDecayFinderModule::performMCMatching(const DecayTree<MCParticle>* decay, const DecayDescriptor* dd)
{
  auto newParticle = buildParticleFromDecayTree(decay, dd);
  int status = MCMatching::getMCErrors(newParticle);
  return (status == MCMatching::c_Correct);
}


Particle* MCDecayFinderModule::buildParticleFromDecayTree(const DecayTree<MCParticle>* decay, const DecayDescriptor* dd)
{
  const int nDaughters = dd->getNDaughters();
  const vector<DecayTree<MCParticle>*> decayDaughters = decay->getDaughters();

  // sanity check
  if ((int)decayDaughters.size() != nDaughters) {
    B2ERROR("MCDecayFinderModule::buildParticleFromDecayTree Inconsistency on the number daughters between DecayTree and DecayDescriptor");
    return nullptr;
  }

  // build particle from head of DecayTree
  MCParticle* mcp = decay->getObj();
  Particle* newParticle = m_particles.appendNew(mcp);
  newParticle->addRelationTo(mcp);

  int property = dd->getProperty();
  property |= dd->getMother()->getProperty();
  newParticle->setProperty(property);

  // if nDaughters is 0 but mcp is not FSP, all primary daughters are appended for mcmatching
  if (nDaughters == 0 and not MCMatching::isFSP(mcp->getPDG())) {
    for (auto mcDaughter : mcp->getDaughters()) {
      if (mcDaughter->hasStatus(MCParticle::c_PrimaryParticle) or
          (abs(mcp->getPDG()) == Const::Lambda.getPDGCode())) { // Lambda's daughters are not primary but it is not FSP at mcmatching

        auto partDaughter = createParticleRecursively(mcDaughter, true); // for mcmatching non-primary should be omitted
        newParticle->appendDaughter(partDaughter, false);
      }
    }
  }


  // Daughters of DecayTree were filled in ascending order of depth of nest.
  std::vector<std::pair<int, int>> daughtersDepthMapD; // first: -1*depth, second:iDD
  for (int iDD = 0; iDD < nDaughters; iDD++) {
    auto daugD =  dd->getDaughter(iDD);

    int depth = 1; // to be incremented
    countMaxDepthOfNest(daugD, depth);
    daughtersDepthMapD.push_back({-1 * depth, iDD});
  }
  std::sort(daughtersDepthMapD.begin(), daughtersDepthMapD.end());

  for (int iDD = 0; iDD < nDaughters; iDD++) {

    int index_decayDaughter = 0;
    for (auto pair : daughtersDepthMapD) {
      if (pair.second == iDD) break;
      index_decayDaughter++;
    }

    Particle* partDaughter = buildParticleFromDecayTree(decayDaughters[index_decayDaughter], dd->getDaughter(iDD));

    int daughterProperty = dd->getDaughter(iDD)->getMother()->getProperty();
    newParticle->appendDaughter(partDaughter, false, daughterProperty);
  }

  return newParticle;
}

Particle* MCDecayFinderModule::createParticleRecursively(const MCParticle* mcp, bool skipNonPrimaryDaughters)
{
  Particle* newParticle = m_particles.appendNew(mcp);
  newParticle->addRelationTo(mcp);

  for (auto mcDaughter : mcp->getDaughters()) {
    if (mcDaughter->hasStatus(MCParticle::c_PrimaryParticle) or not skipNonPrimaryDaughters
        or abs(mcp->getPDG()) == Const::Lambda.getPDGCode()) { // Lambda's daughters are not primary but it is not FSP at mcmatching
      auto partDaughter = createParticleRecursively(mcDaughter, skipNonPrimaryDaughters);
      newParticle->appendDaughter(partDaughter, false);
    }
  }

  return newParticle;
}
