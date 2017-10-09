#include "TFile.h"
#include "TH1F.h"

#include <fstream>
#include <iostream>

void getCurvePars(){

  ifstream in;
  in.open("parameters.bgcurve.fit");

  int ncurvepars = 15;
  int nsigmapars = 12;

  int num;
  double par;
  TFile* dedxpars = new TFile("DedxPars.root","RECREATE");
  TH1F* CDCDedxCurvePars = new TH1F("CDCDedxCurvePars","",ncurvepars+1,0,ncurvepars);
  TH1F* CDCDedxSigmaPars = new TH1F("CDCDedxSigmaPars","",nsigmapars+1,0,nsigmapars);

  while (1) {
    in >> num >> par;
    std::cout << num << "\t" << par << std::endl;
    if( !in.good() ) break;
    if( num == -1 ) CDCDedxCurvePars->SetBinContent(1,par);
    else if( num == -2 ) CDCDedxSigmaPars->SetBinContent(1,par);
    else if( num <= ncurvepars ) CDCDedxCurvePars->SetBinContent(num+1,par);
    else CDCDedxSigmaPars->SetBinContent((num-ncurvepars+1),par);
  }
  in.close();

  CDCDedxCurvePars->Write();
  CDCDedxSigmaPars->Write();
}
