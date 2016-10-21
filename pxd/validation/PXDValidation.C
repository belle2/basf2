//
//  PXDValidation2.C
//    ROOT macro for validation      ver. 0.1
//                     15 November 2013
//                     Peter Kodys (Charles Univ. Prague)
//                     peter.kodys@mff.cuni.cz
//
//    Usage: Batch mode
//           root -b PXDValidation.C+O
//                                  ~~ compilation for faster execution
//           Interactive mode
//           root [1] .L PXDValidation.C+O
//           root [2] PXDValidation()
//
//    Even in the interactive mode, PXDValidation2() exits from root
//    after its execution. If you do not want to exit from root and
//    reuse histograms independently, use FillHisto().
//
//  History of update
//  15 Nov 2013    ver. 0.1   Peter Kodys
//  21 Oct 2016    ver. 0.3   Actualization, P.Kodys
//
// -----------------------------------------------------
// Options:
// Set SensorMirroring flag: default = 0
//               1: sensor u&v count with respect of readout chips
//               0: sensor u&v count with respect to detector axis
// -----------------------------------------------------
// Simulation:
//  1: EDoposition 1D (Energy deposition per micron) [from SimHits]
//  2: EDepositionLost 2D (Energy lost vs energy deposition) [from TrueHits]
//  3: HitTypes 1D (Type of particle movement over sensor) [from TrueHits]
// -----------------------------------------------------
// Digitization:
//  4: SignalToNoise 1D 4x(pitch) (Pixel signal noise ratio for every pitch)
//          [from Digitizer]
// -----------------------------------------------------
// Clasterization:
//  5: ClusterChargeToEDeposition 2D (Cluster charge vs. deposited energy)
//          [from TrueHit and Clasterizer]
//  6: ClusterizerGQ parameter [from 5]
//      ????
//  7: ErrorDistribution 1D 32x ( (Cluster position - true position) / sigma cluster
//          for u, v, 4xpitch, 4xCS(1,2,3,>3) ) [from TrueHit and Clasterizer]
// -----------------------------------------------------
// Rest of plots (standards):
//  8: EtaDistribution 1D 8x (In pixel cluster position for 4x pitch, u, v)
//          [from TrueHit and Clasterizer]
//  9: ClusterCharge 1D (Cluster charge) [from Clasterizer]
// 10: Seed 1D (Seed) [from Clasterizer]
// 11: ClusterSizeUV 1D (Cluster size) [from Clasterizer]
// 12: ClusterSizeU 1D (Cluster size in u projection) [from Clasterizer]
// 13: ClusterSizeV 1D (Cluster size in v projection) [from Clasterizer]
// 14: Statistics file plots: on
// -----------------------------------------------------
/*
<header>
<input>PXDValidationTTreeOutput.root</input>
<input>PXDValidationTTreeSimHitOutput.root</input>
<input>PXDValidationTTreeDigitOutput.root</input>
<output>PXDValidation.root</output>
<contact>Peter Kodys, peter.kodys@mff.cuni.cz</contact>
</header>
*/
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TLine.h"

