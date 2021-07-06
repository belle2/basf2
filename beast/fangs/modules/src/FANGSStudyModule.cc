/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/fangs/modules/FANGSStudyModule.h>
#include <beast/fangs/dataobjects/FANGSSimHit.h>
#include <beast/fangs/dataobjects/FANGSHit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <cmath>

#include <fstream>
#include <string>

// ROOT
#include <TVector3.h>
#include <TH1.h>
#include <TH2.h>

int eventNum = 0;

using namespace std;

using namespace Belle2;
using namespace fangs;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FANGSStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FANGSStudyModule::FANGSStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for Fangs (BEAST)");

}

FANGSStudyModule::~FANGSStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void FANGSStudyModule::defineHisto()
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
  for (int i = 0; i < 3; i++) {
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
  h_Edep = new TH2F("h_Edep", "det # # vs. energy deposited", 20, 0., 20., 1000, 0., 10.);
  h_pxNb = new TH2F("h_pxNb", "det # # vs. nb pixel", 20, 0., 20., 1000, 0., 1000.);
  for (int i = 0; i < 15; i++) {
    h_cvr[i] = new TH2F(TString::Format("cvr_%d", i) , " col v. row", 80, 0., 80., 336, 0., 336.);
  }
}


void FANGSStudyModule::initialize()
{
  B2INFO("FANGSStudyModule: Initialize");

  REG_HISTOGRAM

  //convert sample time into rate in Hz
  //rateCorrection = m_sampletime / 1e6;
  //get FANGS paramters
  getXMLData();

  fctQ_Calib1 = new TF1("fctQ_Calib1", "[0]*([1]*x-[2])/([3]-x)", 0., 15.);
  fctQ_Calib1->SetParameters(m_TOTQ1, m_TOTC1, m_TOTA1 * m_TOTB1, m_TOTA1);

  fctQ_Calib2 = new TF1("fctQ_Calib2", "[0]*([1]*x-[2])/([3]-x)", 0., 15.);
  fctQ_Calib2->SetParameters(m_TOTQ2, m_TOTC2, m_TOTA2 * m_TOTB2, m_TOTA2);
}

void FANGSStudyModule::beginRun()
{
}

