/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/ROFBuilderModule.h>

#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>

#include <map>
#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ROFBuilder)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ROFBuilderModule::ROFBuilderModule() : Module()
{
  //Set module properties
  setDescription("Generates a ROOT file containing the readout frames for a single subdetector and a specific background component and generator.");

  addParam("Subdetector", m_subdetector, "The subdetector for which the ROF Root file is created (1 = PXD, 2 = SVD).");
  addParam("SimHitCollectionName", m_simHitCollectionName, "The name of the SimHit collection.");
  addParam("SimHitMCPartRelationName", m_simHitMCPartRelationName, "The SimHit to MCParticle relation name.");
  addParam("EventsPerReadoutFrame", m_eventsPerReadoutFrame, "The number of events that represent one readout frame.");
  addParam("OutputRootFileName", m_outputRootFileName, "The name of the ROF ROOT output file.");
  addParam("ComponentName", m_componentName, "The name of the background component (e.g. Touschek).");
  addParam("GeneratorName", m_generatorName, "The name of the background generator (e.g. SAD_LER).");
  addParam("MCParticleWriteMode", m_mcParticleWriteMode, "The MCParticle write mode.", 0);

  //Create Subdetector class name list
  m_SimHitClassNames.push_back("None");
  m_SimHitClassNames.push_back(PXDSimHit::Class_Name());
  m_SimHitClassNames.push_back(SVDSimHit::Class_Name());
}


void ROFBuilderModule::initialize()
{
  //Create output Root file
  m_outputRootFile = new TFile(m_outputRootFileName.c_str(), "recreate");
  m_rofTree        = new TTree("ROFTree", "The readout frames");
  m_contentTree    = new TTree("ContentTree", "The file content");
  m_readoutFrame   = new TClonesArray(m_SimHitClassNames[m_subdetector].c_str());
  m_mcParticles    = new TClonesArray(MCParticle::Class_Name());
  m_mcPartRels     = new TClonesArray(RelationElement::Class_Name());

  //The readout frame tree
  m_rofTree->Branch("ReadoutFrames", &m_readoutFrame);
  m_rofTree->Branch("MCParticles", &m_mcParticles);
  m_rofTree->Branch("MCPartRels",  &m_mcPartRels);

  //The content tree (a tree is overkill here, but it avoids writing a specific class for the content)
  m_contentTree->Branch("Subdetector", &m_subdetector, "SBD/I");
  m_contentTree->Branch("Component", &m_componentName);
  m_contentTree->Branch("Generator", &m_generatorName);
  m_contentTree->Branch("SimHitCollection", &m_simHitCollectionName);
  m_contentTree->Branch("SimHitRelation", &m_simHitMCPartRelationName);
  m_contentTree->Branch("MCParticleWriteMode", &m_mcParticleWriteMode, "MCW/I");
  m_contentTree->Fill();

  //Start a new readout frame
  m_rofMCParticleGraph.clear();
  m_mcpToSimHitMap.clear();
  m_currReadoutFrameIdx = 0;
  m_event = 0;
  m_numberSimHits = 0;
  m_rofGraphUniqueID = -1;

  map<int, string> writeModeLabels;
  writeModeLabels.insert(make_pair(0, "seen in the subdetector"));
  writeModeLabels.insert(make_pair(1, "seen in the subdetector + mothers"));
  writeModeLabels.insert(make_pair(2, "all"));

  B2INFO("=======================================================")
  B2INFO("                    ROFBuilder                         ")
  B2INFO("-------------------------------------------------------")
  B2INFO("Events per ROF       : " << m_eventsPerReadoutFrame)
  B2INFO("Output file          : " << m_outputRootFileName)
  B2INFO("Subdetector SimHits  : " << m_SimHitClassNames[m_subdetector])
  B2INFO("Component            : " << m_componentName)
  B2INFO("Generator            : " << m_generatorName)
  B2INFO("MCParticle Write Mode: " << writeModeLabels[m_mcParticleWriteMode])
  B2INFO("-------------------------------------------------------")
}


void ROFBuilderModule::event()
{
  //Check if a new readout frame has to be created
  if (m_event >= ((m_currReadoutFrameIdx + 1) * m_eventsPerReadoutFrame)) {
    fillMCParticleROFTree();
    B2INFO(">> Save readout frame: #" << m_currReadoutFrameIdx << " (" << m_numberSimHits << " SimHits" << ")")

    //Clear the TClones Arrays
    m_readoutFrame->Clear();
    m_mcParticles->Clear();
    m_mcPartRels->Clear();

    m_numberSimHits = 0;
  }

  //Depending on the subdetector, add the appropriate SimHits to the ROF
  switch (m_subdetector) {
    case 1 : addSimHitsToROF<PXDSimHit>();
      break;
    case 2 : addSimHitsToROF<SVDSimHit>();
      break;
  }

  m_event++;
}


