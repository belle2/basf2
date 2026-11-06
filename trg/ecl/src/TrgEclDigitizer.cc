/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLHit.h>

#include "trg/ecl/TrgEclDigitizer.h"


#include "trg//ecl/dataobjects/TRGECLDigi0.h"
#include "trg/ecl/dataobjects/TRGECLWaveform.h" // by shebalin 

#include <iostream>
#include <math.h>
#include <TRandom.h>

using namespace std;
using namespace Belle2;
//using namespace TRG;
//
//
//
TrgEclDigitizer::TrgEclDigitizer():
  m_TimeRange(0), m_SaveTCWaveForm(0), m_FADC(1), m_BeambkgTag(0)
{
  m_MatrixParallel.clear();
  m_MatrixSerial.clear();

  m_TCMap = new TrgEclMapping();
  m_DataBase = new TrgEclDataBase();

  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    m_TCEnergy_tot[iTCIdm] = 0;
    m_TCTiming_tot[iTCIdm] = 0;
    for (int  iTime = 0; iTime < 80; iTime++) {
      m_TCEnergy[iTCIdm][iTime] = 0;
      m_TCTiming[iTCIdm][iTime] = 0;
      m_TCBkgContribution[iTCIdm][iTime] = 0;
      m_TCSigContribution[iTCIdm][iTime] = 0;
      m_TCBeambkgTag[iTCIdm][iTime] = 0;
    }

    for (int iii = 0; iii < 60 ; iii++) {
      m_TCRawEnergy[iTCIdm][iii] = 0;
      m_TCRawTiming[iTCIdm][iii] = 0;
      m_TCRawBkgTag[iTCIdm][iii] = 0;
    }
    for (int iii = 0; iii < 64 ; iii++) {
      m_WaveForm[iTCIdm][iii] = 0;
    }
  }
}
//
//
//
TrgEclDigitizer::~TrgEclDigitizer()
{
  delete m_TCMap;
  delete m_DataBase;
}
//
//
//

