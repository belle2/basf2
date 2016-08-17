/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/analysis/modules/AnalysisPhase1StudyModule.h>
#include <mdst/dataobjects/MCParticle.h>
#include <generators/SAD/dataobjects/SADMetaHit.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <cmath>
#include <boost/foreach.hpp>


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// ROOT
#include <TVector3.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>

using namespace std;

using namespace Belle2;

//using namespace analysis;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(AnalysisPhase1Study)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

AnalysisPhase1StudyModule::AnalysisPhase1StudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for BEAST");

}

AnalysisPhase1StudyModule::~AnalysisPhase1StudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void AnalysisPhase1StudyModule::defineHisto()
{
  h_count = new TH1F("h_count", "", 10, 0., 10.);
  for (int i = 0; i < 10; i++) {
    h_prodvtx[i] = new TH2F(TString::Format("h_prodvtx_%d", i), "", 200, -400., 400., 200, 0., 400.);
    h_decavtx[i] = new TH2F(TString::Format("h_decavtx_%d", i), "", 200, -400., 400., 200, 0., 400.);
    h_kineticvz[i] = new TH2F(TString::Format("h_kineticvz_%d", i), "", 200, -400., 400., 1000, 0., 10.);
    h_kineticvz_zoom[i] = new TH2F(TString::Format("h_kineticvz_zoom_%d", i), "", 200, -400., 400., 1000, 0., 0.01);
    h_phivz[i] = new TH2F(TString::Format("h_phivz_%d", i), "", 200, -400., 400., 360, -180., 180.);
    h_thetavz[i] = new TH2F(TString::Format("h_thetavz_%d", i), "", 200, -400., 400., 180, 0., 180.);
  }
  h_g4_xy = new TH2F("h_g4_xy", "", 100, -5.99, 5.99, 100, -5.99, 5.99);
  for (int i = 0; i < 2; i++) {
    h_sad_xy[i] = new TH2F(TString::Format("h_sad_xy_%d", i), "", 100, -5.99, 5.99, 100, -5.99, 5.99);
    h_sad_sir[i] = new TH1F(TString::Format("h_sad_sir_%d", i), "", 100, -3.99, 3.99);
    h_sad_sall[i] = new TH1F(TString::Format("h_sad_sall_%d", i), "", 100, -1499.99, 1499.99);
    h_sad_sE[i] = new TH2F(TString::Format("h_sad_sE_%d", i), "", 100, -3.99, 3.99, 1000, 0., 10.);
    h_sad_sraw[i] = new TH1F(TString::Format("h_sad_sraw_%d", i), "", 100, -1499.99, 1499.99);
    h_sad_E[i] = new TH1F(TString::Format("h_sad_E_%d", i), "", 8000, 0, 7.99);
    h_sad_s[i] = new TH1F(TString::Format("h_sad_s_%d", i), "", 400, -3.99, 3.99);
  }
  h_dpx = new TH1F("h_dpx", "", 1000, -1., 1.);
  h_dpy = new TH1F("h_dpy", "", 1000, -1., 1.);
  h_dE = new TH1F("h_dE", "", 1000, -1., 1.);
  h_px = new TH1F("h_px", "", 10000, -10., 10.);
  h_py = new TH1F("h_py", "", 10000, -10., 10.);
  h_pz = new TH1F("h_pz", "", 10000, -10., 10.);
  h_dx = new TH1F("h_dx", "", 10000, -400., 400.);
  h_dy = new TH1F("h_dy", "", 10000, -400., 400.);
  h_dz = new TH1F("h_dz", "", 10000, -400., 400.);
  h_E = new TH1F("h_E", "", 8000, 0., 7.99);
  h_P = new TH1F("h_P", "", 8000, 0., 7.99);
}

void AnalysisPhase1StudyModule::initialize()
{
  B2INFO("AnalysisPhase1StudyModule: Initialize");

  REG_HISTOGRAM

}

void AnalysisPhase1StudyModule::beginRun()
{
}

