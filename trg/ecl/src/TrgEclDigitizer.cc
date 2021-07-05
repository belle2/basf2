/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TrgEclDigitizer.cc
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to represent TRG ECL
//---------------------------------------------------------------
// $Log$
// 2017-02-16 : v01
//---------------------------------------------------------------
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
  TimeRange(0), _waveform(0), _FADC(1), _BeambkgTag(0)
{
  MatrixParallel.clear();
  MatrixSerial.clear();

  _TCMap = new TrgEclMapping();
  _DataBase = new TrgEclDataBase();

  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    TCEnergy_tot[iTCIdm] = 0;
    TCTiming_tot[iTCIdm] = 0;
    for (int  iTime = 0; iTime < 80; iTime++) {
      TCEnergy[iTCIdm][iTime] = 0;
      TCTiming[iTCIdm][iTime] = 0;
      TCBkgContribution[iTCIdm][iTime] = 0;
      TCSigContribution[iTCIdm][iTime] = 0;
      TCBeambkgTag[iTCIdm][iTime] = 0;
    }

    for (int iii = 0; iii < 60 ; iii++) {
      TCRawEnergy[iTCIdm][iii] = 0;
      TCRawTiming[iTCIdm][iii] = 0;
      TCRawBkgTag[iTCIdm][iii] = 0;
    }
    for (int iii = 0; iii < 64 ; iii++) {
      WaveForm[iTCIdm][iii] = 0;
    }
  }
}
//
//
//
TrgEclDigitizer::~TrgEclDigitizer()
{
  delete _TCMap;
  delete _DataBase;
}
//
//
//