void PXDValidation()
{
// Options:
//  int SensorMirroring = 0;   // sensor u&v counting, default = 0
//               1: sensor u&v count with respect of readout chips
//               0: sensor u&v count with respect to detector axis
  float CloseSurf = 0.98;    // how cloase surface is acceptance of particle entry/exit
  float cmToMicron = 10000.0;// conversion constant for plots
  int nPixelTypes = 4;       // we have 4 pixel sizes in v
  int nPixSizeGroups = 4;    // we have 4 groups for pixel size analyzing: 1, 2, 3, more than 3

// TODO ************** Need to impreve or set much more sistematicaly!!!  *************
// PXD:
// gq = 0.5 nA/e
// Slope of amplfication curve: 70 nA/ADU (fine mode), 130 nA/ADU (coarse mode, default)
// eToADU = 130 (70) / gq e/ADU
// Full ADU range: 8 bits = 256
  float ADURange = 256;
  float ADUToElectrons = 260;
// TODO ************** Need to impreve or set much more sistematicaly!!!  *************

// -----------------------------------------------------
  // open the files with simulated and reconstructed EvtGen particles
  TFile* input = TFile::Open("PXDValidationTTreeOutput.root");
  TTree* tree = (TTree*) input->Get("tree");
  TH1F* h_consts = (TH1F*) input->Get("h_consts");
  TFile* inputSimHit = TFile::Open("PXDValidationTTreeSimHitOutput.root");
  TTree* treeSimHit = (TTree*) inputSimHit->Get("tree");
  TFile* inputDigit = TFile::Open("PXDValidationTTreeDigitOutput.root");
  TTree* treeDigit = (TTree*) inputDigit->Get("tree");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("PXDValidation.root", "recreate");

// -----------------------------------------------------
// Load constants:
  float DetThicknes = h_consts->GetBinContent(1);
  float DigitNoise = h_consts->GetBinContent(12);
//  int PixNoU = h_consts->GetBinContent(7);
//  float PixSizeU = h_consts->GetBinContent(2);
  int *PixNoV = new int[nPixelTypes];
  PixNoV[0] = h_consts->GetBinContent(8);
  PixNoV[1] = h_consts->GetBinContent(9);
  PixNoV[2] = h_consts->GetBinContent(10);
  PixNoV[3] = h_consts->GetBinContent(11);
  float *PixSizeV = new float[nPixelTypes];
  PixSizeV[0] = h_consts->GetBinContent(3);
  PixSizeV[1] = h_consts->GetBinContent(4);
  PixSizeV[2] = h_consts->GetBinContent(5);
  PixSizeV[3] = h_consts->GetBinContent(6);

// -----------------------------------------------------
// Precalculation:
  float *PixPosCorV = new float[nPixelTypes];
  PixPosCorV[0] = -0.5 * (PixNoV[0] * PixSizeV[0] + PixNoV[1] * PixSizeV[1]) / PixSizeV[0];
  PixPosCorV[1] = (-0.5 * (PixNoV[0] * PixSizeV[0] + PixNoV[1] * PixSizeV[1]) + PixNoV[0] * PixSizeV[0]) / PixSizeV[1];
  PixPosCorV[0] -= TMath::Floor(PixPosCorV[0]);
  PixPosCorV[1] -= TMath::Floor(PixPosCorV[1]);
  PixPosCorV[0] *= PixSizeV[0];
  PixPosCorV[1] *= PixSizeV[1];
  PixPosCorV[2] = -0.5 * (PixNoV[2] * PixSizeV[2] + PixNoV[3] * PixSizeV[3]) / PixSizeV[2];
  PixPosCorV[3] = (-0.5 * (PixNoV[2] * PixSizeV[2] + PixNoV[3] * PixSizeV[3]) + PixNoV[2] * PixSizeV[2]) / PixSizeV[3];
  PixPosCorV[2] -= TMath::Floor(PixPosCorV[2]);
  PixPosCorV[3] -= TMath::Floor(PixPosCorV[3]);
  PixPosCorV[2] *= PixSizeV[2];
  PixPosCorV[3] *= PixSizeV[3];

// plot parameters:
  // 1: EDoposition:
  int iplot1 = 200;  // binning of plot
  float lplot1 = 0.0;  // low edge of plot
  float hplot1 = 500.0;  // high edge of plot
  // 2: EDepositionLost 2D:
  int iplot2 = 300;  // binning of plot
  float lplot2 = 0.0;  // low edge of plot
  float hplot2 = 0.0003;  // high edge of plot
  // 4: SignalToNoise:
  int iplot4 = 200;  // binning of plot
  float lplot4 = 0.0/DigitNoise;  // low edge of plot
  float hplot4 = ADURange * ADUToElectrons / DigitNoise;  // high edge of plot
//  float hplot4 = 30000.0/DigitNoise;  // high edge of plot
  // 5: ClusterChargeToEDeposition 2D:
  int iplot5a = ADURange;  // binning of plot
  float lplot5a = 0.0;  // low edge of plot
  float hplot5a = ADURange * ADUToElectrons;  // high edge of plot
//  float hplot5a = 30000.0;  // high edge of plot
  // 9: ClusterCharge:
  // 10: Seed:
  int iplot5 = ADURange;  // binning of plot
  float lplot5 = 0.0;  // low edge of plot
  float hplot5 = ADURange;  // high edge of plot
  // 7: ErrorDistribution:
  int iplot7 = 200;  // binning of plot
  float lplot7 = -10.0;  // low edge of plot
  float hplot7 = 10.0;  // high edge of plot
  // 8: EtaDistribution:
  int iplot8 = 200;  // binning of plot
  float lplot8 = 0.0;  // low edge of plot
  float hplot8 = 1.0;  // high edge of plot
  // 11: ClusterSizeUV:
  // 12: ClusterSizeU:
  // 13: ClusterSizeV:
  int iplot11 = 10;  // binning of plot
  float lplot11 = 0.0;  // low edge of plot
  float hplot11 = 10.0;  // high edge of plot

// -----------------------------------------------------
// Simulation:
//  1: EDoposition 1D (Energy deposition per micron) [from SimHits]
//      OK: simhit_Length, simhit_EnergyDep
  TString title(Form("Validation: Simulated energy deposition per micron"));
  TString name(Form("hValidSimEnergyDepositPerMicron"));
  TH1F *hValidSimEnergyDepositMicron = new TH1F(name, title, iplot1, lplot1, hplot1);
  hValidSimEnergyDepositMicron->GetXaxis()->SetTitle("Electrons per micron");
  TString expr(Form("simhit_EnergyDep/(simhit_Length*1E4)>>%s",name.Data()));
  treeSimHit->Draw(expr);
  hValidSimEnergyDepositMicron->GetListOfFunctions()->Add(new TNamed("Description", title.Data()));
  hValidSimEnergyDepositMicron->GetListOfFunctions()->Add(new TNamed("Check", "Validation: Check maximum, should be 60."));
  hValidSimEnergyDepositMicron->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
  hValidSimEnergyDepositMicron->Write();

//  2: EDepositionLost 2D (Energy lost vs energy deposition) [from TrueHits]
//      OK: truehit_LossMomentum, truehit_charge
  TCanvas *cValidTrueDepositChargeTrueLostMomentum;
  cValidTrueDepositChargeTrueLostMomentum = new TCanvas("cValidTrueDepositChargeTrueLostMomentum","cValidTrueDepositChargeTrueLostMomentum",800,600);
  cValidTrueDepositChargeTrueLostMomentum->Draw();
  TH2F* hValidTrueDepositChargeTrueLostMomentum = new TH2F("hValidTrueDepositChargeTrueLostMomentum", "Validation: true deposit energy vs. true lost momentum, should be bit bellow 45 deg.", iplot2, lplot2, hplot2, iplot2, lplot2, hplot2);
  hValidTrueDepositChargeTrueLostMomentum->GetXaxis()->SetTitle("Lost momentum (eV)");
  hValidTrueDepositChargeTrueLostMomentum->GetYaxis()->SetTitle("True deposit energy (eV)");
  tree->Draw("truehit_charge:truehit_LossMomentum>>hValidTrueDepositChargeTrueLostMomentum","","COLZ");
  hValidTrueDepositChargeTrueLostMomentum->GetListOfFunctions()->Add(new TNamed("Description", "Validation: true deposit energy vs. true lost momentum."));
  hValidTrueDepositChargeTrueLostMomentum->GetListOfFunctions()->Add(new TNamed("Check", "Validation: should be bit bellow 45 deg."));
  hValidTrueDepositChargeTrueLostMomentum->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
  hValidTrueDepositChargeTrueLostMomentum->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));
  hValidTrueDepositChargeTrueLostMomentum->Write();
  TLine *line = new TLine(lplot2,lplot2,hplot2,hplot2);
  line->SetLineWidth(2);
  line->SetLineColor(kRed);
  line->Draw();
  cValidTrueDepositChargeTrueLostMomentum->Write();

