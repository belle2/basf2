/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/MCMatcherParticles/MCMatcherParticlesModule.h>

// utility
#include <analysis/utility/MCMatching.h>
#include <analysis/utility/AnalysisConfiguration.h>

// map
#include <unordered_map>
#include <algorithm>
typedef std::unordered_map<unsigned int, unsigned int> CounterMap;

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(MCMatcherParticles);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MCMatcherParticlesModule::MCMatcherParticlesModule() : Module()
{
  setDescription("Performs MC matching (sets relation Particle->MCParticle) for all particles\n"
                 "(and its (grand)^N-daughter particles) in the ParticleList. The relation can\n"
                 "be used in conjunction with MCMatching::MCErrorFlags flags, e.g. using the\n"
                 "isSignal or mcPDG & mcErrors variables.\n"
                 "\n"
                 "In addition to the usual mc matching algorithm the module can run also loose mc\n"
                 "matching. The difference between loose and normal mc matching algorithm is that\n"
                 "the loose algorithm will find the common mother of the majority of daughter\n"
                 "particles while the normal algorithm finds the common mother of all daughters.\n"
                 "The results of loose mc matching algorithm are stored to the following extraInfo\n"
                 "items:\n\n"
                 "- looseMCMotherPDG: PDG code of most common mother\n"
                 "- looseMCMotherIndex: 1-based StoreArray<MCParticle> index of most common mother\n"
                 "- looseMCWrongDaughterN: number of daughters that don't originate from the most"
                 "  common mother\n"
                 "- looseMCWrongDaughterPDG: PDG code of the daughter that doesn't originate from\n"
                 "  the most common mother (only if looseMCWrongDaughterN = 1)\n"
                 "- looseMCWrongDaughterBiB: 1 if the wrong daughter is Beam Induced Background Particle\n"
                 "\n"
                 "Can also perform tag matching for (ccbar) tags. Requires that normal MC\n"
                 "matching has already been performed and set relations.\n"
                 "Low energy photons with energy < 0.1 GeV and ISR are ignored.\n"
                 "The results of (ccbar) tag matching algorithm are stored to the following extraInfo\n"
                 "items:\n\n"
                 "- ccbarTagSignal: 1st digit is status of signal particle, 2nd digit is Nleft-1, 3rd digit is NextraFSP.\n"
                 "- ccbarTagMCpdg: PDG code of (charm) hadron outside tag (signal side).\n"
                 "- ccbarTagMCpdgMother: PDG code of the mother of the (charm) hadron outside tag (signal side).\n"
                 "- ccbarTagNleft: number of particles (composites have priority) left outisde tag.\n"
                 "- ccbarTagNextraFSP: number of extra FSP particles attached to the tag.\n"
                 "- ccbarTagSignalStatus: status of the targeted signal side particle.\n"
                 "- ccbarTagNwoMC: number of daughters without MC match.\n"
                 "- ccbarTagNwoMCMother: number of daughters without MC mother.\n"
                 "- ccbarTagNnoAllMother: number of daughters without common allmother.\n"
                 "- ccbarTagNmissGamma: number of daughters with missing gamma mc error.\n"
                 "- ccbarTagNmissNeutrino: number of daughters with missing neutrino mc error.\n"
                 "- ccbarTagNdecayInFlight: number of daughters with decay in flight mc error.\n"
                 "- ccbarTagNsevereMCError: number of daughters with severe mc error.\n"
                 "- ccbarTagNmissRecoDaughters: number of daughters with any mc error.\n"
                 "- ccbarTagNleft2ndPDG: PDG of one particle left additionally to the signal particle.\n"
                 "- ccbarTagAllMotherPDG: PDG code of the allmother (Z0 or virtual photon).");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("listName", m_listName, "Name of the input ParticleList.");
  addParam("looseMCMatching", m_looseMatching, "Perform loose mc matching", false);
  addParam("ccbarTagMatching", m_ccbarTagMatching, "Perform ccbar tag matching", false);
}


