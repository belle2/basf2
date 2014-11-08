/*
<header>
  <input>EvtGenSimRec_dedx.root</input>
  <contact>christian.pulvermacher@kit.edu</contact>
  <description>check log-likelihood difference pi vs. K</description>
</header>
*/

#include <TFile.h>
#include <TMath.h>
#include <TTree.h>
#include <TF1.h>
#include <TH2F.h>
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

  TFile *output_file = new TFile("dedx_LLdiff_pi_K.root", "RECREATE");
  output_file->cd();

  const int show_particles = 2;
  const int num_particles = 5;
  const int pdg_codes[] = { 211, 321, 2212, 11, 13};
  const char* pdg_names[] = { "pions", "kaons", "protons", "electrons", "muons" };
  for (int det = 0; det < 2; det++) {
    TString selection;
    if (det == 0) //SVD
      selection = TString::Format("(DedxTracks.m_svdLogl[][0] - DedxTracks.m_svdLogl[][1]):m_p_true");
    else //CDC
      selection = TString::Format("(DedxTracks.m_cdcLogl[][0] - DedxTracks.m_cdcLogl[][1]):m_p_true");

    for (int part = 0; part < show_particles; part++) {
      //now create histograms with log-likelihood difference
      tree->Project(TString::Format("%d_%d_LLdiff", det, pdg_codes[part]), selection,
          TString::Format("abs(DedxTracks.m_pdg) == %d", pdg_codes[part]));
      TH1* hist = (TH1*)output_file->Get(TString::Format("%d_%d_LLdiff", det, pdg_codes[part]));
      hist->SetTitle(TString::Format("LL(pi) - LL(K) for true %s, over momentum (in %s)", pdg_names[part], (det==0)?"VXD":"CDC"));
      hist->GetListOfFunctions()->Add(new TNamed("Description", hist->GetTitle()));
      if (pdg_codes[part] == 211) {
        hist->GetListOfFunctions()->Add(new TNamed("Check", "Should be as high as possible (esp. for low momenta), with almost no entries <0 "));
      } else {
        hist->GetListOfFunctions()->Add(new TNamed("Check", "Should be as low as possible (esp. for low momenta), with almost no entries >0 "));
      }
      hist->Write();

    }
  }
  output_file->Close();
}

void dedx3_lldiff_pi_K()
{
  gROOT->SetStyle("Plain");

  plot("EvtGenSimRec_dedx.root");
}