//  3: HitTypes 1D (Type of particle movement over sensor) [from TrueHits]
//      OK: truehit_EntryW, truehit_ExitW
  float fEntryW;
  float fExitW;
  TH1F* hValidParticleMovementThru = new TH1F("hValidParticleMovementThru", "Validation: Statistics particle movement thru detector.", 7,0,7);
  hValidParticleMovementThru->GetXaxis()->SetTitle("");
  TBranch *EntryW = tree->GetBranch("truehit_EntryW");
  TBranch *ExitW = tree->GetBranch("truehit_ExitW");
  EntryW->SetAddress(&fEntryW);
  ExitW->SetAddress(&fExitW);
  EntryW->SetAutoDelete(kTRUE);
  ExitW->SetAutoDelete(kTRUE);
  Int_t nentries = (Int_t)tree->GetEntries();
  for (Int_t i=0; i<nentries; i++) {
    tree->GetEntry(i);
    if ((fabs(fEntryW) >  DetThicknes*CloseSurf/2.0) && (fabs(fExitW) >  DetThicknes*CloseSurf/2.0)) {
      if (fabs(fEntryW + fExitW) < (1.0-CloseSurf)*DetThicknes) {
        if ((fEntryW - fExitW) < 0.0)
          hValidParticleMovementThru->Fill(0);  // particle flying thru both sides (drection -)
        else
          hValidParticleMovementThru->Fill(1);  // particle flying thru both sides (drection +)
      }
      else {
        if (fEntryW < 0.0)
          hValidParticleMovementThru->Fill(4);  // particle flying thru the same side (drection -)
        else
          hValidParticleMovementThru->Fill(5);  // particle flying thru the same side (drection +)
      }
    }
    else if (fabs(fEntryW) >  DetThicknes*CloseSurf/2.0) {
      hValidParticleMovementThru->Fill(2);  // particle flying thru one side (die inside)
    }
    else if (fabs(fExitW) >  DetThicknes*CloseSurf/2.0) {
      hValidParticleMovementThru->Fill(3);  // particle flying thru other side (growth inside)
    }
    else {
      hValidParticleMovementThru->Fill(6);  // particle growth and die inside (should be 0)
    }
  }
  hValidParticleMovementThru->GetXaxis()->SetBinLabel(1,"particle flying thru both sides (drection -)");
  hValidParticleMovementThru->GetXaxis()->SetBinLabel(2,"particle flying thru both sides (drection +)");
  hValidParticleMovementThru->GetXaxis()->SetBinLabel(3,"particle flying thru one side (die inside)");
  hValidParticleMovementThru->GetXaxis()->SetBinLabel(4,"particle flying thru other side (growth inside)");
  hValidParticleMovementThru->GetXaxis()->SetBinLabel(5,"particle flying thru the same side (drection -)");
  hValidParticleMovementThru->GetXaxis()->SetBinLabel(6,"particle flying thru the same side (drection +)");
  hValidParticleMovementThru->GetXaxis()->SetBinLabel(7,"particle growth and die inside (should be 0)");
  hValidParticleMovementThru->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Statistics particle movement thru detector."));
  hValidParticleMovementThru->GetListOfFunctions()->Add(new TNamed("Check", "Validation: Check bin content, should first be highest, res of bins less 1 percent."));
  hValidParticleMovementThru->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
  hValidParticleMovementThru->Write();

