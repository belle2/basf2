/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/modules/arichToNtuple/arichToNtupleModule.h>

#include <arich/dataobjects/ARICHLikelihood.h>
#include <arich/dataobjects/ARICHAeroHit.h>
#include <arich/dataobjects/ARICHTrack.h>
#include <arich/dataobjects/ARICHPhoton.h>
#include <arich/dataobjects/ARICHHit.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>

// analysis
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>

// framework
#include <framework/logging/Logger.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/core/ModuleParam.templateDetails.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

// framework - root utilities
#include <framework/utilities/MakeROOTCompatible.h>
#include <framework/utilities/RootFileCreationManager.h>

#include <cmath>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(arichToNtuple)


arichToNtupleModule::arichToNtupleModule() :
  Module(), m_tree("", DataStore::c_Persistent)
{
  //Set module properties
  setDescription("Local arich extension of VariblesToNtuple module to append detailed arich information to reconstructed candidates in the analysis output Ntuple. The TNtuple is candidate-based, meaning that the variables of each candidate are saved separate rows.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  vector<string> emptylist;
  addParam("particleList", m_particleList,
           "Name of particle list with reconstructed particles. If no list is provided the variables are saved once per event (only possible for event-type variables)",
           std::string(""));
  addParam("arichSelector", m_arichSelector,
           "Decay string with selected particles to which arich info should be appened", std::string(""));
  addParam("variables", m_variables,
           "List of variables (or collections) to save. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector.",
           emptylist);
  addParam("arichVariables", m_arichVariables,
           "List of aliases for particles to which arich info will be appended (used for tree branch names)",
           emptylist);

  addParam("fileName", m_fileName, "Name of ROOT file for output.", string("arichToNtuple.root"));
  addParam("treeName", m_treeName, "Name of the NTuple in the saved file.", string("ntuple"));

  std::tuple<std::string, std::map<int, unsigned int>> default_sampling{"", {}};
  addParam("sampling", m_sampling,
           "Tuple of variable name and a map of integer values and inverse sampling rate. E.g. (signal, {1: 0, 0:10}) selects all signal candidates and every 10th background candidate.",
           default_sampling);
}

void arichToNtupleModule::initialize()
{
  m_eventMetaData.isRequired();

  StoreArray<Track> tracks;
  tracks.isRequired();
  StoreArray<ExtHit> extHits;
  extHits.isRequired();
  StoreArray<ARICHTrack> arichTracks;
  arichTracks.isRequired();


  if (not m_particleList.empty())
    StoreObjPtr<ParticleList>().isRequired(m_particleList);

  m_decaydescriptor.init(m_arichSelector);

  // Initializing the output root file
  if (m_fileName.empty()) {
    B2FATAL("Output root file name is not set. Please set a vaild root output file name (\"fileName\" module parameter).");
  }
  // See if there is already a file in which case add a new tree to it ...
  // otherwise create a new file (all handled by framework)
  m_file =  RootFileCreationManager::getInstance().getFile(m_fileName);
  if (!m_file) {
    B2ERROR("Could not create file \"" << m_fileName <<
            "\". Please set a vaild root output file name (\"fileName\" module parameter).");
    return;
  }

  TDirectory::TContext directoryGuard(m_file.get());

  // check if TTree with that name already exists
  if (m_file->Get(m_treeName.c_str())) {
    B2FATAL("Tree with the name \"" << m_treeName
            << "\" already exists in the file \"" << m_fileName << "\"\n"
            << "\nYou probably want to either set the output fileName or the treeName to something else:\n\n"
            << "   from modularAnalysis import arichToNtuple\n"
            << "   arichToNtuple('pi+:all', ['p'], treename='pions', filename='arichToNtuple.root')\n"
            << "   arichToNtuple('gamma:all', ['p'], treename='photons', filename='arichToNtuple.root') # two trees, same file\n"
            << "\n == Or ==\n"
            << "   from modularAnalysis import arichToNtuple\n"
            << "   arichToNtuple('pi+:all', ['p'], filename='pions.root')\n"
            << "   arichToNtuple('gamma:all', ['p'], filename='photons.root') # two files\n"
           );
    return;
  }

  // set up tree and register it in the datastore
  m_tree.registerInDataStore(m_fileName + m_treeName, DataStore::c_DontWriteOut);
  m_tree.construct(m_treeName.c_str(), "");
  m_tree->get().SetCacheSize(100000);

  // declare counter branches - pass through variable list, remove counters added by user
  m_tree->get().Branch("__experiment__", &m_experiment, "__experiment__/I");
  m_tree->get().Branch("__run__", &m_run, "__run__/I");
  m_tree->get().Branch("__event__", &m_event, "__event__/I");
  if (not m_particleList.empty()) {
    m_tree->get().Branch("__candidate__", &m_candidate, "__candidate__/I");
    m_tree->get().Branch("__ncandidates__", &m_ncandidates, "__ncandidates__/I");
  }
  for (const auto& variable : m_variables)
    if (Variable::isCounterVariable(variable)) {
      B2WARNING("The counter '" << variable
                << "' is handled automatically by arichToNtuple, you don't need to add it.");
    }

  // declare branches and get the variable strings
  m_variables = Variable::Manager::Instance().resolveCollections(m_variables);
  m_branchAddresses.resize(m_variables.size() + 1);
  m_tree->get().Branch("__weight__", &m_branchAddresses[0], "__weight__/D");
  size_t enumerate = 1;
  for (const string& varStr : m_variables) {

    string branchName = makeROOTCompatible(varStr);
    m_tree->get().Branch(branchName.c_str(), &m_branchAddresses[enumerate], (branchName + "/D").c_str());

    // also collection function pointers
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(varStr);
    if (!var) {
      B2ERROR("Variable '" << varStr << "' is not available in Variable::Manager!");
    } else {
      m_functions.push_back(var->function);
    }
    enumerate++;
  }

  // add arich related branches
  for (const string& varStr : m_arichVariables) {
    string branchName = makeROOTCompatible(varStr);
    addARICHBranches(branchName);
  }

  m_tree->get().SetBasketSize("*", 1600);

  m_sampling_name = std::get<0>(m_sampling);
  m_sampling_rates = std::get<1>(m_sampling);

  if (m_sampling_name != "") {
    m_sampling_variable = Variable::Manager::Instance().getVariable(m_sampling_name);
    if (m_sampling_variable == nullptr) {
      B2FATAL("Couldn't find sample variable " << m_sampling_name << " via the Variable::Manager. Check the name!");
    }
    for (const auto& pair : m_sampling_rates)
      m_sampling_counts[pair.first] = 0;
  } else {
    m_sampling_variable = nullptr;
  }
}


float arichToNtupleModule::getInverseSamplingRateWeight(const Particle* particle)
{
  if (m_sampling_variable == nullptr)
    return 1.0;

  long target = std::lround(m_sampling_variable->function(particle));
  if (m_sampling_rates.find(target) != m_sampling_rates.end() and m_sampling_rates[target] > 0) {
    m_sampling_counts[target]++;
    if (m_sampling_counts[target] % m_sampling_rates[target] != 0)
      return 0;
    else {
      m_sampling_counts[target] = 0;
      return m_sampling_rates[target];
    }
  }
  return 1.0;
}

void arichToNtupleModule::event()
{
  m_event = m_eventMetaData->getEvent();
  m_run = m_eventMetaData->getRun();
  m_experiment = m_eventMetaData->getExperiment();

  if (m_particleList.empty()) {
    m_branchAddresses[0] = getInverseSamplingRateWeight(nullptr);
    if (m_branchAddresses[0] > 0) {
      for (unsigned int iVar = 0; iVar < m_variables.size(); iVar++) {
        m_branchAddresses[iVar + 1] = m_functions[iVar](nullptr);
      }
      for (auto& arich : m_arich) arich->clear();
      m_tree->get().Fill();
    }

  } else {
    StoreObjPtr<ParticleList> particlelist(m_particleList);
    m_ncandidates = particlelist->getListSize();

    for (unsigned int iPart = 0; iPart < m_ncandidates; iPart++) {
      m_candidate = iPart;
      const Particle* particle = particlelist->getParticle(iPart);
      m_branchAddresses[0] = getInverseSamplingRateWeight(particle);
      if (m_branchAddresses[0] > 0) {
        for (unsigned int iVar = 0; iVar < m_variables.size(); iVar++) {
          m_branchAddresses[iVar + 1] = m_functions[iVar](particle);
        }
        for (auto& arich : m_arich) arich->clear();
        fillARICHTree(particle);
        m_tree->get().Fill();
      }
    }
  }
}

void arichToNtupleModule::terminate()
{
  if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
    B2INFO("Writing NTuple " << m_treeName);
    TDirectory::TContext directoryGuard(m_file.get());
    m_tree->write(m_file.get());

    const bool writeError = m_file->TestBit(TFile::kWriteError);
    m_file.reset();
    if (writeError) {
      B2FATAL("A write error occured while saving '" << m_fileName  << "', please check if enough disk space is available.");
    }
  }
}

