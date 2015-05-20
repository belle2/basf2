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

  // usage with const pointer. NOTE: the const is important, without it gets called with a reference which doesnot work
  for (int iTC = 0; iTC < spacePointTCs.getEntries(); ++iTC) {
    B2DEBUG(150, "Calculating purities for SP container " << iTC << ", name: " << spacePointTCs.getName());
    const SpacePointTrackCand* container = spacePointTCs[iTC];
    // vector<pair<int, double> > purities = calculatePurity(container);
    vector<MCVXDPurityInfo> purInfos = createPurityInfos(container);

    for (auto& info : purInfos) {
      B2DEBUG(499, "PurityInfo: " << info.dumpToString());
      B2DEBUG(499, "purities: PXD: " << info.getPurityPXD().second << ", SVD U: " << info.getPuritySVDU().second <<
              ", SVD V: " << info.getPuritySVDV().second << ", overall: " << info.getPurity().second);
    }
  }

  // // usage with reference
  // for (const SpacePointTrackCand& container : spacePointTCs) {
  //   // vector<pair<int, double> > purities = calculatePurity(container);
  //   vector<MCVXDPurityInfo> purInfos = createPurityInfos(container);
  // }

}

// ============================================== TERMINATE ===========================================================
void ThreeHitSamplesGeneratorModule::terminate()
{
  // TODO
}

// ============================================= INITIALIZE ROOT FILE ==============================================================
void ThreeHitSamplesGeneratorModule::initializeRootFile(const std::string& filename, const std::string& writemode)
{
  m_rootFilePtr = new TFile(filename.c_str(), writemode.c_str());
  m_treePtr = new TTree("ThreeHitSamplesTree", "three hit training samples");

  // TODO: linking variables
}
