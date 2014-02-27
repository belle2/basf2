/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <background/modules/mixbkg/ROFBuilderModule.h>

#include <framework/core/InputController.h>
#include <framework/gearbox/Unit.h>
#include <simulation/dataobjects/SimHitBase.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <top/dataobjects/TOPSimHit.h>
#include <arich/dataobjects/ARICHSimHit.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLHit.h>
#include <eklm/dataobjects/EKLMSimHit.h>
#include <bklm/dataobjects/BKLMSimHit.h>

#include <map>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ROFBuilder)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

const string ROFBuilderModule::s_auxMCParticlesName = "ROFBuilderMCParticleEvent";

ROFBuilderModule::ROFBuilderModule() : Module()
{
  //Set module properties
  setDescription("Generates a ROOT file containing the readout frames for a single subdetector and a specific background component and generator.");

  addParam("Subdetector", m_subdetector, "The subdetector for which the ROF Root file is created (1 = PXD, 2 = SVD).");
  addParam("SimHitCollectionName", m_simHitCollectionName, "The name of the SimHit collection.");
  addParam("SimHitMCPartRelationName", m_simHitMCPartRelationName, "The SimHit to MCParticle relation name.");
  // Timing mode
  addParam("TimeAwareMode", m_timeAwareMode, "Randomize events and generate time information in ROFs", bool(false));
  // For timeless mode - fixed number of base events per ROF
  addParam("EventsPerReadoutFrame", m_eventsPerReadoutFrame, "The number of events that represent one readout frame.", double(1));
  // For time-aware mode - Poisson-distributed number of base events, placed uniformly in time.
  addParam("WindowStart", m_windowStart, "Start time of subdetector acceptance window [ns]", -150.0);                 /**< Start of ROF windomw.*/
  addParam("WindowSize", m_windowSize, "Size of subdetector acceptance window [ns]", 330.0);
  addParam("BaseSampleSize", m_baseSampleSize, "Time equivalent of the base sample [us]", 200.0);

  addParam("OutputRootFileName", m_outputRootFileName, "The name of the ROF ROOT output file.");
  addParam("ComponentName", m_componentName, "The name of the background component (e.g. Touschek).");
  addParam("GeneratorName", m_generatorName, "The name of the background generator (e.g. SAD_LER).");
  addParam("MCParticleWriteMode", m_mcParticleWriteMode, "The MCParticle write mode.", 0);

  //Create Subdetector class name list FIXME: Is it enough just to fill other subdetector names?
  m_SimHitClassNames.push_back("None");
  m_SimHitClassNames.push_back(PXDSimHit::Class_Name());
  m_SimHitClassNames.push_back(SVDSimHit::Class_Name());
  m_SimHitClassNames.push_back(CDCSimHit::Class_Name());
  m_SimHitClassNames.push_back(TOPSimHit::Class_Name());
  m_SimHitClassNames.push_back(ARICHSimHit::Class_Name());
  m_SimHitClassNames.push_back(ECLHit::Class_Name());
  m_SimHitClassNames.push_back(EKLMSimHit::Class_Name());
  m_SimHitClassNames.push_back(BKLMSimHit::Class_Name());
  m_SimHitClassNames.push_back(ECLSimHit::Class_Name());
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
  // FIXME: Shall I write window parameters here, too?
  m_contentTree->Branch("Subdetector", &m_subdetector, "SBD/I");
  m_contentTree->Branch("Component", &m_componentName);
  m_contentTree->Branch("Generator", &m_generatorName);
  m_contentTree->Branch("SimHitCollection", &m_simHitCollectionName);
  m_contentTree->Branch("SimHitRelation", &m_simHitMCPartRelationName);
  m_contentTree->Branch("MCParticleWriteMode", &m_mcParticleWriteMode, "MCW/I");
  m_contentTree->Fill();

  // Register the auxiliary MCParticle StoreArray in the DataStore.
  StoreArray<MCParticle>::registerTransient(s_auxMCParticlesName.c_str());

  // Set the background tag for SimHits
  // FIXME: Move this to SimHitBase code to keep related things together.
  bool isHER = (boost::to_upper_copy(m_generatorName).find("HER") != string::npos);
  if (boost::to_upper_copy(m_componentName).find("COULOMB") != string::npos) {
    m_backgroundTag = isHER ? SimHitBase::bg_Coulomb_HER : SimHitBase::bg_Coulomb_LER;
  } else if (boost::to_upper_copy(m_componentName).find("RBB") != string::npos) {
    m_backgroundTag = isHER ? SimHitBase::bg_RBB_HER : SimHitBase::bg_RBB_LER;
  } else if (boost::to_upper_copy(m_componentName).find("TOUSCHEK") != string::npos) {
    m_backgroundTag = isHER ? SimHitBase::bg_Touschek_HER : SimHitBase::bg_Touschek_LER;
  } else if (boost::to_upper_copy(m_componentName).find("TWOPHOTON") != string::npos) {
    m_backgroundTag = SimHitBase::bg_twoPhoton;
  } else
    m_backgroundTag = SimHitBase::bg_other;

  //Start a new readout frame
  m_rofMCParticleGraph.clear();
  m_mcpToSimHitMap.clear();
  m_currReadoutFrameIdx = 0;
  m_event = 0;
  m_numberSimHits = 0;
  m_rofGraphUniqueID = -1;

  // Number of events on input
  int nBaseEvents = InputController::numEntries();

  if (m_timeAwareMode) {
    // Set time-dependent parameters.
    m_baseSampleSize *= Unit::us;
    m_windowStart *= Unit::ns;
    m_windowSize *= Unit::ns;
    double tau = 1.0 * m_baseSampleSize / nBaseEvents;
    m_timer = new RandomTimer(tau, m_windowStart, m_windowSize);
    m_eventTime = m_windowStart - 1; // so that we don't store an empty frame at the start
    m_eventsPerReadoutFrame = nBaseEvents / m_baseSampleSize * m_windowSize;
  }

  map<int, string> writeModeLabels;
  writeModeLabels.insert(make_pair(0, "none"));
  writeModeLabels.insert(make_pair(1, "seen in the subdetector"));
  writeModeLabels.insert(make_pair(2, "seen in the subdetector + mothers"));
  writeModeLabels.insert(make_pair(3, "all"));

  B2INFO("=======================================================")
  B2INFO("                    ROFBuilder                         ")
  B2INFO("-------------------------------------------------------")
  B2INFO("Operation mode       : " << (m_timeAwareMode ? "Time-Aware" : "Timeless"))
  B2INFO("Events per ROF       : " << m_eventsPerReadoutFrame << (m_timeAwareMode ? " (average) " : ""))
  B2INFO("Output file          : " << m_outputRootFileName)
  B2INFO("Subdetector SimHits  : " << m_SimHitClassNames[m_subdetector])
  B2INFO("Component            : " << m_componentName)
  B2INFO("Generator            : " << m_generatorName)
  B2INFO("MCParticle Write Mode: " << writeModeLabels[m_mcParticleWriteMode])
  if (m_timeAwareMode) {
    B2INFO("Window size  (ns)    : " << m_windowSize)
    B2INFO("Window start (ns)    : " << m_windowStart)
    B2INFO("Base sample size (ns): " << m_baseSampleSize)
    B2INFO("Events on input      : " << nBaseEvents)
  }
  B2INFO("-------------------------------------------------------")
}