void
TrgEclDigitizer::setup()
{
  // prepare Matrix for Noise generation
  //
  _DataBase->  readNoiseLMatrix(MatrixParallel, MatrixSerial);
  //
  // 1=ECLHit, 2=ECLSimHit, 3=ECLHit+TRGECLBGTCHit
  int TableFlag = 3;
  // initialize parameters
  getTCHit(TableFlag);
  //
  //
  //
  return;
}
//
//
//
void
TrgEclDigitizer::getTCHit(int TableFlag)
{

  std::vector< std::vector<float> > E_cell(8736, std::vector<float>(80, 0.0));
  std::vector< std::vector<float> > T_ave(8736, std::vector<float>(80, 0.0));
  std::vector< std::vector<float> > Tof_ave(8736, std::vector<float>(80, 0.0));
  std::vector< std::vector<float> > beambkg_tag(8736, std::vector<float>(80, 0.0));

  int nBinTime = 80;
  TimeRange = 4000; // -4us ~ 4us
  //-------------------------------------------------------------------
  //                          read Xtal data
  //---------------------------------------------------------------------
  if (TableFlag == 1) { // read  ECLHit table
    StoreArray<ECLHit> eclHitArray("ECLHits");
    int nHits_hit = eclHitArray.getEntries() - 1;
    //
    for (int iHits = 0; iHits < nHits_hit; iHits++) {
      // Get a hit
      ECLHit* aECLHit = eclHitArray[iHits];
      int beambkg = aECLHit->getBackgroundTag();

      // Hit geom. info
      int hitCellId  = aECLHit->getCellId() - 1;
      float hitE     = aECLHit->getEnergyDep() / Unit::GeV;
      float aveT     = aECLHit->getTimeAve(); // ns :time from  IP  to PD
      if (aveT < - TimeRange || aveT > TimeRange) {continue;} //Choose - TimeRange ~ TimeTange
      int  TimeIndex = (int)((aveT + TimeRange) / 100); //Binning : -4000 = 1st bin ~  4000 80th bin.

      int iTCIdm = _TCMap->getTCIdFromXtalId(hitCellId + 1) - 1;
      TCEnergy[iTCIdm][TimeIndex] += hitE;
      TCTiming[iTCIdm][TimeIndex] += hitE * aveT;
      if (beambkg > 0) {TCBkgContribution[iTCIdm][TimeIndex] += hitE  ;}
      else if (beambkg == 0) {TCSigContribution[iTCIdm][TimeIndex] += hitE;}

    }
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      for (int  iTime = 0; iTime < nBinTime; iTime++) {
        if (TCEnergy[iTCIdm][iTime] < 1e-9) {continue;}// 0.01MeV cut
        /* cppcheck-suppress variableScope */
        double maxbkgE = 0;
        /* cppcheck-suppress variableScope */
        int maxbkgtag = 0;
        TCTiming[iTCIdm][iTime] /= TCEnergy[iTCIdm][iTime];
        if (_BeambkgTag == 1) {
          if (TCBkgContribution[iTCIdm][iTime] < TCSigContribution[iTCIdm][iTime]) {
            TCBeambkgTag[iTCIdm][iTime] = 0; //signal Tag : 0
          } else {
            for (int iXtalIdm = 0; iXtalIdm < 8736; iXtalIdm++) {
              int iTCId = _TCMap->getTCIdFromXtalId(iXtalIdm + 1) - 1;
              if (iTCIdm != iTCId) {continue;}
              if (maxbkgE < E_cell[iXtalIdm][iTime]) {
                maxbkgE = E_cell[iXtalIdm][iTime];
                maxbkgtag =  beambkg_tag[iXtalIdm][iTime];
              }
            }
            TCBeambkgTag[iTCIdm][iTime] = maxbkgtag;
          }
        }
      }
    }
  }


  if (TableFlag == 2) { // read ECLSimHit
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
      TVector3 HitInPos(t.x(), t.y(), t.z()); // = aECLSimHit->getPosIn(); // [cm], Hit position in Xtal (based on from IP)
      TVector3 PosCell  = eclp->GetCrystalPos(hitCellId);// [cm], Xtal position (based on from IP)
      TVector3 VecCell  = eclp->GetCrystalVec(hitCellId);
      float local_pos_r = (15.0 - (HitInPos - PosCell) * VecCell);
      if (hitTOF < - TimeRange || hitTOF >  TimeRange) {continue;}
      int TimeIndex = (int)((hitTOF + TimeRange) / 100);
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
      int iTCIdm = _TCMap->getTCIdFromXtalId(iXtalIdm + 1) - 1;
      for (int  iTime = 0; iTime < nBinTime; iTime++) {
        if (E_cell[iXtalIdm][iTime] < 1e-9) {continue;}  // 0.01MeV cut
        TCEnergy[iTCIdm][iTime] += E_cell[iXtalIdm][iTime];
        TCTiming[iTCIdm][iTime] += E_cell[iXtalIdm][iTime] * (T_ave[iXtalIdm][iTime]);
        if (beambkg_tag[iXtalIdm][iTime] > 0) {TCBkgContribution[iTCIdm][iTime] += E_cell[iXtalIdm][iTime];}
        if (beambkg_tag[iXtalIdm][iTime] == 0) {TCSigContribution[iTCIdm][iTime] += E_cell[iXtalIdm][iTime];}

      }
    }
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      for (int  iTime = 0; iTime < nBinTime; iTime++) {
        /* cppcheck-suppress variableScope */
        double maxbkgE = 0;
        /* cppcheck-suppress variableScope */
        int maxbkgtag = 0;
        if (TCEnergy[iTCIdm][iTime] < 1e-9) {continue;}  // 0.01MeV cut
        TCTiming[iTCIdm][iTime] /= TCEnergy[iTCIdm][iTime];
        if (_BeambkgTag == 1) {
          if (TCBkgContribution[iTCIdm][iTime] < TCSigContribution[iTCIdm][iTime]) {
            TCBeambkgTag[iTCIdm][iTime] = 0; //signal Tag : 0
          } else {
            for (int iXtalIdm = 0; iXtalIdm < 8736; iXtalIdm++) {
              int iTCId = _TCMap->getTCIdFromXtalId(iXtalIdm + 1) - 1;
              if (iTCIdm != iTCId) {continue;}
              if (maxbkgE < E_cell[iXtalIdm][iTime]) {
                maxbkgE = E_cell[iXtalIdm][iTime];
                maxbkgtag =  beambkg_tag[iXtalIdm][iTime];
              }
            }
            TCBeambkgTag[iTCIdm][iTime] = maxbkgtag;
          }
        }
      }

    }

  }
  //--------------------------------------------------------
  //
  //--------------------------------------------------------
  if (TableFlag == 3) {
    StoreArray<ECLHit> eclHitArray("ECLHits");
    int nHits_hit = eclHitArray.getEntries() - 1;
    // signal hit
    for (int iHits = 0; iHits < nHits_hit; iHits++) {
      // Get a hit
      ECLHit* aECLHit = eclHitArray[iHits];
      // Hit geom. info
      int hitCellId  = aECLHit->getCellId() - 1;
      float hitE     = aECLHit->getEnergyDep() / Unit::GeV;
      float aveT     = aECLHit->getTimeAve(); // ns :time from  IP  to PD
      // Choose - TimeRange ~ TimeTange
      if (aveT < - TimeRange || aveT > TimeRange) {continue;}
      // Binning : -4000 = 1st bin ~  4000 80th bin.
      int TimeIndex = (int)((aveT + TimeRange) / 100);

      int iTCIdm = _TCMap->getTCIdFromXtalId(hitCellId + 1) - 1;
      TCEnergy[iTCIdm][TimeIndex] += hitE;
      TCTiming[iTCIdm][TimeIndex] += hitE * aveT;
    }
    // background hit
    StoreArray<TRGECLBGTCHit> m_trgeclBGTCHits("TRGECLBGTCHits_beamBG");
    for (const TRGECLBGTCHit& ttt : m_trgeclBGTCHits) {
      // TC timing
      double tcbg_t = ttt.getTimeAve();
      // Timing cut
      if (abs(tcbg_t) > TimeRange) continue;
      //Binning : -4000 = 1st bin ~  4000 80th bin.
      int TimeIndex = (int)((tcbg_t + TimeRange) / 100);
      // TC energy
      double tcbg_e = ttt.getEnergyDep();
      // TC ID index
      int iTCIdm = ttt.getTCId() - 1;
      // TC energy and timing
      double tc_e = TCEnergy[iTCIdm][TimeIndex];
      double tc_t = TCTiming[iTCIdm][TimeIndex];
      TCEnergy[iTCIdm][TimeIndex] += tcbg_e;
      TCTiming[iTCIdm][TimeIndex] += (tcbg_e * tcbg_t + tc_e * tc_t) / TCEnergy[iTCIdm][TimeIndex];
    }
  }
  //--------------------------
  // TC energy and timing in t=0-1us as true values.
  //--------------------------
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    for (int  iTime = 40; iTime < 50; iTime++) {
      TCEnergy_tot[iTCIdm] += TCEnergy[iTCIdm][iTime];
      TCTiming_tot[iTCIdm] += TCTiming[iTCIdm][iTime] * TCEnergy[iTCIdm][iTime];
    }
    TCTiming_tot[iTCIdm] /= TCEnergy_tot[iTCIdm];
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
  TCDigiE.resize(576, vector<double>(64 , 0));
  TCDigiT.clear();
  TCDigiT.resize(576, vector<double>(64 , 0));

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
    if (TCEnergy_tot[iTCIdm] < cut_energy_tot) {continue;} // TC energy_tot cut
    for (int iTimeBin = 0; iTimeBin < 80; iTimeBin++) {
      if (TCEnergy[iTCIdm][iTimeBin] < 0.0001) {continue;} // 0.1MeV cut on TC bin_energy
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        // inputTiming is in [us] <-- Be careful, here is NOT [ns]
        double inputTiming
          = (-TCTiming[iTCIdm][iTimeBin] - TimeRange + (-nbin_pedestal + iSampling) * fam_sampling_interval) * 0.001;
        inputTiming += random_sampling_correction * 0.001;
        if (inputTiming < 0 || inputTiming > 2.0) {continue;} // Shaping in t0 ~t0+ 2.0 us
        if (_FADC == 1) {

          TCDigiE[iTCIdm][iSampling] += interFADC(inputTiming) * TCEnergy[iTCIdm][iTimeBin];
        } else {
          TCDigiE[iTCIdm][iSampling] += SimplifiedFADC(0, inputTiming) * TCEnergy[iTCIdm][iTimeBin];
        }
      }

      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        TCDigiT[iTCIdm][iSampling] = (-nbin_pedestal + iSampling - TimeRange / fam_sampling_interval) * fam_sampling_interval;
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
      if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut
      fprintf(f_out_dat, "%5i %8.5f %8.5f %8.1f ",
              iTCIdm + 1, TCEnergy_tot[iTCIdm], TCEnergy[iTCIdm][0], TCDigiT[iTCIdm][0]);
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
  double  tmin_noise = -4; // orignal
  double tgen = 10.3;   // orignal
  int bkg_level = 1030;
  double ttt0 = 0; // [us]
  /* cppcheck-suppress variableScope */
  double ttt1 = 0; // [us]
  /* cppcheck-suppress variableScope */
  double ttt2 = 0; // [us]
  //
  double frac_pileup   = 0.035; // pileup noise fraction?
  double frac_parallel = 0.023; // parralel noise fraction?
  double frac_serial   = 0.055; // serial noise fraction?
  double times_pileup   =  1;   // noise scale based on Belle noise.
  double times_parallel =  3.15;   // noise scale
  double times_serial   =  3.15;   // noise scale

  double corr_pileup   = times_pileup   * frac_pileup   * sqrt(fam_sampling_interval * 0.001);
  double corr_parallel = times_parallel * frac_parallel * sqrt(fam_sampling_interval * 0.001);
  double corr_serial   = times_serial   * frac_serial   * sqrt(fam_sampling_interval * 0.001);


  if (0) {
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut
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
    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {

      if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut

      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        X_pr[iSampling] = gRandom ->Gaus(0, 1);
        X_sr[iSampling] = gRandom ->Gaus(0, 1);
      }

      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        for (int jSampling = 0; jSampling < NSampling; jSampling++) {
          noise_parallel[iTCIdm][iSampling] += 10 * corr_parallel * MatrixParallel[iSampling][jSampling] * X_pr[jSampling];
          noise_serial[iTCIdm][iSampling] += 10 * corr_serial * MatrixSerial[iSampling][jSampling] * X_sr[jSampling];
        }
      }

      for (int iSampling = 0; iSampling < NSampling; iSampling++) {

        TCDigiE[iTCIdm][iSampling] += noise_pileup[iTCIdm][iSampling] + noise_parallel[iTCIdm][iSampling] +
                                      noise_serial[iTCIdm][iSampling];
      }
    }
    if (0) {
      for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) { //Only Pile-up noise use old method.
        if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // TC energy_tot cut
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

  if (_waveform == 1) {

    for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        WaveForm[iTCIdm][iSampling] = TCDigiE[iTCIdm][iSampling];
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
  TCDigiE.resize(576, vector<double>(666 , 0));
  TCDigiT.clear();
  TCDigiT.resize(576, vector<double>(666 , 0));
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
    if (TCEnergy_tot[iTCIdm] < cut_energy_tot) {continue;} // TC energy_tot cut
    for (int iTimeBin = 0; iTimeBin < 80; iTimeBin++) {
      if (TCEnergy[iTCIdm][iTimeBin] < 0.0001) { continue; }   // 0.1 MeV cut on TC bin_energy
      for (int iSampling = 0; iSampling < NSampling; iSampling++) {
        // inputTiming is in [us] <-- Be careful, here is NOT [ns]
        float inputTiming
          = (-TCTiming[iTCIdm][iTimeBin] - TimeRange + (-nbin_pedestal + iSampling) * fam_sampling_interval) * 0.001;

        inputTiming += random_sampling_correction * 0.001;
        if (inputTiming < 0 || inputTiming > 2.0) {continue;} // Shaping in t0 ~t0+ 2.0 us

        TCDigiEnergy[iTCIdm][iSampling] += FADC(0, inputTiming) * TCEnergy[iTCIdm][iTimeBin];
      }
    }
    for (int iSampling = 0; iSampling < NSampling; iSampling++) {
      TCDigiTiming[iTCIdm][iSampling] = (-nbin_pedestal + iSampling - TimeRange / fam_sampling_interval) * fam_sampling_interval;
      TCDigiTiming[iTCIdm][iSampling] += random_sampling_correction;
    }
  }
  //==================
  // (03)noise embedding
  //==================

  double tmin_noise = -4; // orignal
  double   tgen = 10.3;   //
  int bkg_level = 1030;
  double ttt0 = 0; // [us]
  double ttt1 = 0; // [us]
  double ttt2 = 0; // [us]
  //double frac_pileup   = 0.035; // pileup noise fraction?
  //double frac_parallel = 0.023; // parralel noise fraction?
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
    if (TCEnergy_tot[iTCIdm] < cut_energy_tot) { continue; }   // 1 MeV TC energy cut
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
      if (TCEnergy[iTCIdm][iBinTime] < 0.001) {continue;}
      StoreArray<TRGECLDigi0> TCDigiArray;
      TCDigiArray.appendNew();
      m_hitNum = TCDigiArray.getEntries() - 1;

      TCDigiArray[m_hitNum]->setEventId(m_nEvent);
      TCDigiArray[m_hitNum]->setTCId(iTCIdm + 1);
      TCDigiArray[m_hitNum]->setiBinTime(iBinTime);
      TCDigiArray[m_hitNum]->setRawEnergy(TCEnergy[iTCIdm][iBinTime]);
      TCDigiArray[m_hitNum]->setRawTiming(TCTiming[iTCIdm][iBinTime]);
      TCDigiArray[m_hitNum]->setBeamBkgTag(TCBeambkgTag[iTCIdm][iBinTime]);
    }
  }

  if (_waveform == 1) {
    StoreArray<TRGECLWaveform> TCWaveformArray;
    for (int iTCIdm = 0; iTCIdm < 576;  iTCIdm++) {
      if (iTCIdm == 80) iTCIdm =  512; // skip barrel
      int tc_phi_id = _TCMap->getTCPhiIdFromTCId(iTCIdm + 1);
      int tc_theta_id   = _TCMap->getTCThetaIdFromTCId(iTCIdm + 1);
      TRGECLWaveform* newWf =
        TCWaveformArray.appendNew(iTCIdm + 1, WaveForm[iTCIdm]);
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

  std::vector<double> SignalAmp;
  std::vector<double> SignalTime;

  SignalAmp.clear();
  SignalTime.clear();

  timing = timing * 1000;
  int startbin = (int)(timing) / 12;
  int endbin = startbin + 1;

  SignalAmp = { 0, 3.47505e-06, 0.000133173, 0.000939532, 0.00337321, 0.00845977, 0.0170396, 0.0296601, 0.0465713, 0.0677693, 0.0930545, 0.122086, 0.154429, 0.189595, 0.227068, 0.266331, 0.306882, 0.348243, 0.389971, 0.431664, 0.472959, 0.513539, 0.553127, 0.59149, 0.628431, 0.663791, 0.697445, 0.729297, 0.759281, 0.787354, 0.813494, 0.837698, 0.859982, 0.880372, 0.898908, 0.915639, 0.930622, 0.943919, 0.955598, 0.965731, 0.97439, 0.98165, 0.987587, 0.992275, 0.995788, 0.9982, 0.999581, 1, 0.999525, 0.998219, 0.996143, 0.993356, 0.989846, 0.985364, 0.979439, 0.971558, 0.961304, 0.948421, 0.932809, 0.914509, 0.893664, 0.870494, 0.845267, 0.818279, 0.789837, 0.76025, 0.729815, 0.698815, 0.667511, 0.636141, 0.604919, 0.574035, 0.543654, 0.513915, 0.484939, 0.456822, 0.429643, 0.403462, 0.378324, 0.354259, 0.331286, 0.309412, 0.288633, 0.26894, 0.250316, 0.232736, 0.216174, 0.200597, 0.185972, 0.172262, 0.159428, 0.147431, 0.136232, 0.12579, 0.116067, 0.107022, 0.0986191, 0.0908193, 0.0835871, 0.0768874, 0.0706867, 0.0649528, 0.0596551, 0.0547641, 0.0502523, 0.0460932, 0.042262, 0.0387353, 0.0354909, 0.0325082, 0.0297677, 0.0272511, 0.0249416, 0.0228232, 0.020881, 0.0191014, 0.0174714, 0.0159792, 0.0146138, 0.0133649, 0.012223, 0.0111793, 0.0102258, 0.00935504, 0.00856, 0.00783438, 0.00717232, 0.00656842, 0.00601773, 0.00551567, 0.00505807, 0.00464106, 0.00426114, 0.00391506, 0.00359985, 0.0033128, 0.00305143, 0.00281346, 0.00259681, 0.00239957, 0.00222002, 0.00205655, 0.00190773, 0.00177223, 0.00164885, 0.00153648, 0.00143413, 0.00134087, 0.00125589, 0.00117842, 0.00110777, 0.00104332, 0.000984488, 0.000930766, 0.000881678, 0.000836798, 0.000795734, 0.000758134, 0.000723677, 0.00069207, 0.000663051, 0.000636377, 0.000611832, 0.000589219, 0.000568358, 0.000549086, 0.000531258, 0.000514738, 0.000499407, 0.000485156, 0.000471884, 0.000459502, 0.00044793, 0.000437092, 0.000426923, 0.000417363, 0.000408356, 0.000399853, 0.000391811, 0.000384187, 0.000376946, 0.000370055, 0.000363483, 0.000357203, 0.000351192, 0.000345426, 0.000339886, 0.000334554, 0.000329413, 0.000324448, 0.000319645, 0.000314993, 0.000310481, 0.000306098, 0.000301836, 0.000297686, 0.00029364, 0.000289693, 0.000285837, 0.000282068, 0.00027838, 0.000274768, 0.000271229, 0.000267759, 0.000264354, 0.000261011, 0.000257727, 0.000254499, 0.000251326, 0.000248204, 0.000245132, 0.000242108, 0.000239131, 0.000236198, 0.000233308, 0.00023046, 0.000227653, 0.000224885, 0.000222155, 0.000219463, 0.000216807, 0.000214187, 0.000211602, 0.00020905, 0.000206532, 0.000204046, 0.000201593, 0.00019917, 0.000196779, 0.000194417, 0.000192085, 0.000189782, 0.000187508, 0.000185262, 0.000183044, 0.000180853, 0.000178689, 0.000176552, 0.000174441, 0.000172355, 0.000170295, 0.00016826, 0.000166249, 0.000164263, 0.000162301,  };

  SignalTime = { 0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 168, 180, 192, 204, 216, 228, 240, 252, 264, 276, 288, 300, 312, 324, 336, 348, 360, 372, 384, 396, 408, 420, 432, 444, 456, 468, 480, 492, 504, 516, 528, 540, 552, 564, 576, 588, 600, 612, 624, 636, 648, 660, 672, 684, 696, 708, 720, 732, 744, 756, 768, 780, 792, 804, 816, 828, 840, 852, 864, 876, 888, 900, 912, 924, 936, 948, 960, 972, 984, 996, 1008, 1020, 1032, 1044, 1056, 1068, 1080, 1092, 1104, 1116, 1128, 1140, 1152, 1164, 1176, 1188, 1200, 1212, 1224, 1236, 1248, 1260, 1272, 1284, 1296, 1308, 1320, 1332, 1344, 1356, 1368, 1380, 1392, 1404, 1416, 1428, 1440, 1452, 1464, 1476, 1488, 1500, 1512, 1524, 1536, 1548, 1560, 1572, 1584, 1596, 1608, 1620, 1632, 1644, 1656, 1668, 1680, 1692, 1704, 1716, 1728, 1740, 1752, 1764, 1776, 1788, 1800, 1812, 1824, 1836, 1848, 1860, 1872, 1884, 1896, 1908, 1920, 1932, 1944, 1956, 1968, 1980, 1992, 2004, 2016, 2028, 2040, 2052, 2064, 2076, 2088, 2100, 2112, 2124, 2136, 2148, 2160, 2172, 2184, 2196, 2208, 2220, 2232, 2244, 2256, 2268, 2280, 2292, 2304, 2316, 2328, 2340, 2352, 2364, 2376, 2388, 2400, 2412, 2424, 2436, 2448, 2460, 2472, 2484, 2496, 2508, 2520, 2532, 2544, 2556, 2568, 2580, 2592, 2604, 2616, 2628, 2640, 2652, 2664, 2676, 2688, 2700, 2712, 2724, 2736, 2748, 2760, 2772, 2784, 2796, 2808, 2820, 2832, 2844, 2856, 2868, 2880, 2892, 2904, 2916, 2928,  };


  double E_out = 0;
  if (timing < 0 || timing > 2928) {return E_out;}

  if (timing > SignalTime[startbin] && timing < SignalTime[endbin]) {
    E_out = (((SignalAmp[endbin] - SignalAmp[startbin])) / (SignalTime[endbin] - SignalTime[startbin])) *
            (timing - SignalTime[startbin]) +  SignalAmp[startbin];
    return E_out;
  } else {
    E_out = 0;
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
