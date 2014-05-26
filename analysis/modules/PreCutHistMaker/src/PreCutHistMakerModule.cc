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
#include <analysis/utility/mcParticleMatching.h>
#include <analysis/ParticleCombiner/ParticleCombiner.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TH1F.h>

#include <algorithm>


using namespace std;
using namespace Belle2;



//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(PreCutHistMaker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PreCutHistMakerModule::PreCutHistMakerModule()
{
  setDescription("Saves distribution of a variable of combined particles (from input ParticleLists) into histogram 'all'. If the daughters in the given particle lists can be combined into a correctly reconstructed (!) particle of specified PDG code, save variable value for this combination to a histogram called 'signal'. This is equivalent to running ParticleCombiner on the given lists and saving the variable value of Particles with isSignal == 1 and everything else, but much faster (since Particles don't need to be saved).");
  setPropertyFlags(c_ParallelProcessingCertified); //histograms are saved through HistModule, so this is ok


  addParam("PDG", m_pdg, "PDG code of particle to reconstruct (anti-particles reconstructed implicitly)");
  addParam("channelName", m_channelName, "Optional name for the channel which is used to name the histograms (e.g. 'signal'+channelName)", string(""));
  addParam("inputListNames", m_inputListNames, "Particle lists of the daughter particles to be combined. MCMatching should be run on these lists beforehand.");

  HistParams defaultHistParams = std::make_tuple(100, 0, 6);
  addParam("histParams", m_histParams, "Tuple specifying number of bins, lower and upper boundary of the variable histogram. (for invariant mass M in GeV)", defaultHistParams);
  addParam("fileName", m_fileName, "Name of the TFile where the histograms are saved.");
  addParam("variable", m_variable, "Variable for which the distributions are calculated");
  addParam("customBinning", m_customBinning, "Custom Binning, which is used instead of histParams", std::vector<float>());

}

PreCutHistMakerModule::~PreCutHistMakerModule()
{
}

