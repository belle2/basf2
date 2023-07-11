/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/modules/svdTimeCalibrationCollector/SVDTimeCalibrationCollectorModule.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDTimeCalibrationCollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDTimeCalibrationCollectorModule::SVDTimeCalibrationCollectorModule() : CalibrationCollectorModule()
{
  //Set module properties

  setDescription("Collector module used to create the histograms needed for the SVD 6-Sample CoG, 3-Sample CoG and 3-Sample ELS Time calibration");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDClustersFromTracksName", m_svdClustersOnTracks, "Name of the SVDClusters list", m_svdClustersOnTracks);
  addParam("EventT0Name", m_eventTime, "Name of the EventT0 list", m_eventTime);
  addParam("SVDEventInfoName", m_svdEventInfo, "Name of the SVDEventInfo list", m_svdEventInfo);
  addParam("RawCoGBinWidth", m_rawCoGBinWidth, "Bin Width [ns] for raw CoG time", m_rawCoGBinWidth);
  addParam("RawTimeIoVMin", m_minRawTimeForIoV,
           "Minimum value of the raw time distribution used to determine whether change IoV or not", m_minRawTimeForIoV);
  addParam("RawTimeIoVMax", m_maxRawTimeForIoV,
           "Maximum value of the raw time distribution used to determine whether change IoV or not", m_maxRawTimeForIoV);
}

void SVDTimeCalibrationCollectorModule::prepare()
{

  m_hEventT0FromCDC = new TH1F("hEventT0FromCDC", "EventT0FromCDC", 200, -100, 100);
  registerObject<TH1F>("hEventT0FromCDC", m_hEventT0FromCDC);
  m_hEventT0FromCDCSync = new TH1F("hEventT0FromCDCSync", "EventT0FromCDCSync", 200, -100, 100);
  registerObject<TH1F>("hEventT0FromCDCSync", m_hEventT0FromCDCSync);
  m_hRawTimeL3V = new TH1F("hRawTimeL3V", "RawTimeL3V", 150, 0, 150);
  registerObject<TH1F>("hRawTimeL3V", m_hRawTimeL3V);
  m_hRawTimeL3VFullRange = new TH1F("hRawTimeL3VFullRange", "RawTimeL3V full range", 400, -150, 250);
  registerObject<TH1F>("hRawTimeL3VFullRange", m_hRawTimeL3VFullRange);

  m_svdClsOnTrk.isRequired(m_svdClustersOnTracks);
  m_eventT0.isRequired(m_eventTime);
  //  m_svdEI.isRequired(m_svdEventInfo);

  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();
  std::vector<Belle2::VxdID> allSensors;
  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD))
    for (auto ladder : geoCache.getLadders(layer))
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder))
        allSensors.push_back(sensor);

  int numberOfSensorBin = 2 * int(allSensors.size());
  B2INFO("Number of SensorBin: " << numberOfSensorBin);

  TH3F* __hEventT0vsCoG__   = new TH3F("__hEventT0vsCoG__", "EventT0Sync vs rawTime",
                                       int(200 / m_rawCoGBinWidth), -100, 100, 60, -100, 20,
                                       numberOfSensorBin, + 0.5, numberOfSensorBin + 0.5);
  TH2F* __hEventT0__        = new TH2F("__hEventT0__", "EventT0Sync",
                                       100, -100, 100,
                                       numberOfSensorBin, + 0.5, numberOfSensorBin + 0.5);
  TH2F* __hEventT0NoSync__  = new TH2F("__hEventT0NoSync__", "EventT0NoSync",
                                       100, -100, 100,
                                       numberOfSensorBin, + 0.5, numberOfSensorBin + 0.5);
  TH1F* __hBinToSensorMap__ = new TH1F("__hBinToSensorMap__", "__BinToSensorMap__",
                                       numberOfSensorBin, + 0.5, numberOfSensorBin + 0.5);
  __hEventT0vsCoG__->GetZaxis()->SetTitle("Sensor");
  __hEventT0vsCoG__->GetYaxis()->SetTitle("EventT0Sync (ns)");
  __hEventT0vsCoG__->GetXaxis()->SetTitle("raw_time (ns)");
  __hEventT0__->GetYaxis()->SetTitle("Sensor");
  __hEventT0__->GetXaxis()->SetTitle("event_t0 (ns)");
  __hEventT0NoSync__->GetYaxis()->SetTitle("sensor");
  __hEventT0NoSync__->GetXaxis()->SetTitle("event_t0 (ns)");

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
  registerObject<TH3F>(__hEventT0vsCoG__->GetName(), __hEventT0vsCoG__);
  registerObject<TH2F>(__hEventT0__->GetName(), __hEventT0__);
  registerObject<TH2F>(__hEventT0NoSync__->GetName(), __hEventT0NoSync__);
  registerObject<TH1F>(__hBinToSensorMap__->GetName(), __hBinToSensorMap__);
}

