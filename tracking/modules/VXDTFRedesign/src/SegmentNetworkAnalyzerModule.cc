/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2016 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/modules/VXDTFRedesign/SegmentNetworkAnalyzerModule.h>

// #include <tracking/trackFindingVXD/segmentNetwork/Segment.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/PurityCalculatorTools.h>
// #include <tracking/spacePointCreation/MCVXDPurityInfo.h>

#include <vector>

using namespace Belle2;
using namespace std;

REG_MODULE(SegmentNetworkAnalyzer);

SegmentNetworkAnalyzerModule::SegmentNetworkAnalyzerModule() :
  Module(),
  m_rFilePtr(nullptr),
  m_treePtr(nullptr)
{
  setDescription("module analyzing the segment network");

  addParam("networkInputName", m_PARAMnetworkName, "name of the StoreObjPtr to the DNN Container");
  addParam("rootFileName", m_PARAMrootFileName, "name of the output root file name",
           std::string("SegmentNetworkAnalyzer_output.root"));

}

void SegmentNetworkAnalyzerModule::initialize()
{
  B2INFO("SegmentNetworkAnalyzer::initialize() ------------------------------");
  m_network.isRequired(m_PARAMnetworkName);
  m_mcParticles = StoreArray<MCParticle>("");
  m_mcParticles.isRequired();

  m_rFilePtr = new TFile(m_PARAMrootFileName.c_str(), "recreate");
  m_treePtr = new TTree("SegmentNetworkAnalyzer", "segment network analysis output");

  makeBranches();
}

void SegmentNetworkAnalyzerModule::event()
{
  m_rootVariables = RootVariables(); // clear rootVariables

  auto& segmentNetwork = m_network->accessSegmentNetwork();
  m_rootVariables.networkSize = segmentNetwork.size();

  for (const auto& segmentNode : segmentNetwork.getNodes()) { // loop over all nodes
    for (const auto& innerSegmentNode : segmentNode->getInnerNodes()) { // loop over all inner segments
      analyzeCombination(segmentNode->getEntry(), innerSegmentNode->getEntry());
    } // end loop inner Segments
  } // end segments loop

  m_treePtr->Fill();
}

void SegmentNetworkAnalyzerModule::terminate()
{
  // write and close root file
  if (m_rFilePtr && m_treePtr) {
    m_rFilePtr->cd();
    m_rFilePtr->Write();
    m_rFilePtr->Close();
  }
}

void SegmentNetworkAnalyzerModule::makeBranches()
{
  m_treePtr->Branch("phi", &m_rootVariables.phi);
  m_treePtr->Branch("theta", &m_rootVariables.theta);
  m_treePtr->Branch("pT", &m_rootVariables.pT);
  m_treePtr->Branch("signal", &m_rootVariables.signal);
  m_treePtr->Branch("pdg", &m_rootVariables.pdg);
  m_treePtr->Branch("virtualIP", &m_rootVariables.virtualIP);
  m_treePtr->Branch("networkSize", &m_rootVariables.networkSize);
}

void SegmentNetworkAnalyzerModule::analyzeCombination(const Belle2::Segment<Belle2::TrackNode>& outer,
                                                      const Belle2::Segment<Belle2::TrackNode>& inner)
{
  std::vector<const Belle2::SpacePoint*> combinationSPs; // order in which they are stored does not really matter
  combinationSPs.push_back(outer.getOuterHit()->spacePoint);
  combinationSPs.push_back(outer.getInnerHit()->spacePoint);
  combinationSPs.push_back(inner.getInnerHit()->spacePoint);

  const vector<MCVXDPurityInfo> purityInfo = createPurityInfosVec(combinationSPs);
  auto mcId = purityInfo[0].getPurity();
  bool signal = mcId.first >= 0 && mcId.second == 1;
  m_rootVariables.signal.push_back((int) signal);

  // define some default values for non-signal combinations
  int pdg = 0;
  double pT = -999;

  if (signal) {
    const MCParticle* part = m_mcParticles[mcId.first];
    pdg = part->getPDG();
    pT = part->getMomentum().Pt();
  }

  m_rootVariables.pdg.push_back(pdg);
  m_rootVariables.pT.push_back(pT);

  // angles are determined from inner most hit in combination
  auto position = inner.getInnerHit()->spacePoint->getPosition();
  m_rootVariables.phi.push_back(position.Phi());
  m_rootVariables.theta.push_back(position.Theta());

  B2DEBUG(100, "Collected combination with: phi " << position.Phi() << ", theta " << position.Theta() <<
          ", pdg " << pdg << ", pT " << pT << ", signal " << signal << ", mcId " << mcId.first);
}
