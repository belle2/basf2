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
  addParam("CreateNewStoreArray", m_createNewStoreArray,
           "If True copy selected SpacePoints to new StoreArray, if False deactivate remaining SpacePoints.", bool(false));
  addParam("NewNameSpacePointTrackCands", m_newNameSpacePointTrackCands,
           "Only required if 'CreateNewStoreArray' is true. Name of StoreArray to store the subset.", std::string("BestSpacePointTrackCands"));
}

void VXDTrackCandidatesQualityIndexCutterModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_nameSpacePointTrackCands);
  if (m_createNewStoreArray) {
    m_newSpacePointTrackCands.registerInDataStore(m_newNameSpacePointTrackCands, DataStore::c_DontWriteOut);
  }
}

void VXDTrackCandidatesQualityIndexCutterModule::event()
{
  if (m_createNewStoreArray) copyCandidates();
  else deactivateCandidates();
}

void VXDTrackCandidatesQualityIndexCutterModule::deactivateCandidates()
{
  for (SpacePointTrackCand sptc : m_spacePointTrackCands) {
    if (sptc.getQualityIndex() < m_minRequiredQuality) {
      sptc.removeRefereeStatus(SpacePointTrackCand::c_isActive);
    }
  }
}

void VXDTrackCandidatesQualityIndexCutterModule::copyCandidates()
{
  for (SpacePointTrackCand sptc : m_spacePointTrackCands) {
    if (sptc.getQualityIndex() >= m_minRequiredQuality) {
      m_newSpacePointTrackCands.appendNew(sptc);
    }
  }
}
