/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Corona, Giulia Casarosa                            *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <svd/modules/svdCoGTimeCalibrationCollector/SVDCoGTimeCalibrationCollectorModule.h>
#include <TH2F.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDCoGTimeCalibrationCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDCoGTimeCalibrationCollectorModule::SVDCoGTimeCalibrationCollectorModule() : CalibrationCollectorModule()
{
  //Set module properties

  setDescription("Collector module used to create the histograms needed for the SVD 6-Sample CoG, 3-Sample CoG and 3-Sample ELS Time calibration");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDClustersFromTracksName", m_svdClusters, "Name of the SVDClusters list", m_svdClusters);
  addParam("EventT0Name", m_eventTime, "Name of the EventT0 list", m_eventTime);
  addParam("SVDEventInfoName", m_svdEventInfo, "Name of the SVDEventInfo list", m_svdEventInfo);
  addParam("RawCoGBinWidth", m_rawCoGBinWidth, "Bin Width [ns] for raw CoG time", m_rawCoGBinWidth);
}

void SVDCoGTimeCalibrationCollectorModule::prepare()
{
  TH2F hEventT0vsCoG("eventT0vsCoG__L@layerL@ladderS@sensor@view",
                     "EventT0Sync vs rawCoG in @layer.@ladder.@sensor @view/@side",
                     int(200 / m_rawCoGBinWidth), -100, 100, 100, -100, 100);
  hEventT0vsCoG.GetYaxis()->SetTitle("EventT0Sync (ns)");
  hEventT0vsCoG.GetXaxis()->SetTitle("raw_cog (ns)");
  m_hEventT0vsCoG = new SVDHistograms<TH2F>(hEventT0vsCoG);

  TH1F hEventT0("eventT0__L@layerL@ladderS@sensor@view",
                "EventT0Sync in @layer.@ladder.@sensor @view/@side",
                100, -100, 100);
  hEventT0.GetXaxis()->SetTitle("event_t0 (ns)");
  m_hEventT0 = new SVDHistograms<TH1F>(hEventT0);

  TH1F hEventT0NoSync("eventT0nosync__L@layerL@ladderS@sensor@view",
                      "EventT0NoSync in @layer.@ladder.@sensor @view/@side",
                      100, -100, 100);
  hEventT0NoSync.GetXaxis()->SetTitle("event_t0 (ns)");
  m_hEventT0nosync = new SVDHistograms<TH1F>(hEventT0NoSync);

  m_hEventT0FromCDST = new TH1F("hEventT0FromCDST", "EventT0FromCDST", 200, -100, 100);
  registerObject<TH1F>("hEventT0FromCDST", m_hEventT0FromCDST);
  m_hEventT0FromCDSTSync = new TH1F("hEventT0FromCDSTSync", "EventT0FromCDSTSync", 200, -100, 100);
  registerObject<TH1F>("hEventT0FromCDSTSync", m_hEventT0FromCDSTSync);
  m_hRawCoGTimeL3V = new TH1F("hRawCoGTimeL3V", "RawCoGTimeL3V", 200, -100, 100);
  registerObject<TH1F>("hRawCoGTimeL3V", m_hRawCoGTimeL3V);

  m_svdCls.isRequired(m_svdClusters);
  m_eventT0.isRequired(m_eventTime);
  m_svdEI.isRequired(m_svdEventInfo);

  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
    for (auto ladder : geoCache.getLadders(layer)) {
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder)) {
        for (int view = SVDHistograms<TH2F>::VIndex ; view < SVDHistograms<TH2F>::UIndex + 1; view++) {
          registerObject<TH2F>(m_hEventT0vsCoG->getHistogram(sensor, view)->GetName(), m_hEventT0vsCoG->getHistogram(sensor, view));
          registerObject<TH1F>(m_hEventT0->getHistogram(sensor, view)->GetName(), m_hEventT0->getHistogram(sensor, view));
          registerObject<TH1F>(m_hEventT0nosync->getHistogram(sensor, view)->GetName(), m_hEventT0nosync->getHistogram(sensor, view));
        }
      }
    }
  }
}

void SVDCoGTimeCalibrationCollectorModule::startRun()
{

  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
    for (auto ladder : geoCache.getLadders(layer)) {
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder)) {
        for (int view = SVDHistograms<TH2F>::VIndex ; view < SVDHistograms<TH2F>::UIndex + 1; view++) {
          // std::string s = std::string(sensor);
          // std::string v = std::to_string(view);
          // std::string name = string("eventT0vsCog_")+s+string("_")+v;
          // registerObject<TH2F>(name.c_str(),m_hEventT0vsCoG->getHistogram(sensor, view));
          getObjectPtr<TH2F>(m_hEventT0vsCoG->getHistogram(sensor, view)->GetName())->Reset();
          getObjectPtr<TH1F>(m_hEventT0->getHistogram(sensor, view)->GetName())->Reset();
          getObjectPtr<TH1F>(m_hEventT0nosync->getHistogram(sensor, view)->GetName())->Reset();
        }
      }
    }
  }
  m_hEventT0FromCDST->Reset();
  m_hEventT0FromCDSTSync->Reset();
}

void SVDCoGTimeCalibrationCollectorModule::collect()
{
  float eventT0 = 0;
  if (m_eventT0->hasEventT0()) {
    eventT0 = m_eventT0->getEventT0();
    getObjectPtr<TH1F>("hEventT0FromCDST")->Fill(eventT0);
  }
  if (!m_svdCls.isValid()) {
    B2WARNING("!!!! File is not Valid: isValid() = " << m_svdCls.isValid());
    return;
  }

  //get SVDEventInfo
  StoreObjPtr<SVDEventInfo> temp_eventinfo("SVDEventInfo");
  std::string m_svdEventInfoName = "SVDEventInfo";
  if (!temp_eventinfo.isOptional("SVDEventInfo"))
    m_svdEventInfoName = "SVDEventInfoSim";
  StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
  if (!eventinfo) B2ERROR("No SVDEventInfo!");

  for (int cl = 0 ; cl < m_svdCls.getEntries(); cl++) {
    // get cluster time
    float clTime = m_svdCls[cl]->getClsTime();

    //remove firstFrame and triggerBin correction applied in the clusterizer
    clTime = clTime - eventinfo->getSVD2FTSWTimeShift(m_svdCls[cl]->getFirstFrame());

    //get cluster side
    int side = m_svdCls[cl]->isUCluster();

    //get VxdID
    VxdID::baseType theVxdID = (VxdID::baseType)m_svdCls[cl]->getSensorID();
    short unsigned int layer = m_svdCls[cl]->getSensorID().getLayerNumber();

    //fill histograms only if EventT0 is there
    if (m_eventT0->hasEventT0()) {

      float eventT0Sync = eventT0 - eventinfo->getSVD2FTSWTimeShift(m_svdCls[cl]->getFirstFrame());

      getObjectPtr<TH2F>(m_hEventT0vsCoG->getHistogram(theVxdID, side)->GetName())->Fill(clTime, eventT0Sync);
      getObjectPtr<TH1F>(m_hEventT0->getHistogram(theVxdID, side)->GetName())->Fill(eventT0Sync);
      getObjectPtr<TH1F>(m_hEventT0nosync->getHistogram(theVxdID, side)->GetName())->Fill(eventT0);
      getObjectPtr<TH1F>("hEventT0FromCDSTSync")->Fill(eventT0Sync);
      if (layer == 3 && side == 0) {getObjectPtr<TH1F>("hRawCoGTimeL3V")->Fill(clTime);}
    }
  };
}
