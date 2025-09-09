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
TrgEclFAMFit::TrgEclFAMFit(): m_BeamBkgTag(0), m_AnaTag(0), m_EventId(0) //, bin(0)
{

  m_CoeffSigPDF0.clear();
  m_CoeffSigPDF1.clear();
  m_CoeffNoise31.clear();
  m_CoeffNoise32.clear();
  m_CoeffNoise33.clear();

  m_TCMap = new TrgEclMapping();
  m_DataBase = new TrgEclDataBase();

  m_TCEThreshold.clear();

  m_TCFitEnergy.clear();
  m_TCFitTiming.clear();
  m_TCRawEnergy.clear();
  m_TCRawTiming.clear();
  m_BeamBkgInfo.clear();
  m_TCLatency.clear();

  m_TCEThreshold.resize(576, 100.0);
  m_TCFitEnergy.resize(576);
  m_TCFitTiming.resize(576);
  m_TCRawEnergy.resize(576);
  m_TCRawTiming.resize(576);
  m_BeamBkgInfo.resize(576);
}
//
//
//
TrgEclFAMFit::~TrgEclFAMFit()
{

  delete m_TCMap;
  delete m_DataBase;
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

  m_DataBase->getCoeffSigPDF(m_CoeffSigPDF0,  m_CoeffSigPDF1);
  m_DataBase->getCoeffNoise(0,  m_CoeffNoise31, m_CoeffNoise32, m_CoeffNoise33);

  m_EventId = eventId;
  //
  return;
}

