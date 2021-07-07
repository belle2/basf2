/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/*
<header>
  <input>TOPNtuple.root</input>
  <output>kaonID.root, numberOfPhotons.root, trackResolutions.root</output>
  <contact>marko.staric@ijs.si</contact>
  <description>Makes validation histograms</description>
</header>
*/

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TNtuple.h>
#include <TCut.h>

// basic event selection requirement
TCut evt_select = "primary == 1 && barHit.moduleID == extHit.moduleID && pValue > 0.001";

void kaonID(TTree* top);
void numberOfPhotons(TTree* top);
void trackResolutions(TTree* top);

void makePlots() 
{
  // open the file with input data (flat ntuple)

  TFile* input = TFile::Open("../TOPNtuple.root");
  TTree* top = (TTree*) input->Get("top");

  kaonID(top);
  numberOfPhotons(top);
  trackResolutions(top);

  input->Close();
  
}


double getEfficiency(TTree* top, TCut forWhat) 
{
  TH1F* hh = new TH1F("hh", "", 2, 0, 2);
  top->Draw("logL.K>logL.pi >> hh", evt_select && forWhat);
  double efficiency = int(hh->GetBinContent(2)/hh->GetEntries()*1000+0.5)/10.0;
  delete hh;
  return efficiency;
}

void kaonID(TTree* top)
{
  // open the output file for the validation histograms
  TFile* output = TFile::Open("kaonID.root", "recreate");

  // efficiency 

  double efficiencyLow = getEfficiency(top, "abs(PDG) == 321 && p < 2");
  double fakeLow = getEfficiency(top, "abs(PDG) == 211 && p < 2");
  double efficiencyHigh = getEfficiency(top, "abs(PDG) == 321 && p > 2");
  double fakeHigh = getEfficiency(top, "abs(PDG) == 211 && p > 2");
  
  TNtuple* nt = new TNtuple("efficiency", 
			    "PID efficiency in % for log L_{K} > log L_{#pi}",
			    "kaons_Low:pions_Low:kaons_High:pions_High");
  nt->Fill(efficiencyLow, fakeLow, efficiencyHigh, fakeHigh);
  nt->SetAlias("Description", 
    "Kaon ID: efficiency in % for log L_K > log L_pi, "
    "for low (p < 2 GeV/c) and high (p > 2 GeV/c) momentum regions.");
  nt->SetAlias("Check", "Several % difference to the reference may signal a problem.");
  nt->SetAlias("Contact", "marko.staric@ijs.si");
  nt->Write();

  // log likelihood differences

  TH1* h = 0;

  h = new TH1F("h101", "log likelihood difference for pions (p < 2 GeV/c)", 
		      100, -200, 200);
  h->GetXaxis()->SetTitle("log L_{K} - log L_{#pi}");
  h->GetYaxis()->SetTitle("entries/bin");
  top->Draw("logL.K-logL.pi >> h101", evt_select && TCut("abs(PDG) == 211 && p < 2"));
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Difference between kaon and pion log likelihoods for true pions below 2 GeV/c"));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
      "Most of the entries should be distributed at negative values."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter"));
  h->Write();

  h = new TH1F("h102", "log likelihood difference for kaons (p < 2 GeV/c)", 
		      100, -200, 200);
  h->GetXaxis()->SetTitle("log L_{K} - log L_{#pi}");
  h->GetYaxis()->SetTitle("entries/bin");
  top->Draw("logL.K-logL.pi >> h102", evt_select && TCut("abs(PDG) == 321 && p < 2"));
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Difference between kaon and pion log likelihoods for true kaons below 2 GeV/c"));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
      "Most of the entries should be distributed at positive values."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter"));
  h->Write();

  h = new TH1F("h103", "log likelihood difference for pions (p > 2 GeV/c)", 
		      100, -100, 100);
  h->GetXaxis()->SetTitle("log L_{K} - log L_{#pi}");
  h->GetYaxis()->SetTitle("entries/bin");
  top->Draw("logL.K-logL.pi >> h103", evt_select && TCut("abs(PDG) == 211 && p > 2"));
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Difference between kaon and pion log likelihoods for true pions above 2 GeV/c"));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
      "Most of the entries should be distributed at negative values."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter"));
  h->Write();

  h = new TH1F("h104", "log likelihood difference for kaons (p > 2 GeV/c)", 
		      100, -100, 100);
  h->GetXaxis()->SetTitle("log L_{K} - log L_{#pi}");
  h->GetYaxis()->SetTitle("entries/bin");
  top->Draw("logL.K-logL.pi >> h104", evt_select && TCut("abs(PDG) == 321 && p > 2"));
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Difference between kaon and pion log likelihoods for true kaons above 2 GeV/c"));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
      "Most of the entries should be distributed at positive values."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter"));
  h->Write();

  h = new TH2F("h105", "log likelihood difference vs. momentum for pions", 
		      100, 0, 4, 100, -200, 200);
  h->GetXaxis()->SetTitle("p [GeV/c]");
  h->GetYaxis()->SetTitle("log L_{K} - log L_{#pi}");
  top->Draw("logL.K-logL.pi:p >> h105", evt_select && TCut("abs(PDG) == 211"));
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Difference between kaon and pion log likelihoods versus momentum for true pions."));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
    "Most of the entries should be distributed below zero."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->Write();

  h = new TH2F("h106", "log likelihood difference vs. momentum for kaons", 
		      100, 0, 4, 100, -200, 200);
  h->GetXaxis()->SetTitle("p [GeV/c]");
  h->GetYaxis()->SetTitle("log L_{K} - log L_{#pi}");
  top->Draw("logL.K-logL.pi:p >> h106", evt_select && TCut("abs(PDG) == 321"));
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Difference between kaon and pion log likelihoods versus momentum for true kaons."));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
    "Most of the entries should be distributed above zero."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->Write();

  output->Close();
}


