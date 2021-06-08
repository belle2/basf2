/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdCalibration/SVDCoGOnlyErrorScaleFactorImporterModule.h>
#include <svd/calibration/SVDCoGOnlyErrorScaleFactors.h>
#include <vxd/geometry/GeoCache.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationVector.h>
#include <framework/datastore/RelationArray.h>
#include <framework/dataobjects/EventMetaData.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDCoGOnlyErrorScaleFactorImporter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDCoGOnlyErrorScaleFactorImporterModule::SVDCoGOnlyErrorScaleFactorImporterModule() : Module()
{
  // Set module properties
  setDescription("Module to produce a list of histograms showing the uploaded calibration constants");

  // Parameter definitions
  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDCoGOnlyErrorScaleFactors.root"));
  addParam("uniqueID", m_uniqueID, "Payload uniqueID.", std::string(m_uniqueID));
  addParam("minPulls", m_min, "min of the pulls histograms.", float(m_min));
  addParam("maxPulls", m_max, "max of the pulls histograms.", float(m_max));
  addParam("nBinsPulls", m_nBins, "number of bins of the pulls histograms.", float(m_nBins));
  addParam("noOutliers", m_noOutliers, "If True, removes outliers from cluster position error scale factor computation.",
           bool(false));
}

void SVDCoGOnlyErrorScaleFactorImporterModule::initialize()
{
  m_clusters.isRequired();
  m_truehits.isRequired();
}

void SVDCoGOnlyErrorScaleFactorImporterModule::beginRun()
{
  //to avoid publicAllocationError:
  delete  m_rootFilePtr;

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  //tree initialization
  m_tree = new TTree("scf", "RECREATE");
  m_tree->Branch("exp", &m_exp, "exp/i");
  m_tree->Branch("run", &m_run, "run/i");
  m_tree->Branch("layer", &m_layer, "layer/i");
  m_tree->Branch("ladder", &m_ladder, "ladder/i");
  m_tree->Branch("sensor", &m_sensor, "sensor/i");
  m_tree->Branch("side", &m_side, "side/i");
  m_tree->Branch("size", &m_size, "size/i");
  m_tree->Branch("clsCharge", &m_clsCharge, "clsCharge/F");
  m_tree->Branch("clsTime", &m_clsTime, "clsTime/F");
  m_tree->Branch("clsPos", &m_clsPos, "clsPos/F");
  m_tree->Branch("clsErr", &m_clsErr, "clsErr/F");
  m_tree->Branch("clsResid", &m_clsResid, "clsResid/F");
  m_tree->Branch("clsPull", &m_clsPull, "clsPull/F");

  //CLUSTER POSITION PULLS
  TH1F hClsPullSize1("clusterPulls1_L@layerL@ladderS@sensor@view",
                     "Cluster CoGOnly Pulls for Size 1 in @layer.@ladder.@sensor @view/@side",
                     m_nBins, m_min, m_max);
  hClsPullSize1.GetXaxis()->SetTitle("cluster pull");
  m_hClsPullSize1 = new SVDHistograms<TH1F>(hClsPullSize1);

  TH1F hClsPullSize2("clusterPulls2_L@layerL@ladderS@sensor@view",
                     "Cluster CoGOnly Pulls for Size 2 in @layer.@ladder.@sensor @view/@side",
                     m_nBins, m_min, m_max);
  hClsPullSize2.GetXaxis()->SetTitle("cluster pull");
  m_hClsPullSize2 = new SVDHistograms<TH1F>(hClsPullSize2);

  TH1F hClsPullSize3("clusterPulls3_L@layerL@ladderS@sensor@view",
                     "Cluster CoGOnly Pulls for Size 3 in @layer.@ladder.@sensor @view/@side",
                     m_nBins, m_min, m_max);
  hClsPullSize3.GetXaxis()->SetTitle("cluster pull");
  m_hClsPullSize3 = new SVDHistograms<TH1F>(hClsPullSize3);

  TH1F hClsPullSize4("clusterPulls4_L@layerL@ladderS@sensor@view",
                     "Cluster CoGOnly Pulls for Size 4 in @layer.@ladder.@sensor @view/@side",
                     m_nBins, m_min, m_max);
  hClsPullSize4.GetXaxis()->SetTitle("cluster pull");
  m_hClsPullSize4 = new SVDHistograms<TH1F>(hClsPullSize4);

  TH1F hClsPullSize5("clusterPulls5_L@layerL@ladderS@sensor@view",
                     "Cluster CoGOnly Pulls for Size > 4 in @layer.@ladder.@sensor @view/@side",
                     m_nBins, m_min, m_max);
  hClsPullSize5.GetXaxis()->SetTitle("cluster pull");
  m_hClsPullSize5 = new SVDHistograms<TH1F>(hClsPullSize5);

  for (int i = 0; i < 2; i++)
    for (int s = 0; s < maxSize; s++) {
      TString sside = "u";
      if (i == 0) sside = "v";
      m_hL3Pulls[s][i] = new TH1F(Form("l3_%s_size%d", sside.Data(), s + 1), Form("Size %d Cluster CoGOnly Pulls for L3 %s sensors",
                                  s + 1, sside.Data()), m_nBins, m_min, m_max);
      m_hL3Pulls[s][i]->GetXaxis()->SetTitle("cluster pull");
      m_hBWPulls[s][i] = new TH1F(Form("bw_%s_size%d", sside.Data(), s + 1), Form("Size %d Cluster CoGOnly Pulls for BW %s sensors",
                                  s + 1, sside.Data()), m_nBins, m_min, m_max);
      m_hBWPulls[s][i]->GetXaxis()->SetTitle("cluster pull");
      m_hFWPulls[s][i] = new TH1F(Form("fw_%s_size%d", sside.Data(), s + 1), Form("Size %d Cluster CoGOnly Pulls for FW %s sensors",
                                  s + 1, sside.Data()), m_nBins, m_min, m_max);
      m_hFWPulls[s][i]->GetXaxis()->SetTitle("cluster pull");
      m_hORPulls[s][i] = new TH1F(Form("or_%s_size%d", sside.Data(), s + 1), Form("Size %d Cluster CoGOnly Pulls for ORIGAMI %s sensors",
                                  s + 1, sside.Data()), m_nBins, m_min, m_max);
      m_hORPulls[s][i]->GetXaxis()->SetTitle("cluster pull");
    }


}

