//
//  CDCvalidation021.c 
//    ROOT macro for validation      ver. 0.21
//                      28 June 2013
//                     Hiroki Kanda (Tohoku Univ.)
//                     kanda@m.tains.tohoku.ac.jp
//
//    Usage: Batch mode 
//           root -b CDCvalidation.c+O
//                                  ~~ compilation for faster execution
//           Interactive mode
//           root [1] .L CDCvalidation.c+0
//           root [2] CDCvalidation()
//
//    The input root file, output root file, output drawing files
//    should be specified by editing the macro file itself
//    See "Definition of files" below.
//
//    Even in the interactive mode, CDCvalidation() exits from root
//    after its execution. If you do not want to exit from root and
//    reuse histograms independently, use FillHisto().
//
//  History of update
//  30 May 2013    ver. 0.1   Release version (but still under construction)
//  25 Jun 2013    ver. 0.2   TGraph -> TH1D 
//                            histograms per super layer (saved in another root file)
//  28 Jun 2013    ver. 0.21  Bug fix
//


#include "string.h"

#include "TROOT.h"
#include "TStyle.h"
#include "TFile.h"
#include "TGraph.h"
#include "TTree.h"
#include "TF1.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TVectorD.h"
#include "TMatrixD.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TCanvas.h"
#include "TLine.h"


// Definitions of files
const string cInRoot = "../CDCOutputGen.root";  // input root file
const string cOutRootSL  = "CDCValidation.root";       // output root file (super layer histograms)
const string cOutRootAll = "CDCvalidatationAll.root";      // output root file (all histograms)
const string cOutDraw = "CDCvalidation.pdf";             // output drawing file

const string cOutDrawInit = cOutDraw+"(";
const string cOutDrawFinal = cOutDraw+")";

TFile *flINRoot = new TFile(cInRoot.c_str());
TTree *tree = (TTree*)flINRoot->Get("tree");


// Write option
const Int_t kWriteHisto = 1;       // 0: Disable   1: Enable

// Drawing option
const Int_t kDrawFitting    = 0;   // 0: Disable   1: Enable
const Int_t kDrawLayerHisto = 0;   // 0: Disable   1: Enable


// Bin definitions for TDC histograms
Int_t NbinTDC = 251;
Double_t MinTDC = 0;
Double_t MaxTDC = 502;

// Bin definitions for ADC histograms
Int_t NbinADC = 8192;
Double_t MinADC = 0.;
Double_t MaxADC = 65536.;
Double_t RangeADC = 500.;
Double_t BinFactorADC = (MaxADC-MinADC)/NbinADC;

// Bin definitions for ADC - TDC scatter plot
Int_t Nbin2ADC = 250;                 // number of bins for ADC (horizontal) axis
Double_t Min2ADC = 0;                 // minimum of ADC axis
Double_t Max2ADC = 500;               // maximum of ADC axis
Int_t Nbin2TDC = 250;                 // number of bins for TDC (vertical) axis
Double_t Min2TDC = 0;                 // minimum of TDC axis
Double_t Max2TDC = 500;               // maximum of TDC axis


// Layer definitions
const Int_t MAXLAYER=56;                       // number of layers
const Int_t MAXSUPERLAYER=9;                   // number of super layers
const Int_t MAXLOCALLAYER=8;                   // maximum number of local layers in total

Int_t LayerReady = 0;
Int_t SuperLayer[MAXLAYER];           // Super layer ID of each layer
Int_t LocalLayer[MAXLAYER];           // Local layer ID of each layer
Int_t LayerID[MAXSUPERLAYER][MAXLOCALLAYER];      // Layer ID defined by Super layer ID and Local layer ID
Int_t EncodedLayer[MAXLAYER];         // WireID for a cut on root e_Wire
Int_t MaxLocalLayer[MAXSUPERLAYER];   // number of local layers in each superlayer
Int_t MaxWire[MAXSUPERLAYER] = {160, 160, 192, 224, 256, 288, 320, 352, 384}; // number of cells in each layer


// Histograms
TH1D *hHitPattern[MAXLAYER];
TH1D *hTDC[MAXLAYER];
TH1D *hADC[MAXLAYER], *hADCfit[MAXLAYER];
TH2D *hADCTDC[MAXLAYER];
TH1D *hHitPatternSL[MAXSUPERLAYER];
TH1D *hTDCSL[MAXSUPERLAYER];
TH1D *hADCSL[MAXLAYER];
TH2D *hADCTDCSL[MAXLAYER];

