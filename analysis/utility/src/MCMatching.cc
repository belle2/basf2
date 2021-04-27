/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * Copyright(C) 2013-2018 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Christian Pulvermacher, Yo Sato,            *
 *               Alejandro Mora                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/utility/MCMatching.h>
#include <analysis/utility/AnalysisConfiguration.h>

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <unordered_set>

using namespace Belle2;
using namespace std;

const std::string MCMatching::c_extraInfoMCErrors = "mcErrors";

std::string MCMatching::explainFlags(unsigned int flags)
{
  if (flags == c_Correct)
    return "c_Correct";

  std::string s;
  unsigned int f = 1;
  while (flags != 0) {
    if (flags & f) {
      switch (f) {
        case c_MissFSR               : s += "c_MissFSR"; break;
        case c_MissingResonance      : s += "c_MissingResonance"; break;
        case c_DecayInFlight         : s += "c_DecayInFlight"; break;
        case c_MissNeutrino          : s += "c_MissNeutrino"; break;
        case c_MissGamma             : s += "c_MissGamma"; break;
        case c_MissMassiveParticle   : s += "c_MissMassiveParticle"; break;
        case c_MissKlong             : s += "c_MissKlong"; break;
        case c_MisID                 : s += "c_MisID"; break;
        case c_AddedWrongParticle    : s += "c_AddedWrongParticle"; break;
        case c_InternalError         : s += "c_InternalError"; break;
        case c_MissPHOTOS            : s += "c_MissPHOTOS"; break;
        case c_AddedRecoBremsPhoton  : s += "c_AddedRecoBremsPhoton"; break;
        default:
          s += to_string(f);
          B2ERROR("MCMatching::explainFlags() doesn't know about flag " << f << ", please update it.");
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
  //if extra-info is set, we already handled this particle
  //TODO check wether this actually speeds things up or not
  if (particle->hasExtraInfo(c_extraInfoMCErrors))
    return true;

  if (particle->getRelatedTo<MCParticle>()) {
    //nothing to do
    return true;
  }

  int nChildren = particle->getNDaughters();
  if (nChildren == 0) {
    //no daughters -> should be an FSP, but no related MCParticle. Probably background.
    return false;
  }

  // check, if for all daughter particles Particle -> MCParticle relation exists
  bool daugMCTruth = true;
  for (int i = 0; i < nChildren; ++i) {
    const Particle* daugP = particle->getDaughter(i);
    // call setMCTruth for all daughters
    daugMCTruth &= setMCTruth(daugP);
  }
  if (!daugMCTruth)
    return false;

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


//utility functions used by setMCErrorsExtraInfo() getMissingParticleFlags()
namespace {
  /** Recursively gather all matched MCParticles in daughters of p (taking special care of decay-in-flight things). */
  void appendParticles(const Particle* p, unordered_set<const MCParticle*>& mcMatchedParticles)
  {
    for (unsigned i = 0; i < p->getNDaughters(); ++i) {
      const Particle* daug = p->getDaughter(i);

      //add matched MCParticle for 'daug'
      const MCParticle* mcParticle = daug->getRelatedTo<MCParticle>();
      if (mcParticle)
        mcMatchedParticles.insert(mcParticle);

      if (daug->getNDaughters() != 0) {
        // if daug has daughters, call appendParticles recursively.
        appendParticles(daug, mcMatchedParticles);
        continue;
      }

      if (daug->hasExtraInfo(MCMatching::c_extraInfoMCErrors)) {
        if (static_cast<unsigned int>(daug->getExtraInfo(MCMatching::c_extraInfoMCErrors)) & MCMatching::c_DecayInFlight) {
          //now daug does not have any daughters.
          //particle at the bottom of reconstructed decay tree, reconstructed from an MCParticle that is actually slightly deeper than we want,
          //so we'll also add all mother MCParticles until the first primary mother
          mcParticle = mcParticle->getMother();
          while (mcParticle) {
            mcMatchedParticles.insert(mcParticle);
            if (mcParticle->hasStatus(MCParticle::c_PrimaryParticle))
              break;

            mcParticle = mcParticle->getMother();
          }
        }
      }

    }
  }

  /** Recursively gather all daughters of 'gen' we want to reconstruct. */
  void appendParticles(const MCParticle* gen, vector<const MCParticle*>& children)
  {
    if (MCMatching::isFSP(gen->getPDG()))
      return; //stop at the bottom of the MC decay tree (ignore secondaries)

    const vector<MCParticle*>& genDaughters = gen->getDaughters();
    for (auto daug : genDaughters) {
      children.push_back(daug);
      appendParticles(daug, children);
    }
  }

  // Check if mcDaug is accepted to be missed by the property of part.
  bool isDaughterAccepted(const MCParticle* mcDaug, const Particle* part)
  {
    const int property = part->getProperty();

    const int absPDG = abs(mcDaug->getPDG());

    // if mcDaug is not FSP, check c_IsIgnoreIntermediate property
    if (!MCMatching::isFSP(absPDG)) {
      if (property & Particle::PropertyFlags::c_IsIgnoreIntermediate)
        return true;
    } else if (absPDG == Const::photon.getPDGCode()) { // gamma
      if (MCMatching::isFSR(mcDaug) or mcDaug->hasStatus(MCParticle::c_IsPHOTOSPhoton)
          or (AnalysisConfiguration::instance()->useLegacyMCMatching() and MCMatching::isFSRLegacy(mcDaug))) {
        if (property & Particle::PropertyFlags::c_IsIgnoreRadiatedPhotons)
          return true;
      } else {
        if (property & Particle::PropertyFlags::c_IsIgnoreGamma)
          return true;
      }
    } else if (absPDG == 12 or absPDG == 14 or absPDG == 16) { // neutrino
      if (property & Particle::PropertyFlags::c_IsIgnoreNeutrino)
        return true;
    } else { // otherwise, massive FSP
      if (property & Particle::PropertyFlags::c_IsIgnoreMassive)
        return true;
    }

    return false;
  }

  /** Recursively gather all daughters which are accepted to be missed. */
  void appendAcceptedMissingDaughters(const Particle* p, unordered_set<const MCParticle*>& acceptedParticles)
  {
    const MCParticle* mcParticle = p->getRelatedTo<MCParticle>();
    if (mcParticle) {
      vector<const MCParticle*> genDaughters;
      appendParticles(mcParticle, genDaughters);

      for (auto mcDaug : genDaughters) {
        if (isDaughterAccepted(mcDaug, p))
          acceptedParticles.insert(mcDaug);
        else
          acceptedParticles.erase(mcDaug);
      }
    }

    for (unsigned i = 0; i < p->getNDaughters(); ++i) {
      const Particle* daug = p->getDaughter(i);
      appendAcceptedMissingDaughters(daug, acceptedParticles);
    }

  }


}

int MCMatching::getMCErrors(const Particle* particle, const MCParticle* mcParticle)
{
  if (particle->hasExtraInfo(c_extraInfoMCErrors)) {
    return particle->getExtraInfo(c_extraInfoMCErrors);
  } else {
    if (!mcParticle)
      mcParticle = particle->getRelatedTo<MCParticle>();
    return setMCErrorsExtraInfo(const_cast<Particle*>(particle), mcParticle);
  }
}

int MCMatching::setMCErrorsExtraInfo(Particle* particle, const MCParticle* mcParticle)
{
  auto setStatus = [](Particle * part, int s) -> int {
    part->addExtraInfo(c_extraInfoMCErrors, s);
    return s;
  };

  if (!mcParticle)
    return setStatus(particle, MCErrorFlags::c_InternalError);

  if (particle->getNDaughters() == 0) { //FSP
    //other checks concern daughters of particle, so we're done here
    return setStatus(particle, getFlagsOfFSP(particle, mcParticle));;
  }

  int status = 0;

  // Check if particle (non FSP) has different PDG code than mcParticle
  const Particle::EFlavorType flavorType = particle->getFlavorType();
  if ((flavorType == Particle::c_Flavored and particle->getPDGCode() != mcParticle->getPDG())
      or (flavorType == Particle::c_Unflavored and abs(particle->getPDGCode()) != abs(mcParticle->getPDG()))) {
    auto mother = mcParticle->getMother();

    // Check if mother particle has the correct pdg code, if so we have to take care of the special case
    // tau -> rho nu, where a the matched mother is  the rho, but we have only a missing resonance and not added a wrong particle.
    if (mother and particle->getPDGCode() == mother->getPDG() and getNumberOfDaughtersWithoutNeutrinos(mother) == 1
        and !particle->hasExtraInfo("bremsCorrected")) {
      if (abs(mother->getPDG()) != 15 and abs(mcParticle->getPDG()) != 15) {
        B2WARNING("Special treatment in MCMatching for tau is called for a non-tau particle. Check if you discovered another special case here, or if we have a bug! "
                  << mother->getPDG() << " " << particle->getPDGCode() << " " << mcParticle->getPDG());
      }
      // if particle has c_IsIgnoreIntermediate flag, c_MissingResonance will not be added.
      if (not(particle->getProperty() & Particle::PropertyFlags::c_IsIgnoreIntermediate))
        status |= MCErrorFlags::c_MissingResonance;
    } else if (!(particle->getProperty() & Particle::PropertyFlags::c_IsUnspecified)) {
      // Check if the particle is unspecified. If so the flag of c_AddedWrongParticle will be ignored.
      status |= MCErrorFlags::c_AddedWrongParticle;
    }
  }

  status |= getFlagsOfDaughters(particle, mcParticle);

  status |= getMissingParticleFlags(particle, mcParticle);

  // Mask the flags ignored by PropertyFlags of the particle
  status &= ~(getFlagsIgnoredByProperty(particle));

  return setStatus(particle, status);
}

int MCMatching::getFlagsOfFSP(const Particle* particle, const MCParticle* mcParticle)
{
  if (particle->getPDGCode() == mcParticle->getPDG())
    return MCErrorFlags::c_Correct;

  // if PDG of particle is different from that of mcParticle
  if (mcParticle->hasStatus(MCParticle::c_PrimaryParticle)) {
    // if particle is primary, add the c_MisID flag.
    return MCErrorFlags::c_MisID;
  } else {
    // secondary particle, so the original particle probably decayed
    int status = MCErrorFlags::c_DecayInFlight;

    //find first primary mother
    const MCParticle* primary = mcParticle->getMother();
    while (primary and !primary->hasStatus(MCParticle::c_PrimaryParticle))
      primary = primary->getMother();

    if (!primary) {
      status |= MCErrorFlags::c_InternalError;
    } else if (particle->getPDGCode() != primary->getPDG()) {
      //if primary particle also has wrong PDG code, we're actually MisIDed
      status |= MCErrorFlags::c_MisID;
    }
    return status;
  }
}

int MCMatching::getFlagsOfDaughters(const Particle* particle, const MCParticle* mcParticle)
{
  unsigned nChildren = particle->getNDaughters();

  //Vector to store all the MC (n*grand-)daughters of the mother of the bremsstrahlung corrected particle
  vector<const MCParticle*> genParts;
  //Fill it only in the case we have a particle that has been brems corrected
  if (particle->hasExtraInfo("bremsCorrected") && nChildren > 1) {
    if (mcParticle && mcParticle->getMother())
      appendParticles(mcParticle->getMother(), genParts);
  }

  int daughterStatuses = 0;
  vector<int> daughterProperties = particle->getDaughterProperties();
  for (unsigned i = 0; i < nChildren; ++i) {
    const Particle* daughter = particle->getDaughter(i);
    int daughterStatus = 0;
    if (particle->hasExtraInfo("bremsCorrected") && daughter->getPDGCode() == Const::photon.getPDGCode()) {
      // if the daughter is a brems photon, start the special treatment
      daughterStatus |= getFlagsOfBremsPhotonDaughter(daughter, mcParticle, genParts);
    } else {
      daughterStatus |= getMCErrors(daughter);
    }

    int daughterStatusAcceptMask = (~c_Correct);
    if (i < daughterProperties.size()) // sanity check. daughterProperties should be larger than i.
      daughterStatusAcceptMask = makeDaughterAcceptMask(daughterProperties[i]);

    daughterStatuses |= (daughterStatus & daughterStatusAcceptMask);
  }

  //add up all (accepted) status flags we collected for our daughters
  const int daughterStatusesAcceptMask = c_MisID | c_AddedWrongParticle | c_DecayInFlight | c_InternalError | c_AddedRecoBremsPhoton;
  return (daughterStatuses & daughterStatusesAcceptMask);

}


int MCMatching::getFlagsOfBremsPhotonDaughter(const Particle* daughter, const MCParticle* mcParticle,
                                              const vector<const MCParticle*>& genParts)
{
  //At first, call getMCErrors as usual
  int daughterStatus = getMCErrors(daughter);

  //Check if the daugther has an MC particle related
  const MCParticle* mcDaughter = daughter->getRelatedTo<MCParticle>();
  //If it hasn't, add the c_BremsPhotonAdded flag to the mother and stop the propagation of c_InternalError
  if (!mcDaughter) {
    daughterStatus &= (~c_InternalError);
    daughterStatus |= c_AddedRecoBremsPhoton;
  }
  //If it has, check if MCParticle of the daughter is same as the mother. If so, we'll stop the propagation of c_MisID
  else if (mcDaughter == mcParticle) {
    daughterStatus &= (~c_MisID);
  }
  //If it has, check if the MC particle is (n*grand)-daughter of the particle mother. If it isn't, we'll add the error flag
  else if (std::find(genParts.begin(), genParts.end(), mcDaughter) == genParts.end()) {
    daughterStatus |= c_AddedRecoBremsPhoton;
  }
  //else nothing to do

  return daughterStatus;
}

int MCMatching::getNumberOfDaughtersWithoutNeutrinos(const MCParticle* mcParticle)
{
  auto daughters = mcParticle->getDaughters();
  unsigned int number_of_neutrinos = 0;
  for (auto& p : daughters) {
    auto pdg = abs(p->getPDG());
    if (pdg == 12 || pdg == 14 || pdg == 16) {
      number_of_neutrinos++;
    }
  }
  return daughters.size() - number_of_neutrinos;
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
  return p->hasStatus(MCParticle::c_IsFSRPhoton);
}

bool MCMatching::isFSRLegacy(const MCParticle* p)
{
  // In older versions of MC MCParticles don't have c_IsFSRPhoton, c_IsISRPhoton or c_ISPHOTOSPhoton bits
  // properly set. Instead this approximation is used to check if given photon is radiative (physics) photon
  // or produced by PHOTOS (FSR).

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


bool MCMatching::isRadiativePhoton(const MCParticle* p)
{
  // Check if any of the bits c_IsFSRPhoton, c_IsISRPhoton or c_ISPHOTOSPhoton is set
  return p->getStatus(MCParticle::c_IsRadiativePhoton) != 0;
}


int MCMatching::getMissingParticleFlags(const Particle* particle, const MCParticle* mcParticle)
{
  int flags = 0;

  vector<const MCParticle*> genParts;
  appendParticles(mcParticle, genParts);

  unordered_set<const MCParticle*> mcMatchedParticles;
  appendParticles(particle, mcMatchedParticles);
  unordered_set<const MCParticle*> acceptedParticles;
  appendAcceptedMissingDaughters(particle, acceptedParticles);
  for (auto part : acceptedParticles) {
    mcMatchedParticles.insert(part);
  }


  for (const MCParticle* genPart : genParts) {

    // if genPart exists in mcMatchedParticles, continue.
    if (mcMatchedParticles.find(genPart) != mcMatchedParticles.end())
      continue;

    //we want to set a flag, so what kind of particle is genPart?
    const int generatedPDG = genPart->getPDG();
    const int absGeneratedPDG = abs(generatedPDG);

    if (!isFSP(generatedPDG)) {
      flags |= c_MissingResonance;
    } else if (generatedPDG == Const::photon.getPDGCode()) { //missing photon
      if (AnalysisConfiguration::instance()->useLegacyMCMatching()) {
        if (flags & (c_MissFSR | c_MissGamma)) {
          // if flags already have c_MissFSR or c_MissGamm, do nothing.
        } else {
          if (isFSRLegacy(genPart)) {
            flags |= c_MissFSR;
            flags |= c_MissPHOTOS;
          } else {
            flags |= c_MissGamma;
          }
        }
      } else {
        if (isFSR(genPart))
          flags |= c_MissFSR;
        else if (genPart->hasStatus(MCParticle::c_IsPHOTOSPhoton))
          flags |= c_MissPHOTOS;
        else
          flags |= c_MissGamma;
      }
    } else if (absGeneratedPDG == 12 || absGeneratedPDG == 14 || absGeneratedPDG == 16) { // missing neutrino
      flags |= c_MissNeutrino;
    } else { //neither photon nor neutrino -> massive
      flags |= c_MissMassiveParticle;
      if (absGeneratedPDG == Const::Klong.getPDGCode())
        flags |= c_MissKlong;
    }
  }
  return flags;
}

int MCMatching::countMissingParticle(const Particle* particle, const MCParticle* mcParticle, const vector<int>& daughterPDG)
{
  unordered_set<const MCParticle*> mcMatchedParticles;
  // Missing particles which are accepted by the property flags are NOT stored.
  // --> Such particles are also counted in nMissingDaughter.
  appendParticles(particle, mcMatchedParticles);
  vector<const MCParticle*> genParts;
  appendParticles(mcParticle, genParts);

  int nMissingDaughter = 0;

  for (const MCParticle* genPart : genParts) {
    const bool missing = (mcMatchedParticles.find(genPart) == mcMatchedParticles.end());
    if (missing) {

      const int generatedPDG = genPart->getPDG();
      const int absGeneratedPDG = abs(generatedPDG);

      auto result = find(daughterPDG.begin(), daughterPDG.end(), absGeneratedPDG);
      if (result != daughterPDG.end())
        nMissingDaughter++;
    }
  }

  return nMissingDaughter;
}

int MCMatching::getFlagsIgnoredByProperty(const Particle* part)
{
  int flags = 0;

  if (part->getProperty() & Particle::PropertyFlags::c_IsIgnoreBrems) flags |= (MCMatching::c_AddedRecoBremsPhoton);

  return flags;
}

int MCMatching::makeDaughterAcceptMask(int daughterProperty)
{
  int flags = 0;

  if (daughterProperty & Particle::PropertyFlags::c_IsIgnoreMisID) flags |= (MCMatching::c_MisID);
  if (daughterProperty & Particle::PropertyFlags::c_IsIgnoreDecayInFlight) flags |= (MCMatching::c_DecayInFlight);

  return (~flags);

}
