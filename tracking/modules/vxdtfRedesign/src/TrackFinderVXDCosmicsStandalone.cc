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
           "SpacePoints collection name", std::string(""));

  addParam("QualityCut", m_qualityCut,
           "Cut value to be applied to chi2 of fit.", m_qualityCut);

  addParam("MinSPs", m_minSPs,
           "Minimal number of SpacePoints for a candidate.", m_minSPs);

  addParam("MaxRejectedSPs", m_maxRejectedSPs,
           "Maximal number of rejected SPs in the event.", m_maxRejectedSPs);
}


void TrackFinderVXDCosmicsStandaloneModule::initialize()
{
  m_spacePoints.isRequired(m_spacePointsName);
  m_TCs.registerInDataStore(m_PARAMSpacePointTrackCandArrayName, DataStore::c_DontWriteOut);
}


void TrackFinderVXDCosmicsStandaloneModule::event()
{
  m_eventCounter++;
  B2WARNING("Looking at event " << m_eventCounter << ".\n");

  m_SCC.addSpacePoints(m_spacePoints);

  if (m_SCC.doFit(m_qualityCut, m_maxRejectedSPs, m_minSPs)) {
    std::pair<std::vector<double>, std::vector<double>> res = m_SCC.getResult();

    auto sptc = SpacePointTrackCand(m_SCC.getSPTC());
    sptc.setChargeSeed(1.);
    sptc.setQualityIndex(m_SCC.getChi2());

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
    B2WARNING("new SPTC with nhits = " << sptc.getNHits() << " and chi2 = " << sptc.getQualityIndex() << ".");
    m_TCs.appendNew(sptc);
  }
}