// Landau fitting results
Double_t meanADC[MAXLAYER], sigmaADC[MAXLAYER], Layer[MAXLAYER];
TH1D *hmeanADC, *hsigmaADC;

// Canvases
TCanvas *cv[MAXSUPERLAYER];
TCanvas *cvSL;
TCanvas *cvsummary;


// Functions
void SetLayerID();
void SetHistoSizes(TH1D *h);
void SetPadMargin();

void FillHitPattern(Int_t iLayer);     // Filling functions can be used independently
void FillTDC(Int_t iLayer);            // Filling functions can be used independently
void FillADC(Int_t iLayer);            // Filling functions can be used independently 
void FitADC(Int_t iLayer, Int_t kDraw);// Filling functions can be used independently
void FillADCTDC(Int_t iLayer);         // Filling functions can be used independently

void FillHisto();                      // Fill histograms for all the layers
void PlotHisto();                      // Draw histograms on canvases on the screen
void PrintHisto();                     // Save drawing files of the canvases for layer histograms
void WriteHisto();                     // Save histograms to a root file
Int_t CDCValidation();                 // Main macro for plotting, printing and saving


void SetLayerID()
{
  Int_t iLayer  = 0;
  Int_t MaxLL, WireID;
  Int_t iSL, iLL;

  printf("Initializing layer identifiers\n");
  for(iSL=0; iSL < MAXSUPERLAYER; iSL++){
    for(iLL=0; iLL < MAXLOCALLAYER; iLL++){
      LayerID[iSL][iLL] = -1;
    }
  }

  for(iSL=0; iSL < MAXSUPERLAYER; iSL++){
    if(iSL > 0){
      MaxLL = 6;
    } else {
      MaxLL = 8;
    }
    MaxLocalLayer[iSL] = MaxLL;
    for(iLL=0; iLL <MaxLL; iLL ++){
      WireID = iSL * 8 + iLL;
      SuperLayer[iLayer] = iSL;
      LocalLayer[iLayer] = iLL;
      EncodedLayer[iLayer] = WireID*512;
      Layer[iLayer] = (double) iLayer;
      LayerID[iSL][iLL] = iLayer;
      printf("LayerID: %02d  Super Layer: %02d   Local Layer: %02d  wireID: %02d\n", 
	     iLayer, iSL, iLL, WireID);
      iLayer++;
    }
  }

  LayerReady=1;

  printf("\n\n");

  return;
}


void SetHistoSizes(TH1D *h)
{
  h->GetXaxis()->SetLabelSize(0.06);
  h->GetYaxis()->SetLabelSize(0.06);
  h->GetXaxis()->SetTitleSize(0.08);
  h->GetYaxis()->SetTitleSize(0.08);
  h->GetXaxis()->SetTitleOffset(0.7);
  h->GetYaxis()->SetTitleOffset(0.5);
}


void SetPadMargin()
{
  gPad->SetLeftMargin(0.14);
  gPad->SetRightMargin(0.08);
  gPad->SetTopMargin(0.11);
  gPad->SetBottomMargin(0.15);
}


void FillHitPattern(Int_t iLayer)
{
  if(LayerReady == 0){
    SetLayerID();
  }

  Char_t CutLayer[100];

  sprintf(CutLayer, "int(CDCHits.m_eWire&0xfe00)==%d", EncodedLayer[iLayer]);

  if(hHitPattern[iLayer] == NULL){

    Char_t chName[100], chTitle[100];
    Double_t Min = -0.5, Max =MaxWire[SuperLayer[iLayer]] - 0.5;
  
    sprintf(chName, "hHitPattern%02d", iLayer);
    sprintf(chTitle,"Hit Pattern (Layer %02d:  SuperLayer %2d  LocalLayer %2d",
	    iLayer, SuperLayer[iLayer], LocalLayer[iLayer]);

    hHitPattern[iLayer] = new TH1D(chName, chTitle, MaxWire[SuperLayer[iLayer]], Min, Max);
    hHitPattern[iLayer]->GetXaxis()->SetTitle("Cell");
    hHitPattern[iLayer]->GetYaxis()->SetTitle("Entries");
    hHitPattern[iLayer]->SetStats(0);
    SetHistoSizes(hHitPattern[iLayer]);

    tree->Project(chName, "int(CDCHits.m_eWire&0x01ff)", CutLayer);
  } else {
    printf("Hit pattern for Layer %d is already filled\n", iLayer);
  }

  return;
}


