/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef BEASTTREE_V5_H
#define BEASTTREE_V5_H

#include <RtypesCore.h>

#include <vector>

namespace Belle2 {
  namespace BEAST_v5 {

    /**
     * Structure for the input of the data BEAST (TTree "Data_BEAST")
     *
     */
    struct BEASTTree_v5 {

      /**Beast PVs **/
      UInt_t ts; /** unix time [s] */
      UInt_t event; /** eventID */
      UInt_t run; /** runID */
      UInt_t subrun; /** subrunID */
      std::vector<double>*  SKB_HER_current;/** HER current [mA] */
      std::vector<double>*  SKB_LER_current;/** LER current [mA] */
      std::vector<double>*  SKB_HER_lifetime;/** HER beam lifetime [min] */
      std::vector<double>*  SKB_LER_lifetime;/** HER beam lifetime [min] */
      std::vector<float>* SKB_LER_Zeff_D02;/** LER Zeff at D02 section */
      std::vector<float>* SKB_LER_Zeff_D06;/** LER Zeff at D06 section */
      std::vector<double>  SKB_HER_RLR;/** HER Ring Loss Rate */
      std::vector<double>  SKB_LER_RLR;/** LER Ring Loss Rate */
      std::vector<double>*  SKB_HER_integratedCurrent;/** Beast PV */
      std::vector<double>*  SKB_LER_integratedCurrent;/** Beast PV */
      std::vector<double>*  SKB_HER_injectionFlag;/** Beast PV */
      std::vector<double>*  SKB_LER_injectionFlag;/** Beast PV */
      std::vector<double>*  SKB_HER_injectionFlag_safe;/** Beast PV */
      std::vector<double>*  SKB_LER_injectionFlag_safe;/** Beast PV */
      std::vector<double>*  SKB_HER_abortFlag;/** Beast PV */
      std::vector<double>*  SKB_LER_abortFlag;/** Beast PV */
      std::vector<double>*  SKB_HER_abortFlag_safe;/** Beast PV */
      std::vector<double>*  SKB_LER_abortFlag_safe;/** Beast PV */
      std::vector<double>*  SKB_HER_injectionRate;/** HER injection rate [mA/s] */
      std::vector<double>*  SKB_LER_injectionRate;/** LER injection rate [mA/s] */
      std::vector<double>*  SKB_HER_injectionEfficiency;/** HER injection efficiency [%] */
      std::vector<double>*  SKB_LER_injectionEfficiency;/** LER injection efficiency [%] */
      std::vector<double>*  SKB_HER_injectionCharge;/** HER injection charge */
      std::vector<double>*  SKB_LER_injectionCharge;/** LER injection charge */
      std::vector<double>*  SKB_HER_injectionRepetitionRate;/** HER injection repetition rate [Hz] */
      std::vector<double>*  SKB_LER_injectionRepetitionRate;/** LER injection repetition rate [Hz] */
      std::vector<double>*  SKB_HER_injectionNumberOfBunches;/** HER injection number of bunches */
      std::vector<double>*  SKB_LER_injectionNumberOfBunches;/** LER injection number of bunches */
      std::vector<double>*  SKB_HER_beamSize_xray_X;/** HER horizontal beam size by X-ray monitor [um] */
      std::vector<double>*  SKB_HER_beamSize_xray_Y;/** HER vertical beam size by X-ray monitor [um] */
      std::vector<double>*  SKB_HER_correctedBeamSize_xray_Y;/** HER corrected vertical beam size by X-ray monitor [um] */
      std::vector<double>*  SKB_LER_beamSize_xray_X;/** LER horizontal beam size by X-ray monitor [um] */
      std::vector<double>*  SKB_LER_beamSize_xray_Y;/** LER vertical beam size by X-ray monitor [um] */
      std::vector<double>*  SKB_LER_correctedBeamSize_xray_Y;/** LER corrected vertical beam size by X-ray monitor [um] */
      std::vector<double>*  SKB_HER_beamSize_SR_Y;/** HER vertical beam size by SR monitor [um] */
      std::vector<double>*  SKB_LER_beamSize_SR_Y;/** LER vertical beam size by SR monitor [um] */
      std::vector<double>*  SKB_HER_beamSize_SR_X;/** HER horizontal beam size by SR monitor [um] */
      std::vector<double>*  SKB_LER_beamSize_SR_X;/** LER horizontal beam size by SR monitor [um] */
      std::vector<double>*  SKB_beamLoss_nearCollimators;/** Collimator BLM */
      std::vector<double>*  SKB_beamLoss_aroundMasks;/** Mask BLM */
      std::vector<double>*  SKB_HER_pressure_average;/** HER ring average pressure [Pa] */
      std::vector<double>*  SKB_LER_pressure_average;/** LER ring average pressure [Pa] */
      std::vector<float>*   SKB_HER_pressures;/** Beast PV */
      std::vector<float>*   SKB_LER_pressures;/** Beast PV */
      std::vector<float>*   SKB_LER_pressures_local;/** Beast PV */
      std::vector<float>*   SKB_HER_pressures_local;/** Beast PV */
      std::vector<double>*  SKB_HER_pressure_average_corrected;/** HER corrected ring average pressure [Pa] */
      std::vector<double>*  SKB_LER_pressure_average_corrected;/** LER corrected ring average pressure [Pa] */
      std::vector<float>*   SKB_HER_pressures_corrected;/** Beast PV */
      std::vector<float>*   SKB_LER_pressures_corrected;/** Beast PV */
      std::vector<float>*   SKB_LER_pressures_local_corrected;/** Beast PV */
      std::vector<float>*   SKB_HER_pressures_local_corrected;/** Beast PV */
      std::vector<float>*   SKB_HER_collimatorPositions_inX;/** HER collimator head horizontal position [mm] */
      std::vector<float>*   SKB_HER_collimatorPositions_inY;/** HER collimator head vertical position [mm] */
      std::vector<float>*   SKB_HER_collimatorPositions_mm;/** HER collimator position [mm] */
      std::vector<float>*   SKB_LER_collimatorPositions_mm;/** LER collimator position [mm] */
      std::vector<float>*   SKB_LER_partialPressures_D06;/** LER partial RGA pressure at D06 section [Pa] */
      std::vector<float>*   SKB_LER_partialPressures_D02;/** LER partial RGA pressure at D02 section [Pa] */
      std::vector<float>*   DIA_current;/** Beast PV */
      std::vector<float>*   CSI_temperature;/** Beast PV */
      std::vector<float>*   CSI_humidity;/** Beast PV */
      std::vector<float>*   TPC_gas_flow;/** TPC gas flow */
      std::vector<float>*   TPC_gas_pressure;/** TPC gas pressure */
      std::vector<double>*  SKB_Status;/** Beast PV */
      std::vector<short>*   SKB_beamLoss_ionChambers_mean;/** BLM mean ion chamber*/
      std::vector<short>*   SKB_beamLoss_PINdiodes_mean;/** BLM mean pin-diod */
      std::vector<float>*   SKB_HER_collimatorPositions_DMM;/** Beast PV */
      std::vector<float>*   SKB_HER_collimatorPositions_fromBeam;/** HER collimator head position w.r.t. beam [mm] */
      std::vector<float>*   SKB_LER_collimatorPositions_DMM;/** Beast PV */
      std::vector<float>*   SKB_LER_collimatorPositions_X;/** Beast PV */
      std::vector<float>*   SKB_LER_collimatorPositions_Y;/** Beast PV */
      std::vector<float>*   SKB_LER_collimatorPositions_fromBeam;/** LER collimator head position w.r.t. beam [mm] */
      std::vector<float>*   PIN_voltage;/** Pin-diod voltage */
      std::vector<float>*   PIN_temperature;/** Pin-diod temperature */
      std::vector<float> PIN_dose;/** Pin-diod dose */
      std::vector<float> PIN_dose_av;/** Pin-diod average dose */
      std::vector<int>*     TPC3_idx;/** TPC id */
      std::vector<int>*     TPC3_npoints;/** TPC number of points */
      std::vector<int>*     TPC3_channel;/** TPC channel */
      std::vector<int>*     TPC3_sumTOT;/** TPC Time-over-Threshold */
      std::vector<float>*   TPC3_sumE;/** TPC energy deposition */
      std::vector<unsigned short>* TPC3_hitside;/** TPC side of hits */
      std::vector<double>*  TPC3_phi;/** TPC phi ange */
      std::vector<double>*  TPC3_theta;/** TPC theta angle */
      std::vector<float>*  TPC3_chi2;/** TPC chi2 */
      std::vector<double>*  TPC3_dEdx;/** PC dE/dx */
      std::vector<int>*     TPC3_PID_neutrons;/** Beast PV */
      std::vector<int>*     TPC3_PID_alphas_top;/** Beast PV */
      std::vector<int>*     TPC3_PID_alphas_bottom;/** Beast PV */
      std::vector<int>*     TPC3_PID_xrays;/** Beast PV */
      std::vector<int>*     TPC3_PID_protons;/** Beast PV */
      std::vector<int>*     TPC3_PID_others;/** Beast PV */
      std::vector<unsigned int>* TPC3_N_neutrons;/** Beast PV */
      std::vector<unsigned int>* TPC3_N_alphas_top;/** Beast PV */
      std::vector<unsigned int>* TPC3_N_alphas_bottom;/** Beast PV */
      std::vector<unsigned int>* TPC3_N_xrays;/** Beast PV */
      std::vector<unsigned int>* TPC3_N_protons;/** Beast PV */
      std::vector<unsigned int>* TPC3_N_others;/** Beast PV */
      std::vector<unsigned int>* TPC3_selectedTrackIdx;/** Beast PV */
      std::vector<int>*     TPC3_hits_col;/** Beast PV */
      std::vector<int>*     TPC3_hits_row;/** Beast PV */
      std::vector<int>*     TPC3_hits_bcid;/** Beast PV */
      std::vector<int>*     TPC3_hits_tot;/** Beast PV */
      std::vector<int>*     TPC4_idx;/** Beast PV */
      std::vector<int>*     TPC4_npoints;/** Beast PV */
      std::vector<int>*     TPC4_channel;/** Beast PV */
      std::vector<int>*     TPC4_sumTOT;/** Beast PV */
      std::vector<float>*   TPC4_sumE;/** Beast PV */
      std::vector<unsigned short>* TPC4_hitside;/** Beast PV */
      std::vector<double>* TPC4_phi;/** Beast PV */
      std::vector<double>* TPC4_theta;/** Beast PV */
      std::vector<float>* TPC4_chi2;/** Beast PV */
      std::vector<double>* TPC4_dEdx;/** Beast PV */
      std::vector<int>* TPC4_PID_neutrons;/** Beast PV */
      std::vector<int>* TPC4_PID_alphas_top;/** Beast PV */
      std::vector<int>* TPC4_PID_alphas_bottom;/** Beast PV */
      std::vector<int>* TPC4_PID_xrays;/** Beast PV */
      std::vector<int>* TPC4_PID_protons;/** Beast PV */
      std::vector<int>* TPC4_PID_others;/** Beast PV */
      std::vector<unsigned int>* TPC4_N_neutrons;/** Beast PV */
      std::vector<unsigned int>* TPC4_N_alphas_top;/** Beast PV */
      std::vector<unsigned int>* TPC4_N_alphas_bottom;/** Beast PV */
      std::vector<unsigned int>* TPC4_N_xrays;/** Beast PV */
      std::vector<unsigned int>* TPC4_N_protons;/** Beast PV */
      std::vector<unsigned int>* TPC4_N_others;/** Beast PV */
      std::vector<int>* TPC4_selectedTrackIdx;/** Beast PV */
      std::vector<int>* TPC4_hits_col;/** Beast PV */
      std::vector<int>* TPC4_hits_row;/** Beast PV */
      std::vector<int>* TPC4_hits_bcid;/** Beast PV */
      std::vector<int>* TPC4_hits_tot;/** Beast PV */
      std::vector<float> BGO_energy;/** Beast PV */
      std::vector<float> CSI_hitRate;/** Beast PV */
      std::vector<float> CSI_sumE;/** Beast PV */
      std::vector<float>* CSI_data_sumE;/** Beast PV */
      std::vector<float> CSI_Ebin;/** Beast PV */
      std::vector<float> HE3_rate;/** Beast PV */
      std::vector<float> CLAWS_rate;/** Beast PV */
      std::vector<float> QCSS_rate;/** Beast PV */
      std::vector<float> DIA_dose; /** Beast PV */
      std::vector<float>* DIA_data_dose; /** Beast PV */
      std::vector<float>* PIN_data_dose; /** Beast PV */
      std::vector<float>* BGO_data_dose; /** Beast PV */
      std::vector<float> BGO_energy_av;/** Beast PV */
      std::vector<float> CSI_hitRate_av;/** Beast PV */
      std::vector<float> CSI_sumE_av;/** Beast PV */
      std::vector<float> CSI_Ebin_av;/** Beast PV */
      std::vector<float> HE3_rate_av;/** Beast PV */
      std::vector<float> CLAWS_rate_av;/** Beast PV */
      std::vector<float> QCSS_rate_av;/** Beast PV */
      std::vector<float> DIA_dose_av; /** Beast PV */

