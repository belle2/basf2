/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BEASTTREE_V5_H
#define BEASTTREE_V5_H

#include <TTree.h>

namespace Belle2 {
  namespace BEAST_v5 {

    /**
     * Structure for the input of the data BEAST (TTree "Data_BEAST")
     *
     */
    struct BEASTTree_v5 {

      /**Beast PVs **/
      UInt_t ts; /** */
      UInt_t event; /** */
      UInt_t run; /** */
      UInt_t subrun; /** */
      std::vector<double>*  SKB_HER_current;/** */
      std::vector<double>*  SKB_LER_current;/** */
      std::vector<double>*  SKB_HER_lifetime;/** */
      std::vector<double>*  SKB_LER_lifetime;/** */
      std::vector<float>* SKB_LER_Zeff_D02;/** */
      std::vector<float>* SKB_LER_Zeff_D06;/** */
      std::vector<double>  SKB_HER_RLR;/** */
      std::vector<double>  SKB_LER_RLR;/** */
      std::vector<double>*  SKB_HER_integratedCurrent;/** */
      std::vector<double>*  SKB_LER_integratedCurrent;
      std::vector<double>*  SKB_HER_injectionFlag;/** */
      std::vector<double>*  SKB_LER_injectionFlag;/** */
      std::vector<double>*  SKB_HER_injectionFlag_safe;/** */
      std::vector<double>*  SKB_LER_injectionFlag_safe;/** */
      std::vector<double>*  SKB_HER_abortFlag;/** */
      std::vector<double>*  SKB_LER_abortFlag;/** */
      std::vector<double>*  SKB_HER_abortFlag_safe;/** */
      std::vector<double>*  SKB_LER_abortFlag_safe;/** */
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
      std::vector<double>*  SKB_HER_correctedBeamSize_xray_Y;/** */
      std::vector<double>*  SKB_LER_beamSize_xray_X;/** */
      std::vector<double>*  SKB_LER_beamSize_xray_Y;/** */
      std::vector<double>*  SKB_LER_correctedBeamSize_xray_Y;/** */
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
      std::vector<float>*   SKB_HER_pressures_local;/** */
      std::vector<double>*  SKB_HER_pressure_average_corrected;/** */
      std::vector<double>*  SKB_LER_pressure_average_corrected;/** */
      std::vector<float>*   SKB_HER_pressures_corrected;/** */
      std::vector<float>*   SKB_LER_pressures_corrected;/** */
      std::vector<float>*   SKB_LER_pressures_local_corrected;/** */
      std::vector<float>*   SKB_HER_pressures_local_corrected;/** */
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
      std::vector<float> PIN_dose;/** */
      std::vector<float> PIN_dose_av;/** */
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
      std::vector<float> BGO_energy;/** */
      std::vector<float> CSI_hitRate;/** */
      std::vector<float> CSI_sumE;/** */
      std::vector<float>* CSI_data_sumE;/** */
      std::vector<float> CSI_Ebin;/** */
      std::vector<float> HE3_rate;/** */
      std::vector<float> CLAWS_rate;/** */
      std::vector<float> QCSS_rate;/** */
      std::vector<float> DIA_dose; /** */
      std::vector<float>* DIA_data_dose; /** */
      std::vector<float>* PIN_data_dose; /** */
      std::vector<float>* BGO_data_dose; /** */
      std::vector<float> BGO_energy_av;/** */
      std::vector<float> CSI_hitRate_av;/** */
      std::vector<float> CSI_sumE_av;/** */
      std::vector<float> CSI_Ebin_av;/** */
      std::vector<float> HE3_rate_av;/** */
      std::vector<float> CLAWS_rate_av;/** */
      std::vector<float> QCSS_rate_av;/** */
      std::vector<float> DIA_dose_av; /** */

      std::vector<float>* CSI_data_rate; /** */
      std::vector<float>* HE3_data_rate; /** */
      std::vector<float>* QCSS_data_rate; /** */
      std::vector<float>* CLAWS_data_rate; /** */
      std::vector<float>* CSI_data_Ebin; /** */

      std::vector<double> SAD_HER_lifetime;/** */
      std::vector<double> SAD_LER_lifetime;/** */
      std::vector<double> SAD_HER_lifetime_av;/** */
      std::vector<double> SAD_LER_lifetime_av;/** */
      std::vector<double> SAD_HER_RLR;/** */
      std::vector<double> SAD_LER_RLR;/** */
      std::vector<double> SAD_HER_RLR_av;/** */
      std::vector<double> SAD_LER_RLR_av;/** */

      std::vector<float> DOSI;/** */
      std::vector<float> DOSI_av;/** */

