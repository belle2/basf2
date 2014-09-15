// ******************************************************************
// MC Matching
// authors: A. Zupanc (anze.zupanc@ijs.si), C. Pulvermacher (christian.pulvermacher@kit.edu)
// ******************************************************************

#include <analysis/utility/MCMatching.h>
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/StoreArray.h>

#include <unordered_set>

using namespace Belle2;
using namespace std;

void MCMatching::fillGenMothers(const MCParticle* mcP, vector<int>& genMCPMothers)
{
  while (mcP) {
    genMCPMothers.push_back(mcP->getIndex());
    mcP = mcP->getMother();
  }
}


int MCMatching::findCommonMother(const MCParticle* mcP, const vector<int>& firstMothers, int lastMother)
{
  while (mcP) {
    int idx = mcP->getIndex();
    for (unsigned int i = lastMother; i < firstMothers.size(); i++) {
      if (firstMothers[i] == idx)
        return i;
    }

    mcP = mcP->getMother();
  }
  //not found
  return -1;
}


bool MCMatching::setMCTruth(const Particle* particle)
{
  //if MCTruthStatus is set, we already handled this particle
  //TODO check wether this actually speeds things up or not
  if (particle->hasExtraInfo("MCTruthStatus"))
    return true;

  const MCParticle* mcParticle = particle->getRelatedTo<MCParticle>();
  if (mcParticle) { //nothing to do
    return true;
  }

  int nChildren = particle->getNDaughters();
  if (nChildren == 0) {
    //no daughters -> should be an FSP, but no related MCParticle. Probably background.
    return false;
  }

  // check, if for all daughter particles Particle -> MCParticle relation exists
  for (int i = 0; i < nChildren; ++i) {
    const Particle* daugP = particle->getDaughter(i);
    //returns quickly when found
    bool daugMCTruth = setMCTruth(daugP);
    if (!daugMCTruth)
      return false;
  }

  int motherIndex = 0;
  if (nChildren == 1) {
    // assign mother of MCParticle related to our daughter
    const Particle*    daugP   = particle->getDaughter(0);
    const MCParticle*  daugMCP = daugP->getRelatedTo<MCParticle>();
    if (!daugMCP)
      return false;
    const MCParticle* mom = daugMCP->getMother();
    if (!mom)
      return false;
    motherIndex = mom->getIndex();

  } else {
    // at this stage for all daughters particles the  Particle <-> MCParticle relation exists
    // first fill vector with indices of all mothers of first daughter,
    // then search common mother for each other daughter

    vector<int> firstDaugMothers; // indices of generated mothers of first daughter

    int lastMother = 0; //index in firstDaugMothers (start with first daughter itself)
    for (int i = 0; i < nChildren; ++i) {
      const Particle*    daugP   = particle->getDaughter(i);
      const MCParticle*  daugMCP = daugP->getRelatedTo<MCParticle>();

      if (i == 0) {
        fillGenMothers(daugMCP, firstDaugMothers);
      } else {
        lastMother = findCommonMother(daugMCP, firstDaugMothers, lastMother);
        if (lastMother == -1)
          break; //not found
      }
    }
    if (lastMother >= 0)
      motherIndex = firstDaugMothers[lastMother];
  }

  // if index is less than 1, the common mother particle was not found
  // remember: it's 1-based index
  if (motherIndex < 1)
    return false;

  // finally the relation can be set
  StoreArray<MCParticle> mcParticles;

  // sanity check
  if (motherIndex > mcParticles.getEntries()) {
    B2ERROR("setMCTruth(): sanity check failed!");
    return false;
  }

  const MCParticle* mcMatch = mcParticles[motherIndex - 1];
  particle->addRelationTo(mcMatch);

  return true;
}

int MCMatching::setMCTruthStatus(Particle* particle, const MCParticle* mcParticle)
{
  auto setStatus = [](Particle * particle, int s) -> int {
    particle->addExtraInfo("MCTruthStatus", s);
    return s;
  };

  int status = 0;

  if (!mcParticle)
    return setStatus(particle, status | MCMatchStatus::c_InternalError);

  unsigned nChildren = particle->getNDaughters();
  if (nChildren == 0) { //FSP-like
    if (particle->getPDGCode() != mcParticle->getPDG()) {
      if (!mcParticle->hasStatus(MCParticle::c_PrimaryParticle)) {
        //secondary particle, so the original particle probably decayed
        status |= MCMatchStatus::c_DecayInFlight;
        //find first primary mother
        const MCParticle* primary = mcParticle->getMother();
        while (primary and !primary->hasStatus(MCParticle::c_PrimaryParticle))
          primary = primary->getMother();

        if (!primary) {
          status |= MCMatchStatus::c_InternalError;
        } else if (particle->getPDGCode() != primary->getPDG()) {
          //if primary particle also has wrong PDG code, we're actually MisIDed
          status |= MCMatchStatus::c_MisID;
        }
      } else {
        status |= MCMatchStatus::c_MisID;
      }
    }

    //other checks concern daughters of particle, so we're done here
    return setStatus(particle, status);
  }
  const Particle::EFlavorType flavorType = particle->getFlavorType();
  if ((flavorType == Particle::c_Flavored and particle->getPDGCode() != mcParticle->getPDG())
      or (flavorType == Particle::c_Unflavored and abs(particle->getPDGCode()) != abs(mcParticle->getPDG())))
    status |= MCMatchStatus::c_AddedWrongParticle;

  //add up all (accepted) status flags we collected for our daughters
  const int daughterStatusAcceptMask = c_MisID | c_AddedWrongParticle | c_DecayInFlight | c_InternalError;
  int daughterStatus = 0;
  for (unsigned i = 0; i < nChildren; ++i) {
    const Particle* daughter = particle->getDaughter(i);
    daughterStatus |= getMCTruthStatus(daughter);
  }
  status |= (daughterStatus & daughterStatusAcceptMask);


  //int genMotherPDG = mcParticle->getPDG();
  // TODO: fix this (aim for no hard coded values)
  // TODO: is this neccessary ?
  // -> might speed up things by removing combinatorics  (we don't need missingX flags there)
  //if (genMotherPDG == 10022 || genMotherPDG == 300553 || genMotherPDG == 9000553)
  //  return -1;

  // Ks doesn't have daughters in gen_hepevt table
  // TODO: is there any better way
  //if (genMotherPDG == 310)
  //return 1;

  // fill vectors of reconstructed and generated final state particles
  vector<const Particle*>   recFSPs;
  vector<const MCParticle*> genFSPs;

  int reconstructedNonFSPs = appendFSP(particle,   recFSPs);
  int mcNonFSPs = appendFSP(mcParticle, genFSPs);
  if (mcNonFSPs > reconstructedNonFSPs)
    status |= c_MissingResonance;

  // TODO: do something
  if (genFSPs.empty())
    return setStatus(particle, status | MCMatchStatus::c_InternalError);

  //This might happen with  e.g two ECLClusters from a charged track
  if (recFSPs.size() > genFSPs.size())
    return setStatus(particle, status | MCMatchStatus::c_InternalError);

  status |= getMissingParticleFlags(recFSPs, genFSPs);

  return setStatus(particle, status);
}

