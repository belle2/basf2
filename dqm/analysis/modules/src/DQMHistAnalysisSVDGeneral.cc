//+
// File : DQMHistAnalysisSVDGeneral.cc
// Description :
//
// Author : Giulia Casarosa (PI), Gaetano De Marino (PI), Luigi Corona (PI)
// Date : 20181127
//-


#include <dqm/analysis/modules/DQMHistAnalysisSVDGeneral.h>
#include <vxd/geometry/GeoCache.h>

#include <TROOT.h>
#include <TStyle.h>
#include <TString.h>
#include <TPaletteAxis.h>
#include <TBox.h>
#include <TAxis.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisSVDGeneral)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisSVDGeneralModule::DQMHistAnalysisSVDGeneralModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  B2INFO("DQMHistAnalysisSVDGeneral: Constructor done.");
  addParam("RefHistoFile", m_refFileName, "Reference histrogram file name", std::string("SVDrefHisto.root"));
  addParam("unpackerErrorLevel", m_unpackError, "Maximum bin_content/ # events allowed before throwing ERROR", double(0.00001));
  addParam("occLevel_Error", m_occError, "Maximum Occupancy (%) allowed for safe operations (red)", float(5));
  addParam("occLevel_Warning", m_occWarning, "Occupancy (%) at WARNING level (orange)", float(3));
  addParam("occLevel_Empty", m_occEmpty, "Maximum Occupancy (%) for which the sensor is considered empty", float(0));
  addParam("printCanvas", m_printCanvas, "if True prints pdf of the analysis canvas", bool(false));

}


DQMHistAnalysisSVDGeneralModule::~DQMHistAnalysisSVDGeneralModule() { }

