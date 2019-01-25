/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Giulia Casarosa                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdCalibration/SVDLocalCalibrationsCheckModule.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <TStyle.h>
#include <TLine.h>
#include <TPaveText.h>
#include <TText.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDLocalCalibrationsCheck)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDLocalCalibrationsCheckModule::SVDLocalCalibrationsCheckModule() : Module()
{
  // Set module properties
  setDescription("Module to produce a list of histograms showing the uploaded calibration constants");

  // Parameter definitions
  addParam("outputPdfName", m_outputPdfName, "output pdf filename", std::string("SVDLocalCalibrationCheck.pdf"));
  addParam("reference_root", m_rootFileNameREF, "Name of REFERENCE root file.",
           std::string("SVDLocalCalibrationMonitor_experiment5_run92.root"));
  addParam("check_root", m_rootFileNameCHECK, "Name of CHECK root file.",
           std::string("SVDLocalCalibrationMonitor_experiment5_run408.root"));
  addParam("plotGoodAPVs", m_plotGoodAPVs, "If true draw 1D plots also for good APVs", bool(false));

  addParam("cutN_outliers", m_cutN_out, "Max number of outliers allowed", int(0));
  addParam("cutNoise_average", m_cutNoise_ave, "Max relative deviation of average noise per APV", float(0.1));
  addParam("cutNoise_outliers", m_cutNoise_out, "Max relative deviation of single strip", float(0.3));
  addParam("cutGain_average", m_cutGain_ave, "Max relative deviation of average gain per APV", float(0.1));
  addParam("cutGain_outliers", m_cutGain_out, "Max relative deviation of single strip", float(0.3));
  addParam("cutPedestal_average", m_cutPedestal_ave, "Max relative deviation of average pedestal per APV", float(0.1));
  addParam("cutPedestal_outliers", m_cutPedestal_out, "Max relative deviation of single strip", float(0.3));

}

