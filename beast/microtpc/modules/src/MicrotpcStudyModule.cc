/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/microtpc/modules/MicrotpcStudyModule.h>
#include <beast/microtpc/dataobjects/MicrotpcSimHit.h>
#include <beast/microtpc/dataobjects/MicrotpcHit.h>
#include <beast/microtpc/dataobjects/MicrotpcRecoTrack.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
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

int eventNum = 0;

using namespace std;

using namespace Belle2;
using namespace microtpc;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MicrotpcStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MicrotpcStudyModule::MicrotpcStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for Microtpcs (BEAST)");

}

MicrotpcStudyModule::~MicrotpcStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void MicrotpcStudyModule::defineHisto()
{
  for (int i = 0 ; i < 8 ; i++) {
    h_evtrl[i] = new TH2F(TString::Format("h_evtrl_%d", i), "Deposited energy [keV] v. track length [cm]", 2000, 0., 2000, 200, 0., 6.);
    h_evtrl_p[i] = new TH2F(TString::Format("h_evtrl_p_%d", i), "Deposited energy [keV] v. track length [cm]", 2000, 0., 2000, 200, 0.,
                            6.);
    //h_evtrl_x[i] = new TH2F(TString::Format("h_evtrl_x_%d",i),"Deposited energy [keV] v. track length [cm]",2000,0.,2000,200,0.,6.);
    h_evtrl_Hex[i] = new TH2F(TString::Format("h_evtrl_Hex_%d", i), "Deposited energy [keV] v. track length [cm]", 2000, 0., 2000, 200,
                              0., 6.);
    h_evtrl_He[i] = new TH2F(TString::Format("h_evtrl_He_%d", i), "Deposited energy [keV] v. track length [cm]", 2000, 0., 2000, 200,
                             0., 6.);
    h_evtrl_C[i] = new TH2F(TString::Format("h_evtrl_C_%d", i), "Deposited energy [keV] v. track length [cm]", 2000, 0., 2000, 200, 0.,
                            6.);
    h_evtrl_O[i] = new TH2F(TString::Format("h_evtrl_O_%d", i), "Deposited energy [keV] v. track length [cm]", 2000, 0., 2000, 200, 0.,
                            6.);
    h_evtrl_He_pure[i] = new TH2F(TString::Format("h_evtrl_He_pure_%d", i), "Deposited energy [keV] v. track length [cm]", 2000, 0.,
                                  2000, 200, 0., 6.);

    h_tvp[i]  = new TH2F(TString::Format("h_tvp_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvp[i]  = new TH2F(TString::Format("h_wtvp_%d", i), "Phi [deg] v. theta [deg] - weighted", 180, 0., 180, 360, -180., 180.);
    h_tvp_p[i]  = new TH2F(TString::Format("h_tvp_p_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvp_p[i]  = new TH2F(TString::Format("h_wtvp_p_%d", i), "Phi [deg] v. theta [deg] - weighted", 180, 0., 180, 360, -180., 180.);
    h_tvp_He[i]  = new TH2F(TString::Format("h_tvp_He_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvp_He[i]  = new TH2F(TString::Format("h_wtvp_He_%d", i), "Phi [deg] v. theta [deg] - weighted", 180, 0., 180, 360, -180., 180.);
    h_tvp_Hex[i]  = new TH2F(TString::Format("h_tvp_Hex_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvp_Hex[i]  = new TH2F(TString::Format("h_wtvp_Hex_%d", i), "Phi [deg] v. theta [deg] - weighted", 180, 0., 180, 360, -180.,
                              180.);
    h_tvp_He_pure[i]  = new TH2F(TString::Format("h_tvp_He_pure_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvp_He_pure[i]  = new TH2F(TString::Format("h_wtvp_He_pure_%d", i), "Phi [deg] v. theta [deg] - weighted", 180, 0., 180, 360,
                                  -180., 180.);
    h_tvp_C[i]  = new TH2F(TString::Format("h_tvp_C_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvp_C[i]  = new TH2F(TString::Format("h_wtvp_C_%d", i), "Phi [deg] v. theta [deg] - weighted", 180, 0., 180, 360, -180., 180.);
    h_tvp_O[i]  = new TH2F(TString::Format("h_tvp_O_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvp_O[i]  = new TH2F(TString::Format("h_wtvp_O_%d", i), "Phi [deg] v. theta [deg] - weighted", 180, 0., 180, 360, -180., 180.);
  }

}


void MicrotpcStudyModule::initialize()
{
  B2INFO("MicrotpcStudyModule: Initialize");

  REG_HISTOGRAM

  //convert sample time into rate in Hz
  //rateCorrection = m_sampletime / 1e6;
}

void MicrotpcStudyModule::beginRun()
{
}

void MicrotpcStudyModule::event()
{
  //Here comes the actual event processing

  StoreArray<MicrotpcSimHit>  SimHits;
  StoreArray<MicrotpcHit> Hits;
  StoreArray<MicrotpcRecoTrack> Tracks;

  //Initialize recoil and hit type counters
  bool xRec[8];
  bool pRec[8];
  bool HeRec[8];
  bool CRec[8];
  bool ORec[8];
  for (int i = 0; i < 8; i++) {
    xRec[i] = false;
    pRec[i] = false;
    HeRec[i] = false;
    ORec[i] = false;
    CRec[i] = false;
  }

  //number of entries in SimHit
  int nSimHits = SimHits.getEntries();

  //loop over all SimHit entries
  for (int i = 0; i < nSimHits; i++) {
    MicrotpcSimHit* aHit = SimHits[i];
    int detNb = aHit->getdetNb();
    int PDGid = aHit->gettkPDG();
    cout << "detNb " << detNb << " pdg " << PDGid << endl;
    if (fabs(PDGid) == 11 || PDGid == 22) xRec[detNb] = true;
    if (PDGid == 2212) pRec[detNb] = true;
    if (PDGid == 1000020040) HeRec[detNb] = true;
    if (PDGid == 1000060120) ORec[detNb] = true;
    if (PDGid == 1000080160) CRec[detNb] = true;
  }

  //number of Tracks
  int nTracks = Tracks.getEntries();

  //loop over all Tracks
  for (int i = 0; i < nTracks;  i++) {
    MicrotpcRecoTrack* aTrack = Tracks[i];
    int detNb = aTrack->getdetNb();
    float phi = aTrack->getphi();
    float theta = aTrack->gettheta();
    float trl = aTrack->gettrl();
    float esum = aTrack->getesum();
    //int time_range = aTrack->gettime_range();
    int side[5][4];
    side[0][4] = {aTrack->getside()[0]};
    side[1][4] = {aTrack->getside()[1]};
    side[2][4] = {aTrack->getside()[2]};
    side[3][4] = {aTrack->getside()[3]};
    side[4][4] = {aTrack->getside()[4]};

    h_evtrl[detNb]->Fill(esum, trl);
    h_tvp[detNb]->Fill(theta, phi);
    h_wtvp[detNb]->Fill(theta, phi, esum);
    if (pRec[detNb]) {
      h_evtrl_p[detNb]->Fill(esum, trl);
      h_tvp_p[detNb]->Fill(theta, phi);
      h_wtvp_p[detNb]->Fill(theta, phi, esum);
    }
    if (HeRec[detNb]) {
      h_evtrl_Hex[detNb]->Fill(esum, trl);
      h_tvp_Hex[detNb]->Fill(theta, phi);
      h_wtvp_Hex[detNb]->Fill(theta, phi, esum);
    }
    if (ORec[detNb]) {
      h_evtrl_O[detNb]->Fill(esum, trl);
      h_tvp_O[detNb]->Fill(theta, phi);
      h_wtvp_O[detNb]->Fill(theta, phi, esum);
    }
    if (CRec[detNb]) {
      h_evtrl_C[detNb]->Fill(esum, trl);
      h_tvp_C[detNb]->Fill(theta, phi);
      h_wtvp_C[detNb]->Fill(theta, phi, esum);
    }
    if (HeRec[detNb] && !xRec[detNb]) {
      h_evtrl_He[detNb]->Fill(esum, trl);
      h_tvp_He[detNb]->Fill(theta, phi);
      h_wtvp_He[detNb]->Fill(theta, phi, esum);
      if (side[0][0] == 1 && side[0][1] == 1 && side[0][2] == 2 && side[0][3] == 3) {
        h_evtrl_He_pure[detNb]->Fill(esum, trl);
        h_tvp_He_pure[detNb]->Fill(theta, phi);
        h_wtvp_He_pure[detNb]->Fill(theta, phi, esum);
      }
    }

  }

  eventNum++;




}

void MicrotpcStudyModule::endRun()
{

  //B2RESULT("MicrotpcStudyModule: # of p recoils: " << npHits);
  //B2RESULT("MicrotpcStudyModule: # of He recoils: " << nHeHits);
  //B2RESULT("MicrotpcStudyModule: # of O recoils: " << nOHits);
  //B2RESULT("MicrotpcStudyModule: # of C recoils: " << nCHits);

}

void MicrotpcStudyModule::terminate()
{
}


