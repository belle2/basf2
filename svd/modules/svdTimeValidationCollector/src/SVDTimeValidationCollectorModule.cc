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
REG_MODULE(SVDTimeValidationCollector);

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
  addParam("RecoTracksName", m_recotrack, "Name of the RecoTracks list", m_recotrack);
  addParam("TracksName", m_track, "Name of the Tracks list", m_track);
}

void SVDTimeValidationCollectorModule::prepare()
{

  auto hEventT0 = new TH1F("hEventT0", "EventT0", 300, -150, 150);
  registerObject<TH1F>("hEventT0", hEventT0);

  m_svdCls.isRequired(m_svdClusters);
  m_svdClsOnTrk.isRequired(m_svdClustersOnTracks);
  m_eventT0.isRequired(m_eventTime);
  m_recoTrk.isRequired(m_recotrack);
  m_trk.isRequired(m_track);

  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();
  std::vector<Belle2::VxdID> allSensors;
  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD))
    for (auto ladder : geoCache.getLadders(layer))
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder))
        allSensors.push_back(sensor);

  int numberOfSensorBin = 2 * int(allSensors.size());
  B2INFO("Number of SensorBin: " << numberOfSensorBin);

  TH2F* __hClsTimeOnTracks__     = new TH2F("__hClsTimeOnTracks__", "clsTimeOnTracks",
                                            300, -150, 150,
                                            numberOfSensorBin, + 0.5, numberOfSensorBin + 0.5);
  TH2F* __hClsTimeAll__          = new TH2F("__hClsTimeAll__", "clsTimeAll",
                                            300, -150, 150,
                                            numberOfSensorBin, + 0.5, numberOfSensorBin + 0.5);
  TH2F* __hClsDiffTimeOnTracks__ = new TH2F("__hClsDiffTimeOnTracks__", "clsDiffTimeOnTracks",
                                            300, -150, 150,
                                            numberOfSensorBin, + 0.5, numberOfSensorBin + 0.5);
  TH1F* __hBinToSensorMap__     = new TH1F("__hBinToSensorMap__", "__BinToSensorMap__",
                                           numberOfSensorBin, + 0.5, numberOfSensorBin + 0.5);
  __hClsTimeOnTracks__->GetYaxis()->SetTitle("sensor");
  __hClsTimeOnTracks__->GetXaxis()->SetTitle("clsTime_onTracks (ns)");
  __hClsTimeAll__->GetYaxis()->SetTitle("sensor");
  __hClsTimeAll__->GetXaxis()->SetTitle("clsTime_all (ns)");
  __hClsDiffTimeOnTracks__->GetYaxis()->SetTitle("sensor");
  __hClsDiffTimeOnTracks__->GetXaxis()->SetTitle("clsDiffTime_onTracks (ns)");

  int tmpBinCnt = 0;
  for (auto sensor : allSensors) {
    for (auto view : {'U', 'V'}) {
      tmpBinCnt++;
      TString binLabel = TString::Format("L%iL%iS%i%c",
                                         sensor.getLayerNumber(),
                                         sensor.getLadderNumber(),
                                         sensor.getSensorNumber(),
                                         view);
      __hBinToSensorMap__->GetXaxis()->SetBinLabel(tmpBinCnt, binLabel);
    }
  }
  registerObject<TH2F>(__hClsTimeOnTracks__->GetName(), __hClsTimeOnTracks__);
  registerObject<TH2F>(__hClsTimeAll__->GetName(), __hClsTimeAll__);
  registerObject<TH2F>(__hClsDiffTimeOnTracks__->GetName(), __hClsDiffTimeOnTracks__);
  registerObject<TH1F>(__hBinToSensorMap__->GetName(), __hBinToSensorMap__);
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

      TString binLabel = TString::Format("L%iL%iS%i%c",
                                         svdCluster.getSensorID().getLayerNumber(),
                                         svdCluster.getSensorID().getLadderNumber(),
                                         svdCluster.getSensorID().getSensorNumber(),
                                         svdCluster.isUCluster() ? 'U' : 'V');
      int sensorBin = getObjectPtr<TH1F>("__hBinToSensorMap__")->GetXaxis()->FindBin(binLabel.Data());
      double sensorBinCenter = getObjectPtr<TH1F>("__hBinToSensorMap__")->GetXaxis()->GetBinCenter(sensorBin);

      getObjectPtr<TH2F>("__hClsTimeOnTracks__")->Fill(clTime, sensorBinCenter);
      getObjectPtr<TH2F>("__hClsDiffTimeOnTracks__")->Fill(clTime - eventT0, sensorBinCenter);
    };

    // Fill histograms with all clusters
    for (const auto& svdCluster : m_svdCls) {
      // get cluster time
      float clTime = svdCluster.getClsTime();

      TString binLabel = TString::Format("L%iL%iS%i%c",
                                         svdCluster.getSensorID().getLayerNumber(),
                                         svdCluster.getSensorID().getLadderNumber(),
                                         svdCluster.getSensorID().getSensorNumber(),
                                         svdCluster.isUCluster() ? 'U' : 'V');
      int sensorBin = getObjectPtr<TH1F>("__hBinToSensorMap__")->GetXaxis()->FindBin(binLabel.Data());
      double sensorBinCenter = getObjectPtr<TH1F>("__hBinToSensorMap__")->GetXaxis()->GetBinCenter(sensorBin);

      getObjectPtr<TH2F>("__hClsTimeAll__")->Fill(clTime, sensorBinCenter);
    };
  }
}