void FillTDC(Int_t iLayer)
{

  if(LayerReady == 0){
    SetLayerID();
  }

  Char_t CutLayer[100];

  sprintf(CutLayer, "int(CDCHits.m_eWire&0xfe00)==%d", EncodedLayer[iLayer]);

  if(hTDC[iLayer] == NULL){
    Char_t chName[100], chTitle[100];
    
    sprintf(chName, "hTDC%02d", iLayer);
    sprintf(chTitle,"TDC (Layer %02d:  SuperLayer %2d  LocalLayer %2d",
	    iLayer, SuperLayer[iLayer], LocalLayer[iLayer]);

    hTDC[iLayer] = new TH1D(chName, chTitle, NbinTDC, MinTDC, MaxTDC);
    hTDC[iLayer]->GetXaxis()->SetTitle("TDC count");
    hTDC[iLayer]->GetYaxis()->SetTitle("Entries");
    SetHistoSizes(hTDC[iLayer]);
    tree->Project(chName, "CDCHits.m_tdcCount", CutLayer);
  } else {
    printf("TDC histogram for Layer %d is already filled\n", iLayer);
  }

  return;
}

void FillADC(Int_t iLayer)
{
  if(LayerReady == 0){
    SetLayerID();
  }

  Char_t CutLayer[100];

  sprintf(CutLayer, "int(CDCHits.m_eWire&0xfe00)==%d", EncodedLayer[iLayer]);

  if(hADC[iLayer] == NULL){
    Char_t chName[100], chNameFit[100], chTitle[100];
    
    sprintf(chName, "hADC%02d", iLayer);
    sprintf(chNameFit, "hADC%02dfit", iLayer);
    sprintf(chTitle,"ADC (Layer %02d:  SuperLayer %2d  LocalLayer %2d)",
	    iLayer, SuperLayer[iLayer], LocalLayer[iLayer]);
  
    hADC[iLayer] = new TH1D(chName, chTitle, NbinADC, MinADC, MaxADC);
    hADC[iLayer]->GetXaxis()->SetTitle("ADC count");
    hADC[iLayer]->GetYaxis()->SetTitle("Entries");
    SetHistoSizes(hADC[iLayer]);
    tree->Project(chName, "CDCHits.m_adcCount", CutLayer);

    hADCfit[iLayer] = (TH1D*) hADC[iLayer]->Clone();
    hADCfit[iLayer]->SetName(chNameFit);
  } else {
    printf("ADC histogram for Layer %d is already filled\n", iLayer);
  }

  return;
}


