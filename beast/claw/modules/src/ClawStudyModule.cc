/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/claw/modules/ClawStudyModule.h>
#include <beast/claw/dataobjects/ClawSimHit.h>
#include <beast/claw/dataobjects/ClawHit.h>
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
using namespace claw;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ClawStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ClawStudyModule::ClawStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for Claws (BEAST)");

}

ClawStudyModule::~ClawStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void ClawStudyModule::defineHisto()
{
  h_time = new TH2F("h_time", "Detector # vs. time", 16, 0., 16., 750, 0., 750.);
  h_timeWeighted = new TH2F("h_timeWeigthed", "Detector # vs. time weighted by the energy deposited", 16, 0., 16., 750, 0., 750.);
  h_edep = new TH2F("h_edep", "Time bin # vs. energy deposited", 750, 0., 750., 3000, 0., 3.);
  h_edepThres = new TH2F("h_edepThres", "Time bin # vs. energy deposited", 750, 0., 750., 3000, 0., 3.);

}


void ClawStudyModule::initialize()
{
  B2INFO("ClawStudyModule: Initialize");

  REG_HISTOGRAM

  //convert sample time into rate in Hz
  //rateCorrection = m_sampletime / 1e6;
}

void ClawStudyModule::beginRun()
{
}

void ClawStudyModule::event()
{
  //Here comes the actual event processing

  StoreArray<ClawSimHit>  SimHits;
  StoreArray<ClawHit> Hits;

  //number of entries in Hit
  int nHits = Hits.getEntries();

  //loop over all SimHit entries
  for (int i = 0; i < nHits; i++) {
    ClawHit* aHit = Hits[i];
    int detNb = aHit->getdetNb();
    float edep = aHit->getedep();
    int timeBin = aHit->gettime();
    h_time->Fill(detNb, timeBin);
    h_timeWeighted->Fill(detNb, timeBin, edep);
    h_edep->Fill(timeBin, edep);
    if (edep > 1.0)
      h_edepThres->Fill(timeBin, edep);
  }

  eventNum++;
}

void ClawStudyModule::endRun()
{



}

void ClawStudyModule::terminate()
{
}