      std::vector<double> mc_reweight_LERB;/** */
      std::vector<double> mc_reweight_LERC;/** */
      std::vector<double> mc_reweight_LERT;/** */
      std::vector<double> mc_reweight_HERB;/** */
      std::vector<double> mc_reweight_HERC;/** */
      std::vector<double> mc_reweight_HERT;/** */

      //std::vector<float> TPC_rate_av[2][5];/** */
      //std::vector<float> TPC_rate[2][5];/** */
      float TPC_rate_av[2][5];/** */
      float TPC_rate[2][5];/** */
      //float TPC_dose_av[2][5];/** */
      //float TPC_dose[2][5];/** */
      std::vector<float> TPC_dose;/** */
      std::vector<float> TPC_dose_av;/** */

      float TPC_angular_rate_av[2][9][18];/** */
      float TPC_angular_rate[2][9][18];/** */
      float TPC_angular_dose_av[2][9][18];/** */
      float TPC_angular_dose[2][9][18];/** */

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
        SKB_HER_injectionFlag_safe = 0;
        SKB_LER_injectionFlag_safe = 0;
        SKB_HER_abortFlag = 0;
        SKB_LER_abortFlag = 0;
        SKB_HER_abortFlag_safe = 0;
        SKB_LER_abortFlag_safe = 0;
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
        SKB_LER_pressures_corrected = 0;
        SKB_HER_pressures_corrected = 0;
        SKB_LER_pressure_average_corrected = 0;
        SKB_HER_pressure_average_corrected = 0;
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
        SKB_HER_correctedBeamSize_xray_Y = 0;
        SKB_LER_beamSize_xray_X = 0;
        SKB_LER_beamSize_xray_Y = 0;
        SKB_LER_correctedBeamSize_xray_Y = 0;
        SKB_LER_beamSize_SR_X = 0;
        SKB_LER_beamSize_SR_Y = 0;
        SKB_HER_beamSize_SR_X = 0;
        SKB_HER_beamSize_SR_Y = 0;
        SKB_HER_integratedCurrent = 0;
        SKB_LER_integratedCurrent = 0;
        SKB_LER_partialPressures_D06 = 0;
        SKB_LER_partialPressures_D02 = 0;
        SKB_LER_pressures_local = 0;
        SKB_HER_pressures_local = 0;
        SKB_LER_pressures_local_corrected = 0;
        SKB_HER_pressures_local_corrected = 0;
        SKB_LER_Zeff_D02 = 0;
        SKB_LER_Zeff_D06 = 0;
        PIN_data_dose = 0;
        DIA_data_dose = 0;
        BGO_data_dose = 0;
        CSI_data_sumE = 0;

        HE3_data_rate = 0;
        CSI_data_rate = 0;
        QCSS_data_rate = 0;
        CLAWS_data_rate = 0;
        CSI_data_Ebin = 0;


        PIN_dose.clear();
        BGO_energy.clear();
        HE3_rate.clear();
        CLAWS_rate.clear();
        QCSS_rate.clear();
        CSI_sumE.clear();
        CSI_Ebin.clear();
        CSI_hitRate.clear();
        DIA_dose.clear();

        PIN_dose_av.clear();
        BGO_energy_av.clear();
        HE3_rate_av.clear();
        TPC_dose_av.clear();
        TPC_dose.clear();
        for (int i = 0; i < 2; i ++) {
          for (int j = 0; j < 5; j ++) {
            //TPC_rate_av[i][j].clear();
            //TPC_rate[i][j].clear();
            TPC_rate_av[i][j] = 0;
            TPC_rate[i][j] = 0;
            //TPC_dose_av[i][j] = 0;
            //TPC_dose[i][j] = 0;
          }
          for (int j = 0; j < 9; j ++) {
            for (int k = 0; k < 18; k ++) {
              TPC_angular_rate[i][j][k] = 0;
              TPC_angular_dose[i][j][k] = 0;
              TPC_angular_rate_av[i][j][k] = 0;
              TPC_angular_dose_av[i][j][k] = 0;
            }
          }
        }
        CLAWS_rate_av.clear();
        QCSS_rate_av.clear();
        CSI_sumE_av.clear();
        CSI_Ebin_av.clear();
        CSI_hitRate_av.clear();
        DIA_dose_av.clear();

        SAD_HER_lifetime.clear();
        SAD_LER_lifetime.clear();
        SKB_HER_RLR.clear();
        SKB_LER_RLR.clear();
        SAD_HER_RLR.clear();
        SAD_LER_RLR.clear();
        SAD_HER_RLR_av.clear();
        SAD_LER_RLR_av.clear();
        SAD_HER_lifetime_av.clear();
        SAD_LER_lifetime_av.clear();

        DOSI.clear();
        DOSI_av.clear();
      }

    };

  } // BEAST_v5 namespace
} // Belle2 namespace

#endif