void ROFBuilderModule::terminate()
{
  //Check if a new readout frame has to be created
  if (m_event >= ((m_currReadoutFrameIdx + 1) * m_eventsPerReadoutFrame)) {
    fillMCParticleROFTree();
    B2INFO(">> Save readout frame: #" << m_currReadoutFrameIdx << " (" << m_numberSimHits << " SimHits" << ")")
  }
  B2INFO("=======================================================")

  m_outputRootFile->WriteObject(m_rofTree, m_rofTree->GetName());
  m_outputRootFile->WriteObject(m_contentTree, m_contentTree->GetName());
  m_outputRootFile->Close();

  delete m_mcPartRels;
  delete m_mcParticles;
  delete m_readoutFrame;
}


void ROFBuilderModule::fillMCParticleROFTree()
{
  //Fill the MCParticles
  m_rofMCParticleGraph.generateList("ROFBuilderMCParticleROF");
  StoreArray<MCParticle> mcParticleEventROF("ROFBuilderMCParticleROF");

  int nParticles = mcParticleEventROF.getEntries();
  for (int iParticle = 0; iParticle < nParticles; ++iParticle) {
    new((*m_mcParticles)[iParticle]) MCParticle(*mcParticleEventROF[iParticle]);
  }

  //Create a list that relates the unique identifier of a MCParticleGraph node (list index)
  //with the index of the MCParticle in the final list
  vector<int> uniqueIDMCPartFinal;
  uniqueIDMCPartFinal.resize(m_rofMCParticleGraph.size());
  for (unsigned int iPart = 0; iPart < m_rofMCParticleGraph.size(); ++iPart) {
    MCParticleGraph::GraphParticle &currParticle = m_rofMCParticleGraph[iPart];
    if (currParticle.getIndex() > 0) {
      uniqueIDMCPartFinal[currParticle.getTrackID()] = currParticle.getArrayIndex();
    }
  }

  //Fill the MCParticles to SimHit Relation
  for (unsigned int iRel = 0; iRel < m_mcpToSimHitMap.size(); ++iRel) {
    if (m_mcpToSimHitMap[iRel] < 0) continue;
    int mcpIndex = uniqueIDMCPartFinal[m_mcpToSimHitMap[iRel]];
    if (mcpIndex < 0) continue;
    new((*m_mcPartRels)[m_mcPartRels->GetLast() + 1]) RelationElement(mcpIndex, iRel);
  }

  //Fill the ROOT tree
  m_rofTree->Fill();
  m_currReadoutFrameIdx++;

  //Reset the ROF variables
  m_rofMCParticleGraph.clear();
  m_mcpToSimHitMap.clear();
  m_rofGraphUniqueID = -1;
}


MCParticleGraph::GraphParticle& ROFBuilderModule::createGraphParticle(MCParticleGraph &graph, MCParticle &mcParticle, int motherIndex)
{
  MCParticleGraph::GraphParticle& graphParticle = graph.addParticle();
  graphParticle.setPDG(mcParticle.getPDG());
  graphParticle.setStatus(mcParticle.getStatus());
  graphParticle.setMass(mcParticle.getMass());
  graphParticle.setCharge(mcParticle.getCharge());
  graphParticle.setEnergy(mcParticle.getEnergy());
  graphParticle.setValidVertex(mcParticle.hasValidVertex());
  graphParticle.setProductionTime(mcParticle.getProductionTime());
  graphParticle.setDecayTime(mcParticle.getDecayTime());
  graphParticle.setProductionVertex(mcParticle.getProductionVertex());
  graphParticle.setMomentum(mcParticle.getMomentum());
  graphParticle.setDecayVertex(mcParticle.getDecayVertex());
  graphParticle.setFirstDaughter(mcParticle.getFirstDaughter());
  graphParticle.setLastDaughter(mcParticle.getLastDaughter());
  if (motherIndex > 0) graphParticle.comesFrom(graph[motherIndex-1]); //Add decay

  return graphParticle;
}


void ROFBuilderModule::addParticleToEventGraph(MCParticleGraph &graph, MCParticle &mcParticle, int motherIndex, const std::vector<bool> &keepList)
{
  MCParticleGraph::GraphParticle& graphParticle = createGraphParticle(graph, mcParticle, motherIndex);

  //Keep/Ignore particles based on the keep list
  graphParticle.setTrackID(mcParticle.getArrayIndex());
  graphParticle.setIgnore(!keepList[mcParticle.getArrayIndex()]);

  //Add all children
  int currMotherIndex = graph.size();
  BOOST_FOREACH(MCParticle* daughter, mcParticle.getDaughters()) {
    addParticleToEventGraph(graph, *daughter, currMotherIndex, keepList);
  }
}


void ROFBuilderModule::addParticleToROFGraph(MCParticle &mcParticle, int motherIndex, std::vector<int> &uniqueIDList)
{
  MCParticleGraph::GraphParticle& graphParticle = createGraphParticle(m_rofMCParticleGraph, mcParticle, motherIndex);

  //Use the TrackID to store an unique identifier for each MCParticleGraph node
  graphParticle.setTrackID(++m_rofGraphUniqueID);
  uniqueIDList[mcParticle.getArrayIndex()] = m_rofGraphUniqueID;

  //Add all children
  int currMotherIndex = m_rofMCParticleGraph.size();
  BOOST_FOREACH(MCParticle* daughter, mcParticle.getDaughters()) {
    addParticleToROFGraph(*daughter, currMotherIndex, uniqueIDList);
  }
}