// -----------------------------------------------------
// Digitization:
//  4: SignalToNoise 1D 4x(pitch) (Pixel signal noise ratio for every pitch)
//          [from Digitizer]
//      OK: digit_charge
  TH1F **hValidDigitSignalToNoise;
  hValidDigitSignalToNoise = new TH1F*[nPixelTypes];
  for (int ipixtype=0; ipixtype<nPixelTypes; ipixtype++) {
    TString title(Form("Validation: Reconstructed signal to noise in pixel, for pixel size in v: %i microns, layer %i, region %i",(int)(PixSizeV[ipixtype]*cmToMicron+0.1),(int)(ipixtype/2),ipixtype%2));
    TString name(Form("hValidDigitSignalToNoise_Layer%i_Region%i_PixSize%i",(int)(ipixtype/2),ipixtype%2,(int)(PixSizeV[ipixtype]*cmToMicron+0.1)));
    hValidDigitSignalToNoise[ipixtype] = new TH1F(name, title, iplot4, lplot4, hplot4);
    hValidDigitSignalToNoise[ipixtype]->GetXaxis()->SetTitle("S/N");
    TString expr(Form("digit_charge*%f/%f>>%s", ADUToElectrons, DigitNoise,name.Data()));
    TString cond(Form("pixel_type==%i",ipixtype));
    treeDigit->Draw(expr,cond);
    hValidDigitSignalToNoise[ipixtype]->GetListOfFunctions()->Add(new TNamed("Description", title.Data()));
    hValidDigitSignalToNoise[ipixtype]->GetListOfFunctions()->Add(new TNamed("Check", "Validation: Check shape, should have visible maxima in range 18 - 25."));
    hValidDigitSignalToNoise[ipixtype]->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
    hValidDigitSignalToNoise[ipixtype]->Write();
  }

