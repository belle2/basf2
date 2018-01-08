/*
<header>
  <input>EvtGenSimRec_dedx.root</input>
  <contact>jvbennett@cmu.edu</contact>
  <description>Plot dE/dx over momentum</description>
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
  if(tree->GetEntries() == 0 || tree->GetBranch("CDCDedxTracks.m_p") == 0) {
    std::cerr << "Input file doesn't contain dE/dx data, aborting!\n";
    exit(1);
  }

  TFile *output_file = new TFile("dedx_curves.root", "RECREATE");
  output_file->cd();

  const int num_detectors = 3;
  const char* detectors[] = { "PXD", "SVD", "CDC" };
  const double dedx_cutoff[] = { 10e3, 5.e6, 10.0 };
  for(int idet = 1; idet < num_detectors; idet++) { //PXD not in input file, anyway
    if( idet == 1 )
      tree->Project(TString::Format("dedx_p_%d", idet), TString::Format("VXDDedxTracks.m_dedx_avg_truncated[][%d]:VXDDedxTracks.m_p", idet), TString::Format("VXDDedxTracks.m_p < 3 && VXDDedxTracks.m_dedx_avg_truncated[][%d] < %f ", idet, dedx_cutoff[idet]));
    else
      tree->Project(TString::Format("dedx_p_%d", idet), TString::Format("CDCDedxTracks.m_dedx_avg_truncated[][%d]:CDCDedxTracks.m_p_cdc", idet), TString::Format("CDCDedxTracks.m_p_cdc < 3 && CDCDedxTracks.m_dedx_avg_truncated[][%d] < %f ", idet, dedx_cutoff[idet]));
    TH1* hist = (TH1*)output_file->Get(TString::Format("dedx_p_%d", idet));
    hist->SetTitle(TString::Format("dE/dx curve for %s; p [GeV]; dE/dx", detectors[idet]));
    hist->GetListOfFunctions()->Add(new TNamed("Description", hist->GetTitle()));
    hist->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));
    if (idet == 2) { //CDC
      hist->GetListOfFunctions()->Add(new TNamed("Check", "Distinct bands for pions/kaons/protons below 1GeV, relativistic rise visible . Some misreconstructed tracks at very low dE/dx values."));
    } else { //PXD/SVD
      hist->GetListOfFunctions()->Add(new TNamed("Check", "Distinct bands for pions/kaons/protons below 1GeV, minimal ionisation for higher p. Some misreconstructed tracks at very low dE/dx values."));
    }
    hist->GetListOfFunctions()->Add(new TNamed("Contact","jvbennett@cmu.edu"));
    hist->Write();

  }
  output_file->Close();
}

void dedx1_curves()
{
  gROOT->SetStyle("Plain");

  plot("../EvtGenSimRec_dedx.root");
}
