/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/modules/svdTimeShifterCollector/SVDTimeShifterCollectorModule.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDTimeShifterCollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDTimeShifterCollectorModule::SVDTimeShifterCollectorModule() : CalibrationCollectorModule()
{
  //Set module properties

  setDescription("Collector module used to create the histograms needed for the SVD 6-Sample CoG, 3-Sample CoG and 3-Sample ELS Time calibration");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("MaxClusterSize", m_maxClusterSize, "Maximum size of SVD clusters", m_maxClusterSize);
  addParam("SVDClustersOnTracksName", m_svdClustersOnTracks, "Name of the SVDClustersOnTracks list", m_svdClustersOnTracks);
  addParam("EventT0Name", m_eventTime, "Name of the EventT0 list", m_eventTime);
}

void SVDTimeShifterCollectorModule::prepare()
{

  m_svdClsOnTrk.isRequired(m_svdClustersOnTracks);
  m_eventT0.isRequired(m_eventTime);

  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

  std::vector<Belle2::VxdID> allSensors;
  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD))
    for (auto ladder : geoCache.getLadders(layer))
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder))
        allSensors.push_back(sensor);

  int numberOfSensorBin = 2 * int(allSensors.size());
  B2INFO("Number of SensorBin: " << numberOfSensorBin);

  TH3F* __hClusterSizeVsTimeResidual__  = new TH3F("__hClusterSizeVsTimeResidual__", "ClusterSize vs Time Residual",
                                                   100, -25., 25., m_maxClusterSize, 0.5, m_maxClusterSize + 0.5,
                                                   numberOfSensorBin, + 0.5, numberOfSensorBin + 0.5);
  TH1F* __hBinToSensorMap__             = new TH1F("__hBinToSensorMap__", "__BinToSensorMap__",
                                                   numberOfSensorBin, + 0.5, numberOfSensorBin + 0.5);
  __hClusterSizeVsTimeResidual__->GetZaxis()->SetTitle("Sensor");
  __hClusterSizeVsTimeResidual__->GetYaxis()->SetTitle("Cluster Size");
  __hClusterSizeVsTimeResidual__->GetXaxis()->SetTitle("Cluster Time - EventT0 (ns)");

  int tmpBinCnt = 0;
  for (auto sensor : allSensors) {
    for (auto view : {'U', 'V'}) {
      tmpBinCnt++;
      TString binLabel = TString::Format("L%iS%iS%c",
                                         sensor.getLayerNumber(),
                                         sensor.getSensorNumber(),
                                         view);
      __hBinToSensorMap__->GetXaxis()->SetBinLabel(tmpBinCnt, binLabel);
    }
  }
  registerObject<TH3F>(__hClusterSizeVsTimeResidual__->GetName(), __hClusterSizeVsTimeResidual__);
  registerObject<TH1F>(__hBinToSensorMap__->GetName(), __hBinToSensorMap__);
}

void SVDTimeShifterCollectorModule::startRun()
{
  getObjectPtr<TH3F>("__hClusterSizeVsTimeResidual__")->Reset();
}

void SVDTimeShifterCollectorModule::collect()
{
  if (m_eventT0->hasEventT0()) {
    float eventT0 = m_eventT0->getEventT0();
    getObjectPtr<TH1F>("hEventT0")->Fill(eventT0);

    // Fill histograms clusters on tracks
    for (const auto& svdCluster : m_svdClsOnTrk) {
      // get cluster time
      float clTime = svdCluster.getClsTime();
      float clSize = svdCluster.getSize();

      TString binLabel = TString::Format("L%iS%iS%c",
                                         svdCluster.getSensorID().getLayerNumber(),
                                         svdCluster.getSensorID().getSensorNumber(),
                                         svdCluster.isUCluster() ? 'U' : 'V');
      int sensorBin = getObjectPtr<TH1F>("__hBinToSensorMap__")->GetXaxis()->FindBin(binLabel.Data());
      double sensorBinCenter = getObjectPtr<TH1F>("__hBinToSensorMap__")->GetXaxis()->GetBinCenter(sensorBin);

      getObjectPtr<TH3F>("__hClusterSizeVsTimeResidual__")->Fill(clTime - eventT0, clSize, sensorBinCenter);
    };
  }
}
