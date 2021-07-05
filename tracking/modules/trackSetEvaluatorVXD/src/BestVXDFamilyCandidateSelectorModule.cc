/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "tracking/modules/trackSetEvaluatorVXD/BestVXDFamilyCandidateSelectorModule.h"
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

    float qi = sptc.getQualityIndicator();
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

