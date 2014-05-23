// ******************************************************************
// MC Matching
// authors: A. Zupanc (anze.zupanc@ijs.si), C. Pulvermacher (christian.pulvermacher@kit.edu)
// ******************************************************************

#include <analysis/utility/mcParticleMatching.h>
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/StoreArray.h>

using namespace Belle2;
using namespace std;

void MCMatching::fillGenMothers(const MCParticle* mcP, vector<int>& genMCPMothers)
{
  while (mcP) {
    genMCPMothers.push_back(mcP->getIndex());
    mcP = mcP->getMother();
  }
}


int MCMatching::findCommonMother(unsigned nChildren, const vector<int>& firstMothers, const vector<int>& otherMothers)
{
  if (firstMothers.empty() || otherMothers.empty())
    return -1;

  for (int commonMotherCandidate : firstMothers) {
    unsigned counter = 0;

    //if commonMotherCandidate is a common mother, it should occur (n-1) times in otherMothers
    for (unsigned j = 0; j < otherMothers.size(); ++j)
      if (commonMotherCandidate == otherMothers[j])
        counter++;

    if (counter == nChildren - 1)
      return commonMotherCandidate;
  }

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

  // check, if for all daughter particles Particle <-> MCParticle relation exists
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
    // 1st fill two vertices with indices of mother particles of the first daughters (1st vector)
    // and with indices of mother particles of all other daughters (2nd vector)

    vector<int> firstDaugMothers; // indices of generated mothers of first daughter
    vector<int> otherDaugMothers; // indices of generated mothers of all other daughter

    for (int i = 0; i < nChildren; ++i) {
      const Particle*    daugP   = particle->getDaughter(i);
      const MCParticle*  daugMCP = daugP->getRelatedTo<MCParticle>();

      if (i == 0)
        fillGenMothers(daugMCP, firstDaugMothers);
      else
        fillGenMothers(daugMCP, otherDaugMothers);
    }

    // find first generated common mother of all linked daughter MCParticles
    motherIndex = findCommonMother(nChildren, firstDaugMothers, otherDaugMothers);
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

  if (!mcParticle) {
    mcParticle = particle->getRelatedTo<MCParticle>();
    if (mcParticle) {
      B2ERROR("setMCTruthStatus: got mcparticle=NULL, but found one via relations...");
    }
    return setStatus(particle, status | MCMatchStatus::c_InternalError);
  }

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
  if (particle->getPDGCode() != mcParticle->getPDG())
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
  //if (genMotherPDG == 10022 || genMotherPDG == 300553 || genMotherPDG == 9000553)
  //  return -1;

  // Ks doesn't have daughters in gen_hepevt table
  // TODO: is there any better way
  //if (genMotherPDG == 310)
  //return 1;

  // fill vectors of reconstructed and generated final state particles
  vector<const Particle*>   recFSPs;
  vector<const MCParticle*> genFSPs;

  appendFSP(particle,   recFSPs);
  appendFSP(mcParticle, genFSPs);

  vector<const MCParticle*> missingParticles;
  findMissingGeneratedParticles(recFSPs, genFSPs, missingParticles);

  // TODO: do something
  if (genFSPs.empty())
    return setStatus(particle, status | MCMatchStatus::c_InternalError);

  //This might happen with  e.g two ECLClusters from a charged track
  if (recFSPs.size() > genFSPs.size())
    return setStatus(particle, status | MCMatchStatus::c_InternalError);

  // determine the status bits
  bool missFSR            = false;
  bool missGamma          = false;
  bool missMasiveParticle = false;
  bool missKlong          = false;
  bool missNeutrino       = false;

  if (!missingParticles.empty()) {
    missFSR      = missingFSRPhoton(missingParticles);
    missGamma    = missingRadiativePhoton(missingParticles);
    missNeutrino = missingNeutrino(missingParticles);
    missMasiveParticle = missingMassiveParticle(missingParticles);
    missKlong    = missingKlong(missingParticles);
  }

  if (missFSR)
    status |= MCMatchStatus::c_MissFSR;
  if (missGamma)
    status |= MCMatchStatus::c_MissGamma;
  if (missMasiveParticle)
    status |= MCMatchStatus::c_MissMassiveParticle;
  if (missNeutrino)
    status |= MCMatchStatus::c_MissNeutrino;
  if (missKlong)
    status |= MCMatchStatus::c_MissKlong;

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


void MCMatching::appendFSP(const Particle* p, vector<const Particle*>& children)
{
  for (unsigned i = 0; i < p->getNDaughters(); ++i) {
    const Particle* daug = p->getDaughter(i);

    // TODO: fix this (aim for no hard coded values)
    if (daug->getNDaughters() && daug->getPDGCode() != 111 && daug->getPDGCode() != 310) {
      appendFSP(daug, children);
    } else {
      children.push_back(daug);
    }
  }
}

void MCMatching::appendFSP(const MCParticle* gen, vector<const MCParticle*>& children)
{
  const vector<MCParticle*>& genDaughters = gen->getDaughters();

  for (unsigned i = 0; i < genDaughters.size(); ++i) {
    const MCParticle* daug = genDaughters[i];

    int nDaughs = daug->getNDaughters();
    if (nDaughs && !isFSP(daug)) {
      appendFSP(daug, children);
    } else {
      children.push_back(daug);
    }
  }
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
    case 111:
    case 310:
    case 130:
    case 2112:
      return true;
    default:
      return false;
  }
}

