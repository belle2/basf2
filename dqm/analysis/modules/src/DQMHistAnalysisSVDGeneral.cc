//+
// File : DQMHistAnalysisSVDGeneral.cc
// Description :
//
// Author : Giulia Casarosa (PI), Gaetano De Marino (PI)
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

  m_legError = new TPaveText(1, 49.5, 5, 53.5);
  m_legError->AddText("ERROR!!");
  m_legError->SetFillColor(kRed);
  m_legError->SetTextColor(kWhite);

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

  //occupancy plots legend
  m_leg = new TPaveText(14, findBinY(4, 3), 16, findBinY(4, 3) + 1);
  m_leg->AddText("legend");
  m_leg->SetFillStyle(0);
  m_leg->SetBorderSize(0);
  m_legProblem = new TPaveText(14, findBinY(4, 3) - 1, 16, findBinY(4, 3));
  m_legProblem->AddText("PROBLEM");
  m_legProblem->SetFillColor(kRed);
  m_legWarning = new TPaveText(14, findBinY(4, 3) - 2, 16, findBinY(4, 3) - 1);
  m_legWarning->AddText("WARNING");
  m_legWarning->SetFillColor(kOrange);
  m_legNormal = new TPaveText(14, findBinY(4, 3) - 3, 16, findBinY(4, 3) - 2);
  m_legNormal->AddText("NORMAL");
  m_legNormal->SetFillColor(kGreen);
  m_legEmpty = new TPaveText(14, findBinY(4, 3) - 4, 16, findBinY(4, 3) - 3);
  m_legEmpty->AddText("NO DATA");
  m_legEmpty->SetFillColor(kBlack);
  m_legEmpty->SetTextColor(kWhite);


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


  m_hOccupancyV =  new TH2F("hOccupancyVnew", "Average Sensor Occupancy (%), V side", 16, 0.5, 16.5, 19, 0, 19);
  m_hOccupancyV->SetMarkerSize(1.1);
  m_hOccupancyV->GetXaxis()->SetTitle("ladder number");
  m_hOccupancyV->GetXaxis()->SetLabelSize(0.04);
  for (unsigned short i = 0; i < nY; i++) m_hOccupancyV->GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());

  m_hOccupancyU =  new TH2F("hOccupancyUnew", "Average Sensor Occupancy (%), U side", 16, 0.5, 16.5, 19, 0, 19);
  m_hOccupancyU->SetMarkerSize(1.1);
  m_hOccupancyU->GetXaxis()->SetTitle("ladder number");
  m_hOccupancyU->GetXaxis()->SetLabelSize(0.04);
  for (unsigned short i = 0; i < nY; i++) m_hOccupancyU->GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());
}


void DQMHistAnalysisSVDGeneralModule::beginRun()
{
  //B2DEBUG(20, "DQMHistAnalysisSVDGeneral: beginRun called.");
  m_cUnpacker->Clear();
  m_cOccupancyU->Clear();
  m_cOccupancyV->Clear();

  m_cOccupancyU->cd();
  m_hOccupancyU->Draw("text");
  m_cOccupancyV->cd();
  m_hOccupancyV->Draw("text");

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
    h->Draw("colztext");
    //no entries mean no error
    if (h->GetEntries() == 0)
      m_cUnpacker->SetFillColor(kGreen);
    else {
      color = kRed;
      m_legError->Draw("same");
    }
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

    Int_t bin = m_hOccupancyU->FindBin(tmp_ladder, findBinY(tmp_layer, tmp_sensor));
    //look for U histogram
    TString tmpname = Form("SVDExpReco/DQMER_SVD_%d_%d_%d_StripCountU", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = findHist(tmpname.Data());
    if (htmp == NULL) {
      B2DEBUG(10, "Occupancy U histogram not found");
      m_cOccupancyU->SetFillColor(kRed);
    } else {

      Float_t occU = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancyU->SetBinContent(bin, occU);

      color = kRed;
      if (occU <= m_occEmpty)
        color = kBlack;
      else if (occU <= m_occWarning)
        color = kGreen;
      else if (occU <= m_occError)
        color = kOrange;
      m_cOccupancyU->cd();
      boxOcc(tmp_layer, tmp_ladder, tmp_sensor, color)->Draw("same");

      B2DEBUG(20, " x = " << tmp_ladder << ", y = " << tmp_layer * 10 + tmp_sensor << " U occ = " << occU);
    }

    //look for V histogram
    tmpname = Form("SVDExpReco/DQMER_SVD_%d_%d_%d_StripCountV", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = findHist(tmpname.Data());
    if (htmp == NULL) {
      B2DEBUG(10, "Occupancy V histogram not found");
      m_cOccupancyV->SetFillColor(kRed);
    } else {

      if (tmp_layer != 3)
        nStrips = 512;

      Float_t occV = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancyV->SetBinContent(bin, occV);
      if (tmp_layer == 5 && tmp_ladder == 5 && tmp_sensor == 1)
        B2INFO(htmp->GetEntries() << ", " << nStrips << ", " << nEvents << " = " << occV);

      color = kRed;
      if (occV <= m_occEmpty)
        color = kBlack;
      else if (occV <= m_occWarning)
        color = kGreen;
      else if (occV <= m_occError)
        color = kOrange;
      m_cOccupancyV->cd();
      boxOcc(tmp_layer, tmp_ladder, tmp_sensor, color)->Draw("same");

      B2DEBUG(20, " x = " << tmp_ladder << ", y = " << tmp_layer * 10 + tmp_sensor << " U occ = " << occV);
    }
  }

  //update U canvas
  m_cOccupancyU->cd();
  m_hOccupancyU->Draw("sametext");
  m_leg->Draw("same");
  m_legProblem->Draw("same");
  m_legWarning->Draw("same");
  m_legNormal->Draw("same");
  m_legEmpty->Draw("same");
  m_yTitle->Draw("same");
  m_cOccupancyU->Modified();
  m_cOccupancyU->Update();

  //update V canvas
  m_cOccupancyV->cd();
  m_hOccupancyV->Draw("sametext");
  m_leg->Draw("same");
  m_legProblem->Draw("same");
  m_legWarning->Draw("same");
  m_legNormal->Draw("same");
  m_legEmpty->Draw("same");
  m_yTitle->Draw("same");
  m_cOccupancyV->Modified();
  m_cOccupancyV->Update();

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

TPaveText* DQMHistAnalysisSVDGeneralModule::boxOcc(Int_t layer, Int_t ladder, Int_t sensor, Int_t color)
{

  Float_t y = findBinY(layer, sensor);
  TPaveText* box = new TPaveText(ladder - 0.5, y, ladder + 0.5, y + 1);
  box->SetFillColor(color);

  return box;

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
