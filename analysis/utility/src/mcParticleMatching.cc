// ******************************************************************
// MC Matching
// author: A. Zupanc (anze.zupanc@ijs.si)
// ******************************************************************

#include <analysis/utility/mcParticleMatching.h>
#include <analysis/utility/MCMatchStatus.h>

#include <analysis/utility/VariableManager.h>

// DataStore related
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

// dataobjects
#include <mdst/dataobjects/MCParticle.h>

void fillGenMothers(const MCParticle* mcP, vector<int>& genMCPMothers)
{
  if (!mcP)
    return;

  genMCPMothers.push_back(mcP->getIndex());
  while (mcP->getMother()) {
    mcP = mcP->getMother();
    genMCPMothers.push_back(mcP->getIndex());
  }
}


int findCommonMother(unsigned nChildren, vector<int> firstMothers, vector<int> otherMothers)
{
  if (firstMothers.size() == 0 || otherMothers.size() == 0)
    return -1;

  for (unsigned i = 0; i < firstMothers.size(); ++i) {
    unsigned counter = 0;

    for (unsigned j = 0; j < otherMothers.size(); ++j)
      if (firstMothers[i] == otherMothers[j])
        counter++;

    if (counter == nChildren - 1)
      return firstMothers[i];
  }

  return -1;
}


bool setMCTruth(const Particle* particle)
{

  // if Particle<->MCParticle relation already exists, there is nothing new to do
  const MCParticle* mcParticle = particle->getRelatedTo<MCParticle>();
  if (mcParticle) {
    return true;
  }

  int nChildren = particle->getNDaughters();
  if (nChildren < 2) {
    // TODO: deal with this
    return false;
  }

  // check, if for all daughter particles Particle <-> MCParticle relation exists
  for (int i = 0; i < nChildren; ++i) {
    const Particle*    daugP   = particle->getDaughter(i);
    const MCParticle*  daugMCP = daugP->getRelatedTo<MCParticle>();

    if (!daugMCP) {
      // Particle <-> MCParticle relation does not exist for this daughter; -> Set it!
      bool daugMCTruth = setMCTruth(daugP);
      if (!daugMCTruth)
        return false;
    }
  }

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
  int motherIndex = findCommonMother(nChildren, firstDaugMothers, otherDaugMothers);

  // if index is less than 1, the common mother particle was not found
  // remember: it's 1-based index
  if (motherIndex < 1)
    return false;

  // finaly the relation can be set
  StoreArray<MCParticle> mcParticles;

  // sanity check
  if (motherIndex > mcParticles.getEntries())
    return false;

  MCParticle* mcMatch = mcParticles[motherIndex - 1];

  bool result = particle->addRelationTo(mcMatch);

  return result;
}

int getMCTruthStatus(const Particle* particle, const MCParticle* mcParticle)
{
  int status = 0;

  if (!mcParticle)
    return -2;

  unsigned nChildren = particle->getNDaughters();

  if (nChildren == 0) {
    if (particle->getPDGCode() != mcParticle->getPDG())
      status |= MCMatchStatus::c_MisID;
    return status;
  }

  int genMotherPDG = mcParticle->getPDG();
  // TODO: fix this (aim for no hard coded values)
  if (genMotherPDG == 10022 || genMotherPDG == 300553 || genMotherPDG == 9000553)
    return -1;

  // Ks doesn't have daughters in gen_hepevt table
  // TODO: is there any better way
  //if (genMotherPDG == 310)
  //return 1;

  // fill vectors of reconstructed and generated final state particles
  vector<const Particle*>   recFSPs;
  vector<const MCParticle*> genFSPs;

  appendFSP(particle,   recFSPs);
  appendFSP(mcParticle, genFSPs);

  vector<int> missingParticles;
  findMissingGeneratedParticles(recFSPs, genFSPs, missingParticles);

  // TODO: do something
  if (genFSPs.size() == 0)
    return -2;

  if (recFSPs.size() > genFSPs.size())
    return -3;

  // determine the status bits
  bool missFSR            = false;
  bool misID              = false;
  bool missGamma          = false;
  bool missMasiveParticle = false;
  bool missKlong          = false;
  bool missNeutrino       = false;

  if (missingParticles.size()) {
    missFSR      = missingFSRPhoton(genFSPs, missingParticles);
    missGamma    = missingRadiativePhoton(genFSPs, missingParticles);
    missNeutrino = missingNeutrino(genFSPs, missingParticles);
    missMasiveParticle = missingMassiveParticle(genFSPs, missingParticles);
    missKlong    = missingKlong(genFSPs, missingParticles);
  }

  misID = isMissidentified(recFSPs, genFSPs);

  if (missFSR)
    status |= MCMatchStatus::c_MissFSR;
  if (misID)
    status |= MCMatchStatus::c_MisID;
  if (missGamma)
    status |= MCMatchStatus::c_MissGamma;
  if (missMasiveParticle)
    status |= MCMatchStatus::c_MissMassiveParticle;
  if (missNeutrino)
    status |= MCMatchStatus::c_MissNeutrino;
  if (missKlong)
    status |= MCMatchStatus::c_MissKlong;

  return status;
}


