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
#include <beast/microtpc/dataobjects/TPCG4TrackInfo.h>
#include <beast/microtpc/dataobjects/MicrotpcHit.h>
#include <beast/microtpc/dataobjects/MicrotpcDataHit.h>
#include <beast/microtpc/dataobjects/MicrotpcRecoTrack.h>
#include <generators/SAD/dataobjects/SADMetaHit.h>
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
#include <TMath.h>

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

  //Default values are set here. New values can be in MICROTPC.xml.
  addParam("ChipRowNb", m_ChipRowNb, "Chip number of row", 226);
  addParam("ChipColumnNb", m_ChipColumnNb, "Chip number of column", 80);
  addParam("ChipColumnX", m_ChipColumnX, "Chip x dimension in cm / 2", 1.0);
  addParam("ChipRowY", m_ChipRowY, "Chip y dimension in cm / 2", 0.86);
  addParam("z_DG", m_z_DG, "Drift gap distance [cm]", 12.0);
}

MicrotpcStudyModule::~MicrotpcStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void MicrotpcStudyModule::defineHisto()
{
  for (int i = 0 ; i < 6 ; i++) {
    h_tpc_rate[i]  = new TH1F(TString::Format("h_tpc_rate_%d", i), "detector #", 8, 0., 8.);
  }

  for (int i = 0 ; i < 12 ; i++) {
    h_mctpc_kinetic[i]  = new TH1F(TString::Format("h_mctpc_kinetic_%d", i), "Neutron kin. energy [GeV]", 1000, 0., 10.);
    h_mctpc_kinetic_zoom[i]  = new TH1F(TString::Format("h_mctpc_kinetic_zoom_%d", i), "Neutron kin. energy [MeV]", 1000, 0., 10.);
    h_mctpc_tvp[i] = new TH2F(TString::Format("h_mctpc_tvp_%d", i), "theta v phi", 180, 0., 180., 360, -180., 180.);
    h_mctpc_tvpW[i] = new TH2F(TString::Format("h_mctpc_tvpW_%d", i), "theta v phi weighted by kin", 180, 0., 180., 360, -180., 180.);
    h_mctpc_zr[i]  = new TH2F(TString::Format("h_mctpc_zr_%d", i), "r v z", 200, -400., 400., 200, 0., 400.);
    h_mctpc_kinetic[i]->Sumw2();
    h_mctpc_kinetic_zoom[i]->Sumw2();
    h_mctpc_tvp[i]->Sumw2();
    h_mctpc_tvpW[i]->Sumw2();
    h_mctpc_zr[i]->Sumw2();
  }
  for (int i = 0 ; i < 8 ; i++) {
    for (int j = 0; j < 12; j++) {
      h_Wtvp1[i][j] = new TH2F(TString::Format("h_Wtvp1_%d_%d", i, j), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
      h_Wtvp2[i][j] = new TH2F(TString::Format("h_Wtvp2_%d_%d", i, j), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
      h_Wevtrl1[i][j] = new TH2F(TString::Format("h_Wevtrl1_%d_%d", i, j), "Deposited energy [keV] v. track length [cm]", 2000, 0., 10000,
                                 200, 0., 6.);
      h_Wevtrl2[i][j] = new TH2F(TString::Format("h_Wevtrl2_%d_%d", i, j), "Deposited energy [keV] v. track length [cm]", 2000, 0., 10000,
                                 200, 0., 6.);
    }
  }

  for (int i = 0 ; i < 8 ; i++) {
    h_z[i] = new TH1F(TString::Format("h_z_%d", i), "Charged density per cm^2", 2000, 0.0, 20.0);

    h_zr[i] = new TH2F(TString::Format("h_zr_%d", i), "Charged density vs z vs r", 100, 0, 20, 100, 0., 5.);

    h_xy[i] = new TH2F(TString::Format("h_xy_%d", i), "Charged density vs y vs x", 100, -5., 5., 100, -5., 5.);

    h_zx[i] = new TH2F(TString::Format("h_zx_%d", i), "Charged density vs x vs r", 100, 0, 20, 100, -5., 5.);

    h_zy[i] = new TH2F(TString::Format("h_zy_%d", i), "Charged density vs y vs r", 100, 0, 20, 100, -5., 5.);

    h_evtrl[i] = new TH2F(TString::Format("h_evtrl_%d", i), "Deposited energy [keV] v. track length [cm]", 2000, 0., 10000, 200, 0.,
                          6.);
    h_evtrlb[i] = new TH2F(TString::Format("h_evtrlb_%d", i), "Deposited energy [keV] v. track length [cm]", 2000, 0., 10000, 200, 0.,
                           6.);
    h_evtrlc[i] = new TH2F(TString::Format("h_evtrlc_%d", i), "Deposited energy [keV] v. track length [cm]", 2000, 0., 10000, 200, 0.,
                           6.);
    h_evtrld[i] = new TH2F(TString::Format("h_evtrld_%d", i), "Deposited energy [keV] v. track length [cm]", 2000, 0., 10000, 200, 0.,
                           6.);

    h_evtrl_p[i] = new TH2F(TString::Format("h_evtrl_p_%d", i), "Deposited energy [keV] v. track length [cm]", 2000, 0., 2000, 200, 0.,
                            6.);
    h_evtrl_x[i] = new TH2F(TString::Format("h_evtrl_x_%d", i), "Deposited energy [keV] v. track length [cm]", 2000, 0., 2000, 200, 0.,
                            6.);
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


    h_tevtrl[i] = new TH2F(TString::Format("h_tevtrl_%d", i), "t: Deposited energy [keV] v. track length [cm]", 2000, 0., 2000, 200, 0.,
                           6.);
    h_tevtrl_p[i] = new TH2F(TString::Format("h_tevtrl_p_%d", i), "t: Deposited energy [keV] v. track length [cm]", 2000, 0., 2000, 200,
                             0.,
                             6.);
    h_tevtrl_x[i] = new TH2F(TString::Format("h_tevtrl_x_%d", i), "t: Deposited energy [keV] v. track length [cm]", 2000, 0., 2000, 200,
                             0.,
                             6.);
    h_tevtrl_Hex[i] = new TH2F(TString::Format("h_tevtrl_Hex_%d", i), "t: Deposited energy [keV] v. track length [cm]", 2000, 0., 2000,
                               200,
                               0., 6.);
    h_tevtrl_He[i] = new TH2F(TString::Format("h_tevtrl_He_%d", i), "t: Deposited energy [keV] v. track length [cm]", 2000, 0., 2000,
                              200,
                              0., 6.);
    h_tevtrl_C[i] = new TH2F(TString::Format("h_tevtrl_C_%d", i), "t: Deposited energy [keV] v. track length [cm]", 2000, 0., 2000, 200,
                             0.,
                             6.);
    h_tevtrl_O[i] = new TH2F(TString::Format("h_tevtrl_O_%d", i), "t: Deposited energy [keV] v. track length [cm]", 2000, 0., 2000, 200,
                             0.,
                             6.);
    h_tevtrl_He_pure[i] = new TH2F(TString::Format("h_tevtrl_He_pure_%d", i), "t: Deposited energy [keV] v. track length [cm]", 2000,
                                   0.,
                                   2000, 200, 0., 6.);

    h_tvp[i]  = new TH2F(TString::Format("h_tvp_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvpb[i]  = new TH2F(TString::Format("h_wtvpb_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvpc[i]  = new TH2F(TString::Format("h_wtvpc_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvpd[i]  = new TH2F(TString::Format("h_wtvpd_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);

    h_tvpb[i]  = new TH2F(TString::Format("h_tvpb_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_tvpc[i]  = new TH2F(TString::Format("h_tvpc_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_tvpd[i]  = new TH2F(TString::Format("h_tvpd_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_ttvp[i]  = new TH2F(TString::Format("h_ttvp_%d", i), "t: Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvp[i]  = new TH2F(TString::Format("h_wtvp_%d", i), "Phi [deg] v. theta [deg] - weighted", 180, 0., 180, 360, -180., 180.);
    h_tvp_x[i]  = new TH2F(TString::Format("h_tvp_x_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_ttvp_x[i]  = new TH2F(TString::Format("h_ttvp_x_%d", i), "t: Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvp_x[i]  = new TH2F(TString::Format("h_wtvp_x_%d", i), "Phi [deg] v. theta [deg] - weighted", 180, 0., 180, 360, -180., 180.);
    h_tvp_p[i]  = new TH2F(TString::Format("h_tvp_p_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_ttvp_p[i]  = new TH2F(TString::Format("h_ttvp_p_%d", i), "t: Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvp_p[i]  = new TH2F(TString::Format("h_wtvp_p_%d", i), "Phi [deg] v. theta [deg] - weighted", 180, 0., 180, 360, -180., 180.);
    h_tvp_He[i]  = new TH2F(TString::Format("h_tvp_He_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_ttvp_He[i]  = new TH2F(TString::Format("h_ttvp_He_%d", i), "t: Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvp_He[i]  = new TH2F(TString::Format("h_wtvp_He_%d", i), "Phi [deg] v. theta [deg] - weighted", 180, 0., 180, 360, -180., 180.);
    h_tvp_Hex[i]  = new TH2F(TString::Format("h_tvp_Hex_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_ttvp_Hex[i]  = new TH2F(TString::Format("h_ttvp_Hex_%d", i), "t: Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvp_Hex[i]  = new TH2F(TString::Format("h_wtvp_Hex_%d", i), "Phi [deg] v. theta [deg] - weighted", 180, 0., 180, 360, -180.,
                              180.);
    h_tvp_He_pure[i]  = new TH2F(TString::Format("h_tvp_He_pure_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_ttvp_He_pure[i]  = new TH2F(TString::Format("h_ttvp_He_pure_%d", i), "t: Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180.,
                                  180.);
    h_wtvp_He_pure[i]  = new TH2F(TString::Format("h_wtvp_He_pure_%d", i), "Phi [deg] v. theta [deg] - weighted", 180, 0., 180, 360,
                                  -180., 180.);
    h_twtvp_He_pure[i]  = new TH2F(TString::Format("h_twtvp_He_pure_%d", i), "t: Phi [deg] v. theta [deg] - weighted", 180, 0., 180,
                                   360,
                                   -180., 180.);
    h_tvp_C[i]  = new TH2F(TString::Format("h_tvp_C_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_ttvp_C[i]  = new TH2F(TString::Format("h_ttvp_C_%d", i), "t: Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvp_C[i]  = new TH2F(TString::Format("h_wtvp_C_%d", i), "Phi [deg] v. theta [deg] - weighted", 180, 0., 180, 360, -180., 180.);
    h_tvp_O[i]  = new TH2F(TString::Format("h_tvp_O_%d", i), "Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_ttvp_O[i]  = new TH2F(TString::Format("h_ttvp_O_%d", i), "t: Phi [deg] v. theta [deg]", 180, 0., 180, 360, -180., 180.);
    h_wtvp_O[i]  = new TH2F(TString::Format("h_wtvp_O_%d", i), "Phi [deg] v. theta [deg] - weighted", 180, 0., 180, 360, -180., 180.);

    h_tvp[i]->Sumw2();
    h_tvpb[i]->Sumw2();
    h_tvpc[i]->Sumw2();
    h_tvpd[i]->Sumw2();

    h_wtvpb[i]->Sumw2();
    h_wtvpc[i]->Sumw2();
    h_wtvpd[i]->Sumw2();

    h_ttvp[i]->Sumw2();
    h_tvp_x[i]->Sumw2();
    h_ttvp_x[i]->Sumw2();
    h_tvp_p[i]->Sumw2();
    h_ttvp_p[i]->Sumw2();
    h_tvp_He[i]->Sumw2();
    h_ttvp_He[i]->Sumw2();
    h_wtvp[i]->Sumw2();
    h_wtvp_x[i]->Sumw2();
    h_wtvp_p[i]->Sumw2();
    h_wtvp_He[i]->Sumw2();

  }

}


void MicrotpcStudyModule::initialize()
{
  B2INFO("MicrotpcStudyModule: Initialize");

  //read microtpc xml file
  getXMLData();

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
  StoreArray<TPCG4TrackInfo> mcparts;
  StoreArray<SADMetaHit> sadMetaHits;
  double rate = 0;
  for (const auto& sadMetaHit : sadMetaHits) {
    rate = sadMetaHit.getrate();
  }
  /*
  StoreArray<MicrotpcDataHit> DataHits;
  int dentries = DataHits.getEntries();
  for (int j = 0; j < dentries; j++) {
    MicrotpcDataHit* aHit = DataHits[j];
    int detNb = aHit->getdetNb();
    //int trkID = aHit->gettrkID();
    int col = aHit->getcolumn();
    int row = aHit->getrow();
    int tot = aHit->getTOT();
    cout << " col " << col << " row " << row << " tot " << tot << " detNb " << detNb << endl;
  }
  */
  //Bool_t EdgeCut[8];
  double esum[8];
  //Initialize recoil and hit type counters
  for (int i = 0; i < 8; i++) {
    xRec[i] = false;
    pRec[i] = false;
    HeRec[i] = false;
    ORec[i] = false;
    CRec[i] = false;
    //ARec[i] = false;
    pid_old[i] = 0;
    //EdgeCut[i] = true;
    esum[i] = 0;
  }

  //number of entries in SimHit
  int nSimHits = SimHits.getEntries();

  auto phiArray = new vector<double>[8](); //phi
  auto thetaArray = new vector<double>[8](); //theta
  auto pidArray = new vector<int>[8](); //PID
  //auto edgeArray = new vector<int>[8](); // Edge cut
  auto esumArray = new vector<double>[8](); // esum
  auto trlArray = new vector<double>[8](); // trl

  TVector3 EndPoint;
  //loop over all SimHit entries
  for (int i = 0; i < nSimHits; i++) {
    MicrotpcSimHit* aHit = SimHits[i];
    int detNb = aHit->getdetNb();
    TVector3 position = aHit->gettkPos();
    double xpos = position.X() / 100. - TPCCenter[detNb].X();
    double ypos = position.Y() / 100. - TPCCenter[detNb].Y();
    double zpos = position.Z() / 100. - TPCCenter[detNb].Z() + m_z_DG / 2.;
    if (0. < zpos && zpos <  m_z_DG) {

      int PDGid = aHit->gettkPDG();
      if (PDGid == 1000080160) ORec[detNb] = true;
      if (PDGid == 1000060120) CRec[detNb] = true;
      if (PDGid == 1000020040) HeRec[detNb] = true;
      if (PDGid == 2212) pRec[detNb] = true;
      if (fabs(PDGid) == 11 || PDGid == 22) xRec[detNb] = true;

      double edep = aHit->getEnergyDep();
      double niel = aHit->getEnergyNiel();
      double ioni = (edep - niel) * 1e3; //MeV -> keV

      double r = sqrt(xpos * xpos + ypos * ypos);
      h_z[detNb]->Fill(zpos, ioni);
      h_zr[detNb]->Fill(zpos, r, ioni);
      h_zx[detNb]->Fill(zpos, xpos, ioni);
      h_xy[detNb]->Fill(xpos, ypos, ioni);
      h_zy[detNb]->Fill(zpos, ypos, ioni);
      TVector3 direction = aHit->gettkMomDir();
      double theta = direction.Theta() * TMath::RadToDeg();
      double phi = direction.Phi() * TMath::RadToDeg();

      if ((-m_ChipColumnX < xpos && xpos < m_ChipColumnX) &&
          (-m_ChipRowY < ypos && ypos <  m_ChipRowY) &&
          (0. < zpos && zpos <  m_z_DG)) {
        //edgeArray].push_back(1);
      } else {
        //edgeArray[i].push_back(0);
        //EdgeCut[detNb] = false;
      }

      if (pid_old[detNb] != PDGid) {
        if (esum[detNb] > 0) {
          esumArray[detNb].push_back(esum[detNb]);
          TVector3 BeginPoint;
          BeginPoint.SetXYZ(xpos, ypos, zpos);
          double trl0 = BeginPoint * direction.Unit();
          double trl1 = EndPoint * direction.Unit();
          trlArray[detNb].push_back(fabs(trl0 - trl1));
          /*
            double trl = fabs(trl0 - trl1);
            double ioniz = esum[detNb];
            if (PDGid == 1000080160) {
            h_ttvp_O[detNb]->Fill(theta, phi);
            h_tevtrl_O[detNb]->Fill(ioniz, trl);
            }
            if (PDGid == 1000060120) {
            h_ttvp_C[detNb]->Fill(theta, phi);
            h_tevtrl_C[detNb]->Fill(ioniz, trl);
            }
            if (PDGid == 1000020040) {
            h_ttvp_He[detNb]->Fill(theta, phi);
            h_tevtrl_He[detNb]->Fill(ioniz, trl);
            }
            if (PDGid == 2212) {
            h_ttvp_p[detNb]->Fill(theta, phi);
            h_tevtrl_p[detNb]->Fill(ioniz, trl);
            }
            if (fabs(PDGid) == 11 || PDGid == 22) {
            h_ttvp_x[detNb]->Fill(theta, phi);
            h_tevtrl_x[detNb]->Fill(ioniz, trl);
            }
            h_ttvp[detNb]->Fill(theta, phi);
            h_tevtrl[detNb]->Fill(ioniz, trl);

            if (EdgeCut[detNb]) {
            if (PDGid == 1000020040) {
            h_ttvp_He_pure[detNb]->Fill(theta, phi);
                  h_twtvp_He_pure[detNb]->Fill(theta, phi, ioniz);
                  h_tevtrl_He_pure[detNb]->Fill(ioni, trl);
            }
            }
          */

          thetaArray[detNb].push_back(theta);
          phiArray[detNb].push_back(phi);
          pidArray[detNb].push_back(PDGid);

        }
        pid_old[detNb] = PDGid;
        esum[detNb] = 0;

      } else {
        esum[detNb] += ioni;
        EndPoint.SetXYZ(xpos, ypos, zpos);
      }
    }
  }
  /*
  for (int i = 0; i < 8;  i++) {

    for (int j = 0; j < (int)phiArray[i].size(); j++) {
      //if (EdgeCut[i]) {
      double phi = phiArray[i][j];
      double theta = thetaArray[i][j];
      int PDGid = pidArray[i][j];
      double ioni = esumArray[i][j];
      double trl = trlArray[i][j];
      if (PDGid == 1000080160) {
        h_ttvp_O[i]->Fill(theta, phi);
        h_tevtrl_O[i]->Fill(ioni, trl);
      }
      if (PDGid == 1000060120) {
        h_ttvp_C[i]->Fill(theta, phi);
        h_tevtrl_C[i]->Fill(ioni, trl);
      }
      if (PDGid == 1000020040) {
        h_ttvp_He[i]->Fill(theta, phi);
        h_tevtrl_He[i]->Fill(ioni, trl);
      }
      if (PDGid == 2212) {
        h_ttvp_p[i]->Fill(theta, phi);
        h_tevtrl_p[i]->Fill(ioni, trl);
      }
      if (fabs(PDGid) == 11 || PDGid == 22) {
        h_ttvp_x[i]->Fill(theta, phi);
        h_tevtrl_x[i]->Fill(ioni, trl);
      }
      h_ttvp[i]->Fill(theta, phi);
      h_tevtrl[i]->Fill(ioni, trl);

      if (EdgeCut[i]) {
        //if (PDGid == 1000020040) {
        h_ttvp_He_pure[i]->Fill(theta, phi);
        h_twtvp_He_pure[i]->Fill(theta, phi, ioni);
        h_tevtrl_He_pure[i]->Fill(ioni, trl);
        //}
      }
      //}
    }
  }
  */
  for (const auto& mcpart : mcparts) { // start loop over all Tracks
    const double energy = mcpart.getEnergy();
    const double mass = mcpart.getMass();
    double kin = energy - mass;
    const double PDG = mcpart.getPDG();
    const TVector3 vtx = mcpart.getProductionVertex();
    const TVector3 mom = mcpart.getMomentum();
    double theta = mom.Theta() * TMath::RadToDeg();
    double phi = mom.Phi() * TMath::RadToDeg();
    double z = vtx.Z();
    double r = sqrt(vtx.X() * vtx.X() + vtx.Y() * vtx.Y());
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

    if (PDG == 2112) {
      if (r < 10.0) {
        h_mctpc_kinetic[9]->Fill(kin);
        h_mctpc_kinetic_zoom[9]->Fill(kin * 1e3);
        h_mctpc_tvp[9]->Fill(theta, phi);
        h_mctpc_tvpW[9]->Fill(theta, phi, kin);
        h_mctpc_zr[9]->Fill(z, r);
      }
      if (r > 70.0) {
        h_mctpc_kinetic[10]->Fill(kin);
        h_mctpc_kinetic_zoom[10]->Fill(kin * 1e3);
        h_mctpc_tvp[10]->Fill(theta, phi);
        h_mctpc_tvpW[10]->Fill(theta, phi, kin);
        h_mctpc_zr[10]->Fill(z, r);
      }
    }

    for (int i = 0; i < 9; i++) {
      if (partID[i] == 1) {
        h_mctpc_kinetic[i]->Fill(kin);
        h_mctpc_kinetic_zoom[i]->Fill(kin * 1e3);
        h_mctpc_tvp[i]->Fill(theta, phi);
        h_mctpc_tvpW[i]->Fill(theta, phi, kin);
        h_mctpc_zr[i]->Fill(z, r);
      }
    }
  }
  //number of Tracks
  //int nTracks = Tracks.getEntries();

  //loop over all Tracks
  for (const auto& aTrack : Tracks) { // start loop over all Tracks
    const int detNb = aTrack.getdetNb();
    const float phi = aTrack.getphi();
    const float theta = aTrack.gettheta();
    const float trl = aTrack.gettrl();
    const float tesum = aTrack.getesum();
    const int pixnb = aTrack.getpixnb();
    //const int time_range = aTrack.gettime_range();
    int side[16];
    for (int j = 0; j < 16; j++) {
      side[j] = 0;
      side[j] = aTrack.getside()[j];
    }
    Bool_t EdgeCuts = false;
    if (side[0] == 0 && side[1] == 0 && side[2] == 0 && side[3] == 0) EdgeCuts = true;
    Bool_t Asource = false;
    if (side[4] == 2 && side[5] == 2) Asource = true;
    //Bool_t Goodtrk = false;
    //if (2.015 < trl && trl < 2.03) Goodtrk = true;
    //Bool_t GoodAngle = false;
    //if (88.5 < theta && theta < 91.5) GoodAngle = true;
    int partID[7];
    partID[0] = 1; //[0] for all events
    for (int j = 0; j < 6; j++) partID[j + 1] = aTrack.getpartID()[j];

    if (ORec[detNb] || CRec[detNb]  || HeRec[detNb])
      h_tpc_rate[0]->Fill(detNb);
    if (pRec[detNb])
      h_tpc_rate[1]->Fill(detNb);
    if (xRec[detNb])
      h_tpc_rate[2]->Fill(detNb);

    if (EdgeCuts) {
      if (ORec[detNb] || CRec[detNb]  || HeRec[detNb])
        h_tpc_rate[3]->Fill(detNb);
      if (pRec[detNb])
        h_tpc_rate[4]->Fill(detNb);
      if (xRec[detNb])
        h_tpc_rate[5]->Fill(detNb);
    }

    h_evtrl[detNb]->Fill(tesum, trl);
    h_tvp[detNb]->Fill(theta, phi);
    h_wtvp[detNb]->Fill(theta, phi, tesum);
    h_Wtvp1[detNb][0]->Fill(theta, phi, rate);
    h_Wevtrl1[detNb][0]->Fill(tesum, trl, rate);
    h_Wtvp2[detNb][0]->Fill(theta, phi, rate * tesum);
    //h_Wevtrl1[detNb][0]->Fill(tesum, trl, rate);
    if (EdgeCuts && pixnb > 10. && tesum > 10.) {
      h_evtrlb[detNb]->Fill(tesum, trl);
      h_tvpb[detNb]->Fill(theta, phi);
      h_wtvpb[detNb]->Fill(theta, phi, tesum);
      h_Wtvp1[detNb][1]->Fill(theta, phi, rate);
      h_Wevtrl1[detNb][1]->Fill(tesum, trl, rate);
      h_Wtvp2[detNb][1]->Fill(theta, phi, rate * tesum);
    }

    for (int j = 0; j < 7; j++) {
      if (j == 3 && !EdgeCuts && (partID[1] == 1 || partID[2] == 1 || partID[4] == 1 || partID[5] == 1 || partID[6] == 1)) partID[j] = 0;
      if ((j == 4 || j == 5) && !Asource) partID[j] = 0;
      if (partID[j] == 1) {
        h_Wtvp1[detNb][2 + j]->Fill(theta, phi, rate);
        h_Wevtrl1[detNb][2 + j]->Fill(tesum, trl, rate);
        h_Wtvp2[detNb][2 + j]->Fill(theta, phi, rate * tesum);
        if (j == 0) {
          h_evtrlc[detNb]->Fill(tesum, trl);
          h_tvpc[detNb]->Fill(theta, phi);
          h_wtvpc[detNb]->Fill(theta, phi, tesum);
        }
        if (j == 1) {
          h_evtrld[detNb]->Fill(tesum, trl);
          h_tvpd[detNb]->Fill(theta, phi);
          h_wtvpd[detNb]->Fill(theta, phi, tesum);
        }
        if (j == 2) {
          h_evtrl_x[detNb]->Fill(tesum, trl);
          h_tvp_x[detNb]->Fill(theta, phi);
          h_wtvp_x[detNb]->Fill(theta, phi, tesum);
        }
        if (j == 3) {
          h_evtrl_p[detNb]->Fill(tesum, trl);
          h_tvp_p[detNb]->Fill(theta, phi);
          h_wtvp_p[detNb]->Fill(theta, phi, tesum);
        }
        if (j == 4) {
          h_evtrl_x[detNb]->Fill(tesum, trl);
          h_tvp_x[detNb]->Fill(theta, phi);
          h_wtvp_x[detNb]->Fill(theta, phi, tesum);
        }
        if (j == 5) {
          h_evtrl_He[detNb]->Fill(tesum, trl);
          h_tvp_He[detNb]->Fill(theta, phi);
          h_wtvp_He[detNb]->Fill(theta, phi, tesum);
        }
        if (j == 6) {
          h_evtrl_Hex[detNb]->Fill(tesum, trl);
          h_tvp_Hex[detNb]->Fill(theta, phi);
          h_wtvp_Hex[detNb]->Fill(theta, phi, tesum);
        }
      }
    }
  }

  eventNum++;

  //delete
  delete [] phiArray;
  delete [] thetaArray;
  delete [] pidArray;
  //delete [] edgeArray;
  delete [] esumArray;
  delete [] trlArray;

}
//read tube centers, impulse response, and garfield drift data filename from MICROTPC.xml
void MicrotpcStudyModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"MICROTPC\"]/Content/");

  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

    TPCCenter.push_back(TVector3(activeParams.getLength("TPCpos_x"), activeParams.getLength("TPCpos_y"),
                                 activeParams.getLength("TPCpos_z")));
    nTPC++;
  }

  m_ChipColumnNb = content.getInt("ChipColumnNb");
  m_ChipRowNb = content.getInt("ChipRowNb");
  m_ChipColumnX = content.getDouble("ChipColumnX");
  m_ChipRowY = content.getDouble("ChipRowY");
  m_z_DG = content.getDouble("z_DG");

  B2INFO("TpcDigitizer: Aquired tpc locations and gas parameters");
  B2INFO("              from MICROTPC.xml. There are " << nTPC << " TPCs implemented");

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