void PreCutHistMakerModule::initialize()
{
  setFilename(m_fileName);

  StoreArray<MCParticle>::required();
  StoreArray<Particle>::required();
  for (string name : m_inputListNames) {
    StoreObjPtr<ParticleList>::required(name);

    m_tmpLists.emplace_back("HistMaker" + name);
    m_tmpLists.back().registerAsTransient();
  }

  if (m_customBinning.size() > 0) {
    m_histogramSignal = new TH1F((std::string("signal") + m_channelName).c_str(), "signal", m_customBinning.size(), &m_customBinning[0]);
    m_histogramAll = new TH1F((std::string("all") + m_channelName).c_str(), "all", m_customBinning.size(), &m_customBinning[0]);
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

void PreCutHistMakerModule::clearParticleLists()
{
  for (auto & list : m_tmpLists) {
    int pdg = list->getPDGCode();
    std::string name = list->getParticleListName();
    list.create(true);
    list->initialize(pdg, name);
    // TODO is at this point necessary to bind anti list? probably yes
  }
}

bool PreCutHistMakerModule::fillParticleLists(const std::vector<MCParticle*>& mcDaughters)
{
  for (const MCParticle * mcPart : mcDaughters) {
    //B2WARNING("fillParticleLists: mc " << mcPart->getIndex() << " " << mcPart->getPDG());
    const auto& particles = mcPart->getRelationsWith<Particle>();
    for (const Particle & p : particles) {
      //B2WARNING("fillParticleLists: adding p " << p.getArrayIndex() << " " << p.getPDGCode());

      //add particles into corresponding temporary lists
      for (auto & plist : m_tmpLists) {
        //TODO: are there cases where we want misID-ed Particles? (e.g. pdg different mcPart)
        if (abs(p.getPDGCode()) == abs(plist->getPDGCode())) {
          plist->addParticle(&p);
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
  // Convert input ParticleList(s) to PCombinerList(s)
  vector<PCombinerList> inputPCombinerLists;
  for (const auto & listName : m_tmpLists) {
    StoreObjPtr<ParticleList> list(listName);
    PCombinerList plist;
    convert(list, plist);
    inputPCombinerLists.push_back(plist);
  }

  StoreArray<Particle> particles;
  ParticleCombiner combiner(inputPCombinerLists, !hasAntiParticle(m_pdg));

  while (combiner.loadNext()) {

    const Particle particle = combiner.getCurrentParticle(m_pdg, -m_pdg);
    Particle* part = particles.appendNew(particle);
    MCMatching::setMCTruth(part);
    //B2WARNING("combined Particle created.");
    if (analysis::isSignal(part) < 0.5) {
      //part->print();
      //B2WARNING("mcMatching says No. (status: " << MCMatching::getMCTruthStatus(part, part->getRelated<MCParticle>()));
      continue;
    }
    //B2WARNING("passed");
    m_histogramSignal->Fill(m_var->function(part));
  }

}

void PreCutHistMakerModule::saveAllCombinations()
{
  StoreArray<Particle> particles;

  // Convert input ParticleList(s) to PCombinerList(s)
  vector<PCombinerList> inputPCombinerLists;
  for (unsigned i = 0; i < m_inputListNames.size(); i++) {
    StoreObjPtr<ParticleList> list(m_inputListNames[i]);
    PCombinerList plist;
    convert(list, plist);
    inputPCombinerLists.push_back(plist);
  }

  ParticleCombiner combiner(inputPCombinerLists, !hasAntiParticle(m_pdg));

  while (combiner.loadNext()) {
    const Particle part = combiner.getCurrentParticle(m_pdg, -m_pdg);
    m_histogramAll->Fill(m_var->function(&part));
  }
}

void PreCutHistMakerModule::event()
{
  vector<StoreObjPtr<ParticleList> > plists;
  std::multiset<int> expectedDaughterPDGsAbs;
  for (unsigned i = 0; i < m_inputListNames.size(); i++) {
    const string name = m_inputListNames[i];
    StoreObjPtr<ParticleList> list(name);
    if (!list) {
      B2ERROR("ParticleList " << name << " not found");
      return;
    }
    if (list->getParticleCollectionName() != std::string("Particles")) {
      B2ERROR("ParticleList " << name <<
              " does not refer to the default Particle collection");
      return;
    }
    plists.push_back(list);
    expectedDaughterPDGsAbs.insert(abs(list->getPDGCode()));

    //create temporary output lists (and replace any existing object)
    m_tmpLists[i].create(true);
    m_tmpLists[i]->initialize(list->getPDGCode(), list->getParticleListName());
    // TODO is at this point necessary to bind anti list? probably yes
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
        if (mcDaughters.size() < m_inputListNames.size()) {
          /*
          B2WARNING("size mismatch, discarding " << mcparticle.getIndex());
          for (auto m : mcDaughters) {
            B2WARNING("" << m->getPDG());
          }
          */
          continue;
        }

        clearParticleLists();

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

void PreCutHistMakerModule::convert(const StoreObjPtr<ParticleList>& in, PCombinerList& out)
{
  std::vector<int>     particles = in->getList(ParticleList::c_FlavorSpecificParticle);
  std::vector<int> antiParticles = in->getList(ParticleList::c_FlavorSpecificParticle, true);
  std::vector<int>   scParticles = in->getList(ParticleList::c_SelfConjugatedParticle);

  out.setPDG(in->getPDGCode());
  out.setList(PCombinerList::c_Particle,                  particles);
  out.setList(PCombinerList::c_AntiParticle,          antiParticles);
  out.setList(PCombinerList::c_SelfConjugatedParticle,  scParticles);
}

void PreCutHistMakerModule::terminate()
{
  saveHists();

  delete m_histogramAll;
  delete m_histogramSignal;
}

void PreCutHistMakerModule::writeHists()
{
  m_histogramSignal->Write();
  m_histogramAll->Write();
}
