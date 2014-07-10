/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/PreCutHistMaker/PreCutHistMakerModule.h>

#include <analysis/utility/PSelectorFunctions.h>
#include <analysis/utility/MCMatching.h>
#include <analysis/utility/EvtPDLUtil.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/logging/Logger.h>

#include <TH1F.h>

#include <algorithm>


using namespace std;
using namespace Belle2;


REG_MODULE(PreCutHistMaker)


PreCutHistMakerModule::PreCutHistMakerModule():
  m_pdg(0),
  m_var(nullptr),
  m_histogramSignal(nullptr),
  m_histogramAll(nullptr),
  m_generator_signal(nullptr),
  m_generator_all(nullptr)
{
  setDescription("Saves distribution of a variable of combined particles (from input ParticleLists) into histogram 'all'. If the daughters can be combined into a correctly reconstructed (!) particle of specified type, save variable value for this combination to a histogram called 'signal'. This is equivalent to running ParticleCombiner on the given lists and saving the variable value of Particles with isSignal == 1 and everything else, but much faster (since Particles don't need to be saved).");
  setPropertyFlags(c_ParallelProcessingCertified); //histograms are saved through HistModule, so this is ok

  addParam("decayString", m_decayString, "Decay to reconstruct, see https://belle2.cc.kek.jp/~twiki/bin/view/Physics/DecayString ");

  HistParams defaultHistParams = std::make_tuple(100, 0, 6);
  addParam("histParams", m_histParams, "Tuple specifying number of bins, lower and upper boundary of the variable histogram. (for invariant mass M in GeV)", defaultHistParams);
  addParam("fileName", m_fileName, "Name of the TFile where the histograms are saved.");
  addParam("variable", m_variable, "Variable for which the distributions are calculated");
  addParam("customBinning", m_customBinning, "Custom binning, which is used instead of histParams. Specify low-edges for each bin, with nbins+1 entries.", std::vector<float>());

}

PreCutHistMakerModule::~PreCutHistMakerModule()
{
}

void PreCutHistMakerModule::initialize()
{
  setFilename(m_fileName);

  StoreArray<MCParticle>::required();
  StoreArray<Particle>::required();

  // obtain the input and output particle lists from the decay string
  DecayDescriptor decay;
  bool valid = decay.init(m_decayString);
  if (!valid)
    B2ERROR("Invalid input DecayString: " << m_decayString);

  // Mother particle
  const DecayDescriptorParticle* mother = decay.getMother();

  m_pdg = mother->getPDGCode();
  m_channelName = mother->getFullName();

  std::stringstream onlySignal_decayString;
  onlySignal_decayString << mother->getFullName() << " ==> ";

  // Daughters
  set<int> setOfPDGs;
  set<string> setOfInputListNames;
  int nProducts = decay.getNDaughters();
  for (int i = 0; i < nProducts; ++i) {
    const DecayDescriptorParticle* daughter = decay.getDaughter(i)->getMother();

    const int daughterPDG = daughter->getPDGCode();
    m_inputLists.emplace_back(daughter->getFullName());

    setOfPDGs.insert(daughterPDG);
    setOfInputListNames.insert(daughter->getFullName());

    std::string listName = Belle2::EvtPDLUtil::particleListName(daughterPDG, "HistMaker");
    onlySignal_decayString << " " << listName;

    m_tmpLists.emplace_back(listName);
    m_tmpLists.back().registerAsTransient(false);
    bool isSelfConjugated = !(Belle2::EvtPDLUtil::hasAntiParticle(daughterPDG));
    if (!isSelfConjugated) {
      std::string antiListName = Belle2::EvtPDLUtil::antiParticleListName(daughterPDG, "HistMaker");
      StoreObjPtr<ParticleList>::registerTransient(antiListName, DataStore::c_Event, false);
    }
  }

  if (setOfPDGs.size() != setOfInputListNames.size()) {
    B2ERROR("You're using at least two different input lists for the same particle in decay '" << m_decayString << "'. This won't work at the moment. [Can be implemented by going over all input particle lists in fillParticleLists() and adding Particles to all temporary lists where they are found in the corresponding input lists.]");
  }


  for (auto l : m_inputLists) {
    l.isRequired();
  }

  if (m_customBinning.size() > 0) {
    m_histogramSignal = new TH1F((std::string("signal") + m_channelName).c_str(), "signal", m_customBinning.size() - 1, &m_customBinning[0]);
    m_histogramAll = new TH1F((std::string("all") + m_channelName).c_str(), "all", m_customBinning.size() - 1, &m_customBinning[0]);
  } else {
    int nbins;
    double xlow, xhigh;
    std::tie(nbins, xlow, xhigh) = m_histParams;
    m_histogramSignal = new TH1F((std::string("signal") + m_channelName).c_str(), "signal", nbins, xlow, xhigh);
    m_histogramAll = new TH1F((std::string("all") + m_channelName).c_str(), "all", nbins, xlow, xhigh);
  }

  VariableManager& manager = VariableManager::Instance();
  m_var = manager.getVariable(m_variable);

  if (m_var == nullptr) {
    B2ERROR("PreCutHistMaker: VariableManager doesn't have variable" <<  m_variable)
  }

  m_generator_all = new ParticleGenerator(m_decayString);
  m_generator_signal = new ParticleGenerator(onlySignal_decayString.str());
}