void FitADC(Int_t iLayer, Int_t kDraw)
// iLayer : Layer ID for fitting 
// kDraw  : Drawing / Printing option
//          0:  No drawing, No printing
//          1:  Drawing shown on the screen, No printing
//          2:  Drawing shown on the screen, Printing to the drawing file  
//              ONLY for being called from FillHisto()
{
  if(hADCfit[iLayer] == NULL){
    printf("ADC histogram for Layer %d is not ready\n", iLayer);
    FillADC(iLayer);
    printf("ADC histogram was filled\n");
  }

  Char_t cOP[10]="0";
  TCanvas *cvfit;

  if(kDraw != 0){
    cvfit = new TCanvas("cfit", "Fitting", 700, 800);
    cvfit->Divide(1,2);
    gStyle->SetOptFit(111);
    strcpy(cOP, "");
  }

  TF1 *fbginit = new TF1("fbginit", "gaus", 0, 65536);
  TF1 *fbgfinal = new TF1("fbgfinal", "gaus", 0, 65536);
  TF1 *finit = new TF1("finit", "gaus(0)+landau(3)", 0, 65536);
  TF1 *ffinal = new TF1("ffinal", "gaus(0)+landau(3)", 0, 65536);

  Double_t par[6];
  Double_t MaxX, Entry;

  TH1D *h = hADCfit[iLayer];
  TH1D *hinit = (TH1D*) h->Clone();

  par[0] = (h->GetBinContent(1)+h->GetBinContent(2))/2.;
  par[1] = 0.;
  par[2] = 200;

  fbginit->SetParameters(par);
  fbginit->SetParLimits(1, -100., 100.);
  fbginit->SetLineColor(kGreen-7);

  if(kDraw != 0) {
    cvfit->cd(1);
    SetPadMargin();
  }
  hinit->SetAxisRange(0, RangeADC);
  hinit->Fit("fbginit", cOP, "", 150,400);
  hinit->GetFunction("fbginit")->GetParameters(par);

  MaxX = h->GetMaximumBin()*BinFactorADC;
  Entry = h->GetEntries();

  par[0]*=0.8;
  par[4] = MaxX;
  par[5] = MaxX/5;
  par[3] = (Entry-par[0]*par[2]/3/3)/par[5]*BinFactorADC*0.8;

  ffinal->SetParLimits(0, 0, fbginit->GetParameter(0)*5);
  ffinal->SetParLimits(2, fbginit->GetParameter(2)*0.6, fbginit->GetParameter(2)*5);
  ffinal->SetParLimits(3, 0, Entry/par[5]*4.);

  printf("\n");
  for(Int_t i=0; i<6; i++){
    printf("Parameter: %d   Value: %f\n", i, par[i]);
  }
  printf("\n");

  finit->SetParameters(par);
  finit->SetLineColor(kRed-9);

  if(kDraw != 0){
    finit->Draw("same");
  }
  
  ffinal->SetParameters(par);
  ffinal->SetParName(0, "Const (BG)");
  ffinal->SetParName(1, "Mean  (BG)");
  ffinal->SetParName(2, "Sigma (BG)");
  ffinal->SetParName(3, "Const (Peak)");
  ffinal->SetParName(4, "MPV   (Peak)");
  ffinal->SetParName(5, "Sigma (Peak)");
  
  if(kDraw != 0) {
    cvfit->cd(2);
    SetPadMargin();
  }
  h->SetAxisRange(0, RangeADC);
  h->Fit("ffinal", cOP, "",0, RangeADC); 
  h->GetFunction("ffinal")->GetParameters(par);

  fbgfinal->SetParameters(par);
  fbgfinal->SetLineColor(kGreen+1);

  if(kDraw!=0){
    fbgfinal->Draw("same");
  }

  meanADC[iLayer] = par[4];
  sigmaADC[iLayer] = par[5];

  if(kDraw == 2){
    if(iLayer == 0){
      cvfit->SaveAs(cOutDrawInit.c_str());
    } else if (iLayer == MAXLAYER-1) {
      if(kDrawLayerHisto){
	cvfit->SaveAs(cOutDraw.c_str());
      } else {
	cvfit->SaveAs(cOutDrawFinal.c_str());
      }
    } else {
      cvfit->SaveAs(cOutDraw.c_str());
    }
  }

  return;
}

void FillADCTDC(Int_t iLayer)
{

  if(LayerReady == 0){
    SetLayerID();
  }

  Char_t CutLayer[100];

  sprintf(CutLayer, "int(CDCHits.m_eWire&0xfe00)==%d", EncodedLayer[iLayer]);

  if(hADCTDC[iLayer] == NULL){

    Char_t chName[100], chTitle[100];
    sprintf(chName, "hADCTDC%02d", iLayer);
    sprintf(chTitle,"ADC-TDC (Layer %02d:  SuperLayer %2d  LocalLayer %2d",
	    iLayer, SuperLayer[iLayer], LocalLayer[iLayer]);

    hADCTDC[iLayer] = new TH2D(chName, chTitle, Nbin2ADC, Min2ADC, Max2ADC, Nbin2TDC, Min2TDC, Max2TDC);
    hADCTDC[iLayer]->GetXaxis()->SetTitle("ADC count");
    hADCTDC[iLayer]->GetYaxis()->SetTitle("TDC count");
    hADCTDC[iLayer]->SetStats(0);
    tree->Project(chName, "CDCHits.m_tdcCount:CDCHits.m_adcCount", CutLayer);

  } else {
    printf("ADC vs. TDC 2D histogram for Layer %d is already filled\n", iLayer);  
  }

  return;
}


