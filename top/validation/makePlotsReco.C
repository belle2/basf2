/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/*
<header>
  <input>TOPNtupleRecoTest.root</input>
  <output>efficiency.root</output>
  <contact>marko.staric@ijs.si</contact>
  <description>Makes validation histograms</description>
</header>
*/

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCut.h>

void getEfficiency(TTree* top, TCut forWhat, TH1F* result);


void makePlotsReco() 
{
  // open the file with input data (flat ntuple)

  TFile* input = TFile::Open("../TOPNtupleRecoTest.root");
  TTree* top = (TTree*) input->Get("top");

  // open the output file for the validation histograms

  TFile* output = TFile::Open("efficiency.root", "recreate");

  TH1F* kaonEffi = new TH1F("h001", "Kaon efficiency at 3 GeV/c", 10, -0.50, 0.85);
  getEfficiency(top, "abs(PDG) == 321", kaonEffi);

  TH1F* pionEffi = new TH1F("h002", "Pion fake rate at 3 GeV/c", 10, -0.50, 0.85);
  getEfficiency(top, "abs(PDG) == 211", pionEffi);

  kaonEffi->GetListOfFunctions()->Add(new TNamed("Description",
    "Kaon efficiency at 3 GeV/c versus cosine of track polar angle"));
  kaonEffi->GetListOfFunctions()->Add(new TNamed("Check",
    "Efficiency should follow its reference."));
  kaonEffi->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter"));

  pionEffi->GetListOfFunctions()->Add(new TNamed("Description",
    "Pion fake rate at 3 GeV/c versus cosine of track polar angle"));
  pionEffi->GetListOfFunctions()->Add(new TNamed("Check",
    "Fake rate should follow its reference."));
  pionEffi->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter"));
  
  kaonEffi->Write();
  pionEffi->Write();

  output->Close();
  input->Close();
  
}


void getEfficiency(TTree* top, TCut forWhat, TH1F* result) 
{
  TH1F* h0 = new TH1F(*result);
  h0->SetName("h0");
  top->Draw("cth >> h0", forWhat);
  h0->Sumw2();
  
  TH1F* h1 = new TH1F(*result);
  h1->SetName("h1");
  top->Draw("cth >> h1", forWhat && TCut("logL.K>logL.pi"));
  h1->Sumw2();

  result->Divide(h1, h0, 1, 1, "B");
  result->GetXaxis()->SetTitle("cos #theta");
  result->GetYaxis()->SetTitle("efficiency");
  result->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));

  delete h0;
  delete h1;
}

