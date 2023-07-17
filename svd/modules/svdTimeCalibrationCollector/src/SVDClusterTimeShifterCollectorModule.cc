/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/modules/svdTimeCalibrationCollector/SVDClusterTimeShifterCollectorModule.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDClusterTimeShifterCollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDClusterTimeShifterCollectorModule::SVDClusterTimeShifterCollectorModule() : CalibrationCollectorModule()
{
  //Set module properties

  setDescription("Collector module used to create the histograms needed for the SVD 6-Sample CoG, 3-Sample CoG and 3-Sample ELS Time calibration");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("MaxClusterSize", m_maxClusterSize, "Maximum size of SVD clusters", m_maxClusterSize);
  addParam("EventT0Name", m_eventTime, "Name of the EventT0 list", m_eventTime);
}

void SVDClusterTimeShifterCollectorModule::prepare()
{

  for (auto alg : m_timeAlgorithms) {
    m_svdClsOnTrk[alg] = StoreArray<SVDCluster>();
    m_svdClsOnTrk[alg].isRequired((m_svdClsOnTrksPrefix + '_' + alg).Data());
  }
  m_eventT0.isRequired(m_eventTime);

  // getting all the svd sensors
  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();
  std::vector<Belle2::VxdID> allSensors;
  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD))
    for (auto ladder : geoCache.getLadders(layer))
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder))
        allSensors.push_back(sensor);

  int numberOfSensorBin = 2 * int(allSensors.size());
  B2INFO("Number of SensorBin: " << numberOfSensorBin);

  for (auto alg : m_timeAlgorithms) {
    TH3F* __hClusterSizeVsTimeResidual__  = new TH3F(("__hClusterSizeVsTimeResidual__" + alg).Data(),
                                                     ("ClusterSize vs " + alg + " Time Residual").Data(),
                                                     100, -25., 25., m_maxClusterSize, 0.5, m_maxClusterSize + 0.5,
                                                     numberOfSensorBin, + 0.5, numberOfSensorBin + 0.5);
    __hClusterSizeVsTimeResidual__->GetZaxis()->SetTitle("Sensor");
    __hClusterSizeVsTimeResidual__->GetYaxis()->SetTitle("Cluster Size");
    __hClusterSizeVsTimeResidual__->GetXaxis()->SetTitle("Cluster Time - EventT0 (ns)");
    registerObject<TH3F>(__hClusterSizeVsTimeResidual__->GetName(), __hClusterSizeVsTimeResidual__);
  }

  TH1F* __hBinToSensorMap__ = new TH1F("__hBinToSensorMap__", "__BinToSensorMap__",
                                       numberOfSensorBin, + 0.5, numberOfSensorBin + 0.5);
  int tmpBinCnt = 0;
  for (auto sensor : allSensors) {
    for (auto view : {'U', 'V'}) {
      tmpBinCnt++;
      TString binLabel = TString::Format("L%iS%iS%c", sensor.getLayerNumber(), sensor.getSensorNumber(), view);
      __hBinToSensorMap__->GetXaxis()->SetBinLabel(tmpBinCnt, binLabel);
    }
  }
  registerObject<TH1F>(__hBinToSensorMap__->GetName(), __hBinToSensorMap__);
}

void SVDClusterTimeShifterCollectorModule::startRun()
{
  for (auto alg : m_timeAlgorithms)
    getObjectPtr<TH3F>(("__hClusterSizeVsTimeResidual__" + alg).Data())->Reset();
}

void SVDClusterTimeShifterCollectorModule::collect()
{
  if (m_eventT0->hasEventT0()) {
    float eventT0 = m_eventT0->getEventT0();
    getObjectPtr<TH1F>("hEventT0")->Fill(eventT0);

    // Fill histograms clusters on tracks
    for (auto alg : m_timeAlgorithms)
      for (const auto& svdCluster : m_svdClsOnTrk[alg]) {
        // get cluster time
        float clTime = svdCluster.getClsTime();
        float clSize = svdCluster.getSize();

        TString binLabel = TString::Format("L%iS%iS%c",
                                           svdCluster.getSensorID().getLayerNumber(),
                                           svdCluster.getSensorID().getSensorNumber(),
                                           svdCluster.isUCluster() ? 'U' : 'V');
        int sensorBin = getObjectPtr<TH1F>("__hBinToSensorMap__")->GetXaxis()->FindBin(binLabel.Data());
        double sensorBinCenter = getObjectPtr<TH1F>("__hBinToSensorMap__")->GetXaxis()->GetBinCenter(sensorBin);
        getObjectPtr<TH3F>(("__hClusterSizeVsTimeResidual__" + alg).Data())->Fill(clTime - eventT0, clSize, sensorBinCenter);
      }
  }
}
