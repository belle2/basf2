/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Metzner                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <tracking/modules/vxdtfRedesign/TrackFinderVXDCosmicsStandalone.h>

using namespace Belle2;

REG_MODULE(TrackFinderVXDCosmicsStandalone);

/**
 * The TrackFinderVXDCosmicsStandaloneModule
 */
TrackFinderVXDCosmicsStandaloneModule::TrackFinderVXDCosmicsStandaloneModule() : Module()
{
  //Set module properties
  setDescription("The TrackFinderVXDCosmicsStandalone module.");
  setPropertyFlags(c_ParallelProcessingCertified);


  addParam("SpacePointTrackCandArrayName",
           m_PARAMSpacePointTrackCandArrayName,
           "name for StoreArray< SpacePointTrackCand> to be filled.",
           std::string(""));

  addParam("SpacePoints", m_spacePointsName,
           "SpacePoints collection name", m_spacePointsName);

  addParam("QualityCut", m_qualityCut,
           "Cut value to be applied to reduced chi2 of fit.", m_qualityCut);

  addParam("MinSPs", m_minSPs,
           "Minimal number of SpacePoints for a candidate.", m_minSPs);

  addParam("MaxRejectedSPs", m_maxRejectedSPs,
           "Maximal number of rejected SPs in the event.", m_maxRejectedSPs);

  addParam("SortingMode", m_sortingMode,
           "Sorting mode used when adding new SPs.\n 1: by radius,\n 2: by x,\n 3: by y (default).", m_sortingMode);
}


void TrackFinderVXDCosmicsStandaloneModule::initialize()
{
  for (unsigned i = 0; i < m_spacePointsName.size(); i++) {
    m_spacePoints.push_back(StoreArray<SpacePoint>(m_spacePointsName[i]));
    m_spacePoints[m_spacePoints.size() - 1].isRequired();
  }
  m_TCs.registerInDataStore(m_PARAMSpacePointTrackCandArrayName, DataStore::c_DontWriteOut);
  m_SCC.setSortingMode(m_sortingMode);
}


void TrackFinderVXDCosmicsStandaloneModule::event()
{
  m_SCC.addSpacePoints(m_spacePoints);

  if (m_SCC.doFit(m_qualityCut, m_maxRejectedSPs, m_minSPs)) {
    std::pair<std::vector<double>, std::vector<double>> res = m_SCC.getResult();

    auto sptc = SpacePointTrackCand(m_SCC.getSPTC());
    sptc.setChargeSeed(1.);
    sptc.setQualityIndicator(m_SCC.getReducedChi2());

    TMatrixDSym covSeed(6);
    covSeed(0, 0) = 0.01;
    covSeed(1, 1) = 0.01;
    covSeed(2, 2) = 0.04;
    covSeed(3, 3) = 0.1;
    covSeed(4, 4) = 0.1;
    covSeed(5, 5) = 0.4;
    sptc.setCovSeed(covSeed);

    TVectorD stateSeed6D(6);
    double momentumFactor = 100.;
    stateSeed6D(0) = res.first[0];
    stateSeed6D(1) = res.first[1];
    stateSeed6D(2) = res.first[2];
    stateSeed6D(3) = momentumFactor * res.second[0];
    stateSeed6D(4) = momentumFactor * res.second[1];
    stateSeed6D(5) = momentumFactor * res.second[2];

    sptc.set6DSeed(stateSeed6D);
    B2DEBUG(10, "new SPTC with nhits = " << sptc.getNHits() << " and reduced chi2 = " << sptc.getQualityIndicator() << ".");
    m_TCs.appendNew(sptc);
  }
}
