/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdCalibration/SVDClusterCalibrationsMonitorModule.h>
#include <vxd/geometry/GeoCache.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDClusterCalibrationsMonitor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDClusterCalibrationsMonitorModule::SVDClusterCalibrationsMonitorModule() : Module()
{
  // Set module properties
  setDescription("Module to produce a list of histograms showing the uploaded calibration constants");

  // Parameter definitions
  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDClusterCalibrationMonitor_output.root"));
}

void SVDClusterCalibrationsMonitorModule::beginRun()
{

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  //tree initialization
  m_tree = new TTree("calibCls", "RECREATE");
  b_run = m_tree->Branch("run", &m_run, "run/i");
  b_layer = m_tree->Branch("layer", &m_layer, "layer/i");
  b_ladder = m_tree->Branch("ladder", &m_ladder, "ladder/i");
  b_sensor = m_tree->Branch("sensor", &m_sensor, "sensor/i");
  b_side = m_tree->Branch("side", &m_side, "side/i");
  b_clsSNR = m_tree->Branch("clsSNR", &m_clsSNR, "clsSNR/F");
  b_clsSeedSNR = m_tree->Branch("clsSeedSNR", &m_clsSeedSNR, "clsSeedSNR/F");
  b_clsAdjSNR = m_tree->Branch("clsAdjSNR", &m_clsAdjSNR, "clsAdjSNR/F");
  b_clsScaleErr1 = m_tree->Branch("clsScaleErr1", &m_clsScaleErr1, "clsScaleErr1/F");
  b_clsScaleErr2 = m_tree->Branch("clsScaleErr2", &m_clsScaleErr2, "clsScaleErr2/F");
  b_clsScaleErr3 = m_tree->Branch("clsScaleErr3", &m_clsScaleErr3, "clsScaleErr3/F");
  b_clsScaleErr4 = m_tree->Branch("clsScaleErr4", &m_clsScaleErr4, "clsScaleErr4/F");
  b_clsScaleErr5 = m_tree->Branch("clsScaleErr5", &m_clsScaleErr5, "clsScaleErr5/F");
  b_clsTimeFunc = m_tree->Branch("clsTimeFunc", &m_clsTimeFunc, "clsTimeFunc/i");
  b_clsTimeMin = m_tree->Branch("clsTimeMin", &m_clsTimeMin, "clsTimeMin/F");


  if (! m_ClusterCal.isValid())
    B2WARNING("No valid SVDClustering for the requested IoV");


  ///CLUSTER SNR CUTS
  TH1F hClsSNR("clusterMinSNR__L@layerL@ladderS@sensor@view",
               "Cluster minimum SNR in @layer.@ladder.@sensor @view/@side",
               100, -0.5, 99.5);
  hClsSNR.GetXaxis()->SetTitle("cls min SNR");
  m_hClsSNR = new SVDHistograms<TH1F>(hClsSNR);

  TH1F hClsSeedSNR("clusterSeedSNR__L@layerL@ladderS@sensor@view",
                   "Cluster Seed minimum SNR in @layer.@ladder.@sensor @view/@side",
                   100, -0.5, 99.5);
  hClsSeedSNR.GetXaxis()->SetTitle("cls seed SNR");
  m_hClsSeedSNR = new SVDHistograms<TH1F>(hClsSeedSNR);



  TH1F hClsAdjSNR("clusterAdjSNR__L@layerL@ladderS@sensor@view",
                  "Cluster Adj minimum SNR in @layer.@ladder.@sensor @view/@side",
                  100, -0.5, 99.5);
  hClsAdjSNR.GetXaxis()->SetTitle("cls adj SNR");
  m_hClsAdjSNR = new SVDHistograms<TH1F>(hClsAdjSNR);


  //CLUSTER POSITION ERROR SCALE FACTORS
  TH1F hClsScaleErr1("clusterScaleErr1__L@layerL@ladderS@sensor@view",
                     "Cluster Position Error Scale Factor for Size 1 in @layer.@ladder.@sensor @view/@side",
                     100, -0.5, 9.5);
  hClsScaleErr1.GetXaxis()->SetTitle("scale factor");
  m_hClsScaleErr1 = new SVDHistograms<TH1F>(hClsScaleErr1);

  TH1F hClsScaleErr2("clusterScaleErr2__L@layerL@ladderS@sensor@view",
                     "Cluster Position Error Scale Factor for Size 2 in @layer.@ladder.@sensor @view/@side",
                     100, -0.5, 9.5);
  hClsScaleErr2.GetXaxis()->SetTitle("scale factor");
  m_hClsScaleErr2 = new SVDHistograms<TH1F>(hClsScaleErr2);

  TH1F hClsScaleErr3("clusterScaleErr3__L@layerL@ladderS@sensor@view",
                     "Cluster Position Error Scale Factor for Size 3 in @layer.@ladder.@sensor @view/@side",
                     100, -0.5, 9.5);
  hClsScaleErr3.GetXaxis()->SetTitle("scale factor");
  m_hClsScaleErr3 = new SVDHistograms<TH1F>(hClsScaleErr3);

  TH1F hClsScaleErr4("clusterScaleErr4__L@layerL@ladderS@sensor@view",
                     "Cluster Position Error Scale Factor for Size 4 in @layer.@ladder.@sensor @view/@side",
                     100, -0.5, 9.5);
  hClsScaleErr4.GetXaxis()->SetTitle("scale factor");
  m_hClsScaleErr4 = new SVDHistograms<TH1F>(hClsScaleErr4);

  TH1F hClsScaleErr5("clusterScaleErr5__L@layerL@ladderS@sensor@view",
                     "Cluster Position Error Scale Factor for Size > 4 in @layer.@ladder.@sensor @view/@side",
                     100, -0.5, 9.5);
  hClsScaleErr5.GetXaxis()->SetTitle("scale factor");
  m_hClsScaleErr5 = new SVDHistograms<TH1F>(hClsScaleErr5);


  //CLUSTER TIME CUTS
  TH1F hClsTimeFuncVersion("clusterTimeSelFunction__L@layerL@ladderS@sensor@view",
                           "Cluster Time Selection Function Version in @layer.@ladder.@sensor @view/@side",
                           5, -0.5, 4.5);
  hClsTimeFuncVersion.GetXaxis()->SetTitle("cls time selection function ID");
  m_hClsTimeFuncVersion = new SVDHistograms<TH1F>(hClsTimeFuncVersion);

  //CLUSTER TIME CUTS
  TH1F hClsTimeMin("clusterMinTimeS__L@layerL@ladderS@sensor@view",
                   "Cluster Minimum Time in @layer.@ladder.@sensor @view/@side",
                   200, -100.5, 99.5);
  hClsTimeMin.GetXaxis()->SetTitle("cls min time (ns)");
  m_hClsTimeMin = new SVDHistograms<TH1F>(hClsTimeMin);



}

