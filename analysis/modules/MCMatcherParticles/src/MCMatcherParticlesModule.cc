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
                 "- looseMCWrongDaughterBiB: 1 if the wrong daughter is Beam Induced Background\n"
                 "  Particle");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("listName", m_listName, "Name of the input ParticleList.");
  addParam("looseMCMatching", m_looseMatching, "Perform loose mc matching", false);
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
  }
}

void MCMatcherParticlesModule::setLooseMCMatch(const Particle* particle)
{
  if (particle->hasExtraInfo("looseMCMotherPDG")) // nothing to do; already set
    return;

  // get all FS daughters
  vector<const Belle2::Particle*> fsDaughters = particle->getFinalStateDaughters();

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
                        std::begin(motherCount), std::end(motherCount),
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
  if (commonMother == std::end(motherCount)) {
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