void
TrgEclDigitizer::setup(int SourceOfTC)
{

  // prepare Matrix for Noise generation
  m_DataBase->readNoiseLMatrix(m_MatrixParallel, m_MatrixSerial);

  // Set TC data
  // SourceOfTC => 1=ECLHit, 2=ECLSimHit, 3=ECLHit+TRGECLBGTCHit
  // ("1:=ECLHit" is used for signal w/o bkg, and real time background monitor)
  getTCHit(SourceOfTC);

  return;
}
//
//
//
void
TrgEclDigitizer::getTCHit(int SourceOfTC)
{

  int nBinTime = 80;
  m_TimeRange = 4000; // -4us ~ 4us
  //-------------------------------------------------------------------
  //                          read Xtal data
  //---------------------------------------------------------------------
  if (SourceOfTC == 1) { // read  ECLHit table

    // only needed to resolve the dominant background tag below
    std::vector< std::vector<float> > E_cell(8736, std::vector<float>(80, 0.0));
    std::vector< std::vector<float> > beambkg_tag(8736, std::vector<float>(80, 0.0));

    StoreArray<ECLHit> eclHitArray("ECLHits");

    int nHits_hit = eclHitArray.getEntries();
    //
    for (int iHits = 0; iHits < nHits_hit; iHits++) {
      // Get a hit
      ECLHit* aECLHit = eclHitArray[iHits];
      int beambkg = aECLHit->getBackgroundTag();

      // Hit geom. info
      int hitCellId  = aECLHit->getCellId() - 1;
      float hitE     = aECLHit->getEnergyDep() / Unit::GeV;
      float aveT     = aECLHit->getTimeAve(); // ns :time from  IP  to PD
      if (aveT < - m_TimeRange || aveT > m_TimeRange) {continue;} //Choose - TimeRange ~ TimeTange
      int  TimeIndex = (int)((aveT + m_TimeRange) / 100); //Binning : -4000 = 1st bin ~  4000 80th bin.

      int iTCIdm = m_TCMap->getTCIdFromXtalId(hitCellId + 1) - 1;
      m_TCEnergy[iTCIdm][TimeIndex] += hitE;
      m_TCTiming[iTCIdm][TimeIndex] += hitE * aveT;
      if (beambkg > 0) {m_TCBkgContribution[iTCIdm][TimeIndex] += hitE  ;}
      else if (beambkg == 0) {m_TCSigContribution[iTCIdm][TimeIndex] += hitE;}

    }
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      for (int  iTime = 0; iTime < nBinTime; iTime++) {

        if (m_TCEnergy[iTCIdm][iTime] < 1e-9) {continue;}// 0.01MeV cut
        /* cppcheck-suppress variableScope */
        double maxbkgE = 0;
        /* cppcheck-suppress variableScope */
        int maxbkgtag = 0;
        m_TCTiming[iTCIdm][iTime] /= m_TCEnergy[iTCIdm][iTime];
        if (m_BeambkgTag == 1) {
          if (m_TCBkgContribution[iTCIdm][iTime] < m_TCSigContribution[iTCIdm][iTime]) {
            m_TCBeambkgTag[iTCIdm][iTime] = 0; //signal Tag : 0
          } else {
            for (int iXtalIdm = 0; iXtalIdm < 8736; iXtalIdm++) {
              int iTCId = m_TCMap->getTCIdFromXtalId(iXtalIdm + 1) - 1;
              if (iTCIdm != iTCId) {continue;}
              if (maxbkgE < E_cell[iXtalIdm][iTime]) {
                maxbkgE = E_cell[iXtalIdm][iTime];
                maxbkgtag =  beambkg_tag[iXtalIdm][iTime];
              }
            }
            m_TCBeambkgTag[iTCIdm][iTime] = maxbkgtag;
          }
        }
      }
    }
  }


  if (SourceOfTC == 2) { // read ECLSimHit
    std::vector< std::vector<float> > E_cell(8736, std::vector<float>(80, 0.0));
    std::vector< std::vector<float> > T_ave(8736, std::vector<float>(80, 0.0));
    std::vector< std::vector<float> > Tof_ave(8736, std::vector<float>(80, 0.0));
    std::vector< std::vector<float> > beambkg_tag(8736, std::vector<float>(80, 0.0));

    ECL::ECLGeometryPar* eclp = ECL::ECLGeometryPar::Instance();
    //=====================
    // Loop over all hits of steps
    //=====================
    StoreArray<ECLSimHit> eclArray("ECLSimHits");
    int nHits = eclArray.getEntries();
    //
    for (int iHits = 0; iHits < nHits; iHits++) {
      // Get a hit
      ECLSimHit* aECLSimHit = eclArray[iHits];

      int hitCellId  = aECLSimHit->getCellId() - 1;
      float hitE     = aECLSimHit->getEnergyDep() / Unit::GeV;
      float hitTOF      = aECLSimHit->getFlightTime() / Unit::ns;

      G4ThreeVector t = aECLSimHit->getPosIn(); // [cm], Hit position in Xtal (based on from IP)
      ROOT::Math::XYZVector HitInPos(t.x(), t.y(), t.z()); // = aECLSimHit->getPosIn(); // [cm], Hit position in Xtal (based on from IP)
      ROOT::Math::XYZVector PosCell  = eclp->GetCrystalPos(hitCellId);// [cm], Xtal position (based on from IP)
      ROOT::Math::XYZVector VecCell  = eclp->GetCrystalVec(hitCellId);
      float local_pos_r = 15.0 - (HitInPos - PosCell).Dot(VecCell);
      if (hitTOF < - m_TimeRange || hitTOF >  m_TimeRange) {continue;}
      int TimeIndex = (int)((hitTOF + m_TimeRange) / 100);
      E_cell[hitCellId][TimeIndex]  = E_cell[hitCellId][TimeIndex]  + hitE;
      T_ave[hitCellId][TimeIndex]   = T_ave[hitCellId][TimeIndex]   + hitE * local_pos_r;
      Tof_ave[hitCellId][TimeIndex] = Tof_ave[hitCellId][TimeIndex] + hitE * hitTOF;
    }
    //
    //
    //
    //===============
    // Xtal energy and timing (0-8us, 0.2us interval, 40 bins)
    //===============
    for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
      for (int  TimeIndex = 0; TimeIndex < nBinTime; TimeIndex++) {

        if (E_cell[iECLCell][TimeIndex] < 1e-9) {continue;} // 0.01MeV cut

        T_ave[iECLCell][TimeIndex]   = T_ave[iECLCell][TimeIndex] / E_cell[iECLCell][TimeIndex];
        T_ave[iECLCell][TimeIndex]   =
          6.05 +
          0.0749  * T_ave[iECLCell][TimeIndex] -
          0.00112 * T_ave[iECLCell][TimeIndex] * T_ave[iECLCell][TimeIndex];
        Tof_ave[iECLCell][TimeIndex] = Tof_ave[iECLCell][TimeIndex] / E_cell[iECLCell][TimeIndex];

      } // 40bins,  Time interval = 200ns
    }
    //
    //
    //
    for (int iXtalIdm = 0; iXtalIdm < 8736; iXtalIdm++) {
      int iTCIdm = m_TCMap->getTCIdFromXtalId(iXtalIdm + 1) - 1;
      for (int  iTime = 0; iTime < nBinTime; iTime++) {

        if (E_cell[iXtalIdm][iTime] < 1e-9) {continue;}  // 0.01MeV cut
        m_TCEnergy[iTCIdm][iTime] += E_cell[iXtalIdm][iTime];
        m_TCTiming[iTCIdm][iTime] += E_cell[iXtalIdm][iTime] * (T_ave[iXtalIdm][iTime]);
        if (beambkg_tag[iXtalIdm][iTime] > 0) {m_TCBkgContribution[iTCIdm][iTime] += E_cell[iXtalIdm][iTime];}
        if (beambkg_tag[iXtalIdm][iTime] == 0) {m_TCSigContribution[iTCIdm][iTime] += E_cell[iXtalIdm][iTime];}

      }
    }
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      for (int  iTime = 0; iTime < nBinTime; iTime++) {
        /* cppcheck-suppress variableScope */
        double maxbkgE = 0;
        /* cppcheck-suppress variableScope */
        int maxbkgtag = 0;

        if (m_TCEnergy[iTCIdm][iTime] < 1e-9) {continue;}  // 0.01MeV cut
        m_TCTiming[iTCIdm][iTime] /= m_TCEnergy[iTCIdm][iTime];
        if (m_BeambkgTag == 1) {
          if (m_TCBkgContribution[iTCIdm][iTime] < m_TCSigContribution[iTCIdm][iTime]) {
            m_TCBeambkgTag[iTCIdm][iTime] = 0; //signal Tag : 0
          } else {
            for (int iXtalIdm = 0; iXtalIdm < 8736; iXtalIdm++) {
              int iTCId = m_TCMap->getTCIdFromXtalId(iXtalIdm + 1) - 1;
              if (iTCIdm != iTCId) {continue;}
              if (maxbkgE < E_cell[iXtalIdm][iTime]) {
                maxbkgE = E_cell[iXtalIdm][iTime];
                maxbkgtag =  beambkg_tag[iXtalIdm][iTime];
              }
            }
            m_TCBeambkgTag[iTCIdm][iTime] = maxbkgtag;
          }
        }
      }

    }

  }
  //--------------------------------------------------------
  //
  //--------------------------------------------------------
  if (SourceOfTC == 3) {

    StoreArray<ECLHit> eclHitArray("ECLHits");

    int nHits_hit = eclHitArray.getEntries();
    // signal hit
    for (int iHits = 0; iHits < nHits_hit; iHits++) {
      // Get a hit
      ECLHit* aECLHit = eclHitArray[iHits];
      // Hit geom. info
      int hitCellId  = aECLHit->getCellId() - 1;
      float hitE     = aECLHit->getEnergyDep() / Unit::GeV;
      float aveT     = aECLHit->getTimeAve(); // ns :time from  IP  to PD
      // Choose - TimeRange ~ TimeTange
      if (aveT < - m_TimeRange || aveT > m_TimeRange) {continue;}
      // Binning : -4000 = 1st bin ~  4000 80th bin.
      int TimeIndex = (int)((aveT + m_TimeRange) / 100);

      int iTCIdm = m_TCMap->getTCIdFromXtalId(hitCellId + 1) - 1;
      m_TCEnergy[iTCIdm][TimeIndex] += hitE;
      m_TCTiming[iTCIdm][TimeIndex] += hitE * aveT;
    }
    // background hit
    StoreArray<TRGECLBGTCHit> m_trgeclBGTCHits("TRGECLBGTCHits_beamBG");
    for (const TRGECLBGTCHit& ttt : m_trgeclBGTCHits) {
      // TC timing
      double tcbg_t = ttt.getTimeAve();
      // Timing cut
      if (abs(tcbg_t) > m_TimeRange) continue;
      //Binning : -4000 = 1st bin ~  4000 80th bin.
      int TimeIndex = (int)((tcbg_t + m_TimeRange) / 100);
      // TC energy
      double tcbg_e = ttt.getEnergyDep();
      // TC ID index
      int iTCIdm = ttt.getTCId() - 1;
      // TC energy and timing
      double tc_e = m_TCEnergy[iTCIdm][TimeIndex];
      double tc_t = m_TCTiming[iTCIdm][TimeIndex];
      m_TCEnergy[iTCIdm][TimeIndex] += tcbg_e;
      m_TCTiming[iTCIdm][TimeIndex] += (tcbg_e * tcbg_t + tc_e * tc_t);
    }
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      for (int  iTime = 0; iTime < nBinTime; iTime++) {
        m_TCTiming[iTCIdm][iTime] /= m_TCEnergy[iTCIdm][iTime];
      }
    }
  }
  //--------------------------
  // TC energy and timing in t=0-1us as true values.
  //--------------------------
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for (int  iTime = 40; iTime < 50; iTime++) {
      m_TCEnergy_tot[iTCIdm] += m_TCEnergy[iTCIdm][iTime];
      m_TCTiming_tot[iTCIdm] += m_TCTiming[iTCIdm][iTime] * m_TCEnergy[iTCIdm][iTime];
    }
    m_TCTiming_tot[iTCIdm] /= m_TCEnergy_tot[iTCIdm];
  }

  return;
}
//
//
//
void
TrgEclDigitizer::digitization01(std::vector<std::vector<double>>& TCDigiE,
                                std::vector<std::vector<double>>& TCDigiT)
{
  TCDigiE.clear();
  TCDigiE.resize(576, vector<double>(64, 0));
  TCDigiT.clear();
  TCDigiT.resize(576, vector<double>(64, 0));

  //
  double cut_energy_tot = 0.03; // [GeV]
  int nbin_pedestal = 4; // = nbin_pedestal*fam_sampling_interval [ns] in total
  double fam_sampling_interval = 125; // [ns]
  int NSampling = 64; // # of sampling array


  std::vector< std::vector<float> > noise_pileup(576, std::vector<float>(64, 0.0));   // [GeV]
  std::vector< std::vector<float> > noise_parallel(576, std::vector<float>(64, 0.0));   // [GeV]
  std::vector< std::vector<float> > noise_serial(576, std::vector<float>(64, 0.0));   // [GeV]
  std::vector<float> X_pr(64, 0.0);
  std::vector<float> X_sr(64, 0.0);


  // (Make sampling time random between FAM sampling intervals)
  double random_sampling_correction = 0; // [ns]
  random_sampling_correction = gRandom->Rndm() * fam_sampling_interval;
  //==================
  // (01)Signal digitization
  //==================
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for (int iTimeBin = 0; iTimeBin < 80; iTimeBin++) {

      if (m_TCEnergy[iTCIdm][iTimeBin] < 0.0001) {continue;} // 0.1MeV cut on TC bin_energy
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        // inputTiming is in [us] <-- Be careful, here is NOT [ns]
        double inputTiming
          = (-m_TCTiming[iTCIdm][iTimeBin] - m_TimeRange + (-nbin_pedestal + iSampling) * fam_sampling_interval) * 0.001;
        inputTiming += random_sampling_correction * 0.001;
        if (inputTiming < 0 || inputTiming > 2.0) {continue;} // Shaping in t0 ~t0+ 2.0 us
        if (m_FADC == 1) {

          TCDigiE[iTCIdm][iSampling] += interFADC(inputTiming) * m_TCEnergy[iTCIdm][iTimeBin];
        } else {
          TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(0, inputTiming) * m_TCEnergy[iTCIdm][iTimeBin];
        }
      }

      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        TCDigiT[iTCIdm][iSampling] = (-nbin_pedestal + iSampling - m_TimeRange / fam_sampling_interval) * fam_sampling_interval;
        TCDigiT[iTCIdm][iSampling] += random_sampling_correction;
      }
    }
  }
  //
  //
  //
  if (0) {
    FILE* f_out_dat = fopen("ztsim.no_noise.dat", "w");
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      if (m_TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut
      fprintf(f_out_dat, "%5i %8.5f %8.5f %8.1f ",
              iTCIdm + 1, m_TCEnergy_tot[iTCIdm], m_TCEnergy[iTCIdm][0], TCDigiT[iTCIdm][0]);
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        fprintf(f_out_dat, "%7.4f ", TCDigiE[iTCIdm][iSampling]);
      }
      fprintf(f_out_dat, "\n");
    }
    fclose(f_out_dat);
  }
  //==================
  // (01)noise embedding
  //==================
  double  tmin_noise = -4; // original
  double tgen = 10.3;   // original
  int bkg_level = 1030;
  double ttt0 = 0; // [us]
  /* cppcheck-suppress variableScope */
  double ttt1 = 0; // [us]
  /* cppcheck-suppress variableScope */
  double ttt2 = 0; // [us]
  //
  double frac_pileup   = 0.035; // pileup noise fraction?
  double frac_parallel = 0.023; // parallel noise fraction?
  double frac_serial   = 0.055; // serial noise fraction?
  double times_pileup   =  1;   // noise scale based on Belle noise.
  double times_parallel =  3.15;   // noise scale
  double times_serial   =  3.15;   // noise scale

  [[maybe_unused]] double corr_pileup   = times_pileup   * frac_pileup   * sqrt(fam_sampling_interval * 0.001);
  double corr_parallel = times_parallel * frac_parallel * sqrt(fam_sampling_interval * 0.001);
  double corr_serial   = times_serial   * frac_serial   * sqrt(fam_sampling_interval * 0.001);


  if (0) {
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      if (m_TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut
      for (int jjj = 0; jjj < bkg_level; jjj++) {
        ttt0 = -(tmin_noise + tgen * gRandom->Rndm()); // [us]
        ttt1 = -(tmin_noise + tgen * gRandom->Rndm());  // [us]
        ttt2 = -(tmin_noise + tgen * gRandom->Rndm());  // [us]
        for (int iSampling = 0; iSampling < NSampling; iSampling++) {
          // (pile-up noise)
          if (ttt0 > 0) { TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(0, ttt0) * corr_pileup * 0.001; }
          // (parallel noise)
          if (ttt1 > 0) { TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(1, ttt1) * corr_parallel; }
          // (serial noise)
          if (ttt2 > 0) { TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(2, ttt2) * corr_serial; }
          ttt0 += fam_sampling_interval * 0.001;
          ttt1 += fam_sampling_interval * 0.001;
          ttt2 += fam_sampling_interval * 0.001;
        }
      }
    }
  }
  if (1) { //use L Matrix
    // A TC with no deposited energy in any time bin receives no signal
    // contribution at all, so noise alone cannot bring it over the fit
    // threshold. Note m_TCEnergy_tot covers only bins 40-49 and so must NOT
    // be used here: a real signal arriving just before t=0 falls in bin 39.
    bool hasEnergy[576];
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      hasEnergy[iTCIdm] = false;
      for (int iTimeBin = 0; iTimeBin < 80; iTimeBin++) {
        if (m_TCEnergy[iTCIdm][iTimeBin] >= 1e-9) { hasEnergy[iTCIdm] = true; break; }
      }
    }

    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {

      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        X_pr[iSampling] = gRandom ->Gaus(0, 1);
        X_sr[iSampling] = gRandom ->Gaus(0, 1);
      }

      // drawn for every TC above, so the random sequence is left untouched
      if (!hasEnergy[iTCIdm]) { continue; }

      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        for (int jSampling = 0; jSampling < NSampling; jSampling++) {
          noise_parallel[iTCIdm][iSampling] += 10 * corr_parallel * m_MatrixParallel[iSampling][jSampling] * X_pr[jSampling];
          noise_serial[iTCIdm][iSampling] += 10 * corr_serial * m_MatrixSerial[iSampling][jSampling] * X_sr[jSampling];
        }
      }

      for (int iSampling = 0; iSampling < NSampling; iSampling++) {

        TCDigiE[iTCIdm][iSampling] += noise_pileup[iTCIdm][iSampling] + noise_parallel[iTCIdm][iSampling] +
                                      noise_serial[iTCIdm][iSampling];
      }
    }
    if (0) {
      for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) { //Only Pile-up noise use old method.
        if (m_TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut
        for (int jjj = 0; jjj < bkg_level; jjj++) {
          ttt0 = -(tmin_noise + tgen * gRandom->Rndm()); // [us]
          for (int iSampling = 0; iSampling < NSampling; iSampling++) {
            // (pile-up noise)
            if (ttt0 > 0) { TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(0, ttt0) * corr_pileup * 0.001; }
            ttt0 += fam_sampling_interval * 0.001;
          }
        }
      }
    }
  }

  if (m_SaveTCWaveForm == 1) {

    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        m_WaveForm[iTCIdm][iSampling] = TCDigiE[iTCIdm][iSampling];
      }

    }
  }

}
void
TrgEclDigitizer::digitization02(std::vector<std::vector<double>>& TCDigiE, std::vector<std::vector<double>>& TCDigiT)
{
  //===============
  // (03)Signal digitization (w/ 12ns interval for method-0)
  //===============
  TCDigiE.clear();
  TCDigiE.resize(576, vector<double>(666, 0));
  TCDigiT.clear();
  TCDigiT.resize(576, vector<double>(666, 0));
  double cut_energy_tot = 0.03; // [GeV]
  int nbin_pedestal = 100;
  float fam_sampling_interval = 12; // [ns]

  std::vector< std::vector<float> > TCDigiEnergy(576, std::vector<float>(666, 0.0));    // [GeV]
  std::vector< std::vector<float> > TCDigiTiming(576, std::vector<float>(666, 0.0));    // [ns]
  int NSampling = 666;

  // Make sampling time random between FAM sampling intervals
  float random_sampling_correction = 0; // [ns]
  random_sampling_correction = gRandom->Rndm() * fam_sampling_interval;
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    if (m_TCEnergy_tot[iTCIdm] < cut_energy_tot) {continue;} // TC energy_tot cut
    for (int iTimeBin = 0; iTimeBin < 80; iTimeBin++) {
      if (m_TCEnergy[iTCIdm][iTimeBin] < 0.0001) { continue; }   // 0.1 MeV cut on TC bin_energy
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        // inputTiming is in [us] <-- Be careful, here is NOT [ns]
        float inputTiming
          = (-m_TCTiming[iTCIdm][iTimeBin] - m_TimeRange + (-nbin_pedestal + iSampling) * fam_sampling_interval) * 0.001;

        inputTiming += random_sampling_correction * 0.001;
        if (inputTiming < 0 || inputTiming > 2.0) {continue;} // Shaping in t0 ~t0+ 2.0 us

        TCDigiEnergy[iTCIdm][iSampling] += FADC(0, inputTiming) * m_TCEnergy[iTCIdm][iTimeBin];
      }
    }
    for (int iSampling = 0; iSampling < NSampling; iSampling++) {
      TCDigiTiming[iTCIdm][iSampling] = (-nbin_pedestal + iSampling - m_TimeRange / fam_sampling_interval) * fam_sampling_interval;
      TCDigiTiming[iTCIdm][iSampling] += random_sampling_correction;
    }
  }
  //==================
  // (03)noise embedding
  //==================

  double tmin_noise = -4; // original
  double   tgen = 10.3;   //
  int bkg_level = 1030;
  double ttt0 = 0; // [us]
  double ttt1 = 0; // [us]
  double ttt2 = 0; // [us]
  //double frac_pileup   = 0.035; // pileup noise fraction?
  //double frac_parallel = 0.023; // parallel noise fraction?
  //double frac_serial   = 0.055; // serial noise fraction?
  //double times_pileup   =  1;   // noise scale based on Belle noise.
  //double times_parallel =  1;   // noise scale
  //double times_serial   =  1;   // noise scale
  //double corr_pileup   = times_pileup   * frac_pileup   * sqrt(fam_sampling_interval * 0.001);
  //double corr_parallel = times_parallel * frac_parallel * sqrt(fam_sampling_interval * 0.001);
  //double corr_serial   = times_serial   * frac_serial   * sqrt(fam_sampling_interval * 0.001);
  double corr_pileup   = 0.011068;
  double corr_parallel = 0.00727324;
  double corr_serial   = 0.0173925;


  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    if (m_TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // 1 MeV TC energy cut
    for (int jjj = 0; jjj < bkg_level; jjj++) {
      ttt0 = -(tmin_noise + tgen * gRandom->Rndm());  // [us]
      ttt1 = -(tmin_noise + tgen * gRandom->Rndm());  // [us]
      ttt2 = -(tmin_noise + tgen * gRandom->Rndm());  // [us]
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        // (pile-up noise)
        if (ttt0 > 0) { TCDigiEnergy[iTCIdm][iSampling] += FADC(0, ttt0) * corr_pileup * 0.001; }
        // (parallel noise)
        if (ttt1 > 0) { TCDigiEnergy[iTCIdm][iSampling] += FADC(1, ttt1) * corr_parallel; }
        // (serial noise)
        if (ttt2 > 0) { TCDigiEnergy[iTCIdm][iSampling] += FADC(2, ttt2) * corr_serial; }
        ttt0 += fam_sampling_interval * 0.001;
        ttt1 += fam_sampling_interval * 0.001;
        ttt2 += fam_sampling_interval * 0.001;
      }
    }
  }

}
//
//
//
void
TrgEclDigitizer::save(int m_nEvent)
{
  //---------------
  // Root Output
  //---------------
  int m_hitNum = 0;
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for (int iBinTime = 0; iBinTime < 80; iBinTime++) {
      if (m_TCEnergy[iTCIdm][iBinTime] < 0.001) {continue;}
      StoreArray<TRGECLDigi0> TCDigiArray;
      TCDigiArray.appendNew();
      m_hitNum = TCDigiArray.getEntries() - 1;

      TCDigiArray[m_hitNum]->setEventId(m_nEvent);
      TCDigiArray[m_hitNum]->setTCId(iTCIdm + 1);
      TCDigiArray[m_hitNum]->setiBinTime(iBinTime);
      TCDigiArray[m_hitNum]->setRawEnergy(m_TCEnergy[iTCIdm][iBinTime]);
      TCDigiArray[m_hitNum]->setRawTiming(m_TCTiming[iTCIdm][iBinTime]);
      TCDigiArray[m_hitNum]->setBeamBkgTag(m_TCBeambkgTag[iTCIdm][iBinTime]);
    }
  }

  if (m_SaveTCWaveForm == 1) {
    StoreArray<TRGECLWaveform> TCWaveformArray;
    for (int iTCIdm = 0; iTCIdm < 576;  iTCIdm++) {
      if (iTCIdm == 80) iTCIdm =  512; // skip barrel
      int tc_phi_id = m_TCMap->getTCPhiIdFromTCId(iTCIdm + 1);
      int tc_theta_id   = m_TCMap->getTCThetaIdFromTCId(iTCIdm + 1);
      TRGECLWaveform* newWf =
        TCWaveformArray.appendNew(iTCIdm + 1, m_WaveForm[iTCIdm]);
      newWf->setThetaPhiIDs(tc_theta_id, tc_phi_id);
    }

  }


  return;
}
//
//
//
double
TrgEclDigitizer::interFADC(double timing)
{
  timing = timing * 1000;

  double E_out = 0;

  // m_SignalAmp is sampled every 12 ns up to 2928 ns; nothing to interpolate
  // outside that range. Excluding the upper edge also keeps endbin in range.
  if (timing < 0 || timing >= 12.0 * (m_SignalAmp.size() - 1)) {return E_out;}

  const int startbin = (int)(timing) / 12;
  const int endbin = startbin + 1;
  // the sampling times are exactly 12 ns * index, so no lookup table is needed
  const double starttime = 12.0 * startbin;
  const double endtime = 12.0 * endbin;

  if (timing > starttime) {
    E_out = ((m_SignalAmp[endbin] - m_SignalAmp[startbin]) / (endtime - starttime)) *
            (timing - starttime) + m_SignalAmp[startbin];
  }

  return E_out;

}
//
//
//
double
TrgEclDigitizer::FADC(int flag_gen,
                      double timing)
{

  //--------------------------------------
  //
  // o "timing" unit is [us]
  // o flag_gen = 0(=signal), 1(=parallel), 2(=serial)
  // o return value(PDF) is [GeV]
  //
  //--------------------------------------
  double tsh, dd;
  static double tc, tc2, tsc, tris, b1, b2;
  static double amp, dft, as;
  static double td, t1, t2;

  static int ifir = 0;

  if (ifir == 0) {

    td =  0.10;  // diff time    (  0.10)
    t1 =  0.10;  // integ1 real  (  0.10)
    b1 = 30.90;  // integ1 imag  ( 30.90)
    t2 =  0.01;  // integ2 real  (  0.01)
    b2 = 30.01;  // integ2 imag  ( 30.01)
    double    ts =  1.00;  // scint decay  (  1.00)
    dft = 0.600; // diff delay   ( 0.600)
    as = 0.548;  // diff frac    ( 0.548)
    //
    amp = 1.0;
    tris = 0.01;
    tsc = ts;
    //
    int im = 0;
    int ij = 0;
    int fm = 0;
    tc = 0;
    double tt = u_max(u_max(td, t1), u_max(t2, ts)) * 2;
    int flag_once = 0;
    while (flag_once == 0) {
      double   dt = tt / 1000;
      double tm = 0;
      for (int j = 1; j <= 1000; j++) {
        tc2 = tc - dft;
        double  fff =
          (ShapeF(tc, t1, b1, t2, b2, td, tsc) -
           ShapeF(tc, t1, b1, t2, b2, td, tris) * 0.01) -
          (ShapeF(tc2, t1, b1, t2, b2, td, tsc) -
           ShapeF(tc2, t1, b1, t2, b2, td, tris) * 0.01) * as;
        if (fff > fm) {
          fm = fff;
          tm = tc;
          im = j;
        }
        tc = tc + dt;
      }
      if (im >= 1000) {
        tt = 2 * tt;
        flag_once = 0;
        continue;
      }
      if (ij == 0) {
        ij = 1;
        tc = 0.99 * tm;
        dt = tm * 0.02 / 1000;
        flag_once = 0;
        continue;
      }
      flag_once = 1;
    }
    amp = 1.0 / fm;
    ifir = 1;
  }
  //
  //
  double pdf = 0;
  if (flag_gen == 0) {
    //-----<signal>
    tc2 = timing - dft;
    pdf = amp * (
            (ShapeF(timing,  t1, b1, t2, b2, td, tsc) -
             ShapeF(timing,  t1, b1, t2, b2, td, tris) * 0.01) -
            (ShapeF(tc2,       t1, b1, t2, b2, td, tsc) -
             ShapeF(tc2,       t1, b1, t2, b2, td, tris) * 0.01) * as);
  } else if (flag_gen == 1) {
    //-----<parallel>
    tc2 = timing - dft;
    tsh = 0.001;
    pdf = amp * (
            ShapeF(timing,  t1, b1, t2, b2, td, tsh) -
            ShapeF(tc2,       t1, b1, t2, b2, td, tsh) * as);
    pdf = pdf * 0.001; // GeV
  } else {
    //-----<serial>
    tc2 = timing - dft;
    tsh = 0.001;
    pdf = amp * (
            ShapeF(timing,  t1, b1, t2, b2, td, tsh) -
            ShapeF(tc2,       t1, b1, t2, b2, td, tsh) * as);
    //
    tc = timing - 0.01;
    if (tc < 0) { tc = 0; }
    dd = timing - tc;
    tc2 = tc - dft;
    pdf = (amp * (
             ShapeF(tc, t1, b1, t2, b2, td, tsh) -
             ShapeF(tc2, t1, b1, t2, b2, td, tsh) * as) - pdf) / dd;
    pdf = pdf * 0.001; // GeV
  }

  return pdf;
}
//
//
//
double
TrgEclDigitizer::ShapeF(double t00,
                        double t01,
                        double tb1,
                        double t02,
                        double tb2,
                        double td1,
                        double ts1)
{
  double dr;
  double dzna;
  double das1, das0, dac0;
  double dcs0s, dsn0s, dcs0d, dsn0d;
  double dcs1s, dsn1s, dcs1d, dsn1d;

  double sv123 = 0.0;
  if (t00 < 0) return 0;

  dr = (ts1 - td1) / td1;
  if (fabs(dr) <= 1.0e-5) {
    if (ts1 > td1) { ts1 = td1 * 1.00001; }
    else         { ts1 = td1 * 0.99999; }
  }
  //
  dr = (pow((t01 - t02), 2) + pow((tb1 - tb2), 2)) / (pow((t01), 2) + pow((tb1), 2));
  if (dr <= 1.e-10) {
    if (t01 < t02) { t01 = t02 / 1.00001; }
    else         { t01 = t02 / 0.99999; }
  }
  if (t00 <= 0.0) return 0;
  //
  //
  //
  double a1 = 1 / t01;
  double a2 = 1 / tb1;
  double b1 = 1 / t02;
  double b2 = 1 / tb2;
  double c1 = 1 / td1;
  double c2 = 1 / ts1;

  das0 = b2 * (pow((b1 - a1), 2) + (b2 + a2) * (b2 - a2));
  dac0 = -2 * (b1 - a1) * a2 * b2;
  das1 = a2 * (pow((b1 - a1), 2) - (b2 + a2) * (b2 - a2));

  dsn0s = ((c2 - a1) * das0  - (-a2) * dac0)   / (pow(a2, 2) + pow((c2 - a1), 2));
  dcs0s = ((-a2) * das0 + (c2 - a1) * dac0)   / (pow(a2, 2) + pow((c2 - a1), 2));

  dsn1s = ((c2 - b1) * das1  - (-b2) * (-dac0)) / (pow(b2, 2) + pow((c2 - b1), 2));
  dcs1s = ((-b2) * das1 + (c2 - b1) * (-dac0)) / (pow(b2, 2) + pow((c2 - b1), 2));

  dsn0d = ((c1 - a1) * das0  - (-a2) * dac0)   / (pow(a2, 2) + pow((c1 - a1), 2));
  dcs0d = ((-a2) * das0 + (c1 - a1) * dac0)   / (pow(a2, 2) + pow((c1 - a1), 2));

  dsn1d = ((c1 - b1) * das1  - (-b2) * (-dac0)) / (pow(b2, 2) + pow((c1 - b1), 2));
  dcs1d = ((-b2) * das1 + (c1 - b1) * (-dac0)) / (pow(b2, 2) + pow((c1 - b1), 2));
  //
  //
  //
  dzna = (pow((b1 - a1), 2) + pow((b2 - a2), 2)) * (pow((b1 - a1), 2) + pow((a2 + b2), 2));

  sv123 = (
            (dcs0s + dcs1s) * exp(-c2 * t00) * (-1) +
            (dcs0d + dcs1d) * exp(-c1 * t00) +
            ((dsn0s - dsn0d) * sin(a2 * t00) + (dcs0s - dcs0d) * cos(a2 * t00)) * exp(-a1 * t00) +
            ((dsn1s - dsn1d) * sin(b2 * t00) + (dcs1s - dcs1d) * cos(b2 * t00)) * exp(-b1 * t00)
          )
          / dzna / (1 / c2 - 1 / c1);

  return sv123;
}

