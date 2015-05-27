/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/trainingDataGenerator/ThreeHitSamplesGeneratorModule.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/spacePointCreation/PurityCalculatorTools.h>
#include <tracking/spacePointCreation/MCVXDPurityInfo.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <vxd/dataobjects/VxdID.h>

#include <vector>
#include <utility> // pair

#include <boost/algorithm/string.hpp>

using namespace std;
using namespace Belle2;

REG_MODULE(ThreeHitSamplesGenerator)

ThreeHitSamplesGeneratorModule::ThreeHitSamplesGeneratorModule() :
  Module()
{
  setDescription("Module for generating training data samples. WARNING: currently under development!");
  addParam("containerName", m_PARAMcontainerName,
           "Name of the SpacePoint container in the StoreArray. NOTE: can only be SpacePointTrackCand at the moment!");

  vector<string> defaultOutput = { "ThreeHitSamples", "recreate" };
  addParam("outputFileName", m_PARAMoutputFileName,
           "Name of the output file (root file) that contains the training samples. Two arguments needed: first is filename (withoug .root file-ending), second is write-mode ('update' or 'recreate')",
           defaultOutput);

  initializeCounters();

  // initialize pointers
  m_rootFilePtr = NULL; m_treePtr = NULL;
}

// =========================================== INITIALIZE ==============================================================
void ThreeHitSamplesGeneratorModule::initialize()
{
  B2INFO("ThreeHitSamplesGenerator ----------------- initialize() -----------------------");
  StoreArray<SpacePointTrackCand>::required(m_PARAMcontainerName);

  if (m_PARAMoutputFileName.size() != 2 ||
      (boost::to_upper_copy<std::string>(m_PARAMoutputFileName[1]) != "RECREATE" &&
       boost::to_upper_copy<std::string>(m_PARAMoutputFileName[1]) != "UPDATE")) {
    string output;
    for (const string& element : m_PARAMoutputFileName) { output += "'" + element + "' "; }
    B2FATAL("ThreeHitSamplesGenerator::initialize() : outputFileName is set wrong: entries are: " << output);
  } else {
    initializeRootFile(m_PARAMoutputFileName[0] + ".root", m_PARAMoutputFileName[1]);
  }

}

// ============================================== EVENT ===============================================================
void ThreeHitSamplesGeneratorModule::event()
{
  StoreArray<SpacePointTrackCand> spacePointTCs(m_PARAMcontainerName); // make this a member -> not having to create it every event

  m_combinations = RootCombinations(); // reset in every event

  // loop over all trackCands in the event
  for (int iTC = 0; iTC < spacePointTCs.getEntries(); ++iTC) {
    B2DEBUG(15, "Getting training samples from SP container " << iTC << ", name: " << spacePointTCs.getName());
    const SpacePointTrackCand* fullSPTC = spacePointTCs[iTC];

    vector<SpacePointTrackCand> threeHitCombos = splitTrackCandNHitCombos(fullSPTC, 3);

    for (const SpacePointTrackCand& comb : threeHitCombos) {
      B2DEBUG(25, "Getting purity Infos from combination");
      vector<MCVXDPurityInfo> purInfos = createPurityInfos(comb);

      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 499, PACKAGENAME())) {
        stringstream purInfoStr;
        for (auto& info : purInfos) {
          purInfoStr << info.dumpToString();
        }
        B2DEBUG(499, "combination " << m_combCtr <<  ", PurityInfos:\n" << purInfoStr.str(););
      }

      // only take a combination as signal if it is from a valid MCParticle and if purity is 1
      bool isSignal = (purInfos[0].getPurity().first >= 0 && purInfos[0].getPurity().second == 1);
      addHitCombination(comb, isSignal, m_combinations);
    }
  }

  m_treePtr->Fill();
}

// ============================================== TERMINATE ========================================================================
void ThreeHitSamplesGeneratorModule::terminate()
{
  B2INFO("ThreeHitSampleGenerator::terminate() : generated " << m_combCtr << " combinations: " << m_noiseSampleCtr <<
         " noise samples, " << m_signalSampleCtr << " signal samples. " << m_invalidCombiCtr <<
         " combinations were ruled invalid. " << m_smallContainerCtr << " SPTCs did not have enough hits");

  // root file handling
  if (m_rootFilePtr != NULL && m_treePtr != NULL) {
    m_rootFilePtr->cd();
    m_rootFilePtr->Write();
    m_rootFilePtr->Close();
  }
}

