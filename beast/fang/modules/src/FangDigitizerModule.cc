/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/fang/modules/FangDigitizerModule.h>
#include <beast/fang/dataobjects/FangSimHit.h>
#include <beast/fang/dataobjects/FangHit.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/core/RandomNumbers.h>

//c++
#include <cmath>
#include <boost/foreach.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

// ROOT
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>

using namespace std;
using namespace Belle2;
using namespace fang;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FangDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FangDigitizerModule::FangDigitizerModule() : Module()
{
  // Set module properties
  setDescription("Fang digitizer module");

  //Default values are set here. New values can be in FANG.xml.
  addParam("LookAtRec", m_LookAtRec, "Only look at nuclear recoils", 0);
  addParam("GEMGain1", m_GEMGain1, "GEM1 gain", 10.0);
  addParam("GEMGain2", m_GEMGain2, "GEM1 gain", 20.0);
  addParam("GainRMS1", m_GEMGainRMS1, "GEM1 rms", 0.2);
  addParam("GainRMS2", m_GEMGainRMS2, "GEM1 rms", 0.2);
  addParam("ScaleGain1", m_ScaleGain1, "scale gain 1 by a factor", 2.0);
  addParam("ScaleGain2", m_ScaleGain2, "scale gain 2 by a factor", 4.0);
  addParam("GEMpitch", m_GEMpitch, "GEM pitch", 0.014);
  addParam("PixelThreshold", m_PixelThreshold, "Pixel threshold in [e]", 3000);
  addParam("PixelThresholdRMS", m_PixelThresholdRMS, "Pixel threshold rms in [e]", 150);
  addParam("ChipRowNb", m_ChipRowNb, "Chip number of row", 226);
  addParam("ChipColumnNb", m_ChipColumnNb, "Chip number of column", 80);
  addParam("ChipColumnX", m_ChipColumnX, "Chip x dimension in cm / 2", 1.0);
  addParam("ChipRowY", m_ChipRowY, "Chip y dimension in cm / 2", 0.86);
  addParam("PixelTimeBinNb", m_PixelTimeBinNb, "Pixel number of time bin", 256);
  addParam("PixelTimeBin", m_PixelTimeBin, "Pixel time bin in ns", 25.0);
  addParam("TOTA1", m_TOTA1, "TOT factor A 1", 24.4678);
  addParam("TOTB1", m_TOTB1, "TOT factor B 1", -34.7008);
  addParam("TOTC1", m_TOTC1, "TOT factor C 1", 264.282);
  addParam("TOTQ1", m_TOTQ1, "TOT factor Q 1", 57.);
  addParam("TOTA2", m_TOTA2, "TOT factor A 2", 24.4678);
  addParam("TOTB2", m_TOTB2, "TOT factor B 2", -34.7008);
  addParam("TOTC2", m_TOTC2, "TOT factor C 2", 264.282);
  addParam("TOTQ2", m_TOTQ2, "TOT factor Q 2", 57.);
  addParam("z_DG", m_z_DG, "Drift gap distance [cm]", 12.0);
  addParam("z_TG", m_z_TG, "Transfer gap distance [cm]", 0.28);
  addParam("z_CG", m_z_CG, "Collection gap distance [cm]", 0.34);
  addParam("Dt_DG", m_Dt_DG, "Transverse diffusion in drift gap [cm^-1]", 0.0129286);
  addParam("Dt_TG", m_Dt_TG, "Transverse diffusion in transfer gap [cm^-1]", 0.0153198);
  addParam("Dt_CG", m_Dt_CG, "Transverse diffusion in collection gap [cm^1]", 0.014713);
  addParam("Dl_DG", m_Dl_DG, "Longitudinal diffusion in drift gap [cm^-1]", 0.0124361);
  addParam("Dl_TG", m_Dl_TG, "Longitudinal diffusion in transfer gap distance [cm^-1]", 0.0131141);
  addParam("Dl_CG", m_Dl_CG, "Longitudinal diffusion in collection gap [cm^-1]", 0.0134958);
  addParam("v_DG", m_v_DG, "Drift velocity in gap distance [cm/ns]", 0.00100675);
  addParam("v_TG", m_v_TG, "Drift velocity in transfer gap [cm/ns]", 0.0004079);
  addParam("v_CG", m_v_CG, "Drift velocity in collection gap [cm/ns]", 0.00038828);
  //addParam("P_vessel", m_P_vessel,"Pressure in vessel [atm]",1.0);
  addParam("Workfct", m_Workfct, "Work function", 35.075);
  addParam("Fanofac", m_Fanofac, "Fano factor", 0.19);
  addParam("GasAbs", m_GasAbs, "Gas absorption", 0.05);

}