//
//
//


double
TrgEclDigitizer::SimplifiedFADC(int flag_gen,
                                double timing)
{

  //--------------------------------------
  //
  // o "timing" unit is [us]
  // o flag_gen = 0(=signal), 1(=parallel), 2(=serial)
  // o return value(PDF) is [GeV]
  // o Generate signal shape using a simplified function.
  //
  //
  //--------------------------------------
  double tsh, dd;
  static double tc, tc2, tsc, tris;
  static double amp, dft, as;


  //  int im, ij;

  static int ifir = 0;

  if (ifir == 0) {

    const double td =  0.10;  // diff time    (  0.10)
    const double t1 =  0.10;  // integ1 real  (  0.10)
    // b1 = 30.90;  // integ1 imag  ( 30.90)
    const double t2 =  0.01;  // integ2 real  (  0.01)
    // b2 = 30.01;  // integ2 imag  ( 30.01)
    double ts =  1.00;  // scint decay  (  1.00)
    dft = 0.600; // diff delay   ( 0.600)
    as = 0.548;  // diff frac    ( 0.548)
    //
    amp = 1.0;
    tris = 0.01;
    // ts0 = 0.0;
    tsc = ts;
    double    fm = 0;
    //
    int  im = 0;
    int  ij = 0;
    tc = 0;
    double tt = u_max(u_max(td, t1), u_max(t2, ts)) * 2;
    int flag_once = 0;
    while (flag_once == 0) {
      double dt = tt / 1000;

      double tm = 0;
      for (int j = 1; j <= 1000; j++) {
        tc2 = tc - dft;
        double fff =
          (ShapeF(tc, tsc) -
           ShapeF(tc, tris) * 0.01) -
          (ShapeF(tc2, tsc) -
           ShapeF(tc2, tris) * 0.01) * as;
        if (fff > fm) {
          fm = fff;
          tm = tc;
          im = j;
        }
        tc = tc + dt;
      }
      if (im >= 1000) {
        tt = 2 * tt;
        flag_once = 0;
        continue;
      }
      if (ij == 0) {
        ij = 1;
        tc = 0.99 * tm;
        dt = tm * 0.02 / 1000;
        flag_once = 0;
        continue;
      }
      flag_once = 1;
    }
    amp = 1.0 / fm;
    ifir = 1;
  }
  //
  //
  double pdf = 0;
  if (flag_gen == 0) {
    //-----<signal>
    tc2 = timing - dft;
    pdf = amp * (
            (ShapeF(timing, tsc) -
             ShapeF(timing, tris) * 0.01) -
            (ShapeF(tc2,    tsc) -
             ShapeF(tc2,      tris) * 0.01) * as);
  } else if (flag_gen == 1) {
    //-----<parallel>
    tc2 = timing - dft;
    tsh = 0.001;
    pdf = amp * (
            ShapeF(timing, tsh) -
            ShapeF(tc2,     tsh) * as);
    pdf = pdf * 0.001; // GeV
  } else {
    //-----<serial>
    tc2 = timing - dft;
    tsh = 0.001;
    pdf = amp * (
            ShapeF(timing, tsh) -
            ShapeF(tc2,    tsh) * as);
    //
    tc = timing - 0.01;
    if (tc < 0) { tc = 0; }
    dd = timing - tc;
    tc2 = tc - dft;
    pdf = (amp * (
             ShapeF(tc, tsh) -
             ShapeF(tc2, tsh) * as) - pdf) / dd;
    pdf = pdf * 0.001; // GeV
  }

  return pdf;
}


