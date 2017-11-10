/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/qualityEstimatorVXD/QualityEstimatorVXDModule.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h>


using namespace Belle2;


REG_MODULE(QualityEstimatorVXD)

QualityEstimatorVXDModule::QualityEstimatorVXDModule() : QualityEstimatorBaseModule()
{
  setDescription("The quality estimator module for SpacePointTrackCandidates.");
}

void QualityEstimatorVXDModule::beginRun()
{
  QualityEstimatorBaseModule::beginRun();

  if (m_EstimationMethod == "mcInfo") {
    StoreArray<RecoTrack> mcRecoTracks;
    mcRecoTracks.isRequired(m_MCRecoTracksStoreArrayName);
    std::string svdClustersName = ""; std::string pxdClustersName = "";

    if (mcRecoTracks.getEntries() > 0) {
      svdClustersName = mcRecoTracks[0]->getStoreArrayNameOfSVDHits();
      pxdClustersName = mcRecoTracks[0]->getStoreArrayNameOfPXDHits();
    } else {
      B2WARNING("No Entries in mcRecoTracksStoreArray: using empty cluster name for svd and pxd");
    }

    QualityEstimatorMC* MCestimator = static_cast<QualityEstimatorMC*>(m_estimator.get());
    MCestimator->setClustersNames(svdClustersName, pxdClustersName);
  }
}

// assign a QI computed using the selected QualityEstimator for each given SpacePointTrackCand
void QualityEstimatorVXDModule::singleSPTCevent(SpacePointTrackCand& aTC)
{
  double qi = m_estimator->estimateQuality(aTC.getSortedHits());
  aTC.setQualityIndex(qi);
}
