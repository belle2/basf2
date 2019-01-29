/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2019 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Xing-Yu Zhou                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/MCGenTopoToTree/MCGenTopoToTreeModule.h>
#include <framework/utilities/RootFileCreationManager.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/pcore/ProcHandler.h>

using namespace std;
using namespace Belle2;

// Register the module in the framework
REG_MODULE(MCGenTopoToTree)

MCGenTopoToTreeModule::MCGenTopoToTreeModule() : Module(), m_tree("", DataStore::c_Persistent)
{
  // Set the description
  setDescription("Creates and fills a tree for the topology information of MC generated particles. To be specific, the information involves the following three variables: nMCGen (number of MC generated particles in a given event), MCGenPDG (array of the PDG codes of MC generated particles in a given event), and MCGenMothIndex (array of the mother indexes of MC generated particles in a given event). Along with the variables, the event meta data, namely __experiment__, __run__, __event__, are always stored. In case a decayString is provided, __candidate__ and __ncandidates__ will be stored as well. MCGenPDG and MCGenMothIndex are not scalar variables but array variables with variable length equal to nMCGen. Hence, they can not be stored to flat ntuples with the module VariablesToNtuple. That's the reason why the module is developed. Note that the module is referred to as MCGenTopoToTree instead of MCGenTopoToNtuple, in order to avoid mistaking the tree as a flat ntuple with only scalar values stored. To keep consistent with the case of the module VariablesToNtuple: if a decayString is provided, one entry per candidate will be filled to the tree; otherwise, one entry per event will be filled to the tree.");

  // set the property flags
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  // Define Parameters
  addParam("particleList", m_particleList, "Particle list to be investigated", string(""));
  addParam("treeName", m_treeName, "Name of the tree for output", string("MCGenTopo"));
  addParam("fileName", m_fileName, "Name of the file for output", string("MCGenTopo.root"));
  addParam("nMCGenMax", m_nMCGenMax,
           "Maximum permissible number of MC generated particles in a given event. This number is used as the array length of the following three temporary variables in the event method: MCGenPDG, MCGenMothIndex and MCGenIndex, as well as the following two member variables in the initialize method: m_MCGenPDG and m_MCGenMothIndex. For details, please refer to the two methods of the module.",
           200);

  // Assign initial values to private member variables

  m_experiment = -1;
  m_run = -1;
  m_event = -1;
  m_candidate = -1;
  m_ncandidates = 0;
  m_nMCGen = 0;
  m_MCGenPDG = new int[m_nMCGenMax];
  m_MCGenMothIndex = new int[m_nMCGenMax];
}

void MCGenTopoToTreeModule::initialize()
{
  // Get the file if it already exists; otherwise, create the file.
  m_file =  RootFileCreationManager::getInstance().getFile(m_fileName);
  if (!m_file) {
    B2ERROR("Could not create file " << m_fileName <<
            "! Please set a vaild file name via the parameter \"filename\" of the function MCGenTopoToTree or variablesToNtuple_MCGenTopoToTree in the python steering file. And please check if enough disk space is available for creating the file.");
    return;
  }

  m_file->cd();

  // check if TTree with that name already exists. If yes, fatal the program with corresponding information.
  if (m_file->Get(m_treeName.c_str())) {
    B2FATAL("Tree with the name \"" << m_treeName << "\" already exists in the file \"" << m_fileName <<
            "\". Please check the file and tree names. If necessary, please revise one or both of them via the parameter \"fileName\" or \"treeName\" of the function MCGenTopoToTree or variablesToNtuple_MCGenTopoToTree in the python steering file.");
    return;
  }

  // Create the tree
  m_tree.registerInDataStore(m_fileName + m_treeName, DataStore::c_DontWriteOut);
  m_tree.construct(m_treeName.c_str(), "");
  m_tree->get().SetCacheSize(100000);


  // Create the branches.
  m_tree->get().Branch("__experiment__", &m_experiment, "__experiment__/I");
  m_tree->get().Branch("__run__", &m_run, "__run__/I");
  m_tree->get().Branch("__event__", &m_event, "__event__/I");
  // Note that only when the particle list is specified, the branches for the variables m_candidate and m_ncandidates are created.
  if (!m_particleList.empty()) {
    m_tree->get().Branch("__candidate__",  &m_candidate,  "__candidate__/I");
    m_tree->get().Branch("__ncandidates__", &m_ncandidates, "__ncandidates__/I");
  }
  m_tree->get().Branch("nMCGen", &m_nMCGen, "nMCGen/I");
  m_tree->get().Branch("MCGenPDG", &m_MCGenPDG[0], "MCGenPDG[nMCGen]/I");
  m_tree->get().Branch("MCGenMothIndex", &m_MCGenMothIndex[0], "MCGenMothIndex[nMCGen]/I");
}