void appendFSP(const Particle* p, vector<const Particle*>& children)
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

void appendFSP(const MCParticle* gen, vector<const MCParticle*>& children)
{
  const vector<Belle2::MCParticle*> genDaughters = gen->getDaughters();

  for (unsigned i = 0; i < genDaughters.size(); ++i) {
    const MCParticle* daug = genDaughters[i];

    int nDaughs = (daug->getLastDaughter() - daug->getFirstDaughter()) + 1;
    if (nDaughs && !isFSP(daug)) {
      appendFSP(daug, children);
    } else {
      children.push_back(daug);
    }
  }
}

int isFSP(const MCParticle* P)
{
  switch (abs(P->getPDG())) {
    case 211:
      return 1;
    case 321:
      return 1;
    case 11:
      return 1;
    case 12:
      return 1;
    case 13:
      return 1;
    case 14:
      return 1;
    case 16:
      return 1;
    case 22:
      return 1;
    case 2212:
      return 1;
    case 111:
      return 1;
    case 310:
      return 1;
    case 130:
      return 1;
    case 2112:
      return 1;
    default:
      return 0;
  }
}

void findMissingGeneratedParticles(vector<const Particle*>   reconstructed,
                                   vector<const MCParticle*> generated,
                                   std::vector<int>& missP)
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
      missP.push_back(i);
  }
}

bool missingFSRPhoton(vector<const MCParticle*> generated, std::vector<int> missP)
{
  bool status = false;

  for (int i = 0; i < (int)missP.size(); ++i) {
    if (generated[missP[i]]->getPDG() == 22) {
      if (generated[missP[i]]->getMother()) {
        int ndaug = (generated[missP[i]]->getMother()->getLastDaughter() - generated[missP[i]]->getMother()->getFirstDaughter()) + 1;
        if (ndaug > 2) {
          status = true;
          break;
        }
      }
    }
  }
  return status;
}

bool missingRadiativePhoton(vector<const MCParticle*> generated, std::vector<int> missP)
{
  bool status = false;

  for (int i = 0; i < (int)missP.size(); ++i) {
    if (generated[missP[i]]->getPDG() == 22) {
      if (generated[missP[i]]->getMother()) {
        int ndaug = (generated[missP[i]]->getMother()->getLastDaughter() - generated[missP[i]]->getMother()->getFirstDaughter()) + 1;
        if (ndaug == 2) {
          status = true;
          break;
        }
      }
    }
  }
  return status;
}

bool missingNeutrino(vector<const MCParticle*> generated, std::vector<int> missP)
{
  bool status = false;

  for (int i = 0; i < (int)missP.size(); ++i) {
    // TODO: avoid hard coded values
    if (abs(generated[missP[i]]->getPDG()) == 12 || abs(generated[missP[i]]->getPDG()) == 14 || abs(generated[missP[i]]->getPDG()) == 16) {
      status = true;
      break;
    }
  }
  return status;
}

bool missingMassiveParticle(vector<const MCParticle*> generated, std::vector<int> missP)
{
  bool status = false;

  for (int i = 0; i < (int)missP.size(); ++i) {
    // TODO: avoid hard coded values
    if (abs(generated[missP[i]]->getPDG()) != 12
        && abs(generated[missP[i]]->getPDG()) != 14
        && abs(generated[missP[i]]->getPDG()) != 16
        && generated[missP[i]]->getPDG() != 22) {
      status = true;
      break;
    }
  }
  return status;
}

bool missingKlong(vector<const MCParticle*> generated, std::vector<int> missP)
{
  bool status = false;

  for (int i = 0; i < (int)missP.size(); ++i) {
    // TODO: avoid hard coded values
    if (abs(generated[missP[i]]->getPDG()) == 130) {
      status = true;
      break;
    }
  }
  return status;
}

bool isMissidentified(vector<const Particle*> reconstructed, vector<const MCParticle*> generated)
{
  bool status = false;

  for (int i = 0; i < (int)reconstructed.size(); ++i) {
    const MCParticle* mcParticle = reconstructed[i]->getRelatedTo<MCParticle>();

    for (int j = 0; j < (int)generated.size(); ++j) {
      if (mcParticle->getIndex() == generated[j]->getIndex()) {
        if (reconstructed[i]->getPDGCode() != generated[j]->getPDG()) {
          status = true;
          break;
        }
      }
    }
  }
  return status;
}


