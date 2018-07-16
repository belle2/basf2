#include <svd/modules/svdPerformance/SVDOccupancyAnalysisModule.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;

REG_MODULE(SVDOccupancyAnalysis)

SVDOccupancyAnalysisModule::SVDOccupancyAnalysisModule() : Module()
{

  setDescription("This module check performances of SVD reconstruction of VXD TB data");

  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDOccupancyAnalysis_output.root"));

  addParam("groupNevents", m_group, "Number of events to group", float(10000));
  addParam("FADCmode", m_FADCmode,
           "FADC mode: if true the approximation to integer is done", bool(false));
  addParam("minZScut", m_minZS, "Minimum ZS cut", float(3));
  addParam("maxZScut", m_maxZS, "Maximum ZS cut", float(6));
  addParam("pointsZScut", m_pointsZS, "Number of ZS cuts", int(8));

  addParam("ShaperDigitsName", m_ShaperDigitName, "Name of ShaperDigit Store Array.", std::string(""));
}

SVDOccupancyAnalysisModule::~SVDOccupancyAnalysisModule()
{

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
  h_occtdep_768.GetXaxis()->SetTitle("evt number/10000");
  h_occtdep_768.GetYaxis()->SetTitle("cellID");

  TH2F h_occtdep_512("occ512VSevt_L@layerL@ladderS@sensor@view",
                     "Average Occupancy VS Event Number (layer @layer, ladder @ladder, sensor @sensor, side@view/@side)",
                     1000, 0, 1000, 512, 0, 512);
  h_occtdep_512.GetXaxis()->SetTitle("evt number/10000");
  h_occtdep_512.GetYaxis()->SetTitle("cellID");

  m_histo_occtdep = new SVDHistograms<TH2F>(h_occtdep_768, h_occtdep_768, h_occtdep_768, h_occtdep_512);

}

void SVDOccupancyAnalysisModule::event()
{

  m_nEvents++;
  int nEvent = m_eventMetaData->getEvent();

  //shaper digits
  for (int digi = 0 ; digi < m_svdShapers.getEntries(); digi++) {


    VxdID::baseType theVxdID = (VxdID::baseType)m_svdShapers[digi]->getSensorID();
    int side = m_svdShapers[digi]->isUStrip();

    //fill standard occupancy plot, for default zero suppression
    m_histo_occtdep->fill(theVxdID, side, nEvent / m_group, m_svdShapers[digi]->getCellID());

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

}


void SVDOccupancyAnalysisModule::terminate()
{

}


void SVDOccupancyAnalysisModule::endRun()
{

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TDirectory* oldDir = gDirectory;

    /*
    for (int i = 0; i < m_nLayers; i++) {
      TString layerName = "shaperL";
      layerName += i + 3;
      TDirectory* dir_layer = oldDir->mkdir(layerName.Data());
      dir_layer->cd();
      TIter nextH_shaper(m_histoList_shaper[i]);
      while ((obj = (TH1F*)nextH_shaper())) {
        int nStrips = 768;
        TString name = obj->GetName();
        if (name.Contains("occupancy"))
          obj->Scale(1. / m_nEvents);
        else if (name.Contains("occVSevt"))
          obj->Scale(1. / m_group);
        else {
          if ((! name.Contains("L3")) &&
              (name.Contains("1V") || name.Contains("2V") || name.Contains("3V") || name.Contains("4V") || name.Contains("5V")))
            nStrips = 512;

          obj->Scale(1. / m_nEvents / nStrips);
        }
        obj->Write();
      }

    }
    */

    VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

    for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
      TString layerName = Form("occupancyL%d", layer.getLayerNumber());
      TDirectory* dir_layer = oldDir->mkdir(layerName.Data());
      dir_layer->cd();
      for (auto ladder : geoCache.getLadders(layer))
        for (Belle2::VxdID sensor :  geoCache.getSensors(ladder))
          for (int view = SVDHistograms<TH1F>::VIndex ; view < SVDHistograms<TH1F>::UIndex + 1; view++) {

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


