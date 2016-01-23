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
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
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

  addParam("Ethres", m_Ethres, "Energy threshold in MeV", 0.0);
}

QcsmonitorStudyModule::~QcsmonitorStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void QcsmonitorStudyModule::defineHisto()
{
  for (int i = 0; i < 20; i++) {
    h_qcsms_Evtof1[i] = new TH2F(TString::Format("h_qcsms_Evtof1_%d", i), "Energy deposited [MeV] vs TOF [ns] - all", 5000, 0., 1000.,
                                 1000, 0., 10.);
    h_qcsms_Evtof2[i] = new TH2F(TString::Format("h_qcsms_Evtof2_%d", i), "Energy deposited [MeV] vs TOF [ns] - only photons", 5000, 0.,
                                 1000., 1000, 0., 10.);
    h_qcsms_Evtof3[i] = new TH2F(TString::Format("h_qcsms_Evtof3_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 5000, 0.,
                                 1000., 1000, 0., 10.);
    h_qcsms_edep[i] = new TH1F(TString::Format("h_qcsms_edep_%d", i), "Energy deposited [MeV]", 5000, 0., 10.);
  }
}


void QcsmonitorStudyModule::initialize()
{
  B2INFO("QcsmonitorStudyModule: Initialize");

  REG_HISTOGRAM

  //read QCSMONITOR xml file
  getXMLData();

}

void QcsmonitorStudyModule::beginRun()
{
}

void QcsmonitorStudyModule::event()
{
  //Here comes the actual event processing

  StoreArray<QcsmonitorSimHit>  SimHits;

  //number of entries in SimHits
  int nSimHits = SimHits.getEntries();

  //loop over all SimHit entries
  for (int i = 0; i < nSimHits; i++) {
    QcsmonitorSimHit* aHit = SimHits[i];
    int detNB = aHit->getCellId();
    //int trkID = aHit->getTrackId();
    int pdg = aHit->getPDGCode();
    double Edep = aHit->getEnergyDep() * 1e3; //GeV -> MeV
    double tof = aHit->getFlightTime(); //ns

    h_qcsms_Evtof1[detNB]->Fill(tof, Edep);
    if (pdg == 22) h_qcsms_Evtof2[detNB]->Fill(tof, Edep);
    else if (fabs(pdg) == 11) h_qcsms_Evtof3[detNB]->Fill(tof, Edep);
    else h_qcsms_Evtof3[detNB]->Fill(tof, Edep);
    if (Edep > m_Ethres)h_qcsms_edep[detNB]->Fill(Edep);
  }


  eventNum++;
}

//read energy threshold from QCSMONITOR.xml
void QcsmonitorStudyModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"QCSMONITOR\"]/Content/");
  m_Ethres = content.getDouble("Ethres");

  B2INFO("QcsmonitorStudy");
}

void QcsmonitorStudyModule::endRun()
{



}

void QcsmonitorStudyModule::terminate()
{
}


