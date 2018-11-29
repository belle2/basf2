//+
// File : DQMHistAnalysisSVDGeneral.cc
// Description :
//
// Author : Giulia Casarosa
// Date : 20181127
//-


#include <dqm/analysis/modules/DQMHistAnalysisSVDGeneral.h>
#include <vxd/geometry/GeoCache.h>

#include <TROOT.h>
#include <TStyle.h>
#include <TString.h>
#include <TPaletteAxis.h>
#include <TPaveText.h>
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
  B2DEBUG(20, "DQMHistAnalysisSVDGeneral: Constructor done.");
  addParam("occLevel_Error", m_occError, "Maximum Occupancy (%) allowed for safe operations (red)", float(5));
  addParam("occLevel_Warning", m_occWarning, "Occupancy (%) at WARNING level (orange)", float(3));
  addParam("occLevel_Empty", m_occEmpty, "Maximum Occupancy (%) for which the sensor is considered empty", float(0));
  addParam("printCanvas", m_printCanvas, "if True prints pdf of the analysis canvas", bool(false));

}


DQMHistAnalysisSVDGeneralModule::~DQMHistAnalysisSVDGeneralModule() { }

void DQMHistAnalysisSVDGeneralModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisSVDGeneral: initialized.");

  VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //collect the list of all SVD Modules in the geometry here
  std::vector<VxdID> sensors = geo.getListOfSensors();
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geo.getSensorInfo(aVxdID);
    // B2DEBUG(20,"VXD " << aVxdID);
    if (info.getType() != VXD::SensorInfoBase::SVD) continue;
    m_SVDModules.push_back(aVxdID); // reorder, sort would be better
  }
  std::sort(m_SVDModules.begin(), m_SVDModules.end());  // back to natural order

  gROOT->cd();
  m_cUnpacker = new TCanvas("c_SVDDataFormat");
  m_cOccupancyU = new TCanvas("c_SVDOccupancyU");
  m_cOccupancyV = new TCanvas("c_SVDOccupancyV");

  m_hOccupancyUtext =  new TH2F("hOccupancyU", "Average Sensor Occupancy (%), U side", 16, 0.5, 16.5, 46, 20.5, 70.5);
  m_hOccupancyUtext->SetMarkerSize(1.1);

  m_hOccupancyVtext =  new TH2F("hOccupancyV", "Average Sensor Occupancy (%), V side", 16, 0.5, 16.5, 46, 20.5, 70.5);
  m_hOccupancyVtext->SetMarkerSize(1.1);

  m_hOccupancyUtext->GetYaxis()->SetTitle("layer*10 + sensor number");
  m_hOccupancyUtext->GetXaxis()->SetTitle("ladder number");
  m_hOccupancyVtext->GetYaxis()->SetTitle("layer*10 + sensor number");
  m_hOccupancyVtext->GetXaxis()->SetTitle("ladder number");
  m_hOccupancyUcolz = new TH2F(*m_hOccupancyUtext);
  m_hOccupancyUcolz->SetName("hOccupancyUcolz");
  m_hOccupancyUcolz->SetMinimum(0);
  m_hOccupancyUcolz->SetMaximum(4);
  m_hOccupancyVcolz = new TH2F(*m_hOccupancyVtext);
  m_hOccupancyVcolz->SetName("hOccupancyVcolz");
  m_hOccupancyVcolz->SetMinimum(0);
  m_hOccupancyVcolz->SetMaximum(4);

}


void DQMHistAnalysisSVDGeneralModule::beginRun()
{
  //B2DEBUG(20, "DQMHistAnalysisSVDGeneral: beginRun called.");
  m_cUnpacker->Clear();
  m_cOccupancyU->Clear();
  m_cOccupancyV->Clear();
}

