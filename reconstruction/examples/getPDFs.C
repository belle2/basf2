#include <iostream>
#include <vector>
#include <cmath>

#include "TFile.h"
#include "TH1F.h"
#include "TGraph.h"
#include "TCanvas.h"

#include <include/reconstruction/dbobjects/DedxPDFs.h>

void getPDFs(){

  TFile* runfile = new TFile("/cvmfs/belle.cern.ch/conditions/dbstore_DedxPDFs_rev_1.root");
  Belle2::DedxPDFs* pdfs = (Belle2::DedxPDFs*)runfile->Get("DedxPDFs");
  TH2F cdcpi = pdfs->getCDCPDF(2);  TH2F cdck = pdfs->getCDCPDF(3);
  TCanvas* can = new TCanvas("can","",1200,600);
  can->Divide(2,1);
  can->cd(1);
  cdcpi.DrawCopy("colz");
  can->cd(2);
  cdck.SetMaximum(0.05);
  cdck.DrawCopy("colz");
  can->SaveAs("cdc.pdf");
}
