/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/NtupleMaker/NtupleMakerModule.h>
#include <analysis/NtupleTools/NtupleToolList.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/utility/AnalysisConfiguration.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <cmath>
#include <algorithm>
#include <TParameter.h>
using namespace std;
using namespace Belle2;
using namespace boost::algorithm;

// Register module in the framework
REG_MODULE(NtupleMaker)

TFile* NtupleMakerModule::m_file = NULL;
int NtupleMakerModule::m_nTrees = 0;

NtupleMakerModule::NtupleMakerModule() : Module()
{
  m_tree = NULL;
  m_nCands = 0;
  m_iCand = 0;

  //Set module properties
  setDescription("Make a TTree with the properties of selected decay products. See https://confluence.desy.de/display/BI/Physics+NtupleMaker for an introduction.");
  //Parameter definition
  addParam("fileName", m_fileName, "Name of ROOT file for output", string(""));
  addParam("treeName", m_treeName, "Name of TTree to be filled.", string(""));
  addParam("comment", m_comment, "Comment about the content of the TTree.", string(""));
  addParam("tools", m_toolNames,
           "List of tools and decay descriptors. Available tools are described in https://confluence.desy.de/display/BI/Physics+NtupleTool",
           vector<string>());
  addParam("listName", m_listName, "Name of particle list with reconstructed particles.", string(""));
}

void NtupleMakerModule::initialize()
{
  if (m_fileName.empty() && m_treeName.empty()) {
    B2FATAL("Invalid inputs. Please set a vaild root output file name (fileName) or valid TTree name (treeName) module parameters.");
  }

  // Initializing the output root file. There can only be one NtupleMaker output
  // file so this module can:
  //
  // 1. open a file (filenName not empty, treeName is empty)
  // 2. create a tree in the already opened file (filename is empty, treeName not empty)
  // 3. both at the same time (fileName not empty, treeName not empty)
  //
  // in case of 2. this only works if another instance already opened the file.
  if (!m_file) {
    // No one tried to open a file yet so this needs to be case 1. or 3.
    if (m_fileName.empty()) {
      B2ERROR("Cannot create Ntuple tree: No ntuple file specified so far. Please call ntupleFile() "
              "before calling ntupleTree() or specify a valid fileName parameter if using NtupleMaker directly");
      return;
    }
    // Ok we have a filename, but can we actually create a file there?
    m_file = new TFile(m_fileName.c_str(), "RECREATE");
    if (!m_file->IsOpen()) {
      B2ERROR("Could not create file \"" << m_fileName << "\". Please set a vaild ntuple output file name (fileName)");
      return;
    }
    // everything setup
  } else if (!m_fileName.empty()) {
    // we already have tried to open a file but we have a filename? At least print a warning
    B2WARNING("There is already an open Ntuple file (" << m_file->GetName() <<
              "), ignoring additional request to create " << m_fileName);
  }

  // Ok, we made sure there is at least a valid TFile* at m_file but it might
  // not be open. In that case or if we don't have a tree name we do nothing:
  // we already reported any kind of errors above
  if (!m_file->IsOpen() || m_treeName.empty())
    return;

  // Finally everything is setup: create the TTree ...
  m_file->cd();

  // check if TTree with that name already exists
  if (m_file->Get(m_treeName.c_str())) {
    B2WARNING("Tree with this name already exists: " << m_fileName);
    return;
  }

  m_tree = new TTree(m_treeName.c_str(), m_comment.c_str());
  m_nTrees++;

  B2INFO("NtupleMaker: Creating tree with name " << m_treeName << " for the ParticleList " << m_listName <<
         " filled with the following tools:");
  int nTools = m_toolNames.size();
  for (int iTool =  0; iTool < (nTools - 1); iTool = iTool + 2) {
    // Create decay descriptors with selected particles from string
    DecayDescriptor desc;
    bool isStringOK = desc.init(m_toolNames[iTool + 1]);
    if (!isStringOK) B2ERROR("Could not initialise decay descriptor!" << m_toolNames[iTool + 1]);
    m_decaydescriptors.push_back(desc);

    // select ntuple tools to be used
    NtupleFlatTool* ntool = NtupleToolList::create(m_toolNames[iTool], m_tree, m_decaydescriptors.back());
    if (ntool) m_tools.push_back(ntool);
  }
  B2INFO(" The branch name style is going to be: " + AnalysisConfiguration::instance()->getTupleStyle());


  // Add by default 2 Branches to each tree
  // number of candidates in the event
  // candidate index
  m_tree->Branch("nCands", &m_nCands, "m_nCands/I");
  m_tree->Branch("iCand",  &m_iCand,  "m_iCand/I");
}

void NtupleMakerModule::event()
{
  if (m_treeName.empty())
    return;

  m_nCands = 0;
  m_iCand  = 0;

  // number of NtupleTools
  int nTools = m_tools.size();

  // If no particle list name is specified, just call every
  // ntuple Tool with a NULL pointer for the particle argument
  // (useful if you want to save event information for every event)
  if (m_listName.empty()) {
    for (int iTool = 0; iTool < nTools; ++iTool)
      m_tools[iTool].eval(NULL);
    m_tree->Fill();
    return;
  }

  StoreObjPtr<ParticleList> particlelist(m_listName);
  if (!particlelist) {
    B2WARNING("ParticleList " << m_listName << " not found");
    B2WARNING("Event not written to the ntuple!");
    return;
  }

  m_nCands = particlelist->getListSize();
  for (unsigned i = 0; i < particlelist->getListSize(); i++) {
    m_iCand = int(i);
    const Particle* b = particlelist->getParticle(i);
    // loop over all NtupleTools and fill the tree
    for (int iTool = 0; iTool < nTools; ++iTool) m_tools[iTool].eval(b);
    m_tree->Fill();
  }
}

void NtupleMakerModule::terminate()
{
  if (m_nTrees == 0) {
    B2INFO("Close file " << m_fileName);
    m_file->Close();
    m_file = NULL;
    return;
  }

  B2INFO("Terminate TTree " << m_treeName);
  m_file->cd();
  m_tree->Write();
  m_nTrees--;
  B2INFO("Remaining unsaved Trees: " << m_nTrees);
}
