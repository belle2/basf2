/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//  CDCvalidation021.c 
//    ROOT macro for validation      ver. 0.21
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
//  20 Feb 2015    ver.0.22   Common Start mode -> Common Stop mode

/*
<header>
<input>CDCOutputGen.root</input>
<output>CDCValidation.root</output>
<contact>Hiroki Kanda, kanda@m.tains.tohoku.ac.jp</contact>
</header>
*/

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
Int_t NbinTDC = 256;
//Double_t MinTDC = 7690;//0;   common start mode -> common stop
//Double_t MaxTDC = 8192;//502; common start mode -> common stop
Double_t MinTDC = 4488;
Double_t MaxTDC = 5000;

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
Int_t Nbin2TDC = 255;                 // number of bins for TDC (vertical) axis
//Double_t Min2TDC = 7692;//0;          // minimum of TDC axis
//Double_t Max2TDC = 8192;//500;        // maximum of TDC axis
Double_t Min2TDC = 4490;
Double_t Max2TDC = 5000;


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
void WriteHisto();        // Save histograms to a root file
Int_t CDCValidation();    // Main macro for plotting, printing and saving


void SetLayerID()
{
  Int_t iLayer  = 0;
  Int_t MaxLL, WireID;
  Int_t iSL, iLL;

  //  printf("Initializing layer identifiers\n");
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
      //      printf("LayerID: %02d  Super Layer: %02d   Local Layer: %02d  wireID: %02d\n", 
      //iLayer, iSL, iLL, WireID);
      iLayer++;
    }
  }
  
  LayerReady=1;

  //  printf("\n\n");


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

  TCut CutLayer =Form("int(CDCHits.m_eWire&0xfe00)==%d", EncodedLayer[iLayer]);

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

    tree->Draw(Form("int(CDCHits.m_eWire&0x01ff)>>%s",chName),CutLayer);
    //    hHitPattern[iLayer]->GetListOfFunctions()->Add(new TNamed("Description", chTitle));
  } else {
    printf("Hit pattern for Layer %d is already filled\n", iLayer);
  }


}


void FillTDC(Int_t iLayer)
{

  if(LayerReady == 0){
    SetLayerID();
  }


  TCut CutLayer =Form("int(CDCHits.m_eWire&0xfe00)==%d", EncodedLayer[iLayer]);

  if(hTDC[iLayer] == NULL){
    Char_t chName[100], chTitle[100];
    
    sprintf(chName, "hTDC%02d", iLayer);
    sprintf(chTitle,"TDC (Layer %02d:  SuperLayer %2d  LocalLayer %2d",
	    iLayer, SuperLayer[iLayer], LocalLayer[iLayer]);

    hTDC[iLayer] = new TH1D(chName, chTitle, NbinTDC, MinTDC, MaxTDC);
    hTDC[iLayer]->GetXaxis()->SetTitle("TDC count");
    hTDC[iLayer]->GetYaxis()->SetTitle("Entries");


    tree->Draw(Form("CDCHits.m_tdcCount>>%s",chName),CutLayer);

  } else {
    printf("TDC histogram for Layer %d is already filled\n", iLayer);
  }


}