double TrgEclDigitizer::ShapeF(double t00, double ts1)
{
  static const double realNaN = std::numeric_limits<double>::quiet_NaN();

  double dzna;
  // double das1,das0,dac0;
  double dcs0s = realNaN, dsn0s = realNaN, dcs0d, dsn0d;
  double dcs1s = realNaN, dsn1s = realNaN, dcs1d, dsn1d;
  double a1, a2, b1, b2, c1, c2 = realNaN;
  double sv123 = 0.0;

  if (t00 <= 0.0) return 0;



  a1 = 10 ;
  a2 =  0.0323625 ;
  b1 = 100;
  b2 = 0.0333222;
  c1 = 10;

  if (ts1 == 1) {
    c2 = 1;
    dsn0s =  -29.9897;
    dcs0s =   -0.08627;

    dsn1s =  -2.64784;
    dcs1s =  -0.00285194;
  }
  if (ts1 == 0.01) {
    c2 = 100;
    dsn0s =  2.999 ;
    dcs0s =   -0.00323517;

    dsn1s =  5.82524;
    dcs1s = -7866.7;
  }
  if (ts1 == 0.001) {
    c2 = 1000;
    dsn0s = 0.272636;
    dcs0s = -0.000204983;

    dsn1s = 0.291262;
    dcs1s = 0.000204894;
  }

  dsn0d =  -5.998;
  dcs0d =   -8340.22;

  dsn1d =  -2.91262;
  dcs1d = -0.00323517;
  //
  //
  //
  dzna = 6.561e+07;

  sv123 = (
            (dcs0s + dcs1s) * exp(-c2 * t00) * (-1) +
            (dcs0d + dcs1d) * exp(-c1 * t00) +
            ((dsn0s - dsn0d) * sin(a2 * t00) + (dcs0s - dcs0d) * cos(a2 * t00)) * exp(-a1 * t00) +
            ((dsn1s - dsn1d) * sin(b2 * t00) + (dcs1s - dcs1d) * cos(b2 * t00)) * exp(-b1 * t00)
          )
          / dzna / (1 / c2 - 1 / c1);

  return sv123;
}
//
//
//
double
TrgEclDigitizer::u_max(double aaa, double bbb)
{
  if (aaa > bbb) { return aaa; }
  else        { return bbb; }
}
//
//
//