void numberOfPhotons(TTree* top)
{
  
  // open the output file for the validation histograms
  TFile* output = TFile::Open("numberOfPhotons.root", "recreate");

  TH1* h = 0;

  h = new TH1F("h201", "number of detected photons per track", 
		      100, 0, 200);
  h->GetXaxis()->SetTitle("number of photons");
  h->GetYaxis()->SetTitle("entries/bin");
  top->Draw("numPhot >> h201", evt_select);
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Number of detected photons per track."));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
    "Distribution should peak at around 20 photons; bin at zero must be fairly small; "
    "long tail is normal."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->GetListOfFunctions()->Add(new TNamed("MetaOptions", "shifter,kolmogorov"));
  h->Write();

  h = new TH2F("h202", "number of detected photons per track vs polar angle", 
	       100, -0.6, 0.9, 100, 0, 100);
  h->GetXaxis()->SetTitle("cos #theta");
  h->GetYaxis()->SetTitle("number of photons");
  top->Draw("numPhot:cth >> h202", evt_select);
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Number of detected photons per track versus polar angle."));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
    ""));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->Write();

  output->Close();
}


void trackResolutions(TTree* top)
{

  // open the output file for the validation histograms
  TFile* output = TFile::Open("trackResolutions.root", "recreate");

  TCut trackSelect;
  TH1* h = 0;

  // resolutions below 2 GeV/c

  trackSelect = evt_select && "abs(extHit.y - barHit.y) < 0.01 && p < 2";

  h = new TH1F("h301", "extrapolated track resolution in z for p < 2 GeV/c", 
		      100, -10, 10);
  h->GetXaxis()->SetTitle("#Deltaz [mm]");
  h->GetYaxis()->SetTitle("entries/bin");
  top->Draw("(extHit.z - barHit.z)*10 >> h301", trackSelect);
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Extrapolated minus true z of the track at the bar inner surface "
    "for p < 2 GeV/c."));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
    "Significantly broader distribution or offset to zero signals problems in tracking "
    "and may cause severe PID performance degradation."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->Write();

  h = new TH1F("h302", "extrapolated track resolution in x (e.g. r#phi) for p < 2 GeV/c", 
		      100, -10, 10);
  h->GetXaxis()->SetTitle("#Deltax [mm]");
  h->GetYaxis()->SetTitle("entries/bin");
  top->Draw("(extHit.x - barHit.x)*10 >> h302", trackSelect);
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Extrapolated minus true x of the track at the bar inner surface "
    "for p < 2 GeV/c."));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
    "Significantly broader distribution or offset to zero signals problems in tracking "
    "and may cause severe PID performance degradation."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->Write();

  h = new TH1F("h303", "extrapolated track resolution in #theta for p < 2 GeV/c", 
		      100, -20, 20);
  h->GetXaxis()->SetTitle("#Delta#theta [mrad]");
  h->GetYaxis()->SetTitle("entries/bin");
  top->Draw("(extHit.theta - barHit.theta)*1000 >> h303", trackSelect);
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Extrapolated minus true theta of the track at the bar inner surface "
    "for p < 2 GeV/c."));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
    "Significantly broader distribution or offset to zero signals problems in tracking "
    "and may cause severe PID performance degradation."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->Write();

  h = new TH1F("h304", "extrapolated track resolution in #phi for p < 2 GeV/c", 
		      100, -20, 20);
  h->GetXaxis()->SetTitle("#Delta#phi [mrad]");
  h->GetYaxis()->SetTitle("entries/bin");
  top->Draw("(extHit.phi - barHit.phi)*1000 >> h304", trackSelect);
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Extrapolated minus true phi of the track at the bar inner surface "
    "for p < 2 GeV/c."));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
    "Significantly broader distribution or offset to zero signals problems in tracking "
    "and may cause severe PID performance degradation."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->Write();

  h = new TH1F("h305", "extrapolated track resolution in time-of-flight for p < 2 GeV/c", 
		      100, -100, 100);
  h->GetXaxis()->SetTitle("#Deltat [ps]");
  h->GetYaxis()->SetTitle("entries/bin");
  top->Draw("(extHit.time - barHit.time)*1000 >> h305", trackSelect && 
	    TCut("extHit.PDG == barHit.PDG"));
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Extrapolated minus true time-of-flight of the track at the bar inner surface "
    "for p < 2 GeV/c."));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
    "Significantly broader distribution or offset to zero signals problems in tracking "
    "and may cause severe PID performance degradation."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->Write();

  // resolutions above 2 GeV/c

  trackSelect = evt_select && "abs(extHit.y - barHit.y) < 0.01 && p > 2";

  h = new TH1F("h306", "extrapolated track resolution in z for p > 2 GeV/c", 
		      100, -10, 10);
  h->GetXaxis()->SetTitle("#Deltaz [mm]");
  h->GetYaxis()->SetTitle("entries/bin");
  top->Draw("(extHit.z - barHit.z)*10 >> h306", trackSelect);
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Extrapolated minus true z of the track at the bar inner surface "
    "for p > 2 GeV/c."));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
    "Significantly broader distribution or offset to zero signals problems in tracking "
    "and may cause severe PID performance degradation."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->Write();

  h = new TH1F("h307", "extrapolated track resolution in x (e.g. r#phi) for p > 2 GeV/c", 
		      100, -10, 10);
  h->GetXaxis()->SetTitle("#Deltax [mm]");
  h->GetYaxis()->SetTitle("entries/bin");
  top->Draw("(extHit.x - barHit.x)*10 >> h307", trackSelect);
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Extrapolated minus true x of the track at the bar inner surface "
    "for p > 2 GeV/c."));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
    "Significantly broader distribution or offset to zero signals problems in tracking "
    "and may cause severe PID performance degradation."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->Write();

  h = new TH1F("h308", "extrapolated track resolution in #theta for p > 2 GeV/c", 
		      100, -20, 20);
  h->GetXaxis()->SetTitle("#Delta#theta [mrad]");
  h->GetYaxis()->SetTitle("entries/bin");
  top->Draw("(extHit.theta - barHit.theta)*1000 >> h308", trackSelect);
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Extrapolated minus true theta of the track at the bar inner surface "
    "for p > 2 GeV/c."));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
    "Significantly broader distribution or offset to zero signals problems in tracking "
    "and may cause severe PID performance degradation."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->Write();

  h = new TH1F("h309", "extrapolated track resolution in #phi for p > 2 GeV/c", 
		      100, -20, 20);
  h->GetXaxis()->SetTitle("#Delta#phi [mrad]");
  h->GetYaxis()->SetTitle("entries/bin");
  top->Draw("(extHit.phi - barHit.phi)*1000 >> h309", trackSelect);
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Extrapolated minus true phi of the track at the bar inner surface "
    "for p > 2 GeV/c."));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
    "Significantly broader distribution or offset to zero signals problems in tracking "
    "and may cause severe PID performance degradation."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->Write();

  h = new TH1F("h310", "extrapolated track resolution in time-of-flight for p > 2 GeV/c", 
		      100, -100, 100);
  h->GetXaxis()->SetTitle("#Deltat [ps]");
  h->GetYaxis()->SetTitle("entries/bin");
  top->Draw("(extHit.time - barHit.time)*1000 >> h310", trackSelect && 
	    TCut("extHit.PDG == barHit.PDG"));
  h->GetListOfFunctions()->Add(new TNamed("Description", 
    "Extrapolated minus true time-of-flight of the track at the bar inner surface "
    "for p > 2 GeV/c."));
  h->GetListOfFunctions()->Add(new TNamed("Check", 
    "Significantly broader distribution or offset to zero signals problems in tracking "
    "and may cause severe PID performance degradation."));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "marko.staric@ijs.si"));
  h->Write();

  output->Close();
}

