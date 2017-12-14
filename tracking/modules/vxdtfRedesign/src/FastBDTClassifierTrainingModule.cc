/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2016 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/modules/vxdtfRedesign/FastBDTClassifierTrainingModule.h>
#include <tracking/trackFindingVXD/filterTools/FBDTClassifier.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetwork.h>
#include <tracking/trackFindingVXD/segmentNetwork/TrackNode.h>

#include <tracking/spacePointCreation/PurityCalculatorTools.h>
#include <tracking/spacePointCreation/MCVXDPurityInfo.h>

#include <framework/logging/Logger.h>

#include <fstream>
#include <sstream>

using namespace Belle2;
using namespace std;

REG_MODULE(FastBDTClassifierTraining);

FastBDTClassifierTrainingModule::FastBDTClassifierTrainingModule() : Module()
{
  setDescription("TODO");

  addParam("outputFileName",
           m_PARAMfbdtOutFileName,
           "Output file name to which the trained FBDTClassifier will be stored",
           std::string("FBDTClassifier.dat"));

  addParam("networkInputName", m_PARAMnetworkInputName,
           "Name of the StoreObjPtr where the network container used in this module is stored", std::string(""));

  addParam("train", m_PARAMdoTrain, "Set if the module should train a classifier after collecting or not", true);
  addParam("nTrees", m_PARAMnTrees, "Number of Trees used in the FastBDT", 100);
  addParam("treeDepth", m_PARAMtreeDepth, "Tree depth of the trees used in the FastBDT", 3);
  addParam("shrinkage", m_PARAMshrinkage, "Shrinkage parameter used in the FastBDT", 0.15);
  addParam("randRatio", m_PARAMrandRatio, "ratio of randomly chosen samples for training of one tree", 0.5);
  addParam("storeSamples", m_PARAMstoreSamples, "store the collected samples into a file", false);
  addParam("samplesFileName", m_PARAMsamplesFileName, "the file name into which/from whicht the collected samples are stored/read",
           std::string("FBDTClassifier_samples.dat"));
  addParam("useSamples", m_PARAMuseSamples,
           "use samples for training that have been collected previously and bypass the collection of samples", false);
}

void FastBDTClassifierTrainingModule::initialize()
{

  if (m_PARAMuseSamples && m_PARAMstoreSamples) {
    B2ERROR("storeSamples and useSamples are both set to true. However, only one option can be set at a time");
  }

  if (m_PARAMnTrees < 1) {
    B2WARNING("nTrees was set to " << m_PARAMnTrees << ". Has to be at least 1. Setting to 1.");
    m_PARAMnTrees = 1;
  }

  if (m_PARAMtreeDepth < 0) {
    B2WARNING("Trees have to be at least a stump, but treeDepth was set to " << m_PARAMtreeDepth << ". Setting to 3 (default).");
    m_PARAMtreeDepth = 3;
  }

  if (m_PARAMshrinkage < 0 || m_PARAMshrinkage > 1) { // TODO: check this
    B2WARNING("shrinkage has to be in [0,1] but was " << m_PARAMrandRatio << ". Setting to 0.15 (default).");
    m_PARAMshrinkage = .15;
  }

  if (m_PARAMrandRatio < 0 || m_PARAMrandRatio > 1) {
    B2WARNING("randRatio has to be in [0,1] but was " << m_PARAMrandRatio << ". Setting to 0.5 (default).");
    m_PARAMrandRatio = 0.5;
  }

  if (m_PARAMuseSamples) {
    ifstream sampFile(m_PARAMsamplesFileName);
    if (!sampFile.is_open() || !sampFile.good()) {
      B2ERROR("Was not able to open the samples file: " << m_PARAMsamplesFileName);
    }

    readSamplesFromStream(sampFile, m_samples);
  } else { // only if no samples are provided the collection from the DNN is necessary
    m_network.isRequired(m_PARAMnetworkInputName);
  }
}

