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
#include <analysis/modules/NtupleMaker/NtupleKinematicsTool.h>
#include <analysis/modules/NtupleMaker/NtupleEventMetaDataTool.h>
#include <analysis/modules/NtupleMaker/NtupleDeltaEMbcTool.h>
#include <analysis/dataobjects/ParticleList.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <cmath>
#include <algorithm>
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
  setDescription("Make a TTree with the properties of seleted decay products.");
  //Parameter definition
  addParam("strFileName", m_strFileName, "Name of ROOT file for output", string("test.root"));
  addParam("strTreeName", m_strTreeName, "Name of TTree to be filled.", string("test"));
  addParam("strComment", m_strComment, "Comment about the content of the TTree.", string("No comment."));
  addParam("strTools", m_strTools, "List of tools and decay descriptors", vector<string>());
  addParam("strListName", m_strListName, "Name of particle list with reconstructed particles.", string(""));
}

void NtupleMakerModule::initialize()
{
  // Initializing the output root file
  if (!m_file) m_file = new TFile(m_strFileName.c_str(), "NEW");
  if (!m_file->IsOpen()) {
    B2WARNING("Could not create file " << m_strFileName);
    return;
  }

  m_file->cd();

  // check if TTree with that name already exists
  if (m_file->Get(m_strTreeName.c_str())) {
    B2WARNING("Tree with this name already exists: " << m_strFileName);
    return;
  }

  m_tree = new TTree(m_strTreeName.c_str(), m_strComment.c_str());
  m_nTrees++;

  if (m_strListName.empty()) B2ERROR("No name of list with reconstructed particles provided.");

  int nTools = m_strTools.size();
  for (int iTool =  0; iTool < (nTools - 1); iTool = iTool + 2) {
    // Create decay descriptors with selected particles from string
    DecayDescriptor desc;
    bool isStringOK = desc.init(m_strTools[iTool + 1]);
    if (!isStringOK) B2ERROR("Could not initialise decay descriptor!" << m_strTools[iTool + 1]);
    m_decaydescriptors.push_back(desc);

    // select ntuple tools to be used
    if (m_strTools[iTool].compare("NtupleKinematicsTool") == 0) {
      m_tools.push_back(new NtupleKinematicsTool(m_tree, m_decaydescriptors.back()));
    } else if (m_strTools[iTool].compare("NtupleEventMetaDataTool") == 0) {
      m_tools.push_back(new NtupleEventMetaDataTool(m_tree, m_decaydescriptors.back()));
    } else if (m_strTools[iTool].compare("NtupleDeltaEMbcTool") == 0) {
      m_tools.push_back(new NtupleDeltaEMbcTool(m_tree, m_decaydescriptors.back()));
    } else {
      B2WARNING("The specified NtupleTool is not available!");
    }
  }
}

void NtupleMakerModule::event()
{
  StoreObjPtr<ParticleList> particlelist(m_strListName);
  if (!particlelist) {
    B2ERROR("ParticleList " << m_strListName << " not found");
    return;
  }

  for (unsigned i = 0; i < particlelist->getListSize(); i++) {
    const Particle* b = particlelist->getParticle(i);
    // loop over all NtupleTools and fill the tree
    int nTools = m_tools.size();
    for (int iTool = 0; iTool < nTools; ++iTool) {
      m_tools[iTool].eval(b);
    }        m_tree->Fill();
  }
}

void NtupleMakerModule::terminate()
{
  B2INFO("Terminate TTree " << m_strTreeName);
  B2INFO("Unsaved Trees: " << m_nTrees);
  m_file->cd();
  m_tree->Write();
  m_nTrees--;
  if (m_nTrees == 0) {
    m_file->Close();
    m_file = NULL;
  }
}
