/*
  This macro produces ARICH background plots.

  Run it as: root -l 'BeamBack_arich.cc(simulated time in us,"path to files")'

  Path to files should be directory containing files produced by "ARICHBkg.py".
  It is assumed file names contain source of background ...RBB,BHWide...,...

  !! IMPORTANT
  set the correct scalings in case of using samples of different time lenghts (in line 36)

  Author: Luka Santelj
  Contributor: Manca Mrvar, Leonid Burmistrov
*/

#include <iostream>
#include <map>
#include <sstream>
#include <math.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph2D.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TGaxis.h>
#include <TBranch.h>
#include <TObjArray.h>
#include <TLeaf.h>
#include <TVector3.h>
#include <TCanvas.h>
#include <TPaveText.h>

using namespace std;

// scalings for "RBB", "BHWide", "Touschek_HER", "Touschek_LER", "Coulomb_HER", "Coulomb_LER","2-photon", "BHWideLargeAngle"
double scaling[8] = {100., 1, 10., 10., 10., 10., 10., 0.001};
int sourceType[8] = {0, 1, 2, 3, 4, 5, 6, 1};

// returns "ring number" of HAPD module with id modID
int mod2row(int modID)
{
  if (modID <= 42) return 0;
  if (modID <= 90) return 1;
  if (modID <= 144) return 2;
  if (modID <= 204) return 3;
  if (modID <= 270) return 4;
  if (modID <= 342) return 5;
  if (modID <= 420) return 6;
  return -1; // -1 if invalid input
}

TCanvas* make_plot(double data[][7], TString title, int type)
{

  TH1F* h1 = new TH1F(title + "h1", "RBB", 7, 0.5, 7.5);
  TH1F* h2 = new TH1F(title + "h2", "BHWide", 7, 0.5, 7.5);
  TH1F* h3 = new TH1F(title + "h3", "Touschek HER", 7, 0.5, 7.5);
  TH1F* h4 = new TH1F(title + "h4", "Touschek LER", 7, 0.5, 7.5);
  TH1F* h5 = new TH1F(title + "h5", "Coulomb HER", 7, 0.5, 7.5);
  TH1F* h6 = new TH1F(title + "h6", "Coulomb LER", 7, 0.5, 7.5);
  TH1F* h7 = new TH1F(title + "h7", "2-photon", 7, 0.5, 7.5);
  h1->SetStats(0); h2->SetStats(0); h3->SetStats(0);
  h4->SetStats(0); h5->SetStats(0); h6->SetStats(0); h7->SetStats(0);
  h1->SetBit(TH1::kNoTitle); h2->SetBit(TH1::kNoTitle); h3->SetBit(TH1::kNoTitle);
  h4->SetBit(TH1::kNoTitle); h5->SetBit(TH1::kNoTitle); h6->SetBit(TH1::kNoTitle); h7->SetBit(TH1::kNoTitle);

  for (int i = 0; i < 7; i++) {
    h1->SetBinContent(i + 1, data[i][0]);
    h2->SetBinContent(i + 1, data[i][0] + data[i][1]);
    h3->SetBinContent(i + 1, data[i][0] + data[i][1] + data[i][2]);
    h4->SetBinContent(i + 1, data[i][0] + data[i][1] + data[i][2] + data[i][3]);
    h5->SetBinContent(i + 1, data[i][0] + data[i][1] + data[i][2] + data[i][3] + data[i][4]);
    h6->SetBinContent(i + 1, data[i][0] + data[i][1] + data[i][2] + data[i][3] + data[i][4] + data[i][5]);
    h7->SetBinContent(i + 1, data[i][0] + data[i][1] + data[i][2] + data[i][3] + data[i][4] + data[i][5] + data[i][6]);
  }
  TCanvas* c1 = new TCanvas(title);
  h7->SetLineColor(7);
  h7->GetXaxis()->SetTitle("HAPD ring #");
  TPaveText* pt1 = new TPaveText(0.20, 0.92, 0.8, 0.99, "NDC");
  if (type == 1) { h7->GetYaxis()->SetTitle("radiation dose [gray/year]"); pt1->AddText("Radiation dose");}
  if (type == 2) { h7->GetYaxis()->SetTitle("1MeV equiv. neutrons / cm^2 / year"); pt1->AddText("1Mev equiv. neutron flux");}
  if (type == 3) { h7->GetYaxis()->SetTitle("photons / cm^2 / s"); pt1->AddText("photon flux on HAPDs");}

  h7->SetFillColor(7);
  h7->Draw();
  pt1->Draw();

  h6->SetFillColor(1);
  h6->SetLineColor(1);
  h6->Draw("same");

  h5->SetFillColor(6);
  h5->SetLineColor(6);
  h5->Draw("same");
  h4->SetFillColor(5);
  h4->SetLineColor(5);
  h4->Draw("same");
  h3->SetFillColor(3);
  h3->SetLineColor(3);
  h3->Draw("same");
  h2->SetFillColor(4);
  h2->SetLineColor(4);
  h2->Draw("same");
  h1->SetFillColor(2);
  h1->SetLineColor(2);
  h1->Draw("same");
  c1->BuildLegend(0.6, 0.6, 0.9, 0.9);
  c1->SetTitle("");
  return c1;
}


