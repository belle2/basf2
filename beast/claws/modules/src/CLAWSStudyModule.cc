/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/claws/modules/CLAWSStudyModule.h>
#include <beast/claws/dataobjects/CLAWSSimHit.h>
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
using namespace claws;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CLAWSStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CLAWSStudyModule::CLAWSStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for Claws (BEAST)");

}

CLAWSStudyModule::~CLAWSStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void CLAWSStudyModule::defineHisto()
{
  h_time = new TH2F("h_time", "Detector # vs. time", 20, 0., 20., 1000, 0., 750.);
  h_time->Sumw2();
  h_timeWeighted = new TH2F("h_timeWeigthed", "Detector # vs. time weighted by the energy deposited", 20, 0., 20., 1000, 0., 750.);
  h_timeWeighted->Sumw2();
  h_timeThres = new TH2F("h_timeThres", "Detector # vs. time", 20, 0., 20., 750, 0., 750.);
  h_timeThres->Sumw2();
  h_timeWeightedThres = new TH2F("h_timeWeigthedThres", "Detector # vs. time weighted by the energy deposited", 20, 0., 20., 750, 0.,
                                 750.);
  h_timeWeightedThres->Sumw2();
  h_edep = new TH2F("h_edep", "Time bin # vs. energy deposited", 20, 0., 20., 1000, 0., 10.);
  h_edep->Sumw2();
  h_edep1 = new TH2F("h_edep1", "Time bin # vs. energy deposited", 20, 0., 20., 1000, 0., 10.);
  h_edep1->Sumw2();
  h_edep2 = new TH2F("h_edep2", "Time bin # vs. energy deposited", 20, 0., 20., 1000, 0., 10.);
  h_edep2->Sumw2();
  h_edep3 = new TH2F("h_edep3", "Time bin # vs. energy deposited", 20, 0., 20., 1000, 0., 10.);
  h_edep3->Sumw2();

  h_edepThres = new TH2F("h_edepThres", "Time bin # vs. energy deposited", 20, 0., 20., 1000, 0., 10.);
  h_edepThres->Sumw2();
  h_edepThres1 = new TH2F("h_edepThres1", "Time bin # vs. energy deposited", 20, 0., 20., 1000, 0., 10.);
  h_edepThres1->Sumw2();
  h_edepThres2 = new TH2F("h_edepThres2", "Time bin # vs. energy deposited", 20, 0., 20., 1000, 0., 10.);
  h_edepThres2->Sumw2();
  h_edepThres3 = new TH2F("h_edepThres3", "Time bin # vs. energy deposited", 20, 0., 20., 1000, 0., 10.);
  h_edepThres3->Sumw2();
  for (int i = 0; i < 2; i++) {
    h_zvedep[i] = new TH1F(TString::Format("h_zvedep_%d", i) , "edep [MeV] vs. z [cm]", 2000, -25., 25.);
    h_zvedep[i]->Sumw2();

    h_xvzvedep[i] = new TH2F(TString::Format("h_xvzvedep_%d", i) , "edep [MeV] vs. z [cm]", 2000, -25., 25., 2000, -25., 25.);
    h_xvzvedep[i]->Sumw2();

    h_yvzvedep[i] = new TH2F(TString::Format("h_yvzvedep_%d", i) , "edep [MeV] vs. z [cm]", 2000, -25., 25., 2000, -25., 25.);
    h_yvzvedep[i]->Sumw2();

    h_rvzvedep[i] = new TH2F(TString::Format("h_rvzvedep_%d", i) , "edep [MeV] vs. z [cm]", 2000, 0., 25., 2000, -25., 25.);
    h_rvzvedep[i]->Sumw2();

    h_xvyvedep[i] = new TH2F(TString::Format("h_xvyvedep_%d", i) , "edep [MeV] vs. y [cm]", 2000, -25., 25., 2000, -25., 25.);
    h_xvyvedep[i]->Sumw2();

    h_zvedepW[i] = new TH1F(TString::Format("h_zvedepW_%d", i) , "edep [MeV] vs. z [cm]", 2000, -25., 25.);
    h_zvedepW[i]->Sumw2();

    h_xvzvedepW[i] = new TH2F(TString::Format("h_xvzvedepW_%d", i) , "edep [MeV] vs. x vs. z [cm]", 2000, -25., 25., 2000, -25., 25.);
    h_xvzvedepW[i]->Sumw2();

    h_yvzvedepW[i] = new TH2F(TString::Format("h_yvzvedepW_%d", i) , "edep [MeV] vs. y vs. z [cm]", 2000, -25., 25., 2000, -25., 25.);
    h_yvzvedepW[i]->Sumw2();

    h_xvyvedepW[i] = new TH2F(TString::Format("h_xvyvedepW_%d", i) , "edep [MeV] vs. x vs. y [cm]", 2000, -25., 25., 2000, -25., 25.);
    h_xvyvedepW[i]->Sumw2();

    h_rvzvedepW[i] = new TH2F(TString::Format("h_rvzvedepW_%d", i) , "edep [MeV] vs. z [cm]", 2000, 0., 25., 2000, -25., 25.);
    h_rvzvedepW[i]->Sumw2();


    h_zvedepT[i] = new TH1F(TString::Format("h_zvedepT_%d", i) , "edep [MeV] vs. z [cm]", 2000, -25., 25.);
    h_zvedepT[i]->Sumw2();

    h_xvzvedepT[i] = new TH2F(TString::Format("h_xvzvedepT_%d", i) , "edep [MeV] vs. z [cm]", 2000, -25., 25., 2000, -25., 25.);
    h_xvzvedepT[i]->Sumw2();

    h_yvzvedepT[i] = new TH2F(TString::Format("h_yvzvedepT_%d", i) , "edep [MeV] vs. z [cm]", 2000, -25., 25., 2000, -25., 25.);
    h_yvzvedepT[i]->Sumw2();

    h_rvzvedepT[i] = new TH2F(TString::Format("h_rvzvedepT_%d", i) , "edep [MeV] vs. z [cm]", 2000, 0., 25., 2000, -25., 25.);
    h_rvzvedepT[i]->Sumw2();

    h_xvyvedepT[i] = new TH2F(TString::Format("h_xvyvedepT_%d", i) , "edep [MeV] vs. y [cm]", 2000, -25., 25., 2000, -25., 25.);
    h_xvyvedepT[i]->Sumw2();

    h_zvedepWT[i] = new TH1F(TString::Format("h_zvedepWT_%d", i) , "edep [MeV] vs. z [cm]", 2000, -25., 25.);
    h_zvedepWT[i]->Sumw2();

    h_xvzvedepWT[i] = new TH2F(TString::Format("h_xvzvedepWT_%d", i) , "edep [MeV] vs. x vs. z [cm]", 2000, -25., 25., 2000, -25., 25.);
    h_xvzvedepWT[i]->Sumw2();

    h_yvzvedepWT[i] = new TH2F(TString::Format("h_yvzvedepWT_%d", i) , "edep [MeV] vs. y vs. z [cm]", 2000, -25., 25., 2000, -25., 25.);
    h_yvzvedepWT[i]->Sumw2();

    h_xvyvedepWT[i] = new TH2F(TString::Format("h_xvyvedepWT_%d", i) , "edep [MeV] vs. x vs. y [cm]", 2000, -25., 25., 2000, -25., 25.);
    h_xvyvedepWT[i]->Sumw2();

    h_rvzvedepWT[i] = new TH2F(TString::Format("h_rvzvedepWT_%d", i) , "edep [MeV] vs. z [cm]", 2000, 0., 25., 2000, -25., 25.);
    h_rvzvedepWT[i]->Sumw2();
  }
}


