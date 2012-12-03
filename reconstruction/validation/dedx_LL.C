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
  gSystem->Load("libanalysis_dataobjects");

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

  TFile *output_file = new TFile("dedx_LRplots.root", "RECREATE");
  output_file->cd();

  const int show_particles = 5;
  const int num_particles = 5;
  const int pdg_codes[] = { 211, 321, 11, 13, 2212 };
  TString logl_strings[num_particles];
  for(int part = 0; part < show_particles; part++) {
    //for this particle, take its likelihood...
    logl_strings[part] = TString::Format("exp(DedxTracks.m_logl[][%i]) / (", part);

    //and divide by summed likelihood of all particles
    for(int i = 0; i < num_particles; i++) {
      if(i!=0)
        logl_strings[part] += " + ";
      logl_strings[part] += TString::Format("exp(DedxTracks.m_logl[][%i])", i);
    }
    logl_strings[part] += ") ";
    std::cout << logl_strings[part] << "\n";

    //now create histograms with this (unweighted) probability
    tree->Project(TString::Format("%d_prob", pdg_codes[part]), logl_strings[part].Data(),
        TString::Format("abs(m_pdg) == %d", pdg_codes[part]));
    TH1* hist = (TH1*)output_file->Get(TString::Format("%d_prob", pdg_codes[part]));
    hist->SetTitle(TString::Format("Unweighted output prob. for true %d", pdg_codes[part]));
    hist->Write();

  }
  output_file->Close();
}

void dedx_LL()
{
  gROOT->SetStyle("Plain");

  plot("EvtGenSimRec_dedx.root");
}
