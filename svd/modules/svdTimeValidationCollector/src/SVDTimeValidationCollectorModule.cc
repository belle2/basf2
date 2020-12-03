/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Corona, Giulia Casarosa, Giulio Dujany             *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <svd/modules/svdTimeValidationCollector/SVDTimeValidationCollectorModule.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDTimeValidationCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDTimeValidationCollectorModule::SVDTimeValidationCollectorModule() : CalibrationCollectorModule()
{
  //Set module properties

  setDescription("Collector module used to create the histograms needed for the SVD 6-Sample CoG, 3-Sample CoG and 3-Sample ELS Time calibration");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDClustersName", m_svdClusters, "Name of the SVDClusters list", m_svdClusters);
  addParam("SVDClustersOnTracksName", m_svdClustersOnTracks, "Name of the SVDClustersOnTracks list", m_svdClustersOnTracks);
  addParam("EventT0Name", m_eventTime, "Name of the EventT0 list", m_eventTime);
  addParam("SVDEventInfoName", m_svdEventInfo, "Name of the SVDEventInfo list", m_svdEventInfo);
  addParam("RecoTracksName", m_recotrack, "Name of the RecoTracks list", m_recotrack);
  addParam("TracksName", m_track, "Name of the Tracks list", m_track);
}

void SVDTimeValidationCollectorModule::prepare()
{

  TH1F hClsTimeOnTracks("clsTimeOnTracks__L@layerL@ladderS@sensor@view",
                        "clsTimeOnTracks in @layer.@ladder.@sensor @view/@side",
                        300, -150, 150);
  hClsTimeOnTracks.GetXaxis()->SetTitle("clsTime_onTracks (ns)");
  m_hClsTimeOnTracks = make_unique<SVDHistograms<TH1F>>(hClsTimeOnTracks);

  TH1F hClsTimeAll("clsTimeAll__L@layerL@ladderS@sensor@view",
                   "clsTimeAll in @layer.@ladder.@sensor @view/@side",
                   300, -150, 150);
  hClsTimeAll.GetXaxis()->SetTitle("clsTime_all (ns)");
  m_hClsTimeAll = make_unique<SVDHistograms<TH1F>>(hClsTimeAll);

  TH1F hClsDiffTimeOnTracks("clsDiffTimeOnTracks__L@layerL@ladderS@sensor@view",
                            "clsDiffTimeOnTracks in @layer.@ladder.@sensor @view/@side",
                            300, -150, 150);
  hClsDiffTimeOnTracks.GetXaxis()->SetTitle("clsDiffTime_onTracks (ns)");
  m_hClsDiffTimeOnTracks = make_unique<SVDHistograms<TH1F>>(hClsDiffTimeOnTracks);

  auto hEventT0 = new TH1F("hEventT0", "EventT0", 300, -150, 150);
  registerObject<TH1F>("hEventT0", hEventT0);

  m_svdCls.isRequired(m_svdClusters);
  m_svdClsOnTrk.isRequired(m_svdClustersOnTracks);
  m_eventT0.isRequired(m_eventTime);
  m_svdEI.isRequired(m_svdEventInfo);
  m_recoTrk.isRequired(m_recotrack);
  m_trk.isRequired(m_track);

  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
    for (auto ladder : geoCache.getLadders(layer)) {
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder)) {
        for (int view = SVDHistograms<TH1F>::VIndex ; view < SVDHistograms<TH1F>::UIndex + 1; view++) {
          registerObject<TH1F>(m_hClsTimeOnTracks->getHistogram(sensor, view)->GetName(), m_hClsTimeOnTracks->getHistogram(sensor, view));
          registerObject<TH1F>(m_hClsTimeAll->getHistogram(sensor, view)->GetName(), m_hClsTimeAll->getHistogram(sensor, view));
          registerObject<TH1F>(m_hClsDiffTimeOnTracks->getHistogram(sensor, view)->GetName(), m_hClsDiffTimeOnTracks->getHistogram(sensor,
                               view));
        }
      }
    }
  }
}

void SVDTimeValidationCollectorModule::startRun()
{
  // Probably not needed, see discussion in https://stash.desy.de/projects/B2/repos/software/pull-requests/7162/overview

  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
    for (auto ladder : geoCache.getLadders(layer)) {
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder)) {
        for (int view = SVDHistograms<TH1F>::VIndex ; view < SVDHistograms<TH1F>::UIndex + 1; view++) {
          getObjectPtr<TH1F>(m_hClsTimeOnTracks->getHistogram(sensor, view)->GetName())->Reset();
          getObjectPtr<TH1F>(m_hClsTimeAll->getHistogram(sensor, view)->GetName())->Reset();
          getObjectPtr<TH1F>(m_hClsDiffTimeOnTracks->getHistogram(sensor, view)->GetName())->Reset();
        }
      }
    }
  }
}

void SVDTimeValidationCollectorModule::collect()
{
  float eventT0 = 0;
  if (m_eventT0->hasEventT0()) {
    eventT0 = m_eventT0->getEventT0();
    getObjectPtr<TH1F>("hEventT0")->Fill(eventT0);

    // Fill histograms clusters on tracks
    for (const auto& svdCluster : m_svdClsOnTrk) {
      // get cluster time
      float clTime = svdCluster.getClsTime();

      //get cluster side
      int side = svdCluster.isUCluster();

      //get VxdID
      VxdID::baseType theVxdID = (VxdID::baseType)svdCluster.getSensorID();

      getObjectPtr<TH1F>(m_hClsTimeOnTracks->getHistogram(theVxdID, side)->GetName())->Fill(clTime);
      getObjectPtr<TH1F>(m_hClsDiffTimeOnTracks->getHistogram(theVxdID, side)->GetName())->Fill(clTime - eventT0);
    };

    // Fill histograms with all clusters
    for (const auto& svdCluster : m_svdCls) {
      // get cluster time
      float clTime = svdCluster.getClsTime();

      //get cluster side
      int side = svdCluster.isUCluster();

      //get VxdID
      VxdID::baseType theVxdID = (VxdID::baseType)svdCluster.getSensorID();

      getObjectPtr<TH1F>(m_hClsTimeAll->getHistogram(theVxdID, side)->GetName())->Fill(clTime);
    };
  }
}
