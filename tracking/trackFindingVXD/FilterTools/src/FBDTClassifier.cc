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
#include <framework/logging/Logger.h>

// for development! change this to externals version!
#include <tracking/trackFindingVXD/tmpFastBDT/IO.h>

using namespace Belle2;
using namespace std;

template<size_t Ndims>
void FBDTClassifier<Ndims>::readFromStream(std::istream& is)
{
  m_forest = tmpFastBDT::readForestFromStream(is);
  m_featBins.clear(); // clear possibly present feature Binning
  is >> m_featBins;
  if (!m_decorrMat.readFromStream(is)) {
    B2WARNING("Reading in the decorrelation matrix did not work! The decorrelation matrix of this classifier will be set to identity!");
    m_decorrMat = DecorrelationMatrix<9>();
  }
}

template<size_t Ndims>
void FBDTClassifier<Ndims>::writeToStream(std::ostream& os) const
{
  os << m_forest << std::endl;
  os << m_featBins << std::endl;
  os << m_decorrMat.print() << std::endl;
}

template<size_t Ndims>
void FBDTClassifier<Ndims>::train(const std::vector<Belle2::FBDTTrainSample<Ndims> >& samples,
                                  int nTrees, int depth, double shrinkage, double ratio)
{
  if (samples.empty()) {
    B2ERROR("No samples passed for training a FBDTClassifier.");
    return;
  }

  unsigned int nBinCuts = 8;
  B2DEBUG(100, "Using for training: nBinCuts: " << nBinCuts << ", with " << Ndims << " features.");

  B2DEBUG(100, "FBDTClassifier::train(): Starting to restructure the data into the format better suited for later use");

  // restructure data into more suitable format
  std::array<std::vector<double>, Ndims> data;
  for (const auto& event : samples) {
    for (size_t iSP = 0; iSP < Ndims; ++iSP) {
      data[iSP].push_back(event.hits[iSP]);
    }
  }

  B2DEBUG(100, "FBDTClassifier::train(): Calculating the decorrelation transformation.");
  m_decorrMat.calculateDecorrMatrix(data, true);
  B2DEBUG(100, "FBDTClassifier::train(): Applying decorrelation transformation");
  data = m_decorrMat.decorrelate(data);

  B2DEBUG(100, "FBDTClassifier::train(): Determining the FeatureBinnings");
  std::vector<unsigned int> nBinningLevels;
  m_featBins.clear(); // clear the feature binnings (if present)
  for (auto dataIt = data.begin(); dataIt != data.end(); ++dataIt) {
    m_featBins.push_back(tmpFastBDT::FeatureBinning<double>(nBinCuts, dataIt->begin(), dataIt->end()));
    nBinningLevels.push_back(nBinCuts);
  }

  B2DEBUG(100, "FBDTClassifier::train(): Creating the EventSamples");
  tmpFastBDT::EventSample eventSample(samples.size(), Ndims, nBinningLevels);
  for (const auto& event : samples) {
    std::vector<unsigned> bins(Ndims);
    for (size_t iSP = 0; iSP < Ndims; ++iSP) {
      bins[iSP] = m_featBins[iSP].ValueToBin(event.hits[iSP]);
    }
    eventSample.AddEvent(bins, 1, event.signal);
  }

  B2DEBUG(100, "FBDTClassifier::train(): Training the FastBDT");
  tmpFastBDT::ForestBuilder fbdt(eventSample, nTrees, shrinkage, ratio, depth); // train FastBDT

  B2DEBUG(100, "FBDTClassifier::train(): getting FastBDT to internal member");
  tmpFastBDT::Forest forest(fbdt.GetF0(), fbdt.GetShrinkage());
  for (const auto& tree : fbdt.GetForest()) {
    forest.AddTree(tree);
  }

  m_forest = forest; // check if this can be done better with move or something similar
}

// explicit instantiation for SpacePoint in order to have .h and .cc file separated
template class Belle2::FBDTClassifier<9>;
