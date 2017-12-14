/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sebastian Racs                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/PXDSVDCutModule.h"
#include <framework/logging/Logger.h>

#include <vector>

using namespace Belle2;


REG_MODULE(PXDSVDCut)

PXDSVDCutModule::PXDSVDCutModule() : Module()
{
  // Set module properties
  setDescription("The module to deactivate the SpacePointTrackCandidates with less than *minSVDSPs* SVD SpacePoints.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("minSVDSPs", m_minSVDSPs,
           "Minimum number of SVD SpacePoints to keep a SpacePointTrackCandidate", int(3));

  addParam("SpacePointTrackCandsStoreArrayName", m_SpacePointTrackCandsStoreArrayName,
           "Name of StoreArray containing the SpacePointTrackCandidates to be estimated.", std::string(""));
}

void PXDSVDCutModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_SpacePointTrackCandsStoreArrayName);
}

void PXDSVDCutModule::event()
{
  // Check each SpacePointTrackCand for its number of SVD SpacePoints
  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {

    const std::vector<const Belle2::SpacePoint*> sorted_spacepoints = aTC.getSortedHits();
    int n_svd_spacepoints = 0;

    for (auto& spacepoint : sorted_spacepoints)
      if (spacepoint->getType() == VXD::SensorInfoBase::SVD)
        n_svd_spacepoints++;

    if (n_svd_spacepoints < m_minSVDSPs)
      aTC.removeRefereeStatus(SpacePointTrackCand::c_isActive);

  }
}