// -----------------------------------------------------
// Clasterization:
//  5: ClusterChargeToEDeposition 2D (Cluster charge vs. deposited energy)
//          [from TrueHit and Clasterizer]
//      OK: cluster_charge, truehit_charge
  TCanvas *cValidTrueDepositChargeRecoDepositCharge;
  cValidTrueDepositChargeRecoDepositCharge = new TCanvas("cValidTrueDepositChargeRecoDepositCharge","cValidTrueDepositChargeRecoDepositCharge",800,600);
  cValidTrueDepositChargeRecoDepositCharge->Draw();
  TH2F* hValidTrueDepositChargeRecoDepositCharge = new TH2F("hValidTrueDepositChargeRecoDepositCharge", "Validation: true deposit energy vs. reconstructed deposit energy, should be bit bellow 45 deg.", iplot5a, lplot5a, hplot5a, iplot5a, lplot5a, hplot5a);
  hValidTrueDepositChargeRecoDepositCharge->GetXaxis()->SetTitle("True deposit energy (electrons)");
  hValidTrueDepositChargeRecoDepositCharge->GetYaxis()->SetTitle("Reconstructed deposit energy (electrons)");
  expr = Form("cluster_charge*%f:truehit_charge/3.65*1E9>>hValidTrueDepositChargeRecoDepositCharge", ADUToElectrons);
  tree->Draw(expr,"","COLZ");
//  tree->Draw("cluster_charge*260:truehit_charge/3.65*1E9>>hValidTrueDepositChargeRecoDepositCharge","","COLZ");
//  tree->Draw("cluster_charge:truehit_charge>>hValidTrueDepositChargeRecoDepositCharge","","COLZ");
  TLine *line2 = new TLine(lplot5a,lplot5a,hplot5a,hplot5a);
  line2->SetLineWidth(2);
  line2->SetLineColor(kRed);
  line2->Draw();
  hValidTrueDepositChargeRecoDepositCharge->GetListOfFunctions()->Add(new TNamed("Description", "Validation: true and reconstructed deposit energy."));
  hValidTrueDepositChargeRecoDepositCharge->GetListOfFunctions()->Add(new TNamed("Check", "Validation: should be bit bellow 45 deg."));
  hValidTrueDepositChargeRecoDepositCharge->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
  hValidTrueDepositChargeRecoDepositCharge->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));
  hValidTrueDepositChargeRecoDepositCharge->Write();
  cValidTrueDepositChargeRecoDepositCharge->Write();