void FANGSStudyModule::event()
{
  //Here comes the actual event processing

  StoreArray<FANGSSimHit>  SimHits;
  StoreArray<FANGSHit> Hits;

  int olddetNb = -1;
  int ipix = 0;
  float esum = 0;
  //number of entries in Hits
  for (const auto& FANGSHit : Hits) {
    int detNb = FANGSHit.getdetNb();
    //int pdg = FANGSHit.getPDG();
    //int trkID = FANGSHit.gettrkID();
    int col = FANGSHit.getcolumn();
    int row = FANGSHit.getrow();
    int tot = FANGSHit.getTOT();
    int bcid = FANGSHit.getBCID();

    if (olddetNb != detNb) {
      if (esum > 0) {
        h_Edep->Fill(detNb, esum);
        h_pxNb->Fill(detNb, ipix);
      }
      ipix = 0;
      esum = 0;
      for (int j = 0; j < maxSIZE; j++) {
        x[j] = 0;
        y[j] = 0;
        z[j] = 0;
        e[j] = 0;
      }
      olddetNb = detNb;
    }
    x[ipix] = col * (2. * m_ChipColumnX / (float)m_ChipColumnNb) - m_ChipColumnX;
    y[ipix] = row * (2. * m_ChipRowY / (float)m_ChipRowNb) - m_ChipRowY;
    z[ipix] = (m_PixelTimeBin / 2. + m_PixelTimeBin * bcid) * m_v_sensor;
    if (tot < 3) e[ipix] = fctQ_Calib1->Eval(tot) * m_Workfct * 1e-3;
    else e[ipix] = fctQ_Calib2->Eval(tot) * m_Workfct * 1e-3;
    esum += e[ipix];
    h_cvr[detNb]->Fill(col, row);
    ipix ++;
  }
  //number of entries in SimHits
  int nSimHits = SimHits.getEntries();
  //cout << nSimHits << endl;

  //loop over all SimHit entries
  for (int i = 0; i < nSimHits; i++) {
    FANGSSimHit* aHit = SimHits[i];
    int lad = aHit->getLadder();
    int sen = aHit->getSensor();
    double adep = aHit->getEnergyDep();
    double timeBin = aHit->getTime();
    int  pdg = aHit->getPDG();

    TVector3 position = aHit->getPosEntry();
    double r = sqrt(position.X() * position.X() + position.Y() * position.Y());
    int detNB = (lad - 1) * 5 + sen - 1;
    //cout <<" lad " << lad << " sen " << sen << " detNB " << detNB << " time " << timeBin << " edep " << adep*1e3 << endl;
    //cout <<" x " << position.X()  << " y " << position.Y() << " z " << position.Z()  << endl;
    h_time->Fill(detNB, timeBin);
    h_edep->Fill(detNB, adep * 1e3);
    if (fabs(pdg) == Const::electron.getPDGCode())h_edep1->Fill(detNB, adep * 1e3);
    if (pdg == Const::photon.getPDGCode())h_edep2->Fill(detNB, adep * 1e3);
    if (pdg != Const::photon.getPDGCode() && fabs(pdg) != Const::electron.getPDGCode())h_edep3->Fill(detNB, adep * 1e3);
    if (adep > 50.*1e-6) {
      h_timeThres->Fill(detNB, timeBin);
      h_edepThres->Fill(detNB, adep * 1e3);
      if (fabs(pdg) == Const::electron.getPDGCode())h_edepThres1->Fill(detNB, adep * 1e3);
      if (pdg == Const::photon.getPDGCode())h_edepThres2->Fill(detNB, adep * 1e3);
      if (pdg != Const::photon.getPDGCode() && fabs(pdg) != Const::electron.getPDGCode())h_edepThres3->Fill(detNB, adep * 1e3);
    }
    h_zvedep[lad - 1]->Fill(position.Z());
    h_xvzvedep[lad - 1]->Fill(position.X(), position.Z());
    h_yvzvedep[lad - 1]->Fill(position.Y(), position.Z());
    h_xvyvedep[lad - 1]->Fill(position.X(), position.Y());
    h_rvzvedep[lad - 1]->Fill(r, position.Z());
    h_zvedepW[lad - 1]->Fill(position.Z(), adep * 1e3);
    h_xvzvedepW[lad - 1]->Fill(position.X(), position.Z(), adep * 1e3);
    h_yvzvedepW[lad - 1]->Fill(position.Y(), position.Z(), adep * 1e3);
    h_xvyvedepW[lad - 1]->Fill(position.X(), position.Y(), adep * 1e3);
    h_rvzvedepW[lad - 1]->Fill(r, position.Z(), adep * 1e3);
    if (adep > 50.*1e-6) {
      h_zvedepT[lad - 1]->Fill(position.Z());
      h_xvzvedepT[lad - 1]->Fill(position.X(), position.Z());
      h_yvzvedepT[lad - 1]->Fill(position.Y(), position.Z());
      h_xvyvedepT[lad - 1]->Fill(position.X(), position.Y());
      h_rvzvedepT[lad - 1]->Fill(r, position.Z());
      h_zvedepWT[lad - 1]->Fill(position.Z(), adep * 1e3);
      h_xvzvedepWT[lad - 1]->Fill(position.X(), position.Z(), adep * 1e3);
      h_yvzvedepWT[lad - 1]->Fill(position.Y(), position.Z(), adep * 1e3);
      h_xvyvedepWT[lad - 1]->Fill(position.X(), position.Y(), adep * 1e3);
      h_rvzvedepWT[lad - 1]->Fill(r, position.Z(), adep * 1e3);
    }
  }

  eventNum++;
}

void FANGSStudyModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"FANGS\"]/Content/");

  m_PixelThreshold = content.getInt("PixelThreshold");
  m_PixelThresholdRMS = content.getInt("PixelThresholdRMS");
  m_PixelTimeBinNb = content.getInt("PixelTimeBinNb");
  m_PixelTimeBin = content.getDouble("PixelTimeBin");
  m_ChipColumnNb = content.getInt("ChipColumnNb");
  m_ChipRowNb = content.getInt("ChipRowNb");
  m_ChipColumnX = content.getDouble("ChipColumnX");
  m_ChipRowY = content.getDouble("ChipRowY");
  m_TOTA1 = content.getDouble("TOTA1");
  m_TOTB1 = content.getDouble("TOTB1");
  m_TOTC1 = content.getDouble("TOTC1");
  m_TOTQ1 = content.getDouble("TOTQ1");
  m_TOTA2 = content.getDouble("TOTA2");
  m_TOTB2 = content.getDouble("TOTB2");
  m_TOTC2 = content.getDouble("TOTC2");
  m_TOTQ2 = content.getDouble("TOTQ2");
  m_v_sensor = content.getDouble("v_sensor");
  m_sensor_width = content.getDouble("sensor_width");
  m_Workfct = content.getDouble("Workfct");

}

void FANGSStudyModule::endRun()
{



}

void FANGSStudyModule::terminate()
{
}


