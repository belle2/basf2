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
  m_featBins.clear(); // clear possibly present feature Binning
  is >> m_featBins;
  m_forest = tmpFastBDT::readForestFromStream(is);
  if (!m_decorrMat.readFromStream(is)) { // for some reason this does not stop if there is no decor matrix
    B2ERROR("Reading in the decorrelation matrix did not work! The decorrelation matrix of this classifier will be set to identity!");
    m_decorrMat = DecorrelationMatrix<9>();
  }
}

template<size_t Ndims>
void FBDTClassifier<Ndims>::writeToStream(std::ostream& os) const
{
  os << m_featBins << std::endl;
  os << m_forest << std::endl;
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
  size_t nSamples = samples.size();
  B2DEBUG(10, "Using for training: nBinCuts: " << nBinCuts << ", with " << Ndims << " features and "  << nSamples << " samples.");

  B2DEBUG(10, "FBDTClassifier::train(): Starting to restructure the data into the format better suited for later use");
  std::array<std::vector<double>, Ndims> data;
  for (const auto& event : samples) {
    for (size_t iSP = 0; iSP < Ndims; ++iSP) {
      data[iSP].push_back(event.hits[iSP]);
    }
  }

  B2DEBUG(10, "FBDTClassifier::train(): Calculating the decorrelation transformation.");
  m_decorrMat.calculateDecorrMatrix(data, false);
  B2DEBUG(10, "FBDTClassifier::train(): Applying decorrelation transformation");
  data = m_decorrMat.decorrelate(data);

  B2DEBUG(10, "FBDTClassifier::train(): Determining the FeatureBinnings");
  std::vector<unsigned int> nBinningLevels;
  m_featBins.clear(); // clear the feature binnings (if present)
  for (auto featureVec : data) {
    m_featBins.push_back(tmpFastBDT::FeatureBinning<double>(nBinCuts, featureVec.begin(), featureVec.end()));
    nBinningLevels.push_back(nBinCuts);
  }

  // have to use the decorrelated data for training!!!
  B2DEBUG(10, "FBDTClassifier::train(): Creating the EventSamples");
  tmpFastBDT::EventSample eventSample(nSamples, Ndims, nBinningLevels);
  for (size_t iS = 0; iS < nSamples; ++iS) {
    std::vector<unsigned> bins(Ndims);
    for (size_t iF = 0; iF < Ndims; ++iF) {
      bins[iF] = m_featBins[iF].ValueToBin(data[iF][iS]);
    }
    eventSample.AddEvent(bins, 1.0, samples[iS].signal);
  }

  B2DEBUG(1, "FBDTClassifier::train(): Training the FastBDT");
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