void ROFBuilderModule::event()
{
  //Check if a new readout frame has to be created
  bool frameDone = (m_event >= ((m_currReadoutFrameIdx + 1) * m_eventsPerReadoutFrame));
  if (m_timeAwareMode) {
    float newEventTime = m_timer->getNextTime();
    frameDone = m_timer->isEndOfFrame();
    m_eventTime = newEventTime;
  }
  if (frameDone) {
    fillROFTree();
    B2INFO(">> Save readout frame: #" << m_currReadoutFrameIdx << " (" << m_numberSimHits << " SimHits" << ")")

    //Clear the TClones Arrays
    m_readoutFrame->Clear();
    m_mcParticles->Clear();
    m_mcPartRels->Clear();

    m_numberSimHits = 0;
  }
  // For frames shorter than events: Check for overflow and save empty frames if necessary.
  if (m_timeAwareMode) {
    if (m_timer->isOverFlow()) m_eventTime = m_timer->getNextTime();
    while (m_timer->isOverFlow()) {
      m_eventTime = m_timer->getNextTime();
      fillROFTree();
      B2INFO(">> Save readout frame: #" << m_currReadoutFrameIdx << " (0*SimHits)")
    }
  }
  //Depending on the subdetector, add the appropriate SimHits to the ROF
  switch (m_subdetector) {
    case 1 : addSimHitsToROF<PXDSimHit>();
      break;
    case 2 : addSimHitsToROF<SVDSimHit>();
      break;
    case 3 : addSimHitsToROF<CDCSimHit>();
      break;
    case 4 : addSimHitsToROF<TOPSimHit>();
      break;
    case 5 : addSimHitsToROF<ARICHSimHit>();
      break;
    case 6 : addSimHitsToROF<ECLHit>();
      break;
    case 7 : addSimHitsToROF<EKLMSimHit>();
      break;
    case 8 : addSimHitsToROF<BKLMSimHit>();
      break;
    case 9 : addSimHitsToROF<ECLSimHit>();
  }

  m_event++;

}


