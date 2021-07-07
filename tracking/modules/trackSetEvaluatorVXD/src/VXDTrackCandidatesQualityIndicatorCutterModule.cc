/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "tracking/modules/trackSetEvaluatorVXD/VXDTrackCandidatesQualityIndicatorCutterModule.h"
#include <vector>

using namespace Belle2;


REG_MODULE(VXDTrackCandidatesQualityIndicatorCutter)

VXDTrackCandidatesQualityIndicatorCutterModule::VXDTrackCandidatesQualityIndicatorCutterModule() : Module()
{
  setDescription("Module that selects a subset out of all SpacePointTrackCandidates. Based on qualityIndicator requirement.");

  addParam("NameSpacePointTrackCands", m_nameSpacePointTrackCands, "Name of expected StoreArray.", std::string(""));
  addParam("minRequiredQuality", m_minRequiredQuality, "Minimum value of qualityIndicator to keep candidate active.", float(0));
  addParam("SubsetCreation", m_subsetCreation,
           "If True copy selected SpacePoints to new StoreArray, if False deactivate remaining SpacePoints.", bool(false));
  addParam("NewNameSpacePointTrackCands", m_newNameSpacePointTrackCands,
           "Only required if 'CreateNewStoreArray' is true. Name of StoreArray to store the subset. If the target name is equal to the source candidates not matching the selection criteria are deleted.",
           std::string("BestSpacePointTrackCands"));
}

void VXDTrackCandidatesQualityIndicatorCutterModule::initialize()
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

void VXDTrackCandidatesQualityIndicatorCutterModule::event()
{
  if (m_subsetCreation) selectSubset();
  else deactivateCandidates();
}

void VXDTrackCandidatesQualityIndicatorCutterModule::deactivateCandidates()
{
  for (SpacePointTrackCand& sptc : m_spacePointTrackCands) {
    if (sptc.getQualityIndicator() < m_minRequiredQuality) {
      sptc.removeRefereeStatus(SpacePointTrackCand::c_isActive);
      // Note: assignment state of individual hits does not have to be changed here (assignment not set until SPTC2RTConverterModule)
    }
  }
}

void VXDTrackCandidatesQualityIndicatorCutterModule::selectSubset()
{
  m_goodCandidates.select([this](const SpacePointTrackCand * sptc) {return sptc->getQualityIndicator() >= this->m_minRequiredQuality;});
}
