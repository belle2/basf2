/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/trackFindingVXD/filterTools/FBDTClassifier.h>
#include <framework/logging/Logger.h>

#if FastBDT_VERSION_MAJOR >= 3
#include <FastBDT_IO.h>
#else
#include <IO.h>
#endif

using namespace Belle2;
using namespace std;

template<size_t Ndims>
void FBDTClassifier<Ndims>::readFromStream(std::istream& is)
{
  m_featBins.clear(); // clear possibly present feature Binning
  B2DEBUG(50, "Reading the FeatureBinnings");
  is >> m_featBins;
  B2DEBUG(50, "Reading the Forest");
#if FastBDT_VERSION_MAJOR >= 3
  m_forest = FastBDT::readForestFromStream<unsigned int>(is);
#else
  m_forest = FastBDT::readForestFromStream(is);
#endif
  B2DEBUG(50, "Reading the DecorrelationMatrix");
  if (!m_decorrMat.readFromStream(is)) { // for some reason this does not stop if there is no decor matrix
    B2ERROR("Reading in the decorrelation matrix did not work! The decorrelation matrix of this classifier will be set to identity!");
    m_decorrMat = DecorrelationMatrix<9>();
  }
}

template<size_t Ndims>
void FBDTClassifier<Ndims>::writeToStream(std::ostream& os) const
{
  B2DEBUG(50, "Reading the FeatureBinnings");
  os << m_featBins << std::endl;
  B2DEBUG(50, "Reading the Forest");
  os << m_forest << std::endl;
  B2DEBUG(50, "Reading the DecorrelationMatrix");
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
#if FastBDT_VERSION_MAJOR >= 3
    m_featBins.push_back(FastBDT::FeatureBinning<double>(nBinCuts, featureVec));
#else
    m_featBins.push_back(FastBDT::FeatureBinning<double>(nBinCuts, featureVec.begin(), featureVec.end()));
#endif
    nBinningLevels.push_back(nBinCuts);
  }

  // have to use the decorrelated data for training!!!
  B2DEBUG(10, "FBDTClassifier::train(): Creating the EventSamples");
  FastBDT::EventSample eventSample(nSamples, Ndims, nBinningLevels);
  for (size_t iS = 0; iS < nSamples; ++iS) {
    std::vector<unsigned> bins(Ndims);
    for (size_t iF = 0; iF < Ndims; ++iF) {
      bins[iF] = m_featBins[iF].ValueToBin(data[iF][iS]);
    }
    eventSample.AddEvent(bins, 1.0, samples[iS].signal);
  }

  B2DEBUG(1, "FBDTClassifier::train(): Training the FastBDT");
  FastBDT::ForestBuilder fbdt(eventSample, nTrees, shrinkage, ratio, depth); // train FastBDT

  B2DEBUG(100, "FBDTClassifier::train(): getting FastBDT to internal member");
#if FastBDT_VERSION_MAJOR >= 3
  FBDTForest forest(fbdt.GetF0(), fbdt.GetShrinkage(), true);
#else
  FBDTForest forest(fbdt.GetF0(), fbdt.GetShrinkage());
#endif
  for (const auto& tree : fbdt.GetForest()) {
    forest.AddTree(tree);
  }

  m_forest = forest; // check if this can be done better with move or something similar
}

// explicit instantiation for SpacePoint in order to have .h and .cc file separated
template class Belle2::FBDTClassifier<9>;