//  6: ClusterizerGQ parameter [from 5]
//      ????
//  7: ErrorDistribution 1D 32x ( (Cluster position - true position) / sigma cluster
//          for u, v, 4xpitch, 4xCS(1,2,3,>3) ) [from TrueHit and Clasterizer]
//      OK: cluster_uPull, cluster_vPull
  TCanvas *cValidErrorDistrU;
  cValidErrorDistrU = new TCanvas("cValidErrorDistrU","cValidErrorDistrU",1500,1100);
  cValidErrorDistrU->Divide(nPixelTypes,nPixSizeGroups);
  cValidErrorDistrU->Draw();
  TCanvas *cValidErrorDistrV;
  cValidErrorDistrV = new TCanvas("cValidErrorDistrV","cValidErrorDistrV",1500,1100);
  cValidErrorDistrV->Divide(nPixelTypes,nPixSizeGroups);
  cValidErrorDistrV->Draw();
  TH1F **hValidErrorDistrU;
  TH1F **hValidErrorDistrV;
  hValidErrorDistrU = new TH1F*[nPixelTypes*nPixSizeGroups];
  hValidErrorDistrV = new TH1F*[nPixelTypes*nPixSizeGroups];
  TString *textclsize = new TString[nPixSizeGroups];
  textclsize[0] = "1";
  textclsize[1] = "2";
  textclsize[2] = "3";
  textclsize[3] = "Over3";
  for (int ipixtype=0; ipixtype<nPixelTypes; ipixtype++) {
    for (int iclsize=0; iclsize<nPixSizeGroups; iclsize++) {
      // u direction
      TString title(Form("Validation: Error distribution in u, for cluster size in u: %s, pixel size in v: %i microns, layer %i, region %i",textclsize[iclsize].Data(),(int)(PixSizeV[ipixtype]*cmToMicron+0.1),(int)(ipixtype/2),ipixtype%2));
      TString name(Form("hValidErrorDistrU_CS%s_Layer%i_Region%i_PixSize%i",textclsize[iclsize].Data(),(int)(ipixtype/2),ipixtype%2,(int)(PixSizeV[ipixtype]*cmToMicron+0.1)));
      hValidErrorDistrU[ipixtype*nPixSizeGroups+iclsize] = new TH1F(name, title, iplot7, lplot7, hplot7);
      hValidErrorDistrU[ipixtype*nPixSizeGroups+iclsize]->GetXaxis()->SetTitle("u pull");
      TString exprU1(Form("cluster_uPull>>%s",name.Data()));
      TString cond;
      if (iclsize < 3) {
        cond.Append(Form("pixel_type==%i&&cluster_uSize==%i",ipixtype,iclsize+1));
      }
      else {
        cond.Append(Form("pixel_type==%i&&cluster_uSize>2",ipixtype));
      }
      cValidErrorDistrU->cd(ipixtype*nPixSizeGroups+iclsize+1);
      tree->Draw(exprU1,cond);
      hValidErrorDistrU[ipixtype*nPixSizeGroups+iclsize]->GetListOfFunctions()->Add(new TNamed("Description", title.Data()));
      hValidErrorDistrU[ipixtype*nPixSizeGroups+iclsize]->GetListOfFunctions()->Add(new TNamed("Check", "Validation: Check RMS, should be less 2.0."));
      hValidErrorDistrU[ipixtype*nPixSizeGroups+iclsize]->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
      hValidErrorDistrU[ipixtype*nPixSizeGroups+iclsize]->Write();
      // v direction
      TString title2(Form("Validation: Error distribution in v, for cluster size in v: %s, pixel size in v: %i microns, layer %i, region %i",textclsize[iclsize].Data(),(int)(PixSizeV[ipixtype]*cmToMicron+0.1),(int)(ipixtype/2),ipixtype%2));
      TString name2(Form("hValidErrorDistrV_CS%s_Layer%i_Region%i_PixSize%i",textclsize[iclsize].Data(),(int)(ipixtype/2),ipixtype%2,(int)(PixSizeV[ipixtype]*cmToMicron+0.1)));
      hValidErrorDistrV[ipixtype*nPixSizeGroups+iclsize] = new TH1F(name2, title2, iplot7, lplot7, hplot7);
      hValidErrorDistrV[ipixtype*nPixSizeGroups+iclsize]->GetXaxis()->SetTitle("v pull");
      TString exprV1(Form("cluster_vPull>>%s",name2.Data()));
      TString cond2;
      if (iclsize < 3) {
        cond2.Append(Form("pixel_type==%i&&cluster_vSize==%i",ipixtype,iclsize+1));
      }
      else {
        cond2.Append(Form("pixel_type==%i&&cluster_vSize>2",ipixtype));
      }
      cValidErrorDistrV->cd(ipixtype*nPixSizeGroups+iclsize+1);
      tree->Draw(exprV1,cond2);
      hValidErrorDistrV[ipixtype*nPixSizeGroups+iclsize]->GetListOfFunctions()->Add(new TNamed("Description", title2.Data()));
      hValidErrorDistrV[ipixtype*nPixSizeGroups+iclsize]->GetListOfFunctions()->Add(new TNamed("Check", "Validation: Check RMS, should be less 2.0."));
      hValidErrorDistrV[ipixtype*nPixSizeGroups+iclsize]->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
      hValidErrorDistrV[ipixtype*nPixSizeGroups+iclsize]->Write();
    }
  }
  cValidErrorDistrU->Write();
  cValidErrorDistrV->Write();