void
TrgEclFAMFit::FAMFit01(std::vector<std::vector<double>> digiEnergy,
                       std::vector<std::vector<double>> digiTiming)
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
  int dTBin = 0;
  int ShiftdTBin = 0;
  int Nsmalldt = 10;
  int SmallOffset = 1;
  double IntervaldT  = 125 * 0.001 / Nsmalldt;
  //  double EThreshold = m_Threshold; //[MeV]
  int FitSleepCounter   = 100; // counter to suspend fit
  int FitSleepThreshold = 12;   // # of clk to suspend fit

  for (int iTCIdm = 0; iTCIdm < 576; iTCIdm++) {

    for (int iShift = 20; iShift < (NSampling - 12); iShift++) { // In order to avoid BKG shaping effect, iShift start from 20.

      FitSleepCounter++;
      if (FitSleepCounter <= FitSleepThreshold) {continue;}
      for (int iFitSample = 0; iFitSample < 12; iFitSample++) {
        int iReplace = iFitSample + iShift;
        TCFitSample[iFitSample] = digiEnergy[iTCIdm][iReplace] * 1000.0;
        if (0) {
          if (pedFlag == 1 && iFitSample < 4) {
            // cppcheck-suppress uninitdata
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
      for (int iFitSample = 0; iFitSample < 12; iFitSample++) {
        CoeffAAA += m_CoeffNoise31[dTBin - 1][iFitSample] * TCFitSample[iFitSample];
        CoeffBBB += m_CoeffNoise32[dTBin - 1][iFitSample] * TCFitSample[iFitSample];
      }
      double deltaT = CoeffBBB / CoeffAAA; // deltaT [us]

      ShiftdTBin = int(deltaT / IntervaldT + dTBin);

      double fitE = CoeffAAA;

      //-------
      // Require "expected time" is around middle of table = Nsmalldt.
      //-------
      double condition_t = -(deltaT + dTBin * IntervaldT - fam_sampling_interval * 0.001);

      if (fabs(condition_t) < 0.8 * (fam_sampling_interval * 0.001) && fitE > m_TCEThreshold[iTCIdm]) {
        double fitT =
          condition_t +
          (SmallOffset + iShift + nbin_pedestal - 5) * (fam_sampling_interval * 0.001);


        pedFlag = 1;
        double rand_sampling_correction =
          digiTiming[iTCIdm][iShift] +
          (nbin_pedestal - iShift + 32) * fam_sampling_interval;

        m_TCFitEnergy[iTCIdm].push_back(fitE / 1000.0); // [GeV/c2]
        m_TCFitTiming[iTCIdm].push_back(fitT * 1000 - 4000 +
                                        (m_DataBase->getTCFLatency(iTCIdm + 1)) +
                                        rand_sampling_correction);
        FitSleepCounter = 0;
        ShiftdTBin = 0;

      }
    }
  }
  if (m_BeamBkgTag == 1 || m_AnaTag == 1) {
    setBeamBkgTag();
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
TrgEclFAMFit::FAMFit02(std::vector<std::vector<double>> TCDigiE,
                       std::vector<std::vector<double>> TCDigiT)
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

    //    double threshold = m_Threshold * 0.001; //GeV
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
            if (m_TCEThreshold[iTCIdm] * 0.001 < max) {
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
              m_TCFitEnergy[iTCIdm].push_back(TCDigiE[iTCIdm][maxId[noutput]] - NoiseLevel);
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

                if (TCDigiE[iTCIdm][maxId[noutput] - iSearch]  > 0.6 * m_TCFitEnergy[iTCIdm][noutput] &&
                    TCDigiE[iTCIdm][maxId[noutput] - iSearch - 1] < 0.6 * m_TCFitEnergy[iTCIdm][noutput]) {
                  ta_id[noutput] = maxId[noutput] - iSearch - 1;
                }
              }

              //@ Estimate timing of t0
              if (ta_id[noutput] == 1000) {
                printf("TrgEclFAMFit::digi02> Cannot find TC Timing (TCId=%5i, E=%8.5f)!!!\n", iTCIdm - 1, m_TCFitEnergy[iTCIdm][0]);
                B2ERROR("TrgEclFAMFit::digi02> Cannot find TC Timing");
              } else {
                ttt_a[noutput] = TCDigiT[iTCIdm][ta_id[noutput]];
                ttt_b[noutput] = TCDigiT[iTCIdm][ta_id[noutput] + 1];
                m_TCFitTiming[iTCIdm].push_back((ttt_a[noutput] +
                                                 (0.6 * m_TCFitEnergy[iTCIdm][noutput] - TCDigiE[iTCIdm][ta_id[noutput]]) * (ttt_b[noutput] -
                                                     ttt_a[noutput])
                                                 / (TCDigiE[iTCIdm][ta_id[noutput] + 1] - TCDigiE[iTCIdm][ta_id[noutput]])) - (278.7 + 2) + (m_DataBase->getTCFLatency(iTCIdm + 1)));
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
  if (m_BeamBkgTag == 1 || m_AnaTag == 1) {
    setBeamBkgTag();
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
TrgEclFAMFit::FAMFit03(std::vector<std::vector<double>> TCDigiEnergy,
                       std::vector<std::vector<double>> TCDigiTiming)
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
  //  double threshold = m_Threshold * 0.001; //GeV
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
            if (m_TCEThreshold[iTCIdm] * 0.001 < max) {
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
              m_TCFitEnergy[iTCIdm].push_back(TCDigiEnergy[iTCIdm][maxId[noutput]] - NoiseLevel);
              m_TCFitTiming[iTCIdm].push_back(TCDigiTiming[iTCIdm][maxId[noutput]] - max_shape_time + (m_DataBase->getTCFLatency(iTCIdm)));
              noutput++;
            }
          }
        }
      }
    }
  }

  if (m_BeamBkgTag == 1 || m_AnaTag == 1) {
    setBeamBkgTag();
  }


  return;
}
//
//
//
void
TrgEclFAMFit::setBeamBkgTag()
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
  // m_TCRawEnergy.resize(576,std::vector<double>(size,0.));
  // m_TCRawTiming.resize(576,std::vector<double>(size,0.));
  m_BeamBkgInfo.resize(576);
  m_TCRawEnergy.resize(576);
  m_TCRawTiming.resize(576);

  StoreArray<TRGECLDigi0> trgeclDigiArray;
  for (int ii = 0; ii < trgeclDigiArray.getEntries(); ii++) {
    TRGECLDigi0* aTRGECLDigi = trgeclDigiArray[ii];
    int eventid = aTRGECLDigi->getEventId();
    if (m_EventId != eventid) {continue;}
    TCId.push_back(aTRGECLDigi->getTCId());
    RawTCTiming.push_back(aTRGECLDigi -> getRawTiming());
    RawTCEnergy.push_back(aTRGECLDigi -> getRawEnergy());
    RawBeamBkgTag.push_back(aTRGECLDigi -> getBeamBkgTag());
  }

  for (int iTCId = 0; iTCId < 576 ; iTCId ++) {
    const int hitsize = m_TCFitEnergy[iTCId].size();
    for (int iHit = 0; iHit < hitsize; iHit++) {
      const int rawsize = TCId.size();
      for (int iDigi = 0; iDigi < rawsize; iDigi++) {
        if (TCId[iDigi] != (iTCId + 1)) {continue;}
        if (abs(m_TCFitEnergy[iTCId][iHit] - RawTCEnergy[iDigi]) < 12) {
          if (abs(m_TCFitTiming[iTCId][iHit] - RawTCTiming[iDigi]) < 30) {
            m_BeamBkgInfo[iTCId].push_back(RawBeamBkgTag[iDigi]);
            m_TCRawEnergy[iTCId].push_back(RawTCEnergy[iDigi]);
            m_TCRawTiming[iTCId].push_back(RawTCTiming[iDigi]);
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
TrgEclFAMFit::save(int eventid)
{
  //---------------
  // Root Output
  //---------------
  int hitNum = 0;

  // adjust TC timing (ns) to T=0 ns
  double tc_timing_correction = -15;

  for (int iTCIdm = 0; iTCIdm < 576;  iTCIdm++) {
    const int hitsize = m_TCFitEnergy[iTCIdm].size();
    for (int iHit = 0; iHit < hitsize; iHit++) {
      StoreArray<TRGECLHit> TrgEclHitArray;
      TrgEclHitArray.appendNew();
      hitNum = TrgEclHitArray.getEntries() - 1;
      TrgEclHitArray[hitNum]->setEventId(eventid);
      TrgEclHitArray[hitNum]->setTCId(iTCIdm + 1);
      TrgEclHitArray[hitNum]->setEnergyDep(m_TCFitEnergy[iTCIdm][iHit]);
      TrgEclHitArray[hitNum]->setTimeAve(m_TCFitTiming[iTCIdm][iHit] + tc_timing_correction);
      if (m_BeamBkgTag == 1) {
        TrgEclHitArray[hitNum]->setBeamBkgTag(m_BeamBkgInfo[iTCIdm][iHit]);
      }
    }
  }

  if (m_AnaTag == 1) {
    for (int iTCIdm = 0; iTCIdm < 576;  iTCIdm++) {
      if (m_TCFitEnergy[iTCIdm].size() != m_TCRawEnergy[iTCIdm].size()) {continue;}
      const int hitsize = m_TCFitEnergy[iTCIdm].size();
      for (int iHit = 0; iHit < hitsize; iHit++) {
        StoreArray<TRGECLFAMAna> TrgEclAnaArray;
        TrgEclAnaArray.appendNew();
        hitNum = TrgEclAnaArray.getEntries() - 1;
        TrgEclAnaArray[hitNum]->setEventId(eventid);
        TrgEclAnaArray[hitNum]->setTCId(iTCIdm + 1);
        TrgEclAnaArray[hitNum]->setPhiId(m_TCMap->getTCPhiIdFromTCId(iTCIdm + 1));
        TrgEclAnaArray[hitNum]->setThetaId(m_TCMap->getTCThetaIdFromTCId(iTCIdm + 1));
        TrgEclAnaArray[hitNum]->setRawEnergy(m_TCRawEnergy[iTCIdm][iHit]);
        TrgEclAnaArray[hitNum]->setRawTiming(m_TCRawTiming[iTCIdm][iHit]);

        //        TrgEclAnaArray[hitNum]->setFitEnergy(m_TCFitEnergy[iTCIdm][iHit]);
        int p_ene2adc = 525;
        int ene_i0 = (int)(m_TCFitEnergy[iTCIdm][iHit] * 100000.0 / p_ene2adc);
        double ene_d = (double) ene_i0 * p_ene2adc /  100000;
        TrgEclAnaArray[hitNum]->setFitEnergy(ene_d);

        TrgEclAnaArray[hitNum]->setFitTiming(m_TCFitTiming[iTCIdm][iHit] + tc_timing_correction);
        TrgEclAnaArray[hitNum]->setBeamBkgTag(m_BeamBkgInfo[iTCIdm][iHit]);
      }
    }
  }
  return;
}
