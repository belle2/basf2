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
  <description>check log-likelihood difference pi vs. K</description>
  </header>
*/

#include <TFile.h>
#include <TMath.h>
#include <TTree.h>
#include <TROOT.h>
#include <TH1D.h>
#include <TSystem.h>
#include <TParticlePDG.h>
#include <TDatabasePDG.h>
#include <iostream>

TH1D *GetLLDiffPlot(TFile *file, TTree *tree, TString det="temp", Int_t ipart=0){
  
  int idet = 0;
  TString hname = "", proj = "", cut = "";
  TString name, varY, varX;
  
  const TParticlePDG *pdgname= TDatabasePDG::Instance()->GetParticle(ipart);
  TString pname =  pdgname->GetName();
  
  const double LL_cutoff[] = { 20, 25, 300 };
  const double p_cutoff[] = { 3.0, 3.0, 3.0 };
  
  if(det=="SVD"){
    idet = 0;
    name = TString::Format("%d_%d_LLdiff", idet, ipart);
    varX="abs(VXDDedxTracks.m_p)";
    varY="(VXDDedxLikelihoods.m_vxdLogl[][2] - VXDDedxLikelihoods.m_vxdLogl[][3])";
    proj = TString::Format("%s:%s", varY.Data(),varX.Data());
    if(ipart==321)hname = TString::Format("%s(500,0.01,%0.01f,500,-%0.01f,50)",name.Data(), p_cutoff[1],LL_cutoff[1]);
    else hname = TString::Format("%s(500,0.01,%0.01f,500,-50,%0.01f)",name.Data(), p_cutoff[1],LL_cutoff[1]);
    cut = TString::Format("%s < %0.01f && abs(%s) < %0.01f && abs(VXDDedxTracks.m_pdg) == %d ", varX.Data(), p_cutoff[1], varY.Data(), LL_cutoff[1], ipart);
  }else if(det=="CDC"){
    idet = 1;
    name = TString::Format("%d_%d_LLdiff", idet, ipart);
    varX="abs(CDCDedxTracks.m_pCDC)";
    varY="(CDCDedxLikelihoods.m_cdcLogl[][2] - CDCDedxLikelihoods.m_cdcLogl[][3])";
    proj = TString::Format("%s:%s", varY.Data(),varX.Data());
    if(ipart==321)hname = TString::Format("%s(500,0.01,%0.01f,500,-%0.01f,50)",name.Data(), p_cutoff[2],LL_cutoff[2]);
    else hname = TString::Format("%s(500,0.01,%0.01f,500,-50,%0.01f)",name.Data(), p_cutoff[2],LL_cutoff[2]);
    cut = TString::Format("%s < %0.01f && abs(%s) < %0.01f && abs(CDCDedxTracks.m_pdg) == %d ", varX.Data(), p_cutoff[2], varY.Data(), LL_cutoff[2], ipart);
  }
  
  file->cd();
  tree->Project(TString::Format("%s", hname.Data()), TString::Format("%s", proj.Data()), TString::Format("%s", cut.Data()));
  
  TH1D* hist = (TH1D*)file->Get(TString::Format("%s", name.Data()));
  if(hist)std::cout << "creating histogram for: " << hist->GetName() << std::endl;
  
  hist->SetTitle(TString::Format("LL(pi) - LL(K) for true %s w/ CC (in %s);LL_{#pi}-LL_{K} diff (%s);entries", pname.Data(), det.Data(), (idet==0)?"VXD":"CDC") );
  hist->GetListOfFunctions()->Add(new TNamed("Description", hist->GetTitle()));
  if (ipart == 211) {
    hist->GetListOfFunctions()->Add(new TNamed("Check", "Should be as high as possible (esp. for low momenta), with almost no entries <0 "));
  } else {
    hist->GetListOfFunctions()->Add(new TNamed("Check", "Should be as low as possible (esp. for low momenta), with almost no entries > 0 "));
  }
  hist->GetListOfFunctions()->Add(new TNamed("Contact","Jitendra Kumar: jkumar@andrew.cmu.edu"));
  hist->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter,nostats"));
  return hist;
}


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
  
  if(tree->GetEntries() == 0) {
    std::cerr << "Input file doesn't contain dE/dx data, aborting!\n";
    exit(1);
  }
  
  if(tree->GetBranch("CDCDedxTracks.m_pCDC") == 0 || tree->GetBranch("CDCDedxLikelihoods.m_cdcLogl[6]") == 0) {
    std::cerr << "Input file doesn't contain CDC dE/dx data, aborting!\n";
    exit(1);
  }
  
  if(tree->GetBranch("VXDDedxTracks.m_p") == 0 || tree->GetBranch("VXDDedxLikelihoods.m_vxdLogl[6]") == 0) {
    std::cerr << "Input file doesn't contain VXD dE/dx data, aborting!\n";
    exit(1);
  }

  int count=-1;
  TH1D *hSVD[2], *hCDC[2];
  TFile *output_file = new TFile("dedx_LLdiff_pi_K.root", "RECREATE");
  for ( auto pdg : {211,321}) {
    count++;
    //prepare CDC/SVD plots
    hSVD[count] = (TH1D*)GetLLDiffPlot(f,tree,"SVD",pdg); //0 for all
    hCDC[count] = (TH1D*)GetLLDiffPlot(f,tree,"CDC",pdg); //0 for all
    //add SVD plots
    hSVD[count]->SetMarkerColor(kRed);
    hCDC[count]->SetMarkerColor(kBlue);
    output_file->cd();
    hSVD[count]->Write();
  }
  
  //add CDC plots
  for (int i=0; i<2; i++) {
    output_file->cd();
    hCDC[i]->Write();
  }
  
  output_file->Close();
}

void dedx3_lldiff_pi_K()
{
  gROOT->SetStyle("Plain");
  
  plot("../EvtGenSimRec_dedx.root");
}
