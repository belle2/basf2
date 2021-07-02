/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/*
  <header>
  <input>EvtGenSimRec_dedx.root</input>
  <contact>jkumar@andrew.cmu.edu</contact>
  <description>check PID performance of each particle vs. all others</description>
  </header>
*/

#include <TFile.h>
#include <TMath.h>
#include <TTree.h>
#include <TROOT.h>
#include <TH1D.h>
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
  
  TFile *output_file = new TFile("dedx_LRplots.root", "RECREATE");
  output_file->cd();
  
  const int show_particles = 6;
  const int num_particles = 6;
  const int pdg_codes[] = { 11, 13, 211, 321, 2212, 1000010020 };
  const char* pdg_names[] = { "electrons", "muons", "pions", "kaons", "protons", "deuterons" };
  TString logl_strings[num_particles];
  for(int part = 0; part < show_particles; part++) {
    //for this particle, take its likelihood...
    logl_strings[part] = TString::Format("exp(CDCDedxLikelihoods.m_cdcLogl[][%i]) / (", part);
    
    //and divide by summed likelihood of all particles
    for(int i = 0; i < num_particles; i++) {
      if(i!=0)logl_strings[part] += " + ";
      logl_strings[part] += TString::Format("exp(CDCDedxLikelihoods.m_cdcLogl[][%i])", i);
    }
    logl_strings[part] += ") ";
    std::cout << pdg_names[part] << ":\n";
    std::cout <<"\t" << logl_strings[part] << "\n\n";
    
    //now create histograms with this (unweighted) probability
    tree->Project(TString::Format("%d_prob(110,0,1.10)", pdg_codes[part]), logl_strings[part].Data(),
		  TString::Format("abs(CDCDedxTracks.m_pdg) == %d", pdg_codes[part]));
    TH1D* hist = (TH1D*)output_file->Get(TString::Format("%d_prob", pdg_codes[part]));
    hist->SetTitle(TString::Format("CDC dE/dx: Unweighted PID for true %s; %s PID (CDC); entries", pdg_names[part], pdg_names[part]));
    hist->GetListOfFunctions()->Add(new TNamed("Description", hist->GetTitle()));
    if (pdg_codes[part] == 211) {
      hist->GetListOfFunctions()->Add(new TNamed("Check", "Peak at around middle (because of muons) and some at 1"));
    } else if (pdg_codes[part] == 13) {
      hist->GetListOfFunctions()->Add(new TNamed("Check", "Like pions, but too few entries for evaluation"));
    } else {
      hist->GetListOfFunctions()->Add(new TNamed("Check", "Peak towards 1"));
    }
    hist->GetListOfFunctions()->Add(new TNamed("Contact","Jitendra Kumar: jkumar@andrew.cmu.edu"));
    hist->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter,nostats,pvalue-warn=0.10,pvalue-error=0.005"));
    hist->SetMaximum(1.15*hist->GetMaximum());
    hist->Write();
    
  }
  output_file->Close();
}

void dedx2_LR()
{
  gROOT->SetStyle("Plain");
  plot("../EvtGenSimRec_dedx.root");
}