void MCMatcherParticlesModule::initialize()
{
  // check that there are MCParticles: shout if not
  if (!m_mcparticles.isValid()) {
    B2WARNING("No MCParticles array found!"
              << " This is obvously fine if you're analysing real data,"
              << " but you have added the MCMatcher module to your path,"
              << " did you mean to do this?");
    return;
  }

  // if we have MCParticles then continue with the initialisation
  m_particles.isRequired();
  m_particles.registerRelationTo(m_mcparticles);
  m_plist.isRequired(m_listName);

  bool legacyAlgorithm = AnalysisConfiguration::instance()->useLegacyMCMatching();
  B2INFO("MCMatcher module will search for Particle -> MCParticle associations for the ParticleList " << m_listName << ".");
  if (legacyAlgorithm)
    B2INFO(" - The MCMatcher will use legacy algorithm suitable for analysis of Belle MC.");
  else
    B2INFO("  - The MCMatcher will use default algorithm suitable for analysis of Belle II MC.");
}


void MCMatcherParticlesModule::event()
{
  // if no MCParticles then silently skip
  if (!m_mcparticles.isValid())
    return;
  if (!m_plist) {
    B2ERROR("ParticleList " << m_listName << " not found");
    return;
  }

  const unsigned int n = m_plist->getListSize();
  for (unsigned i = 0; i < n; i++) {
    const Particle* part = m_plist->getParticle(i);

    MCMatching::setMCTruth(part);

    if (m_looseMatching)
      setLooseMCMatch(part);

    if (m_ccbarTagMatching)
      setCCbarTagMatch(part);
  }
}

void MCMatcherParticlesModule::setLooseMCMatch(const Particle* particle)
{
  if (particle->hasExtraInfo("looseMCMotherPDG")) // nothing to do; already set
    return;

  // get all FS daughters
  vector<const Particle*> fsDaughters = particle->getFinalStateDaughters();

  // map for counting how many times given mcparticle is mother of daughters
  CounterMap motherCount;

  for (auto daughter : fsDaughters) {
    const MCParticle* mcDaughter = daughter->getRelatedTo<MCParticle>();
    if (!mcDaughter)
      continue;

    vector<int> genMothers;
    MCMatching::fillGenMothers(mcDaughter, genMothers);

    for (auto motherIndex : genMothers) {
      // exclude ROOT particles: Upsilon(nS), virtual photon
      int motherPDG =  m_mcparticles[motherIndex - 1]->getPDG();
      if ((motherPDG == 553) ||
          (motherPDG == 100553) ||
          (motherPDG == 200553) ||
          (motherPDG == 300553) ||
          (motherPDG == 9000553) ||
          (motherPDG == 9010553) ||
          (motherPDG == 10022))
        continue;

      motherCount[motherIndex]++;
    }
  }

  // find first most common mother
  auto commonMother = std::max_element
                      (
                        motherCount.begin(), motherCount.end(),
  [](std::pair <unsigned int, unsigned int> p1, std::pair <unsigned int, unsigned int> p2) {
    bool returnValue = false;
    if (p1.second < p2.second)
      returnValue = true;
    else if (p1.second == p2.second)
      returnValue = p2.first > p1.first;

    return returnValue;
  }
                      );

  // No common mother found, all daughters have no associated MC Particle
  if (commonMother == motherCount.end()) {
    Particle* thisParticle = m_particles[particle->getArrayIndex()];
    thisParticle->addExtraInfo("looseMCMotherPDG",   -1);
    thisParticle->addExtraInfo("looseMCMotherIndex", -1);
    thisParticle->addExtraInfo("looseMCWrongDaughterN", -1);
    thisParticle->addExtraInfo("looseMCWrongDaughterPDG", -1);
    thisParticle->addExtraInfo("looseMCWrongDaughterBiB", -1);
    return;
  }

  const MCParticle* mcMother = m_mcparticles[commonMother->first - 1];

  Particle* thisParticle = m_particles[particle->getArrayIndex()];
  thisParticle->addExtraInfo("looseMCMotherPDG",   mcMother->getPDG());
  thisParticle->addExtraInfo("looseMCMotherIndex", mcMother->getArrayIndex());
  thisParticle->addExtraInfo("looseMCWrongDaughterN", fsDaughters.size() - commonMother->second);

  // find out what kind of particle was wrongly added
  // only for the case where there is only one such particle
  // This is the most interesting case. If two or more
  // particles are wrongly added, then a candidate looks
  // like background
  int wrongParticlePDG = 0; // PDG code of the wrongly matched particle
  int wrongParticleBiB = 0; // true (false) if particle doesn't (does) have MCParticle relation
  if (fsDaughters.size() - commonMother->second == 1) {
    for (auto daughter : fsDaughters) {
      const MCParticle* mcDaughter = daughter->getRelatedTo<MCParticle>();
      if (!mcDaughter) {
        wrongParticlePDG  = daughter->getPDGCode();
        wrongParticleBiB = 1;
      }

      vector<int> genMothers;
      MCMatching::fillGenMothers(mcDaughter, genMothers);

      // check if current daughter descends from common mother
      if (find(genMothers.begin(), genMothers.end(), commonMother->first) != genMothers.end())
        continue;

      // daughter is not a child of common mother
      wrongParticlePDG = daughter->getPDGCode();
    }
  }

  thisParticle->addExtraInfo("looseMCWrongDaughterPDG", wrongParticlePDG);
  thisParticle->addExtraInfo("looseMCWrongDaughterBiB", wrongParticleBiB);

}