void SVDCoGOnlyErrorScaleFactorImporterModule::event()
{
  StoreObjPtr<EventMetaData> meta;
  m_run = meta->getRun();
  m_exp = meta->getExperiment();

  for (const SVDCluster& cluster : m_clusters) {

    RelationVector<SVDTrueHit> trueHit = cluster.getRelationsTo<SVDTrueHit>();

    if (trueHit.size() == 0) continue;

    m_size = cluster.getSize();
    m_side = cluster.isUCluster();
    m_clsCharge = cluster.getCharge();
    m_clsTime = cluster.getClsTime();
    m_clsPos = m_side ? cluster.getPosition(trueHit[0]->getV()) :  cluster.getPosition();
    m_clsErr = cluster.getPositionSigma();
    m_clsResid = m_side ? m_clsPos - trueHit[0]->getU() : m_clsPos - trueHit[0]->getV();
    m_clsPull = m_clsResid / m_clsErr;

    m_layer = cluster.getSensorID().getLayerNumber();
    m_ladder = cluster.getSensorID().getLadderNumber();
    m_sensor = cluster.getSensorID().getSensorNumber();

    m_tree->Fill();

    if (m_size == 1)  m_hClsPullSize1->fill(cluster.getSensorID(), m_side, m_clsPull);
    else if (m_size == 2)  m_hClsPullSize2->fill(cluster.getSensorID(), m_side, m_clsPull);
    else if (m_size == 3)  m_hClsPullSize3->fill(cluster.getSensorID(), m_side, m_clsPull);
    else if (m_size == 4)  m_hClsPullSize4->fill(cluster.getSensorID(), m_side, m_clsPull);
    else  m_hClsPullSize5->fill(cluster.getSensorID(), m_side, m_clsPull);

  }
}

