/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/trackSetEvaluatorVXD/AddVXDTrackCandidateSubSetsModule.h"
#include <framework/logging/Logger.h>
#include <vector>
#include <numeric>

using namespace Belle2;


REG_MODULE(AddVXDTrackCandidateSubSets)

AddVXDTrackCandidateSubSetsModule::AddVXDTrackCandidateSubSetsModule() : Module()
{
  setDescription("Module that creates additional candidates that each miss a different SpacePoint.");

  addParam("NameSpacePointTrackCands", m_nameSpacePointTrackCands, "Name of expected StoreArray.", std::string(""));
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
  // minimum length of track candidate is 3
  if (nHits < 4) return;

  for (int iHit = 0; iHit < nHits; ++iHit) {
    std::vector<const SpacePoint*> tmp = sptc->getHits();
    tmp.erase(tmp.begin() + iHit);

    m_sptcCreator(m_spacePointTrackCands, tmp, sptc->getFamily());
  }
}

