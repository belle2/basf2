/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
  auto _hClsTimeOnTracks = new  SVDHistograms<TH1F>(hClsTimeOnTracks);

  TH1F hClsTimeAll("clsTimeAll__L@layerL@ladderS@sensor@view",
                   "clsTimeAll in @layer.@ladder.@sensor @view/@side",
                   300, -150, 150);
  hClsTimeAll.GetXaxis()->SetTitle("clsTime_all (ns)");
  auto _hClsTimeAll = new SVDHistograms<TH1F>(hClsTimeAll);

  TH1F hClsDiffTimeOnTracks("clsDiffTimeOnTracks__L@layerL@ladderS@sensor@view",
                            "clsDiffTimeOnTracks in @layer.@ladder.@sensor @view/@side",
                            300, -150, 150);
  hClsDiffTimeOnTracks.GetXaxis()->SetTitle("clsDiffTime_onTracks (ns)");
  auto _hClsDiffTimeOnTracks = new SVDHistograms<TH1F>(hClsDiffTimeOnTracks);

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
          registerObject<TH1F>(_hClsTimeOnTracks->getHistogram(sensor, view)->GetName(), _hClsTimeOnTracks->getHistogram(sensor, view));
          registerObject<TH1F>(_hClsTimeAll->getHistogram(sensor, view)->GetName(), _hClsTimeAll->getHistogram(sensor, view));
          registerObject<TH1F>(_hClsDiffTimeOnTracks->getHistogram(sensor, view)->GetName(), _hClsDiffTimeOnTracks->getHistogram(sensor,
                               view));
        }
      }
    }
  }
}


void SVDTimeValidationCollectorModule::collect()
{
  if (m_eventT0->hasEventT0()) {
    float eventT0 = m_eventT0->getEventT0();
    getObjectPtr<TH1F>("hEventT0")->Fill(eventT0);

    // Fill histograms clusters on tracks
    for (const auto& svdCluster : m_svdClsOnTrk) {
      // get cluster time
      float clTime = svdCluster.getClsTime();

      //get cluster layer, ladder sensor and side
      auto theVxdID = svdCluster.getSensorID();
      auto layer_num = theVxdID.getLayerNumber();
      auto ladder_num = theVxdID.getLadderNumber();
      auto sensor_num = theVxdID.getSensorNumber();
      char side = 'V';
      if (svdCluster.isUCluster())
        side = 'U';

      auto hClsTimeOnTracks_name = Form("clsTimeOnTracks__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side);

      auto hClsDiffTimeOnTracks_name = Form("clsDiffTimeOnTracks__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side);

      getObjectPtr<TH1F>(hClsTimeOnTracks_name)->Fill(clTime);
      getObjectPtr<TH1F>(hClsDiffTimeOnTracks_name)->Fill(clTime - eventT0);
    };

    // Fill histograms with all clusters
    for (const auto& svdCluster : m_svdCls) {
      // get cluster time
      float clTime = svdCluster.getClsTime();

      //get cluster layer, ladder sensor and side
      auto theVxdID = svdCluster.getSensorID();
      auto layer_num = theVxdID.getLayerNumber();
      auto ladder_num = theVxdID.getLadderNumber();
      auto sensor_num = theVxdID.getSensorNumber();
      char side = 'V';
      if (svdCluster.isUCluster())
        side = 'U';

      auto hClsTimeAll_name = Form("clsTimeAll__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side);

      getObjectPtr<TH1F>(hClsTimeAll_name)->Fill(clTime);
    };
  }
}
