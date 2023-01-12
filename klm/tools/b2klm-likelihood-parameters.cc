/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* KLM headers. */
#include <klm/dataobjects/KLMMuidLikelihood.h>
#include <klm/muid/MuidElementNumbers.h>

/* Belle2 headers. */
#include <analysis/dataobjects/Particle.h>
#include <analysis/variables/AcceptanceVariables.h>
#include <mdst/dataobjects/MCParticle.h>

/* ROOT headers. */
#include <Math/Vector4D.h>
#include <TCanvas.h>
#include <TChain.h>
#include <TChainElement.h>
#include <TF1.h>
#include <TFile.h>
#include <TH1D.h>
#include <TMath.h>
#include <TROOT.h>
#include <TStyle.h>

/* C++ headers. */
#include <cmath>
#include <cstdio>
#include <iostream>

#define NOUTCOME 67

using namespace Belle2;

TStyle* Belle2Style()
{
  TStyle* belle2Style = new TStyle("BELLE2", "Belle2 style");

  // use plain black on white colors
  Int_t icol = 0; // WHITE
  belle2Style->SetFrameBorderMode(icol);
  belle2Style->SetFrameFillColor(icol);
  belle2Style->SetCanvasBorderMode(icol);
  belle2Style->SetCanvasColor(icol);
  belle2Style->SetPadBorderMode(icol);
  belle2Style->SetPadColor(icol);
  belle2Style->SetStatColor(icol);
  //belle2Style->SetFillColor(icol); // don't use: white fill color for *all* objects

  // set the paper & margin sizes
  belle2Style->SetPaperSize(20, 26);

  // set margin sizes
  belle2Style->SetPadTopMargin(0.05);
  belle2Style->SetPadRightMargin(0.05);
  belle2Style->SetPadBottomMargin(0.16);
  belle2Style->SetPadLeftMargin(0.16);

  // set title offsets (for axis label)
  belle2Style->SetTitleXOffset(1.4);
  belle2Style->SetTitleYOffset(1.4);

  // use large fonts
  //Int_t font=72; // Helvetica italics
  Int_t font = 42; // Helvetica
  Double_t tsize = 0.05;
  belle2Style->SetTextFont(font);

  belle2Style->SetTextSize(tsize);
  belle2Style->SetLabelFont(font, "x");
  belle2Style->SetTitleFont(font, "x");
  belle2Style->SetLabelFont(font, "y");
  belle2Style->SetTitleFont(font, "y");
  belle2Style->SetLabelFont(font, "z");
  belle2Style->SetTitleFont(font, "z");

  belle2Style->SetLabelSize(tsize, "x");
  belle2Style->SetTitleSize(tsize, "x");
  belle2Style->SetLabelSize(tsize, "y");
  belle2Style->SetTitleSize(tsize, "y");
  belle2Style->SetLabelSize(tsize, "z");
  belle2Style->SetTitleSize(tsize, "z");

  // use bold lines and markers
  belle2Style->SetMarkerStyle(20);
  belle2Style->SetMarkerSize(1.2);
  belle2Style->SetHistLineWidth(2.);
  belle2Style->SetLineStyleString(2, "[12 12]"); // postscript dashes

  // get rid of X error bars
  //belle2Style->SetErrorX(0.001);
  // get rid of error bar caps
  belle2Style->SetEndErrorSize(0.);

  // do not display any of the standard histogram decorations
  belle2Style->SetOptTitle(0);
  //belle2Style->SetOptStat(1111);
  belle2Style->SetOptStat(0);
  //belle2Style->SetOptFit(1111);
  belle2Style->SetOptFit(0);

  // put tick marks on top and RHS of plots
  belle2Style->SetPadTickX(1);
  belle2Style->SetPadTickY(1);

  return belle2Style;

}

void SetBelle2Style()
{
  static TStyle* belle2Style = 0;
  std::cout << "\nApplying BELLE2 style settings...\n" << std::endl ;
  if (belle2Style == 0) belle2Style = Belle2Style();
  gROOT->SetStyle("BELLE2");
  gROOT->ForceStyle();
}