int MCMatcherParticlesModule::ccbarTagPartialHelper(
  const MCParticle* mcParticle,
  std::vector<const Particle*>& fspParticles,
  std::vector<const MCParticle*>& missedParticles
)
{
  bool CaughtAll = true;
  bool missedAll = true;
  for (auto& mcDaughter : mcParticle->getDaughters()) {

    auto it = std::find_if(fspParticles.begin(), fspParticles.end(), [mcDaughter](const Particle * fsp) { return fsp->getMCParticle() == mcDaughter; });
    if (it != fspParticles.end()) {
      fspParticles.erase(it);
      missedAll = false;
    } else {
      if (mcDaughter->getPDG() == Const::photon.getPDGCode() && (mcDaughter->getEnergy() < 0.1
                                                                 or mcDaughter->hasStatus(MCParticle::c_IsISRPhoton))) {
        continue; // ignore radiative photons with energy < 0.1 GeV and ISR as if they are not in the event
      }
      // Belle MC legacy remnants
      if (abs(mcDaughter->getPDG()) == 21) continue; // ignore gluons
      if (abs(mcDaughter->getPDG()) == 1) continue; // ignore down quarks
      if (abs(mcDaughter->getPDG()) == 2) continue; // ignore up quarks
      if (abs(mcDaughter->getPDG()) == 3) continue; // ignore strange quarks
      if (abs(mcDaughter->getPDG()) == 4) continue; // ignore charm quarks
      if (abs(mcDaughter->getPDG()) == 5) continue; // ignore bottom quarks
      if (abs(mcDaughter->getPDG()) == 6) continue; // ignore top quarks
      // TODO are there any other particles that should be ignored, due to Belle MC generation?

      else if (mcDaughter->getNDaughters() == 0) {
        missedParticles.push_back(mcDaughter);
        CaughtAll = false;
      } else {
        std::vector<const MCParticle*> tempMissedParticles;
        int status = ccbarTagPartialHelper(mcDaughter, fspParticles, tempMissedParticles);
        if (status == 0) {
          missedParticles.push_back(mcDaughter);
          CaughtAll = false;
        } else if (status == 1) {
          missedAll = false; // tempMissedParticles is empty
        } else {
          missedAll = false;
          CaughtAll = false;
          missedParticles.insert(missedParticles.end(), tempMissedParticles.begin(), tempMissedParticles.end());
        }
      }
    }

  }

  if (missedAll) return 0;
  else if (CaughtAll) return 1;
  else return 2;
}


