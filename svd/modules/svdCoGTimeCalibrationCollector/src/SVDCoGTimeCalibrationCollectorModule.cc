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

  setDescription(" ");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDClustersFromTracksName", m_svdClusters, "Name of the SVDClusters list", std::string("SVDClustersFromTracks"));
  addParam("SVDRecoDigitsFromTracksName", m_svdRecoDigits, "Name of the SVDRecoDigits list", std::string("SVDRecoDigitsFromTracks"));
  addParam("EventT0Name", m_eventTime, "Name of the EventT0 list", std::string("EventT0"));
  addParam("HistogramTree", m_tree, "Name of the tree in which the histograms are saved", std::string("tree"));
}

void SVDCoGTimeCalibrationCollectorModule::prepare()
{
  TH2F hEventT0vsCoG("eventT0vsCoG__L@layerL@ladderS@sensor@view",
                     "EventT0Sync vs rawCoG in @layer.@ladder.@sensor @view/@side",
                     100, -100, 100, 100, -100, 100);
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

  m_svdCls.isRequired(m_svdClusters);
  m_eventT0.isRequired(m_eventTime);
  m_svdRD.isRequired(m_svdRecoDigits);

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
}


void SVDCoGTimeCalibrationCollectorModule::closeRun()
{
}

void SVDCoGTimeCalibrationCollectorModule::finish()
{
}

void SVDCoGTimeCalibrationCollectorModule::collect()
{
  if (!m_svdCls.isValid()) {
    B2WARNING("!!!! File is not Valid: isValid() = " << m_svdCls.isValid());
    return;
  }
  for (int cl = 0 ; cl < m_svdCls.getEntries(); cl++) {
    SVDCluster* cluster = m_svdCls[cl];
    RelationVector<SVDRecoDigit> reco_rel_cluster = cluster->getRelationsTo<SVDRecoDigit>(m_svdRecoDigits);
    float clTime = m_svdCls[cl]->getClsTime();
    int side = m_svdCls[cl]->isUCluster();
    VxdID::baseType theVxdID = (VxdID::baseType)m_svdCls[cl]->getSensorID();
    if (m_eventT0->hasEventT0()) {
      float eventT0 = m_eventT0->getEventT0();
      float TB = (reco_rel_cluster[0]->getModeByte()).getTriggerBin();
      float eventT0Sync = eventT0 - 7.8625 * (3 - TB);
      getObjectPtr<TH2F>(m_hEventT0vsCoG->getHistogram(theVxdID, side)->GetName())->Fill(clTime, eventT0Sync);
      getObjectPtr<TH1F>(m_hEventT0->getHistogram(theVxdID, side)->GetName())->Fill(eventT0Sync);
      getObjectPtr<TH1F>(m_hEventT0nosync->getHistogram(theVxdID, side)->GetName())->Fill(eventT0);
    }
  };
}
