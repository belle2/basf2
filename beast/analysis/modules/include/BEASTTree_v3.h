/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BEASTTREE_V3_H
#define BEASTTREE_V3_H

#include <TTree.h>

namespace Belle2 {
  namespace BEAST_v3 {

    /**
     * Structure for the input of the data BEAST (TTree "Data_BEAST")
     * https://belle2.cc.kek.jp/~twiki/bin/view/Detector/BEAST/ROOTDataFormats
     */
    struct BEASTTree_v3 {

      /**Beast PVs **/
      UInt_t ts; /** */
      UInt_t event; /** */
      UInt_t run; /** */
      UInt_t subrun; /** */
      std::vector<double>*  SKB_HER_current;/** */
      std::vector<double>*  SKB_LER_current;/** */
      std::vector<double>*  SKB_HER_lifetime;/** */
      std::vector<double>*  SKB_LER_lifetime;/** */
      std::vector<double>*  SKB_HER_integratedCurrent;/** */
      std::vector<double>*  SKB_LER_integratedCurrent;
      std::vector<double>*  SKB_HER_injectionFlag;/** */
      std::vector<double>*  SKB_LER_injectionFlag;/** */
      std::vector<double>*  SKB_HER_injectionRate;/** */
      std::vector<double>*  SKB_LER_injectionRate;/** */
      std::vector<double>*  SKB_HER_injectionEfficiency;/** */
      std::vector<double>*  SKB_LER_injectionEfficiency;/** */
      std::vector<double>*  SKB_HER_injectionCharge;/** */
      std::vector<double>*  SKB_LER_injectionCharge;/** */
      std::vector<double>*  SKB_HER_injectionRepetitionRate;/** */
      std::vector<double>*  SKB_LER_injectionRepetitionRate;/** */
      std::vector<double>*  SKB_HER_injectionNumberOfBunches;/** */
      std::vector<double>*  SKB_LER_injectionNumberOfBunches;/** */
      std::vector<double>*  SKB_HER_beamSize_xray_X;/** */
      std::vector<double>*  SKB_HER_beamSize_xray_Y;/** */
      std::vector<double>*  SKB_LER_beamSize_xray_X;/** */
      std::vector<double>*  SKB_LER_beamSize_xray_Y;/** */
      std::vector<double>*  SKB_HER_beamSize_SR_Y;/** */
      std::vector<double>*  SKB_LER_beamSize_SR_Y;/** */
      std::vector<double>*  SKB_HER_beamSize_SR_X;/** */
      std::vector<double>*  SKB_LER_beamSize_SR_X;/** */
      std::vector<double>*  SKB_beamLoss_nearCollimators;/** */
      std::vector<double>*  SKB_beamLoss_aroundMasks;/** */
      std::vector<double>*  SKB_HER_pressure_average;/** */
      std::vector<double>*  SKB_LER_pressure_average;/** */
      std::vector<float>*   SKB_HER_pressures;/** */
      std::vector<float>*   SKB_LER_pressures;/** */
      std::vector<float>*   SKB_LER_pressures_local;/** */
      std::vector<float>*   SKB_HER_collimatorPositions_inX;/** */
      std::vector<float>*   SKB_HER_collimatorPositions_inY;/** */
      std::vector<float>*   SKB_HER_collimatorPositions_mm;/** */
      std::vector<float>*   SKB_LER_collimatorPositions_mm;/** */
      std::vector<float>*   SKB_LER_partialPressures_D06;/** */
      std::vector<float>*   SKB_LER_partialPressures_D02;/** */
      std::vector<float>*   DIA_current;/** */
      std::vector<float>*   CSI_temperature;/** */
      std::vector<float>*   CSI_humidity;/** */
      std::vector<float>*   TPC_gas_flow;/** */
      std::vector<float>*   TPC_gas_pressure;/** */
      std::vector<double>*  SKB_Status;/** */
      std::vector<short>*   SKB_beamLoss_ionChambers_mean;/** */
      std::vector<short>*   SKB_beamLoss_PINdiodes_mean;/** */
      std::vector<float>*   SKB_HER_collimatorPositions_DMM;/** */
      std::vector<float>*   SKB_HER_collimatorPositions_fromBeam;/** */
      std::vector<float>*   SKB_LER_collimatorPositions_DMM;/** */
      std::vector<float>*   SKB_LER_collimatorPositions_X;/** */
      std::vector<float>*   SKB_LER_collimatorPositions_Y;/** */
      std::vector<float>*   SKB_LER_collimatorPositions_fromBeam;/** */
      std::vector<float>*   PIN_voltage;/** */
      std::vector<float>*   PIN_temperature;/** */
      std::vector<float>* PIN_dose;/** */
      std::vector<float> PIN_dose_mc;/** */
      std::vector<float> PIN_dose_mc_av;/** */
      std::vector<int>*     TPC3_idx;/** */
      std::vector<int>*     TPC3_npoints;/** */
      std::vector<int>*     TPC3_channel;/** */
      std::vector<int>*     TPC3_sumTOT;/** */
      std::vector<float>*   TPC3_sumE;/** */
      std::vector<unsigned short>* TPC3_hitside;/** */
      std::vector<double>*  TPC3_phi;/** */
      std::vector<double>*  TPC3_theta;/** */
      std::vector<float>*  TPC3_chi2;/** */
      std::vector<double>*  TPC3_dEdx;/** */
      std::vector<int>*     TPC3_PID_neutrons;/** */
      std::vector<int>*     TPC3_PID_alphas_top;/** */
      std::vector<int>*     TPC3_PID_alphas_bottom;/** */
      std::vector<int>*     TPC3_PID_xrays;/** */
      std::vector<int>*     TPC3_PID_protons;/** */
      std::vector<int>*     TPC3_PID_others;/** */
      std::vector<unsigned int>* TPC3_N_neutrons;/** */
      std::vector<unsigned int>* TPC3_N_alphas_top;/** */
      std::vector<unsigned int>* TPC3_N_alphas_bottom;/** */
      std::vector<unsigned int>* TPC3_N_xrays;/** */
      std::vector<unsigned int>* TPC3_N_protons;/** */
      std::vector<unsigned int>* TPC3_N_others;/** */
      std::vector<unsigned int>* TPC3_selectedTrackIdx;/** */
      std::vector<int>*     TPC3_hits_col;/** */
      std::vector<int>*     TPC3_hits_row;/** */
      std::vector<int>*     TPC3_hits_bcid;/** */
      std::vector<int>*     TPC3_hits_tot;/** */
      std::vector<int>*     TPC4_idx;/** */
      std::vector<int>*     TPC4_npoints;/** */
      std::vector<int>*     TPC4_channel;/** */
      std::vector<int>*     TPC4_sumTOT;/** */
      std::vector<float>*   TPC4_sumE;/** */
      std::vector<unsigned short>* TPC4_hitside;/** */
      std::vector<double>* TPC4_phi;/** */
      std::vector<double>* TPC4_theta;/** */
      std::vector<float>* TPC4_chi2;/** */
      std::vector<double>* TPC4_dEdx;/** */
      std::vector<int>* TPC4_PID_neutrons;/** */
      std::vector<int>* TPC4_PID_alphas_top;/** */
      std::vector<int>* TPC4_PID_alphas_bottom;/** */
      std::vector<int>* TPC4_PID_xrays;/** */
      std::vector<int>* TPC4_PID_protons;/** */
      std::vector<int>* TPC4_PID_others;/** */
      std::vector<unsigned int>* TPC4_N_neutrons;/** */
      std::vector<unsigned int>* TPC4_N_alphas_top;/** */
      std::vector<unsigned int>* TPC4_N_alphas_bottom;/** */
      std::vector<unsigned int>* TPC4_N_xrays;/** */
      std::vector<unsigned int>* TPC4_N_protons;/** */
      std::vector<unsigned int>* TPC4_N_others;/** */
      std::vector<int>* TPC4_selectedTrackIdx;/** */
      std::vector<int>* TPC4_hits_col;/** */
      std::vector<int>* TPC4_hits_row;/** */
      std::vector<int>* TPC4_hits_bcid;/** */
      std::vector<int>* TPC4_hits_tot;/** */
      std::vector<float>* BGO_energy;/** */
      std::vector<float>* CSI_hitRate;/** */
      std::vector<float>* CSI_sumE;/** */
      std::vector<float>* HE3_rate;/** */
      std::vector<float>* DIA_dose; /** */
      std::vector<float> BGO_energy_av;/** */
      std::vector<float> CSI_hitRate_av;/** */
      std::vector<float> CSI_sumE_av;/** */
      std::vector<float> HE3_rate_av;/** */
      std::vector<float> DIA_dose_av; /** */
      std::vector<float> BGO_energy_mc;/** */
      std::vector<float> CSI_hitRate_mc;/** */
      std::vector<float> CSI_sumE_mc;/** */
      std::vector<float> HE3_rate_mc;/** */
      std::vector<float> DIA_dose_mc; /** */
      std::vector<float> BGO_energy_mc_av;/** */
      std::vector<float> CSI_hitRate_mc_av;/** */
      std::vector<float> CSI_sumE_mc_av;/** */
      std::vector<float> HE3_rate_mc_av;/** */
      std::vector<float> DIA_dose_mc_av; /** */


