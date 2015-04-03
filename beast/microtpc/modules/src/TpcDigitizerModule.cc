/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/microtpc/modules/TpcDigitizerModule.h>
#include <beast/microtpc/dataobjects/MicrotpcSimHit.h>
#include <beast/microtpc/dataobjects/MicrotpcHit.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>


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
using namespace microtpc;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TpcDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TpcDigitizerModule::TpcDigitizerModule() : Module()
{
  // Set module properties
  setDescription("Microtpc digitizer module");

  //Default values of these exist in MICROTPC.xml. If the user prefers, they can try different drift data files or impulse responses.
  addParam("GEMGain1", m_GEMGain1,
           "GEM1 gain. Default taken from MICROTPC.xml");
  addParam("GEMGain2", m_GEMGain2,
           "GEM1 gain. Default taken from MICROTPC.xml");
  addParam("GainRMS1", m_GEMGainRMS1,
           "GEM1 rms. Default taken from MICROTPC.xml");
  addParam("GainRMS2", m_GEMGainRMS2,
           "GEM1 rms. Default taken from MICROTPC.xml");
  addParam("ScaleGain1", m_ScaleGain1,
           "scale gain 1 by a factor. Default taken from MICROTPC.xml");
  addParam("ScaleGain2", m_ScaleGain2,
           "scale gain 2 by a factor. Default taken from MICROTPC.xml");
  addParam("GEMpitch", m_GEMpitch,
           "GEM pitch. Default taken from MICROTPC.xml");
  addParam("PixelThreshold", m_PixelThreshold,
           "Pixel threshold in [e]. Default taken from MICROTPC.xml");
  addParam("PixelThresholdRMS", m_PixelThresholdRMS,
           "Pixel threshold rms in [e]. Default taken from MICROTPC.xml");
  addParam("ChipRowNb", m_ChipRowNb,
           "Chip number of row. Default taken from MICROTPC.xml");
  addParam("ChipColumnNb", m_ChipColumnNb,
           "Chip number of column. Default taken from MICROTPC.xml");
  addParam("ChipColumnX", m_ChipColumnX,
           "Chip x dimension in cm / 2. Default taken from MICROTPC.xml");
  addParam("ChipRowY", m_ChipRowY,
           "Chip y dimension in cm / 2. Default taken from MICROTPC.xml");
  addParam("PixelTimeBinNb", m_PixelTimeBinNb,
           "Pixel number of time bin. Default taken from MICROTPC.xml");
  addParam("PixelTimeBin", m_PixelTimeBin,
           "Pixel time bin in ns. Default taken from MICROTPC.xml");
  addParam("TOTA1", m_TOTA1,
           "TOT factor A 1. Default taken from MICROTPC.xml");
  addParam("TOTB1", m_TOTB1,
           "TOT factor B 1. Default taken from MICROTPC.xml");
  addParam("TOTC1", m_TOTC1,
           "TOT factor C 1. Default taken from MICROTPC.xml");
  addParam("TOTQ1", m_TOTQ1,
           "TOT factor Q 1. Default taken from MICROTPC.xml");
  addParam("TOTA1", m_TOTA1,
           "TOT factor A 2. Default taken from MICROTPC.xml");
  addParam("TOTB1", m_TOTB1,
           "TOT factor B 2. Default taken from MICROTPC.xml");
  addParam("TOTC1", m_TOTC1,
           "TOT factor C 2. Default taken from MICROTPC.xml");
  addParam("TOTQ1", m_TOTQ1,
           "TOT factor Q 2. Default taken from MICROTPC.xml");
  addParam("z_DG", m_z_DG,
           "Drift gap distance [cm]. Default taken from MICROTPC.xml");
  addParam("z_TG", m_z_TG,
           "Transfer gap distance [cm]. Default taken from MICROTPC.xml");
  addParam("z_CG", m_z_CG,
           "Collection gap distance [cm]. Default taken from MICROTPC.xml");
  addParam("Dt_DG", m_Dt_DG,
           "Transverse diffusion in drift gap [cm^-1]. Default taken from MICROTPC.xml");
  addParam("Dt_TG", m_Dt_TG,
           "Transverse diffusion in transfer gap [cm^-1]. Default taken from MICROTPC.xml");
  addParam("Dt_CG", m_Dt_CG,
           "Transverse diffusion in collection gap [cm^1]. Default taken from MICROTPC.xml");
  addParam("Dl_DG", m_Dl_DG,
           "Longitudinal diffusion in drift gap [cm^-1]. Default taken from MICROTPC.xml");
  addParam("Dl_TG", m_Dl_TG,
           "Longitudinal diffusion in transfer gap distance [cm^-1]. Default taken from MICROTPC.xml");
  addParam("Dl_CG", m_Dl_CG,
           "Longitudinal diffusion in collection gap [cm^-1]. Default taken from MICROTPC.xml");
  addParam("v_DG", m_v_DG,
           "Drift velocity in gap distance [cm/ms]. Default taken from MICROTPC.xml");
  addParam("v_TG", m_v_TG,
           "Drift velocity in transfer gap [cm/ns]. Default taken from MICROTPC.xml");
  addParam("v_CG", m_v_CG,
           "Drift velocity in collection gap [cm/ns]. Default taken from MICROTPC.xml");
  addParam("P_vessel", m_P_vessel,
           "Pressure in vessel [atm]. Default taken from MICROTPC.xml");
  addParam("Workfct", m_Workfct,
           "Work function. Default taken from MICROTPC.xml");
  addParam("Fanofac", m_Fanofac,
           "Fano factor. Default taken from MICROTPC.xml");
  addParam("GasAbs", m_GasAbs,
           "Gas absorption. Default taken from MICROTPC.xml");

}

