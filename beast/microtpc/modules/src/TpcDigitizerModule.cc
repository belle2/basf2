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

#include <mdst/dataobjects/MCParticle.h>
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
#include <vector>

// ROOT
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>


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

  //Default values are set here. New values can be in MICROTPC.xml.
  addParam("LowerTimingCut", m_lowerTimingCut, "Lower timing cut", 0.);
  addParam("UpperTimingCut", m_upperTimingCut, "Upper timing cut", 1000000.);
}

TpcDigitizerModule::~TpcDigitizerModule()
{
}

void TpcDigitizerModule::initialize()
{
  B2INFO("TpcDigitizer: Initializing");
  m_microtpcHit.registerInDataStore();

  //get xml data
  getXMLData();

  //converter: electron number to TOT part I
  fctToT_Calib1 = new TF1("fctToT_Calib1", "[0]*(x/[3]+[1])/(x/[3]+[2])", 0., 100000.);
  fctToT_Calib1->SetParameters(m_TOTA1, m_TOTB1, m_TOTC1, m_TOTQ1);
  //converter: electron number to TOT part II
  fctToT_Calib2 = new TF1("fctToT_Calib2", "[0]*(x/[3]+[1])/(x/[3]+[2])", 0., 100000.);
  fctToT_Calib2->SetParameters(m_TOTA2, m_TOTB2, m_TOTC2, m_TOTQ2);
  /*
  for (int i = 0; i < m_nTPC; i++) {
    for (int j = 0; j < 80; j++) {
      for (int k = 0; k < 336; k++) {
        for (int l = 0; l < MAXtSIZE; l++) {
          m_dchip[i][j][k][l] = 0;
          //dchip[j][k][l] = 0;
        }
      }
    }
  }
  */
}

void TpcDigitizerModule::beginRun()
{
}

