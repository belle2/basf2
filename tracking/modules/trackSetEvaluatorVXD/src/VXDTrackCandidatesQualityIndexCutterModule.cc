/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/trackSetEvaluatorVXD/VXDTrackCandidatesQualityIndexCutterModule.h"
#include <framework/logging/Logger.h>
#include <vector>
#include <numeric>

using namespace Belle2;


REG_MODULE(VXDTrackCandidatesQualityIndexCutter)

VXDTrackCandidatesQualityIndexCutterModule::VXDTrackCandidatesQualityIndexCutterModule() : Module()
{
  setDescription("Module that selects a subset out of all SpacePointTrackCandidates. Based on qualityIndex requirement.");

  addParam("NameSpacePointTrackCands", m_nameSpacePointTrackCands, "Name of expected StoreArray.", std::string(""));
  addParam("minRequiredQuality", m_minRequiredQuality, "Minimum value of qualityIndex to keep candidate active.", float(0));
  addParam("SubsetCreation", m_subsetCreation,
           "If True copy selected SpacePoints to new StoreArray, if False deactivate remaining SpacePoints.", bool(false));
  addParam("NewNameSpacePointTrackCands", m_newNameSpacePointTrackCands,
           "Only required if 'CreateNewStoreArray' is true. Name of StoreArray to store the subset. If the target name is equal to the source candidates not matching the selection criteria are deleted.",
           std::string("BestSpacePointTrackCands"));
}

void VXDTrackCandidatesQualityIndexCutterModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_nameSpacePointTrackCands);
  if (m_subsetCreation) {
    if (m_newNameSpacePointTrackCands == m_nameSpacePointTrackCands) {
      m_goodCandidates.registerSubset(m_spacePointTrackCands);
    } else {
      m_goodCandidates.registerSubset(m_spacePointTrackCands, m_newNameSpacePointTrackCands);
      m_goodCandidates.inheritAllRelations();
    }
  }
}

void VXDTrackCandidatesQualityIndexCutterModule::event()
{
  if (m_subsetCreation) selectSubset();
  else deactivateCandidates();
}

void VXDTrackCandidatesQualityIndexCutterModule::deactivateCandidates()
{
  for (SpacePointTrackCand& sptc : m_spacePointTrackCands) {
    if (sptc.getQualityIndex() < m_minRequiredQuality) {
      sptc.removeRefereeStatus(SpacePointTrackCand::c_isActive);
    }
  }
}

void VXDTrackCandidatesQualityIndexCutterModule::selectSubset()
{
  m_goodCandidates.select([this](const SpacePointTrackCand * sptc) {return sptc->getQualityIndex() >= this->m_minRequiredQuality;});
}
