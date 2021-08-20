/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdPerformance/SVDOccupancyAnalysisModule.h>
#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>

#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace SoftwareTrigger;

REG_MODULE(SVDOccupancyAnalysis)

SVDOccupancyAnalysisModule::SVDOccupancyAnalysisModule() : Module()
{

  setDescription("This module check performances of SVD reconstruction of VXD TB data");

  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDOccupancyAnalysis_output.root"));

  addParam("skipHLTRejectedEvents", m_skipRejectedEvents, "If TRUE skip events rejected by HLT", bool(false));
  addParam("groupNevents", m_group, "Number of events to group", float(10000));
  addParam("FADCmode", m_FADCmode,
           "FADC mode: if true the approximation to integer is done", bool(false));
  addParam("minZScut", m_minZS, "Minimum ZS cut", float(3));
  addParam("maxZScut", m_maxZS, "Maximum ZS cut", float(6));
  addParam("pointsZScut", m_pointsZS, "Number of ZS cuts", int(8));

  addParam("ShaperDigitsName", m_ShaperDigitName, "Name of ShaperDigit Store Array.", std::string(""));
}

void SVDOccupancyAnalysisModule::initialize()
{

  m_eventMetaData.isRequired();
  m_svdShapers.isRequired(m_ShaperDigitName);
  B2INFO("    ShaperDigits: " << m_ShaperDigitName);

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  m_nEvents = 0;

}