void CLAWSStudyModule::initialize()
{
  B2INFO("CLAWSStudyModule: Initialize");

  REG_HISTOGRAM

  //convert sample time into rate in Hz
  //rateCorrection = m_sampletime / 1e6;
}

void CLAWSStudyModule::beginRun()
{
}

void CLAWSStudyModule::event()
{
  //Here comes the actual event processing

  StoreArray<CLAWSSimHit>  SimHits;

  //number of entries in SimHits
  int nSimHits = SimHits.getEntries();
  //cout << nSimHits << endl;

  //loop over all SimHit entries
  for (int i = 0; i < nSimHits; i++) {
    CLAWSSimHit* aHit = SimHits[i];
    int lad = aHit->getLadder();
    int sen = aHit->getSensor();
    double adep = aHit->getEnergyVisible();
    double timeBin = aHit->getTime();
    int  pdg = aHit->getPDG();

    TVector3 position = aHit->getPosEntry();
    double r = sqrt(position.X() * position.X() + position.Y() * position.Y());
    int detNB = (lad - 1) * 8 + sen - 1;
    //cout <<" lad " << lad << " sen " << sen << " detNB " << detNB << " time " << timeBin << " edep " << adep*1e3 << endl;
    //cout <<" x " << position.X()  << " y " << position.Y() << " z " << position.Z()  << endl;
    h_time->Fill(detNB, timeBin);
    h_edep->Fill(detNB, adep * 1e3);
    if (fabs(pdg) == 11)h_edep1->Fill(detNB, adep * 1e3);
    if (pdg == 22)h_edep2->Fill(detNB, adep * 1e3);
    if (pdg != 22 && fabs(pdg) != 11)h_edep3->Fill(detNB, adep * 1e3);
    if (adep > 50.*1e-6) {
      h_timeThres->Fill(detNB, timeBin);
      h_edepThres->Fill(detNB, adep * 1e3);
      if (fabs(pdg) == 11)h_edepThres1->Fill(detNB, adep * 1e3);
      if (pdg == 22)h_edepThres2->Fill(detNB, adep * 1e3);
      if (pdg != 22 && fabs(pdg) != 11)h_edepThres3->Fill(detNB, adep * 1e3);
    }
    if (lad == 1) {
      h_zvedep[0]->Fill(position.Z());
      h_xvzvedep[0]->Fill(position.X(), position.Z());
      h_yvzvedep[0]->Fill(position.Y(), position.Z());
      h_xvyvedep[0]->Fill(position.X(), position.Y());
      h_rvzvedep[0]->Fill(r, position.Z());
      h_zvedepW[0]->Fill(position.Z(), adep * 1e3);
      h_xvzvedepW[0]->Fill(position.X(), position.Z(), adep * 1e3);
      h_yvzvedepW[0]->Fill(position.Y(), position.Z(), adep * 1e3);
      h_xvyvedepW[0]->Fill(position.X(), position.Y(), adep * 1e3);
      h_rvzvedepW[0]->Fill(r, position.Z(), adep * 1e3);
      if (adep > 50.*1e-6) {
        h_zvedepT[0]->Fill(position.Z());
        h_xvzvedepT[0]->Fill(position.X(), position.Z());
        h_yvzvedepT[0]->Fill(position.Y(), position.Z());
        h_xvyvedepT[0]->Fill(position.X(), position.Y());
        h_rvzvedepT[0]->Fill(r, position.Z());
        h_zvedepWT[0]->Fill(position.Z(), adep * 1e3);
        h_xvzvedepWT[0]->Fill(position.X(), position.Z(), adep * 1e3);
        h_yvzvedepWT[0]->Fill(position.Y(), position.Z(), adep * 1e3);
        h_xvyvedepWT[0]->Fill(position.X(), position.Y(), adep * 1e3);
        h_rvzvedepWT[0]->Fill(r, position.Z(), adep * 1e3);
      }
    } else if (lad == 2) {
      h_zvedep[1]->Fill(position.Z());
      h_xvzvedep[1]->Fill(position.X(), position.Z());
      h_yvzvedep[1]->Fill(position.Y(), position.Z());
      h_xvyvedep[1]->Fill(position.X(), position.Y());
      h_rvzvedep[1]->Fill(r, position.Z());
      h_zvedepW[1]->Fill(position.Z(), adep * 1e3);
      h_xvzvedepW[1]->Fill(position.X(), position.Z(), adep * 1e3);
      h_yvzvedepW[1]->Fill(position.Y(), position.Z(), adep * 1e3);
      h_xvyvedepW[1]->Fill(position.X(), position.Y(), adep * 1e3);
      h_rvzvedepW[1]->Fill(r, position.Z(), adep * 1e3);
      if (adep > 50.*1e-6) {
        h_zvedepT[1]->Fill(position.Z());
        h_xvzvedepT[1]->Fill(position.X(), position.Z());
        h_yvzvedepT[1]->Fill(position.Y(), position.Z());
        h_xvyvedepT[1]->Fill(position.X(), position.Y());
        h_rvzvedepT[1]->Fill(r, position.Z());
        h_zvedepWT[1]->Fill(position.Z(), adep * 1e3);
        h_xvzvedepWT[1]->Fill(position.X(), position.Z(), adep * 1e3);
        h_yvzvedepWT[1]->Fill(position.Y(), position.Z(), adep * 1e3);
        h_xvyvedepWT[1]->Fill(position.X(), position.Y(), adep * 1e3);
        h_rvzvedepWT[1]->Fill(r, position.Z(), adep * 1e3);
      }
    }
  }
  eventNum++;
}

void CLAWSStudyModule::endRun()
{



}

void CLAWSStudyModule::terminate()
{
}