void DQMHistAnalysisSVDGeneralModule::initialize()
{
  B2INFO("DQMHistAnalysisSVDGeneral: initialized.");
  B2INFO(" black = " << kBlack);
  B2INFO(" green = " << kGreen);
  B2INFO(" orange = " << kOrange);
  B2INFO(" Red = " << kRed);

  m_refFile = NULL;
  if (m_refFileName != "") {
    m_refFile = new TFile(m_refFileName.data(), "READ");
  }

  //search for reference
  if (m_refFile && m_refFile->IsOpen()) {
    B2INFO("SVD DQMHistAnalysis: reference root file (" << m_refFileName << ") FOUND, reading ref histograms");

    TH1F* ref_occ = (TH1F*)m_refFile->Get("refOccupancy");
    if (!ref_occ)
      B2WARNING("SVD DQMHistAnalysis: Occupancy Level Refence not found! using module parameters");
    else {
      m_occEmpty = ref_occ->GetBinContent(1);
      m_occWarning = ref_occ->GetBinContent(2);
      m_occError = ref_occ->GetBinContent(3);
    }
  } else
    B2WARNING("SVD DQMHistAnalysis: reference root file (" << m_refFileName << ") not found, or closed, using module parameters");

  B2INFO(" OCCUPANCY EMPTY occ < " << m_occEmpty);
  B2INFO(" OCCUPANCY OK " << m_occEmpty << " < occ < " << m_occWarning);
  B2INFO(" OCCUPANCY WARNING " << m_occWarning << " < occ < " << m_occError);
  B2INFO(" OCCUPANCY EMPTY occ > " << m_occError);

  m_legError = new TPaveText(-1, 54, 3, 57.5);
  m_legError->AddText("ERROR!!");
  m_legError->SetFillColor(kRed);
  m_legError->SetTextColor(kWhite);

  VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //collect the list of all SVD Modules in the geometry here
  std::vector<VxdID> sensors = geo.getListOfSensors();
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geo.getSensorInfo(aVxdID);
    // B2INFO("VXD " << aVxdID);
    if (info.getType() != VXD::SensorInfoBase::SVD) continue;
    m_SVDModules.push_back(aVxdID); // reorder, sort would be better
  }
  std::sort(m_SVDModules.begin(), m_SVDModules.end());  // back to natural order

  //occupancy plots legend
  m_legProblem = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legProblem->AddText("ERROR!");
  m_legProblem->AddText("at least one sensor with:");
  m_legProblem->AddText(Form("occupancy > %1.1f%%", m_occError));
  m_legProblem->SetFillColor(kRed);
  //  m_legProblem->SetBorderSize(0.5);
  //  m_legProblem->SetLineColor(kBlack);
  m_legWarning = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legWarning->AddText("WARNING!");
  m_legWarning->AddText("at least one sensor with:");
  m_legWarning->AddText(Form("%1.1f%% < occupancy < %1.1f%%", m_occWarning, m_occError));
  m_legWarning->SetFillColor(kOrange);
  //  m_legWarning->SetBorderSize(0.5);
  //m_legWarning->SetLineColor(kBlack);
  m_legNormal = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legNormal->AddText("OCCUPANCY WITHIN LIMITS");
  m_legNormal->AddText(Form("%1.1f%% < occupancy < %1.1f%%", m_occEmpty, m_occWarning));
  m_legNormal->SetFillColor(kGreen);
  m_legNormal->SetBorderSize(0.5);
  m_legNormal->SetLineColor(kBlack);
  m_legEmpty = new TPaveText(11, findBinY(4, 3) - 2, 16, findBinY(4, 3));
  m_legEmpty->AddText("NO DATA RECEIVED");
  m_legEmpty->AddText("from at least one sensor");
  m_legEmpty->SetFillColor(kBlack);
  m_legEmpty->SetTextColor(kWhite);
  m_legEmpty->SetBorderSize(0.5);
  m_legEmpty->SetLineColor(kBlack);


  //occupancy plot Y axis title
  m_yTitle = new TText(-0.75, 13, "layer.ladder.sensor");
  m_yTitle->SetTextAngle(90);
  m_yTitle->SetTextSize(0.03);
  m_yTitle->SetTextFont(42);

  gROOT->cd();
  m_cUnpacker = new TCanvas("c_SVDDataFormat");
  m_cUnpacker->SetGrid(1);
  m_cOccupancyU = new TCanvas("c_SVDOccupancyU");
  //  m_cOccupancyU->SetGrid(1);
  m_cOccupancyV = new TCanvas("c_SVDOccupancyV");
  //  m_cOccupancyV->SetGrid(1);




  const int nY = 19;
  TString Ylabels[nY] = {"", "L3.x.1", "L3.x.2",
                         "", "L4.x.1", "L4.x.2", "L4.x.3",
                         "", "L5.x.1", "L5.x.2", "L5.x.3", "L5.x.4",
                         "", "L6.x.1", "L6.x.2", "L6.x.3", "L6.x.4", "L6.x.5", ""
                        };


  m_hOccupancyV =  new TH2F("hOccupancyV", "Average Sensor Occupancy (%), V side ", 16, 0.5, 16.5, 19, 0, 19);
  m_hOccupancyV->SetMarkerSize(1.1);
  m_hOccupancyV->GetXaxis()->SetTitle("ladder number");
  m_hOccupancyV->GetXaxis()->SetLabelSize(0.04);
  for (unsigned short i = 0; i < nY; i++) m_hOccupancyV->GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());

  m_hOccupancyU =  new TH2F("hOccupancyU", "Average Sensor Occupancy (%), U side ", 16, 0.5, 16.5, 19, 0, 19);
  m_hOccupancyU->SetMarkerSize(1.1);
  m_hOccupancyU->GetXaxis()->SetTitle("ladder number");
  m_hOccupancyU->GetXaxis()->SetLabelSize(0.04);
  for (unsigned short i = 0; i < nY; i++) m_hOccupancyU->GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());
}


void DQMHistAnalysisSVDGeneralModule::beginRun()
{
  B2INFO("DQMHistAnalysisSVDGeneral: beginRun called.");
  m_cUnpacker->Clear();
  m_cOccupancyU->Clear();
  m_cOccupancyV->Clear();

}