      /**
       *.clear the structure: set elements to zero
       */
      void clear()
      {
        ts = 0;
        event = 0;
        run = 0;
        subrun = 0;
        SKB_HER_injectionFlag = 0;
        SKB_LER_injectionFlag = 0;
        SKB_Status = 0;
        SKB_HER_injectionRate = 0;
        SKB_LER_injectionRate = 0;
        SKB_HER_lifetime = 0;
        SKB_LER_lifetime = 0;
        SKB_LER_current = 0;
        SKB_HER_current = 0;
        SKB_LER_injectionEfficiency = 0;
        SKB_HER_injectionEfficiency = 0;
        SKB_beamLoss_ionChambers_mean = 0;
        SKB_beamLoss_PINdiodes_mean = 0;
        SKB_beamLoss_nearCollimators = 0;
        SKB_beamLoss_aroundMasks = 0;
        SKB_LER_injectionCharge = 0;
        SKB_HER_injectionCharge = 0;
        SKB_LER_injectionRepetitionRate = 0;
        SKB_HER_injectionRepetitionRate = 0;
        SKB_LER_injectionNumberOfBunches = 0;
        SKB_HER_injectionNumberOfBunches = 0;
        SKB_LER_pressures = 0;
        SKB_HER_pressures = 0;
        SKB_LER_pressure_average = 0;
        SKB_HER_pressure_average = 0;
        SKB_HER_collimatorPositions_mm = 0;
        SKB_HER_collimatorPositions_DMM = 0;
        SKB_HER_collimatorPositions_inX = 0;
        SKB_HER_collimatorPositions_inY = 0;
        SKB_HER_collimatorPositions_fromBeam = 0;
        SKB_LER_collimatorPositions_DMM = 0;
        SKB_LER_collimatorPositions_mm = 0;
        SKB_LER_collimatorPositions_X = 0;
        SKB_LER_collimatorPositions_Y = 0;
        SKB_LER_collimatorPositions_fromBeam = 0;
        SKB_HER_beamSize_xray_X = 0;
        SKB_HER_beamSize_xray_Y = 0;
        SKB_LER_beamSize_xray_X = 0;
        SKB_LER_beamSize_xray_Y = 0;
        SKB_LER_beamSize_SR_X = 0;
        SKB_LER_beamSize_SR_Y = 0;
        SKB_HER_beamSize_SR_X = 0;
        SKB_HER_beamSize_SR_Y = 0;
        SKB_HER_integratedCurrent = 0;
        SKB_LER_integratedCurrent = 0;
        SKB_LER_partialPressures_D06 = 0;
        SKB_LER_partialPressures_D02 = 0;
        SKB_LER_pressures_local = 0;

        PIN_dose = 0;
        BGO_energy = 0;
        HE3_rate = 0;
        CSI_sumE = 0;
        CSI_hitRate = 0;
        DIA_dose = 0;

        PIN_dose_mc.clear();
        BGO_energy_mc.clear();
        HE3_rate_mc.clear();
        CSI_sumE_mc.clear();
        CSI_hitRate_mc.clear();
        DIA_dose_mc.clear();

        PIN_dose_mc_av.clear();
        BGO_energy_mc_av.clear();
        HE3_rate_mc_av.clear();
        CSI_sumE_mc_av.clear();
        CSI_hitRate_mc_av.clear();
        DIA_dose_mc_av.clear();

      }

    };

  } // BEAST_v3 namespace
} // Belle2 namespace

#endif