void FastBDTClassifierTrainingModule::event()
{
  if (m_PARAMuseSamples) return; // don't collect anything during event if samples are provided

  DirectedNodeNetwork<TrackNode, VoidMetaInfo>& hitNetwork = m_network->accessHitNetwork();

  // B2DEBUG(1, "size of hitNetwork " << hitNetwork.getNodes().size());

  size_t samplePriorEvent = m_samples.size();

  // XXXHit is of type DirectedNode<TrackNode, VoidMetaInfo>
  for (const auto& outerHit : hitNetwork.getNodes()) { // loop over all outer nodes
    for (const auto& centerHit : outerHit->getInnerNodes()) { // loop over all center nodes attached to outer node
      for (const auto& innerHit : centerHit->getInnerNodes()) { // loop over all inner nodes attached to center node
        m_samples.push_back(makeTrainSample(outerHit->getEntry().m_spacePoint,
                                            centerHit->getEntry().m_spacePoint,
                                            innerHit->getEntry().m_spacePoint));
      } // inner node loop
    } // center node loop
  } // outer node loop

  B2DEBUG(10, "collected " << m_samples.size() - samplePriorEvent << " training samples in this event");

}

void FastBDTClassifierTrainingModule::terminate()
{
  if (m_PARAMstoreSamples) {
    B2DEBUG(1, "Storing the collected samples to file: " << m_PARAMsamplesFileName);
    ofstream sampStream(m_PARAMsamplesFileName);
    sampStream.precision(16); // increase precision for sample writeout
    writeSamplesToStream(sampStream, m_samples);
    sampStream.close();
  }
  if (m_PARAMdoTrain) {
    FBDTClassifier<9> classifier{};
    B2DEBUG(1, "Training a FBDTClassifier with " << m_samples.size() << " input samples. Training Parameters: \n" <<
            "nTrees: " << m_PARAMnTrees << "\n" <<
            "treeDetph: " << m_PARAMtreeDepth << "\n" <<
            "shrinkage: " << m_PARAMshrinkage << "\n" <<
            "randRatio: " << m_PARAMrandRatio << "\n");
    classifier.train(m_samples, m_PARAMnTrees, m_PARAMtreeDepth, m_PARAMshrinkage, m_PARAMrandRatio);

    ofstream ofs(m_PARAMfbdtOutFileName);
    classifier.writeToStream(ofs);
    ofs.close();
  }
}

const FastBDTClassifierTrainingModule::TrainSample
FastBDTClassifierTrainingModule::makeTrainSample(const Belle2::SpacePoint* outer, const Belle2::SpacePoint* center,
                                                 const Belle2::SpacePoint* inner)
{
  vector<MCVXDPurityInfo> purityInfos = createPurityInfosVec({outer, center, inner});
  auto mcId = purityInfos[0].getPurity(); // there is at least one entry in this vector!
  bool signal = (mcId.first >= 0 && mcId.second == 1); // only assign true for actual MCParticle and purity 1

  std::array<double, 9> coords {{
      inner->X(), inner->Y(), inner->Z(),
      center->X(), center->Y(), center->Z(),
      outer->X(), outer->Y(), outer->Z()
    }};

  TrainSample sample(coords, signal);

  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 499, PACKAGENAME())) {
    std::stringstream coordOutput;
    for (double d : sample.hits) coordOutput << d << " ";

    B2DEBUG(499, "Created TrainingsSample with coordinates: ( " << coordOutput.str() <<  " ) " << sample.signal);
  }

  return sample;
}

// void FastBDTClassifierTrainingModule::readSamplesFromStream(std::istream& is)
// {
//   std::string line;
//   while(!is.eof()) {
//     getline(is, line);
//     if(line.empty()) break;
//     stringstream ss(line);
//     std::array<double, 9> coords;
//     for(double& c : coords) ss >> c;
//     bool sig; ss >> sig;

//     m_samples.push_back(FBDTTrainSample<9>(coords, sig));
//   }

//   B2INFO("Read in " << m_samples.size() << " training samples.");
// }

// void FastBDTClassifierTrainingModule::writeSamplesToStream(std::ostream& os) const
// {
//   for (const auto& event : m_samples) {
//     for (const auto& val : event.hits) {
//       os << val << " ";
//     }
//     os << event.signal << std::endl;
//   }
//   B2INFO("Wrote out " << m_samples.size() << " training samples.");
// }