void ROFBuilderModule::terminate()
{
  //Check if a new readout frame has to be created
  bool frameDone = (m_event >= ((m_currReadoutFrameIdx + 1) * m_eventsPerReadoutFrame));
  if (m_timeAwareMode) {
    float newEventTime = m_timer->getNextTime();
    frameDone = newEventTime < m_eventTime;
    m_eventTime = newEventTime;
  }
  if (frameDone) {
    fillROFTree();
    B2INFO(">> Save readout frame: #" << m_currReadoutFrameIdx << " (" << m_numberSimHits << " SimHits" << ")")
  }
  B2INFO("=======================================================")

  m_outputRootFile->WriteObject(m_rofTree, m_rofTree->GetName());
  m_outputRootFile->WriteObject(m_contentTree, m_contentTree->GetName());
  m_outputRootFile->Close();

  delete m_mcPartRels;
  delete m_mcParticles;
  delete m_readoutFrame;

  if (m_timeAwareMode) delete m_timer;

}


void ROFBuilderModule::fillROFTree()
{
  //Fill the MCParticles if the MCParticle write mode is set
  if (m_mcParticleWriteMode > 0) {

    m_rofMCParticleGraph.generateList(s_auxMCParticlesName.c_str(), MCParticleGraph::c_clearParticles);
    StoreArray<MCParticle> mcParticleEventROF(s_auxMCParticlesName.c_str());

    int nParticles = mcParticleEventROF.getEntries();
    for (int iParticle = 0; iParticle < nParticles; ++iParticle) {
      new((*m_mcParticles)[iParticle]) MCParticle(*mcParticleEventROF[iParticle]);
    }

    //Create a list that relates the unique identifier of a MCParticleGraph node (list index)
    //with the index of the MCParticle in the final list
    vector<int> uniqueIDMCPartFinal;
    uniqueIDMCPartFinal.resize(m_rofMCParticleGraph.size());
    for (unsigned int iPart = 0; iPart < m_rofMCParticleGraph.size(); ++iPart) {
      MCParticleGraph::GraphParticle& currParticle = m_rofMCParticleGraph[iPart];
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
  }

  //Fill the ROOT tree
  m_rofTree->Fill();
  m_currReadoutFrameIdx++;

  //Reset the ROF variables
  m_rofMCParticleGraph.clear();
  m_mcpToSimHitMap.clear();
  m_rofGraphUniqueID = -1;
}


MCParticleGraph::GraphParticle& ROFBuilderModule::createGraphParticle(MCParticleGraph& graph, MCParticle& mcParticle, int motherIndex)
{
  MCParticleGraph::GraphParticle& graphParticle = graph.addParticle();
  graphParticle.setPDG(mcParticle.getPDG());
  graphParticle.setStatus(mcParticle.getStatus());
  graphParticle.setMass(mcParticle.getMass());
  graphParticle.setEnergy(mcParticle.getEnergy());
  graphParticle.setValidVertex(mcParticle.hasValidVertex());
  graphParticle.setProductionTime(mcParticle.getProductionTime());
  graphParticle.setDecayTime(mcParticle.getDecayTime());
  graphParticle.setProductionVertex(mcParticle.getProductionVertex());
  graphParticle.setMomentum(mcParticle.getMomentum());
  graphParticle.setDecayVertex(mcParticle.getDecayVertex());
  graphParticle.setFirstDaughter(mcParticle.getFirstDaughter());
  graphParticle.setLastDaughter(mcParticle.getLastDaughter());
  if (motherIndex > 0) graphParticle.comesFrom(graph[motherIndex - 1]); //Add decay

  return graphParticle;
}


void ROFBuilderModule::addParticleToEventGraph(MCParticleGraph& graph, MCParticle& mcParticle, int motherIndex, const std::vector<bool>& keepList)
{
  MCParticleGraph::GraphParticle& graphParticle = createGraphParticle(graph, mcParticle, motherIndex);

  //Keep/Ignore particles based on the keep list
  graphParticle.setTrackID(mcParticle.getArrayIndex());
  graphParticle.setIgnore(!keepList[mcParticle.getArrayIndex()]);

  //Add all children
  int currMotherIndex = graph.size();
  for (MCParticle * daughter : mcParticle.getDaughters()) {
    addParticleToEventGraph(graph, *daughter, currMotherIndex, keepList);
  }
}


void ROFBuilderModule::addParticleToROFGraph(MCParticle& mcParticle, int motherIndex, std::vector<int>& uniqueIDList)
{
  MCParticleGraph::GraphParticle& graphParticle = createGraphParticle(m_rofMCParticleGraph, mcParticle, motherIndex);

  //Use the TrackID to store an unique identifier for each MCParticleGraph node
  graphParticle.setTrackID(++m_rofGraphUniqueID);
  uniqueIDList[mcParticle.getArrayIndex()] = m_rofGraphUniqueID;

  //Add all children
  int currMotherIndex = m_rofMCParticleGraph.size();
  for (MCParticle * daughter : mcParticle.getDaughters()) {
    //BOOST_FOREACH(MCParticle * daughter, mcParticle.getDaughters()) {
    addParticleToROFGraph(*daughter, currMotherIndex, uniqueIDList);
  }
}