void FillHisto()
{
  Int_t iLayer=0;

  for(iLayer = 0; iLayer < MAXLAYER; iLayer++){
    printf("\n\n *****  Currently filling %d  *****\n", iLayer);
    FillHitPattern(iLayer);
    FillTDC(iLayer);
    FillADC(iLayer);
    if(kDrawFitting == 0){
      FitADC(iLayer, 0); 
    } else {
      FitADC(iLayer, 2);
    }
    FillADCTDC(iLayer);
  }

  iLayer = 0;
  for(Int_t iSL=0; iSL<MAXSUPERLAYER; iSL++){
    Char_t Name[100], Title[100];

    sprintf(Name, "hHitPatternSL%02d", iSL);
    sprintf(Title, "Hit Pattern (Super Layer %2d)", iSL);
    hHitPatternSL[iSL] = (TH1D*) hHitPattern[iLayer]->Clone();
    hHitPatternSL[iSL]->SetName(Name);
    hHitPatternSL[iSL]->SetTitle(Title);
    hHitPatternSL[iSL]->SetAxisRange(0, RangeADC);
    hHitPatternSL[iSL]->GetXaxis()->SetTitle("Cell");
    hHitPatternSL[iSL]->GetYaxis()->SetTitle("Entries");

    sprintf(Name, "hADCSL%02d", iSL);
    sprintf(Title, "ADC (Super Layer %2d)", iSL);
    hADCSL[iSL] = (TH1D*) hADC[iLayer]->Clone();
    hADCSL[iSL]->SetName(Name);
    hADCSL[iSL]->SetTitle(Title);
    hADCSL[iSL]->SetAxisRange(0, RangeADC);
    hADCSL[iSL]->GetXaxis()->SetTitle("ADC count");
    hADCSL[iSL]->GetYaxis()->SetTitle("Entries");

    sprintf(Name, "hTDCSL%02d", iSL);
    sprintf(Title, "TDC (Super Layer %2d)", iSL);
    hTDCSL[iSL] = (TH1D*) hTDC[iLayer]->Clone();
    hTDCSL[iSL]->SetName(Name);
    hTDCSL[iSL]->SetTitle(Title);
    hTDCSL[iSL]->GetXaxis()->SetTitle("TDC count");
    hTDCSL[iSL]->GetYaxis()->SetTitle("Entries");

    sprintf(Name, "hADCTDCSL%02d", iSL);
    sprintf(Title, "ADC-TDC (Super Layer %2d)", iSL);
    hADCTDCSL[iSL] = (TH2D*) hADCTDC[iLayer]->Clone();
    hADCTDCSL[iSL]->SetName(Name);
    hADCTDCSL[iSL]->SetTitle(Title);
    hADCTDCSL[iSL]->GetXaxis()->SetTitle("ADC count");
    hADCTDCSL[iSL]->GetYaxis()->SetTitle("TDC count");

    iLayer++;
    Int_t MaxLL = MaxLocalLayer[iSL];
    for(Int_t iLL=1; iLL < MaxLL; iLL ++){
      hHitPatternSL[iSL]->Add(hHitPattern[iLayer]);
      hADCSL[iSL]->Add(hADC[iLayer]);
      hTDCSL[iSL]->Add(hTDC[iLayer]);
      hADCTDCSL[iSL]->Add(hADCTDC[iLayer]);
      iLayer++;
    }
  }

  hmeanADC = new TH1D("hmeanADC", "Mean of Landau peak", MAXLAYER, -0.5, MAXLAYER-0.5);
  hsigmaADC =new TH1D("hsigmaADC", "Sigma of Landau peak", MAXLAYER, -0.5, MAXLAYER-0.5);
  
  for(iLayer = 0; iLayer < MAXLAYER; iLayer++){
    hmeanADC->Fill(Layer[iLayer], meanADC[iLayer]);
    hsigmaADC->Fill(Layer[iLayer], sigmaADC[iLayer]);
  }

  hmeanADC->GetXaxis()->SetTitle("Layer");
  hmeanADC->GetYaxis()->SetTitle("Mean (ADC count)");

  hsigmaADC->GetXaxis()->SetTitle("Layer");
  hsigmaADC->GetYaxis()->SetTitle("Sigma (ADC count)");

  return;
}