FangDigitizerModule::~FangDigitizerModule()
{
}

void FangDigitizerModule::initialize()
{
  B2INFO("FangDigitizer: Initializing");
  StoreArray<FangHit>::registerPersistent();

  //get the garfield drift data, gas, and FANG paramters
  getXMLData();

  //converter: electron number to TOT part I
  fctToT_Calib1 = new TF1("fctToT_Calib1", "[0]*(x/[3]+[1])/(x/[3]+[2])", 0., 100000.);
  fctToT_Calib1->SetParameters(m_TOTA1, m_TOTB1, m_TOTC1, m_TOTQ1);
  //converter: electron number to TOT part II
  fctToT_Calib2 = new TF1("fctToT_Calib2", "[0]*(x/[3]+[1])/(x/[3]+[2])", 0., 100000.);
  fctToT_Calib2->SetParameters(m_TOTA2, m_TOTB2, m_TOTC2, m_TOTQ2);

}

void FangDigitizerModule::beginRun()
{
}

void FangDigitizerModule::event()
{

  StoreArray<MCParticle> particles;
  StoreArray<FangSimHit> FangSimHits;

  //Skip events with no FangSimHits, but continue the event counter
  if (FangSimHits.getEntries() == 0) {
    Event++;
    return;
  }

  //auto columnArray = new vector<int>[nfang](); //column
  //auto rowArray = new vector<int>[nfang](); //row
  //auto bcidArray = new vector<int>[nfang](); //BCID
  //auto totArray = new vector<int>[nfang](); //TOT
  /** Initialize 3D array */
  for (int i = 0; i < nfang; i++)
    for (int j = 0; j < 80; j++)
      for (int k = 0; k < 336; k++)
        for (int l = 0; l < MAXtSIZE; l++) {
          dchip[i][j][k][l] = 0;
          //partID[i][j][k][l] = 0;
        }
  //for (int i = 0; i < nfang; i++)
  //for (int j = 0; j < 100; j++)
  //partID[i][j] = 0

  //Determine T0 for each FANG
  int nentries = FangSimHits.getEntries();

  double T0[nfang];
  bool PixelFired[nfang];
  for (int i = 0; i < nfang; i++) {
    T0[i] = 1000000.;
    PixelFired[i] = false;
  }
  for (int i = 0; i < nentries; i++) {
    FangSimHit* aHit = FangSimHits[i];
    int detNb = aHit->getdetNb();
    TVector3 posn = aHit->gettkPos() ;
    //double T = posn.Z() / 100. + (FANGCenter[detNb].Z() + 10.) + m_z_DG;
    double T = posn.Z() / 100. - FANGCenter[detNb].Z() + m_z_DG;
    if (T < T0[detNb])T0[detNb] = T;
  }
  //int iPID[nfang];
  for (int i = 0; i < nfang; i++) {
    //iPID[i]=0;
    if (0. < T0[i] && T0[i] < 1000000.) {
      T0[i] = T0[i] / m_v_DG;
    } else T0[i] = -1.;
  }

  //int oldPID=0;
  //loop on all entries to store in 3D the ionization for each FANG
  for (int i = 0; i < nentries; i++) {
    FangSimHit* aHit = FangSimHits[i];

    int PDGid = aHit->gettkPDG();
    if (m_LookAtRec == 1)
      if (PDGid != 1000020040 || PDGid != 1000060120 || PDGid != 1000080160 || PDGid != 2212)continue;
    //if(oldPID!=PDGid && iPID[i]<100){
    //partID[detNb][iPID[i]]=PDGid;
    //oldPID = PDGid;
    //iPID[i]++;
    //}

    int detNb = aHit->getdetNb();
    double edep = aHit->getEnergyDep();
    double niel = aHit->getEnergyNiel();

    TVector3 position = aHit->gettkPos();
    double xpos = position.X() / 100. - FANGCenter[detNb].X();
    double ypos = position.Y() / 100. - FANGCenter[detNb].Y();
    //double zpos = position.Z() / 100. + (FANGCenter[detNb].Z() + 10) + m_z_DG;
    double zpos = position.Z() / 100. - FANGCenter[detNb].Z() + m_z_DG;
    //cout <<"xpos " << xpos << " ypos " << ypos << " zpos " << zpos << endl;
    //check if ionization within sensitive volume
    if ((-m_ChipColumnX < xpos && xpos < m_ChipColumnX) &&
        (-m_ChipRowY < ypos && ypos <  m_ChipRowY) &&
        (0. < zpos && zpos <  m_z_DG) &&
        (0. < T0[detNb] && T0[detNb] < 1000000.)) {

      //ionization energy
      //MeV -> keV
      double ionEn = (edep - niel) * 1e3;
      // check if enough energy to ionize if not break
      // keV -> eV
      if ((ionEn * 1e3) <  m_Workfct) break;

      ////////////////////////////////
      // check if enough energy to ionize
      else if ((ionEn * 1e3) >  m_Workfct) {
        //cout << "edep " << ionEn << endl;
        double meanEl = ionEn * 1e3 /  m_Workfct;
        double sigma = sqrt(m_Fanofac * meanEl);
        int NbEle = (int)gRandom->Gaus(meanEl, sigma);
        double  NbEle_real = 0;
        NbEle_real  = NbEle - NbEle * m_GasAbs * zpos;
        // start loop on the number of electron-ion-pairs at each interaction point
        for (int ie = 0; ie < (int)NbEle_real; ie++) {
          double x_DG, y_DG, z_DG, t_DG;
          //drift ionization to GEM 1 plane
          Drift(xpos,
                ypos,
                zpos,
                x_DG, y_DG, z_DG, t_DG, m_Dt_DG, m_Dl_DG, m_v_DG);
          //calculate and scale 1st GEM gain
          double GEM_gain1 = gRandom->Gaus(m_GEMGain1, m_GEMGain1 * m_GEMGainRMS1) / m_ScaleGain1;
          //calculate and scale 2nd GEM gain
          double GEM_gain2 = gRandom->Gaus(m_GEMGain2, m_GEMGain2 * m_GEMGainRMS2) / m_ScaleGain2;

          ///////////////////////////////
          // start loop on amplification
          for (int ig1 = 0; ig1 < (int)GEM_gain1; ig1++) {
            double x_GEM1, y_GEM1;
            //1st GEM geometrical effect
            GEMGeo1(x_DG, y_DG, x_GEM1, y_GEM1);
            double x_TG, y_TG, z_TG, t_TG;
            //drift 1st amplication to 2nd GEM
            Drift(x_GEM1, y_GEM1, m_z_TG, x_TG, y_TG, z_TG, t_TG, m_Dt_TG, m_Dl_TG, m_v_TG);

            ///////////////////////////////
            // start loop on amplification
            for (int ig2 = 0; ig2 < (int)GEM_gain2; ig2++) {
              double x_GEM2, y_GEM2;
              //2nd GEN geometrical effect
              GEMGeo2(x_TG, y_TG, x_GEM2, y_GEM2);
              double x_CG, y_CG, z_CG, t_CG;
              //drift 2nd amplification to chip
              Drift(x_GEM2, y_GEM2, m_z_CG, x_CG, y_CG, z_CG, t_CG, m_Dt_CG, m_Dl_CG, m_v_CG);

              //determine col, row, and bc
              int col = (int)((x_CG + m_ChipColumnX) / (2. * m_ChipColumnX / (double)m_ChipColumnNb));
              int row = (int)((y_CG + m_ChipRowY) / (2. * m_ChipRowY / (double)m_ChipRowNb));
              int pix = col +  m_ChipColumnNb * row;
              int quT = gRandom->Uniform(-1, 1);
              int bci = (int)((t_DG + t_TG + t_CG - T0[detNb]) / (double)m_PixelTimeBin) + quT;
              if (bci < 0)bci = 0;

              //check if amplified drifted electron are within pixel boundaries
              if ((0 <= col && col < m_ChipColumnNb) &&
                  (0 <= row && row < m_ChipRowNb) &&
                  (0 <= pix && pix < m_ChipColumnNb * m_ChipRowNb)  &&
                  (0 <= bci && bci < MAXtSIZE)) {
                PixelFired[detNb] = true;
                //store info into 3D array for each FANGs
                dchip[detNb][col][row][bci] += (int)(m_ScaleGain1 * m_ScaleGain2);
                //partID[detNb][col][row][bci] = PDGid;
              }
            }
          }
        }
      }
    }
  }

  //Pixelization of the 3D ionization cloud
  for (int i = 0; i < nfang; i++) {
    if (0. < T0[i] && T0[i] < 1000000. && PixelFired[i]) {
      Pixelization(i);
    }
  }

  Event++;

}
//Make the ionization drifting from (x,y,z) to GEM1 top plane
void FangDigitizerModule::Drift(double x1, double y1, double z1, double& x2, double& y2, double& z2, double& t2, double st,
                                double sl, double vd)
{
  //check if
  if (z1 > 0.) {
    //transverse diffusion
    x2 = x1 + gRandom->Gaus(0., sqrt(z1) * st);
    //transverse diffusion
    y2 = y1 + gRandom->Gaus(0., sqrt(z1) * st);
    //longitidinal diffusion
    z2 = z1 + gRandom->Gaus(0., sqrt(z1) * sl);
    //time to diffuse
    t2 = z2 / vd;
  } else {
    x2 = -1000; y2 = -1000; z2 = -1000; t2 = -1000;
  }
}
//Make GEMization of GEM1
void FangDigitizerModule::GEMGeo1(double x1, double y1, double& x2, double& y2)
{
  y2 = (int)(y1 / (sqrt(3. / 4.) * m_GEMpitch) + (y1 < 0 ? -0.5 : 0.5)) * sqrt(3. / 4.) * m_GEMpitch;
  int yint  = (int)(y1 / (sqrt(3. / 4.) * m_GEMpitch) + 0.5);
  if (yint % 2)
    x2 =  static_cast<int>(x1 / m_GEMpitch + (x1 < 0 ? -0.5 : 0.5)) * m_GEMpitch;
  else
    //everysecond row is shifted with half a pitch
    x2 = (static_cast<int>(x1 / m_GEMpitch) + (x1 < 0 ? -0.5 : 0.5)) * m_GEMpitch;
}
//Make GEMization of GEM2
void FangDigitizerModule::GEMGeo2(double x1, double y1, double& x2, double& y2)
{
  x2 = (int)(x1 / (sqrt(3. / 4.) * m_GEMpitch) + (x1 < 0 ? -0.5 : 0.5)) * sqrt(3. / 4.) * m_GEMpitch;
  int yint  = (int)(x1 / (sqrt(3. / 4.) * m_GEMpitch) + 0.5);
  if (yint % 2)
    y2 =  static_cast<int>(y1 / m_GEMpitch + (y1 < 0 ? -0.5 : 0.5)) * m_GEMpitch;
  else
    //everysecond row is shifted with half a pitch
    y2 = (static_cast<int>(y1 / m_GEMpitch) + (y1 < 0 ? -0.5 : 0.5)) * m_GEMpitch;
}
//Make pixel hit
bool FangDigitizerModule::Pixelization(int detNb)
{
  vector <int> t0; t0.clear();
  vector <int> col; col.clear();
  vector <int> row; row.clear();
  vector <int> ToT; ToT.clear();
  vector <int> bci; bci.clear();
  vector <int> ran; ran.clear();
  //vector <int> pid; pid.clear();
  //int pid[100];
  //for(int i=0;i<20;i++)pid[i]=partID[detNb][i]
  StoreArray<FangHit> FangHits;

  //loop on col.
  for (int i = 0; i < (int)m_ChipColumnNb; i++) {
    //loop on row
    for (int j = 0; j < (int)m_ChipRowNb; j++) {
      int k0 = -10;
      int quE = gRandom->Uniform(0, 2);
      double thresEl = m_PixelThreshold + gRandom->Uniform(-1.*m_PixelThresholdRMS, 1.*m_PixelThresholdRMS);
      //determined t0 ie first time above pixel threshold
      for (int k = 0; k < MAXtSIZE; k++) {
        if (dchip[detNb][i][j][k] > thresEl) {
          k0 = k;
          break;
        }
      }
      //determined nb of bc per pixel

      //if good t0
      if (k0 != -10) {
        int ik = 0;
        int NbOfEl = 0;
        for (int k = k0; k < MAXtSIZE; k++) {
          //sum up charge with 16 cycles
          if (ik < 16) {
            NbOfEl += dchip[detNb][i][j][k];
          } else {
            //calculate ToT
            int tot = -1;
            //tot = (int) ( (NbOfEl - iPixelThreshold[0]) * PixelGAIN) + quE;
            if (NbOfEl > thresEl && NbOfEl <= 45.*m_TOTQ1)
              tot = (int)fctToT_Calib1->Eval((double)NbOfEl) + quE;
            else if (NbOfEl > 45.*m_TOTQ1 && NbOfEl <= 900.*m_TOTQ1)
              tot = (int)fctToT_Calib2->Eval((double)NbOfEl);
            else if (NbOfEl > 800.*m_TOTQ1)
              tot = 14;
            if (tot > 13)tot = 14;
            if (tot >= 0) {
              int nbofel = NbOfEl;
              ran.push_back(nbofel);
              ToT.push_back(tot);

              //find start time
              t0.push_back(k0);
              //pid.push_back(partID[detNb][i][j][k]);
              col.push_back(i);
              row.push_back(j);
              bci.push_back(k0);
            }
            ik = 0;
            NbOfEl = 0;
          }
          ik++;
        }
      }
    }
    //end loop on row
  }
  // end loop on col
  bool PixHit = false;
  //if entry
  if (bci.size() > 0) {
    PixHit = true;
    //find start time
    sort(t0.begin(), t0.end());

    //loop on nb of hit
    for (int i = 0; i < (int)bci.size(); i++) {

      if ((bci[i] - t0[0]) > (m_PixelTimeBinNb - 1))continue;

      //create FangHit
      //FangHits.appendNew(FangHit(col[i], row[i], bci[i] - t0[0], ToT[i], detNb, pid[i]));
      FangHits.appendNew(FangHit(col[i], row[i], bci[i] - t0[0], ToT[i], detNb));
    }
    //end on loop nb of hit
  }
  //end if entry

  return PixHit;
}