// -----------------------------------------------------
// Rest of plots:
//  8: EtaDistribution 1D 8x (In pixel cluster position for 4x pitch, u, v)
//          [from TrueHit and Clasterizer]
//      OK: cluster_u/Pitch_u, cluster_v/Pitch_v
  TCanvas *cValidEtaDist;
  cValidEtaDist = new TCanvas("cValidEtaDist","cValidEtaDist",1500,1100);
  cValidEtaDist->Divide(nPixelTypes,2);
  cValidEtaDist->Draw();
  TH1F **hValidEtaDistU;
  TH1F **hValidEtaDistV;
  hValidEtaDistU = new TH1F*[nPixelTypes];
  hValidEtaDistV = new TH1F*[nPixelTypes];
  for (int ipixtype=0; ipixtype<nPixelTypes; ipixtype++) {
    // u direction
    TString title(Form("Validation: Reconstructed in-pixel eta distribution in u, for pixel size in v: %i microns, layer %i, region %i",(int)(PixSizeV[ipixtype]*cmToMicron+0.1),(int)(ipixtype/2),ipixtype%2));
    TString name(Form("hValidEtaDistU_Layer%i_Region%i_PixSize%i",(int)(ipixtype/2),ipixtype%2,(int)(PixSizeV[ipixtype]*cmToMicron+0.1)));
    hValidEtaDistU[ipixtype] = new TH1F(name, title, iplot8, lplot8, hplot8);
    hValidEtaDistU[ipixtype]->GetXaxis()->SetTitle("Reconstructed u in-pixel position");
//    TString expr(Form("(cluster_u+%i/2*%f)/%f-TMath::Floor((cluster_u+%i/2*%f)/%f)>>%s",
//        PixNoU,PixSizeU,PixSizeU,PixNoU,PixSizeU,PixSizeU,name.Data()));
    TString exprU1(Form("cluster_uEtaPosition>>%s",name.Data()));
    TString exprU2(Form("1-cluster_uEtaPosition>>+%s",name.Data()));
//    TString exprU1(Form("cluster_u/%f-TMath::Floor(cluster_u/%f)>>%s",
//        PixSizeU,PixSizeU,name.Data()));
//    TString exprU2(Form("1-(cluster_u/%f-TMath::Floor(cluster_u/%f))>>+%s",
//        PixSizeU,PixSizeU,name.Data()));
    TString cond(Form("pixel_type==%i",ipixtype));
    TString cond2(Form("pixel_type==%i&&sensor==1",ipixtype));
    TString cond3(Form("pixel_type==%i&&sensor==2",ipixtype));
    cValidEtaDist->cd(ipixtype*2+0+1);
//    if (SensorMirroring == 1) {
//      tree->Draw(exprU1,cond2);
//      tree->Draw(exprU2,cond3);
//    }
//    else {
    tree->Draw(exprU1,cond);
//    }
    hValidEtaDistU[ipixtype]->SetLineColor(kRed);
    hValidEtaDistU[ipixtype]->GetListOfFunctions()->Add(new TNamed("Description", title.Data()));
    hValidEtaDistU[ipixtype]->GetListOfFunctions()->Add(new TNamed("Check", "Validation: Check shape, should be peak in middle and rest of bins flat."));
    hValidEtaDistU[ipixtype]->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
    hValidEtaDistU[ipixtype]->Write();
    // v direction
    TString title2(Form("Validation: Reconstructed in-pixel eta distribution in v, for pixel size in v: %i microns, layer %i, region %i",(int)(PixSizeV[ipixtype]*cmToMicron+0.1),(int)(ipixtype/2),ipixtype%2));
    TString name2(Form("hValidEtaDistV_Layer%i_Region%i_PixSize%i",(int)(ipixtype/2),ipixtype%2,(int)(PixSizeV[ipixtype]*cmToMicron+0.1)));
    hValidEtaDistV[ipixtype] = new TH1F(name2, title2, iplot8, lplot8, hplot8);
    hValidEtaDistV[ipixtype]->GetXaxis()->SetTitle("Reconstructed v in-pixel position");
    TString exprV1(Form("cluster_vEtaPosition>>%s",name2.Data()));
    TString exprV2(Form("1-cluster_vEtaPosition>>+%s",name2.Data()));
//    TString exprV1(Form("(cluster_v+%f)/%f-TMath::Floor((cluster_v+%f)/%f)>>%s",
//        PixPosCorV[ipixtype],PixSizeV[ipixtype],PixPosCorV[ipixtype],PixSizeV[ipixtype],name2.Data()));
//    TString exprV2(Form("1-((cluster_v+%f)/%f-TMath::Floor((cluster_v+%f)/%f))>>+%s",
//        PixPosCorV[ipixtype],PixSizeV[ipixtype],PixPosCorV[ipixtype],PixSizeV[ipixtype],name2.Data()));
//    TString expr2(Form("(cluster_v+%i/2*%f)/%f-TMath::Floor((cluster_v+%i/2*%f)/%f)>>%s",
//        PixNoV[ipixtype],PixSizeV[ipixtype],PixSizeV[ipixtype],PixNoV[ipixtype],PixSizeV[ipixtype],PixSizeV[ipixtype],name2.Data()));
    cValidEtaDist->cd(ipixtype*2+1+1);
//    if (SensorMirroring == 1) {
//      tree->Draw(exprV1,cond2);
//      tree->Draw(exprV2,cond3);
//    }
//    else {
    tree->Draw(exprV1,cond);
//    }
    hValidEtaDistV[ipixtype]->GetListOfFunctions()->Add(new TNamed("Description", title2.Data()));
    hValidEtaDistV[ipixtype]->GetListOfFunctions()->Add(new TNamed("Check", "Validation: Check shape, should be peak in middle and rest of bins flat."));
    hValidEtaDistV[ipixtype]->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
    hValidEtaDistV[ipixtype]->Write();
  }
  cValidEtaDist->Write();