void DQMHistAnalysisSVDGeneralModule::event()
{

  //SETUP gSTYLE - all plots
  //  gStyle->SetOptStat(0);
  //  gStyle->SetTitleY(.97);

  //find nEvents
  TH1* hnEvnts = findHist("SVDExpReco/SVDDQM_nEvents");
  if (hnEvnts == NULL) {
    B2INFO("no events, nothing to do here");
    return;
  }
  TString runID = TString((hnEvnts->GetTitle())).Remove(0, 21);
  B2INFO("runID = " << runID);
  Float_t nEvents = hnEvnts->GetEntries();

  //check DATA FORMAT
  TH1* h = findHist("SVDUnpacker/DQMUnpackerHisto");

  //test ERROR:
  //  h->SetBinContent(100,0.01);

  if (h != NULL) {
    h->SetTitle("SVD Data Format Monitor " + runID);
    //check if number of errors is above the allowed limit
    bool hasError = false;
    for (int un = 0; un < h->GetNcells(); un++)
      if (h->GetBinContent(un) / nEvents > m_unpackError)
        hasError = true;
    if (! hasError) {
      m_cUnpacker->SetFillColor(kGreen);
      m_cUnpacker->SetFrameFillColor(10);
    } else {
      m_legError->Draw("same");
      m_cUnpacker->SetFillColor(kRed);
      m_cUnpacker->SetFrameFillColor(10);
    }
  } else {
    B2INFO("Histogram SVDUnpacker/DQMUnpackerHisto from SVDUnpackerDQM not found!");
    m_cUnpacker->SetFillColor(kRed);
  }

  m_cUnpacker->cd();
  h->Draw("colztext");
  h->SetStats(0);


  m_cUnpacker->Modified();
  m_cUnpacker->Update();

  if (m_printCanvas)
    m_cUnpacker->Print("c_SVDDataFormat.pdf");



  //check MODULE OCCUPANCY

  //update titles with exp and run number
  m_hOccupancyU->SetTitle(m_hOccupancyU->GetTitle() + runID);
  m_hOccupancyU->SetStats(0);
  m_hOccupancyV->SetTitle(m_hOccupancyV->GetTitle() + runID);
  m_hOccupancyU->SetStats(0);

  //set dedicate gStyle
  const Int_t colNum = 4;
  Int_t palette[colNum] {kBlack,  kGreen, kOrange, kRed};
  gStyle->SetPalette(colNum, palette);
  gStyle->SetOptStat(0);
  gStyle->SetPaintTextFormat("2.3f");

  TH1* htmp = NULL;

  Int_t nStrips = 768;
  for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
    int tmp_layer = m_SVDModules[i].getLayerNumber();
    int tmp_ladder = m_SVDModules[i].getLadderNumber();
    int tmp_sensor = m_SVDModules[i].getSensorNumber();

    Int_t bin = m_hOccupancyU->FindBin(tmp_ladder, findBinY(tmp_layer, tmp_sensor));

    //look for U histogram - OFFLINE ZS
    TString tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_StripCountU", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = findHist(tmpname.Data());
    if (htmp == NULL) {
      B2INFO("Occupancy U histogram not found");
      m_cOccupancyU->SetFillColor(kRed);
    } else {

      Float_t occU = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancyU->SetBinContent(bin, occU);

      //test ERRORS
      /*
      if(bin == m_hOccupancyU->FindBin(2, findBinY(3, 1))){
      occU = 0; //0,1,1.5,1.8,2,3
      m_hOccupancyU->SetBinContent(bin, occU);
      }

      if(bin == m_hOccupancyU->FindBin(3, findBinY(3, 1))){
      occU = 1.8; //0,1,1.5,1.8,2,3
      m_hOccupancyU->SetBinContent(bin, occU);
      }

      if(bin == m_hOccupancyU->FindBin(4, findBinY(3, 1))){
      occU = 2.1; //0,1,1.5,1.8,2,3
      m_hOccupancyU->SetBinContent(bin, occU);
      }
      */
      if (occU > m_occEmpty && occU < m_occWarning) {
        if (m_occUstatus == 0) m_occUstatus = 0;
      } else {
        if (occU <= m_occEmpty) {
          if (m_occUstatus < 1) m_occUstatus = 1;
        } else if (occU <= m_occError) {
          if (m_occUstatus < 2) m_occUstatus = 2;
        } else if (occU > m_occError) {
          if (m_occUstatus < 3) m_occUstatus = 3;
        }
      }
      B2INFO(" x = " << tmp_ladder << ", y = " << tmp_layer * 10 + tmp_sensor << " U occ = " << occU << " status = " << m_occUstatus);

    }

    //look for V histogram - OFFLINE ZS
    tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_StripCountV", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = findHist(tmpname.Data());
    if (htmp == NULL) {
      B2INFO("Occupancy V histogram not found");
      m_cOccupancyV->SetFillColor(kRed);
    } else {

      if (tmp_layer != 3)
        nStrips = 512;

      Float_t occV = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancyV->SetBinContent(bin, occV);

      //test ERRORS
      /*
      if(bin == m_hOccupancyV->FindBin(3, findBinY(3, 1))){
      occV = 1.8; //0,1,1.5,1.8,2,3
      m_hOccupancyV->SetBinContent(bin, occV);
      }
      */

      if (occV > m_occEmpty && occV < m_occWarning) {
        if (m_occVstatus == 0) m_occVstatus = 0;
      } else {
        if (occV <= m_occEmpty) {
          if (m_occVstatus < 1) m_occVstatus = 1;
        } else if (occV <= m_occError) {
          if (m_occVstatus < 2) m_occVstatus = 2;
        } else if (occV > m_occError) {
          if (m_occVstatus < 3) m_occVstatus = 3;
        }
      }

      //B2INFO(" x = " << tmp_ladder << ", y = " << tmp_layer * 10 + tmp_sensor << " V occ = " << occV << " status = " << m_occVstatus);
    }
  }

  //update U canvas
  m_cOccupancyU->cd();
  m_hOccupancyU->Draw("text");
  m_yTitle->Draw("same");

  if (m_occUstatus == 0) {
    m_cOccupancyU->SetFillColor(kGreen);
    m_cOccupancyU->SetFrameFillColor(10);
    m_legNormal->Draw("same");
  } else {
    if (m_occUstatus == 3) {
      m_cOccupancyU->SetFillColor(kRed);
      m_cOccupancyU->SetFrameFillColor(10);
      m_legProblem->Draw("same");
    }
    if (m_occUstatus == 2) {
      m_cOccupancyU->SetFillColor(kOrange);
      m_cOccupancyU->SetFrameFillColor(10);
      m_legWarning->Draw("same");
    }
    if (m_occUstatus == 1) {
      m_cOccupancyU->SetFillColor(kGray);
      m_cOccupancyU->SetFrameFillColor(10);
      m_legEmpty->Draw("same");
    }
  }
  m_cOccupancyU->Draw();
  m_cOccupancyU->Update();
  m_cOccupancyU->Modified();
  m_cOccupancyU->Update();


  //update V canvas
  m_cOccupancyV->cd();
  m_hOccupancyV->Draw("text");
  m_yTitle->Draw("same");

  if (m_occVstatus == 0) {
    m_cOccupancyV->SetFillColor(kGreen);
    m_cOccupancyV->SetFrameFillColor(10);
    m_legNormal->Draw("same");
  } else {
    if (m_occVstatus == 3) {
      m_cOccupancyV->SetFillColor(kRed);
      m_cOccupancyV->SetFrameFillColor(10);
      m_legProblem->Draw("same");
    }
    if (m_occVstatus == 2) {
      m_cOccupancyV->SetFillColor(kOrange);
      m_cOccupancyV->SetFrameFillColor(10);
      m_legWarning->Draw("same");
    }
    if (m_occVstatus == 1) {
      m_cOccupancyV->SetFillColor(kGray);
      m_cOccupancyV->SetFrameFillColor(10);
      m_legEmpty->Draw("same");
    }
  }

  m_cOccupancyV->Draw();
  m_cOccupancyV->Update();
  m_cOccupancyV->Modified();
  m_cOccupancyV->Update();

  if (m_printCanvas) {
    m_cOccupancyU->Print("c_SVDOccupancyU.pdf");
    m_cOccupancyV->Print("c_SVDOccupancyV.pdf");
  }
}

void DQMHistAnalysisSVDGeneralModule::endRun()
{
  B2INFO("DQMHistAnalysisSVDGeneral:  endRun called");
}


void DQMHistAnalysisSVDGeneralModule::terminate()
{
  B2INFO("DQMHistAnalysisSVDGeneral: terminate called");

  delete m_legProblem;
  delete m_legWarning;
  delete m_legNormal;
  delete m_legEmpty;
  delete m_legError;
  delete m_yTitle;

  delete m_cUnpacker;
  delete m_hOccupancyU;
  delete m_cOccupancyU;
  delete m_hOccupancyV;
  delete m_cOccupancyV;

}


Int_t DQMHistAnalysisSVDGeneralModule::findBinY(Int_t layer, Int_t sensor)
{

  if (layer == 3)
    return sensor; //2
  if (layer == 4)
    return 2 + 1 + sensor; //6
  if (layer == 5)
    return 6 + 1 + sensor; // 11
  if (layer == 6)
    return 11 + 1 + sensor; // 17
  else
    return -1;
}
