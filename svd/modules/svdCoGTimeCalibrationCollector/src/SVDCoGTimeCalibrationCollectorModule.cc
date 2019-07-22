/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gaetano de Marino, Tadeas Bilka                          *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <svd/modules/svdCoGTimeCalibrationCollector/SVDCoGTimeCalibrationCollectorModule.h>

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

  setDescription(" ");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDClustersFromTracksName", m_svdClusters, "Name of the SVDShaperDigits list", std::string("SVDClustersFromTracks"));
  addParam("EventT0Name", m_eventTime, "Name of the EventT0 list", std::string("EventT0"));
  addParam("HistogramTree", m_tree, "Name of the tree in which the histograms are saved", std::string("tree"));
}

void SVDCoGTimeCalibrationCollectorModule::prepare()
{
  TH2F hEventT0vsCoG("eventT0vsCoG__L@layerL@ladderS@sensor@view",
                     "EventT0 vs rawCoG in @layer.@ladder.@sensor @view/@side",
                     300, -150, 150, 300, -150, 150);
  hEventT0vsCoG.GetYaxis()->SetTitle("eventT0 (ns)");
  hEventT0vsCoG.GetXaxis()->SetTitle("raw_cog (ns)");
  m_hEventT0vsCoG = new SVDHistograms<TH2F>(hEventT0vsCoG);

  m_histogramTree = new TTree("tree", "tree");
  m_svdCls.isRequired(m_svdClusters);
  m_eventT0.isRequired(m_eventTime);

  m_histogramTree->Branch("hist", "TH2F", &m_hist, 32000, 0);
  m_histogramTree->Branch("layer", &m_layer, "layer/I");
  m_histogramTree->Branch("ladder", &m_ladder, "ladder/I");
  m_histogramTree->Branch("sensor", &m_sensor, "sensor/I");
  m_histogramTree->Branch("view", &m_side, "view/I");
  registerObject<TTree>("HTreeCoGTimeCalib", m_histogramTree);
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
          (m_hEventT0vsCoG->getHistogram(sensor, view))->Reset();
        }
      }
    }
  }
}


void SVDCoGTimeCalibrationCollectorModule::closeRun()
{
  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
    for (auto ladder : geoCache.getLadders(layer)) {
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder)) {
        for (int view = SVDHistograms<TH2F>::VIndex ; view < SVDHistograms<TH2F>::UIndex + 1; view++) {
          m_hist = m_hEventT0vsCoG->getHistogram(sensor, view);
          m_layer = layer.getLayerNumber();
          m_ladder = ladder.getLadderNumber();
          m_sensor = sensor.getSensorNumber();
          m_side = view;
          m_histogramTree->Fill();
        }
      }
    }
  }
}

void SVDCoGTimeCalibrationCollectorModule::finish()
{
}

void SVDCoGTimeCalibrationCollectorModule::collect()
{
  for (int cl = 0 ; cl < m_svdCls.getEntries(); cl++) {
    float clTime = m_svdCls[cl]->getClsTime();
    int side = m_svdCls[cl]->isUCluster();
    VxdID::baseType theVxdID = (VxdID::baseType)m_svdCls[cl]->getSensorID();
    if (m_eventT0->hasEventT0()) {
      float eventT0 = m_eventT0->getEventT0();
      m_hEventT0vsCoG->fill(theVxdID, side, clTime, eventT0);
    }
  };
}
