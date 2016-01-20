/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/qcsmonitor/modules/QcsmonitorStudyModule.h>
#include <beast/qcsmonitor/dataobjects/QcsmonitorSimHit.h>
#include <beast/qcsmonitor/dataobjects/QcsmonitorHit.h>
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
using namespace qcsmonitor;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(QcsmonitorStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

QcsmonitorStudyModule::QcsmonitorStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for Qcsmonitors (BEAST)");

}

QcsmonitorStudyModule::~QcsmonitorStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void QcsmonitorStudyModule::defineHisto()
{
  h_time = new TH2F("h_time", "Detector # vs. time", 16, 0., 16., 750, 0., 750.);
  h_time->Sumw2();
  h_timeWeighted = new TH2F("h_timeWeigthed", "Detector # vs. time weighted by the energy deposited", 16, 0., 16., 750, 0., 750.);
  h_timeWeighted->Sumw2();
  h_timeThres = new TH2F("h_timeThres", "Detector # vs. time", 16, 0., 16., 750, 0., 750.);
  h_timeThres->Sumw2();
  h_timeWeightedThres = new TH2F("h_timeWeigthedThres", "Detector # vs. time weighted by the energy deposited", 16, 0., 16., 750, 0.,
                                 750.);
  h_timeWeightedThres->Sumw2();
  h_edep = new TH2F("h_edep", "Time bin # vs. energy deposited", 750, 0., 750., 3000, 0., 3.);
  h_edep->Sumw2();
  h_edepThres = new TH2F("h_edepThres", "Time bin # vs. energy deposited", 750, 0., 750., 3000, 0., 3.);
  h_edepThres->Sumw2();
  for (int i = 0; i < 2; i++) {
    h_zvedep[i] = new TH1F(TString::Format("h_zvedep_%d", i) , "edep [MeV] vs. z [cm]", 200, -10., 10.);
    h_zvedep[i]->Sumw2();
  }
}


void QcsmonitorStudyModule::initialize()
{
  B2INFO("QcsmonitorStudyModule: Initialize");

  REG_HISTOGRAM

  //convert sample time into rate in Hz
  //rateCorrection = m_sampletime / 1e6;
}

void QcsmonitorStudyModule::beginRun()
{
}

void QcsmonitorStudyModule::event()
{
  //Here comes the actual event processing

  StoreArray<QcsmonitorSimHit>  SimHits;
  StoreArray<QcsmonitorHit> Hits;

  //number of entries in SimHits
  int nSimHits = SimHits.getEntries();
  /*
  //loop over all SimHit entries
  for (int i = 0; i < nSimHits; i++) {
    QcsmonitorSimHit* aHit = SimHits[i];
    int detNb = aHit->getdetNb();
    double adep = aHit->getEnergyNiel();
    TVector3 position = aHit->gettkPos();

    if (0 <= detNb && detNb <= 7)
      h_zvedep[0]->Fill(position.Z() / 100., adep);
    else if (8 <= detNb && detNb <= 15)
      h_zvedep[1]->Fill(position.Z() / 100., adep);
  }

  //number of entries in Hit
  int nHits = Hits.getEntries();

  //loop over all Hit entries
  for (int i = 0; i < nHits; i++) {
    QcsmonitorHit* aHit = Hits[i];
    int detNb = aHit->getdetNb();
    float edep = aHit->getedep();
    int timeBin = aHit->gettime();
    h_time->Fill(detNb, timeBin);
    h_timeWeighted->Fill(detNb, timeBin, edep);
    h_edep->Fill(timeBin, edep);
    if (edep > 1.0) {
      h_edepThres->Fill(timeBin, edep);
      h_timeThres->Fill(detNb, timeBin);
      h_timeWeightedThres->Fill(detNb, timeBin, edep);
    }
  }
  */
  eventNum++;
}

void QcsmonitorStudyModule::endRun()
{



}

void QcsmonitorStudyModule::terminate()
{
}


