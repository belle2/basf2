//////////////////////////////////////////////////////////
// 4 - print payloads
//
// Print the payloads included in the calibration_results.
//
// Usage: root calib4_printPayloads.C
//
// Input: calibration_results
// Output: none
//
// Contributors: Jake Bennett
//////////////////////////////////////////////////////////

#include <iostream>
#include <vector>
#include <cmath>

#include "TFile.h"
#include "TH1F.h"
#include "TGraph.h"
#include "TCanvas.h"

#include <include/reconstruction/dbobjects/CDCDedxScaleFactor.h>
#include <include/reconstruction/dbobjects/CDCDedxMomentumCor.h>
#include <include/reconstruction/dbobjects/CDCDedxRunGain.h>
#include <include/reconstruction/dbobjects/CDCDedxWireGain.h>
#include <include/reconstruction/dbobjects/CDCDedxCosineCor.h>
#include <include/reconstruction/dbobjects/CDCDedx2DCor.h>
#include <include/reconstruction/dbobjects/CDCDedx1DCleanup.h>
#include <include/reconstruction/dbobjects/CDCDedxHadronCor.h>
#include <include/reconstruction/dbobjects/CDCDedxCurvePars.h>
#include <include/reconstruction/dbobjects/CDCDedxSigmaPars.h>

void calib4_printScaleFactor( TString dir ){
  std::cout << "Scale factor corrections:" << std::endl;  
  TFile* scalefile = new TFile(dir+"/dbstore_CDCDedxScaleFactor_rev_1.root");
  Belle2::CDCDedxScaleFactor* sf = (Belle2::CDCDedxScaleFactor*)scalefile->Get("CDCDedxScaleFactor");
  std::cout << sf->getScaleFactor() << std::endl;
}

void calib4_printMomCor( TString dir ){
  std::cout << "Momentum corrections:" << std::endl;
  TFile* momfile = new TFile(dir+"/dbstore_CDCDedxMomentumCor_rev_1.root");
  Belle2::CDCDedxMomentumCor* mc = (Belle2::CDCDedxMomentumCor*)momfile->Get("CDCDedxMomentumCor");
  std::vector<double> momcor = mc->getMomCor();
  const int nmbins = mc->getNBins();
  TH1F* hmc = new TH1F("hmc",";p;Correction",nmbins,0,10);
  for (unsigned int i = 0; i < nmbins; ++i) {
    hmc->SetBinContent(i+1,momcor[i]);
  }
  TCanvas* canmc = new TCanvas("canmc","",600,600);
  canmc->cd();
  hmc->SetTitle(";p;Correction");
  hmc->SetMinimum(0.);
  hmc->SetMaximum(2.0);
  hmc->SetStats(0);
  hmc->Draw();
  canmc->SaveAs("momentumCorrection.eps");
  canmc->SaveAs("momentumCorrection.gif");
}

void calib4_printRunGain( TString dir ){
  std::cout << "Run gain corrections:" << std::endl;  
  TFile* runfile = new TFile(dir+"/dbstore_CDCDedxRunGain_rev_1.root");
  Belle2::CDCDedxRunGain* rg = (Belle2::CDCDedxRunGain*)runfile->Get("CDCDedxRunGain");
  std::cout << rg->getRunGain() << std::endl;
  
}

void calib4_printWireGain( TString dir ){
  TH1F* hwg = new TH1F("hwg",";Wire Number (continuous);Wire Gains",14336,-0.5,14335.5);
  std::cout << "Wire gain corrections:" << std::endl;  
  TFile* wirefile = new TFile(dir+"/dbstore_CDCDedxWireGain_rev_1.root");
  Belle2::CDCDedxWireGain* wg = (Belle2::CDCDedxWireGain*)wirefile->Get("CDCDedxWireGain");
  for( int i = 0; i < 14336; ++i ){
    hwg->SetBinContent(i+1,wg->getWireGain(i));
  }
  std::cout << std::endl;
  TCanvas* can2 = new TCanvas("can2","",600,600);
  can2->cd();
  hwg->Draw();
  can2->SaveAs("wireGains.eps");
  can2->SaveAs("wireGains.gif");
}

void calib4_printCosGain( TString dir ){
  std::cout << "Cosine corrections:" << std::endl;
  TFile* cosfile = new TFile(dir+"/dbstore_CDCDedxCosineCor_rev_1.root");
  Belle2::CDCDedxCosineCor* cc = (Belle2::CDCDedxCosineCor*)cosfile->Get("CDCDedxCosineCor");
  const int ncbins = cc->getNBins();
  std::cout << "cosine: " << ncbins << std::endl;
  TH1F* hcg = new TH1F("hcg",";cos(#theta);Correction",ncbins,-1,1);
  for (unsigned int i = 0; i < ncbins; ++i) {
    double costh = -1.0+(i+0.5)*2.0/ncbins;
    std::cout << costh << "\t" << cc->getMean(costh) << std::endl;
    hcg->SetBinContent(i+1,cc->getMean(costh));
  }
  TCanvas* can3 = new TCanvas("can3","",600,600);
  can3->cd();
  hcg->SetTitle(";cos(#theta);dE/dx");
  hcg->SetMinimum(0.6);
  hcg->SetMaximum(1.1);
  hcg->SetStats(0);
  hcg->Draw();
  can3->SaveAs("cosineCorrection.eps");
  can3->SaveAs("cosineCorrection.gif");
}

