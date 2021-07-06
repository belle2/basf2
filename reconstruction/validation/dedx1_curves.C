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
  <description>Plot dE/dx over momentum</description>
  </header>
*/

#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TH1D.h>
#include <TParticlePDG.h>
#include <TDatabasePDG.h>
#include <iostream>
#include <cstdlib>

TH1D *GetBandPlot(TFile *file, TTree *tree, TString det="temp", Int_t ipart=0){
  
  TString pname="";
  if(ipart!=0){
    const TParticlePDG *pdgname= TDatabasePDG::Instance()->GetParticle(ipart);
    pname =  pdgname->GetName();
  }else{
    pname =  "all charged";
  }
  
  TString name = TString::Format("dedx_p_%s_%d", det.Data(), ipart);
  TString hname = "", proj = "", cut = "";
  TString varY, varX;
  
  const double dedx_cutoff[] = { 10e3, 5.e6, 10.0 };
  const double p_cutoff[] = { 3.0, 3.0, 3.0 };
  
  if(det=="SVD"){
    varY = "VXDDedxTracks.m_dedxAvgTruncated[][1]";
    varX = "VXDDedxTracks.m_p";
    hname = TString::Format("%s(500,0.01,%0.01f,500,2.e2,%0.01f)",name.Data(), p_cutoff[1],dedx_cutoff[1]);
    proj = TString::Format("%s:%s", varY.Data(),varX.Data());
    if(ipart==0)cut = TString::Format("%s < %0.01f && %s < %0.01f", varX.Data(), p_cutoff[1], varY.Data(), dedx_cutoff[1]);
    else cut = TString::Format("%s < %0.01f && %s < %0.01f && abs(VXDDedxTracks.m_pdg) == %d ", varX.Data(), p_cutoff[1], varY.Data(), dedx_cutoff[1],ipart);
  }else if(det=="CDC"){
    varY = "CDCDedxTracks.m_dedxAvgTruncated";
    varX = "CDCDedxTracks.m_pCDC";
    hname = TString::Format("%s(500,0.01,%0.01f,500,0.3,%0.01f)",name.Data(), p_cutoff[2],dedx_cutoff[2]);
    proj = TString::Format("%s:%s", varY.Data(),varX.Data());
    if(ipart==0)cut = TString::Format("%s < %0.01f && %s < %0.01f", varX.Data(), p_cutoff[2], varY.Data(), dedx_cutoff[2]);
    else cut = TString::Format("%s < %0.01f && %s < %0.01f && abs(CDCDedxTracks.m_pdg) == %d ", varX.Data(), p_cutoff[2], varY.Data(), dedx_cutoff[2],ipart);
  }
  
  file->cd();
  tree->Project(TString::Format("%s", hname.Data()), TString::Format("%s", proj.Data()), TString::Format("%s", cut.Data()));
  
  TH1D* hist = (TH1D*)file->Get(TString::Format("%s", name.Data()));
  if(hist)std::cout << "histogram created as : " << hist->GetName() << std::endl;
  hist->SetTitle(TString::Format("dE/dx curve for %s (%s w/ CC); p [GeV/c]; %s dE/dx", det.Data(), pname.Data(), det.Data()));
  hist->GetListOfFunctions()->Add(new TNamed("Description", hist->GetTitle()));
  hist->GetListOfFunctions()->Add(new TNamed("Contact","Jitendra Kumar: jkumar@andrew.cmu.edu"));
  if(ipart==0){
    hist->GetListOfFunctions()->Add(new TNamed("Check", "Distinct bands for pions/kaons/protons below 1 GeV. Some misreconstructed tracks at very low dE/dx values"));
    hist->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter,nostats,colz"));
  }else {
    hist->GetListOfFunctions()->Add(new TNamed("Check", "Individual particle band plot. Report if band spread is too-wide wrt reference"));
    hist->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter"));
  }
  return hist;
}

//--------------
void plot(const TString &input_filename)
{
  gSystem->Load("libreconstruction_dataobjects");
  
  TFile *f = new TFile(input_filename,"READ");
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
  
  if(tree->GetBranch("CDCDedxTracks.m_pCDC") == 0 || tree->GetBranch("CDCDedxTracks.m_dedxAvgTruncated") == 0) {
    std::cerr << "Input file doesn't contain CDC dE/dx data, aborting!\n";
    exit(1);
  }
  
  if(tree->GetBranch("VXDDedxTracks.m_p") == 0 || tree->GetBranch("VXDDedxTracks.m_dedxAvgTruncated[2]") == 0) {
    std::cerr << "Input file doesn't contain SVD dE/dx data, aborting!\n";
    exit(1);
  }
  
  int count=-1;
  TH1D *hSVD[7], *hCDC[7];
  TFile *output_file = new TFile("dedx_curves.root", "RECREATE");
  for ( auto pdg : { 0,11,13,211,321,2212,1000010020}) {
    count++;
    //prepare CDC/SVD plots
    hSVD[count] = (TH1D*)GetBandPlot(f,tree,"SVD",pdg); //0 for all
    hCDC[count] = (TH1D*)GetBandPlot(f,tree,"CDC",pdg); //0 for all
    //add SVD plots to file
    Color_t color = count+1;
    if(count==4) color = kGreen+3;
    if(count==6) color = kBlack;
    hSVD[count]->SetMarkerColor(color);
    hCDC[count]->SetMarkerColor(color);
    output_file->cd();
    hSVD[count]->Write();
  }
  
  //add CDC plots now
  for (int i=0; i<7; i++) {
    output_file->cd();
    hCDC[i]->Write();
  }
  
  output_file->Close();
}

void dedx1_curves()
{
  gROOT->SetStyle("Plain");
  plot("../EvtGenSimRec_dedx.root");
}
