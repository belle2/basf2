/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "tracking/modules/trackSetEvaluatorVXD/AddVXDTrackCandidateSubSetsModule.h"
#include <vector>

using namespace Belle2;


REG_MODULE(AddVXDTrackCandidateSubSets)

AddVXDTrackCandidateSubSetsModule::AddVXDTrackCandidateSubSetsModule() : Module()
{
  setDescription("Module that creates additional candidates that each miss a different SpacePoint.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("NameSpacePointTrackCands", m_nameSpacePointTrackCands, "Name of expected StoreArray.", m_nameSpacePointTrackCands);
  addParam("MinOriginalSpacePoints", m_minOriginalSpacePoints,
           "Minimal number of SpacePoints required for the original SpacePointTrackCandidate to create subsets from it."
           " Should be at least 4, so that the subsets have 3 SpacePoints.",
           m_minOriginalSpacePoints);
}

void AddVXDTrackCandidateSubSetsModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_nameSpacePointTrackCands);
}

void AddVXDTrackCandidateSubSetsModule::event()
{
  const unsigned int nTracks = m_spacePointTrackCands.getEntries();
  std::vector<int> trackCandIndices;
  trackCandIndices.reserve(nTracks);
  for (auto& sptc : m_spacePointTrackCands) {
    if (not sptc.hasRefereeStatus(SpacePointTrackCand::c_isActive)) continue;
    trackCandIndices.push_back(sptc.getArrayIndex());
  }

  for (int iCand : trackCandIndices) {
    addSubCandidates(iCand);
  }
}

void AddVXDTrackCandidateSubSetsModule::addSubCandidates(int iCand)
{
  auto sptc = m_spacePointTrackCands[iCand];
  int nHits = sptc->getNHits();

  // minimum length of subset track candidate is 3, thus original track should have at least 4
  if (nHits < m_minOriginalSpacePoints) return;

  for (int iHit = 0; iHit < nHits; ++iHit) {
    std::vector<const SpacePoint*> tmp = sptc->getHits();
    tmp.erase(tmp.begin() + iHit);

    m_sptcCreator.createSPTC(m_spacePointTrackCands, tmp, sptc->getFamily());
  }
}

