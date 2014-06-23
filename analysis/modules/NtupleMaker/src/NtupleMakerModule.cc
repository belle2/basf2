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
  //Set module properties
  setDescription("Make a TTree with the properties of selected decay products. See https://belle2.cc.kek.jp/~twiki/bin/view/Physics/NtupleMaker for an introduction.");
  //Parameter definition
  addParam("fileName", m_fileName, "Name of ROOT file for output", string("test.root"));
  addParam("treeName", m_treeName, "Name of TTree to be filled.", string("test"));
  addParam("comment", m_comment, "Comment about the content of the TTree.", string("No comment."));
  addParam("tools", m_toolNames, "List of tools and decay descriptors. Available tools are described in https://belle2.cc.kek.jp/~twiki/bin/view/Physics/NtupleTool", vector<string>());
  addParam("listName", m_listName, "Name of particle list with reconstructed particles.", string(""));
}

void NtupleMakerModule::initialize()
{
  // Initializing the output root file
  if (!m_file) m_file = new TFile(m_fileName.c_str(), "RECREATE");
  if (!m_file->IsOpen()) {
    B2WARNING("Could not create file " << m_fileName);
    return;
  }

  m_file->cd();

  // check if TTree with that name already exists
  if (m_file->Get(m_treeName.c_str())) {
    B2WARNING("Tree with this name already exists: " << m_fileName);
    return;
  }

  m_tree = new TTree(m_treeName.c_str(), m_comment.c_str());
  m_nTrees++;

  B2INFO("NtupleMaker: Creating tree with name " << m_treeName << " for the ParticleList " << m_listName << " filled with the following tools:");
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

  // book two variables in the data store to save the number of
  //  candidates per event and the current candidate index
  StoreObjPtr< TParameter<Int_t> >::registerTransient(m_treeName + "_nCands");
  StoreObjPtr< TParameter<Int_t> >::registerTransient(m_treeName + "_iCand");
}

void NtupleMakerModule::event()
{
  // number of NtupleTools
  int nTools = m_tools.size();

  // Number of candidates in this event?
  StoreObjPtr< TParameter<int> > nCands(m_treeName + "_nCands");
  nCands.create();
  nCands->SetVal(-1);
  // candidate index?
  StoreObjPtr< TParameter<int> > iCand(m_treeName + "_iCand");
  iCand.create();
  iCand->SetVal(-1);

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

  nCands->SetVal(particlelist->getListSize());
  for (unsigned i = 0; i < particlelist->getListSize(); i++) {
    iCand->SetVal(i);
    const Particle* b = particlelist->getParticle(i);
    // loop over all NtupleTools and fill the tree
    for (int iTool = 0; iTool < nTools; ++iTool) m_tools[iTool].eval(b);
    m_tree->Fill();
  }
}

void NtupleMakerModule::terminate()
{
  B2INFO("Terminate TTree " << m_treeName);
  m_file->cd();
  m_tree->Write();
  m_nTrees--;
  B2INFO("Remaining unsaved Trees: " << m_nTrees);
  if (m_nTrees == 0) {
    B2INFO("Close file " << m_fileName);
    m_file->Close();
    m_file = NULL;
  }
}