TpcDigitizerModule::~TpcDigitizerModule()
{
}

void TpcDigitizerModule::initialize()
{
  B2INFO("TpcDigitizer: Initializing");
  StoreArray<MicrotpcHit>::registerPersistent();

  //get the garfield drift data and create impulse response function
  getXMLData();
  if (m_driftDataFile != "") getDriftData();
  impulseResponse();

}

void TpcDigitizerModule::beginRun()
{
}

void TpcDigitizerModule::event()
{

  StoreArray<MCParticle> particles;
  StoreArray<MicrotpcSimHit> TpcSimHits;

  //Skip events with no TpcSimHits, but continue the event counter
  if (TpcSimHits.getEntries() == 0) {
    Event++;
    return;
  }

  //auto columnArray = new vector<int>[nTPC](); //column
  //auto rowArray = new vector<int>[nTPC](); //row
  //auto bcidArray = new vector<int>[nTPC](); //BCID
  //auto totArray = new vector<int>[nTPC](); //TOT

  //Determine T0 for each TPC
  int nentries = TpcSimHits.getEntries();
  double T0[nTPC];
  for (int i = 0; i < nTPC; i++) {
    T0[i] = 10000;
  }
  for (int i = 0; i < nentries; i++) {
    MicrotpcSimHit* aHit = TpcSimHits[i];
    int detNb = aHit->getdetNb();
    TVector3 posn = aHit->gettkPos();
    double T = posn.Z() / 100. - TPCCenter[detNb].Z() + m_z_DG;
    if (T < T0[detNb])T0[detNb] = T;
  }
  for (int i = 0; i < nTPC; i++) {
    T0[i] = T0[i] / m_v_DG - 1000.;
  }
  //loop on all entries to store in 3D the ionization for each TPC
  for (int i = 0; i < nentries; i++) {
    MicrotpcSimHit* aHit = TpcSimHits[i];
    int detNb = aHit->getdetNb();

    for (int i = 0; i < 80; i++)
      for (int j = 0; j < 336; j++)
        for (int k = 0; k < MAXtSIZE; k++)
          dchip[detNb][i][j][k] = 0;


    double edep = aHit->getEnergyDep();
    double niel = aHit->getEnergyNiel();
    TVector3 position = aHit->gettkPos();
    double xpos = position.X() - TPCCenter[detNb].X();
    double ypos = position.Y() - TPCCenter[detNb].Y();
    double zpos = position.Z() - TPCCenter[detNb].Z();
    if ((-m_ChipColumnX < xpos && xpos < m_ChipColumnX) &&
        (-m_ChipRowY < ypos && ypos <  m_ChipRowY) &&
        (0. < zpos && zpos <  m_z_DG)) {

      //ionization energy
      //MeV -> keV
      double ionEn = (edep - niel) * 1e3;

      // check if enough energy to ionize if not break
      // keV -> eV
      if ((ionEn * 1e3) <  m_Workfct) break;

      ////////////////////////////////
      // check if enough energy to ionize
      else if ((ionEn * 1e3) >  m_Workfct) {

        double meanEl = ionEn * 1e3 /  m_Workfct;
        double sigma = sqrt(m_Fanofac * meanEl);
        int NbEle = (int)fRandom->Gaus(meanEl, sigma);
        double  NbEle_real = 0;
        NbEle_real  = NbEle - NbEle * m_GasAbs * ypos;

        // start loop on the number of electron-ion-pairs at each interaction point
        for (int ie = 0; ie < (int)NbEle_real; ie++) {
          double x_DG, y_DG, z_DG, t_DG;
          Drift(xpos,
                ypos,
                zpos,
                x_DG, y_DG, z_DG, t_DG, m_Dt_DG, m_Dl_DG, m_v_DG);
          double GEM_gain1 = fRandom->Gaus(m_GEMGain1, m_GEMGain1 * m_GEMGainRMS1) / m_ScaleGain1;
          double GEM_gain2 = fRandom->Gaus(m_GEMGain2, m_GEMGain2 * m_GEMGainRMS2) / m_ScaleGain2;

          ///////////////////////////////
          // start loop on amplification
          for (int ig1 = 0; ig1 < (int)GEM_gain1; ig1++) {

            double x_GEM1, y_GEM1;
            GEMGeo1(x_DG, y_DG, x_GEM1, y_GEM1);

            double x_TG, y_TG, z_TG, t_TG;
            Drift(x_GEM1, y_GEM1, m_z_TG, x_TG, y_TG, z_TG, t_TG, m_Dt_TG, m_Dl_TG, m_v_TG);

            ///////////////////////////////
            // start loop on amplification
            for (int ig2 = 0; ig2 < (int)GEM_gain2; ig2++) {
              double x_GEM2, y_GEM2;
              GEMGeo2(x_TG, y_TG, x_GEM2, y_GEM2);

              double x_CG, y_CG, z_CG, t_CG;
              Drift(x_GEM2, y_GEM2, m_z_CG, x_CG, y_CG, z_CG, t_CG, m_Dt_CG, m_Dl_CG, m_v_CG);

              int col = (int)((x_CG + m_ChipColumnX) / (2. * m_ChipColumnX / m_ChipColumnNb));
              int row = (int)((y_CG + m_ChipRowY) / (2. * m_ChipRowY / m_ChipRowNb));
              int pix = col +  m_ChipColumnNb * row;
              int quT = fRandom->Uniform(-1, 1);
              int bci = (int)((t_DG + t_TG + t_CG - T0[detNb]) / m_PixelTimeBin) + quT;

              //check if amplified drifted electron are within pixel boundaries
              if ((0 <= col && col < m_ChipColumnNb) &&
                  (0 <= row && row < m_ChipRowNb) &&
                  (0 <= pix && pix < m_ChipColumnNb * m_ChipRowNb)  &&
                  (0 <= bci && bci < MAXtSIZE)) {
                //PixelFired = true;
                dchip[detNb][col][row][bci] += m_ScaleGain1 * m_ScaleGain2;
              }
            }
          }
        }
      }
    }
  }
  for (int i = 0; i < nTPC; i++) {
    if (T0[i] < 10000)
      Pixelization(i);
  }

  Event++;

}
//Make the ionization drifting from (x,y,z) to GEM1 top plane
void TpcDigitizerModule::Drift(double x1, double y1, double z1, double& x2, double& y2, double& z2, double& t2, double st,
                               double sl, double vd)
{
  //check if
  if (z1 > 0.) {
    x2 = x1 + fRandom->Gaus(0., sqrt(y1) * st);
    y2 = y1 + fRandom->Gaus(0., sqrt(y1) * st);
    z2 = z1 + fRandom->Gaus(0., sqrt(y1) * sl);
    t2 = y2 / vd;
  } else {
    x2 = -1000; y2 = -1000; z2 = -1000; t2 = -1000;
  }
}
//Make GEMization of GEM1
void TpcDigitizerModule::GEMGeo1(double x1, double y1, double& x2, double& y2)
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
void TpcDigitizerModule::GEMGeo2(double x1, double y1, double& x2, double& y2)
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
bool TpcDigitizerModule::Pixelization(int detNb)
{

  vector <int> t0; t0.clear();
  vector <int> col; col.clear();
  vector <int> row; row.clear();
  vector <int> ToT; ToT.clear();
  vector <int> bci; bci.clear();
  vector <int> ran; ran.clear();
  StoreArray<MicrotpcHit> TpcHits;

  //loop on col.
  for (int i = 0; i < (int)m_ChipColumnNb; i++) {
    //loop on row
    for (int j = 0; j < (int)m_ChipRowNb; j++) {
      int k0 = -10;
      int quE = fRandom->Uniform(0, 2);
      double thresEl = m_PixelThreshold + fRandom->Uniform(-1.*m_PixelThresholdRMS, 1.*m_PixelThresholdRMS);
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
            if (tot > 0) {
              int nbofel = NbOfEl;
              ran.push_back(nbofel);
              ToT.push_back(tot - 1);

              //find start time
              t0.push_back(k0);

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

      //create MicrotpcHit
      TpcHits.appendNew(MicrotpcHit(col[i], row[i], bci[i] - t0[0], ToT[i], detNb));
    }
    //end on loop nb of hit


  }
  //end if entry
  return PixHit;
}

//read tube centers, impulse response, and garfield drift data filename from MICROTPC.xml
void TpcDigitizerModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"MICROTPC\"]/Content/");

  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

    TPCCenter.push_back(TVector3(activeParams.getLength("x_microtpc"), activeParams.getLength("y_microtpc"),
                                 activeParams.getLength("z_microtpc")));
    nTPC++;
  }

  B2INFO("TpcDigitizer: Aquired tpc locations and gas parameters");
  B2INFO("              from MICROTPC.xml. There are " << nTPC << " tubes implemented");

}

void TpcDigitizerModule::endRun()
{
}

void TpcDigitizerModule::terminate()
{
}