void makeprob(TChain* chain, FILE* output, char* xmllabel)
{

  //Check first 2 char of the strings if equal (==0) or not
  bool isMuon = (strncmp(xmllabel, "Mu", 2) == 0);
  bool isPion = (strncmp(xmllabel, "Pi", 2) == 0);
  bool isKaon = (strncmp(xmllabel, "Ka", 2) == 0);
  bool isElec = (strncmp(xmllabel, "El", 2) == 0) || (strncmp(xmllabel, "Po", 2) == 0);
  bool isProt = (strncmp(xmllabel, "Pr", 2) == 0) || (strncmp(xmllabel, "An", 2) == 0);
  bool isDeut = (strncmp(xmllabel, "De", 2) == 0);

  char label[80], title[80];
  char detectorName[3][40] = { "BarrelAndEndcap", "BarrelOnly", "EndcapOnly" };
  //List of all the possible outcome and relative explanations
  char outcomeName[NOUTCOME][40] = { "",
                                     "Barrel stop",
                                     "Fwd Endcap stop",
                                     "Barrel exit",
                                     "Fwd Endcap exit",
                                     "Bwd Endcap stop",
                                     "Bwd Endcap exit",
                                     "Fwd Endcap stop B0+E",
                                     "Fwd Endcap stop B1+E",
                                     "Fwd Endcap stop B2+E",
                                     "Fwd Endcap stop B3+E",
                                     "Fwd Endcap stop B4+E",
                                     "Fwd Endcap stop B5+E",
                                     "Fwd Endcap stop B6+E",
                                     "Fwd Endcap stop B7+E",
                                     "Fwd Endcap stop B8+E",
                                     "Fwd Endcap stop B9+E",
                                     "Fwd Endcap stop B10+E",
                                     "Fwd Endcap stop B11+E",
                                     "Fwd Endcap stop B12+E",
                                     "Fwd Endcap stop B13+E",
                                     "Fwd Endcap stop B14+E",
                                     "Bwd Endcap stop B0+E",
                                     "Bwd Endcap stop B1+E",
                                     "Bwd Endcap stop B2+E",
                                     "Bwd Endcap stop B3+E",
                                     "Bwd Endcap stop B4+E",
                                     "Bwd Endcap stop B5+E",
                                     "Bwd Endcap stop B6+E",
                                     "Bwd Endcap stop B7+E",
                                     "Bwd Endcap stop B8+E",
                                     "Bwd Endcap stop B9+E",
                                     "Bwd Endcap stop B10+E",
                                     "Bwd Endcap stop B11+E",
                                     "Bwd Endcap stop B12+E",
                                     "Bwd Endcap stop B13+E",
                                     "Bwd Endcap stop B14+E",
                                     "Fwd Endcap exit B0+E",
                                     "Fwd Endcap exit B1+E",
                                     "Fwd Endcap exit B2+E",
                                     "Fwd Endcap exit B3+E",
                                     "Fwd Endcap exit B4+E",
                                     "Fwd Endcap exit B5+E",
                                     "Fwd Endcap exit B6+E",
                                     "Fwd Endcap exit B7+E",
                                     "Fwd Endcap exit B8+E",
                                     "Fwd Endcap exit B9+E",
                                     "Fwd Endcap exit B10+E",
                                     "Fwd Endcap exit B11+E",
                                     "Fwd Endcap exit B12+E",
                                     "Fwd Endcap exit B13+E",
                                     "Fwd Endcap exit B14+E",
                                     "Bwd Endcap exit B0+E",
                                     "Bwd Endcap exit B1+E",
                                     "Bwd Endcap exit B2+E",
                                     "Bwd Endcap exit B3+E",
                                     "Bwd Endcap exit B4+E",
                                     "Bwd Endcap exit B5+E",
                                     "Bwd Endcap exit B6+E",
                                     "Bwd Endcap exit B7+E",
                                     "Bwd Endcap exit B8+E",
                                     "Bwd Endcap exit B9+E",
                                     "Bwd Endcap exit B10+E",
                                     "Bwd Endcap exit B11+E",
                                     "Bwd Endcap exit B12+E",
                                     "Bwd Endcap exit B13+E",
                                     "Bwd Endcap exit B14+E"
                                   };
  char outcomeComment[NOUTCOME][100] = { "",
                                         "soft tracks that range out (stop) within the barrel KLM",                             // outcome=1
                                         "soft tracks that range out (stop) within the forward endcap KLM (no barrel hits)",    // outcome=2
                                         "hard tracks that escape from the barrel KLM",                                         // outcome=3
                                         "hard tracks that escape from the forward endcap KLM (no barrel hits)",                // outcome=4
                                         "soft tracks that range out (stop) within the backward endcap KLM (no barrel hits)",   // outcome=5
                                         "hard tracks that escape from the backward endcap KLM (no barrel hits)",               // outcome=6
                                         "soft tracks that range out (stop) within the forward endcap KLM with B0 hit(s)",      // outcome=7
                                         "soft tracks that range out (stop) within the forward endcap KLM with B1 hit(s)",      // outcome=8
                                         "soft tracks that range out (stop) within the forward endcap KLM with B2 hit(s)",      // outcome=9
                                         "soft tracks that range out (stop) within the forward endcap KLM with B3 hit(s)",      // outcome=10
                                         "soft tracks that range out (stop) within the forward endcap KLM with B4 hit(s)",      // outcome=11
                                         "soft tracks that range out (stop) within the forward endcap KLM with B5 hit(s)",      // outcome=12
                                         "soft tracks that range out (stop) within the forward endcap KLM with B6 hit(s)",      // outcome=13
                                         "soft tracks that range out (stop) within the forward endcap KLM with B7 hit(s)",      // outcome=14
                                         "soft tracks that range out (stop) within the forward endcap KLM with B8 hit(s)",      // outcome=15
                                         "soft tracks that range out (stop) within the forward endcap KLM with B9 hit(s)",      // outcome=16
                                         "soft tracks that range out (stop) within the forward endcap KLM with B10 hit(s)",      // outcome=17
                                         "soft tracks that range out (stop) within the forward endcap KLM with B11 hit(s)",      // outcome=18
                                         "soft tracks that range out (stop) within the forward endcap KLM with B12 hit(s)",      // outcome=19
                                         "soft tracks that range out (stop) within the forward endcap KLM with B13 hit(s)",      // outcome=20
                                         "soft tracks that range out (stop) within the forward endcap KLM with B14 hit(s)",      // outcome=21
                                         "soft tracks that range out (stop) within the backward endcap KLM with B0 hit(s)",      // outcome=22
                                         "soft tracks that range out (stop) within the backward endcap KLM with B1 hit(s)",      // outcome=23
                                         "soft tracks that range out (stop) within the backward endcap KLM with B2 hit(s)",      // outcome=24
                                         "soft tracks that range out (stop) within the backward endcap KLM with B3 hit(s)",      // outcome=25
                                         "soft tracks that range out (stop) within the backward endcap KLM with B4 hit(s)",      // outcome=26
                                         "soft tracks that range out (stop) within the backward endcap KLM with B5 hit(s)",      // outcome=27
                                         "soft tracks that range out (stop) within the backward endcap KLM with B6 hit(s)",      // outcome=28
                                         "soft tracks that range out (stop) within the backward endcap KLM with B7 hit(s)",      // outcome=29
                                         "soft tracks that range out (stop) within the backward endcap KLM with B8 hit(s)",      // outcome=30
                                         "soft tracks that range out (stop) within the backward endcap KLM with B9 hit(s)",      // outcome=31
                                         "soft tracks that range out (stop) within the backward endcap KLM with B10 hit(s)",      // outcome=32
                                         "soft tracks that range out (stop) within the backward endcap KLM with B11 hit(s)",      // outcome=33
                                         "soft tracks that range out (stop) within the backward endcap KLM with B12 hit(s)",      // outcome=34
                                         "soft tracks that range out (stop) within the backward endcap KLM with B13 hit(s)",      // outcome=35
                                         "soft tracks that range out (stop) within the backward endcap KLM with B14 hit(s)",      // outcome=36
                                         "hard tracks that escape from the forward endcap KLM with B0 hit(s)",      // outcome=37
                                         "hard tracks that escape from the forward endcap KLM with B1 hit(s)",      // outcome=38
                                         "hard tracks that escape from the forward endcap KLM with B2 hit(s)",      // outcome=39
                                         "hard tracks that escape from the forward endcap KLM with B3 hit(s)",      // outcome=40
                                         "hard tracks that escape from the forward endcap KLM with B4 hit(s)",      // outcome=41
                                         "hard tracks that escape from the forward endcap KLM with B5 hit(s)",      // outcome=42
                                         "hard tracks that escape from the forward endcap KLM with B6 hit(s)",      // outcome=43
                                         "hard tracks that escape from the forward endcap KLM with B7 hit(s)",      // outcome=44
                                         "hard tracks that escape from the forward endcap KLM with B8 hit(s)",      // outcome=45
                                         "hard tracks that escape from the forward endcap KLM with B9 hit(s)",      // outcome=46
                                         "hard tracks that escape from the forward endcap KLM with B10 hit(s)",      // outcome=47
                                         "hard tracks that escape from the forward endcap KLM with B11 hit(s)",      // outcome=48
                                         "hard tracks that escape from the forward endcap KLM with B12 hit(s)",      // outcome=49
                                         "hard tracks that escape from the forward endcap KLM with B13 hit(s)",      // outcome=50
                                         "hard tracks that escape from the forward endcap KLM with B14 hit(s)",      // outcome=51
                                         "hard tracks that escape from the backward endcap KLM with B0 hit(s)",      // outcome=52
                                         "hard tracks that escape from the backward endcap KLM with B1 hit(s)",      // outcome=53
                                         "hard tracks that escape from the backward endcap KLM with B2 hit(s)",      // outcome=54
                                         "hard tracks that escape from the backward endcap KLM with B3 hit(s)",      // outcome=55
                                         "hard tracks that escape from the backward endcap KLM with B4 hit(s)",      // outcome=56
                                         "hard tracks that escape from the backward endcap KLM with B5 hit(s)",      // outcome=57
                                         "hard tracks that escape from the backward endcap KLM with B6 hit(s)",      // outcome=58
                                         "hard tracks that escape from the backward endcap KLM with B7 hit(s)",      // outcome=59
                                         "hard tracks that escape from the backward endcap KLM with B8 hit(s)",      // outcome=60
                                         "hard tracks that escape from the backward endcap KLM with B9 hit(s)",      // outcome=61
                                         "hard tracks that escape from the backward endcap KLM with B10 hit(s)",      // outcome=62
                                         "hard tracks that escape from the backward endcap KLM with B11 hit(s)",      // outcome=63
                                         "hard tracks that escape from the backward endcap KLM with B12 hit(s)",      // outcome=64
                                         "hard tracks that escape from the backward endcap KLM with B13 hit(s)",      // outcome=65
                                         "hard tracks that escape from the backward endcap KLM with B14 hit(s)"
                                       };      // outcome=66
  sprintf(label, "Muon ID PDFs for %s", xmllabel);

  //Histograms definition
  TH1D* layerHitU[15][NOUTCOME];
  TH1D* layerHitV[15][NOUTCOME];
  TH1D* layerHitW[15][NOUTCOME];
  TH1D* rchisqA[21][3];
  TH1D* rchisqB[21][3];
  TH1D* discardA;
  TH1D* discardB;

  //prepare histograms: title, bins and intervals
  //Sumw2 -> store sum of squares of weights
  discardA = new TH1D("discardA", "discard fraction", 70, -0.5, 69.5);
  discardB = new TH1D("discardB", "discard denominator", 70, -0.5, 69.5);
  for (int outcome = 1; outcome < NOUTCOME; ++outcome) {
    for (int layer = 0; layer < 15; ++layer) {
      sprintf(label, "LayerHitU-layer%02d-outcome%d", layer, outcome);
      sprintf(title, "LayerHitU for %s: layer %02d (%s)", xmllabel, layer, outcomeName[outcome]);
      layerHitU[layer][outcome] = new TH1D(label, title, 32, -0.5, 31.5);
      layerHitU[layer][outcome]->Sumw2(true);
      sprintf(label, "LayerHitV-layer%02d-outcome%d", layer, outcome);
      sprintf(title, "LayerHitV for %s: layer %02d (%s)", xmllabel, layer, outcomeName[outcome]);
      layerHitV[layer][outcome] = new TH1D(label, title, 32, -0.5, 31.5);
      layerHitV[layer][outcome]->Sumw2(true);
      sprintf(label, "LayerHitW-layer%02d-outcome%d", layer, outcome);
      sprintf(title, "LayerHitW for %s: layer %02d (%s)", xmllabel, layer, outcomeName[outcome]);
      layerHitW[layer][outcome] = new TH1D(label, title, 32, -0.5, 31.5);
      layerHitW[layer][outcome]->Sumw2(true);
    }
  }

  for (int detector = 0; detector <= 2; ++detector) {
    for (int halfNdof = 0; halfNdof <= 20; ++halfNdof) {
      sprintf(label, "ReducedChiSquaredA-%02ddof-detector%d", 2 * halfNdof, detector);
      sprintf(title, "Reduced Chi-squared for %s: %02d dof (%s)", xmllabel, 2 * halfNdof, detectorName[detector]);
      rchisqA[halfNdof][detector] = new TH1D(label, title, 100, 0.0, 10.0);
      rchisqA[halfNdof][detector]->Sumw2(true);
      sprintf(label, "ReducedChiSquaredB-%02ddof-detector%d", 2 * halfNdof, detector);
      sprintf(title, "Reduced Chi-squared for %s: %02d dof (%s)", xmllabel, 2 * halfNdof, detectorName[detector]);
      rchisqB[halfNdof][detector] = new TH1D(label, title, 1000, 0.0, 100.0);  // 10x wider range for integration
      rchisqB[halfNdof][detector]->Sumw2(true);
    }
  }

  printf("Finished defining histograms\n");

  TObjArray* fileElements = chain->GetListOfFiles();
  TIter next(fileElements);
  TChainElement* chEl = 0;
  while ((chEl = (TChainElement*)next())) {
    //printf("Processing file %s ...\n", chEl->GetTitle());
    TFile file(chEl->GetTitle());
    TTree* tree = (TTree*)file.Get("tree");
    tree->SetBranchStatus("*", 0);
    tree->SetBranchStatus("MCParticles*", 1);
    tree->SetBranchStatus("KLMMuidLikelihoods*", 1);
    TClonesArray* mcParticles = 0;
    tree->SetBranchAddress("MCParticles", &mcParticles);
    TClonesArray* muids = 0;
    tree->SetBranchAddress("KLMMuidLikelihoods", &muids);

    printf("Number of events: %lld in file %s\n", tree->GetEntriesFast(), chEl->GetTitle());
    ROOT::Math::PxPyPzEVector decayMomentum(0.0, 0.0, 0.0, 0.0);
    //ROOT::Math::PxPyPzEVector temp(0.0, 0.0, 0.0, 0.0);
    for (int event = 0; event < tree->GetEntriesFast(); ++event) {
      tree->GetEntry(event);
      if (mcParticles->GetEntriesFast() == 0) continue;
      Belle2::MCParticle* mcp = (Belle2::MCParticle*)((*mcParticles)[0]);
      Particle* particle = new Particle(mcp);
      // look for muon decay-in-flight or hard radiation/delta production by examining daughters
      if (isMuon) {
        if (mcp->getNDaughters() > 0) {
          decayMomentum.SetPxPyPzE(0.0, 0.0, 0.0, 0.0);
          int firstDaughter = mcp->getFirstDaughter();
          int lastDaughter = mcp->getLastDaughter();
          for (int d = firstDaughter - 1; d < lastDaughter; ++d) {
            Belle2::MCParticle* daughter = (Belle2::MCParticle*)((*mcParticles)[d]);
            int dPDG = abs(daughter->getPDG());
            if ((dPDG != 11) && (dPDG != 12) && (dPDG != 14) && (dPDG != 22)) continue; // electron or neutrino or gamma?
            //temp.SetVectM(daughter->getMomentum()*1000.0, (dPDG == 11 ? Const::electron.getMass() : 0.0));
            //decayMomentum += temp;
            ROOT::Math::XYZVector p3 = daughter->getMomentum();
            decayMomentum.SetPx(decayMomentum.Px() + p3.X());
            decayMomentum.SetPy(decayMomentum.Py() + p3.Y());
            decayMomentum.SetPz(decayMomentum.Pz() + p3.Z());
          }
          if (decayMomentum.P() > 0.05) {
            //debug printf("Event %5d: p(mu) = %f\n", event, decayMomentum.P());
            continue;
          }
        }
      }
      // end decay-in-flight check

      //Check acceptances and outcomes
      double mcMomentumZ = 1000.0 * mcp->getMomentum().Z(); // MeV/c
      //double mcMomentumR = 1000.0 * mcp->getMomentum().Rho(); // MeV/c
      double mcTheta = mcp->getMomentum().Theta() * 180.0 / TMath::Pi(); // degrees
      bool isForward = (mcMomentumZ > 0.0);
      //      if ((mcTheta > 31.5) && (mcTheta < 33.1)) continue;  // avoid forward ECL crack
      //      if ((mcTheta > 126.9) && (mcTheta < 128.4)) continue;  // avoid backward ECL crack
      if (Variable::thetaInECLAcceptance(particle) == 0) continue; // avoid forward & backward ECL crack
      double mcPhi = mcp->getMomentum().Phi() * 180.0 / TMath::Pi(); // degrees
      if ((mcTheta > 110.0) && (mcTheta < 125.0) && (mcPhi > 60.0) && (mcPhi < 120.0)) continue; // avoid chimney
      for (int m = 0; m < muids->GetEntriesFast(); ++m) {
        Belle2::KLMMuidLikelihood* muid = (Belle2::KLMMuidLikelihood*)((*muids)[m]);
        //        if (fabs(mcMomentumZ - muid->getMomentum().Z())    > 60.0) continue;
        //        if (fabs(mcMomentumR - muid->getMomentum().Perp()) > 60.0) continue;
        int outcome = muid->getOutcome();
        if (outcome <= 0) continue;
        unsigned int extLayerPattern = muid->getExtLayerPattern();
        unsigned int hitLayerPattern = muid->getHitLayerPattern();
        int blayer = muid->getBarrelExtLayer();
        int elayer = muid->getEndcapExtLayer();
        int layer = blayer;
        if (outcome == MuidElementNumbers::c_StopInForwardEndcap) { // forward endcap stop (no barrel hits). outcome=2
          layer = elayer;
          if (blayer < 0) {
            outcome = isForward ? MuidElementNumbers::c_StopInForwardEndcap :
                      MuidElementNumbers::c_StopInBackwardEndcap; // backward endcap stop (no barrel hits)
          } else {
            outcome = (isForward ? MuidElementNumbers::c_CrossBarrelStopInForwardMin : MuidElementNumbers::c_CrossBarrelStopInBackwardMin) +
                      blayer; // forward endcap stop (B+E)
          }
        } else if (outcome == MuidElementNumbers::c_ExitForwardEndcap) { // forward endcap exit (no barrel hits)
          layer = elayer;
          if (blayer < 0) {
            outcome = isForward ? MuidElementNumbers::c_ExitForwardEndcap :
                      MuidElementNumbers::c_ExitBackwardEndcap; // backward endcap exit (no barrel hits)
          } else {
            outcome = (isForward ? MuidElementNumbers::c_CrossBarrelExitForwardMin : MuidElementNumbers::c_CrossBarrelExitBackwardMin) +
                      blayer; // forward endcap exit (B+E)
          }
        }
        int ndof = muid->getDegreesOfFreedom();
        double rchisq = (ndof > 0 ? muid->getChiSquared() / ndof : -1.0);
        if (ndof >= 40) continue;
        int detector = 0; // crossed barrel and endcap
        if (elayer < 0) {
          detector = 1; // crossed barrel only
        } else if (blayer < 0) {
          detector = 2;  // crossed endcap only
        }
        rchisqA[ndof / 2][detector]->Fill(rchisq);
        unsigned int testbit = 1;
        unsigned int mask = 0;
        for (int bit = 0; bit <= blayer; ++bit) {
          mask |= testbit;
          testbit <<= 1;
        }
        //        testbit = 1 << 15;
        testbit = 1 << (MuidElementNumbers::getMaximalBarrelLayer() + 1);
        for (int bit = 0; bit <= elayer; ++bit) {
          mask |= testbit;
          testbit <<= 1;
        }
        discardB->Fill(outcome);
        if (extLayerPattern == mask) {
          testbit = 1;
          for (int bit = 0; bit < 32; ++bit) {
            if ((testbit & extLayerPattern) != 0) {
              double xLyr = (double)(bit);
              layerHitV[layer][outcome]->Fill(xLyr);
              if ((testbit & hitLayerPattern) != 0) {
                layerHitU[layer][outcome]->Fill(xLyr);
                layerHitW[layer][outcome]->Fill(xLyr);
              }
            }
            testbit <<= 1;
          }
        } else {
          discardA->Fill(outcome);
        }
      }
      delete particle;
    }

  } // end of loop over files in the chain

  TF1* rchisqFunc = new TF1("rchisqFunc", "[0]*[2]*std::pow(x*[1]*[2],[2]-1)/TMath::Gamma([2])*std::exp(-x*[1]*[2])", 0.0, 10.0);
  TF1* layerHitFunc = new TF1("layerHitFunc", "[0]*std::exp(-[1]*(x-[2]))", -0.5, 28.5);
  fprintf(output, "  <%s>\n", xmllabel);
  for (int outcome = 1; outcome <= 6; ++outcome) {
    for (int layer = 0; layer < MuidElementNumbers::getMaximalBarrelLayer() + 1; ++layer) {
      if ((outcome == MuidElementNumbers::c_StopInBarrel) && (layer > MuidElementNumbers::getMaximalBarrelLayer() - 1)) continue;
      if ((outcome == MuidElementNumbers::c_StopInForwardEndcap)
          && (layer > MuidElementNumbers::getMaximalEndcapForwardLayer() - 1)) continue;
      if ((outcome == MuidElementNumbers::c_ExitBarrel) && (layer > MuidElementNumbers::getMaximalBarrelLayer())) continue;
      if ((outcome == MuidElementNumbers::c_ExitForwardEndcap) && (layer > MuidElementNumbers::getMaximalEndcapForwardLayer())) continue;
      if ((outcome == MuidElementNumbers::c_StopInBackwardEndcap)
          && (layer > MuidElementNumbers::getMaximalEndcapBackwardLayer() - 1)) continue;
      if ((outcome == MuidElementNumbers::c_ExitBackwardEndcap)
          && (layer > MuidElementNumbers::getMaximalEndcapBackwardLayer())) continue;
      int bin0 = ((outcome == MuidElementNumbers::c_StopInBarrel)
                  || (outcome == MuidElementNumbers::c_ExitBarrel)) ? 0 : MuidElementNumbers::getMaximalBarrelLayer() + 1;
      if (layerHitV[layer][outcome]->GetEntries() > 0.0) {
        layerHitU[layer][outcome]->Divide(layerHitV[layer][outcome]);
      }
      printf("Fitting spectrum for outcome %d layer %d bin0 %d ...\n", outcome, layer, bin0);
      double p0 = 1.0;
      double p0Min = 0.01;
      double p0Max = 2.0;
      double p1 = 0.0;
      double p1Min = 0.1;
      double p1Max = 1.0;
      double xMin = bin0 + 0.5;
      if ((outcome == MuidElementNumbers::c_StopInBarrel) || (outcome == MuidElementNumbers::c_ExitBarrel)) xMin = bin0 + 1.5;
      double xMax = bin0 - 0.5 + layer;
      if (isMuon) {
        p0 = 0.97; p0Min = 0.9; p0Max = 1.0; p1 = 0.005; p1Min = 0.0; p1Max = 0.02;
      } else if (isPion) {
        p0 = 0.29; p0Min = 0.02; p0Max = 0.5; p1 = 0.30; p1Min = 0.2; p1Max = 0.5;
      } else if (isKaon) {
        p0 = 0.33; p0Min = 0.02; p0Max = 0.5; p1 = 0.23; p1Min = 0.17; p1Max = 0.6;
      } else if (isProt) {
        p0 = 0.05; p0Min = 0.001; p0Max = 0.4; p1 = 0.36; p1Min = 0.3; p1Max = 0.7;
        if ((outcome == MuidElementNumbers::c_StopInBarrel) && (layer >= 12)) xMin = bin0 + 3.5;
        if ((outcome == MuidElementNumbers::c_StopInForwardEndcap) && (layer >=  8)) xMin = bin0 + 1.5;
        if ((outcome == MuidElementNumbers::c_StopInForwardEndcap) && (layer >= 10)) xMin = bin0 + 2.5;
        if ((outcome == MuidElementNumbers::c_StopInBackwardEndcap) && (layer >=  8)) xMin = bin0 + 1.5;
        if ((outcome == MuidElementNumbers::c_StopInBackwardEndcap) && (layer >= 10)) xMin = bin0 + 2.5;
      } else if (isDeut) {
        p0 = 0.20; p0Min = 0.0002; p0Max = 0.26; p1 = 0.45; p1Min = 0.40; p1Max = 0.50;
        if ((outcome == MuidElementNumbers::c_StopInBarrel) && (layer <= 4)) { p0 = 0.00076; p1 = 0.745; }
        if ((outcome == MuidElementNumbers::c_StopInForwardEndcap) && (layer <= 5)) { p0 = 0.00027; p1 = 0.202; }
        if ((outcome == MuidElementNumbers::c_ExitForwardEndcap) && (layer <= 4)) { p0 = 0.03; p1 = 0.83; }
        if ((outcome == MuidElementNumbers::c_StopInBackwardEndcap) && (layer <= 4)) { p0 = 0.00021; p1 = 0.26; }
      } else if (isElec) {
        p0 = 0.035; p0Min = 0.005; p0Max = 0.25; p1 = 1.65; p1Min = 1.5; p1Max = 2.5;
        if ((outcome == MuidElementNumbers::c_ExitForwardEndcap) && (layer >= 11)) xMin = bin0 + 1.5;
        if ((outcome == MuidElementNumbers::c_ExitForwardEndcap) && (layer <= 10)) { p0 = 0.012; p1 = 1.186; }
        if ((outcome == MuidElementNumbers::c_StopInBackwardEndcap) && (layer <= 10)) { p0 = 0.010; p1 = 2.33; }
        if ((outcome == MuidElementNumbers::c_ExitBackwardEndcap) && (layer <= 10)) { p0 = 0.008; p1 = 1.94; }
      }
      double e0 = 0.0;
      double e1 = 0.0;
      printf(",B,%d,%d,%g,%g,%g,%g\n", outcome, layer, p0, 0.0, p1, 0.0);
      layerHitFunc->SetParameters(p0, p1, (double)bin0);
      layerHitFunc->SetParLimits(0, p0Min, p0Max);
      layerHitFunc->SetParLimits(1, p1Min, p1Max);
      layerHitFunc->FixParameter(2, (double)bin0);
      layerHitFunc->ReleaseParameter(1);
      if (int(xMax - xMin + 0.5) < layerHitFunc->GetNumberFreeParameters()) layerHitFunc->FixParameter(1, p1);
      if (int(xMax - xMin + 0.5) >= layerHitFunc->GetNumberFreeParameters()) {
        layerHitU[layer][outcome]->Fit("layerHitFunc", "WL", "", xMin, xMax);
        p0 = layerHitFunc->GetParameter(0);
        p1 = layerHitFunc->GetParameter(1);
        e0 = layerHitFunc->GetParError(0);
        e1 = layerHitFunc->GetParError(1);
        printf(",A,%d,%d,%g,%g,%g,%g\n", outcome, layer, p0, e0, p1, e1);
      }
      for (int bin = bin0 + 1; bin <= bin0 + 15; ++bin) {
        double y = layerHitU[layer][outcome]->GetBinContent(bin) * layerHitV[layer][outcome]->GetBinContent(bin);
        double yFit = p0 * std::exp(-p1 * (bin - 1 - bin0)) * layerHitV[layer][outcome]->GetBinContent(bin);
        if ((TMath::Sq(y - yFit) < 9.0 * (yFit + TMath::Max(1.0, y))) || (y <= 2.0)) {
          layerHitW[layer][outcome]->SetBinContent(bin, yFit);
          layerHitW[layer][outcome]->SetBinError(bin, yFit * TMath::Sqrt(TMath::Sq(e0 / p1) + TMath::Sq((bin - 1 - bin0)*e1)));
        }
      }
    }
  }
  for (int outcome = MuidElementNumbers::c_CrossBarrelStopInForwardMin; outcome < NOUTCOME; ++outcome) {
    for (int layer = 0; layer < MuidElementNumbers::getMaximalBarrelLayer() + 1; ++layer) {
      if ((outcome >= MuidElementNumbers::c_CrossBarrelStopInForwardMin)
          && (outcome <= MuidElementNumbers::c_CrossBarrelStopInForwardMax)
          && (layer > MuidElementNumbers::getMaximalEndcapForwardLayer() - 1)) continue; // like outcome == 2
      if ((outcome >= MuidElementNumbers::c_CrossBarrelStopInBackwardMin)
          && (outcome <= MuidElementNumbers::c_CrossBarrelStopInBackwardMax)
          && (layer > MuidElementNumbers::getMaximalEndcapBackwardLayer() - 1)) continue; // like outcome == 5
      if ((outcome >= MuidElementNumbers::c_CrossBarrelExitForwardMin) && (outcome <= MuidElementNumbers::c_CrossBarrelExitForwardMax)
          && (layer > MuidElementNumbers::getMaximalBarrelLayer() - 1)) continue; // like outcome == 4
      if ((outcome >= MuidElementNumbers::c_CrossBarrelExitBackwardMin) && (outcome <= MuidElementNumbers::c_CrossBarrelExitBackwardMax)
          && (layer > MuidElementNumbers::getMaximalEndcapBackwardLayer())) continue; // like outcome == 6
      // Smooth out layerHitW, avoid zeroes
      int elayer = layer;
      int blayer = outcome - 7;
      int outcomeRef = 2;
      if (outcome >= MuidElementNumbers::c_CrossBarrelStopInBackwardMin) { outcomeRef = MuidElementNumbers::c_StopInBackwardEndcap; blayer = outcome - MuidElementNumbers::c_CrossBarrelStopInBackwardMin; }
      if (outcome >= MuidElementNumbers::c_CrossBarrelExitForwardMin) { outcomeRef = MuidElementNumbers::c_ExitForwardEndcap; blayer = outcome - MuidElementNumbers::c_CrossBarrelExitForwardMin; }
      if (outcome >= MuidElementNumbers::c_CrossBarrelExitBackwardMin) { outcomeRef = MuidElementNumbers::c_ExitBackwardEndcap; blayer = outcome - MuidElementNumbers::c_CrossBarrelExitBackwardMin; }
      double wSum = 0.0;
      double vSum = 0.0;
      double wSumRef = 0.0;
      double vSumRef = 0.0;
      for (int bin = 1; bin <= blayer + 1; ++bin) {
        wSum += layerHitW[layer][outcome]->GetBinContent(bin);
        vSum += layerHitV[layer][outcome]->GetBinContent(bin);
        wSumRef += layerHitW[blayer][3]->GetBinContent(bin);
        vSumRef += layerHitV[blayer][3]->GetBinContent(bin);
      }
      if (wSum == 0.0) {
        wSum = 0.2;
        if (vSum == 0.0) vSum = wSum;
      }
      double factor = (wSum / vSum) / (wSumRef / vSumRef);
      for (int bin = 1; bin <= blayer + 1; ++bin) {
        double w = layerHitW[layer][outcome]->GetBinContent(bin);
        double v = layerHitV[layer][outcome]->GetBinContent(bin);
        double v1 = TMath::Max(v, 1.0);
        double wRef = layerHitW[blayer][3]->GetBinContent(bin);
        double vRef = layerHitV[blayer][3]->GetBinContent(bin);
        double wRef1 = TMath::Max(wRef, 1.0);
        double wNew = wRef * factor * (v1 / vRef);
        wNew = TMath::Min((isMuon ? 0.95 : 0.6667) * v1, wNew);
        double e = (w == 0.0) ? 1.0 : w;
        double eNew = factor * factor * wRef1 * v1 * (v1 * (vRef - wRef) + wRef * vRef) / (vRef * vRef * vRef);
        wNew = (w / e + wNew / eNew) / (1.0 / e + 1.0 / eNew);
        if (!isMuon && (bin > 1)) {
          wNew = TMath::Min(wNew, layerHitW[layer][outcome]->GetBinContent(bin - 1));
        }
        //if ((TMath::Sq(w - wNew) < 9.0 * (e + eNew)) || (w <= 2.0)) {
        layerHitW[layer][outcome]->SetBinContent(bin, wNew);
        //}
        printf("%d,%d,%d,%d,%g,%g,%g,%g,%g,%g,%g\n", outcome, layer, blayer, bin, factor, w, v, wRef, vRef, wNew,
               TMath::Sq(w - wNew) / (e + eNew));
      }
      wSum = 0.0;
      vSum = 0.0;
      wSumRef = 0.0;
      vSumRef = 0.0;
      for (int bin = 1; bin <= elayer + 1; ++bin) {
        wSum += layerHitW[layer][outcome]->GetBinContent(bin + 15);
        vSum += layerHitV[layer][outcome]->GetBinContent(bin + 15);
        wSumRef += layerHitW[layer][outcomeRef]->GetBinContent(bin + 15);
        vSumRef += layerHitV[layer][outcomeRef]->GetBinContent(bin + 15);
      }
      if (wSum <= 1.0) {
        factor *= (layerHitW[blayer][3]->GetBinContent(blayer + 1) / layerHitV[blayer][3]->GetBinContent(blayer + 1))
                  / (layerHitW[layer][outcomeRef]->GetBinContent(1 + 15) / layerHitV[layer][outcomeRef]->GetBinContent(1 + 15));
      } else {
        factor = (wSum / vSum) / (wSumRef / vSumRef);
      }
      for (int bin = 1; bin <= elayer + 1; ++bin) {
        double w = layerHitW[layer][outcome]->GetBinContent(bin + 15);
        double v = layerHitV[layer][outcome]->GetBinContent(bin + 15);
        double v1 = TMath::Max(v, 1.0);
        double wRef = layerHitW[layer][outcomeRef]->GetBinContent(bin + 15);
        double vRef = layerHitV[layer][outcomeRef]->GetBinContent(bin + 15);
        double wRef1 = TMath::Max(wRef, 1.0);
        double wNew = wRef * factor * (v1 / vRef);
        wNew = TMath::Min((isMuon ? 0.95 : 0.6667) * v1, wNew);
        double e = (w == 0.0) ? 1.0 : w;
        double eNew = factor * factor * wRef1 * v1 * (v1 * (vRef - wRef) + wRef * vRef) / (vRef * vRef * vRef);
        wNew = (w / e + wNew / eNew) / (1.0 / e + 1.0 / eNew);
        if (!isMuon && (bin > 1)) {
          wNew = TMath::Min(wNew, layerHitW[layer][outcome]->GetBinContent(bin + 15 - 1));
        }
        //if ((TMath::Sq(w - wNew) < 9.0 * (e + eNew)) || (w <= 2.0)) {
        layerHitW[layer][outcome]->SetBinContent(bin + 15, wNew);
        //}
        printf("%d,%d,%d,%d,%g,%g,%g,%g,%g,%g,%g\n", outcome, layer, blayer, bin, factor, w, v, wRef, vRef, wNew,
               TMath::Sq(w - wNew) / (e + eNew));
      }
    }
  }
  fprintf(output, "    <LayerProfile>\n");
  for (int outcome = 1; outcome < NOUTCOME; ++outcome) {
    fprintf(output, "      <!-- %s -->\n", outcomeComment[outcome]);
    fprintf(output, "      <Outcome outcome=\"%d\">\n", outcome);
    for (int layer = 0; layer < MuidElementNumbers::getMaximalBarrelLayer() + 1; ++layer) {
      if ((outcome == MuidElementNumbers::c_StopInBarrel) && (layer > MuidElementNumbers::getMaximalBarrelLayer() - 1)) continue;
      if ((outcome == MuidElementNumbers::c_StopInForwardEndcap)
          && (layer > MuidElementNumbers::getMaximalEndcapForwardLayer() - 1)) continue;
      if ((outcome == MuidElementNumbers::c_ExitBarrel) && (layer > MuidElementNumbers::getMaximalBarrelLayer())) continue;
      if ((outcome == MuidElementNumbers::c_ExitForwardEndcap) && (layer > MuidElementNumbers::getMaximalEndcapForwardLayer())) continue;
      if ((outcome == MuidElementNumbers::c_StopInBackwardEndcap)
          && (layer > MuidElementNumbers::getMaximalEndcapBackwardLayer() - 1)) continue;
      if ((outcome == MuidElementNumbers::c_ExitBackwardEndcap)
          && (layer > MuidElementNumbers::getMaximalEndcapBackwardLayer())) continue;
      if ((outcome >= MuidElementNumbers::c_CrossBarrelStopInForwardMin)
          && (outcome <= MuidElementNumbers::c_CrossBarrelStopInForwardMax)
          && (layer > MuidElementNumbers::getMaximalEndcapForwardLayer() - 1)) continue; // like outcome == 2
      if ((outcome >= MuidElementNumbers::c_CrossBarrelStopInBackwardMin)
          && (outcome <= MuidElementNumbers::c_CrossBarrelStopInBackwardMax)
          && (layer > MuidElementNumbers::getMaximalEndcapBackwardLayer() - 1)) continue; // like outcome == 5
      if ((outcome >= MuidElementNumbers::c_CrossBarrelExitForwardMin) && (outcome <= MuidElementNumbers::c_CrossBarrelExitForwardMax)
          && (layer > MuidElementNumbers::getMaximalEndcapForwardLayer())) continue; // like outcome == 4
      if ((outcome >= MuidElementNumbers::c_CrossBarrelExitBackwardMin) && (outcome <= MuidElementNumbers::c_CrossBarrelExitBackwardMax)
          && (layer > MuidElementNumbers::getMaximalEndcapBackwardLayer())) continue; // like outcome == 6
      int lastBin = layer + 1;
      if ((outcome != MuidElementNumbers::c_StopInBarrel)
          && (outcome != MuidElementNumbers::c_ExitBarrel)) lastBin += MuidElementNumbers::getMaximalBarrelLayer() + 1;
      if (layerHitV[layer][outcome]->GetEntries() > 0.0) {
        if (outcome >= MuidElementNumbers::c_CrossBarrelStopInForwardMin) layerHitU[layer][outcome]->Divide(layerHitV[layer][outcome]);
        layerHitW[layer][outcome]->Divide(layerHitV[layer][outcome]);
      }
      fprintf(output, "        <LastLayer layer=\"%d\">\n    ", layer);
      for (int bin = 1; bin <= lastBin; ++bin) {
        fprintf(output, "%16.8E", layerHitW[layer][outcome]->GetBinContent(bin));
        if ((bin % 8 == 0) && (bin != lastBin)) fprintf(output, "\n    ");
      }
      fprintf(output, "\n        </LastLayer>\n");
    }
    fprintf(output, "      </Outcome>\n");
  }
  fprintf(output, "    </LayerProfile>\n");

  // --------------------------------------------------------------------------

  int nBins = rchisqA[0][1]->GetNbinsX();
  double binWidth = rchisqA[0][1]->GetBinWidth(1);
  double lowEdge = rchisqA[0][1]->GetBinLowEdge(1);
  fprintf(output, "    <TransversePDF>\n");
  for (int detector = 0; detector <= 2; ++detector) {
    fprintf(output, "      <%s>\n", detectorName[detector]);
    double p1 = 1.0; // successful best-fit value will be inherited for next iteration of the ndof loop
    for (int halfNdof = 1; halfNdof <= 18; ++halfNdof) {
      //double p0Err = 0.0;
      //double p1Err = 0.0;
      fprintf(output, "        <DegreesOfFreedom ndof=\"%d\">\n", 2 * halfNdof);
      double entries = rchisqA[halfNdof][detector]->GetEntries();
      double p0 = entries * binWidth;
      if (entries == 0) {
        //int bin = (int)((1.0 - lowEdge) / binWidth + 0.5) + 1;
        p0 = 1.0 * binWidth;
        // DIVOT p1 = 1.0;
      }
      double p2 = (double)(halfNdof);
      rchisqFunc->SetParameters(p0, p1, p2);
      rchisqFunc->ReleaseParameter(0);
      rchisqFunc->ReleaseParameter(1);
      rchisqFunc->FixParameter(2, p2);
      int peakBin = rchisqA[halfNdof][detector]->GetMaximumBin();
      double xMin = 0.0;
      double xMax = lowEdge + binWidth * (peakBin + 10);
      double p0Left = p0;
      double p1Left = p1;
      if ((halfNdof > 1) && (entries > 0)) {
        rchisqA[halfNdof][detector]->Fit("rchisqFunc", "WL", "", xMin, xMax);
        p0Left = rchisqFunc->GetParameter(0);
        p1Left = rchisqFunc->GetParameter(1);
      }
      if (halfNdof == 2) {
        if (!isMuon) p0 = entries * (isElec ? 1.0 : 10.0);
        p1 = 1.4;
      }
      xMax = 10.0;
      if ((entries > 500.0) && (halfNdof <= 15)) {
        double peak   = rchisqA[halfNdof][detector]->GetBinContent(peakBin);
        //double peakX  = lowEdge + binWidth * peakBin;
        int    binLow = peakBin;
        double threshold = ((isMuon || isPion || isKaon) ? 0.1 : 0.2) * peak;
        if (halfNdof >= 10) { threshold *= 2.0; }
        for (int bin = peakBin; bin <= nBins; ++bin) {
          if (rchisqA[halfNdof][detector]->GetBinContent(bin) < threshold) break;
          binLow = bin - 2;
          if (binLow < 1) { binLow = 1; }
        }
        xMin = lowEdge + binWidth * binLow;
        /*
        if (isMuon) {
          if (halfNdof == 1) { xMin = 6.4; }
        } else {
          if (halfNdof == 1) { xMin = 8.0; }
          if (halfNdof == 2) { xMin = (isElec ? 3.8 : 5.4); }
          if (halfNdof == 3) { xMin = (isElec ? 2.4 : 4.0); }
          if (halfNdof == 4) { xMin = (isElec ? 0.0 : 3.4); }
        }
        */
        for (int bin = rchisqA[halfNdof][detector]->GetNbinsX(); bin > 0; --bin) {
          if (rchisqA[halfNdof][detector]->GetBinContent(bin) > 0.0) {
            int bin2 = bin - (33 - 3 * halfNdof);
            if (bin2 > bin - 2) { bin2 = bin - 2; }
            if (bin2 < peakBin) {
              xMin = 0.0;
            } else {
              double xMin2 = lowEdge + binWidth * bin2;
              if (xMin2 < xMin) {
                xMin = xMin2;
              }
            }
            break;
          }
        }
        if (isMuon) {
          if (halfNdof == 1) { xMin = 6.4; }
        } else {
          if (halfNdof == 1) { xMin = 8.0; }
          if (halfNdof == 2) { xMin = (isElec ? 3.8 : 4.6); }
          if (halfNdof == 3) { xMin = (isElec ? 2.4 : 4.0); }
          if (halfNdof == 4) { xMin = (isElec ? 0.0 : 3.4); }
        }
      }
      /*
      if (halfNdof == 1) {
        rchisqFunc->SetParLimits(1, 0.25, 3.0);
      } else {
        rchisqFunc->SetParLimits(1, 0.25, 2.0);
      }
      */
      if (entries < 50.0) {
        rchisqFunc->FixParameter(1, p1);
      }
      if ((entries > 0) && (int((xMax - xMin) / binWidth + 0.5) >= rchisqFunc->GetNumberFreeParameters())) {
        rchisqA[halfNdof][detector]->Fit("rchisqFunc", "WL", "", xMin, xMax);
        p0 = rchisqFunc->GetParameter(0);
        //p0Err = rchisqFunc->GetParError(0);
        p1 = rchisqFunc->GetParameter(1);
        //p1Err = rchisqFunc->GetParError(1);
      }

      double yNorm = 1.0;
      if ((entries > 0.0) && (rchisqA[halfNdof][detector]->GetEntries() > 0.0)) {
        yNorm = rchisqA[halfNdof][detector]->GetEntries() / entries;
      }
      double yScaleLeft = yNorm * p0Left * p2 / TMath::Gamma(p2);
      double yScale = yNorm * p0 * p2 / TMath::Gamma(p2);
      for (int bin = 1; bin <= nBins; ++bin) {
        double x = lowEdge + (bin - 0.5) * binWidth;
        double y = rchisqA[halfNdof][detector]->GetBinContent(bin) * yNorm;
        if ((rchisqA[halfNdof][detector]->GetBinContent(bin) == 0.0) || (x > xMin)) {
          if ((bin < peakBin) && (xMin > 0.0)) {
            x *= p1Left * p2;
            y = yScaleLeft * std::pow(x, p2 - 1.0) * std::exp(-x);
          } else {
            x *= p1 * p2;
            y = yScale * std::pow(x, p2 - 1.0) * std::exp(-x);
          }
        }
        rchisqB[halfNdof][detector]->SetBinContent(bin, y);
      }
      for (int bin = nBins + 1; bin <= 100; ++bin) {
        double x = (lowEdge + (bin - 0.5) * binWidth) * p1 * p2;
        double y = yScale * std::pow(x, p2 - 1.0) * std::exp(-x);
        rchisqB[halfNdof][detector]->SetBinContent(bin, y);
      }
      double sum = 0.0;
      for (int bin = 100; bin > 0; --bin) {
        sum += rchisqB[halfNdof][detector]->GetBinContent(bin);
      }
      sum *= binWidth;
      fprintf(output, "          <Tail>\n");
      fprintf(output, "            <Threshold>%16.7E</Threshold>\n", xMin);
      fprintf(output, "            <ScaleY>%16.7E</ScaleY>\n", yScale / sum);
      fprintf(output, "            <ScaleX>%16.7E</ScaleX>\n", p1);
      fprintf(output, "          </Tail>\n");
      fprintf(output, "          <Histogram>\n");
      for (int bin = 1; bin <= nBins; bin += 10) {
        fprintf(output, "         %16.7E%16.7E%16.7E%16.7E%16.7E%16.7E%16.7E%16.7E%16.7E%16.7E\n",
                std::log(rchisqB[halfNdof][detector]->GetBinContent(bin) / sum),
                std::log(rchisqB[halfNdof][detector]->GetBinContent(bin + 1) / sum),
                std::log(rchisqB[halfNdof][detector]->GetBinContent(bin + 2) / sum),
                std::log(rchisqB[halfNdof][detector]->GetBinContent(bin + 3) / sum),
                std::log(rchisqB[halfNdof][detector]->GetBinContent(bin + 4) / sum),
                std::log(rchisqB[halfNdof][detector]->GetBinContent(bin + 5) / sum),
                std::log(rchisqB[halfNdof][detector]->GetBinContent(bin + 6) / sum),
                std::log(rchisqB[halfNdof][detector]->GetBinContent(bin + 7) / sum),
                std::log(rchisqB[halfNdof][detector]->GetBinContent(bin + 8) / sum),
                std::log(rchisqB[halfNdof][detector]->GetBinContent(bin + 9) / sum));
      }
      fprintf(output, "          </Histogram>\n");
      fprintf(output, "        </DegreesOfFreedom>\n");
    }
    fprintf(output, "      </%s>\n", detectorName[detector]);
  }
  fprintf(output, "    </TransversePDF>\n");
  fprintf(output, "  </%s>\n", xmllabel);

  //---------------------------------------------------------------------

  TCanvas* canvas = new TCanvas("tree", label, 900, 900);
  //gStyle->SetPaperSize(1); // BAD BAD BAD!
  gStyle->SetOptStat(10);
  gStyle->SetOptFit(0);
  canvas->GetPad(0)->SetLogy(0);
  canvas->GetPad(0)->SetGrid(1, 1);
  canvas->Clear();
  canvas->Divide(1, 1);
  canvas->GetPad(1)->SetLogy(1);
  sprintf(label, "Muid-%s.pdf[", xmllabel); // open file without drawing
  canvas->SaveAs(label);
  sprintf(label, "Muid-%s.pdf", xmllabel); // save each canvas to same file
  canvas->cd(1);
  discardA->Divide(discardB);
  discardA->SetLineColor(kRed);
  discardA->Draw("");
  canvas->SaveAs(label);
  for (int outcome = 1; outcome < NOUTCOME; ++outcome) {
    for (int layer = 0; layer < MuidElementNumbers::getMaximalBarrelLayer() + 1; ++layer) {
      canvas->cd(1);
      layerHitU[layer][outcome]->SetMaximum(2.0);
      layerHitU[layer][outcome]->SetMinimum(1.0E-4);
      if (isProt or isDeut) layerHitU[layer][outcome]->SetMinimum(1.0E-6);
      if (isElec) layerHitU[layer][outcome]->SetMinimum(1.0E-10);
      layerHitU[layer][outcome]->GetXaxis()->SetTitle("KLM Layer");
      layerHitU[layer][outcome]->GetYaxis()->SetTitle("Frequency");
      layerHitU[layer][outcome]->Draw("E1");
      layerHitW[layer][outcome]->SetLineColor(kMagenta);
      layerHitW[layer][outcome]->Draw("HIST,SAME");
      layerHitU[layer][outcome]->Draw("E1,SAME");
      canvas->SaveAs(label);
    }
  }

  canvas->Clear();
  canvas->Divide(1, 2);
  canvas->GetPad(1)->SetLogy(1);
  canvas->GetPad(2)->SetLogy(1);
  gStyle->SetOptStat(110);
  gStyle->SetOptFit(2);
  for (int detector = 0; detector <= 2; ++detector) {
    for (int halfNdof = 1; halfNdof <= 18; ++halfNdof) {
      int padNo = (halfNdof - 1) % 2 + 1;
      canvas->cd(padNo);
      gPad->Clear();
      rchisqA[halfNdof][detector]->SetMinimum(0.001); // for vertical log axis
      if (rchisqA[halfNdof][detector]->GetEntries() == 0.0) { rchisqA[halfNdof][detector]->SetMaximum(2.0); }
      rchisqA[halfNdof][detector]->GetXaxis()->SetTitle("#chi^{2}/ndf");
      rchisqA[halfNdof][detector]->GetYaxis()->SetTitle("Frequency");
      rchisqA[halfNdof][detector]->Draw("");
      rchisqB[halfNdof][detector]->SetLineColor(kMagenta);
      rchisqB[halfNdof][detector]->Draw("SAME");
      rchisqA[halfNdof][detector]->Draw("SAME,E1");
      if (padNo == 2) {
        canvas->SaveAs(label);
      }
    }
  }

  sprintf(label, "Muid-%s.pdf]", xmllabel); // close file without drawing
  canvas->SaveAs(label);

  printf("Done.\n");

}

int main(int argc, char** argv)
{
  TChain* chain;
  FILE* output;
  if (argc < 4)
    goto badUsage;
  chain = new TChain;
  chain->Add(argv[1]);
  output = std::fopen(argv[2], "a");
  makeprob(chain, output, argv[3]);
  std::fclose(output);
  delete chain;
  return 0;
badUsage:
  std::printf(
    "Usage:\n"
    "b2klm-likelihood-parameters input_file output_file xml_label\n");
  return -1;
}
