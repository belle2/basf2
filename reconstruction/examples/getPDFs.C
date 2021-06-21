/*
  Example macro to draw PDFs histogram from payload
  Run Instruction: with basf2 environment
  root -l -b >> .L getPDFs.C++g >> getPDFs("test.root")
 */

#include <iostream>
#include <vector>
#include "TFile.h"
#include "TH2F.h"
#include "TStyle.h"
#include "TCanvas.h"
#include <include/reconstruction/dbobjects/DedxPDFs.h>

void getPDFs(std::string inputfile = "temp.root") {

    TFile* runfile = new TFile(Form("%s", inputfile.data()));
    if (!runfile) B2FATAL("Input payload file not found");

    Belle2::DedxPDFs* pdfs = (Belle2::DedxPDFs*)runfile->Get("DedxPDFs");
    std::vector<TH2F*> hDedxPDFs(6);

    std::array<std::string, 6> part = {"Electron", "Muon", "Pion", "Kaon", "Proton", "Deuteron"};
    std::array<std::string, 3> det = {"CDC", "PXD", "VXD"};

    TCanvas* can = new TCanvas("can", "blah-blah", 1200, 700);
    can->Divide(3, 2);
    gStyle->SetOptStat(11);

    for (const auto& idet : det) {
      for (bool trunmean : { false, true }) {
	std::stringstream check;
	check << std::boolalpha << trunmean;
	for (int iPart = 0; iPart < 6; iPart++) {
	  if (idet.compare("CDC") == 0)hDedxPDFs[iPart] = (TH2F*)pdfs->getCDCPDF(iPart, trunmean);
	  else if (idet.compare("SVD") == 0)hDedxPDFs[iPart] = (TH2F*)pdfs->getSVDPDF(iPart, trunmean);
	  else if (idet.compare("VXD") == 0)hDedxPDFs[iPart] = (TH2F*)pdfs->getPXDPDF(iPart, trunmean);
	  can->cd(iPart + 1);
	  hDedxPDFs[iPart]->SetTitle(Form("%s; p(GeV/c) of %s; dE/dx", hDedxPDFs[iPart]->GetTitle(), part[iPart].data()));
	  hDedxPDFs[iPart]->DrawCopy("colz");
	}
	can->SetTitle(Form("Likehood dist. of charged particles from %s, trunmean = %s", idet.data(), check.str().data()));
	can->SaveAs(Form("Plots_%sDedxPDFs_wTrucMean_%s.pdf", idet.data(), check.str().data()));
      }
    }
    delete can;
    
}
