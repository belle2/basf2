/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2016 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <vector>

#include <tracking/modules/vxdtfRedesign/SegmentNetworkAnalyzerModule.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/PurityCalculatorTools.h>


using namespace Belle2;
using namespace std;

REG_MODULE(SegmentNetworkAnalyzer);

SegmentNetworkAnalyzerModule::SegmentNetworkAnalyzerModule() :
  Module(),
  m_rFilePtr(nullptr),
  m_treePtr(nullptr)
{
  setDescription("Module for analyzing the SegmentNetwork.");

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

  auto& hitNetwork = m_network->accessHitNetwork();
  auto& segmentNetwork = m_network->accessSegmentNetwork();
  m_rootVariables.networkSize = segmentNetwork.size();
  m_rootVariables.networkConnections = getNConnections(segmentNetwork);

  for (const auto& outerHit : hitNetwork.getNodes()) {
    for (const auto& centerHit : outerHit->getInnerNodes()) {
      for (const auto& innerHit : centerHit->getInnerNodes()) {

        Segment<TrackNode>* innerSegment = new Segment<TrackNode>(centerHit->getEntry().m_sector->getFullSecID(),
                                                                  innerHit->getEntry().m_sector->getFullSecID(),
                                                                  &centerHit->getEntry(),
                                                                  &innerHit->getEntry());
        Segment<TrackNode>* outerSegment = new Segment<TrackNode>(outerHit->getEntry().m_sector->getFullSecID(),
                                                                  centerHit->getEntry().m_sector->getFullSecID(),
                                                                  &outerHit->getEntry(),
                                                                  &centerHit->getEntry());

        bool passed = false;
        // check if the outerSegment is in the network and then look if the inner is connected to it
        if (auto outerNode = segmentNetwork.getNode(outerSegment->getID())) {
          for (const auto& connectedNode : outerNode->getInnerNodes()) {
            if (connectedNode->getEntry() == *innerSegment) {
              passed = true;
              break;
            }
          }
        }
        analyzeCombination(*innerSegment, *outerSegment, passed);

      } // end inner loop
    } // end center loop
  } // end outer loop

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
  m_treePtr->Branch("passed", &m_rootVariables.passed);
  m_treePtr->Branch("pdg", &m_rootVariables.pdg);
  m_treePtr->Branch("virtualIP", &m_rootVariables.virtualIP);
  m_treePtr->Branch("networkSize", &m_rootVariables.networkSize);
  m_treePtr->Branch("networkConns", &m_rootVariables.networkConnections);
}

void SegmentNetworkAnalyzerModule::analyzeCombination(const Belle2::Segment<Belle2::TrackNode>& outer,
                                                      const Belle2::Segment<Belle2::TrackNode>& inner,
                                                      bool passed)
{
  m_rootVariables.passed.push_back(passed);

  std::vector<const Belle2::SpacePoint*> combinationSPs; // order in which they are stored does not really matter
  combinationSPs.push_back(outer.getOuterHit()->m_spacePoint);
  combinationSPs.push_back(outer.getInnerHit()->m_spacePoint);
  combinationSPs.push_back(inner.getInnerHit()->m_spacePoint);

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

  // check if the innermost hit is virtualIP, if so get the outer hit to retrieve the position information from it
  auto spacePoint = inner.getInnerHit()->m_spacePoint;
  if (spacePoint->getType() == VXD::SensorInfoBase::VXD) {
    m_rootVariables.virtualIP.push_back(1);
    spacePoint = inner.getOuterHit()->m_spacePoint;
  } else m_rootVariables.virtualIP.push_back(0);

  // angles are determined from inner most (accessible) hit in combination
  auto position = spacePoint->getPosition();
  m_rootVariables.phi.push_back(position.Phi());
  m_rootVariables.theta.push_back(position.Theta());

  B2DEBUG(100, "Collected combination with: phi " << position.Phi() << ", theta " << position.Theta() <<
          ", pdg " << pdg << ", pT " << pT << ", signal " << signal << ", passed " << passed << ", mcId " << mcId.first);
}

template<typename EntryType, typename MetaInfoType >
size_t SegmentNetworkAnalyzerModule::getNConnections(Belle2::DirectedNodeNetwork<EntryType, MetaInfoType>& network) const
{
  size_t nLinks{};
  for (const auto& outerNodes : network) {
    nLinks += outerNodes->getInnerNodes().size();
  }
  return nLinks;
}