void SVDTimeCalibrationCollectorModule::startRun()
{
  getObjectPtr<TH3F>("__hEventT0vsCoG__")->Reset();
  getObjectPtr<TH2F>("__hEventT0__")->Reset();
  getObjectPtr<TH2F>("__hEventT0NoSync__")->Reset();
}

void SVDTimeCalibrationCollectorModule::collect()
{
  float eventT0 = 0;
  // Set the CDC event t0 value if it exists
  if (m_eventT0->hasTemporaryEventT0(Const::EDetector::CDC)) {
    auto evtT0List_CDC = m_eventT0->getTemporaryEventT0s(Const::EDetector::CDC);
    // Set the CDC event t0 value for filling into the histogram
    // The most accurate CDC event t0 value is the last one in the list.
    eventT0 = evtT0List_CDC.back().eventT0;
    getObjectPtr<TH1F>("hEventT0FromCDC")->Fill(eventT0);
  } else {return;}

  if (!m_svdClsOnTrk.isValid()) {
    B2WARNING("!!!! File is not Valid: isValid() = " << m_svdClsOnTrk.isValid());
    return;
  }

  //get SVDEventInfo
  StoreObjPtr<SVDEventInfo> temp_eventinfo("SVDEventInfo");
  std::string m_svdEventInfoName = "SVDEventInfo";
  if (!temp_eventinfo.isOptional("SVDEventInfo"))
    m_svdEventInfoName = "SVDEventInfoSim";
  StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
  if (!eventinfo) B2ERROR("No SVDEventInfo!");
  eventinfo->setAPVClock(m_hwClock);

  for (int cl = 0 ; cl < m_svdClsOnTrk.getEntries(); cl++) {
    // get cluster time
    float clTime_ftsw = m_svdClsOnTrk[cl]->getClsTime();

    //remove firstFrame and triggerBin correction applied in the clusterizer AND relative shift 3/6 mixed sample DAQ
    float clTime = eventinfo->getTimeInSVDReference(clTime_ftsw, m_svdClsOnTrk[cl]->getFirstFrame());

    //get cluster side
    int side = m_svdClsOnTrk[cl]->isUCluster();

    //get layer
    short unsigned int layer = m_svdClsOnTrk[cl]->getSensorID().getLayerNumber();

    //fill histograms only if EventT0 is there
    if (m_eventT0->hasTemporaryEventT0(Const::EDetector::CDC)) {

      float eventT0Sync = eventinfo->getTimeInSVDReference(eventT0, m_svdClsOnTrk[cl]->getFirstFrame());

      TString binLabel = TString::Format("L%iL%iS%i%c",
                                         m_svdClsOnTrk[cl]->getSensorID().getLayerNumber(),
                                         m_svdClsOnTrk[cl]->getSensorID().getLadderNumber(),
                                         m_svdClsOnTrk[cl]->getSensorID().getSensorNumber(),
                                         side ? 'U' : 'V');
      int sensorBin = getObjectPtr<TH1F>("__hBinToSensorMap__")->GetXaxis()->FindBin(binLabel.Data());
      double sensorBinCenter = getObjectPtr<TH1F>("__hBinToSensorMap__")->GetXaxis()->GetBinCenter(sensorBin);
      getObjectPtr<TH3F>("__hEventT0vsCoG__")->Fill(clTime, eventT0Sync, sensorBinCenter);
      getObjectPtr<TH2F>("__hEventT0__")->Fill(eventT0Sync, sensorBinCenter);
      getObjectPtr<TH2F>("__hEventT0NoSync__")->Fill(eventT0, sensorBinCenter);

      getObjectPtr<TH1F>("hEventT0FromCDCSync")->Fill(eventT0Sync);
      if (layer == 3 && side == 0) {
        if (clTime_ftsw >= m_minRawTimeForIoV && clTime_ftsw <= m_maxRawTimeForIoV) {
          getObjectPtr<TH1F>("hRawTimeL3V")->Fill(clTime_ftsw);
        }
        getObjectPtr<TH1F>("hRawTimeL3VFullRange")->Fill(clTime_ftsw);
      }
    }
  };
}