void SVDCoGOnlyErrorScaleFactorImporterModule::endRun()
{

  // 1. compute scale factors for each sensor (not used in the payload!)
  // and add them to histogram title

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

        m_layer = (int)itSvdSensors->getLayerNumber();
        m_ladder = (int)itSvdSensors->getLadderNumber();
        m_sensor = (int)itSvdSensors->getSensorNumber();
        Belle2::VxdID theVxdID(m_layer, m_ladder, m_sensor);

        for (m_side = 0; m_side < 2; m_side++) {

          TH1F* h = m_hClsPullSize1->getHistogram(theVxdID, m_side);
          h->SetTitle(Form("%s, scf = %1.2f", h->GetTitle(), oneSigma(h)));
          h = m_hClsPullSize2->getHistogram(theVxdID, m_side);
          h->SetTitle(Form("%s, scf = %1.2f", h->GetTitle(), oneSigma(h)));
          h = m_hClsPullSize3->getHistogram(theVxdID, m_side);
          h->SetTitle(Form("%s, scf = %1.2f", h->GetTitle(), oneSigma(h)));
          h = m_hClsPullSize4->getHistogram(theVxdID, m_side);
          h->SetTitle(Form("%s, scf = %1.2f", h->GetTitle(), oneSigma(h)));
          h = m_hClsPullSize5->getHistogram(theVxdID, m_side);
          h->SetTitle(Form("%s, scf = %1.2f", h->GetTitle(), oneSigma(h)));

          //layer 3
          if (theVxdID.getLayerNumber() == 3) {
            m_hL3Pulls[0][m_side]->Add(m_hClsPullSize1->getHistogram(theVxdID, m_side));
            m_hL3Pulls[1][m_side]->Add(m_hClsPullSize2->getHistogram(theVxdID, m_side));
            m_hL3Pulls[2][m_side]->Add(m_hClsPullSize3->getHistogram(theVxdID, m_side));
            m_hL3Pulls[3][m_side]->Add(m_hClsPullSize4->getHistogram(theVxdID, m_side));
            m_hL3Pulls[4][m_side]->Add(m_hClsPullSize5->getHistogram(theVxdID, m_side));
          }

          //forward
          else if (theVxdID.getSensorNumber() == 1) {
            m_hFWPulls[0][m_side]->Add(m_hClsPullSize1->getHistogram(theVxdID, m_side));
            m_hFWPulls[1][m_side]->Add(m_hClsPullSize2->getHistogram(theVxdID, m_side));
            m_hFWPulls[2][m_side]->Add(m_hClsPullSize3->getHistogram(theVxdID, m_side));
            m_hFWPulls[3][m_side]->Add(m_hClsPullSize4->getHistogram(theVxdID, m_side));
            m_hFWPulls[4][m_side]->Add(m_hClsPullSize5->getHistogram(theVxdID, m_side));
          }
          //backward
          else if (theVxdID.getSensorNumber() == theVxdID.getLayerNumber() - 1) {
            m_hBWPulls[0][m_side]->Add(m_hClsPullSize1->getHistogram(theVxdID, m_side));
            m_hBWPulls[1][m_side]->Add(m_hClsPullSize2->getHistogram(theVxdID, m_side));
            m_hBWPulls[2][m_side]->Add(m_hClsPullSize3->getHistogram(theVxdID, m_side));
            m_hBWPulls[3][m_side]->Add(m_hClsPullSize4->getHistogram(theVxdID, m_side));
            m_hBWPulls[4][m_side]->Add(m_hClsPullSize5->getHistogram(theVxdID, m_side));
          }
          //  origami
          else {
            m_hORPulls[0][m_side]->Add(m_hClsPullSize1->getHistogram(theVxdID, m_side));
            m_hORPulls[1][m_side]->Add(m_hClsPullSize2->getHistogram(theVxdID, m_side));
            m_hORPulls[2][m_side]->Add(m_hClsPullSize3->getHistogram(theVxdID, m_side));
            m_hORPulls[3][m_side]->Add(m_hClsPullSize4->getHistogram(theVxdID, m_side));
            m_hORPulls[4][m_side]->Add(m_hClsPullSize5->getHistogram(theVxdID, m_side));
          }
        }
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }


  IntervalOfValidity iov(0, 0, -1, -1);

  auto scfs = new Belle2::SVDPosErrScaleFactors;
  auto payload = new Belle2::SVDCoGOnlyErrorScaleFactors::t_payload(*scfs, m_uniqueID);

  // 2. compute scale factors using cumulative histograms
  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD))
    for (auto ladder : geoCache.getLadders(layer))
      for (Belle2::VxdID theVxdID :  geoCache.getSensors(ladder))

        for (m_side = 0; m_side < 2; m_side++) {

          //layer 3
          if (theVxdID.getLayerNumber() == 3) {
            scfs->scaleError_clSize1 = oneSigma(m_hL3Pulls[0][m_side]);
            scfs->scaleError_clSize2 = oneSigma(m_hL3Pulls[1][m_side]);
            scfs->scaleError_clSize3 = oneSigma(m_hL3Pulls[2][m_side]);
            scfs->scaleError_clSize4 = oneSigma(m_hL3Pulls[3][m_side]);
            scfs->scaleError_clSize5 = oneSigma(m_hL3Pulls[4][m_side]);
          }
          //forward
          else if (theVxdID.getSensorNumber() == 1) {
            scfs->scaleError_clSize1 = oneSigma(m_hFWPulls[0][m_side]);
            scfs->scaleError_clSize2 = oneSigma(m_hFWPulls[1][m_side]);
            scfs->scaleError_clSize3 = oneSigma(m_hFWPulls[2][m_side]);
            scfs->scaleError_clSize4 = oneSigma(m_hFWPulls[3][m_side]);
            scfs->scaleError_clSize5 = oneSigma(m_hFWPulls[4][m_side]);
          }
          //backward
          else if (theVxdID.getSensorNumber() == theVxdID.getLayerNumber() - 1) {
            scfs->scaleError_clSize1 = oneSigma(m_hBWPulls[0][m_side]);
            scfs->scaleError_clSize2 = oneSigma(m_hBWPulls[1][m_side]);
            scfs->scaleError_clSize3 = oneSigma(m_hBWPulls[2][m_side]);
            scfs->scaleError_clSize4 = oneSigma(m_hBWPulls[3][m_side]);
            scfs->scaleError_clSize5 = oneSigma(m_hBWPulls[4][m_side]);
          }
          //  origami
          else {
            scfs->scaleError_clSize1 = oneSigma(m_hORPulls[0][m_side]);
            scfs->scaleError_clSize2 = oneSigma(m_hORPulls[1][m_side]);
            scfs->scaleError_clSize3 = oneSigma(m_hORPulls[2][m_side]);
            scfs->scaleError_clSize4 = oneSigma(m_hORPulls[3][m_side]);
            scfs->scaleError_clSize5 = oneSigma(m_hORPulls[4][m_side]);
          }

          payload->set(theVxdID.getLayerNumber(), theVxdID.getLadderNumber(), theVxdID.getSensorNumber(), m_side, 1, *scfs);
        }

  Database::Instance().storeData(SVDCoGOnlyErrorScaleFactors::name, payload, iov);

  B2RESULT("SVDCoGOnlyErrorScaleFactors imported to database.");


  //now write the rootfile
  if (m_rootFilePtr != nullptr) {

    m_rootFilePtr->cd();

    //write the tree
    m_tree->Write();

    m_rootFilePtr->mkdir("sensor_pulls");
    m_rootFilePtr->mkdir("pulls");

    for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD))
      for (auto ladder : geoCache.getLadders(layer))
        for (Belle2::VxdID sensor :  geoCache.getSensors(ladder))
          for (int view = SVDHistograms<TH1F>::VIndex ; view < SVDHistograms<TH1F>::UIndex + 1; view++) {

            //writing the histograms to root:

            m_rootFilePtr->cd("sensor_pulls");
            (m_hClsPullSize1->getHistogram(sensor, view))->Write();
            (m_hClsPullSize2->getHistogram(sensor, view))->Write();
            (m_hClsPullSize3->getHistogram(sensor, view))->Write();
            (m_hClsPullSize4->getHistogram(sensor, view))->Write();
            (m_hClsPullSize5->getHistogram(sensor, view))->Write();

          }

    for (int s = 0; s < maxSize; s++) {
      m_rootFilePtr->cd("pulls");
      m_hL3Pulls[s][1]->Write();
      m_hFWPulls[s][1]->Write();
      m_hBWPulls[s][1]->Write();
      m_hORPulls[s][1]->Write();
      m_hL3Pulls[s][0]->Write();
      m_hFWPulls[s][0]->Write();
      m_hBWPulls[s][0]->Write();
      m_hORPulls[s][0]->Write();
    }
    m_rootFilePtr->Close();
    B2RESULT("The rootfile containing the list of histograms has been filled and closed.");

  }
}

double SVDCoGOnlyErrorScaleFactorImporterModule::oneSigma(TH1F* h1)
{
  TH1F* h1_res = (TH1F*)h1->Clone("h1_res");

  double probs[2] = {0.16, 1 - 0.16};
  double quant[2] = {0, 0};

  int nbinsHisto = h1_res->GetNbinsX();

  if (m_noOutliers) {
    h1_res->SetBinContent(1, h1_res->GetBinContent(0) + h1_res->GetBinContent(1));
    h1_res->SetBinContent(nbinsHisto, h1_res->GetBinContent(nbinsHisto) + h1_res->GetBinContent(nbinsHisto + 1));
    h1_res->SetBinContent(0, 0);
    h1_res->SetBinContent(nbinsHisto + 1, 0);
  }

  h1_res->GetQuantiles(2, quant, probs);

  return (-quant[0] + quant[1]) / 2;

}
