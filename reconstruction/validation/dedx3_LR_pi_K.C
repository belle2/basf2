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

  TFile *output_file = new TFile("dedx_LRplots_pi_K.root", "RECREATE");
  output_file->cd();

  const int show_particles = 2;
  const int num_particles = 5;
  const int pdg_codes[] = { 211, 321, 2212, 11, 13};
  TString logl_strings[num_particles];
  for(int part = 0; part < show_particles; part++) {
    //now create histograms with this (unweighted) probability
    tree->Project(TString::Format("%d_LR", pdg_codes[part]), "(DedxTracks.m_logl[][0] - DedxTracks.m_logl[][1]):m_p_true",
        TString::Format("abs(DedxTracks.m_pdg) == %d", pdg_codes[part]));
    TH1* hist = (TH1*)output_file->Get(TString::Format("%d_LR", pdg_codes[part]));
    hist->SetTitle(TString::Format("LL(pi) - LL(K) for true %d", pdg_codes[part]));
    hist->Write();

  }
  output_file->Close();
}

void dedx3_LR_pi_K()
{
  gROOT->SetStyle("Plain");

  plot("EvtGenSimRec_dedx.root");
}
