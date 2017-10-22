#include "TFile.h"
#include "TH1F.h"

#include <fstream>
#include <iostream>

void getCurvePars(){

  ifstream in;
  in.open("parameters.bgcurve.fit");

  int ncurvepars = 15;
  int nsigmapars = 12;
  int nhadronpars = 5;

  int num;
  double par;
  TFile* dedxpars = new TFile("DedxPars.root","RECREATE");
  TH1F* CDCDedxCurvePars = new TH1F("CDCDedxCurvePars","",ncurvepars+1,0,ncurvepars);
  TH1F* CDCDedxSigmaPars = new TH1F("CDCDedxSigmaPars","",nsigmapars+1,0,nsigmapars);
  TH1F* CDCDedxHadronCor = new TH1F("CDCDedxHadronCor","",nhadronpars+1,0,nhadronpars);

  while (1) {
    in >> num >> par;
    std::cout << num << "\t" << par << std::endl;
    if( !in.good() ) break;
    if( num == -1 ) CDCDedxCurvePars->SetBinContent(1,par);
    else if( num == -2 ) CDCDedxSigmaPars->SetBinContent(1,par);
    else if( num == -3 ) CDCDedxHadronCor->SetBinContent(1,par);
    else if( num <= ncurvepars ) CDCDedxCurvePars->SetBinContent(num+1,par);
    else if( num <= nsigmapars+ncurvepars ) CDCDedxSigmaPars->SetBinContent((num-ncurvepars+1),par);
    else CDCDedxHadronCor->SetBinContent((num-ncurvepars-nsigmapars+1),par);
  }
  in.close();

  CDCDedxCurvePars->Write();
  CDCDedxSigmaPars->Write();
  CDCDedxHadronCor->Write();
}
