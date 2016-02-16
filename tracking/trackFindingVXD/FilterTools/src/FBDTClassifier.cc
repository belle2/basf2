/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/trackFindingVXD/FilterTools/FBDTClassifier.h>
#include <algorithm> // std::move
#include <tracking/spacePointCreation/SpacePoint.h> // needed for explicit instantiation

// for development! change this to externals version!
#include <tracking/trackFindingVXD/tmpFastBDT/IO.h>
#include <tracking/trackFindingVXD/FilterTools/TestSpacePoint.h> // temporary

using namespace Belle2;
using namespace std;

template<typename PointType>
void FBDTClassifier<PointType>::readFromStream(std::istream& is)
{
  m_forest = tmpFastBDT::readForestFromStream(is);
  m_featBins.clear(); // clear possibly present feature Binning
  is >> m_featBins;
  if (!m_decorrMat.readFromStream(is)) {
    B2WARNING("Reading in the decorrelation matrix did not work! The decorrelation matrix of this classifier will be set to identity!");
    m_decorrMat = DecorrelationMatrix<9>();
  }
}

template<typename PointType>
void FBDTClassifier<PointType>::writeToStream(std::ostream& os) const
{
  os << m_forest << std::endl;
  os << m_featBins << std::endl;
  os << m_decorrMat.print() << std::endl;
}

template<typename PointType>
void FBDTClassifier<PointType>::train(const std::vector<Belle2::FBDTTrainSample<PointType> >& samples,
                                      int nTrees, int depth, double shrinkage, double ratio)
{
  if (samples.empty()) {
    B2ERROR("No samples passed for training a FBDTClassifier.");
    return;
  }

  unsigned int nFeatures = samples[0].hits.size() * 3; // three coordinates per hit
  unsigned int nBinCuts = 8;

  m_featBins.clear();
  // restructure data into more suitable format
  std::array<std::vector<double>, 9> data; // WARNING: hardcoded at the moment -> COULDDO. use std::vector
  for (const auto& event : samples) {
    for (size_t iSP = 0; iSP < event.hits.size(); ++iSP) {
      data[iSP * 3].push_back(event.hits[iSP]->X());
      data[iSP * 3 + 1].push_back(event.hits[iSP]->Y());
      data[iSP * 3 + 2].push_back(event.hits[iSP]->Z());
    }
  }

  // calculate the appropriate decorrelation matrix and apply the transformation to the data afterwards before training
  m_decorrMat.calculateDecorrMatrix(data, true);
  data = m_decorrMat.decorrelate(data);

  std::vector<unsigned int> nBinningLevels;
  for (auto dataIt = data.begin(); dataIt != data.end(); ++dataIt) {
    m_featBins.push_back(tmpFastBDT::FeatureBinning<double>(nBinCuts, dataIt->begin(), dataIt->end()));
    nBinningLevels.push_back(nBinCuts);
  }

  tmpFastBDT::EventSample eventSample(samples.size(), nFeatures, nBinningLevels);
  for (const auto& event : samples) {
    std::vector<unsigned> bins(nFeatures);
    for (size_t iSP = 0; iSP < event.hits.size(); ++iSP) {
      bins[iSP * 3] = m_featBins[iSP * 3].ValueToBin(event.hits[iSP]->X());
      bins[iSP * 3 + 1] = m_featBins[iSP * 3 + 1].ValueToBin(event.hits[iSP]->Y());
      bins[iSP * 3 + 2] = m_featBins[iSP * 3 + 2].ValueToBin(event.hits[iSP]->Z());
    }
    eventSample.AddEvent(bins, 1, event.signal);
  }

  tmpFastBDT::ForestBuilder fbdt(eventSample, nTrees, shrinkage, ratio, depth); // train FastBDT
  tmpFastBDT::Forest forest(fbdt.GetF0(), fbdt.GetShrinkage());
  for (const auto& tree : fbdt.GetForest()) {
    forest.AddTree(tree);
  }

  m_forest = forest; // check if this can be done better with move or something similar
}

// explicit instantiation for SpacePoint in order to have .h and .cc file separated
template class Belle2::FBDTClassifier<Belle2::SpacePoint>;
template class Belle2::FBDTClassifier<VXDTFFilterTest::TestSpacePoint>; // for tests TODO: make the TestSpacePoint known here