//read tube centers, impulse response, and garfield drift data filename from FANG.xml
void FangDigitizerModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"FANG\"]/Content/");

  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

    FANGCenter.push_back(TVector3(activeParams.getLength("FANGpos_x"), activeParams.getLength("FANGpos_y"),
                                  activeParams.getLength("FANGpos_z")));
    nfang++;
  }
  m_LookAtRec = content.getDouble("LookAtRec");
  m_GEMGain1 = content.getDouble("GEMGain1");
  m_GEMGain2 = content.getDouble("GEMGain2");
  m_GEMGainRMS1 = content.getDouble("GEMGainRMS1");
  m_GEMGainRMS2 = content.getDouble("GEMGainRMS2");
  m_ScaleGain1 = content.getDouble("ScaleGain1");
  m_ScaleGain2 = content.getDouble("ScaleGain2");
  m_GEMpitch = content.getDouble("GEMpitch");
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
  m_z_DG = content.getDouble("z_DG");
  m_z_TG = content.getDouble("z_TG");
  m_z_CG = content.getDouble("z_CG");
  m_Dl_DG = content.getDouble("Dl_DG");
  m_Dl_TG = content.getDouble("Dl_TG");
  m_Dl_CG = content.getDouble("Dl_CG");
  m_Dt_DG = content.getDouble("Dt_DG");
  m_Dt_TG = content.getDouble("Dt_TG");
  m_Dt_CG = content.getDouble("Dt_CG");
  m_v_DG = content.getDouble("v_DG");
  m_v_TG = content.getDouble("v_TG");
  m_v_CG = content.getDouble("v_CG");
  m_Workfct = content.getDouble("Workfct");
  m_Fanofac = content.getDouble("Fanofac");
  m_GasAbs = content.getDouble("GasAbs");

  B2INFO("FangDigitizer: Aquired fang locations and gas parameters");
  B2INFO("              from FANG.xml. There are " << nfang << " FANGs implemented");

}

void FangDigitizerModule::endRun()
{
}

void FangDigitizerModule::terminate()
{
}


