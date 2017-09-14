/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/trackSetEvaluatorVXD/BestVXDFamilyCandidateSelectorModule.h"
#include <framework/logging/Logger.h>
#include <vector>
#include <numeric>
#include <unordered_map>

using namespace Belle2;


REG_MODULE(BestVXDFamilyCandidateSelector)

BestVXDFamilyCandidateSelectorModule::BestVXDFamilyCandidateSelectorModule() : Module()
{
  setDescription("Module that selects the best candidate for each SPTC family");
  setPropertyFlags(c_ParallelProcessingCertified);


  addParam("NameSpacePointTrackCands", m_nameSpacePointTrackCands, "Name of expected StoreArray.", std::string(""));
}

void BestVXDFamilyCandidateSelectorModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_nameSpacePointTrackCands);
}

void BestVXDFamilyCandidateSelectorModule::event()
{
  /* family identifier -> (sptc array index, quality index) */
  std::unordered_map<unsigned short, std::pair<int, float>> familyToCand;
  for (SpacePointTrackCand& sptc : m_spacePointTrackCands) {
    if (not sptc.hasRefereeStatus(SpacePointTrackCand::c_isActive)) continue;

    float qi = sptc.getQualityIndex();
    unsigned short family = sptc.getFamily();
    auto iter = familyToCand.find(family);
    if (iter != familyToCand.end()) {
      auto& entry = iter->second;
      if (entry.second < qi) {
        entry.first = sptc.getArrayIndex();
        entry.second = qi;
      }
    } else {
      familyToCand.emplace(std::piecewise_construct, std::forward_as_tuple(family), std::forward_as_tuple(sptc.getArrayIndex(), qi));
    }
  }
  for (SpacePointTrackCand& sptc : m_spacePointTrackCands) {
    if (not sptc.hasRefereeStatus(SpacePointTrackCand::c_isActive)) continue;
    int bestCandIndex = familyToCand.at(sptc.getFamily()).first;
    if (sptc.getArrayIndex() != bestCandIndex) {
      sptc.removeRefereeStatus(SpacePointTrackCand::c_isActive);
    }
  }
}

