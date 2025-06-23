/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/modules/svdTimeCalibrationCollector/SVDClusterAbsoluteTimeShifterCollectorModule.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDClusterAbsoluteTimeShifterCollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDClusterAbsoluteTimeShifterCollectorModule::SVDClusterAbsoluteTimeShifterCollectorModule() : CalibrationCollectorModule()
{
  //Set module properties

  setDescription("Collector module used to create the histograms needed for the SVD 6-Sample CoG, 3-Sample CoG and 3-Sample ELS Time calibration. "
                 "This is for the absolute shift of the calibrated time.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDClustersOnTrackPrefix", m_svdClustersOnTrackPrefix, "set prefix of the list of clusters on track",
           m_svdClustersOnTrackPrefix);
  addParam("TimeAlgorithms", m_timeAlgorithms, "set list of time algorithms", m_timeAlgorithms);
  addParam("EventT0Name", m_eventT0Name, "Name of the EventT0 list", m_eventT0Name);
}

void SVDClusterAbsoluteTimeShifterCollectorModule::prepare()
{

  for (auto alg : m_timeAlgorithms) {
    m_svdClustersOnTrack[alg] = StoreArray<SVDCluster>();
    m_svdClustersOnTrack[alg].isRequired((m_svdClustersOnTrackPrefix + '_' + alg).data());
  }
  m_eventT0.isRequired(m_eventT0Name);

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

    // 12 bins for L1-6 U-V sides

    TH2F* __hClsOnTrack__ = new TH2F(("hClsTimeOnTracks_" + alg).data(),
                                     ("Cluster time for " + alg).data(),
                                     300, -150., 150., 12, 1., 13.);
    __hClsOnTrack__->GetXaxis()->SetTitle("Cluster Time (ns)");
    __hClsOnTrack__->GetYaxis()->SetTitle("LayerSideID");
    registerObject<TH2F>(__hClsOnTrack__->GetName(), __hClsOnTrack__);
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

void SVDClusterAbsoluteTimeShifterCollectorModule::startRun()
{
  for (auto alg : m_timeAlgorithms) {
    getObjectPtr<TH2F>(("hClsTimeOnTracks_" + alg).data())->Reset();
  }
  getObjectPtr<TH1F>("__hBinToSensorMap__")->Reset();
}

void SVDClusterAbsoluteTimeShifterCollectorModule::collect()
{
  float eventT0 = 0;
  // Set the CDC event t0 value if it exists
  if (m_eventT0->hasTemporaryEventT0(Const::EDetector::CDC)) {
    const auto evtT0CDC = m_eventT0->getBestCDCTemporaryEventT0();
    eventT0 = evtT0CDC->eventT0;
  } else {return;}

  // Fill histograms clusters on tracks
  for (auto alg : m_timeAlgorithms) {

    if (!m_svdClustersOnTrack[alg].isValid()) {
      B2WARNING("!!!! List is not Valid: isValid() = " << m_svdClustersOnTrack[alg].isValid());
      return;
    }
    for (const auto& svdCluster : m_svdClustersOnTrack[alg]) {

      // get cluster time
      float clusterTime = svdCluster.getClsTime();

      int side = svdCluster.isUCluster() ? 0 : 1;
      int layerNumber = svdCluster.getSensorID().getLayerNumber();
      int LayerSideID = 2 * layerNumber - side ; // 1 to 12 something similar to sensorBin
      double LayerSideBinCenter = getObjectPtr<TH2F>("hClsTimeOnTracks_" + alg)->GetYaxis()->GetBinCenter(LayerSideID);

      getObjectPtr<TH2F>(("hClsTimeOnTracks_" + alg).data())->Fill(clusterTime - eventT0, LayerSideBinCenter);




    }
  } // loop over alg
}