void MCMatching::findMissingGeneratedParticles(vector<const Particle*>   reconstructed,
                                               vector<const MCParticle*> generated,
                                               std::vector<const MCParticle*>& missP)
{
  if (reconstructed.size() >= generated.size())
    return;

  for (int i = 0; i < (int)generated.size(); ++i) {
    int link = 0;

    for (int j = 0; j < (int)reconstructed.size(); ++j) {
      const MCParticle* mcParticle = reconstructed[j]->getRelatedTo<MCParticle>();

      if (mcParticle)
        if (mcParticle->getIndex() == generated[i]->getIndex()) {
          link = 1;
          break;
        }
    }

    if (!link)
      missP.push_back(generated[i]);
  }
}

bool MCMatching::missingFSRPhoton(std::vector<const MCParticle*> missP)
{
  for (const MCParticle * generated : missP) {
    if (generated->getPDG() == 22) {
      if (generated->getMother()) {
        int ndaug = generated->getMother()->getNDaughters();
        if (ndaug > 2) {
          return true;
        }
      }
    }
  }
  return false;
}

bool MCMatching::missingRadiativePhoton(std::vector<const MCParticle*> missP)
{
  for (const MCParticle * generated : missP) {
    if (generated->getPDG() == 22) {
      if (generated->getMother()) {
        int ndaug = generated->getMother()->getNDaughters();
        if (ndaug == 2) {
          return true;
        }
      }
    }
  }
  return false;
}

bool MCMatching::missingNeutrino(std::vector<const MCParticle*> missP)
{
  for (const MCParticle * generated : missP) {
    // TODO: avoid hard coded values
    if (abs(generated->getPDG()) == 12 || abs(generated->getPDG()) == 14 || abs(generated->getPDG()) == 16) {
      return true;
    }
  }
  return false;
}

bool MCMatching::missingMassiveParticle(std::vector<const MCParticle*> missP)
{
  for (const MCParticle * generated : missP) {
    // TODO: avoid hard coded values
    if (abs(generated->getPDG()) != 12
        && abs(generated->getPDG()) != 14
        && abs(generated->getPDG()) != 16
        && generated->getPDG() != 22) {
      return true;
    }
  }
  return false;
}

bool MCMatching::missingKlong(std::vector<const MCParticle*> missP)
{
  for (const MCParticle * generated : missP) {
    // TODO: avoid hard coded values
    if (abs(generated->getPDG()) == 130) {
      return true;
    }
  }
  return false;
}

