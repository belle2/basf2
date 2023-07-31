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

  addParam("SVDClusterOnTracksPrefix", m_svdClsOnTrksPrefix, "set prefix of the list of clusters on track", m_svdClsOnTrksPrefix);
  addParam("TimeAlgorithms", m_timeAlgorithms, "set list of time algorithms", m_timeAlgorithms);
  addParam("MaxClusterSize", m_maxClusterSize, "Maximum size of SVD clusters", m_maxClusterSize);
  addParam("EventT0Name", m_eventTime, "Name of the EventT0 list", m_eventTime);
}

void SVDClusterTimeShifterCollectorModule::prepare()
{

  for (auto alg : m_timeAlgorithms) {
    m_svdClsOnTrk[alg] = StoreArray<SVDCluster>();
    m_svdClsOnTrk[alg].isRequired((m_svdClsOnTrksPrefix + '_' + alg).data());
  }
  m_eventT0.isRequired(m_eventTime);

  // getting all the svd sensors
  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();
  std::vector<Belle2::VxdID> allSensors;
  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD))
    for (auto ladder : geoCache.getLadders(layer)) {
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder))
        allSensors.push_back(sensor);
      break;
    }

  int numberOfSensorBin = 2 * int(allSensors.size());
  B2INFO("Number of SensorBin: " << numberOfSensorBin);

  for (auto alg : m_timeAlgorithms) {
    TH3F* __hClusterSizeVsTimeResidual__  = new TH3F(("__hClusterSizeVsTimeResidual__" + alg).data(),
                                                     ("ClusterSize vs " + alg + " Time Residual").data(),
                                                     400, -50., 50., m_maxClusterSize, 0.5, m_maxClusterSize + 0.5,
                                                     numberOfSensorBin, + 0.5, numberOfSensorBin + 0.5);
    __hClusterSizeVsTimeResidual__->GetZaxis()->SetTitle("Sensor");
    __hClusterSizeVsTimeResidual__->GetYaxis()->SetTitle("Cluster Size");
    __hClusterSizeVsTimeResidual__->GetXaxis()->SetTitle("Cluster Time - EventT0 (ns)");
    registerObject<TH3F>(__hClusterSizeVsTimeResidual__->GetName(), __hClusterSizeVsTimeResidual__);

    auto __hTimeL3V__ = new TH1F(("__hTimeL3V__" + alg).data(), (alg + "Time L3V").data(), 150, 0, 150);
    registerObject<TH1F>(__hTimeL3V__->GetName(), __hTimeL3V__);
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
  for (auto alg : m_timeAlgorithms) {
    getObjectPtr<TH3F>(("__hClusterSizeVsTimeResidual__" + alg).data())->Reset();
    getObjectPtr<TH1F>(("__hTimeL3V__" + alg).data())->Reset();
  }
  getObjectPtr<TH1F>("__hBinToSensorMap__")->Reset();
}

void SVDClusterTimeShifterCollectorModule::collect()
{
  float eventT0 = 0;
  // Set the CDC event t0 value if it exists
  if (m_eventT0->hasTemporaryEventT0(Const::EDetector::CDC)) {
    auto evtT0List_CDC = m_eventT0->getTemporaryEventT0s(Const::EDetector::CDC);
    // The most accurate CDC event t0 value is the last one in the list.
    eventT0 = evtT0List_CDC.back().eventT0;
  } else {return;}

  // Fill histograms clusters on tracks
  for (auto alg : m_timeAlgorithms) {

    if (!m_svdClsOnTrk[alg].isValid()) {
      B2WARNING("!!!! List is not Valid: isValid() = " << m_svdClsOnTrk[alg].isValid());
      return;
    }
    for (const auto& svdCluster : m_svdClsOnTrk[alg]) {

      // get cluster time
      float clTime = svdCluster.getClsTime();
      int clSize = svdCluster.getSize();
      if (clSize > m_maxClusterSize) clSize = m_maxClusterSize;

      TString binLabel = TString::Format("L%iS%iS%c",
                                         svdCluster.getSensorID().getLayerNumber(),
                                         svdCluster.getSensorID().getSensorNumber(),
                                         svdCluster.isUCluster() ? 'U' : 'V');
      int sensorBin = getObjectPtr<TH1F>("__hBinToSensorMap__")->GetXaxis()->FindBin(binLabel.Data());
      double sensorBinCenter = getObjectPtr<TH1F>("__hBinToSensorMap__")->GetXaxis()->GetBinCenter(sensorBin);
      getObjectPtr<TH3F>(("__hClusterSizeVsTimeResidual__" + alg).data())->Fill(clTime - eventT0, clSize, sensorBinCenter);

      if (svdCluster.getSensorID().getLayerNumber() == 3 && svdCluster.isUCluster() == 0)
        getObjectPtr<TH1F>(("__hTimeL3V__" + alg).data())->Fill(clTime);
    }
  } // loop over alg
}
