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
  for (int i = 0; i < 9; i++) {
    h_prodvtx[i] = new TH2F(TString::Format("h_prodvtx_%d", i), "", 200, -400., 400., 200, -400., -400.);
    h_decavtx[i] = new TH2F(TString::Format("h_decavtx_%d", i), "", 200, -400., 400., 200, -400., -400.);
    h_kineticvz[i] = new TH2F(TString::Format("h_kineticvz_%d", i), "", 200, -400., 400., 1000, 0., 10.);
    h_phivz[i] = new TH2F(TString::Format("h_phivz_%d", i), "", 200, -400., 400., 360, -180., 180.);
    h_thetavz[i] = new TH2F(TString::Format("h_thetavz_%d", i), "", 200, -400., 400., 180, 0., 180.);
  }
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
    int partID[8] = {0, 0, 0, 0, 0, 0, 0, 0};

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
        h_thetavz[i]->Fill(prodvtx[2], theta);
        h_phivz[i]->Fill(prodvtx[2], phi);
      }
    }
    //if(Mother == 1) {
    h_count->Fill(9);
    h_prodvtx[8]->Fill(prodvtx[2], prodr);
    h_decavtx[8]->Fill(decavtx[2], decar);
    h_kineticvz[8]->Fill(prodvtx[2], Kinetic);
    h_thetavz[8]->Fill(prodvtx[2], theta);
    h_phivz[8]->Fill(prodvtx[2], phi);
    //}
  }

}

void AnalysisPhase1StudyModule::endRun()
{


}

void AnalysisPhase1StudyModule::terminate()
{
}


