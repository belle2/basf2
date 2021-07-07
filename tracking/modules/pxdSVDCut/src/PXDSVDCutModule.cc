/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "../include/PXDSVDCutModule.h"

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