int MCMatcherParticlesModule::ccbarTagPartialHelper(
  const MCParticle* mcParticle,
  const std::vector<const Particle*>& fspParticles
)
{
  bool CaughtAll = true;
  bool missedAll = true;
  for (auto& mcDaughter : mcParticle->getDaughters()) {
    if (mcDaughter->getPDG() == Const::photon.getPDGCode()
        && mcDaughter->getEnergy() < 0.1) continue; // ignore radiative photons with energy < 0.1 GeV as if they are not in the event
    auto it = std::find_if(fspParticles.begin(), fspParticles.end(), [mcDaughter](const Particle * fsp) { return fsp->getMCParticle() == mcDaughter; });
    if (it != fspParticles.end()) missedAll = false;
    else if (mcDaughter->getNDaughters() == 0) CaughtAll = false;
    else {
      int status = ccbarTagPartialHelper(mcDaughter, fspParticles);
      if (status == 0) CaughtAll = false;
      else if (status == 1) missedAll = false;
      else {
        missedAll = false;
        CaughtAll = false;
      }
    }
  }
  if (missedAll) return 0;
  else if (CaughtAll) return 1;
  else return 2;
}


void MCMatcherParticlesModule::setCCbarTagMatch(const Particle* particle)
{
  if (particle->hasExtraInfo("ccbarTagSignalStatus")) // nothing to do; already set
    return;
  int ccbarTagSignal = 0;
  Particle* thisParticle = m_particles[particle->getArrayIndex()];

  // get Z0, virtual photon
  StoreArray<MCParticle> mcparticles;
  const MCParticle* allMother = nullptr;
  for (int i = 0; i < mcparticles.getEntries(); i++) {
    if (abs(mcparticles[i]->getPDG()) == 23 ||
        abs(mcparticles[i]->getPDG()) == 553 ||
        abs(mcparticles[i]->getPDG()) == 100553 ||
        abs(mcparticles[i]->getPDG()) == 200553 ||
        abs(mcparticles[i]->getPDG()) == 300553 ||
        abs(mcparticles[i]->getPDG()) == 9000553 ||
        abs(mcparticles[i]->getPDG()) == 9010553 ||
        abs(mcparticles[i]->getPDG()) == 10022) {
      allMother = mcparticles[i];
      break;
    }
  }
  if (!allMother) {
    B2WARNING("MCMatcherParticlesModule; tag matching - event has no AllMother?");
    return;
  }
  thisParticle->addExtraInfo("ccbarTagAllMotherPDG", allMother->getPDG());

  int sigPDGCode = particle->getPDGCode() * (-1); // get info about signal particles
  int eventStatus = 0; // default: signal particle is missing
  for (int i = 0; i < mcparticles.getEntries(); i++) {
    if (mcparticles[i]->getPDG() == sigPDGCode) {
      int recStatus = ccbarTagPartialHelper(mcparticles[i], particle->getFinalStateDaughters());
      if (recStatus == 0) {
        eventStatus = 1; // 1 still present in event
        break;
      } else if (recStatus == 2) eventStatus = 2; // 2 partially reconstructed by tag
      else if (recStatus == 1 && eventStatus != 2) eventStatus = 3; // 3 fully absorbed by tag
    }
  }
  thisParticle->addExtraInfo("ccbarTagSignalStatus", eventStatus);
  ccbarTagSignal += eventStatus;

  // recursive TagMCpdg, Nleft particles ---------------------------------
  std::vector<const Particle*> fspDaughters = particle->getFinalStateDaughters();
  std::vector<const MCParticle*> missedParticles;
  ccbarTagPartialHelper(allMother, fspDaughters, missedParticles);
  bool foundCharm = false;
  for (auto* mcpart : missedParticles) {
    int pdg = abs(mcpart->getPDG());
    if ((pdg > 400 && pdg < 500) || (pdg > 4000 && pdg < 5000) || (pdg > 10400 && pdg < 10500) || (pdg > 20400 && pdg < 20500)) {
      foundCharm = true;
      thisParticle->addExtraInfo("ccbarTagMCpdg", mcpart->getPDG());
      const MCParticle* mcpartMother = mcpart->getMother();
      int mcpartMotherPDG = 0;
      if (mcpartMother) mcpartMotherPDG = mcpartMother->getPDG();
      thisParticle->addExtraInfo("ccbarTagMCpdgMother", mcpartMotherPDG);
      break;
    }
  }
  if (!foundCharm) {
    thisParticle->addExtraInfo("ccbarTagMCpdg", 0);
    thisParticle->addExtraInfo("ccbarTagMCpdgMother", 0);
  }
  thisParticle->addExtraInfo("ccbarTagNextraFSP", fspDaughters.size());
  ccbarTagSignal += 10 * fspDaughters.size();
  thisParticle->addExtraInfo("ccbarTagNleft", missedParticles.size());
  ccbarTagSignal += 100 * (missedParticles.size() - 1);
  thisParticle->addExtraInfo("ccbarTagSignal", ccbarTagSignal);

  // debugging purposes only, to be removed, probably
  int secondLeftParticle = 0;
  if (missedParticles.size() == 2) {
    if (missedParticles[0]->getPDG() == sigPDGCode) secondLeftParticle = missedParticles[1]->getPDG();
    else if (missedParticles[1]->getPDG() == sigPDGCode) secondLeftParticle = missedParticles[0]->getPDG();
  }
  thisParticle->addExtraInfo("ccbarTagNleft2ndPDG", secondLeftParticle);


  // daughter and allmother checks
  int nWOmcParticles = 0;
  int nWOmother = 0;
  int nNonCommonAllMother = 0;
  for (auto& daughter : particle->getDaughters()) {
    if (!daughter->getMCParticle()) {
      nWOmcParticles++;
      continue;
    }
    const MCParticle* curMCMother = daughter->getMCParticle()->getMother();
    if (curMCMother == nullptr) {
      nWOmother++;
      continue;
    }

    const MCParticle* grandMother = curMCMother->getMother();
    while (grandMother != nullptr) {
      curMCMother = grandMother;
      grandMother = curMCMother->getMother();
    }
    if (curMCMother->getPDG() != Const::photon.getPDGCode() && curMCMother->getPDG() != 23
        && curMCMother->getPDG() != 10022) nNonCommonAllMother++;
    else if (allMother != curMCMother) nNonCommonAllMother++;
  }
  thisParticle->addExtraInfo("ccbarTagNwoMC", nWOmcParticles);
  thisParticle->addExtraInfo("ccbarTagNwoMCMother", nWOmother);
  thisParticle->addExtraInfo("ccbarTagNnoAllMother", nNonCommonAllMother);

  // daughter mcErrors ---------------------------------
  int nHasMissingGamma = 0;
  int nHasMissingNeutrino = 0;
  int nHasDecayInFlight = 0;
  int nHasSevereMCError = 0;
  int nMissRecoDaughters = 0;
  for (auto& daughter : particle->getDaughters()) { // TODO think about less strict conditions
    if (daughter->hasExtraInfo("ccbarTagNmissRecoDaughters")) {
      nHasMissingGamma    += daughter->getExtraInfo("ccbarTagNmissGamma");
      nHasMissingNeutrino += daughter->getExtraInfo("ccbarTagNmissNeutrino");
      nHasDecayInFlight   += daughter->getExtraInfo("ccbarTagNdecayInFlight");
      nHasSevereMCError   += daughter->getExtraInfo("ccbarTagNsevereMCError");
      nMissRecoDaughters  += daughter->getExtraInfo("ccbarTagNmissRecoDaughters");
      continue;
    }

    int mcError = MCMatching::getMCErrors(daughter, daughter->getMCParticle());
    nMissRecoDaughters += 1;
    if (mcError == MCMatching::c_Correct || mcError == MCMatching::c_MissingResonance) {
      nMissRecoDaughters -= 1;
      continue;
    } else if (mcError == MCMatching::c_MissGamma || mcError == MCMatching::c_MissFSR
               || mcError == MCMatching::c_MissPHOTOS) nHasMissingGamma += 1;
    else if (mcError == MCMatching::c_MissNeutrino) nHasMissingNeutrino += 1;
    else if (mcError == MCMatching::c_DecayInFlight) nHasDecayInFlight += 1;
    else nHasSevereMCError += 1;
  }
  thisParticle->addExtraInfo("ccbarTagNmissGamma", nHasMissingGamma);
  thisParticle->addExtraInfo("ccbarTagNmissNeutrino", nHasMissingNeutrino);
  thisParticle->addExtraInfo("ccbarTagNdecayInFlight", nHasDecayInFlight);
  thisParticle->addExtraInfo("ccbarTagNsevereMCError", nHasSevereMCError);
  thisParticle->addExtraInfo("ccbarTagNmissRecoDaughters", nMissRecoDaughters);
}