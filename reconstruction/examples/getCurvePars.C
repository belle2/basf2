#include "TFile.h"
#include "TH1F.h"

#include <fstream>
#include <iostream>

void getCurvePars(){

  ifstream in;
  in.open("parameters.bgcurve.fit");

  int nmeanpars = 15;
  int nsigmapars = 12;
  int nhadronpars = 5;

  int num;
  double par;
  TFile* dedxpars = new TFile("DedxPars.root","RECREATE");
  TH1F* CDCDedxMeanPars = new TH1F("CDCDedxMeanPars","",nmeanpars+1,0,nmeanpars);
  TH1F* CDCDedxSigmaPars = new TH1F("CDCDedxSigmaPars","",nsigmapars+1,0,nsigmapars);
  TH1F* CDCDedxHadronCor = new TH1F("CDCDedxHadronCor","",nhadronpars+1,0,nhadronpars);

  while (1) {
    in >> num >> par;
    std::cout << num << "\t" << par << std::endl;
    if( num == -1 ) CDCDedxMeanPars->SetBinContent(1,par);
    else if( num == -2 ) CDCDedxSigmaPars->SetBinContent(1,par);
    else if( num == -3 ) CDCDedxHadronCor->SetBinContent(1,par);
    else if( num <= nmeanpars ) CDCDedxMeanPars->SetBinContent(num+1,par);
    else if( num <= nsigmapars+nmeanpars ) CDCDedxSigmaPars->SetBinContent((num-nmeanpars+1),par);
    else CDCDedxHadronCor->SetBinContent((num-nmeanpars-nsigmapars+1),par);
    if( !in.good() ) break;
  }
  in.close();

  CDCDedxMeanPars->Write();
  CDCDedxSigmaPars->Write();
  CDCDedxHadronCor->Write();
}
