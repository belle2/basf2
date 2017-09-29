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

#include "TFile.h"
#include "TH1F.h"
#include "TGraph.h"
#include "TCanvas.h"

#include <include/reconstruction/dbobjects/CDCDedxRunGain.h>
#include <include/reconstruction/dbobjects/CDCDedxWireGain.h>
#include <include/reconstruction/dbobjects/CDCDedxCosine.h>

void calib4_printPayloads(){
  std::cout << "Run gain corrections:" << std::endl;  
  TFile* runfile = new TFile("calibration_results/CDCDedxRunGainCalibration/outputdb/dbstore_CDCDedxRunGain_rev_1.root");
  Belle2::CDCDedxRunGain* rg = (Belle2::CDCDedxRunGain*)runfile->Get("CDCDedxRunGain");
  std::cout << rg->getRunGain() << std::endl;
  double runs[1];
  runs[0] = 3118;
  double rungains[1];
  rungains[0] = rg->getRunGain();  
  TGraph* grg = new TGraph(1,runs,rungains);
  grg->SetTitle("");
  grg->GetXaxis()->SetTitle("Run");
  grg->GetYaxis()->SetTitle("Run Gain");
  TCanvas* can1 = new TCanvas("can1","",600,600);
  can1->cd();
  grg->Draw();
  
  TH1F* hwg = new TH1F("hwg",";Wire Number (continuous);Wire Gains",14336,-0.5,14335.5);
  std::cout << "Wire gain corrections:" << std::endl;  
  TFile* wirefile = new TFile("calibration_results/CDCDedxWireGainCalibration/outputdb/dbstore_CDCDedxWireGain_rev_1.root");
  Belle2::CDCDedxWireGain* wg = (Belle2::CDCDedxWireGain*)wirefile->Get("CDCDedxWireGain");
  for( int i = 0; i < 14336; ++i ){
    hwg->SetBinContent(i,wg->getWireGain(i));
  }
  std::cout << std::endl;
  TCanvas* can2 = new TCanvas("can2","",600,600);
  can2->cd();
  hwg->Draw();

  TH1F* hcg = new TH1F("hcg",";cos(#theta);Correction",40,-1,1);
  std::cout << "Cosine corrections:" << std::endl;
  TFile* cosfile = new TFile("calibration_results/CDCDedxCosineCalibration/outputdb/dbstore_CDCDedxCosine_rev_1.root");
  Belle2::CDCDedxCosine* cc = (Belle2::CDCDedxCosine*)cosfile->Get("CDCDedxCosine");
  std::vector<double> m_cosbinedges = cc->getCosThetaBins();
  for (unsigned int i = 0; i < m_cosbinedges.size(); ++i) {
    std::cout << m_cosbinedges[i] << "\t" << cc->getMean(m_cosbinedges[i]) << std::endl;
    hcg->SetBinContent(i+1,cc->getMean(m_cosbinedges[i]));
  }
  TCanvas* can3 = new TCanvas("can3","",600,600);
  can3->cd();
  hcg->Draw();

  // also peak at curve parameters (locally for now)
  
  TFile* curvefile = new TFile("localdb/dbstore_CDCDedxCurvePars_rev_1.root");
  Belle2::CDCDedxCurvePars* cp = (Belle2::CDCDedxCurvePars*)curvefile->Get("CDCDedxCurvePars");
  std::cout << "Curve parameters:" << std::endl;
  for( int i = 0; i < cp->getSize(); ++i ){
    std::cout << cp->getCurvePar(i) << std::endl;
  }
  std::cout << cp->getMean(1.5) << std::endl;

  TFile* sigmafile = new TFile("localdb/dbstore_CDCDedxSigmaPars_rev_1.root");
  Belle2::CDCDedxSigmaPars* sp = (Belle2::CDCDedxSigmaPars*)sigmafile->Get("CDCDedxSigmaPars");
  std::cout << "Sigma parameters:" << std::endl;
  for( int i = 0; i < sp->getSize(); ++i ){
    std::cout << sp->getSigmaPar(i) << std::endl;
  }
  std::cout << sp->getSigma(1.0,10,0.0) << std::endl;
}