// ============================================= INITIALIZE ROOT FILE ==============================================================
void ThreeHitSamplesGeneratorModule::initializeRootFile(const std::string& filename, const std::string& writemode)
{
  m_rootFilePtr = new TFile(filename.c_str(), writemode.c_str());
  m_treePtr = new TTree("ThreeHitSamplesTree", "three hit training samples");

  m_treePtr->Branch("hit1X", &m_combinations.Hit1X);
  m_treePtr->Branch("hit1Y", &m_combinations.Hit1Y);
  m_treePtr->Branch("hit1Z", &m_combinations.Hit1Z);

  m_treePtr->Branch("hit2X", &m_combinations.Hit2X);
  m_treePtr->Branch("hit2Y", &m_combinations.Hit2Y);
  m_treePtr->Branch("hit2Z", &m_combinations.Hit2Z);

  m_treePtr->Branch("hit3X", &m_combinations.Hit3X);
  m_treePtr->Branch("hit3Y", &m_combinations.Hit3Y);
  m_treePtr->Branch("hit3Z", &m_combinations.Hit3Z);

  m_treePtr->Branch("signal", &m_combinations.Signal);

  m_treePtr->Branch("layer1", &m_combinations.Layer1);
  m_treePtr->Branch("layer2", &m_combinations.Layer2);
  m_treePtr->Branch("layer3", &m_combinations.Layer3);
}

// ============================================ SPLIT TRACKCAND N HIT COMBOS =======================================================
std::vector<Belle2::SpacePointTrackCand>
ThreeHitSamplesGeneratorModule::splitTrackCandNHitCombos(const Belle2::SpacePointTrackCand* trackCand, unsigned nHits)
{
  const std::vector<const SpacePoint*>& spacePoints = trackCand->getHits();

  B2DEBUG(25, "Now trying to get " << nHits << " hit combinations from trackCand " << trackCand->getArrayIndex() <<
          ". It contains " << trackCand->getNHits() << " SpacePoints.");

  std::vector<SpacePointTrackCand> combiContainer;

  if (nHits > trackCand->getNHits()) {
    B2DEBUG(25, "SpacePointTrackCand contains an insufficient number of hits! " << nHits <<
            " are needed. Returning an empty vector of combinations!");
    m_smallContainerCtr++;
    return combiContainer;
  }

  auto itNHit = spacePoints.begin() + nHits;
  auto itBegin = spacePoints.begin();
  while (itNHit != spacePoints.end() + 1) { // constructor from iterators is [first,last)
    vector<const SpacePoint*> combination(itBegin, itNHit);
    if (isValidHitCombination(combination)) {
      combiContainer.push_back(SpacePointTrackCand(combination));
    }

    ++itBegin; ++itNHit;
  }

  B2DEBUG(25, "Created " << combiContainer.size() << " combinations from trackCand " << trackCand->getArrayIndex());
  return combiContainer;
}

// ====================================================== IS VALID HIT COMBINATION =================================================
bool ThreeHitSamplesGeneratorModule::isValidHitCombination(const std::vector<const Belle2::SpacePoint*>& combination)
{
  B2DEBUG(150, "Checking if the combination is valid");
  // unsigned short layer = (*combination.begin())->getVxdID().getLayerNumber();
  // getVxdID of SpacePoint returns unsigned short so we have to convert it to a VxdID first to get a layer number
  unsigned short prevLayer = VxdID((*combination.begin())->getVxdID()).getLayerNumber();
  for (auto itSP = combination.begin() + 1; itSP != combination.end(); ++itSP) { // do not use first SpacePoint
    unsigned short layer = VxdID((*itSP)->getVxdID()).getLayerNumber();
    if (layer != prevLayer + 1) {
      B2DEBUG(150, "The hits in the passed combination are not in subsequent order: previous layer = " << prevLayer <<
              ", this layer = " << layer);
      ++m_invalidCombiCtr;
      // B2ERROR("==================================== INVALID =============================================");
      return false;
    }
    prevLayer = layer;
  }

  B2DEBUG(150, "Combination is valid!")
  return true;
}

// ================================================== ADD HIT COMBINATION ==========================================================
void ThreeHitSamplesGeneratorModule::addHitCombination(const Belle2::SpacePointTrackCand& combination, bool signal,
                                                       RootCombinations& combinations)
{
  // TODO: make sure ordering is kept as it should be
  // TODO: make this less ugly
  const std::vector<const SpacePoint*>& spacePoints = combination.getHits();

  combinations.Hit1X.push_back(spacePoints[0]->X());
  combinations.Hit1Y.push_back(spacePoints[0]->Y());
  combinations.Hit1Z.push_back(spacePoints[0]->Z());

  combinations.Hit2X.push_back(spacePoints[1]->X());
  combinations.Hit2Y.push_back(spacePoints[1]->Y());
  combinations.Hit2Z.push_back(spacePoints[1]->Z());

  combinations.Hit3X.push_back(spacePoints[2]->X());
  combinations.Hit3Y.push_back(spacePoints[2]->Y());
  combinations.Hit3Z.push_back(spacePoints[2]->Z());

  combinations.Layer1.push_back(VxdID(spacePoints[0]->getVxdID()).getLayerNumber());
  combinations.Layer2.push_back(VxdID(spacePoints[1]->getVxdID()).getLayerNumber());
  combinations.Layer3.push_back(VxdID(spacePoints[2]->getVxdID()).getLayerNumber());

  combinations.Signal.push_back(signal);

  m_combCtr++;
  m_noiseSampleCtr += (!signal);
  m_signalSampleCtr += signal;
}