//  9: ClusterCharge 1D (Cluster charge) [from Clasterizer]
//      OK: cluster_charge
  TH1F* hValidRecoDepositEnergy = new TH1F("hValidRecoDepositEnergy", "Validation: reconstructed deposit energy, MPV should be in about 22.", iplot5, lplot5, hplot5);
  hValidRecoDepositEnergy->GetXaxis()->SetTitle("Reconstructed deposit energy (ADU)");
  tree->Draw("cluster_charge>>hValidRecoDepositEnergy");
  hValidRecoDepositEnergy->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Reconstructed Deposit Energy."));
  hValidRecoDepositEnergy->GetListOfFunctions()->Add(new TNamed("Check", "Validation: Check MPV of energy plot, should be in about 22."));
  hValidRecoDepositEnergy->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
  hValidRecoDepositEnergy->Write();

// 10: Seed 1D (Seed) [from Clasterizer]
//      OK: cluster_seed
  TH1F* hValidRecoSeed = new TH1F("hValidRecoSeed", "Validation: reconstructed seed energy, MPV should be in about 20.", iplot5, lplot5, hplot5);
  hValidRecoSeed->GetXaxis()->SetTitle("Reconstructed seed (ADU)");
  tree->Draw("cluster_seed>>hValidRecoSeed");
  hValidRecoSeed->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Reconstructed seed."));
  hValidRecoSeed->GetListOfFunctions()->Add(new TNamed("Check", "Validation: Check MPV of seed plot, should be in about 20."));
  hValidRecoSeed->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
  hValidRecoSeed->Write();

// 11: ClusterSizeUV 1D (Cluster size) [from Clasterizer]
//      OK: cluster_size
  TH1F* hClusterSizeUV = new TH1F("hClusterSizeUV", "Validation: Reconstructed full cluster size, maximum should be in 1.", iplot11, lplot11, hplot11);
  hClusterSizeUV->GetXaxis()->SetTitle("Cluster Size");
  tree->Draw("cluster_size>>hClusterSizeUV");
  hClusterSizeUV->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Reconstructed cluster size - sume of all pixels over threshold."));
  hClusterSizeUV->GetListOfFunctions()->Add(new TNamed("Check", "Validation: Check maximum of plot, should be in 1."));
  hClusterSizeUV->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
  hClusterSizeUV->Write();

// 12: ClusterSizeU 1D (Cluster size in u projection) [from Clasterizer]
//      OK: cluster_uSize
  TH1F* hClusterSizeU = new TH1F("hClusterSizeU", "Validation: Reconstructed cluster size in u, maximum should be in 1.", iplot11, lplot11, hplot11);
  hClusterSizeU->GetXaxis()->SetTitle("Cluster size u");
  tree->Draw("cluster_uSize>>hClusterSizeU");
  hClusterSizeU->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Reconstructed cluster size - sume of all pixels in u projection over threshold."));
  hClusterSizeU->GetListOfFunctions()->Add(new TNamed("Check", "Validation: Check maximum of plot, should be in 1."));
  hClusterSizeU->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
  hClusterSizeU->Write();

// 13: ClusterSizeV 1D (Cluster size in v projection) [from Clasterizer]
//      OK: cluster_vSize
  TH1F* hClusterSizeV = new TH1F("hClusterSizeV", "Validation: Reconstructed cluster size in v, maximum should be in 1.", iplot11, lplot11, hplot11);
  hClusterSizeV->GetXaxis()->SetTitle("Cluster size v");
  tree->Draw("cluster_vSize>>hClusterSizeV");
  hClusterSizeV->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Reconstructed cluster size - sume of all pixels in v projection over threshold."));
  hClusterSizeV->GetListOfFunctions()->Add(new TNamed("Check", "Validation: Check maximum of plot, should be in 1."));
  hClusterSizeV->GetListOfFunctions()->Add(new TNamed("Contact", "peter.kodys@mff.cuni.cz"));
  hClusterSizeV->Write();

// -----------------------------------------------------

  delete output;
}