void arichToNtupleModule::addARICHBranches(const std::string& name)
{

  ARICH::ARICHTree* tree = new ARICH::ARICHTree();
  m_arich.push_back(tree);
  m_tree->get().Branch((name + "_detPhot").c_str(),  &tree->detPhot,  "detPhot/I");
  m_tree->get().Branch((name + "_expPhot").c_str(),  &tree->expPhot,  "e/F:mu:pi:K:p:d");
  m_tree->get().Branch((name + "_logL").c_str(),  &tree->logL,  "e/F:mu:pi:K:p:d");
  m_tree->get().Branch((name + "_recHit").c_str(), &tree->recHit,  "PDG/I:x/F:y:z:p:theta:phi");
  m_tree->get().Branch((name + "_mcHit").c_str(), &tree->mcHit,  "PDG/I:x/F:y:z:p:theta:phi");
  m_tree->get().Branch((name + "_winHit").c_str(),  &tree->winHit,  "x/F:y");
  m_tree->get().Branch((name + "_photons").c_str(), "std::vector<Belle2::ARICHPhoton>", &tree->photons);

}

void arichToNtupleModule::fillARICHTree(const Particle* particle)
{

  std::vector<const Particle*> selParticles = m_decaydescriptor.getSelectionParticles(particle);
  int iTree = 0;
  for (auto p : selParticles) {
    const Track* track = p->getTrack();
    if (!track) continue;
    for (const ExtHit& hit : track->getRelationsTo<ExtHit>()) {
      const ARICHTrack* atrk = hit.getRelated<ARICHTrack>();
      if (!atrk) continue;

      if (atrk->hitsWindow()) {
        TVector2 winHit = atrk->windowHitPosition();
        m_arich[iTree]->winHit[0] = winHit.X();
        m_arich[iTree]->winHit[1] = winHit.Y();
      }

      m_arich[iTree]->photons = atrk->getPhotons();

      TVector3 recPos = atrk->getPosition();
      m_arich[iTree]->recHit.x = recPos.X();
      m_arich[iTree]->recHit.y = recPos.Y();
      m_arich[iTree]->recHit.z = recPos.Z();

      TVector3 recMom = atrk->getDirection() * atrk->getMomentum();
      m_arich[iTree]->recHit.p = recMom.Mag();
      m_arich[iTree]->recHit.theta = recMom.Theta();
      m_arich[iTree]->recHit.phi = recMom.Phi();

      const ARICHLikelihood* lkh = NULL;
      lkh = track->getRelated<ARICHLikelihood>();
      if (lkh) {
        m_arich[iTree]->logL.e = lkh->getLogL(Const::electron);
        m_arich[iTree]->logL.mu = lkh->getLogL(Const::muon);
        m_arich[iTree]->logL.pi = lkh->getLogL(Const::pion);
        m_arich[iTree]->logL.K = lkh->getLogL(Const::kaon);
        m_arich[iTree]->logL.p = lkh->getLogL(Const::proton);
        m_arich[iTree]->logL.d = lkh->getLogL(Const::deuteron);

        m_arich[iTree]->expPhot.e = lkh->getExpPhot(Const::electron);
        m_arich[iTree]->expPhot.mu = lkh->getExpPhot(Const::muon);
        m_arich[iTree]->expPhot.pi = lkh->getExpPhot(Const::pion);
        m_arich[iTree]->expPhot.K = lkh->getExpPhot(Const::kaon);
        m_arich[iTree]->expPhot.p = lkh->getExpPhot(Const::proton);
        m_arich[iTree]->expPhot.d = lkh->getExpPhot(Const::deuteron);

        m_arich[iTree]->detPhot = lkh->getDetPhot();
      }

      const ARICHAeroHit* aeroHit = atrk->getRelated<ARICHAeroHit>();
      if (aeroHit) {
        TVector3 truePos = aeroHit->getPosition();
        m_arich[iTree]->mcHit.x = truePos.X();
        m_arich[iTree]->mcHit.y = truePos.Y();
        m_arich[iTree]->mcHit.z = truePos.Z();

        TVector3 trueMom = aeroHit->getMomentum();
        m_arich[iTree]->mcHit.p = trueMom.Mag();
        m_arich[iTree]->mcHit.theta = trueMom.Theta();
        m_arich[iTree]->mcHit.phi = trueMom.Phi();
        m_arich[iTree]->mcHit.PDG = aeroHit->getPDG();

      }
    }
    iTree++;
  }
}
