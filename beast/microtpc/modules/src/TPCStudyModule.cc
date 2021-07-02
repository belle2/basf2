/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/microtpc/modules/TPCStudyModule.h>
#include <beast/microtpc/dataobjects/MicrotpcSimHit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Const.h>
#include <boost/foreach.hpp>

#include <iostream>
#include <string>

// ROOT
#include <TVector3.h>
#include <TH1.h>
#include <TH2.h>

int ctr = 0;
int co_ctr[4] = {0, 0, 0, 0};
int coe_ctr[4] = {0, 0, 0, 0};
int h1_ctr[4] = {0, 0, 0, 0};
int n_ctr[4] = {0, 0, 0, 0};
int he4_ctr[4] = {0, 0, 0, 0};
int o16_ctr[4] = {0, 0, 0, 0};
int c12_ctr[4] = {0, 0, 0, 0};
int ctr_ele[4] = {0, 0, 0, 0};
int ctr_pos[4] = {0, 0, 0, 0};
int ctr_pro[4] = {0, 0, 0, 0};
int ctr_neu[4] = {0, 0, 0, 0};
int ctr_good_neu[4] = {0, 0, 0, 0};
int ctr_bad_neu[4] = {0, 0, 0, 0};
int ctr_bak[4] = {0, 0, 0, 0};

using namespace std;

using namespace Belle2;
using namespace microtpc;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TPCStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TPCStudyModule::TPCStudyModule() : HistoModule()
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

TPCStudyModule::~TPCStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void TPCStudyModule::defineHisto()
{
  for (int i = 0; i < 11; i ++) {
    h_tpc_kin[i] = new TH1F(TString::Format("tpc_kin_%d", i), "", 1000, 0., 100.);
    h_tpc_xy[i] = new TH2F(TString::Format("tpc_xy_%d", i), "", 300, -3., 3., 300, -3., 3.);
  }

}

void TPCStudyModule::initialize()
{
  B2INFO("TPCStudyModule: Initialize");

  //read microtpc xml file
  getXMLData();

  REG_HISTOGRAM

}

void TPCStudyModule::beginRun()
{
}