void SVDLocalCalibrationsCheckModule::beginRun()
{

  gStyle->SetOptStat(0);
  gStyle->SetLegendBorderSize(0);
  gStyle->SetTextFont(42);

  // read REFERENCE root file
  m_rootFilePtrREF = new TFile(m_rootFileNameREF.c_str(), "READ");

  //  m_rootFilePtrREF->Print();

  //REF tree initialization
  m_treeREF = (TTree*)m_rootFilePtrREF->Get("calibLocalDetailed");
  m_treeREF->SetBranchAddress("run", &m_runREF, &b_runREF);
  m_treeREF->SetBranchAddress("layer", &m_layerREF, &b_layerREF);
  m_treeREF->SetBranchAddress("ladder", &m_ladderREF, &b_ladderREF);
  m_treeREF->SetBranchAddress("sensor", &m_sensorREF, &b_sensorREF);
  m_treeREF->SetBranchAddress("side", &m_sideREF, &b_sideREF);
  m_treeREF->SetBranchAddress("strip", &m_stripREF, &b_stripREF);
  m_treeREF->SetBranchAddress("mask", &m_maskREF, &b_maskREF);
  m_treeREF->SetBranchAddress("noise", &m_noiseREF, &b_noiseREF);
  m_treeREF->SetBranchAddress("noiseEl", &m_noiseElREF, &b_noiseElREF);
  m_treeREF->SetBranchAddress("gain", &m_gainREF,  &b_gainREF);
  m_treeREF->SetBranchAddress("pedestal", &m_pedestalREF, &b_pedestalREF);
  m_treeREF->SetBranchAddress("peakTime", &m_peakTimeREF, &b_peakTimeREF);
  m_treeREF->SetBranchAddress("pulseWidth", &m_pulseWidthREF, &b_pulseWidthREF);

  // read CHECK root file
  m_rootFilePtrCHECK = new TFile(m_rootFileNameCHECK.c_str(), "READ");
  //  m_rootFilePtrCHECK->Print();

  //CHECK tree initialization
  m_treeCHECK = (TTree*)m_rootFilePtrCHECK->Get("calibLocalDetailed");
  m_treeCHECK->SetBranchAddress("run", &m_runCHECK, &b_runCHECK);
  m_treeCHECK->SetBranchAddress("layer", &m_layerCHECK, &b_layerCHECK);
  m_treeCHECK->SetBranchAddress("ladder", &m_ladderCHECK, &b_ladderCHECK);
  m_treeCHECK->SetBranchAddress("sensor", &m_sensorCHECK, &b_sensorCHECK);
  m_treeCHECK->SetBranchAddress("side", &m_sideCHECK, &b_sideCHECK);
  m_treeCHECK->SetBranchAddress("strip", &m_stripCHECK, &b_stripCHECK);
  m_treeCHECK->SetBranchAddress("mask", &m_maskCHECK, &b_maskCHECK);
  m_treeCHECK->SetBranchAddress("noise", &m_noiseCHECK, &b_noiseCHECK);
  m_treeCHECK->SetBranchAddress("noiseEl", &m_noiseElCHECK, &b_noiseElCHECK);
  m_treeCHECK->SetBranchAddress("gain", &m_gainCHECK,  &b_gainCHECK);
  m_treeCHECK->SetBranchAddress("pedestal", &m_pedestalCHECK, &b_pedestalCHECK);
  m_treeCHECK->SetBranchAddress("peakTime", &m_peakTimeCHECK, &b_peakTimeCHECK);
  m_treeCHECK->SetBranchAddress("pulseWidth", &m_pulseWidthCHECK, &b_pulseWidthCHECK);

  /*
  ///MASKS
  m_h2MaskREF =  (SVDHistograms<TH2F>*)m_rootFilePtrREF->Get("expert/h2Mask");
  m_h2MaskCHECK =  (SVDHistograms<TH2F>*)m_rootFilePtrREF->Get("expert/h2Mask");

  TH1F template_mask("maskedDIFF_L@layerL@ladderS@sensor@view@apv",
         //        "masked strip in @layer.@ladder.@sensor @view/@side APV @apv",
         "masked strip in @layer.@ladder.@sensor @view/@side",
         100, -1, 1);
  template_mask.GetXaxis()->SetTitle("( ref - check )/ ref");
  m_hMaskDIFF = new SVDAPVHistograms<TH1F>(template_mask);
  setAPVHistoStyles(m_hMaskDIFF);
  */

  ///NOISES
  m_h2NoiseREF = (SVDHistograms<TH2F>*)m_rootFilePtrREF->Get("expert/h2Noise");
  m_h2NoiseCHECK = (SVDHistograms<TH2F>*)m_rootFilePtrCHECK->Get("expert/h2Noise");

  TH1F template_noise("noiseDIFF_L@layerL@ladderS@sensor@view@apv",
                      //         "Noise Deviation Distribution in @layer.@ladder.@sensor @view/@side APV @apv",
                      "Noise Deviation Distribution in @layer.@ladder.@sensor @view/@side",
                      100, -1, 1);
  template_noise.GetXaxis()->SetTitle("( ref - check )/ ref");
  m_hNoiseDIFF = new SVDAPVHistograms<TH1F>(template_noise);
  setAPVHistoStyles(m_hNoiseDIFF);
  m_hNoiseSummary = new SVDSummaryPlots("noiseSummary@view", "Noise Summary for @view/@side Side");

  ///GAINS
  m_h2GainREF = (SVDHistograms<TH2F>*)m_rootFilePtrREF->Get("expert/h2Gain");
  m_h2GainCHECK = (SVDHistograms<TH2F>*)m_rootFilePtrCHECK->Get("expert/h2Gain");

  TH1F template_gain("gainDIFF_L@layerL@ladderS@sensor@view@apv",
                     //         "Gain Deviation Distribution in @layer.@ladder.@sensor @view/@side APV @apv",
                     "Gain Deviation Distribution in @layer.@ladder.@sensor @view/@side",
                     100, -2, 2);
  template_gain.GetXaxis()->SetTitle("( ref - check )/ ref");
  m_hGainDIFF = new SVDAPVHistograms<TH1F>(template_gain);
  setAPVHistoStyles(m_hGainDIFF);
  m_hGainSummary = new SVDSummaryPlots("gainSummary@view", "Gain Summary for @view/@side Side");

  ///PEDESTALS
  m_h2PedestalREF = (SVDHistograms<TH2F>*)m_rootFilePtrREF->Get("expert/h2Pedestal");
  m_h2PedestalCHECK = (SVDHistograms<TH2F>*)m_rootFilePtrCHECK->Get("expert/h2Pedestal");

  TH1F template_pedestal("pedestalDIFF_L@layerL@ladderS@sensor@view@apv",
                         //         "Pedestal Deviation Distribution in @layer.@ladder.@sensor @view/@side APV @apv",
                         "Pedestal Deviation Distribution in @layer.@ladder.@sensor @view/@side",
                         100, -1, 1);
  template_pedestal.GetXaxis()->SetTitle("( ref - check )/ ref");
  m_hPedestalDIFF = new SVDAPVHistograms<TH1F>(template_pedestal);
  setAPVHistoStyles(m_hPedestalDIFF);
  m_hPedestalSummary = new SVDSummaryPlots("pedestalSummary@view", "Pedestal Summary for @view/@side Side");

  createLegends();
}