void MCGenTopoToTreeModule::event()
{
  // Get m_experiment, m_run and m_event
  StoreObjPtr<EventMetaData> eventmetadata;
  if (eventmetadata) {
    m_experiment = eventmetadata->getExperiment();
    m_run = eventmetadata->getRun();
    m_event = eventmetadata->getEvent();
  } else {
    m_experiment = -1;
    m_run = -1;
    m_event = -1;
  }

  // Get m_nMCGen, MCGenPDG and MCGenMothIndex ********** BEGIN **********
  StoreArray<MCParticle> mcParticles("MCParticles");

  // Get the desired quantities from the MCParticles and store them to the temporary variables, and meanwhile build the relationships between the array indexes in the member variables of the class and those in the MCParticles.
  int* MCGenPDG = new int[m_nMCGenMax];
  int* MCGenMothIndex = new int[m_nMCGenMax];
  int MCGenindex = 0;
  int* MCGenIndex = new int[m_nMCGenMax];

  for (const auto& mcParticle : mcParticles) {
    if (mcParticle.getArrayIndex() < m_nMCGenMax) {
      if (mcParticle.hasStatus(MCParticle::c_PrimaryParticle) && (!mcParticle.hasStatus(MCParticle::c_IsVirtual))
          && (!mcParticle.hasStatus(MCParticle::c_Initial))) {
        MCGenPDG[mcParticle.getArrayIndex()] = mcParticle.getPDG();
        MCParticle* mother = mcParticle.getMother();
        if (mother) MCGenMothIndex[mcParticle.getArrayIndex()] = mother->getArrayIndex();
        else MCGenMothIndex[mcParticle.getArrayIndex()] = -1;
        MCGenIndex[mcParticle.getArrayIndex()] = MCGenindex;
        MCGenindex++;
      } else MCGenIndex[mcParticle.getArrayIndex()] = -1;
    } else {
      B2WARNING("The size of the mcParticles in the event is " << mcParticles.getEntries() <<
                ", which goes beyond the scope of the following three arrays: MCGenIndex, MCGenPDG and MCGenMothIndex, whose sizes are only " <<
                m_nMCGenMax <<
                "! To avoid the problem, please set the parameter nMCGenMax of the function MCGenTopoToTree or variablesToNtuple_MCGenTopoToTree at a large enough value, for example, "
                << mcParticles.getEntries() + 10 << " in the python steering file.");
      break;
    }
  }

  // Store the desired quantities to the member variables of the class.
  m_nMCGen = MCGenindex;
  int ArrayIndexMax = mcParticles.getEntries() < m_nMCGenMax ? mcParticles.getEntries() : m_nMCGenMax;
  for (int ArrayIndex = 0; ArrayIndex < ArrayIndexMax; ArrayIndex++) {
    if (MCGenIndex[ArrayIndex] != -1) {
      MCGenindex = MCGenIndex[ArrayIndex];
      m_MCGenPDG[MCGenindex] = MCGenPDG[ArrayIndex];
      if (MCGenMothIndex[ArrayIndex] != ArrayIndex
          && MCGenIndex[MCGenMothIndex[ArrayIndex]] != -1) m_MCGenMothIndex[MCGenindex] = MCGenIndex[MCGenMothIndex[ArrayIndex]];
      else m_MCGenMothIndex[MCGenindex] = -1;
    }
  }
  // Get m_nMCGen, MCGenPDG and MCGenMothIndex ********** END **********

  // If no particle list is specified, just fill the tree. If the particle list is specified, get m_ncandidates and m_candidate, and then fill the tree.
  if (m_particleList.empty()) {
    m_tree->get().Fill();
  } else {
    StoreObjPtr<ParticleList> particlelist(m_particleList);
    if (!particlelist) {
      B2WARNING("ParticleList " << m_particleList << " not found.");
      B2WARNING("Event not written to the tree.");
      return;
    }
    m_ncandidates = particlelist->getListSize();
    for (int i = 0; i < m_ncandidates; i++) {
      m_candidate = i;
      m_tree->get().Fill();
    }
  }
}

void MCGenTopoToTreeModule::terminate()
{
  if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
    B2INFO("Writing tree " << m_treeName << ".");
    m_file->cd();
    m_tree->write(m_file.get());
    const bool writeError = m_file->TestBit(TFile::kWriteError);
    if (writeError) {
      m_file.reset();
      B2FATAL("A write error occured while saving \"" << m_treeName  << "\" to \"" << m_fileName  <<
              "\", please check if enough disk space is available.");
    } else m_file.reset();
  }
}