void SVDOccupancyAnalysisModule::beginRun()
{


  m_occ_L3U = new TH1F("occL3U", "Occupancy Distribution for L3 U side", m_distr_Nbins, m_distr_min, m_distr_max);
  m_occ_L3U->GetXaxis()->SetTitle("occupancy(%)");
  m_occ_L3V = new TH1F("occL3V", "Occupancy Distribution for L3 V side", m_distr_Nbins, m_distr_min, m_distr_max);
  m_occ_L3V->GetXaxis()->SetTitle("occupancy(%)");
  m_occ_L4U = new TH1F("occL4U", "Occupancy Distribution for L4 U side", m_distr_Nbins, m_distr_min, m_distr_max);
  m_occ_L4U->GetXaxis()->SetTitle("occupancy(%)");
  m_occ_L4V = new TH1F("occL4V", "Occupancy Distribution for L4 V side", m_distr_Nbins, m_distr_min, m_distr_max);
  m_occ_L4V->GetXaxis()->SetTitle("occupancy(%)");
  m_occ_L5U = new TH1F("occL5U", "Occupancy Distribution for L5 U side", m_distr_Nbins, m_distr_min, m_distr_max);
  m_occ_L5U->GetXaxis()->SetTitle("occupancy(%)");
  m_occ_L5V = new TH1F("occL5V", "Occupancy Distribution for L5 V side", m_distr_Nbins, m_distr_min, m_distr_max);
  m_occ_L5V->GetXaxis()->SetTitle("occupancy(%)");
  m_occ_L6U = new TH1F("occL6U", "Occupancy Distribution for L6 U side", m_distr_Nbins, m_distr_min, m_distr_max);
  m_occ_L6U->GetXaxis()->SetTitle("occupancy(%)");
  m_occ_L6V = new TH1F("occL6V", "Occupancy Distribution for L6 V side", m_distr_Nbins, m_distr_min, m_distr_max);
  m_occ_L6V->GetXaxis()->SetTitle("occupancy(%)");


  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //collect the list of all SVD Modules in the geometry here
  std::vector<VxdID> sensors = geo.getListOfSensors();
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geo.getSensorInfo(aVxdID);
    if (info.getType() != VXD::SensorInfoBase::SVD) continue;
    m_SVDModules.push_back(aVxdID); // reorder, sort would be better
  }
  std::sort(m_SVDModules.begin(), m_SVDModules.end());  // back to natural order

  m_hit = new SVDSummaryPlots("hits@view", "Number of hits on @view/@side Side");

  TH1F h_dist("dist_L@layerL@ladderS@sensor@view",
              "Occupancy Distribution (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
              m_distr_Nbins, m_distr_min, m_distr_max);
  h_dist.GetXaxis()->SetTitle("occupancy (%)");
  m_histo_dist = new SVDHistograms<TH1F>(h_dist);

  TH1F h_occ_768("occ768_L@layerL@ladderS@sensor@view", "Occupancy (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                 768, 0, 768);
  h_occ_768.GetXaxis()->SetTitle("cellID");
  TH1F h_occ_512("occ512_L@layerL@ladderS@sensor@view", "Occupancy (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                 512, 0, 512);
  h_occ_512.GetXaxis()->SetTitle("cellID");
  m_histo_occ = new SVDHistograms<TH1F>(h_occ_768, h_occ_768, h_occ_768, h_occ_512);


  TH1F h_zsVSocc("occVSzs_L@layerL@ladderS@sensor@view",
                 "Average Occupancy VS Zero Suppression (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)", m_pointsZS, m_minZS,
                 m_maxZS);
  h_zsVSocc.GetXaxis()->SetTitle("ZS cut");
  m_histo_zsOcc = new SVDHistograms<TH1F>(h_zsVSocc);


  TH1F h_zsVSoccSQ("zsVSoccSQ_L@layerL@ladderS@sensor@view",
                   "Average Occupancy VS (ZS cut)^2 (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)", 100, TMath::Power(m_minZS,
                       2) - 5, TMath::Power(m_maxZS, 2));
  h_zsVSoccSQ.GetXaxis()->SetTitle("(ZS cut)^2");
  m_histo_zsOccSQ = new SVDHistograms<TH1F>(h_zsVSoccSQ);


  TH2F h_occtdep_768("occ768VSevt_L@layerL@ladderS@sensor@view",
                     "Average Occupancy VS Event Number (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                     1000, 0, 1000, 768, 0, 768);
  h_occtdep_768.GetXaxis()->SetTitle(Form("evt number/%1.0f", m_group));
  h_occtdep_768.GetYaxis()->SetTitle("cellID");

  TH2F h_occtdep_512("occ512VSevt_L@layerL@ladderS@sensor@view",
                     "Average Occupancy VS Event Number (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                     1000, 0, 1000, 512, 0, 512);
  h_occtdep_512.GetXaxis()->SetTitle(Form("evt number/%1.0f", m_group));
  h_occtdep_512.GetYaxis()->SetTitle("cellID");

  m_histo_occtdep = new SVDHistograms<TH2F>(h_occtdep_768, h_occtdep_768, h_occtdep_768, h_occtdep_512);

}

void SVDOccupancyAnalysisModule::event()
{

  if (m_skipRejectedEvents && (m_resultStoreObjectPointer.isValid())) {
    const bool eventAccepted = FinalTriggerDecisionCalculator::getFinalTriggerDecision(*m_resultStoreObjectPointer);
    if (!eventAccepted) return;
  }

  m_nEvents++;
  int nEvent = m_eventMetaData->getEvent();

  //shaper digits
  for (int digi = 0 ; digi < m_svdShapers.getEntries(); digi++) {


    VxdID::baseType theVxdID = (VxdID::baseType)m_svdShapers[digi]->getSensorID();
    int side = m_svdShapers[digi]->isUStrip();

    //fill standard occupancy plot, for default zero suppression
    m_histo_occtdep->fill(theVxdID, side, nEvent / m_group, m_svdShapers[digi]->getCellID());

    m_hit->fill(theVxdID, side, 1);

    m_histo_occ->fill(theVxdID, side, m_svdShapers[digi]->getCellID());

    float noise = m_NoiseCal.getNoise(theVxdID, side, m_svdShapers[digi]->getCellID());
    float step = (m_maxZS - m_minZS) / m_pointsZS;

    for (int z = 0; z <= m_pointsZS; z++) {
      int nOKSamples = 0;
      float cutMinSignal = (m_minZS + step * z) * noise;

      if (m_FADCmode) {
        cutMinSignal = cutMinSignal + 0.5;
        cutMinSignal = (int)cutMinSignal;
      }


      Belle2::SVDShaperDigit::APVFloatSamples samples_vec = m_svdShapers[digi]->getSamples();

      for (int k = 0; k < 6; k ++)
        if (samples_vec[k] > cutMinSignal)
          nOKSamples++;

      if (nOKSamples > 0) {
        m_histo_zsOcc->fill(theVxdID, side, m_minZS + z * step);
        m_histo_zsOccSQ->fill(theVxdID, side, TMath::Power(m_minZS + z * step, 2));
      }
    }

  }

  //loop on sensors, fill and clear
  for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
    B2DEBUG(10, "module " << i << "," << m_SVDModules[i]);
    float nStripsV = 512;
    if (m_SVDModules[i].getLayerNumber() == 3)
      nStripsV = 768;

    double occU = 100. * m_hit->getValue(m_SVDModules[i], 1) / 768;
    double occV = 100. * m_hit->getValue(m_SVDModules[i], 0) / nStripsV;

    m_histo_dist->fill(m_SVDModules[i], 1, occU);
    m_histo_dist->fill(m_SVDModules[i], 0, occV);

    if (m_SVDModules[i].getLayerNumber() == 3) {
      m_occ_L3U->Fill(occU);
      m_occ_L3V->Fill(occV);
    }
    if (m_SVDModules[i].getLayerNumber() == 4) {
      m_occ_L4U->Fill(occU);
      m_occ_L4V->Fill(occV);
    }
    if (m_SVDModules[i].getLayerNumber() == 5) {
      m_occ_L5U->Fill(occU);
      m_occ_L5V->Fill(occV);
    }
    if (m_SVDModules[i].getLayerNumber() == 6) {
      m_occ_L6U->Fill(occU);
      m_occ_L6V->Fill(occV);
    }

  }

  (m_hit->getHistogram(0))->Reset();
  (m_hit->getHistogram(1))->Reset();

}


void SVDOccupancyAnalysisModule::endRun()
{

  if (m_rootFilePtr != nullptr) {
    m_rootFilePtr->cd();

    TDirectory* oldDir = gDirectory;

    m_occ_L3U->Write();
    m_occ_L3V->Write();
    m_occ_L4U->Write();
    m_occ_L4V->Write();
    m_occ_L5U->Write();
    m_occ_L5V->Write();
    m_occ_L6U->Write();
    m_occ_L6V->Write();

    VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

    for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
      TString layerName = Form("occupancyL%d", layer.getLayerNumber());
      TDirectory* dir_layer = oldDir->mkdir(layerName.Data());
      dir_layer->cd();
      for (auto ladder : geoCache.getLadders(layer))
        for (Belle2::VxdID sensor :  geoCache.getSensors(ladder))
          for (int view = SVDHistograms<TH1F>::VIndex ; view < SVDHistograms<TH1F>::UIndex + 1; view++) {
            (m_histo_dist->getHistogram(sensor, view))->Write();

            (m_histo_occ->getHistogram(sensor, view))->Scale(1. / m_nEvents);
            (m_histo_occ->getHistogram(sensor, view))->Write();

            int nStrips = 768;
            if (sensor.getLayerNumber() != 3 && view == SVDHistograms<TH1F>::VIndex)
              nStrips = 512;

            (m_histo_zsOcc->getHistogram(sensor, view))->Scale(1. / m_nEvents / nStrips);
            (m_histo_zsOcc->getHistogram(sensor, view))->Write();
            (m_histo_zsOccSQ->getHistogram(sensor, view))->Scale(1. / m_nEvents / nStrips);
            (m_histo_zsOccSQ->getHistogram(sensor, view))->Write();

            (m_histo_occtdep->getHistogram(sensor, view))->Scale(1. / m_group);
            (m_histo_occtdep->getHistogram(sensor, view))->Write();
          }
    }

    m_rootFilePtr->Close();

  }
}


