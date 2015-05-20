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
  StoreArray<SpacePointTrackCand> spacePointTCs(m_PARAMcontainerName);

  // loop over all trackCands in the event
  for (int iTC = 0; iTC < spacePointTCs.getEntries(); ++iTC) {
    B2DEBUG(15, "Getting training samples from SP container " << iTC << ", name: " << spacePointTCs.getName());
    const SpacePointTrackCand* fullSPTC = spacePointTCs[iTC];

    vector<SpacePointTrackCand> threeHitCombos = splitTrackCandNHitCombos(fullSPTC, 3);

    for (const SpacePointTrackCand& comb : threeHitCombos) {
      vector<MCVXDPurityInfo> purInfos = createPurityInfos(comb);
      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 499, PACKAGENAME())) {
        for (auto& info : purInfos) {
          B2DEBUG(499, "PurityInfo: " << info.dumpToString());
        }
      }

      addHitCombination(comb, (purInfos[0].getPurity().second == 1), m_combinations);
    }
  }

  m_treePtr->Fill();
}

// ============================================== TERMINATE ===========================================================
void ThreeHitSamplesGeneratorModule::terminate()
{
  // TODO: info output

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
}

// ============================================ SPLIT TRACKCAND N HIT COMBOS =======================================================
std::vector<Belle2::SpacePointTrackCand>
ThreeHitSamplesGeneratorModule::splitTrackCandNHitCombos(const Belle2::SpacePointTrackCand* trackCand, unsigned nHits)
{
  const std::vector<const SpacePoint*> spacePoints = trackCand->getHits();

  B2DEBUG(25, "Now trying to get " << nHits << " hit combinations from trackCand " << trackCand->getArrayIndex() <<
          ". It contains " << trackCand->getNHits() << " SpacePoints.");

  std::vector<SpacePointTrackCand> combiContainer;

  if (nHits > trackCand->getNHits()) {
    B2DEBUG(25, "SpacePointTrackCand contains an insufficient number of hits! " << nHits <<
            " are needed. Returning an empty vector of combinations!");
    return combiContainer;
  }

  auto itNHit = spacePoints.begin() + nHits;
  auto itBegin = spacePoints.begin();
  while (itNHit != spacePoints.end() + 1) { // constructor from iterators is [first,last)
    vector<const SpacePoint*> combination(itBegin, itNHit);
    if (isValidHitCombination(combination)) {
      combiContainer.push_back(SpacePointTrackCand(combination));
    }

    itBegin++; itNHit++;
  }

  B2DEBUG(25, "Created " << combiContainer.size() << " combinations from trackCand " << trackCand->getArrayIndex());
  return combiContainer;
}

// ====================================================== IS VALID HIT COMBINATION =================================================
bool ThreeHitSamplesGeneratorModule::isValidHitCombination(const std::vector<const Belle2::SpacePoint*>&) // no name, no warning
{
  // TODO
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

  combinations.Signal.push_back(signal);
}