void TpcDigitizerModule::event()
{
  StoreArray<MCParticle> mcParticles;
  StoreArray<MicrotpcSimHit> microtpcSimHits;

  m_dchip_map.clear();
  m_dchip.clear();
  m_dchip_detNb_map.clear();
  m_dchip_pdg_map.clear();
  m_dchip_trkID_map.clear();

  std::vector<double> T0(m_nTPC,
                         m_upperTimingCut);  // TODO: why this number? Maybe pick something larger the the upperTiming cut? e.g. m_upperTimingCut + 1
  //std::vector<bool> PixelFired(m_nTPC, false);

  for (const auto& microtpcSimHit : microtpcSimHits) {
    const int detNb = microtpcSimHit.getdetNb();
    const TVector3 simHitPosition = microtpcSimHit.gettkPos();
    TVector3 chipPosition(
      simHitPosition.X() / 100. - m_TPCCenter[detNb].X(),
      simHitPosition.Y() / 100. - m_TPCCenter[detNb].Y(),
      simHitPosition.Z() / 100. - m_TPCCenter[detNb].Z()
    );
    chipPosition.RotateZ(-m_TPCAngleZ[detNb] * TMath::DegToRad());
    chipPosition.RotateX(-m_TPCAngleX[detNb] * TMath::DegToRad());
    const double T = chipPosition.Z() + m_z_DG / 2.;
    if (T < T0[detNb]) {
      T0[detNb] = T;
    }
  }

  for (auto& val : T0) {
    if (m_lowerTimingCut < val && val < m_upperTimingCut) {
      val = val / m_v_DG;
    } else {
      val = -1.;
    }
  }
  olddetNb = -1;
  oldtrkID = -1;
  //loop on all entries to store in 3D the ionization for each TPC
  for (const auto& microtpcSimHit : microtpcSimHits) {

    const int PDGid = microtpcSimHit.gettkPDG();
    if (m_LookAtRec == 1) {
      if (PDGid != 1000020040 || PDGid != 1000060120 || PDGid != 1000080160 || PDGid != 2212) {
        continue;
      }
    }

    const int detNb = microtpcSimHit.getdetNb();
    const double edep = microtpcSimHit.getEnergyDep();
    const double niel = microtpcSimHit.getEnergyNiel();
    const int pdg = microtpcSimHit.gettkPDG();
    const int trkID = microtpcSimHit.gettkID();

    const TVector3 simHitPosition = microtpcSimHit.gettkPos();
    TVector3 chipPosition(
      simHitPosition.X() / 100. - m_TPCCenter[detNb].X(),
      simHitPosition.Y() / 100. - m_TPCCenter[detNb].Y(),
      simHitPosition.Z() / 100. - m_TPCCenter[detNb].Z()
    );
    chipPosition.RotateZ(-m_TPCAngleZ[detNb] * TMath::DegToRad());
    chipPosition.RotateX(-m_TPCAngleX[detNb] * TMath::DegToRad());

    TVector3 ChipPosition(0, 0, 0);
    if (m_phase == 1) {
      if (detNb == 0 || detNb == 3) {
        ChipPosition.SetX(-chipPosition.Y());
        ChipPosition.SetY(-chipPosition.X());
        ChipPosition.SetZ(chipPosition.Z() + m_z_DG / 2.);
      }
      if (detNb == 1 || detNb == 2) {
        ChipPosition.SetX(chipPosition.Y());
        ChipPosition.SetY(chipPosition.X());
        ChipPosition.SetZ(chipPosition.Z() + m_z_DG / 2.);
      }
    }
    if (m_phase == 2) {
      ChipPosition.SetX(chipPosition.Y());
      ChipPosition.SetY(chipPosition.X());
      ChipPosition.SetZ(chipPosition.Z() + m_z_DG / 2);
    }


    //If new detector filled the chip
    if (olddetNb != detNb && m_dchip_map.size() > 0 && m_dchip.size() < 20000 && oldtrkID != trkID) {
      Pixelization();
      olddetNb = detNb;
      oldtrkID = trkID;
      m_dchip_map.clear();
      m_dchip.clear();
      m_dchip_detNb_map.clear();
      m_dchip_pdg_map.clear();
      m_dchip_trkID_map.clear();
    }

    //check if ionization within sensitive volume
    if ((-m_ChipColumnX < ChipPosition.X() && ChipPosition.X() < m_ChipColumnX) &&
        (-m_ChipRowY < ChipPosition.Y() && ChipPosition.Y() <  m_ChipRowY) &&
        (0. < ChipPosition.Z() && ChipPosition.Z() <  m_z_DG) &&
        (m_lowerTimingCut < T0[detNb] && T0[detNb] < m_upperTimingCut)) {

      //ionization energy
      //MeV -> keV
      const double ionEn = (edep - niel) * 1e3; // TODO: Use Unit constants instead of self made magic numbers
      // check if enough energy to ionize if not break
      // keV -> eV

      //if ((ionEn * 1e3) <  m_Workfct) continue; // TODO: Use Unit constants instead of self made magic numbers
      ////////////////////////////////
      // check if enough energy to ionize
      //else if ((ionEn * 1e3) >  m_Workfct) { // TODO: Use Unit constants instead of self made magic numbers
      if ((ionEn * 1e3) >  m_Workfct) { // TODO: Use Unit constants instead of self made magic numbers

        const double meanEl = ionEn * 1e3 /  m_Workfct;
        const double sigma = sqrt(m_Fanofac * meanEl);
        const int NbEle = (int)gRandom->Gaus(meanEl, sigma);
        const double NbEle_real = NbEle - NbEle * m_GasAbs * chipPosition.Z();

        // start loop on the number of electron-ion-pairs at each interaction point
        for (int ie = 0; ie < (int)NbEle_real; ie++) {

          //drift ionization to GEM 1 plane
          //const TLorentzVector driftGap(Drift(chipPosition.X(), chipPosition.Y(), chipPosition.Z(), m_Dt_DG, m_Dl_DG, m_v_DG));
          double x_DG, y_DG, z_DG, t_DG;
          Drift(ChipPosition.X(),
                ChipPosition.Y(),
                ChipPosition.Z(),
                x_DG, y_DG, z_DG, t_DG, m_Dt_DG, m_Dl_DG, m_v_DG);

          //calculate and scale 1st GEM gain
          const double GEM_gain1 = gRandom->Gaus(m_GEMGain1, m_GEMGain1 * m_GEMGainRMS1) / m_ScaleGain1;

          //calculate and scale 2nd GEM gain
          const double GEM_gain2 = gRandom->Gaus(m_GEMGain2, m_GEMGain2 * m_GEMGainRMS2) / m_ScaleGain2;

          ///////////////////////////////
          // start loop on amplification
          for (int ig1 = 0; ig1 < (int)GEM_gain1; ig1++) {
            //1st GEM geometrical effect
            //const TVector2 GEM1(GEMGeo1(driftGap.X(), driftGap.Y()));
            const TVector2 GEM1(GEMGeo1(x_DG, y_DG));
            //drift 1st amplication to 2nd GEM
            //const TLorentzVector transferGap(Drift(GEM1.X(), GEM1.Y(), m_z_TG, m_Dt_TG, m_Dl_TG, m_v_TG));
            double x_TG, y_TG, z_TG, t_TG;
            Drift(GEM1.X(), GEM1.Y(), m_z_TG, x_TG, y_TG, z_TG, t_TG, m_Dt_TG, m_Dl_TG, m_v_TG);

            ///////////////////////////////
            // start loop on amplification
            for (int ig2 = 0; ig2 < (int)GEM_gain2; ig2++) {
              //2nd GEN geometrical effect
              //const TVector2 GEM2(GEMGeo2(transferGap.X(), transferGap.Y()));
              const TVector2 GEM2(GEMGeo2(x_TG, y_TG));
              //drift 2nd amplification to chip
              //const TLorentzVector collectionGap(Drift(GEM2.X(), GEM2.Y(), m_z_CG, m_Dt_CG, m_Dl_CG, m_v_CG));
              double x_CG, y_CG, z_CG, t_CG;
              Drift(GEM2.X(), GEM2.Y(), m_z_CG, x_CG, y_CG, z_CG, t_CG, m_Dt_CG, m_Dl_CG, m_v_CG);

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
                //PixelFired[detNb] = true;
                //store info into 3D array for each TPCs
                ////m_dchip[detNb][col][row][bci] += (int)(m_ScaleGain1 * m_ScaleGain2);
                //m_dchip_map[std::tuple<int, int, int>(detNb, col, row)] = 1;
                //m_dchip[std::tuple<int, int, int, int>(detNb, col, row, bci)] += (int)(m_ScaleGain1 * m_ScaleGain2);
                m_dchip_map[std::tuple<int, int>(col, row)] = 1;
                m_dchip_detNb_map[std::tuple<int, int>(col, row)] = detNb;
                m_dchip_pdg_map[std::tuple<int, int>(col, row)] = pdg;
                m_dchip_trkID_map[std::tuple<int, int>(col, row)] = trkID;
                m_dchip[std::tuple<int, int, int>(col, row, bci)] += (int)(m_ScaleGain1 * m_ScaleGain2);
              }
            }
          }
        }
      }
    }
  }

  //Pixelization of the 3D ionization cloud
  /*for (int i = 0; i < m_nTPC; i++) {
    if (m_lowerTimingCut < T0[i] && T0[i] < m_upperTimingCut && PixelFired[i]) {
      Pixelization(i);

      for (int j = 0; j < 80; j++) {
        for (int k = 0; k < 336; k++) {
          for (int l = 0; l < MAXtSIZE; l++) {
            m_dchip[i][j][k][l] = 0;
          }
        }
      }

    }
  }
  */
  if (m_dchip_map.size() > 0 && m_dchip.size() < 20000) {
    //std::cout << " event size " << m_dchip_map.size() << " all " << m_dchip.size() << std::endl;
    Pixelization();
  }
  m_dchip_map.clear();
  m_dchip.clear();
  m_dchip_detNb_map.clear();
  m_dchip_pdg_map.clear();
  m_dchip_trkID_map.clear();
}
/*
TLorentzVector TpcDigitizerModule::Drift(
  double x1, double y1, double z1,
  double st, double sl, double vd
)
{
  double x2 = 0;
  double y2 = 0;
  double z2 = 0;
  double t2 = 0;
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
  return TLorentzVector(x2, y2, z2, t2);
}
*/
void TpcDigitizerModule::Drift(double x1, double y1, double z1, double& x2, double& y2, double& z2, double& t2, double st,
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
TVector2 TpcDigitizerModule::GEMGeo1(double x1, double y1)
{
  static const double sqrt3o4 = std::sqrt(3. / 4.);
  double x2 = 0;
  double y2 = (int)(y1 / (sqrt3o4 * m_GEMpitch) + (y1 < 0 ? -0.5 : 0.5)) * sqrt3o4 * m_GEMpitch;
  int yint  = (int)(y1 / (sqrt3o4 * m_GEMpitch) + 0.5);
  if (yint % 2) {
    x2 =  static_cast<int>(x1 / m_GEMpitch + (x1 < 0 ? -0.5 : 0.5)) * m_GEMpitch;
  } else {
    //everysecond row is shifted with half a pitch
    x2 = (static_cast<int>(x1 / m_GEMpitch) + (x1 < 0 ? -0.5 : 0.5)) * m_GEMpitch;
  }
  return TVector2(x2, y2);
}

TVector2 TpcDigitizerModule::GEMGeo2(double x1, double y1)
{
  static const double sqrt3o4 = std::sqrt(3. / 4.);
  double x2 = (int)(x1 / (sqrt3o4 * m_GEMpitch) + (x1 < 0 ? -0.5 : 0.5)) * sqrt3o4 * m_GEMpitch;
  double y2 = 0;
  int yint  = (int)(x1 / (sqrt3o4 * m_GEMpitch) + 0.5);
  if (yint % 2) {
    y2 =  static_cast<int>(y1 / m_GEMpitch + (y1 < 0 ? -0.5 : 0.5)) * m_GEMpitch;
  } else {
    //everysecond row is shifted with half a pitch
    y2 = (static_cast<int>(y1 / m_GEMpitch) + (y1 < 0 ? -0.5 : 0.5)) * m_GEMpitch;
  }
  return TVector2(x2, y2);
}





//bool TpcDigitizerModule::Pixelization(int detNb)
void TpcDigitizerModule::Pixelization()
{
  std::vector<int> t0;
  std::vector<int> col;
  std::vector<int> row;
  std::vector<int> ToT;
  std::vector<int> bci;
  t0.clear();
  col.clear();
  row.clear();
  ToT.clear();
  bci.clear();
  StoreArray<MicrotpcHit> microtpcHits;

  for (auto& keyValuePair : m_dchip_map) {
    const auto& key = keyValuePair.first;
    //column
    int i = std::get<0>(key);
    //raw
    int j = std::get<1>(key);

    if (m_dchip_map[std::tuple<int, int>(i, j)] == 1) {

      int k0 = 1e9;
      const int quE = gRandom->Uniform(0, 2);
      const double thresEl = m_PixelThreshold + gRandom->Uniform(-1.*m_PixelThresholdRMS, 1.*m_PixelThresholdRMS);
      int kcounter = 0;
      //determined t0 ie first time above pixel threshold
      for (auto& keyValuePair2 : m_dchip) {
        const auto& key2 = keyValuePair2.first;
        int k = std::get<2>(key2);
        if (m_dchip[std::tuple<int, int, int>(i, j, k)] > thresEl) {
          if (k0 > k)k0 = k;
          kcounter ++;
        }
      }
      //std::cout<<"kcounter " << kcounter << std::endl;
      //determined nb of bc per pixel
      //if good t0
      if (k0 != 1e9) {
        int ik = 0;
        int NbOfEl = 0;
        for (auto& keyValuePair2 : m_dchip) {
          const auto& key2 = keyValuePair2.first;
          int k = std::get<2>(key2);
          //sum up charge with 16 cycles
          if (ik < 16) {
            NbOfEl += m_dchip[std::tuple<int, int, int>(i, j, k)];
          } else {
            //calculate ToT
            int tot = -1;
            if (NbOfEl > thresEl && NbOfEl <= 45.*m_TOTQ1) {
              tot = (int)fctToT_Calib1->Eval((double)NbOfEl) + quE;
            } else if (NbOfEl > 45.*m_TOTQ1 && NbOfEl <= 900.*m_TOTQ1) {
              tot = (int)fctToT_Calib2->Eval((double)NbOfEl);
            } else if (NbOfEl > 800.*m_TOTQ1) {
              tot = 13;
            }
            if (tot > 12) {
              tot = 13;
            }
            if (tot >= 0) {
              ToT.push_back(tot);
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

        if (kcounter < 16) {
          //calculate ToT
          int tot = -1;
          if (NbOfEl > thresEl && NbOfEl <= 45.*m_TOTQ1) {
            tot = (int)fctToT_Calib1->Eval((double)NbOfEl) + quE;
          } else if (NbOfEl > 45.*m_TOTQ1 && NbOfEl <= 900.*m_TOTQ1) {
            tot = (int)fctToT_Calib2->Eval((double)NbOfEl);
          } else if (NbOfEl > 800.*m_TOTQ1) {
            tot = 13;
          }
          if (tot > 12) {
            tot = 13;
          }
          if (tot >= 0) {
            ToT.push_back(tot);
            t0.push_back(k0);
            col.push_back(i);
            row.push_back(j);
            bci.push_back(k0);
          }
        }

      }
    } //end loop on row
  } // end loop on col

  //bool PixHit = false;
  //if entry

  if (bci.size() > 0) {
    //std::cout << " size " << bci.size() << std::endl;
    //PixHit = true;
    //find start time
    sort(t0.begin(), t0.end());

    //loop on nb of hit
    for (int j = 0; j < (int)bci.size(); j++) {
      if ((bci[j] - t0[0]) > (m_PixelTimeBinNb - 1)) {
        continue;
      }
      //create MicrotpcHit
      microtpcHits.appendNew(MicrotpcHit(col[j] + 1, row[j] + 1, bci[j] - t0[0] + 1, ToT[j],
                                         m_dchip_detNb_map[std::tuple<int, int>(col[j], row[j])],
                                         m_dchip_pdg_map[std::tuple<int, int>(col[j], row[j])],
                                         m_dchip_trkID_map[std::tuple<int, int>(col[j], row[j])]));
    } //end if entry
  }
  //return PixHit;
  /*
  std::vector<int> t0[10];
  std::vector<int> col[10];
  std::vector<int> row[10];
  std::vector<int> ToT[10];
  std::vector<int> bci[10];

  StoreArray<MicrotpcHit> microtpcHits;

  for (auto& keyValuePair : m_dchip_map) {
    const auto& key = keyValuePair.first;
    //detector number
    int detNb = std::get<0>(key);
    //column
    int i = std::get<1>(key);
    //raw
    int j = std::get<2>(key);

    if (m_dchip_map[std::tuple<int, int, int>(detNb, i, j)] == 1) {

      int k0 = -10;
      const int quE = gRandom->Uniform(0, 2);
      const double thresEl = m_PixelThreshold + gRandom->Uniform(-1.*m_PixelThresholdRMS, 1.*m_PixelThresholdRMS);
      //determined t0 ie first time above pixel threshold
      //for (int k = 0; k < MAXtSIZE; k++) {
      for (auto& keyValuePair2 : m_dchip) {
        const auto& key2 = keyValuePair2.first;
        int k = std::get<3>(key2);
        if (m_dchip[std::tuple<int, int, int, int>(detNb, i, j, k)] > thresEl) {
          //if (m_dchip[detNb][i][j][k] > thresEl) {
          k0 = k;
          break;
        }
      }
      //determined nb of bc per pixel

      //if good t0
      if (k0 != -10) {
        int ik = 0;
        int NbOfEl = 0;
        //for (int k = k0; k < MAXtSIZE; k++) {
        for (auto& keyValuePair2 : m_dchip) {
          const auto& key2 = keyValuePair2.first;
          int k = std::get<3>(key2);
          //sum up charge with 16 cycles
          if (ik < 16) {
            NbOfEl += m_dchip[std::tuple<int, int, int, int>(detNb, i, j, k)];
            //NbOfEl += m_dchip[detNb][i][j][k];
          } else {
            //calculate ToT
            int tot = -1;
            if (NbOfEl > thresEl && NbOfEl <= 45.*m_TOTQ1) {
              tot = (int)fctToT_Calib1->Eval((double)NbOfEl) + quE;
            } else if (NbOfEl > 45.*m_TOTQ1 && NbOfEl <= 900.*m_TOTQ1) {
              tot = (int)fctToT_Calib2->Eval((double)NbOfEl);
            } else if (NbOfEl > 800.*m_TOTQ1) {
              tot = 14;
            }
            if (tot > 13) {
              tot = 14;
            }
            if (tot >= 0) {
              ToT[detNb].push_back(tot);
              t0[detNb].push_back(k0);
              col[detNb].push_back(i);
              row[detNb].push_back(j);
              bci[detNb].push_back(k0);
            }
            ik = 0;
            NbOfEl = 0;
          }
          ik++;
        }
      }
    } //end loop on row
    //for (int k = 0; k < MAXtSIZE; k++) m_dchip[detNb][i][j][k] = 0;
  } // end loop on col

  //bool PixHit = false;
  for (int i = 0; i < 10 ; i++) {
    //if entry
    if (bci[i].size() > 0) {
      //PixHit = true;
      //find start time
      sort(t0[i].begin(), t0[i].end());

      //loop on nb of hit
      for (int j = 0; j < (int)bci[i].size(); j++) {
        if ((bci[i][j] - t0[i][0]) > (m_PixelTimeBinNb - 1)) {
          continue;
        }
        //create MicrotpcHit
        microtpcHits.appendNew(MicrotpcHit(col[i][j], row[i][j], bci[i][j] - t0[i][0], ToT[i][j], i));
      } //end on loop nb of hit
    } //end if entry
  }
  //return PixHit;
  */
}

/*

bool TpcDigitizerModule::Pixelization(int detNb)
{
  std::vector<int> t0;
  std::vector<int> col;
  std::vector<int> row;
  std::vector<int> ToT;
  std::vector<int> bci;

  StoreArray<MicrotpcHit> microtpcHits;

  //loop on col.
  for (int i = 0; i < (int)m_ChipColumnNb; i++) {
    //loop on row
    for (int j = 0; j < (int)m_ChipRowNb; j++) {
      int k0 = -10;
      const int quE = gRandom->Uniform(0, 2);
      const double thresEl = m_PixelThreshold + gRandom->Uniform(-1.*m_PixelThresholdRMS, 1.*m_PixelThresholdRMS);
      //determined t0 ie first time above pixel threshold
      for (int k = 0; k < MAXtSIZE; k++) {
        //if (m_dchip[std::tuple<int, int, int, int>(detNb, i, j, k)] > thresEl) {
  if (m_dchip[detNb][i][j][k] > thresEl) {
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
            //NbOfEl += m_dchip[std::tuple<int, int, int, int>(detNb, i, j, k)];
      NbOfEl += m_dchip[detNb][i][j][k];
          } else {
            //calculate ToT
            int tot = -1;
            if (NbOfEl > thresEl && NbOfEl <= 45.*m_TOTQ1) {
              tot = (int)fctToT_Calib1->Eval((double)NbOfEl) + quE;
            } else if (NbOfEl > 45.*m_TOTQ1 && NbOfEl <= 900.*m_TOTQ1) {
              tot = (int)fctToT_Calib2->Eval((double)NbOfEl);
            } else if (NbOfEl > 800.*m_TOTQ1) {
              tot = 14;
            }
            if (tot > 13) {
              tot = 14;
            }
            if (tot >= 0) {
              ToT.push_back(tot);
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
    } //end loop on row
  } // end loop on col

  bool PixHit = false;
  //if entry
  if (bci.size() > 0) {
    PixHit = true;
    //find start time
    sort(t0.begin(), t0.end());

    //loop on nb of hit
    for (int i = 0; i < (int)bci.size(); i++) {
      if ((bci[i] - t0[0]) > (m_PixelTimeBinNb - 1)) {
        continue;
      }
      //create MicrotpcHit
      microtpcHits.appendNew(MicrotpcHit(col[i], row[i], bci[i] - t0[0], ToT[i], detNb));
    } //end on loop nb of hit
  } //end if entry

  return PixHit;
}
*/
//read tube centers, impulse response, and garfield drift data filename from MICROTPC.xml
void TpcDigitizerModule::getXMLData()
{
  //const GearDir& content;
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"MICROTPC\"]/Content/");

  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

    m_TPCCenter.push_back(TVector3(activeParams.getLength("SensVol_x"), activeParams.getLength("SensVol_y"),
                                   activeParams.getLength("SensVol_z")));
    m_TPCAngleX.push_back(activeParams.getLength("AngleX"));
    m_TPCAngleZ.push_back(activeParams.getLength("AngleZ"));

    m_nTPC++;
  }
  m_phase = content.getDouble("Phase");
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

  B2INFO("TpcDigitizer: Aquired tpc locations and gas parameters");
  B2INFO("              from MICROTPC.xml. There are " << m_nTPC << " TPCs implemented");

}

void TpcDigitizerModule::endRun()
{
}

void TpcDigitizerModule::terminate()
{
}


