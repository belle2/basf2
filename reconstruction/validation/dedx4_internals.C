/*
<header>
  <input>EvtGenSimRec_dedx.root</input>
  <contact>jvbennett@cmu.edu</contact>
  <description>Check some internals of the module</description>
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
  if(tree->GetEntries() == 0 || tree->GetBranch("CDCDedxTracks.m_p") == 0) {
    std::cerr << "Input file doesn't contain dE/dx data, aborting!\n";
    exit(1);
  }

  TFile *output_file = new TFile("dedx_internals.root", "RECREATE");
  output_file->cd();

  tree->Project("layer_ID(70,-10,60)", "CDCDedxTracks.m_lLayer", "");
  tree->Project("vxd_layer_ID(70,-10,60)", "VXDDedxTracks.dedxLayer", "");

  TH1* hist = (TH1*)output_file->Get("layer_ID");
  TH1* vxdhist = (TH1*)output_file->Get("vxd_layer_ID");
  hist->Add(vxdhist);

  hist->SetTitle("layer ID for each dE/dx measurement");
  hist->GetListOfFunctions()->Add(new TNamed("Description", "layer ID for each dE/dx measurement (negative values for PXD/SVD)"));
  hist->GetListOfFunctions()->Add(new TNamed("Check", "Smooth distribution for values > 0, VXD measurements (< 0) at around the same level (in particular, should not be twice as high). Gap at -1, -2 indicates PXD is not being used (=default)."));
  hist->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));
  hist->Write();

  output_file->Close();
}

void dedx4_internals()
{
  gROOT->SetStyle("Plain");

  plot("../EvtGenSimRec_dedx.root");
}
