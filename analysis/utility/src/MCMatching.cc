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

const std::string MCMatching::c_extraInfoMCStatus = "MCTruthStatus";

std::string MCMatching::explainFlags(unsigned int flags)
{
  if (flags == c_Correct)
    return "c_Correct";

  std::string s;
  unsigned int f = 1;
  while (flags != 0) {
    if (flags & f) {
      switch (f) {
        case c_MissFSR             : s += "c_MissFSR"; break;
        case c_MissingResonance    : s += "c_MissingResonance"; break;
        case c_DecayInFlight       : s += "c_DecayInFlight"; break;
        case c_MissNeutrino        : s += "c_MissNeutrino"; break;
        case c_MissGamma           : s += "c_MissGamma"; break;
        case c_MissMassiveParticle : s += "c_MissMassiveParticle"; break;
        case c_MissKlong           : s += "c_MissKlong"; break;
        case c_MisID               : s += "c_MisID"; break;
        case c_AddedWrongParticle  : s += "c_AddedWrongParticle"; break;
        case c_InternalError       : s += "c_InternalError"; break;
        default:
          s += to_string(f);
          B2ERROR("MCMatching::explainFlags() doesn't know about flag " << f << ", please update it.")
      }
      flags -= f; //remove flag
      if (flags != 0)
        s += " | ";
    }
    f *= 2;
  }
  return s;
}

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
  if (particle->hasExtraInfo(c_extraInfoMCStatus))
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
    particle->addExtraInfo(c_extraInfoMCStatus, s);
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

  status |= getMissingParticleFlags(particle, mcParticle);

  return setStatus(particle, status);
}

int MCMatching::getMCTruthStatus(const Particle* particle, const MCParticle* mcParticle)
{
  if (particle->hasExtraInfo(c_extraInfoMCStatus)) {
    return particle->getExtraInfo(c_extraInfoMCStatus);
  } else {
    if (!mcParticle)
      mcParticle = particle->getRelatedTo<MCParticle>();
    return setMCTruthStatus(const_cast<Particle*>(particle), mcParticle);
  }
}

bool MCMatching::isFSP(int pdg)
{
  switch (abs(pdg)) {
    case 211:
    case 321:
    case 11:
    case 12:
    case 13:
    case 14:
    case 16:
    case 22:
    case 2212:
    case 310: // K0S
    case 130: // K0L
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

//utility functions used by getMissingParticleFlags()
namespace {
  using namespace MCMatching;

  /** Recursively gather all matched MCParticles in daughters of p (taking special care of decay-in-flight things). */
  void appendParticles(const Particle* p, unordered_set<const MCParticle*>& mcMatchedParticles)
  {
    for (unsigned i = 0; i < p->getNDaughters(); ++i) {
      const Particle* daug = p->getDaughter(i);

      //add matched MCParticle for 'daug'
      const MCParticle* mcParticle = daug->getRelatedTo<MCParticle>();
      if (mcParticle) {
        mcMatchedParticles.insert(mcParticle);
        if (daug->getNDaughters() == 0 and (unsigned int)daug->getExtraInfo(c_extraInfoMCStatus) & c_DecayInFlight) {
          //particle at the bottom of reconstructed decay tree, reconstructed from an MCParticle that is actually slightly deeper than we want,
          //so we'll also add all mother MCParticles until the first primary mother
          do {
            mcParticle = mcParticle->getMother();
            if (mcParticle)
              mcMatchedParticles.insert(mcParticle);
          } while (mcParticle and !mcParticle->hasStatus(MCParticle::c_PrimaryParticle));
        }
      }
      appendParticles(daug, mcMatchedParticles);
    }
  }

  /** Recursively gather all daughters of 'gen' we want to reconstruct. */
  void appendParticles(const MCParticle* gen, vector<const MCParticle*>& children)
  {
    if (MCMatching::isFSP(gen->getPDG()))
      return; //stop at the bottom of the MC decay tree (ignore secondaries)

    const vector<MCParticle*>& genDaughters = gen->getDaughters();
    for (unsigned i = 0; i < genDaughters.size(); ++i) {
      const MCParticle* daug = genDaughters[i];
      children.push_back(daug);
      appendParticles(daug, children);
    }
  }
}


int MCMatching::getMissingParticleFlags(const Particle* particle, const MCParticle* mcParticle)
{
  int flags = 0;

  unordered_set<const MCParticle*> mcMatchedParticles;
  appendParticles(particle, mcMatchedParticles);
  vector<const MCParticle*> genParts;
  appendParticles(mcParticle, genParts);

  for (const MCParticle * genPart : genParts) {
    const bool missing = (mcMatchedParticles.find(genPart) == mcMatchedParticles.end());
    if (missing) {

      //we want to set a flag, so what kind of particle is genPart?
      const int generatedPDG = genPart->getPDG();
      const int absGeneratedPDG = abs(generatedPDG);

      if (!isFSP(generatedPDG)) {
        flags |= c_MissingResonance;
      } else if (generatedPDG == 22) { //missing photon
        if (!(flags & c_MissFSR) or !(flags & c_MissGamma)) {
          if (isFSR(genPart))
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