void TPCStudyModule::event()
{
  //Here comes the actual event processing

  StoreArray<MicrotpcSimHit>  SimHits;
  //StoreArray<MicrotpcHit> Hits;
  //StoreArray<MicrotpcRecoTrack> Tracks;
  //StoreArray<TPCG4TrackInfo> mcparts;
  //StoreArray<SADMetaHit> sadMetaHits;

  int old_trkID[4] = { -1, -1, -1, -1};
  int old_trkID_h1[4] = { -1, -1, -1, -1};
  int old_trkID_he4[4] = { -1, -1, -1, -1};
  int old_trkID_c12[4] = { -1, -1, -1, -1};
  int old_trkID_o16[4] = { -1, -1, -1, -1};
  bool aneu[4] = {false, false, false, false};
  bool apro[4] = {false, false, false, false};
  bool atrk[4] = {false, false, false, false};
  bool aele[4] = {false, false, false, false};
  bool apos[4] = {false, false, false, false};

  vector <double> RecoilE;
  vector <int> Pdg[4];
  int NbofPart[4] = {0, 0, 0, 0};

  cout << "Look at evt " << ctr << endl;

  for (const auto& MicrotpcSimHit : SimHits) {
    int detNb = MicrotpcSimHit.getdetNb();
    int pdg = MicrotpcSimHit.gettkPDG();
    int trkID = MicrotpcSimHit.gettkID();
    TVector3 position = MicrotpcSimHit.gettkPos();
    TVector3 direction = MicrotpcSimHit.gettkMomDir();
    double xpos = position.X() / 100. - TPCCenter[detNb].X();
    double ypos = position.Y() / 100. - TPCCenter[detNb].Y();
    double zpos = position.Z() / 100. - TPCCenter[detNb].Z() + m_z_DG / 2.;
    //if (ctr == 95 && detNb == 0 && (MicrotpcSimHit.getEnergyDep() * 1e6) > 250.0) {
    if (ctr == 95 && detNb == 0 && MicrotpcSimHit.getEnergyDep() > 0) {
      cout << " pdg " << pdg << " Energy deposited " << MicrotpcSimHit.getEnergyDep() << " zpos " << zpos << endl;
      h_tpc_xy[7]->Fill(xpos, ypos);
      if (pdg == 1000020040) h_tpc_xy[8]->Fill(xpos, ypos);
      if (pdg == Const::proton.getPDGCode()) h_tpc_xy[9]->Fill(xpos, ypos);
      if (pdg == Const::electron.getPDGCode()) h_tpc_xy[10]->Fill(xpos, ypos);
    }
    if (old_trkID[detNb] != trkID && MicrotpcSimHit.gettkKEnergy() > 0 && MicrotpcSimHit.getEnergyDep() > 0) {
      old_trkID[detNb] = trkID;
      Pdg[detNb].push_back(pdg);
      NbofPart[detNb] ++;
    }

    if (pdg == 1000020040) {
      //cout << "He4 detNb " << detNb << " trID " << trkID << endl;
      //cout << "Direction x " << direction.X() << " y " << direction.Y() << " z " << direction.Z()  << endl;
      //cout << "Vertex x " << position.X() << " y " << position.Y() << " z " << position.Z() << endl;
      //cout << "Kinetic energy " << MicrotpcSimHit.gettkKEnergy() << endl;
      if (old_trkID_he4[detNb] != trkID && MicrotpcSimHit.gettkKEnergy() > 0 && MicrotpcSimHit.getEnergyDep() > 0) {
        old_trkID_he4[detNb] = trkID;
        //cout << "Output alpha track direction and vertex and momentum"<<endl;
        //cout << "Direction x " << direction.X() << " y " << direction.Y() << " z " << direction.Z()  << endl;
        //cout << "Vertex x " << position.X() << " y " << position.Y() << " z " << position.Z() << endl;
        //cout << "Kinetic energy " << MicrotpcSimHit.gettkKEnergy() << endl;
        atrk[detNb] = true;
        RecoilE.push_back(MicrotpcSimHit.gettkKEnergy());
        h_tpc_kin[0]->Fill(MicrotpcSimHit.gettkKEnergy());
        he4_ctr[detNb] ++;
      }
      h_tpc_xy[0]->Fill(xpos, ypos);
    }

    if (pdg == 1000060120) {
      //cout << "C 12 detNb " << detNb << " trID " << trkID << endl;
      //cout << "Direction x " << direction.X() << " y " << direction.Y() << " z " << direction.Z()  << endl;
      //cout << "Vertex x " << position.X() << " y " << position.Y() << " z " << position.Z() << endl;
      //cout << "Kinetic energy " << MicrotpcSimHit.gettkKEnergy() << endl;
      if (old_trkID_c12[detNb] != trkID && MicrotpcSimHit.gettkKEnergy() > 0 && MicrotpcSimHit.getEnergyDep() > 0) {
        old_trkID_c12[detNb] = trkID;
        //cout << "Output alpha track direction and vertex and momentum"<<endl;
        //cout << "Direction x " << direction.X() << " y " << direction.Y() << " z " << direction.Z()  << endl;
        //cout << "Vertex x " << position.X() << " y " << position.Y() << " z " << position.Z() << endl;
        //cout << "Kinetic energy " << MicrotpcSimHit.gettkKEnergy() << endl;
        RecoilE.push_back(MicrotpcSimHit.gettkKEnergy());
        atrk[detNb] = true;
        h_tpc_kin[1]->Fill(MicrotpcSimHit.gettkKEnergy());
        c12_ctr[detNb] ++;
      }
      h_tpc_xy[1]->Fill(xpos, ypos);
    }

    if (pdg == 1000080160) {
      //cout << "O 16 detNb " << detNb << " trID " << trkID << endl;
      //cout << "Direction x " << direction.X() << " y " << direction.Y() << " z " << direction.Z()  << endl;
      //cout << "Vertex x " << position.X() << " y " << position.Y() << " z " << position.Z() << endl;
      //cout << "Kinetic energy " << MicrotpcSimHit.gettkKEnergy() << endl;
      if (old_trkID_o16[detNb] != trkID && MicrotpcSimHit.gettkKEnergy() > 0 && MicrotpcSimHit.getEnergyDep() > 0) {
        old_trkID_o16[detNb] = trkID;
        //cout << "Output alpha track direction and vertex and momentum"<<endl;
        //cout << "Direction x " << direction.X() << " y " << direction.Y() << " z " << direction.Z()  << endl;
        //cout << "Vertex x " << position.X() << " y " << position.Y() << " z " << position.Z() << endl;
        //cout << "Kinetic energy " << MicrotpcSimHit.gettkKEnergy() << endl;
        RecoilE.push_back(MicrotpcSimHit.gettkKEnergy());
        atrk[detNb] = true;
        h_tpc_kin[2]->Fill(MicrotpcSimHit.gettkKEnergy());
        o16_ctr[detNb] ++;
      }
      h_tpc_xy[2]->Fill(xpos, ypos);
    }

    if (pdg == Const::proton.getPDGCode()) {
      //atrk[detNb] = true;
      //cout << "He4 detNb " << detNb << " trID " << trkID << endl;
      //cout << "Direction x " << direction.X() << " y " << direction.Y() << " z " << direction.Z()  << endl;
      //cout << "Vertex x " << position.X() << " y " << position.Y() << " z " << position.Z() << endl;
      //cout << "Kinetic energy " << MicrotpcSimHit.gettkKEnergy() << endl;
      if (old_trkID_h1[detNb] != trkID && MicrotpcSimHit.gettkKEnergy() > 0 && MicrotpcSimHit.getEnergyDep() > 0) {
        old_trkID_h1[detNb] = trkID;
        //cout << "Output alpha track direction and vertex and momentum"<<endl;
        //cout << "Direction x " << direction.X() << " y " << direction.Y() << " z " << direction.Z()  << endl;
        //cout << "Vertex x " << position.X() << " y " << position.Y() << " z " << position.Z() << endl;
        //cout << "Kinetic energy " << MicrotpcSimHit.gettkKEnergy() << endl;
        apro[detNb] = true;
        h_tpc_kin[3]->Fill(MicrotpcSimHit.gettkKEnergy());
        h1_ctr[detNb] ++;
      }
      h_tpc_xy[3]->Fill(xpos, ypos);
    }

    if (pdg == Const::neutron.getPDGCode()) {
      aneu[detNb] = true;
      h_tpc_kin[4]->Fill(MicrotpcSimHit.gettkKEnergy());
      h_tpc_xy[4]->Fill(xpos, ypos);
      n_ctr[detNb] ++;
    }

    if (pdg == Const::electron.getPDGCode()) {
      aele[detNb] = true;
      h_tpc_kin[5]->Fill(MicrotpcSimHit.gettkKEnergy());
      h_tpc_xy[5]->Fill(xpos, ypos);
    }

    if (pdg == -Const::electron.getPDGCode()) {
      apos[detNb] = true;
      h_tpc_kin[6]->Fill(MicrotpcSimHit.gettkKEnergy());
      h_tpc_xy[6]->Fill(xpos, ypos);
    }
  }
  for (int i = 0; i < 4; i ++) {
    if (apro[i] && atrk[i] && aneu[i]) co_ctr[i]++;
    if (apro[i] && atrk[i] && aneu[i] && aele[i]) coe_ctr[i]++;
    if (apro[i] && atrk[i]) {
      for (int j = 0; j < (int) RecoilE.size(); j ++) {
        h_tpc_kin[7]->Fill(RecoilE[j]);
      }
      ctr_pro[i] ++;
    }
    if (apro[i] && aneu[i]) ctr_bak[i] ++;
    if (atrk[i] && aneu[i]) {
      for (int j = 0; j < (int) RecoilE.size(); j ++) {
        h_tpc_kin[8]->Fill(RecoilE[j]);
      }
      ctr_neu[i] ++;
      if (NbofPart[i] == 1) ctr_good_neu[i] ++;
      if (NbofPart[i] > 1) ctr_bad_neu[i] ++;
    }
    if (atrk[i] && aele[i]) {
      for (int j = 0; j < (int) RecoilE.size(); j ++) {
        h_tpc_kin[9]->Fill(RecoilE[j]);
      }
      ctr_ele[i] ++;
    }
    if (atrk[i] && apos[i]) {
      for (int j = 0; j < (int) RecoilE.size(); j ++) {
        h_tpc_kin[10]->Fill(RecoilE[j]);
      }
      ctr_pos[i] ++;
    }
  }

  for (int i = 0; i < 4; i ++) {
    if (NbofPart[i] > 0) {
      cout << "det # " << i << " number of particle " << NbofPart[i] << endl;
      for (int j = 0; j < (int) Pdg[i].size(); j ++) {
        cout << " pdg " << Pdg[i][j] << endl;
      }
    }
  }

  ctr ++;
}
//read tube centers, impulse response, and garfield drift data filename from MICROTPC.xml
void TPCStudyModule::getXMLData()
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
void TPCStudyModule::endRun()
{

  //B2RESULT("TPCStudyModule: # of p recoils: " << npHits);
  //B2RESULT("TPCStudyModule: # of He recoils: " << nHeHits);
  //B2RESULT("TPCStudyModule: # of O recoils: " << nOHits);
  //B2RESULT("TPCStudyModule: # of C recoils: " << nCHits);
  cout << " Total nb of evts " << ctr << endl;
  for (int i = 0; i < 4; i ++) {
    cout << "n " << n_ctr[i] << " n-recoil-p " << co_ctr[i] << " n-recoil " << ctr_neu[i] << " p-n " << ctr_bak[i] << " p-He4 " <<
         ctr_pro[i] << " H1 " << h1_ctr[i] << " He4 " << he4_ctr[i] << " C12 " << c12_ctr[i] << " O16 " << o16_ctr[i] << " good n-recoil " <<
         ctr_good_neu[i] << " bad n-recoil " << ctr_bad_neu[i];
    cout << " w/ e- " << ctr_ele[i] << " w/ e+ " << ctr_pos[i] << " n-p-recoil-e- " << coe_ctr[i] << endl;
  }
}

void TPCStudyModule::terminate()
{
}