void SVDLocalCalibrationsCheckModule::event()
{
  printFirstPage();

  Long64_t nentries = m_treeREF->GetEntriesFast();

  for (Long64_t jentry = 0; jentry < nentries; jentry++) {
    m_treeREF->GetEntry(jentry);
    m_treeCHECK->GetEntry(jentry);
    VxdID theVxdID(m_layerREF, m_ladderREF, m_sensorREF);
    float diff = 0;

    //    diff = (m_maskREF - m_maskCHECK)/ m_maskREF;
    //    m_hMaskDIFF->fill(theVxdID, (int)m_sideREF, (int)m_stripREF / 128, diff);

    diff = (m_noiseREF - m_noiseCHECK) / m_noiseREF;
    m_hNoiseDIFF->fill(theVxdID, (int)m_sideREF, (int)m_stripREF / 128, diff);

    diff = (m_gainREF - m_gainCHECK) / m_gainREF;
    m_hGainDIFF->fill(theVxdID, (int)m_sideREF, (int)m_stripREF / 128, diff);

    diff = (m_pedestalREF - m_pedestalCHECK) / m_pedestalREF;
    m_hPedestalDIFF->fill(theVxdID, (int)m_sideREF, (int)m_stripREF / 128, diff);
  }


  //call for a geometry instance
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> svdLayers = aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();

  while ((itSvdLayers != svdLayers.end()) && (itSvdLayers->getLayerNumber() != 7)) { //loop on Layers

    int layer = itSvdLayers->getLayerNumber();
    printLayerPage(layer);

    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) { //loop on Ladders

      std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();

      while (itSvdSensors != svdSensors.end()) { //loop on sensors
        B2DEBUG(1, "    svd sensor info " << *itSvdSensors);


        int ladder =  itSvdSensors->getLadderNumber();
        int sensor = itSvdSensors->getSensorNumber();
        Belle2::VxdID theVxdID(layer, ladder, sensor);
        const SVD::SensorInfo* currentSensorInfo = dynamic_cast<const SVD::SensorInfo*>(&VXD::GeoCache::get(theVxdID));

        //noise
        TList* listNoiseUBAD = new TList;
        TList* listNoiseUGOOD = new TList;
        TList* listNoiseVBAD = new TList;
        TList* listNoiseVGOOD = new TList;

        //gain
        TList* listGainUBAD = new TList;
        TList* listGainUGOOD = new TList;
        TList* listGainVBAD = new TList;
        TList* listGainVGOOD = new TList;

        //pedestal
        TList* listPedestalUBAD = new TList;
        TList* listPedestalUGOOD = new TList;
        TList* listPedestalVBAD = new TList;
        TList* listPedestalVGOOD = new TList;

        bool needPlot = false;
        for (int side = 0; side < 2; side++) {

          int Ncells = currentSensorInfo->getUCells();
          if (side == 0)
            Ncells = currentSensorInfo->getVCells();

          int Napv = Ncells / 128;

          for (int m_APV = 0; m_APV < Napv; m_APV++) {

            int problem = 0;

            //noise analysis
            TH1F* hNoise = m_hNoiseDIFF->getHistogram(theVxdID, side, m_APV);
            problem = hasAnyProblem(hNoise, m_cutNoise_ave, m_cutNoise_out);
            if (problem)   {
              needPlot = true;
              B2INFO("WARNING, ONE APV has Noise problems in: L" << layer << "L" << ladder << "S" << sensor << " side = " << side <<
                     ", APV number =" << m_APV << " problem ID = " << problem);
              m_hNoiseSummary->fill(theVxdID, side, 1);
              if (side == 0)
                listNoiseVBAD->Add(hNoise);
              else
                listNoiseUBAD->Add(hNoise);
            } else {
              if (side == 0)
                listNoiseVGOOD->Add(hNoise);
              else
                listNoiseUGOOD->Add(hNoise);
            }

            //gain analysis
            TH1F* hGain = m_hGainDIFF->getHistogram(theVxdID, side, m_APV);
            problem = hasAnyProblem(hGain, m_cutGain_ave, m_cutGain_out);
            if (problem)   {
              needPlot = true;
              B2INFO("WARNING, ONE APV has Gain problems in: L" << layer << "L" << ladder << "S" << sensor << " side = " << side <<
                     ", APV number =" << m_APV << " problem ID = " << problem);
              m_hGainSummary->fill(theVxdID, side, 1);
              if (side == 0)
                listGainVBAD->Add(hGain);
              else
                listGainUBAD->Add(hGain);
            } else {
              if (side == 0)
                listGainVGOOD->Add(hGain);
              else
                listGainUGOOD->Add(hGain);
            }

            //pedestal analysis
            TH1F* hPedestal = m_hPedestalDIFF->getHistogram(theVxdID, side, m_APV);
            problem = hasAnyProblem(hPedestal, m_cutPedestal_ave, m_cutPedestal_out);
            if (problem)   {
              needPlot = true;
              B2INFO("WARNING, ONE APV has Pedestal problems in: L" << layer << "L" << ladder << "S" << sensor << " side = " << side <<
                     ", APV number =" << m_APV << " problem ID = " << problem);
              m_hPedestalSummary->fill(theVxdID, side, 1);
              if (side == 0)
                listPedestalVBAD->Add(hPedestal);
              else
                listPedestalUBAD->Add(hPedestal);
            } else {
              if (side == 0)
                listPedestalVGOOD->Add(hPedestal);
              else
                listPedestalUGOOD->Add(hPedestal);
            }



          }
        }
        if (needPlot) {
          printPage(theVxdID, listNoiseUBAD, listNoiseVBAD, listNoiseUGOOD, listNoiseVGOOD, "Noise");
          printPage(theVxdID, listGainUBAD, listGainVBAD, listGainUGOOD, listGainVGOOD, "Gain");
          printPage(theVxdID, listPedestalUBAD, listPedestalVBAD, listPedestalUGOOD, listPedestalVGOOD, "Pedestal");
        }
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }

  printSummaryPages();
  printLastPage();

}

void SVDLocalCalibrationsCheckModule::printFirstPage()
{

  TCanvas* empty = new TCanvas();
  TString pdf_open = TString(m_outputPdfName) + "[";
  empty->Print(pdf_open);

  TCanvas* first = new TCanvas("open_pag1", "test first page");
  first->cd();
  TPaveText* pt_title = new TPaveText(.05, .9, .95, 1, "blNDC");
  char name[50];
  sprintf(name, "Local Calibration Check Results");
  pt_title->AddText(name);
  pt_title->SetTextFont(42);
  pt_title->SetTextColor(kBlack);
  pt_title->SetShadowColor(0);
  pt_title->SetFillColor(10);
  pt_title->SetBorderSize(0);
  pt_title->SetTextSize(0.08);
  pt_title->Draw();

  TPaveText* pt_input_title = new TPaveText(.05, .8, .95, .85);
  TPaveText* pt_input = new TPaveText(.05, .72, .8, .8);
  char input[150];
  sprintf(input, "%s", "input files");
  pt_input_title->AddText(input);
  pt_input_title->SetShadowColor(0);
  pt_input_title->SetBorderSize(0);
  pt_input_title->SetTextSize(0.03);
  sprintf(input, "reference = %s",  m_rootFileNameREF.c_str());
  pt_input->AddText(input);
  ((TText*)pt_input->GetListOfLines()->Last())->SetTextColor(kRed);
  sprintf(input, "calibration = %s", m_rootFileNameCHECK.c_str());
  pt_input->AddText(input);
  ((TText*)pt_input->GetListOfLines()->Last())->SetTextColor(kBlue);
  pt_input->SetTextSize(0.02);
  pt_input->SetTextAlign(12);
  pt_input->SetShadowColor(0);
  pt_input->SetBorderSize(0);
  pt_input->SetFillColor(10);

  pt_input_title->Draw();
  pt_input->Draw();

  TPaveText* pt_cuts_title = new TPaveText(.05, .65, .95, .7);
  TPaveText* pt_cuts = new TPaveText(.05, .35, .8, .55);
  char cuts[512];
  sprintf(cuts, "%s", "algorithm parameters");
  pt_cuts_title->AddText(cuts);
  pt_cuts_title->SetShadowColor(0);
  pt_cuts_title->SetBorderSize(0);
  pt_cuts_title->SetTextSize(0.03);
  // NOISE
  sprintf(cuts, "  Noise: ");
  pt_cuts->AddText(cuts);
  sprintf(cuts, "         1. abs( (ref_ave - check_ave) / ref_ave) ) > %1.2f",  m_cutNoise_ave);
  pt_cuts->AddText(cuts);
  sprintf(cuts, "         2. more than %d strips with abs( (ref_ave - check_ave) / ref_ave) ) > %1.2f",  m_cutN_out, m_cutNoise_out);
  pt_cuts->AddText(cuts);
  sprintf(cuts, "           where {ref,check}_ave is the noise averaged on one APV chip of the reference or the check calibration");
  pt_cuts->AddText(cuts);
  // GAIN
  sprintf(cuts, "  Gain: ");
  pt_cuts->AddText(cuts);
  sprintf(cuts, "         1. abs( (ref_ave - check_ave) / ref_ave) ) > %1.2f",  m_cutGain_ave);
  pt_cuts->AddText(cuts);
  sprintf(cuts, "         2. more than %d strips with abs( (ref_ave - check_ave) / ref_ave) ) > %1.2f",  m_cutN_out, m_cutGain_out);
  pt_cuts->AddText(cuts);
  sprintf(cuts, "           where {ref,check}_ave is the gain averaged on one APV chip of the reference or the check calibration");
  pt_cuts->AddText(cuts);
  // PEDESTAL
  sprintf(cuts, "  Pedestal: ");
  pt_cuts->AddText(cuts);
  sprintf(cuts, "         1. abs( (ref_ave - check_ave) / ref_ave) ) > %1.2f",  m_cutPedestal_ave);
  pt_cuts->AddText(cuts);
  sprintf(cuts, "         2. more than %d strips with abs( (ref_ave - check_ave) / ref_ave) ) > %1.2f",  m_cutN_out,
          m_cutPedestal_out);
  pt_cuts->AddText(cuts);
  sprintf(cuts,
          "           where {ref,check}_ave is the pedestal averaged on one APV chip of the reference or the check calibration");
  pt_cuts->AddText(cuts);
  pt_cuts->SetTextSize(0.02);
  pt_cuts->SetShadowColor(0);
  pt_cuts->SetBorderSize(0);
  pt_cuts->SetFillColor(10);
  pt_cuts->SetTextAlign(12);

  pt_cuts_title->Draw();
  pt_cuts->Draw();

  TPaveText* pt_tag_title = new TPaveText(.05, .03, .95, .07);
  char tag[100];
  sprintf(tag, "analysis algorithm ID 0.0");
  pt_tag_title->AddText(tag);
  //  pt_tag_title->SetTextFont(62);
  pt_tag_title->SetShadowColor(0);
  pt_tag_title->SetFillColor(18);
  pt_tag_title->SetBorderSize(0);
  pt_tag_title->SetTextSize(0.02);
  pt_tag_title->Draw();
  first->Print(m_outputPdfName.c_str());
}

void SVDLocalCalibrationsCheckModule::printLayerPage(int layer)
{

  TCanvas* cLayer = new TCanvas();
  cLayer->cd();
  TPaveText* pt_title = new TPaveText(.05, .4, .95, 0.6, "blNDC");
  char name[50];
  sprintf(name, "Layer %d", layer);
  pt_title->AddText(name);
  pt_title->SetTextFont(42);
  pt_title->SetTextColor(kBlack);
  pt_title->SetShadowColor(0);
  pt_title->SetFillColor(10);
  pt_title->SetBorderSize(0);
  pt_title->SetTextSize(0.08);
  pt_title->Draw();
  cLayer->Print(m_outputPdfName.c_str());

}

void SVDLocalCalibrationsCheckModule::printPage(VxdID theVxdID, TList* listUBAD, TList* listVBAD, TList* listUGOOD,
                                                TList* listVGOOD, TString variable)
{

  TH2F* refU = nullptr;
  TH2F* refV = nullptr;
  TH2F* checkU = nullptr;
  TH2F* checkV = nullptr;

  //  listUBAD->Print();
  //  listUGOOD->Print();
  //  listVBAD->Print();
  //  listVGOOD->Print();

  if (variable == "Noise") {
    refU = m_h2NoiseREF->getHistogram(theVxdID, 1);
    refV = m_h2NoiseREF->getHistogram(theVxdID, 0);
    checkU = m_h2NoiseCHECK->getHistogram(theVxdID, 1);
    checkV = m_h2NoiseCHECK->getHistogram(theVxdID, 0);
  } else   if (variable == "Gain") {
    refU = m_h2GainREF->getHistogram(theVxdID, 1);
    refV = m_h2GainREF->getHistogram(theVxdID, 0);
    checkU = m_h2GainCHECK->getHistogram(theVxdID, 1);
    checkV = m_h2GainCHECK->getHistogram(theVxdID, 0);
  } else   if (variable == "Pedestal") {
    refU = m_h2PedestalREF->getHistogram(theVxdID, 1);
    refV = m_h2PedestalREF->getHistogram(theVxdID, 0);
    checkU = m_h2PedestalCHECK->getHistogram(theVxdID, 1);
    checkV = m_h2PedestalCHECK->getHistogram(theVxdID, 0);
  } /*else if(variable == "Mask"){
    refU = m_h2MaskREF->getHistogram(theVxdID, 1);
    refV = m_h2MaskREF->getHistogram(theVxdID, 0);
    checkU = m_h2MaskCHECK->getHistogram(theVxdID, 1);
    checkV = m_h2MaskCHECK->getHistogram(theVxdID, 0);
    } */

  refU->SetMarkerColor(kRed);
  refV->SetMarkerColor(kRed);
  checkU->SetMarkerColor(kBlue);
  checkV->SetMarkerColor(kBlue);

  float min = refU->GetYaxis()->GetXmin();
  float max = refU->GetYaxis()->GetXmax();

  //create APVlines
  TLine l1(128, min, 128, max);
  TLine l2(128 * 2, min, 128 * 2, max);
  TLine l3(128 * 3, min, 128 * 3, max);
  TLine l4(128 * 4, min, 128 * 4, max);
  TLine l5(128 * 5, min, 128 * 5, max);
  l1.SetLineColor(15);
  l2.SetLineColor(15);
  l3.SetLineColor(15);
  l4.SetLineColor(15);
  l5.SetLineColor(15);
  TCanvas* c = new TCanvas();
  c->Divide(2, 2);
  c->cd(1);
  refU->Draw();
  l1.Draw("same");
  l2.Draw("same");
  l3.Draw("same");
  l4.Draw("same");
  l5.Draw("same");
  refU->Draw("same");
  checkU->Draw("same");
  m_leg2D->Draw("same");

  c->cd(2);
  TH1F* objDiff;
  int count = 0;
  if (m_plotGoodAPVs) {
    TIter nextH_uGood(listUGOOD);
    while ((objDiff = (TH1F*)nextH_uGood())) {
      objDiff->SetFillStyle(3004);
      if (count == 0)
        objDiff->Draw();
      else
        objDiff->Draw("same");
      count++;
    }
  }
  TIter nextH_uBad(listUBAD);
  while ((objDiff = (TH1F*)nextH_uBad())) {
    objDiff->SetFillStyle(0);
    if (count == 0)
      objDiff->Draw();
    else
      objDiff->Draw("same");
    count++;
  }
  if (count > 0)
    m_legU->Draw("same");

  c->cd(3);
  refV->Draw();
  l1.Draw("same");
  l2.Draw("same");
  l3.Draw("same");
  refV->Draw("same");
  checkV->Draw("same");
  m_leg2D->Draw("same");

  c->cd(4);
  TIter nextH_vBad(listVBAD);
  count = 0;
  if (m_plotGoodAPVs) {
    TIter nextH_vGood(listVGOOD);
    while ((objDiff = (TH1F*)nextH_vGood())) {
      objDiff->SetFillStyle(3004);
      if (count == 0)
        objDiff->Draw();
      else
        objDiff->Draw("same");
      count++;
    }
  }
  while ((objDiff = (TH1F*)nextH_vBad())) {
    objDiff->SetFillStyle(0);
    if (count == 0)
      objDiff->Draw();
    else
      objDiff->Draw("same");
    count++;
  }
  if (count > 0)
    m_legV->Draw("same");

  c->Print(m_outputPdfName.c_str());

}

int SVDLocalCalibrationsCheckModule::hasAnyProblem(TH1F* h, float cutAve, float cutCOUNT)
{

  float average = h->GetMean();
  if (average > cutAve)
    return 1;

  TAxis* xaxis = h->GetXaxis();
  Int_t bin1 = xaxis->FindBin(-cutCOUNT);
  Int_t bin2 = xaxis->FindBin(cutCOUNT);
  if (bin1 > bin2) {
    int tmp = bin1;
    bin1 = bin2;
    bin2 = tmp;
  }

  B2DEBUG(1, bin1 << " -> " << bin2 << " with " << xaxis->GetNbins() << " bins");

  if (h->Integral(1, bin1) > m_cutN_out - 1) return 2;

  if (h->Integral(bin2, xaxis->GetNbins()) > m_cutN_out - 1) return 3;

  return 0;
}


void SVDLocalCalibrationsCheckModule::setAPVHistoStyles(SVDAPVHistograms<TH1F>* m_APVhistos)
{

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> svdLayers = aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();

  while ((itSvdLayers != svdLayers.end()) && (itSvdLayers->getLayerNumber() != 7)) { //loop on Layers

    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) { //loop on Ladders

      std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();

      while (itSvdSensors != svdSensors.end()) { //loop on sensors

        int layer = itSvdSensors->getLayerNumber();
        int ladder =  itSvdSensors->getLadderNumber();
        int sensor = itSvdSensors->getSensorNumber();
        Belle2::VxdID theVxdID(layer, ladder, sensor);
        const SVD::SensorInfo* currentSensorInfo = dynamic_cast<const SVD::SensorInfo*>(&VXD::GeoCache::get(theVxdID));

        for (int side = 0; side < 2; side++) {

          int Ncells = currentSensorInfo->getUCells();
          if (side == 0)
            Ncells = currentSensorInfo->getVCells();

          int Napv = Ncells / 128;

          for (int m_APV = 0; m_APV < Napv; m_APV++) {

            TH1F* h = m_APVhistos->getHistogram(theVxdID, side, m_APV);

            h->SetFillColor(m_apvColors[m_APV]);
            h->SetLineColor(m_apvColors[m_APV]);
            h->SetMarkerColor(m_apvColors[m_APV]);
          }
        }

        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }


}


void SVDLocalCalibrationsCheckModule::printSummaryPages()
{

  TCanvas* noise = new TCanvas();
  noise->SetGridx();
  noise->Divide(2, 2);
  noise->cd(1);
  m_hNoiseSummary->getHistogram(1)->Draw("colztext");
  noise->cd(3);
  m_hNoiseSummary->getHistogram(0)->Draw("colztext");
  noise->Print(m_outputPdfName.c_str());

  TCanvas* gain = new TCanvas();
  gain->Divide(2, 2);
  gain->cd(1);
  m_hGainSummary->getHistogram(1)->Draw("colztext");
  gain->cd(3);
  m_hGainSummary->getHistogram(0)->Draw("colztext");
  gain->Print(m_outputPdfName.c_str());

  TCanvas* pedestal = new TCanvas();
  pedestal->Divide(2, 2);
  pedestal->cd(1);
  m_hPedestalSummary->getHistogram(1)->Draw("text");
  pedestal->cd(3);
  m_hPedestalSummary->getHistogram(0)->Draw("text");
  pedestal->Print(m_outputPdfName.c_str());


}

void SVDLocalCalibrationsCheckModule::printLastPage()
{

  TCanvas* empty = new TCanvas();
  TString pdf_close = TString(m_outputPdfName) + "]";
  empty->Print(pdf_close);


}


void SVDLocalCalibrationsCheckModule::createLegends()
{


  m_legU = new TLegend(0.75, 0.55, 0.89, 0.89);
  m_legV = new TLegend(0.75, 0.65, 0.89, 0.89);
  m_legU->SetFillStyle(0);
  m_legV->SetFillStyle(0);


  TH1F* hAPV1 = new TH1F("apv1", "apv 1", 1, 0, 1);
  hAPV1->SetLineColor(m_apvColors[0]);
  m_legU->AddEntry(hAPV1, "apv 1", "l");
  m_legV->AddEntry(hAPV1, "apv 1", "l");
  TH1F* hAPV2 = new TH1F("apv2", "apv 2", 2, 0, 2);
  hAPV2->SetLineColor(m_apvColors[1]);
  hAPV2->SetMarkerColor(m_apvColors[1]);
  hAPV2->SetMarkerStyle(21);
  hAPV2->SetMarkerSize(0.5);
  m_legU->AddEntry(hAPV2, "apv 2", "l");
  m_legV->AddEntry(hAPV2, "apv 2", "l");
  TH1F* hAPV3 = new TH1F("apv3", "apv 3", 3, 0, 3);
  hAPV3->SetLineColor(m_apvColors[2]);
  m_legU->AddEntry(hAPV3, "apv 3", "l");
  m_legV->AddEntry(hAPV3, "apv 3", "l");
  TH1F* hAPV4 = new TH1F("apv4", "apv 4", 4, 0, 4);
  hAPV4->SetLineColor(m_apvColors[3]);
  hAPV4->SetMarkerColor(m_apvColors[3]);
  hAPV4->SetMarkerStyle(21);
  hAPV4->SetMarkerSize(0.5);
  m_legU->AddEntry(hAPV4, "apv 4", "l");
  m_legV->AddEntry(hAPV4, "apv 4", "l");
  TH1F* hAPV5 = new TH1F("apv5", "apv 5", 5, 0, 5);
  hAPV5->SetLineColor(m_apvColors[4]);
  m_legU->AddEntry(hAPV5, "apv 5", "l");
  TH1F* hAPV6 = new TH1F("apv6", "apv 6", 6, 0, 6);
  hAPV6->SetLineColor(m_apvColors[5]);
  m_legU->AddEntry(hAPV6, "apv 6", "l");

  m_leg2D = new TLegend(0.78, 0.75, 0.89, 0.89);
  m_leg2D->AddEntry(hAPV2, "ref", "pl");
  m_leg2D->AddEntry(hAPV4, "check", "pl");
  m_leg2D->SetFillStyle(0);
}

void SVDLocalCalibrationsCheckModule::endRun()
{
}