void SVDClusterCalibrationsMonitorModule::event()
{

  StoreObjPtr<EventMetaData> meta;
  m_run = meta->getRun();

  if (!m_ClusterCal.isValid())
    return;

  //call for a geometry instance
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> svdLayers = aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();

  while ((itSvdLayers != svdLayers.end()) && (itSvdLayers->getLayerNumber() != 7)) { //loop on Layers

    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) { //loop on Ladders

      std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();
      B2DEBUG(1, "    svd sensor info " << * (svdSensors.begin()));

      while (itSvdSensors != svdSensors.end()) { //loop on sensors
        B2DEBUG(1, "    svd sensor info " << *itSvdSensors);

        m_layer = itSvdSensors->getLayerNumber();
        m_ladder =  itSvdSensors->getLadderNumber();
        m_sensor = itSvdSensors->getSensorNumber();
        Belle2::VxdID theVxdID(m_layer, m_ladder, m_sensor);

        for (m_side = 0; m_side < 2; m_side++) {

          m_clsSNR = m_ClusterCal.getMinClusterSNR(theVxdID, m_side);
          m_hClsSNR->fill(theVxdID, m_side, m_clsSNR);

          m_clsSeedSNR = m_ClusterCal.getMinSeedSNR(theVxdID, m_side);
          m_hClsSeedSNR->fill(theVxdID, m_side, m_clsSeedSNR);

          m_clsAdjSNR = m_ClusterCal.getMinAdjSNR(theVxdID, m_side);
          m_hClsAdjSNR->fill(theVxdID, m_side, m_clsAdjSNR);

          m_clsScaleErr1 = m_CoGOnlySF.getCorrectedClusterPositionError(theVxdID, m_side, 1, 1);
          m_hClsScaleErr1->fill(theVxdID, m_side, m_clsScaleErr1);

          m_clsScaleErr2 = m_CoGOnlySF.getCorrectedClusterPositionError(theVxdID, m_side, 2, 1);
          m_hClsScaleErr2->fill(theVxdID, m_side, m_clsScaleErr2);
          m_clsScaleErr3 = m_CoGOnlySF.getCorrectedClusterPositionError(theVxdID, m_side, 3, 1);
          m_hClsScaleErr3->fill(theVxdID, m_side, m_clsScaleErr3);
          m_clsScaleErr4 = m_CoGOnlySF.getCorrectedClusterPositionError(theVxdID, m_side, 4, 1);
          m_hClsScaleErr4->fill(theVxdID, m_side, m_clsScaleErr4);
          m_clsScaleErr5 = m_CoGOnlySF.getCorrectedClusterPositionError(theVxdID, m_side, 5, 1);
          m_hClsScaleErr5->fill(theVxdID, m_side, m_clsScaleErr5);


          m_clsTimeMin = m_HitTimeCut.getMinClusterTime(theVxdID, m_side);
          m_hClsTimeMin->fill(theVxdID, m_side, m_clsTimeMin);

          m_clsTimeFunc = m_HitTimeCut.getTimeSelectionFunction(theVxdID, m_side);
          m_hClsTimeFuncVersion->fill(theVxdID, m_side, m_clsTimeFunc);

          m_tree->Fill();

        }
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }

}

void SVDClusterCalibrationsMonitorModule::endRun()
{
  B2RESULT("******************************************");
  B2RESULT("** UNIQUE IDs of calibration DB objects **");
  B2RESULT("");
  if (m_ClusterCal.isValid())
    B2RESULT("   - SVDClusterCalibrations:" << m_ClusterCal.getUniqueID());
  else
    B2WARNING("No valid SVDClusterCalibrations for the requested IoV");

  if (m_rootFilePtr != nullptr) {

    m_rootFilePtr->cd();

    //write the tree
    m_tree->Write();


    m_rootFilePtr->mkdir("snr_cuts");
    m_rootFilePtr->mkdir("time_cuts");
    m_rootFilePtr->mkdir("scale_factor");

    VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

    for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD))
      for (auto ladder : geoCache.getLadders(layer))
        for (Belle2::VxdID sensor :  geoCache.getSensors(ladder))
          for (int view = SVDHistograms<TH1F>::VIndex ; view < SVDHistograms<TH1F>::UIndex + 1; view++) {

            //writing the histograms to root:

            m_rootFilePtr->cd("snr_cuts");
            (m_hClsSNR->getHistogram(sensor, view))->Write();
            (m_hClsSeedSNR->getHistogram(sensor, view))->Write();
            (m_hClsAdjSNR->getHistogram(sensor, view))->Write();


            m_rootFilePtr->cd("time_cuts");
            (m_hClsTimeFuncVersion->getHistogram(sensor, view))->Write();
            (m_hClsTimeMin->getHistogram(sensor, view))->Write();

            m_rootFilePtr->cd("scale_factor");
            (m_hClsScaleErr1->getHistogram(sensor, view))->Write();
            (m_hClsScaleErr2->getHistogram(sensor, view))->Write();
            (m_hClsScaleErr3->getHistogram(sensor, view))->Write();
            (m_hClsScaleErr4->getHistogram(sensor, view))->Write();
            (m_hClsScaleErr5->getHistogram(sensor, view))->Write();


          }

    m_rootFilePtr->Close();
    B2RESULT("The rootfile containing the list of histograms has been filled and closed [Cluster].");


  }
}