void getMatchingDaughters(const MCParticle& mcparticle, std::vector<MCParticle*>& daughters, const std::multiset<int>& expectedDaughterPDGsAbs, std::map<int, int>& foundPDGs)
{
  const std::vector<MCParticle*>& mcDaughters = mcparticle.getDaughters();
  for (auto d : mcDaughters) {
    bool secondary = !d->hasStatus(MCParticle::c_PrimaryParticle);
    int abspdg = abs(d->getPDG());
    foundPDGs[abspdg]++;

    //we'll accept secondaries anyway because of decay in flight (depends on isSignal definition)
    if (secondary or expectedDaughterPDGsAbs.count(abspdg) != 0)
      daughters.push_back(d);

    getMatchingDaughters(*d, daughters, expectedDaughterPDGsAbs, foundPDGs);
  }
}

bool getMatchingDaughters(const MCParticle& mcparticle, std::vector<MCParticle*>& daughters, const std::multiset<int>& expectedDaughterPDGsAbs)
{
  std::map<int, int> foundPDGs;
  getMatchingDaughters(mcparticle, daughters, expectedDaughterPDGsAbs, foundPDGs);

  //check we found all expected daughters
  for (auto pdg : expectedDaughterPDGsAbs) {
    if (foundPDGs[pdg] <= 0) {
      //not found
      return false;
    }
    //
    foundPDGs[pdg]--;
  }
  return true;
}


bool hasAntiParticle(int pdg)
{

  if (pdg < 0) return true;
  if (pdg == 22) return false;
  if (pdg == 310) return false;
  if (pdg == 130) return false;
  int nnn = pdg / 10;
  int q3 = nnn % 10; nnn /= 10;
  int q2 = nnn % 10; nnn /= 10;
  int q1 = nnn % 10;
  if (q1 == 0 && q2 == q3) return false;

  return true;
}

void PreCutHistMakerModule::clearTemporaryLists()
{
  for (auto & list : m_tmpLists) {
    int pdg = list->getPDGCode();
    std::string name = list->getParticleListName();
    list.create(true);
    list->initialize(pdg, name);

    //create and bind antiparticle list if necessary
    bool isSelfConjugated = !(Belle2::EvtPDLUtil::hasAntiParticle(pdg));
    if (!isSelfConjugated) {
      std::string antiListName = Belle2::EvtPDLUtil::antiParticleListName(pdg, "HistMaker");
      StoreObjPtr<ParticleList> antiParticleList(antiListName);
      antiParticleList.create(true);
      antiParticleList->initialize(-pdg, antiListName);
      list->bindAntiParticleList(*(antiParticleList));
    }
  }
}

