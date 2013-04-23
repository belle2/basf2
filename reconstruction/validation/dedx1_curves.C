#include <TFile.h>
#include <TMath.h>
#include <TTree.h>
#include <TF1.h>
#include <TH2F.h>
#include <TProfile.h>
#include <TApplication.h>
#include <TROOT.h>
#include <TSystem.h>

#include <iostream>
#include <cstdlib>


void plot(const TString &input_filename)
{
  gSystem->Load("libreconstruction_dataobjects");

  TFile *f = new TFile(input_filename, "READ");
  if(!f) {
    std::cerr << "Couldn't read file!\n";
    exit(1);
  }
  TTree *tree = (TTree*)f->Get("tree");
  if(!tree) {
    std::cerr << "Couldn't find 'tree'!\n";
    exit(1);
  }
  if(tree->GetEntries() == 0 || tree->GetBranch("DedxTracks.m_p") == 0) {
    std::cerr << "Input file doesn't contain dE/dx data, aborting!\n";
    exit(1);
  }

  TFile *output_file = new TFile("dedx_curves.root", "RECREATE");
  output_file->cd();

  const int num_detectors = 3;
  const double dedx_cutoff[] = { 2.5e7, 0.003, 150.0 };
  for(int idet = 1; idet < num_detectors; idet++) { //PXD not in input file, anyway
    tree->Project(TString::Format("dedx_p_%d", idet), TString::Format("DedxTracks.m_dedx_avg_truncated[][%d]:m_p", idet),
        TString::Format("DedxTracks.m_p < 3 && DedxTracks.m_dedx_avg_truncated[][%d] < %f ", idet, dedx_cutoff[idet]));
    TH1* hist = (TH1*)output_file->Get(TString::Format("dedx_p_%d", idet));
    hist->SetTitle(TString::Format("dE/dx curve for detector %d; p [GeV]; dE/dx", idet));
    hist->Write();

  }
  output_file->Close();
}

void dedx1_curves()
{
  gROOT->SetStyle("Plain");

  plot("EvtGenSimRec_dedx.root");
}
