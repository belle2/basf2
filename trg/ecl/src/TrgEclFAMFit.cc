/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/datastore/StoreArray.h>

#include <framework/logging/Logger.h>
#include "trg/ecl/TrgEclFAMFit.h"

#include "trg/ecl/dataobjects/TRGECLHit.h"
#include "trg/ecl/dataobjects/TRGECLDigi0.h"
#include "trg/ecl/dataobjects/TRGECLFAMAna.h"

#include <stdlib.h>
#include <iostream>
#include <math.h>

using namespace std;
using namespace Belle2;
//using namespace TRG;
//
//
//
TrgEclFAMFit::TrgEclFAMFit(): _BeamBkgTag(0), _AnaTag(0), EventId(0) //, bin(0)
{

  CoeffSigPDF0.clear();
  CoeffSigPDF1.clear();
  CoeffNoise31.clear();
  CoeffNoise32.clear();
  CoeffNoise33.clear();

  _TCMap = new TrgEclMapping();
  _DataBase = new TrgEclDataBase();

  Threshold.clear();

  TCFitEnergy.clear();
  TCFitTiming.clear();
  TCRawEnergy.clear();
  TCRawTiming.clear();
  BeamBkgTag.clear();
  TCLatency.clear();

  Threshold.resize(576, 100.0);
  TCFitEnergy.resize(576);
  TCFitTiming.resize(576);
  TCRawEnergy.resize(576);
  TCRawTiming.resize(576);
  BeamBkgTag.resize(576);
}
//
//
//
TrgEclFAMFit::~TrgEclFAMFit()
{

  delete _TCMap;
  delete _DataBase;
}
//
//
//

void
TrgEclFAMFit::setup(int eventId)
{
  //
  // prepare coefficient for fitting
  //

  _DataBase -> getCoeffSigPDF(CoeffSigPDF0 ,  CoeffSigPDF1);
  _DataBase -> getCoeffNoise(0,  CoeffNoise31 , CoeffNoise32, CoeffNoise33);

  EventId = eventId;
  //
  return;
}