//int BeamBack_arich(double time = 1000, std::string path = "/gpfs/home/belle/mmanca/basf2/background/16th_campaign/arich_")
void BeamBack_arich(double time = 1000, std::string path = "/gpfs/home/belle/mmanca/basf2/background/16th_campaign/arich_")
{

  gROOT->SetBatch(kTRUE);
  TGraph2D* dteb = new TGraph2D(420);
  TGraph2D* dtnb = new TGraph2D(420);
  Belle2::ARICHChannelHist* chHits = new Belle2::ARICHChannelHist("neutronFlux", "1MeV eq. neutron flux /cm2 / year on HAPD board",
      1);

  TGraph2D* dteh = new TGraph2D(420);
  TGraph2D* dtnh = new TGraph2D(420);
  TGraph2D* dtp = new TGraph2D(420);
  dteb->SetName("dteb"); dteb->SetTitle("Deposited energy [gray/year] in HAPD board");
  dteh->SetName("dteh"); dteh->SetTitle("Deposited energy [gray/year] in HAPD bottom");
  dtnb->SetName("dtnb"); dtnb->SetTitle("1MeV equiv. neutron flux / cm2 / year on HAPD board [x10^{11}]");
  dtnh->SetName("dtnh"); dtnh->SetTitle("1MeV equiv. neutron flux / cm2 / year on HAPD bottom [x10^{11}]");
  dtp->SetName("dtp");   dtp->SetTitle("Optical photon flux on hapd [photons / s]");

  double edep_board[420][7]; // background contribution from each source
  double edep_hapd[420][7];
  double nflux_board[420][7]; // This holds neutron flux on each HAPD module
  double nflux_hapd[420][7];
  double flux_phot[420][7];
  double edep_board_all[420]; // sum of background from all sources
  double nflux_board_all[420];
  double edep_hapd_all[420];
  double nflux_hapd_all[420];
  double phot_all[420];

  // hapd x,y positions
  double origins[420][2];

  // intialize
  for (int j = 0; j < 7; j++) {
    for (int i = 0; i < 420; i++) {
      edep_board[i][j] = 0;
      edep_hapd[i][j] = 0;
      nflux_board[i][j] = 0;
      nflux_hapd[i][j] = 0;
      flux_phot[i][j] = 0;
      origins[i][0] = 0.; origins[i][1] = 0.;
      edep_board_all[i] = 0.; edep_hapd_all[i] = 0.;
      nflux_board_all[i] = 0.; nflux_hapd_all[i] = 0.;
      phot_all[i] = 0.;
    }
  }

  TChain* tree = new TChain("TrHits");

  // sources
  //TString tip[7] = {"RBB", "BHWide", "Touschek_HER", "Touschek_LER", "Coulomb_HER", "Coulomb_LER", "2-photon"};


  // input file names
  /*  for (int j = 0; j < 7; j++) {
      for (int i = 0; i < 1; i++) {
      stringstream filename;
      filename << path.c_str() << "*" << tip[j] << "*" << ".root";
      printf("%s\n", filename.str().c_str());
      tree->Add(filename.str().c_str());
      }
      }*/

  stringstream filename;
  filename << path.c_str() << "*.root";
  tree->Add(filename.str().c_str());

  // this is neutron energy spectrum
  TH1D* nenergy = new TH1D("nenergy", "neutron spectrum; log10(k.e./MeV)", 2000, -10., 2.);
  nenergy->SetStats(0);
  // set variables from input tree files
  int type = -1;
  int modID;
  int pdg = -1;
  int source;
  double edep = 0;
  TVector3* modOrig = 0;
  TVector3* mom = 0;
  double phnw = 0; double trlen = 0; double en = 0;
  tree->SetBranchAddress("phPDG", &pdg);
  tree->SetBranchAddress("moduleID", &modID);
  tree->SetBranchAddress("type", &type);
  tree->SetBranchAddress("edep", &edep);
  tree->SetBranchAddress("modOrig", &modOrig);
  tree->SetBranchAddress("phmom", &mom);
  tree->SetBranchAddress("phnw", &phnw);
  tree->SetBranchAddress("trlen", &trlen);
  tree->SetBranchAddress("en", &en);
  tree->SetBranchAddress("source", &source);

  int nevents = tree->GetEntries();
  //double nmax = 0;
  // loop over all hits
  for (int e = 0; e < nevents; e++) {

    edep = 0;
    type = -1;
    pdg = -1;
    //modID = 1;
    mom->SetXYZ(0., 0., 0.);

    tree->GetEvent(e);

    if (source < 0) continue;
    if (modID < 1) continue;

    // read the corresponding module (HAPD) x,y coordinates
    if (modID > 0) {
      origins[modID - 1][0] = modOrig->X();
      origins[modID - 1][1] = modOrig->Y();
    } else continue;

    // this is for photon flux
    if (type == 2) {
      flux_phot[modID - 1][sourceType[source]] += scaling[source];
    }

    if (type != 2) {

      //  energy deposit from a hit (two volumes are sensitive el. board and hapd bottom, commonly I show results for board.
      //  Anyhow the rates are approximately equal.)
      if (pdg != 2112) {
        edep *= scaling[source];
        if (type == 0) edep_board[modID - 1][sourceType[source]] += edep;
        if (type == 1) edep_hapd[modID - 1][sourceType[source]]  += edep;
      }

      else { // if neutron

        // Calculate neutron kinetic energy in MeV. mom is obtained from BeamBackHit->GetMomentum() and is in GeV
        double mass = 940.0;
        double pp = mom->Mag() * 1000.;
        double ee = sqrt(pp * pp + mass * mass) - mass;
        double lee = log10(ee);

        // contibution to neutron flux
        if (type == 0) {
          if (modID == 10) continue;
          phnw *= scaling[source];
          nflux_board[modID - 1][sourceType[source]] += phnw * trlen / 0.2; // trlen/0.2, this is kind of cos(theta) correction to the flux.
          // trlen = track length in volume, and 0.2cm is thickness of board.
          nenergy->Fill(lee);
        }
        if (type == 1) {
          phnw *= scaling[source];
          nflux_hapd[modID - 1][sourceType[source]] += phnw * trlen / 0.05;
        }
      }
    }

  } // end of event loop


  // values averaged over HAPD rings;
  double avgeb[7][7];
  double avgeh[7][7];
  double avgnb[7][7];
  double avgnh[7][7];
  double avgf[7][7];

  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 7; j++) {
      avgeb[i][j] = 0;  avgeh[i][j] = 0;
      avgnb[i][j] = 0;  avgnh[i][j] = 0;
      avgf[i][j] = 0;
    }
  }

  double nhapd[7] = {42., 48., 54., 60., 66., 72., 78.}; // 7 rings

  // This is important. nflux_board holds total flux of neutrons wighted by 1MeV equiv. factor in a given
  // time "time". Here this is transformed in flux/cm^2/year.

  TH1F*  ndist = new TH1F("ndist", "nflux distribution;1MeV equiv. neutrons / cm^2 / year; # of HAPDs", 1000, 0, 8);
  TH1F*  edist = new TH1F("edist", "deposit en. distribution; radiation dose [gray/year]; # of HAPDs", 1000, 0, 10);

  for (int i = 0; i < 420; i++) {
    int nrow = mod2row(i + 1);
    for (int j = 0; j < 7; j++) {

      edep_board[i][j] = edep_board[i][j] * 1.6E+6 / 47.25 / time; // 47.25 mass of FEB in grams
      edep_hapd[i][j] = edep_hapd[i][j] * 1.6E+6 / 5.49 / time; // 5.49 mass of APD chip (volume 62.7x62.7x0.06)

      nflux_board[i][j] = nflux_board[i][j] * 1.E+13 / 56.25 / time; // board surface is 56.25 cm^2
      nflux_hapd[i][j] = nflux_hapd[i][j] * 1.E+13 / 39.3 / time; // apd surface

      flux_phot[i][j] = flux_phot[i][j] * 1.E+6 / 53.29 / time;  // photocathode size = 7.3x7.3cm

      edep_board_all[i] += edep_board[i][j];
      edep_hapd_all[i] += edep_hapd[i][j];
      nflux_board_all[i] += nflux_board[i][j];
      nflux_hapd_all[i] += nflux_hapd[i][j];
      phot_all[i] += flux_phot[i][j];

      avgeb[nrow][j] += edep_board[i][j] / nhapd[nrow];
      avgeh[nrow][j] += edep_hapd[i][j] / nhapd[nrow];
      avgnb[nrow][j] += nflux_board[i][j] / nhapd[nrow];
      avgnh[nrow][j] += nflux_hapd[i][j] / nhapd[nrow];
      avgf[nrow][j] += flux_phot[i][j] / nhapd[nrow];
    }

    double nn = nflux_board_all[i] / 1E+11;
    ndist->Fill(nn);
    edist->Fill(edep_board_all[i]);
    // fill 2D graphs
    dteb->SetPoint(i, origins[i][0], origins[i][1], edep_board_all[i]);
    dtnb->SetPoint(i, origins[i][0], origins[i][1], nn);
    dteh->SetPoint(i, origins[i][0], origins[i][1], edep_hapd_all[i]);
    dtnh->SetPoint(i, origins[i][0], origins[i][1], nflux_hapd_all[i]);
    dtp->SetPoint(i, origins[i][0], origins[i][1], phot_all[i]);
    chHits->setBinContent(i + 1, nn);

  } // end of HAPD loop

  dteb->SetMarkerStyle(21);
  dteh->SetMarkerStyle(21);
  dtnb->SetMarkerStyle(21);
  dteh->SetMarkerStyle(21);
  dtp->SetMarkerStyle(21);
  //chHits->SetMarkerStyle(21);


  TGaxis::SetMaxDigits(1);
  TCanvas* c1 = make_plot(avgnb, "n_board", 2);

  TCanvas* c2 = make_plot(avgeb, "e_board", 1);

  TCanvas* c3 = make_plot(avgnh, "n_hapd", 2);

  TCanvas* c4 = make_plot(avgeh, "e_hapd", 1);

  TCanvas* c5 = make_plot(avgf, "photons", 3);

  // write output file
  TFile* f = new TFile("arich_background_phase3.root", "RECREATE");

  c1->Write(); c2->Write(); c3->Write(); c4->Write(); c5->Write();
  dteb->Write(); dteh->Write();
  dtnb->Write(); dtnh->Write();
  dtp->Write();
  chHits->Write();
  ndist->Write();
  edist->Write();
  nenergy->Write();
  f->Close();

  delete c1;
  delete c2;
  delete c3;
  delete c4;
  delete c5;

  //return 0;
}