void DQMHistAnalysisSVDGeneralModule::event()
{

  //SETUP gSTYLE - all plots
  gStyle->SetOptStat(0);

  //check DATA FORMAT
  TH1* h = findHist("SVDUnpacker/DQMUnpackerHisto");

  TBox* b = new TBox(-1, -4.6, 20, 54.5);
  b->SetLineWidth(12);
  b->SetFillStyle(0);
  Int_t color = kGreen;

  if (h != NULL) {
    h->SetTitle("SVD Data Format Monitor");
    m_cUnpacker->cd();
    m_cUnpacker->SetGrid(1);

    //no entries mean no error
    if (h->GetEntries() == 0)
      m_cUnpacker->SetFillColor(kGreen);
    else
      color = kRed;
    h->Draw("colztext");
    b->Draw("same");
    b->SetLineColor(color);
  } else {
    B2DEBUG(20, "Histogram SVDUnpacker/DQMUnpackerHisto from SVDUnpackedDQM not found!");
    m_cUnpacker->SetFillColor(kRed);
  }
  m_cUnpacker->Modified();
  m_cUnpacker->Update();

  if (m_printCanvas)
    m_cUnpacker->Print("c_SVDDataFormat.pdf");

  //check MODULE OCCUPANCY

  //set dedicate gStyle
  const Int_t colNum = 4;
  Int_t palette[colNum] {kBlack,  kGreen, kOrange, kRed};
  gStyle->SetPalette(colNum, palette);
  gStyle->SetOptStat(0);
  gStyle->SetPaintTextFormat("2.3f");

  //find nEvents
  TH1* hnEvnts = findHist("SVDExpReco/DQMER_SVD_nEvents");
  if (hnEvnts == NULL) {
    B2DEBUG(20, "no events, nothing to do here");
    return;
  }
  Float_t nEvents = hnEvnts->GetEntries();

  int  tmp_layer;
  int  tmp_ladder;
  int  tmp_sensor;
  TH1* htmp = NULL;

  Int_t nStrips = 768;
  for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
    tmp_layer = m_SVDModules[i].getLayerNumber();
    tmp_ladder = m_SVDModules[i].getLadderNumber();
    tmp_sensor = m_SVDModules[i].getSensorNumber();

    Int_t bin = m_hOccupancyUtext->FindBin(tmp_ladder, tmp_layer * 10 + tmp_sensor);
    TString tmpname = Form("SVDExpReco/DQMER_SVD_%d_%d_%d_StripCountU", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = findHist(tmpname.Data());
    if (htmp == NULL) {
      B2DEBUG(10, "Occupancy histogram not found");
      m_cOccupancyV->SetFillColor(kRed);
      m_cOccupancyU->SetFillColor(kRed);
    } else {
      Float_t occU = htmp->GetEntries() / nStrips / nEvents * 100;

      m_hOccupancyUtext->SetBinContent(bin, occU);
      if (occU < m_occEmpty)
        m_hOccupancyUcolz->SetBinContent(bin, 1);
      else if (occU < m_occWarning)
        m_hOccupancyUcolz->SetBinContent(bin, 2);
      else if (occU < m_occError)
        m_hOccupancyUcolz->SetBinContent(bin, 3);
      else
        m_hOccupancyUcolz->SetBinContent(bin, 4);

      tmpname = Form("SVDExpReco/DQMER_SVD_%d_%d_%d_StripCountV", tmp_layer, tmp_ladder, tmp_sensor);
      htmp = findHist(tmpname.Data());
      if (htmp == NULL)
        B2DEBUG(10, "Occupancy histogram not found");
      if (tmp_layer != 3)
        nStrips = 512;

      Float_t occV = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancyVtext->SetBinContent(bin, occV);

      if (occV < m_occEmpty)
        m_hOccupancyVcolz->SetBinContent(bin, 1);
      else if (occV < m_occWarning)
        m_hOccupancyVcolz->SetBinContent(bin, 2);
      else if (occV < m_occError)
        m_hOccupancyVcolz->SetBinContent(bin, 3);
      else
        m_hOccupancyVcolz->SetBinContent(bin, 4);

      B2DEBUG(20, " x = " << tmp_ladder << ", y = " << tmp_layer * 10 + tmp_sensor << " U occ = " << occU << ", V occ = " << occV);

    }
    m_hOccupancyUcolz->SetBinContent(m_hOccupancyUtext->FindBin(10, 25), 1);
    m_hOccupancyUcolz->SetBinContent(m_hOccupancyUtext->FindBin(11, 25), 2);
    m_hOccupancyUcolz->SetBinContent(m_hOccupancyUtext->FindBin(12, 25), 3);
    m_hOccupancyUcolz->SetBinContent(m_hOccupancyUtext->FindBin(13, 25), 4);

    m_hOccupancyVcolz->SetBinContent(m_hOccupancyVtext->FindBin(10, 25), 1);
    m_hOccupancyVcolz->SetBinContent(m_hOccupancyVtext->FindBin(11, 25), 2);
    m_hOccupancyVcolz->SetBinContent(m_hOccupancyVtext->FindBin(12, 25), 3);
    m_hOccupancyVcolz->SetBinContent(m_hOccupancyVtext->FindBin(13, 25), 4);

    //legend
    TPaveText* leg = new TPaveText(9.5, 26, 13.5, 27);
    leg->AddText("legend");
    leg->SetFillStyle(0);
    leg->SetBorderSize(0);
    TPaveText* legProblem = new TPaveText(12.5, 24.9, 13.5, 25.9);
    legProblem->AddText("PROBLEM");
    legProblem->SetFillStyle(0);
    TPaveText* legWarning = new TPaveText(11.5, 24.9, 12.5, 25.9);
    legWarning->AddText("WARNING");
    legWarning->SetFillStyle(0);
    TPaveText* legNormal = new TPaveText(10.5, 24.9, 11.5, 25.9);
    legNormal->AddText("NORMAL");
    legNormal->SetFillStyle(0);
    TPaveText* legEmpty = new TPaveText(9.5, 24.9, 10.5, 25.9);
    legEmpty->AddText("EMPTY");
    legEmpty->SetFillStyle(0);
    legEmpty->SetTextColor(kWhite);

    //update U canvas
    m_cOccupancyU->cd();
    m_hOccupancyUtext->Draw("text");
    /*
      FIXME: colors are not shown correctly
      m_hOccupancyUcolz->Draw("samecolz");
      m_hOccupancyUtext->Draw("sametext");
      leg->Draw("same");
      legProblem->Draw("same");
      legWarning->Draw("same");
      legNormal->Draw("same");
      legEmpty->Draw("same");*/
    m_cOccupancyU->Modified();
    m_cOccupancyU->Update();
    //update V canvas
    m_cOccupancyV->cd();
    m_hOccupancyVtext->Draw("text");
    /*
      FIXME: colors are not shown correctly
      m_hOccupancyVcolz->Draw("samecolz");
      m_hOccupancyVtext->Draw("sametext");
      leg->Draw("same");
      legProblem->Draw("same");
      legWarning->Draw("same");
      legNormal->Draw("same");
      legEmpty->Draw("same");
    */
    m_cOccupancyV->Modified();
    m_cOccupancyV->Update();


  }

  if (m_printCanvas) {
    m_cOccupancyU->Print("c_SVDOccupancyU.pdf");
    m_cOccupancyV->Print("c_SVDOccupancyV.pdf");
  }
}

void DQMHistAnalysisSVDGeneralModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisSVDGeneral:  endRun called");
}


void DQMHistAnalysisSVDGeneralModule::terminate()
{
  B2DEBUG(20, "DQMHistAnalysisSVDGeneral: terminate called");
}

