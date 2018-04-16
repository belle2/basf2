/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/trackSetEvaluatorVXD/BestVXDTrackCandidatesSelectorModule.h"
#include <framework/logging/Logger.h>
#include <vector>
#include <numeric>

using namespace Belle2;


REG_MODULE(BestVXDTrackCandidatesSelector)

BestVXDTrackCandidatesSelectorModule::BestVXDTrackCandidatesSelectorModule() : Module()
{
  setDescription("Module that selects a subset with a fixed size x out of all SpacePointTrackCandidates. Based on qualityIndicator.");
  setPropertyFlags(c_ParallelProcessingCertified);


  addParam("NameSpacePointTrackCands", m_nameSpacePointTrackCands, "Name of expected StoreArray.", std::string(""));
  addParam("SubsetSize", m_subsetSize, "Target size of selected subset.", (unsigned short)(1000));
  addParam("SubsetCreation", m_subsetCreation,
           "If True copy selected SpacePoints to new StoreArray, if False deactivate remaining SpacePoints.", bool(false));
  addParam("NewNameSpacePointTrackCands", m_newNameSpacePointTrackCands,
           "Only required if 'CreateNewStoreArray' is true. Name of StoreArray to store the subset. If the target name is equal to the source candidates not matching the selection criteria are deleted.",
           std::string("BestSpacePointTrackCands"));
}

void BestVXDTrackCandidatesSelectorModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_nameSpacePointTrackCands);
  if (m_subsetCreation) {
    if (m_newNameSpacePointTrackCands == m_nameSpacePointTrackCands) {
      m_bestCandidates.registerSubset(m_spacePointTrackCands);
    } else {
      m_bestCandidates.registerSubset(m_spacePointTrackCands, m_newNameSpacePointTrackCands);
      m_bestCandidates.inheritAllRelations();
    }
  }
}

void BestVXDTrackCandidatesSelectorModule::event()
{
  if (m_subsetCreation) selectSubset();
  else deactivateCandidates();
}

void BestVXDTrackCandidatesSelectorModule::deactivateCandidates()
{
  const int nTracks = m_spacePointTrackCands.getEntries();
  // define subset
  if (nTracks > m_subsetSize) {
    // sort by lowest -> highest quality index
    std::vector<int> sortedTrackCandIndices = getSortedTrackCandIndices(true);

    // deactivate candidates with lowest quality index until desired subsetSize is reached;
    for (int iTracks = 0; iTracks < nTracks - m_subsetSize; ++iTracks) {
      int iCandidate = sortedTrackCandIndices[iTracks];
      m_spacePointTrackCands[iCandidate]->removeRefereeStatus(SpacePointTrackCand::c_isActive);
    }
  }
}

void BestVXDTrackCandidatesSelectorModule::selectSubset()
{
  const unsigned int nTracks = m_spacePointTrackCands.getEntries();
  // sorting is only required if there are too many candidates
  if (nTracks > m_subsetSize) {
    // sort by highest -> lowest quality index
    std::vector<int> sortedTrackCandIndices = getSortedTrackCandIndices(false);

    // select subset of desired size
    std::set<int> subset(sortedTrackCandIndices.cbegin(), sortedTrackCandIndices.cbegin() + m_subsetSize);
    m_bestCandidates.select([subset](const SpacePointTrackCand * sptc) {return subset.count(sptc->getArrayIndex()) != 0;});
  } else {
    // only need to do something if target StoreArray is different from source StoreArray
    if (m_newNameSpacePointTrackCands != m_nameSpacePointTrackCands) {
      m_bestCandidates.select([](const SpacePointTrackCand*) {return true;});
    }
  }
}

std::vector<int> BestVXDTrackCandidatesSelectorModule::getSortedTrackCandIndices(bool increasing)
{
  // Create an index for all spacePointTrackCandidates in the StoreArray.
  // Should be faster than calling 'getArrayIndex()' on all of them.
  std::vector<int> sortedTrackCandIndices(m_spacePointTrackCands.getEntries());
  std::iota(sortedTrackCandIndices.begin(), sortedTrackCandIndices.end(), 0);

  std::sort(sortedTrackCandIndices.begin(), sortedTrackCandIndices.end(),
  [this, increasing](const int lhs, const int rhs) {
    if (increasing) return m_spacePointTrackCands[lhs]->getQualityIndicator() <
                             m_spacePointTrackCands[rhs]->getQualityIndicator();
    else return m_spacePointTrackCands[lhs]->getQualityIndicator() > m_spacePointTrackCands[rhs]->getQualityIndicator();
  });

  return sortedTrackCandIndices;
}