bool PreCutHistMakerModule::fillParticleLists(const std::vector<MCParticle*>& mcDaughters)
{
  //map abs(PDG) to list
  //TODO: are there cases where we want misID-ed Particles? (e.g. pdg different mcPart)
  std::map<int, StoreObjPtr<ParticleList>> pdgToTmpList;
  std::map<int, StoreObjPtr<ParticleList>> pdgToInputList;
  for (auto & plist : m_tmpLists) {
    pdgToTmpList[abs(plist->getPDGCode())] = plist;
  }
  for (auto & plist : m_inputLists) {
    pdgToInputList[abs(plist->getPDGCode())] = plist;
  }

  for (const MCParticle * mcPart : mcDaughters) {
    //B2WARNING("fillParticleLists: mc " << mcPart->getIndex() << " " << mcPart->getPDG());
    const auto& particles = mcPart->getRelationsWith<Particle>();
    for (const Particle & p : particles) {
      //B2WARNING("fillParticleLists: adding p " << p.getArrayIndex() << " " << p.getPDGCode());

      const auto& it = pdgToTmpList.find(abs(p.getPDGCode()));
      if (it != pdgToTmpList.end()) {
        //in principle this seems ok, also check if Particle occurs in our input lists
        if (pdgToInputList.find(abs(p.getPDGCode()))->second->contains(&p)) {
          //add particles into corresponding temporary lists
          it->second->addParticle(&p);
        }
      }
    }
  }

  //check if all lists are filled
  for (auto & plist : m_tmpLists) {
    if (plist->getListSize() == 0)
      return false;
  }
  return true;
}

void PreCutHistMakerModule::saveCombinationsForSignal()
{

  StoreArray<Particle> particles;
  m_generator_signal->init();
  while (m_generator_signal->loadNext()) {

    const Particle particle = m_generator_signal->getCurrentParticle();
    Particle* part = particles.appendNew(particle);
    MCMatching::setMCTruth(part);
    //B2WARNING("combined Particle created.");
    if (analysis::isSignal(part) < 0.5) {
      /*
      B2WARNING("mcMatching says No. (status: " << MCMatching::getMCTruthStatus(part));
      part->print();
      */
      continue;
    }
    //B2WARNING("passed");
    m_histogramSignal->Fill(m_var->function(part));
  }

}

void PreCutHistMakerModule::saveAllCombinations()
{
  m_generator_all->init();
  while (m_generator_all->loadNext()) {
    const Particle part = m_generator_all->getCurrentParticle();
    m_histogramAll->Fill(m_var->function(&part));
  }
}

void PreCutHistMakerModule::event()
{
  std::multiset<int> expectedDaughterPDGsAbs;
  for (unsigned i = 0; i < m_inputLists.size(); i++) {
    StoreObjPtr<ParticleList>& list = m_inputLists[i];
    const string name = list.getName();
    if (!list) {
      B2ERROR("ParticleList " << name << " not found");
      return;
    }
    if (list->getParticleCollectionName() != std::string("Particles")) {
      B2ERROR("ParticleList " << name <<
              " does not refer to the default Particle collection");
      return;
    }
    expectedDaughterPDGsAbs.insert(abs(list->getPDGCode()));

    //create temporary input lists (and replace any existing object)
    m_tmpLists[i].create(true);
    m_tmpLists[i]->initialize(list->getPDGCode(), list->getParticleListName());
    //antiparticle lists are created in clearTemporaryLists(), so this doesn't need to be done here
  }

  //for signal + background
  saveAllCombinations();


  //combinations for signal decays
  StoreArray<MCParticle> mcparticles;
  for (const MCParticle & mcparticle : mcparticles) {
    if (abs(mcparticle.getPDG()) == abs(m_pdg)) {
      //we found a matching mcparticle, but is it the same decay?

      //compare PDG codes with what we expect from particlelists (absolute values only, as we might not know the flavour of reconstructed Particles)
      std::vector<MCParticle*> mcDaughters;
      if (getMatchingDaughters(mcparticle, mcDaughters, expectedDaughterPDGsAbs)) {
        //B2WARNING("mcdecay found");
        if (mcDaughters.size() < m_inputLists.size()) {
          /*
          B2WARNING("size mismatch, discarding " << mcparticle.getIndex());
          for (auto m : mcDaughters) {
            B2WARNING("" << m->getPDG());
          }
          */
          continue;
        }

        clearTemporaryLists();

        //we found a true decay, collect all Particles associated with the daughters
        if (!fillParticleLists(mcDaughters)) {
          //B2WARNING("not all hypotheses found!");
          continue;
        }

        //do combinations and keep the correct ones
        saveCombinationsForSignal();
      }
    }
  }
}

void PreCutHistMakerModule::terminate()
{
  saveHists();

  delete m_generator_signal;
  delete m_generator_all;
  delete m_histogramAll;
  delete m_histogramSignal;
}

void PreCutHistMakerModule::writeHists()
{
  m_histogramSignal->Write();
  m_histogramAll->Write();
}