int MCMatching::getMCTruthStatus(const Particle* particle, const MCParticle* mcParticle)
{
  if (particle->hasExtraInfo("MCTruthStatus")) {
    return particle->getExtraInfo("MCTruthStatus");
  } else {
    if (!mcParticle)
      mcParticle = particle->getRelatedTo<MCParticle>();
    return setMCTruthStatus(const_cast<Particle*>(particle), mcParticle);
  }
}


int MCMatching::appendFSP(const Particle* p, vector<const Particle*>& children)
{
  int nonFSPs = 0;
  for (unsigned i = 0; i < p->getNDaughters(); ++i) {
    const Particle* daug = p->getDaughter(i);

    // TODO: fix this (aim for no hard coded values)
    // don't add K_S^0 daughters since they are also in isFSPs()
    if (daug->getNDaughters() && daug->getPDGCode() != 310) {
      nonFSPs += 1 + appendFSP(daug, children);
    } else {
      children.push_back(daug);
    }
  }
  return nonFSPs;
}

int MCMatching::appendFSP(const MCParticle* gen, vector<const MCParticle*>& children)
{
  int nonFSPs = 0;
  const vector<MCParticle*>& genDaughters = gen->getDaughters();
  for (unsigned i = 0; i < genDaughters.size(); ++i) {
    const MCParticle* daug = genDaughters[i];

    int nDaughs = daug->getNDaughters();
    if (nDaughs && !isFSP(daug)) {
      nonFSPs += 1 + appendFSP(daug, children);
    } else {
      children.push_back(daug);
    }
  }
  return nonFSPs;
}

bool MCMatching::isFSP(const MCParticle* p)
{
  switch (abs(p->getPDG())) {
    case 211:
    case 321:
    case 11:
    case 12:
    case 13:
    case 14:
    case 16:
    case 22:
    case 2212:
    case 310:
    case 130:
    case 2112:
      return true;
    default:
      return false;
  }
}

bool MCMatching::isFSR(const MCParticle* p)
{
  const MCParticle* mother = p->getMother();
  if (!mother) {
    B2ERROR("The hell? why would this gamma not have a mother?");
    return false; //?
  }

  int ndaug = mother->getNDaughters();
  if (ndaug > 2) { // M -> A B (...) gamma is probably FSR
    return true;
  } else { // M -> A gamma is probably a decay
    return false;
  }
}

int MCMatching::getMissingParticleFlags(const std::vector<const Particle*>& reconstructedFSPs, const std::vector<const MCParticle*>& generatedFSPs)
{
  int flags = 0;

  //a) same number for reconstructed and generated FSPs: nothing missing (provided the common mother is the same)
  //b) more FSPs reconstructed than generated: most likely something like K_S, which counts as FSP. TODO: I don't think this should happen when K_S = 310 gets special handling in appendFSP(Particle*, ...)
  if (reconstructedFSPs.size() >= generatedFSPs.size())
    return flags;

  //matched MCParticles of reconstructed FSPs
  std::unordered_set<const MCParticle*> mcMatchedFSPs;
  for (const Particle * rec : reconstructedFSPs) {
    const MCParticle* mcParticle = rec->getRelatedTo<MCParticle>();
    if (mcParticle)
      mcMatchedFSPs.insert(mcParticle);
  }

  for (const MCParticle * genFSP : generatedFSPs) {
    const bool missing = (mcMatchedFSPs.find(genFSP) == mcMatchedFSPs.end());
    if (missing) {
      //we want to set a flag, so what kind of particle is genFSP?
      const int generatedPDG = genFSP->getPDG();
      const int absGeneratedPDG = abs(generatedPDG);
      if (generatedPDG == 22) { //missing photon
        if (!(flags & c_MissFSR) or !(flags & c_MissGamma)) {
          if (isFSR(genFSP))
            flags |= c_MissFSR;
          else
            flags |= c_MissGamma;
        }

      } else if (absGeneratedPDG == 12 || absGeneratedPDG == 14 || absGeneratedPDG == 16) { // missing neutrino
        flags |= c_MissNeutrino;

      } else { //neither photon nor neutrino -> massive
        flags |= c_MissMassiveParticle;
        if (absGeneratedPDG == 130) {
          flags |= c_MissKlong;
        }
      }
    }
  }
  return flags;
}