void
TrgEclFAMFit::FAMFit01(std::vector<std::vector<double>> digiEnergy, std::vector<std::vector<double>> digiTiming)
{
  //============================
  // In this function,
  // o Energy unit must be [MeV/c2]
  // o Time unit must be [us]
  // but
  // o return energy unit must be [GeV/c2]
  // o return time unit must be [ns]
  //============================
  double* TCFitSample = new double [12];  // MeV/c2
  double* preped = new double [4];

  int nbin_pedestal = 4;
  int fam_sampling_interval = 125;
  int NSampling = 64;// 8 [us] / 125 [ns]

  int pedFlag = 0;
  double CoeffAAA = 0;
  double CoeffBBB = 0;
  double CoeffPPP = 0;
  int dTBin = 0;
  int ShiftdTBin = 0;
  int Nsmalldt = 10;
  int SmallOffset = 1;
  double IntervaldT  = 125 * 0.001 / Nsmalldt;
  //  double EThreshold = _Threshold; //[MeV]
  int FitSleepCounter   = 100; // counter to suspend fit
  int FitSleepThreshold = 2;   // # of clk to suspend fit
  /* cppcheck-suppress variableScope */
  double FitE;
  /* cppcheck-suppress variableScope */
  double FitT;

  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {

    FitE = 0;
    FitT = 0;
    for (int iShift = 20; iShift < (NSampling - 12); iShift++) { // In order to avoid BKG shaping effect, iShift start from 20.

      FitSleepCounter++;
      if (FitSleepCounter <= FitSleepThreshold) {continue;}
      for (int iFitSample = 0; iFitSample < 12; iFitSample++) {
        int iReplace = iFitSample + iShift;
        TCFitSample[iFitSample] = digiEnergy[iTCIdm][iReplace] * 1000.0;
        if (0) {
          if (pedFlag == 1 && iFitSample < 4) {
            TCFitSample[iFitSample] = preped[iFitSample];
            pedFlag = 0;
          }
        }
      }
      //
      //
      //

      dTBin = (int)(ShiftdTBin + Nsmalldt);
      if (dTBin < 1)  {dTBin =   1;}
      if (dTBin > 20) {dTBin = 20;}

      CoeffAAA = 0;
      CoeffBBB = 0;
      CoeffPPP = 0;
      for (int iFitSample = 0; iFitSample < 12; iFitSample++) {
        CoeffAAA += CoeffNoise31[dTBin - 1][iFitSample] * TCFitSample[iFitSample];
        CoeffBBB += CoeffNoise32[dTBin - 1][iFitSample] * TCFitSample[iFitSample];
        CoeffPPP += CoeffNoise33[dTBin - 1][iFitSample] * TCFitSample[iFitSample];
      }
      double deltaT = CoeffBBB / CoeffAAA; // deltaT [us]

      ShiftdTBin = int(deltaT / IntervaldT + dTBin);

      FitE = CoeffAAA;

      //-------
      // Require "expected time" is around middle of table = Nsmalldt.
      //-------
      double condition_t = -(deltaT + dTBin * IntervaldT - fam_sampling_interval * 0.001);

      if (fabs(condition_t) < 0.8 * (fam_sampling_interval * 0.001) && FitE > Threshold[iTCIdm]) {
        FitT = condition_t + (SmallOffset + iShift + nbin_pedestal - 5) * (fam_sampling_interval * 0.001);


        pedFlag = 1;
        double rand_sampling_correction = digiTiming[iTCIdm][iShift] + (nbin_pedestal - iShift + 32) * fam_sampling_interval;

        TCFitEnergy[iTCIdm].push_back(FitE / 1000.0); // [GeV/c2]
        TCFitTiming[iTCIdm].push_back(FitT * 1000 - 4000 + (_DataBase->GetTCFLatency(iTCIdm + 1)) + rand_sampling_correction);
        FitSleepCounter = 0;
        ShiftdTBin = 0;

      }
    }
  }
  if (_BeamBkgTag == 1 || _AnaTag == 1) {
    SetBeamBkgTag();
  }
  //
  //
  //
  delete [] TCFitSample;
  delete [] preped;
  return;
}
//
//
//
void
TrgEclFAMFit::FAMFit02(std::vector<std::vector<double>> TCDigiE, std::vector<std::vector<double>> TCDigiT)
{

  int NSampling = 64;

  //==================
  // Peak search
  //==================
  //@ T_a and T_b is time at sampling points in which 0.6*E exists.
  int ta_id[20] = {1000}; //@ id of T_a
  double ttt_a[20] = {0}; //@ time of T_a
  double ttt_b[20] = {0}; //@ time of T_b
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    int noutput = 0;

    //    double threshold = _Threshold * 0.001; //GeV
    int maxId[500] = {0};
    for (int iii = 0 ; iii < 500 ; iii++) {

      maxId[iii] = 0;
    }
    int count_up = 0;
    int count_down = 0;
    int flag_up = 3;
    int flag_down = 3;

    double max = 0;
    for (int iSampling = 1; iSampling < NSampling; iSampling++) {
      //-------------------------------------------------------------------------
      //Peak finding Method 1
      //------------------------------------------------------------------------

      if (TCDigiE[iTCIdm][iSampling] >= max) {

        max = TCDigiE[iTCIdm][iSampling];
        maxId[noutput] = iSampling;
        count_up ++;
        count_down = 0;
      } else {
        count_down++;
        if (count_down >= flag_down) {
          if (count_up >= flag_up) {
            if (Threshold[iTCIdm] * 0.001 < max) {
              max = 0;
              count_up = 0;
              count_down = 0;

              double NoiseLevel = 0;
              double NoiseCount = 0;
              for (int iNoise = 0; iNoise < 5; iNoise++) {
                int iNoiseReplace = (maxId[noutput] - 10) + iNoise;
                if (iNoiseReplace >= 0) {
                  NoiseLevel += TCDigiE[iTCIdm][iNoiseReplace];
                  NoiseCount++;
                }
              }
              if (NoiseCount != 0) { NoiseLevel /= NoiseCount; }
              //** Peak point is the Energy */
              TCFitEnergy[iTCIdm].push_back(TCDigiE[iTCIdm][maxId[noutput]] - NoiseLevel);
              if (!(maxId[noutput] - 1)) {
                for (int jSampling = 1; jSampling < maxId[noutput] + 3; jSampling++) {
                  TCDigiE[iTCIdm][jSampling] -= NoiseLevel;
                }
              } else {
                for (int jSampling = maxId[noutput] - 1; jSampling < maxId[noutput] + 3; jSampling++) {
                  TCDigiE[iTCIdm][jSampling] -= NoiseLevel;
                }
              }
              //@ Search T_a ID
              for (int iSearch = 0; iSearch < 5; iSearch++) {

                if (TCDigiE[iTCIdm][maxId[noutput] - iSearch]  > 0.6 * TCFitEnergy[iTCIdm][noutput] &&
                    TCDigiE[iTCIdm][maxId[noutput] - iSearch - 1] < 0.6 * TCFitEnergy[iTCIdm][noutput]) {
                  ta_id[noutput] = maxId[noutput] - iSearch - 1;
                }
              }

              //@ Estimate timing of t0
              if (ta_id[noutput] == 1000) {
                printf("TrgEclFAMFit::digi02> Cannot find TC Timing (TCId=%5i, E=%8.5f)!!!\n", iTCIdm - 1, TCFitEnergy[iTCIdm][0]);
                B2ERROR("TrgEclFAMFit::digi02> Cannot find TC Timing");
              } else {
                ttt_a[noutput] = TCDigiT[iTCIdm][ta_id[noutput]];
                ttt_b[noutput] = TCDigiT[iTCIdm][ta_id[noutput] + 1];
                TCFitTiming[iTCIdm].push_back((ttt_a[noutput] +
                                               (0.6 * TCFitEnergy[iTCIdm][noutput] - TCDigiE[iTCIdm][ta_id[noutput]]) * (ttt_b[noutput] -
                                                   ttt_a[noutput])
                                               / (TCDigiE[iTCIdm][ta_id[noutput] + 1] - TCDigiE[iTCIdm][ta_id[noutput]])) - (278.7 + 2) + (_DataBase->GetTCFLatency(iTCIdm + 1)));
                //@ time between t0 and 0.6*peak_energy
                //@ Alex's number = 274.4 (how he got this value ?)
                //@ by my check = 278.7 [ns]
                //@ here "+2" is a shift due to imperfectness of no-fit method.
              }

            }
          }
        }
      }
    }
  }
  if (_BeamBkgTag == 1 || _AnaTag == 1) {
    SetBeamBkgTag();
  }

  //
  //
  //
  return;

}
//
//
//
void
TrgEclFAMFit::FAMFit03(std::vector<std::vector<double>> TCDigiEnergy, std::vector<std::vector<double>> TCDigiTiming)
{
  //===============
  // (03)Signal digitization (w/ 12ns interval for method-0)
  //===============
  // double cut_energy_tot = 0.03; // [GeV]
  // int nbin_pedestal = 100;
  // float fam_sampling_interval = 12; // [ns]
  int NSampling = 666;

  //==================
  // (03)Peak search
  //==================
  float max_shape_time = 563.48; // [ns], time between peak of PDF and t0.
  //  double threshold = _Threshold * 0.001; //GeV
  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {
    int noutput = 0;
    int maxId[500] = {0};
    int count_up = 0;
    int count_down = 0;
    int flag_up = 30;;
    int flag_down = 40;
    double max = 0;
    for (int iSampling = 1; iSampling < NSampling; iSampling++) {

      if (TCDigiEnergy[iTCIdm][iSampling] >= max) {

        max = TCDigiEnergy[iTCIdm][iSampling];
        maxId[noutput] = iSampling;
        count_up ++;
        count_down = 0;
      } else {
        count_down++;
        if (count_down >= flag_down) {
          if (count_up >= flag_up) {
            if (Threshold[iTCIdm] * 0.001 < max) {
              max = 0;
              count_up = 0;
              count_down = 0;
              //@ Remove noise effect
              float NoiseLevel = 0;
              float NoiseCount = 0;
              for (int iNoise = 0; iNoise < 42; iNoise++) {
                int iNoiseReplace = (maxId[noutput] - 88) + iNoise;
                if (iNoiseReplace >= 0) {
                  NoiseLevel += TCDigiEnergy[iTCIdm][iNoiseReplace];
                  NoiseCount++;
                }
              }
              if (NoiseCount != 0) { NoiseLevel /= NoiseCount; }
              TCFitEnergy[iTCIdm].push_back(TCDigiEnergy[iTCIdm][maxId[noutput]] - NoiseLevel);
              TCFitTiming[iTCIdm].push_back(TCDigiTiming[iTCIdm][maxId[noutput]] - max_shape_time + (_DataBase->GetTCFLatency(iTCIdm)));
              noutput++;
            }
          }
        }
      }
    }
  }

  if (_BeamBkgTag == 1 || _AnaTag == 1) {
    SetBeamBkgTag();
  }


  return;
}
//
//
//
void
TrgEclFAMFit::SetBeamBkgTag()
{
  std::vector<int> TCId;
  std::vector<double> RawTCTiming;
  std::vector<double> RawTCEnergy;
  std::vector<double> RawBeamBkgTag;
  RawBeamBkgTag.clear();
  TCId.clear();
  RawTCTiming.clear();
  RawTCEnergy.clear();
  // BeamBkgTag.resize(576  ,std::vector<int> (size,100));
  // TCRawEnergy.resize(576,std::vector<double>(size,0.));
  // TCRawTiming.resize(576,std::vector<double>(size,0.));
  BeamBkgTag.resize(576);
  TCRawEnergy.resize(576);
  TCRawTiming.resize(576);

  StoreArray<TRGECLDigi0> trgeclDigiArray;
  for (int ii = 0; ii < trgeclDigiArray.getEntries(); ii++) {
    TRGECLDigi0* aTRGECLDigi = trgeclDigiArray[ii];
    int eventid = aTRGECLDigi->getEventId();
    if (EventId != eventid) {continue;}
    TCId.push_back(aTRGECLDigi->getTCId());
    RawTCTiming.push_back(aTRGECLDigi -> getRawTiming());
    RawTCEnergy.push_back(aTRGECLDigi -> getRawEnergy());
    RawBeamBkgTag.push_back(aTRGECLDigi -> getBeamBkgTag());
  }

  for (int iTCId = 0; iTCId < 576 ; iTCId ++) {
    const int hitsize = TCFitEnergy[iTCId].size();
    for (int iHit = 0; iHit < hitsize; iHit++) {
      const int rawsize = TCId.size();
      for (int iDigi = 0; iDigi < rawsize; iDigi++) {
        if (TCId[iDigi] != (iTCId + 1)) {continue;}
        if (abs(TCFitEnergy[iTCId][iHit] - RawTCEnergy[iDigi]) < 12) {
          if (abs(TCFitTiming[iTCId][iHit] - RawTCTiming[iDigi]) < 30) {
            BeamBkgTag[iTCId].push_back(RawBeamBkgTag[iDigi]);
            TCRawEnergy[iTCId].push_back(RawTCEnergy[iDigi]);
            TCRawTiming[iTCId].push_back(RawTCTiming[iDigi]);
          }
        }
      }
    }
  }

}
//
//
//
void
TrgEclFAMFit::save(int m_nEvent)
{
  //---------------
  // Root Output
  //---------------
  int m_hitNum = 0;

  for (int iTCIdm = 0; iTCIdm < 576;  iTCIdm++) {
    const int hitsize = TCFitEnergy[iTCIdm].size();
    for (int iHit = 0; iHit < hitsize; iHit++) {
      StoreArray<TRGECLHit> TrgEclHitArray;
      TrgEclHitArray.appendNew();
      m_hitNum = TrgEclHitArray.getEntries() - 1;
      TrgEclHitArray[m_hitNum]->setEventId(m_nEvent);
      TrgEclHitArray[m_hitNum]->setTCId(iTCIdm + 1);
      TrgEclHitArray[m_hitNum]->setEnergyDep(TCFitEnergy[iTCIdm][iHit]);
      TrgEclHitArray[m_hitNum]->setTimeAve(TCFitTiming[iTCIdm][iHit]);
      if (_BeamBkgTag == 1) {
        TrgEclHitArray[m_hitNum]->setBeamBkgTag(BeamBkgTag[iTCIdm][iHit]);
      }
    }
  }

  m_hitNum = 0;
  if (_AnaTag == 1) {
    for (int iTCIdm = 0; iTCIdm < 576;  iTCIdm++) {
      if (TCFitEnergy[iTCIdm].size() != TCRawEnergy[iTCIdm].size()) {continue;}
      const int hitsize = TCFitEnergy[iTCIdm].size();
      for (int iHit = 0; iHit < hitsize; iHit++) {
        StoreArray<TRGECLFAMAna> TrgEclAnaArray;
        TrgEclAnaArray.appendNew();
        m_hitNum = TrgEclAnaArray.getEntries() - 1;
        TrgEclAnaArray[m_hitNum]->setEventId(m_nEvent);
        TrgEclAnaArray[m_hitNum]->setTCId(iTCIdm + 1);
        TrgEclAnaArray[m_hitNum]->setPhiId(_TCMap->getTCPhiIdFromTCId(iTCIdm + 1));
        TrgEclAnaArray[m_hitNum]->setThetaId(_TCMap->getTCThetaIdFromTCId(iTCIdm + 1));
        TrgEclAnaArray[m_hitNum]->setRawEnergy(TCRawEnergy[iTCIdm][iHit]);
        TrgEclAnaArray[m_hitNum]->setRawTiming(TCRawTiming[iTCIdm][iHit]);

        //        TrgEclAnaArray[m_hitNum]->setFitEnergy(TCFitEnergy[iTCIdm][iHit]);
        int p_ene2adc = 525;
        int ene_i0 = (int)(TCFitEnergy[iTCIdm][iHit] * 100000.0 / p_ene2adc);
        double ene_d = (double) ene_i0 * p_ene2adc /  100000;
        TrgEclAnaArray[m_hitNum]->setFitEnergy(ene_d);

        TrgEclAnaArray[m_hitNum]->setFitTiming(TCFitTiming[iTCIdm][iHit]);
        TrgEclAnaArray[m_hitNum]->setBeamBkgTag(BeamBkgTag[iTCIdm][iHit]);
      }
    }
  }
  return;
}
