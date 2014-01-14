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
  const MCParticle* mcParticle = DataStore::getRelated<MCParticle>(particle);
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
    const MCParticle*  daugMCP = DataStore::getRelated<MCParticle>(daugP);

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
    const MCParticle*  daugMCP = DataStore::getRelated<MCParticle>(daugP);

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

  bool result = DataStore::addRelationFromTo(particle, mcMatch);

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

  if (recFSPs.size() > recFSPs.size())
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


// number and type of final state particles should be the same
// Flags
// -11 : gen_hepevt link of one of children not found in list of daughters of matched gen_hepevt (something wrong)
// -10 : one of children has not link to gen_hepevt (something wrong)
//  -9 : as -10 but in addition size of reconstructed < size of generated
//  -5 : more reconstructed than generated particle in final state (something wrong)
//  -2 : gen_hepvt particle doesn't have any daughters (something wrong in maching)
//  -1 : random combination (common mother is virtual gamma (ccbar, uds mc), or Upsilon(4S), Upsilon(5S))
//   0 : no link to gen_hepevt particle
//   1 : particle is correctly reconstructed; including correct particle id of final state particles (SIGNAL)
//   2 : one or more FSP are misidentified, but have common mother
//   3 : FSP have common mother, but at least one massive particle is missing
//   4 : FSP have common mother, but at least one massless particle is missing (radiative photon)
//   5 : final state includes pi0 without perfect match to gen_hepevt
//   6 : ID = 2 and 5 are true
//  10 : particle is correctly reconstructed; including correct particle id of final state particles, but FSR photon is missing (SIGNAL)
//  11 : in addition to FSR photon missing one more radiative photon is missing
//  20 : missing neutrino
//  21 : missing neutrino and radiative photon
//  23 : missing neutrino and massive particle
//  24 : missing neutrino and another massles particle (FSR photon)
int getMCTruthFlag(const Particle* particle, const MCParticle* mcParticle)
{
  if (!mcParticle)
    return 0;

  unsigned nChildren = particle->getNDaughters();

  if (nChildren == 0) {
    if (particle->getPDGCode() == mcParticle->getPDG()) {
      return 1;
    } else {
      return 2;
    }
  }

  int genMotherPDG = mcParticle->getPDG();
  // TODO: fix this (aim for no hard coded values)
  if (genMotherPDG == 10022 || genMotherPDG == 300553 || genMotherPDG == 9000553)
    return -1;

  // Ks doesn't have daughters in gen_hepevt table
  // TODO: is there any better way
  if (genMotherPDG == 310)
    return 1;

  // fill vectors of reconstructed and generated final state particles
  vector<const Particle*>   recFSPs;
  vector<const MCParticle*> genFSPs;

  appendFSP(particle,   recFSPs);
  appendFSP(mcParticle, genFSPs);

  vector<int> missingParticles;
  findMissingGeneratedParticles(recFSPs, genFSPs, missingParticles);

  // TODO: do something
  //if (genFSPs.size() == 0)
  //  return -2;

  int truth = compareFinalStates(recFSPs, genFSPs);

  return truth;
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

int compareFinalStates(vector<const Particle*> reconstructed, vector<const MCParticle*> generated)
{

  if (reconstructed.size() == generated.size()) {
    int missID  = 0;
    int missPi0 = 0;

    for (int i = 0; i < (int)reconstructed.size(); ++i) {
      const MCParticle* mcParticle = DataStore::getRelated<MCParticle>(reconstructed[i]);

      if (mcParticle) {
        int link = 0;

        for (int j = 0; j < (int)generated.size(); ++j) {
          if (mcParticle->getIndex() == generated[j]->getIndex()) {
            link = 1;

            if (reconstructed[i]->getPDGCode() != generated[j]->getPDG())
              missID++;
            //if(reconstructed[i]->getPDGCode()==111 && getMCtruthPi0Flag(reconstructed[i])!=1)
            //missPi0++;

            break;
          }
        }
        if (!link)
          return -11;
      } else
        return -10;
    }
    if (missID && missPi0)
      return 6;
    else if (missID)
      return 2;
    else if (missPi0)
      return 5;

    return 1;
  } else if (reconstructed.size() < generated.size()) { // missing particle
    int missing = 0;
    std::vector<int> missP;

    for (int i = 0; i < (int)generated.size(); ++i) {
      int link = 0;

      for (int j = 0; j < (int)reconstructed.size(); ++j) {
        const MCParticle* mcParticle = DataStore::getRelated<MCParticle>(reconstructed[j]);
        if (mcParticle) {
          if (mcParticle->getIndex() == generated[i]->getIndex()) {
            link = 1;
            break;
          }
        } else
          return -9;
      }
      if (!link) {
        missing++;
        missP.push_back(i);
      }
    }
    if (missing) {
      int masslessOnly = 1;
      int missNu = 0;
      int missGfsr = 0;
      int missGrad = 0;

      for (int i = 0; i < (int)missP.size(); ++i) {
        if (abs(generated[missP[i]]->getPDG()) == 12 || abs(generated[missP[i]]->getPDG()) == 14 || abs(generated[missP[i]]->getPDG()) == 16)
          missNu = 1;
        else if (generated[missP[i]]->getPDG() == 22) {
          if (generated[missP[i]]->getMother()) {
            int ndaug = (generated[missP[i]]->getMother()->getLastDaughter() - generated[missP[i]]->getMother()->getFirstDaughter()) + 1;
            if (ndaug == 2)
              missGrad = 1;
            else
              missGfsr = 1;
          } else
            missGfsr = 1;
        } else
          masslessOnly = 0;
      }
      if (missNu) {
        if (missGrad && !masslessOnly)
          return 21;
        else if (missGrad && masslessOnly)
          return 24;
        else if (!missGrad && !masslessOnly)
          return 23;
        else
          return 20;
      } else {
        if (missGfsr && !missGrad && masslessOnly)
          return 10;
        else if (missGrad && !masslessOnly)
          return 11;
        else if (missGrad && masslessOnly)
          return 4;
        else if (!missGrad && !masslessOnly)
          return 3;
        else
          return -20;
      }
    } else
      return -8;
  } else
    return -5;
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
      const MCParticle* mcParticle = DataStore::getRelated<MCParticle>(reconstructed[j]);

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
    const MCParticle* mcParticle = DataStore::getRelated<MCParticle>(reconstructed[i]);

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


double mcTruthFlag(const Particle* part)
{
  return getMCTruthFlag(part, part->getRelated<MCParticle>());
}

REGISTER_VARIABLE("mcTruthFlag", mcTruthFlag, "Monte Carlo truth flag, different values are described in getMCTruthFlag() documentation");