void AnalysisPhase1StudyModule::event()
{
  double px = 0;
  double py = 0;
  double x = 0;
  double y = 0;
  double s = 0;
  //double xraw = 0;
  //double yraw = 0;
  double sraw = 0;
  double E = 0;
  double rate = 0;
  StoreArray<SADMetaHit> sadMetaHits;
  for (const auto& sadMetaHit : sadMetaHits) {
    px = sadMetaHit.getpx();
    py = sadMetaHit.getpy();
    x = sadMetaHit.getx();
    y = sadMetaHit.gety();
    s = sadMetaHit.gets();
    //xraw = sadMetaHit.getxraw();
    //yraw = sadMetaHit.getyraw();
    sraw = sadMetaHit.getsraw();
    E = sadMetaHit.getE();
    rate = sadMetaHit.getrate();
    h_sad_sir[0]->Fill(s / 100., rate);
    h_sad_sall[0]->Fill(s / 100., rate);
    h_sad_sE[0]->Fill(s / 100., E);
    h_sad_s[0]->Fill(s / 100.);
    h_sad_E[0]->Fill(E);
    h_sad_sraw[0]->Fill(sraw / 100., rate);
    if (-400. < s && s < 400.) {
      h_sad_xy[1]->Fill(x, y);
      h_sad_sir[1]->Fill(s / 100., rate);
      h_sad_sall[1]->Fill(s / 100., rate);
      h_sad_sraw[1]->Fill(sraw / 100., rate);
      h_sad_sE[1]->Fill(s / 100., E);
      h_sad_s[1]->Fill(s / 100.);
      h_sad_E[1]->Fill(E);
    }
  }
  int counter = 0;
  StoreArray<MCParticle> mcParticles;
  for (const auto& mcParticle : mcParticles) { // start loop over all MC particles
    int PDG = mcParticle.getPDG();
    //int* Mother = mcParticle.getMother();
    float Mass = mcParticle.getMass();
    float Energy = mcParticle.getEnergy();
    float Kinetic = Energy - Mass;
    float prodvtx[3];
    prodvtx[0] = mcParticle.getProductionVertex().X();
    prodvtx[1] = mcParticle.getProductionVertex().Y();
    prodvtx[2] = mcParticle.getProductionVertex().Z();
    float prodr = sqrt(prodvtx[0] * prodvtx[0] + prodvtx[1] * prodvtx[1]);
    float decavtx[3];
    decavtx[0] = mcParticle.getDecayVertex().X();
    decavtx[1] = mcParticle.getDecayVertex().Y();
    decavtx[2] = mcParticle.getDecayVertex().Z();
    float decar = sqrt(decavtx[0] * decavtx[0] + decavtx[1] * decavtx[1]);
    float mom[3];
    mom[0] = mcParticle.getMomentum().X();
    mom[1] = mcParticle.getMomentum().Y();
    mom[2] = mcParticle.getMomentum().Z();
    float theta = mcParticle.getMomentum().Theta() * TMath::RadToDeg();
    float phi = mcParticle.getMomentum().Phi() * TMath::RadToDeg();
    int partID[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    if (PDG == 11) partID[0] = 1; //positron
    else if (PDG == -11) partID[1] = 1; //electron
    else if (PDG == 22) partID[2] = 1; //photon
    else if (PDG == 2112) partID[3] = 1; //neutron
    else if (PDG == 2212) partID[4] = 1; //proton
    else if (PDG == 1000080160) partID[5] = 1; // O
    else if (PDG == 1000060120) partID[6] = 1; // C
    else if (PDG == 1000020040) partID[7] = 1; // He
    else partID[8] = 1;
    for (int i = 0; i < 8; i++) {
      if (partID[i] == 1) {
        h_count->Fill(i);
        h_prodvtx[i]->Fill(prodvtx[2], prodr);
        h_decavtx[i]->Fill(decavtx[2], decar);
        h_kineticvz[i]->Fill(prodvtx[2], Kinetic);
        h_kineticvz_zoom[i]->Fill(prodvtx[2], Kinetic);
        h_thetavz[i]->Fill(prodvtx[2], theta);
        h_phivz[i]->Fill(prodvtx[2], phi);
      }
    }

    //h_count->Fill(9);
    h_prodvtx[8]->Fill(prodvtx[2], prodr);
    h_decavtx[8]->Fill(decavtx[2], decar);
    h_kineticvz[8]->Fill(prodvtx[2], Kinetic);
    h_kineticvz_zoom[8]->Fill(prodvtx[2], Kinetic);
    h_thetavz[8]->Fill(prodvtx[2], theta);
    h_phivz[8]->Fill(prodvtx[2], phi);

    if (counter == 0) {
      h_count->Fill(9);
      h_prodvtx[9]->Fill(prodvtx[2], prodr);
      h_decavtx[9]->Fill(decavtx[2], decar);
      h_kineticvz[9]->Fill(prodvtx[2], Kinetic);
      h_kineticvz_zoom[9]->Fill(prodvtx[2], Kinetic);
      h_thetavz[9]->Fill(prodvtx[2], theta);
      h_phivz[9]->Fill(prodvtx[2], phi);

      h_dx->Fill(x - prodvtx[0]);
      h_dy->Fill(-y - prodvtx[1]);
      h_dz->Fill(-s - prodvtx[2]);

      h_dpx->Fill(px - mom[0]);
      h_dpy->Fill(-py - mom[1]);
      h_dE->Fill(E - Energy);

      h_g4_xy->Fill(prodvtx[0], prodvtx[1]);

      h_px->Fill(mom[0]);
      h_py->Fill(mom[1]);
      h_pz->Fill(mom[2]);
      h_E->Fill(Kinetic);
      h_P->Fill(sqrt(mom[0] * mom[0] + mom[1] * mom[1] + mom[2] * mom[2]));
    }
    counter++;
  }

}

void AnalysisPhase1StudyModule::endRun()
{


}

void AnalysisPhase1StudyModule::terminate()
{
}