      std::vector<float>* CSI_data_rate; /** Beast PV */
      std::vector<float>* HE3_data_rate; /** Beast PV */
      std::vector<float>* QCSS_data_rate; /** Beast PV */
      std::vector<float>* CLAWS_data_rate; /** Beast PV */
      std::vector<float>* CSI_data_Ebin; /** Beast PV */

      std::vector<double> SAD_HER_lifetime;/** Beast PV */
      std::vector<double> SAD_LER_lifetime;/** Beast PV */
      std::vector<double> SAD_HER_lifetime_av;/** Beast PV */
      std::vector<double> SAD_LER_lifetime_av;/** Beast PV */
      std::vector<double> SAD_HER_RLR;/** SAD HER Ring Loss Rate */
      std::vector<double> SAD_LER_RLR;/** SAD LER Ring Loss Rate */
      std::vector<double> SAD_HER_RLR_av;/** SAD HER average Ring Loss Rate */
      std::vector<double> SAD_LER_RLR_av;/** SAD LER average Ring Loss Rate */

      std::vector<float> DOSI;/** Beast PV */
      std::vector<float> DOSI_av;/** Beast PV */

      std::vector<double> mc_reweight_LERB;/** LER Bremsstrahlung scale coef. */
      std::vector<double> mc_reweight_LERC;/** LER Coulomb scale coef. */
      std::vector<double> mc_reweight_LERT;/** LER Touschek scale coef. */
      std::vector<double> mc_reweight_HERB;/** HER Bremsstrahlung scale coef. */
      std::vector<double> mc_reweight_HERC;/** HER Coulomb scale coef. */
      std::vector<double> mc_reweight_HERT;/** HER Touschek scale coef. */

      //std::vector<float> TPC_rate_av[2][5];/** */
      //std::vector<float> TPC_rate[2][5];/** */
      float TPC_rate_av[2][5];/** TPC average rate */
      float TPC_rate[2][5];/** TPC rate */
      //float TPC_dose_av[2][5];/** */
      //float TPC_dose[2][5];/** */
      std::vector<float> TPC_dose;/** TPC dose */
      std::vector<float> TPC_dose_av;/** TPC average dose */

      float TPC_angular_rate_av[2][9][18];/** TPC angular average rate */
      float TPC_angular_rate[2][9][18];/** TPC angular rate */
      float TPC_angular_dose_av[2][9][18];/** TPC angular average dose */
      float TPC_angular_dose[2][9][18];/** TPC angular dose */

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

