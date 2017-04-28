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
  setDescription("Module that selects a subset with a fixed size x out of all SpacePointTrackCandidates. Based on qualityIndex.");

  addParam("NameSpacePointTrackCands", m_nameSpacePointTrackCands, "Name of expected StoreArray.", std::string(""));
  addParam("SubsetSize", m_subsetSize, "Target size of selected subset.", (unsigned short)(200));
  addParam("CreateNewStoreArray", m_createNewStoreArray,
           "If True copy selected SpacePoints to new StoreArray, if False deactivate remaining SpacePoints.", bool(false));
  addParam("NewNameSpacePointTrackCands", m_newNameSpacePointTrackCands,
           "Only required if 'CreateNewStoreArray' is true. Name of StoreArray to store the subset.", std::string("BestSpacePointTrackCands"));
}

void BestVXDTrackCandidatesSelectorModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_nameSpacePointTrackCands);
  if (m_createNewStoreArray) {
    m_newSpacePointTrackCands.registerInDataStore(m_newNameSpacePointTrackCands, DataStore::c_DontWriteOut);
  }
}

void BestVXDTrackCandidatesSelectorModule::event()
{
  if (m_createNewStoreArray) copyCandidates();
  else deactivateCandidates();
}

void BestVXDTrackCandidatesSelectorModule::deactivateCandidates()
{
  const int nTracks = m_spacePointTrackCands.getEntries();
  // define subset
  if (nTracks > m_subsetSize) {
    // sort by lowest -> highest quality index
    std::vector<int> sortedTrackCandIndices(nTracks);
    std::iota(sortedTrackCandIndices.begin(), sortedTrackCandIndices.end(), 0);

    std::sort(sortedTrackCandIndices.begin(), sortedTrackCandIndices.end(),
    [this](const int lhs, const int rhs) {
      return this->m_spacePointTrackCands[lhs]->getQualityIndex() < this->m_spacePointTrackCands[rhs]->getQualityIndex();
    });

    for (int iCandidate = 0; iCandidate < nTracks - m_subsetSize; ++iCandidate) {
      m_spacePointTrackCands[iCandidate]->removeRefereeStatus(SpacePointTrackCand::c_isActive);
    }
  }
}

void BestVXDTrackCandidatesSelectorModule::copyCandidates()
{
  const unsigned int nTracks = m_spacePointTrackCands.getEntries();
  if (nTracks > m_subsetSize) {
    // sort by highest -> lowest quality index
    std::vector<SpacePointTrackCand> sortedTrackCands(m_spacePointTrackCands.begin(), m_spacePointTrackCands.end());

    std::sort(sortedTrackCands.begin(), sortedTrackCands.end(),
    [](const SpacePointTrackCand & lhs, const SpacePointTrackCand & rhs) {
      return lhs.getQualityIndex() > rhs.getQualityIndex();
    });
    for (unsigned int iCand = 0; iCand < m_subsetSize; ++iCand) {
      SpacePointTrackCand sptc = sortedTrackCands.at(iCand);
      m_newSpacePointTrackCands.appendNew(sptc);
    }
  } else {
    for (SpacePointTrackCand sptc : m_spacePointTrackCands) {
      m_newSpacePointTrackCands.appendNew(sptc);
    }
  }
}