void PlotHisto()
{
  Int_t iLayer=0;
  TLine *lborder_mean[MAXSUPERLAYER], *lborder_sigma[MAXSUPERLAYER];

  for(Int_t iSL=0; iSL<MAXSUPERLAYER; iSL++){
    Char_t Name[100], Title[100];
    sprintf(Name, "cv%02d", iSL);
    sprintf(Title, "Super Layer %2d", iSL);
    cv[iSL] = new TCanvas(Name, Title, 800, 1000);
    gStyle->SetOptFit(1111);

    cv[iSL]->Divide(4, 7);
    Int_t MaxLL = MaxLocalLayer[iSL];
    for(Int_t iLL=0; iLL < MaxLL; iLL ++){
      Int_t cid = iLL*4+1;
      cv[iSL]->cd(cid); 
      SetPadMargin();
      hHitPattern[iLayer]->Draw();

      cid = iLL*4+2;
      cv[iSL]->cd(cid);
      SetPadMargin();
      hTDC[iLayer]->Draw();

      cid = iLL*4+3;
      cv[iSL]->cd(cid);
      SetPadMargin();
      gStyle->SetStatH(0.23);
      gStyle->SetStatW(0.26);
      hADCfit[iLayer]->SetAxisRange(0, RangeADC);
      hADCfit[iLayer]->Draw();
      hADCfit[iLayer]->GetFunction("ffinal")->Draw("same");

      cid = iLL*4+4;
      cv[iSL]->cd(cid);
      hADCTDC[iLayer]->Draw("col");
      iLayer++;
    }
  }

  cvSL = new TCanvas("cvSL", "Histograms per Super Layer", 800, 1000);
  cvSL->Divide(4, MAXSUPERLAYER);
  for(Int_t iSL=0; iSL<MAXSUPERLAYER; iSL++){
    Int_t cid = iSL*4+1;
    cvSL->cd(cid);
    hHitPatternSL[iSL]->Draw();
    cid = iSL*4+2;
    cvSL->cd(cid);
    hTDCSL[iSL]->Draw();
    cid = iSL*4+3;
    cvSL->cd(cid);
    hADCSL[iSL]->Draw();
    cid = iSL*4+4;
    cvSL->cd(cid);
    hADCTDCSL[iSL]->Draw("col");
  }
  

  cvsummary = new TCanvas("cvsummary", "Summary of Fitting", 700, 800);
  cvsummary->Divide(1,2);
  cvsummary->cd(1);
  hmeanADC->Draw("P");
  hmeanADC->SetStats(0);
  hmeanADC->SetMarkerStyle(20);
  Double_t ymax = hmeanADC->GetMaximum();
  Double_t ymin = 0;
  hmeanADC->SetMinimum(0);
  Double_t xborder;
  Int_t iborder = 0;
  for(Int_t iSL =0; iSL<MAXSUPERLAYER ; iSL++){
    iborder += MaxLocalLayer[iSL];
    xborder = iborder - 0.5;
    lborder_mean[iSL] = new TLine(xborder, ymin, xborder, ymax);
    lborder_mean[iSL]->Draw();
  }
  cvsummary->cd(2);
  hsigmaADC->Draw("P");
  hsigmaADC->SetStats(0);
  hsigmaADC->SetMarkerStyle(20);
  ymax = hsigmaADC->GetMaximum();
  hsigmaADC->SetMinimum(0);
  ymin = 0;
  iborder = 0;
  for(Int_t iSL =0; iSL<MAXSUPERLAYER ; iSL++){
    iborder += MaxLocalLayer[iSL];
    xborder = iborder - 0.5;
    lborder_sigma[iSL] = new TLine(xborder, ymin, xborder, ymax);
    lborder_sigma[iSL]->Draw();
  }

  return;
}


void PrintHisto()
{

  if(cv[0]==NULL){
    PlotHisto();
  }

  if(kDrawFitting){
    cv[0]->SaveAs(cOutDraw.c_str());
  } else {
    cv[0]->SaveAs(cOutDrawInit.c_str());
  }
  
  for(Int_t iSL=1; iSL < MAXSUPERLAYER; iSL++){
    cv[iSL]->SaveAs(cOutDraw.c_str());
  }

  cvSL->SaveAs(cOutDraw.c_str());

  cvsummary->SaveAs(cOutDrawFinal.c_str());

  return;
}


void WriteHisto()
{

  TFile *flOutRoot = new TFile(cOutRootAll.c_str(), "recreate");

  for(Int_t iLayer = 0; iLayer<MAXLAYER; iLayer++){
    hHitPattern[iLayer]->Write();
    hTDC[iLayer]->Write();
    hADC[iLayer]->Write();
    hADCfit[iLayer]->Write();
    hADCTDC[iLayer]->Write();
  }

  hmeanADC->Write();
  hsigmaADC->Write();

  flOutRoot->Close();


  TFile *flOutRootSL = new TFile(cOutRootSL.c_str(), "recreate");

  for(Int_t iSL = 0; iSL<MAXSUPERLAYER; iSL++){
    hHitPatternSL[iSL]->Write();
    hTDCSL[iSL]->Write();
    hADCSL[iSL]->Write();
    hADCTDCSL[iSL]->Write();
  }

  hmeanADC->Write();
  hsigmaADC->Write();

  flOutRootSL->Close();

  return;
}


Int_t CDCValidation()
{

  SetLayerID();
  FillHisto();
  //  PlotHisto();
  if(kDrawLayerHisto){
    PrintHisto();
  }
  if(kWriteHisto){
    WriteHisto();
  }

  exit(0);
}