void FillADC(Int_t iLayer)
{
  if(LayerReady == 0){
    SetLayerID();
  }


  TCut CutLayer =Form("int(CDCHits.m_eWire&0xfe00)==%d", EncodedLayer[iLayer]);
  if(hADC[iLayer] == NULL){
    Char_t chName[100], chNameFit[100], chTitle[100];
    
    sprintf(chName, "hADC%02d", iLayer);
    sprintf(chNameFit, "hADC%02dfit", iLayer);
    sprintf(chTitle,"ADC (Layer %02d:  SuperLayer %2d  LocalLayer %2d)",
	    iLayer, SuperLayer[iLayer], LocalLayer[iLayer]);
  
    hADC[iLayer] = new TH1D(chName, chTitle, NbinADC, MinADC, MaxADC);
    hADC[iLayer]->GetXaxis()->SetTitle("ADC count");
    hADC[iLayer]->GetYaxis()->SetTitle("Entries");


    tree->Draw(Form("CDCHits.m_adcCount>>%s",chName),CutLayer);
    hADCfit[iLayer] = (TH1D*) hADC[iLayer]->Clone();
    hADCfit[iLayer]->SetName(chNameFit);
  } else {
    printf("ADC histogram for Layer %d is already filled\n", iLayer);
  }


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

  if(hADCfit[iLayer]->GetEntries() ==0){
    printf("No entries for Layer %d\n", iLayer);
    return 0;
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


}

void FillADCTDC(Int_t iLayer)
{

  if(LayerReady == 0){
    SetLayerID();
  }


  TCut CutLayer =Form("int(CDCHits.m_eWire&0xfe00)==%d", EncodedLayer[iLayer]);
  if(hADCTDC[iLayer] == NULL){

    Char_t chName[100], chTitle[100];
    sprintf(chName, "hADCTDC%02d", iLayer);
    sprintf(chTitle,"ADC-TDC (Layer %02d:  SuperLayer %2d  LocalLayer %2d",
	    iLayer, SuperLayer[iLayer], LocalLayer[iLayer]);

    hADCTDC[iLayer] = new TH2D(chName, chTitle, Nbin2ADC, Min2ADC, Max2ADC, Nbin2TDC, Min2TDC, Max2TDC);
    hADCTDC[iLayer]->GetXaxis()->SetTitle("ADC count");
    hADCTDC[iLayer]->GetYaxis()->SetTitle("TDC count");
    hADCTDC[iLayer]->SetStats(0);
    tree->Draw(Form("CDCHits.m_tdcCount:CDCHits.m_adcCount>>%s",chName),CutLayer);

  } else {
    printf("ADC vs. TDC 2D histogram for Layer %d is already filled\n", iLayer);  
  }


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

    // CDC hit pattern.

    sprintf(Name, "hHitPatternSL%02d", iSL);
    sprintf(Title, "Hit Pattern (Super Layer %2d)", iSL);
    hHitPatternSL[iSL] = (TH1D*) hHitPattern[iLayer]->Clone();
    hHitPatternSL[iSL]->SetName(Name);

    hHitPatternSL[iSL]->GetListOfFunctions()->Add(new TNamed("Description", Title));
    hHitPatternSL[iSL]->GetListOfFunctions()->Add(new TNamed("Check", "Flat distribution within statistics."));


    hHitPatternSL[iSL]->SetAxisRange(0, RangeADC);
    hHitPatternSL[iSL]->GetXaxis()->SetTitle("Cell");
    hHitPatternSL[iSL]->GetYaxis()->SetTitle("Entries");

    // ADC dist.

    sprintf(Name, "hADCSL%02d", iSL);
    sprintf(Title, "ADC (Super Layer %2d)", iSL);
    hADCSL[iSL] = (TH1D*) hADC[iLayer]->Clone();
    hADCSL[iSL]->SetName(Name);
    hADCSL[iSL]->GetListOfFunctions()->Add(new TNamed("Description", Title));
    if(iSL==0){
      hADCSL[iSL]->GetListOfFunctions()->Add(new TNamed("Check", "Landau peak is around 50 ADC count."));
    } else {
      hADCSL[iSL]->GetListOfFunctions()->Add(new TNamed("Check", "Landau peak is around 100 ADC count."));
    }

    hADCSL[iSL]->SetAxisRange(0, RangeADC);
    hADCSL[iSL]->GetXaxis()->SetTitle("ADC count");
    hADCSL[iSL]->GetYaxis()->SetTitle("Entries");

    // TDC dist.

    sprintf(Name, "hTDCSL%02d", iSL);
    sprintf(Title, "TDC (Super Layer %2d)", iSL);
    hTDCSL[iSL] = (TH1D*) hTDC[iLayer]->Clone();
    hTDCSL[iSL]->SetName(Name);
    hTDCSL[iSL]->GetXaxis()->SetTitle("TDC count");
    hTDCSL[iSL]->GetYaxis()->SetTitle("Entries");
    hTDCSL[iSL]->GetListOfFunctions()->Add(new TNamed("Description", Title));
    if(iSL==0){
      hTDCSL[iSL]->GetListOfFunctions()->Add(new TNamed("Check", "Width of TDC count is around 100 nsec."));
    } else {
      hTDCSL[iSL]->GetListOfFunctions()->Add(new TNamed("Check", "Width of TDC count is around 200 nsec."));
    }


    // ADC-TDC corr.

    sprintf(Name, "hADCTDCSL%02d", iSL);
    sprintf(Title, "ADC-TDC (Super Layer %2d)", iSL);
    hADCTDCSL[iSL] = (TH2D*) hADCTDC[iLayer]->Clone();
    hADCTDCSL[iSL]->SetName(Name);

    hADCTDCSL[iSL]->GetListOfFunctions()->Add(new TNamed("Description", Title));
    if(iSL<2){
      hADCTDCSL[iSL]->GetListOfFunctions()->Add(new TNamed("Check", "No correlation within a range of 0 to 100-120 nsec. TDC beyond the range correlated with smaller ADC count."));
    } else {
      hADCTDCSL[iSL]->GetListOfFunctions()->Add(new TNamed("Check", "No correlation within a range of 0 to 150-200 nsec. TDC beyond the range correlated with smaller ADC count."));
    }

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

  // Mean and Sigma of ADC dist.
  hmeanADC = new TH1D("hmeanADC", "Mean of Landau peak", MAXLAYER, -0.5, MAXLAYER-0.5);
  hsigmaADC =new TH1D("hsigmaADC", "Sigma of Landau peak", MAXLAYER, -0.5, MAXLAYER-0.5);
  hmeanADC->GetListOfFunctions()->Add(new TNamed("Description", "Mean of Landau peak")); 
  hmeanADC->GetListOfFunctions()->Add(new TNamed("Check", "Small cell part (inner 8 layers) : 50 ADC count. Normal cell part (layer8 - 55) : 100-140 ADC count.")); 
  hsigmaADC->GetListOfFunctions()->Add(new TNamed("Description", "Sigma of Landau peak")); 
  hsigmaADC->GetListOfFunctions()->Add(new TNamed("Check", "Small cell part (inner 8 layers) : ~16 ADC count. Normal cell part (layer8 - 55) : 20 - 30 ADC count.")); 
  for(iLayer = 0; iLayer < MAXLAYER; iLayer++){
    hmeanADC->Fill(Layer[iLayer], meanADC[iLayer]);
    hsigmaADC->Fill(Layer[iLayer], sigmaADC[iLayer]);
  }

  hmeanADC->GetXaxis()->SetTitle("Layer");
  hmeanADC->GetYaxis()->SetTitle("Mean (ADC count)");

  hsigmaADC->GetXaxis()->SetTitle("Layer");
  hsigmaADC->GetYaxis()->SetTitle("Sigma (ADC count)");


}



void WriteHisto()
{


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
  delete flOutRootSL;
}


Int_t CDCValidation()
{
  SetLayerID();
  FillHisto();
  if(kWriteHisto){
    WriteHisto();
  }
  return 0;
}