void calib4_print2DCor( TString dir ){
  std::cout << "2D corrections:" << std::endl;
  TFile* twodfile = new TFile(dir+"/dbstore_CDCDedx2DCor_rev_1.root");
  Belle2::CDCDedx2DCor* twod = (Belle2::CDCDedx2DCor*)twodfile->Get("CDCDedx2DCor");
  const TH2F* h2d = twod->getHist();
  TCanvas* can4 = new TCanvas("can4","",600,600);
  can4->cd();
  h2d->DrawCopy("colz");
  can4->SaveAs("TwoDCorrection.eps");
  can4->SaveAs("TwoDCorrection.gif");
}

void calib4_print1DCleanup( TString dir ){
  std::cout << "1D cleanup:" << std::endl;
  TFile* onedfile = new TFile(dir+"/dbstore_CDCDedx1DCleanup_rev_1.root");
  Belle2::CDCDedx1DCleanup* oned = (Belle2::CDCDedx1DCleanup*)onedfile->Get("CDCDedx1DCleanup");
  const int n1dbins = oned->getNBins();
  TH1F* h1d = new TH1F("h1d",";entrance angle;correction",n1dbins,0,n1dbins);
  std::cout << "1D: " << n1dbins << std::endl;
  for (unsigned int i = 0; i < n1dbins; ++i){
    double enta = std::asin(-1.0+(i+0.5)*(2.0/n1dbins));
    h1d->SetBinContent(i+1,oned->getMean(0,enta));
    std::cout << enta << "\t" << oned->getMean(0,enta) << std::endl;
  }
  TCanvas* can4b = new TCanvas("can4b","",600,600);
  can4b->cd();
  h1d->DrawCopy();
  can4b->SaveAs("OneDCorrection.eps");
  can4b->SaveAs("OneDCorrection.gif");
}

void calib4_printOther( TString dir ){
  // also peak at the other parameters (locally for now)
  /*
  TFile* scalefile = new TFile("localdb/dbstore_CDCDedxScaleFactor_rev_1.root");
  Belle2::CDCDedxScaleFactor* sf = (Belle2::CDCDedxScaleFactor*)scalefile->Get("CDCDedxScaleFactor");
  std::cout << "Scale factor: " << std::endl;
  std::cout << sf->getScaleFactor() << std::endl;

  TFile* hadronfile = new TFile("localdb/dbstore_CDCDedxHadronCor_rev_1.root");
  Belle2::CDCDedxHadronCor* hc = (Belle2::CDCDedxHadronCor*)hadronfile->Get("CDCDedxHadronCor");
  std::cout << "Hadron parameters:" << std::endl;
  std::cout << "Version: " << hc->getVersion() << std::endl;
  for( int i = 0; i < hc->getSize(); ++i ){
    std::cout << i << "\t" << hc->getHadronPar(i) << std::endl;
  }

  TFile* curvefile = new TFile("localdb/dbstore_CDCDedxCurvePars_rev_1.root");
  Belle2::CDCDedxCurvePars* cp = (Belle2::CDCDedxCurvePars*)curvefile->Get("CDCDedxCurvePars");
  std::cout << "Curve parameters:" << std::endl;
  std::cout << "Version: " << cp->getVersion() << std::endl;
  for( int i = 0; i < cp->getSize(); ++i ){
    std::cout << i << "\t" << cp->getCurvePar(i) << std::endl;
  }

  TFile* sigmafile = new TFile("localdb/dbstore_CDCDedxSigmaPars_rev_1.root");
  Belle2::CDCDedxSigmaPars* sp = (Belle2::CDCDedxSigmaPars*)sigmafile->Get("CDCDedxSigmaPars");
  std::cout << "Sigma parameters:" << std::endl;
  std::cout << "Version: " << sp->getVersion() << std::endl;
  for( int i = 0; i < sp->getSize(); ++i ){
    std::cout << i << "\t" << sp->getSigmaPar(i) << std::endl;
  }
  */
}


void calib4_printPayloads(){

  TString dir("localdb");  
  //TString dir("calibration_results/CDCDedxCalibrations/outputdb");

  //  calib4_printScaleFactor(dir);
  calib4_printMomCor(dir);
  calib4_printCosGain(dir);  
  calib4_printRunGain(dir);
  calib4_printWireGain(dir);
  calib4_print2DCor(dir);
  calib4_print1DCleanup(dir);
}
