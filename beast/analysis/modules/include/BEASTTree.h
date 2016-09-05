/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BEASTTREE_H
#define BEASTTREE_H

#include <TTree.h>

namespace Belle2 {
  namespace BEAST {

    /**
     * Structure for the input of the data BEAST (TTree "Data_BEAST")
     * https://belle2.cc.kek.jp/~twiki/bin/view/Detector/BEAST/ROOTDataFormats
     */
    struct BEASTTree {

      /**Beast PVs **/
      double ts;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_injectionFlag;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_injectionFlag;/**Beast PVs **/
      std::vector<Double_t> SKB_status;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_injectionRate;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_injectionRate;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_beamLife;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_beamLife;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_current;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_current;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_injectionEfficiency;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_injectionEfficiency;/**Beast PVs **/
      std::vector<Double_t> SKB_beamLoss_ionChambers_mean;/**Beast PVs **/
      std::vector<Double_t> SKB_beamLoss_PINdiodes_mean;/**Beast PVs **/
      std::vector<Double_t> SKB_beamLoss_nearCollimators;/**Beast PVs **/
      std::vector<Double_t> SKB_beamLoss_aroundMasks;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_injectionCharge;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_injectionCharge;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_beamLife_2;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_beamLife_2;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_injectionRepetitionRate;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_injectionRepetitionRate;/**Beast PVs **/
      std::vector<Double_t> SKB_injection_BucketSelection;/**Beast PVs **/
      std::vector<Double_t> SKB_injection_AutoFill_Mode;/**Beast PVs **/
      std::vector<Double_t> SKB_injection_AutoFill_Nbunches;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_injectionNumberOfBunches;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_injectionNumberOfBunches;/**Beast PVs **/
      std::vector<Double_t> SKB_injection_OneBunch_Nbunches;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_abort;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_abort;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_pressures;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_pressures;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_pressure_average;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_pressure_average;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_collimatorPositions_DMM;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_collimatorPositions_mm;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_collimatorPositions_X;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_collimatorPositions_Y;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_collimatorPosition_fromBeam;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_collimatorPositions_DMM;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_collimatorPositions_mm;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_collimatorPositions_X;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_collimatorPositions_Y;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_collimatorPosition_fromBeam;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_beamSize_xray_X;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_beamSize_xray_Y;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_beamSize_xray_X;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_beamSize_xray_Y;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_beamSize_SR_X;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_beamSize_SR_Y;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_beamSize_SR_X;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_beamSize_SR_Y;/**Beast PVs **/
      std::vector<Double_t> SKB_HER_integratedCurrent;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_integratedCurrent;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_partialPressures_D06;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_partialPressures_D02;/**Beast PVs **/
      std::vector<Double_t> SKB_LER_pressures_local;/**Beast PVs **/


      std::vector<Double_t> PIN_dose;/**Beast PVs **/
      std::vector<Double_t> BGO_energy;/**Beast PVs **/
      std::vector<Double_t> HE3_rate;/**Beast PVs **/
      std::vector<Double_t> CSI_sumE;/**Beast PVs **/
      std::vector<Double_t> CSI_hitRate;/**Beast PVs **/
      std::vector<Double_t> DIA_dose;/**Beast PVs **/
      std::vector<UInt_t> TPC_neutrons_N;/**Beast PVs **/
      std::vector<Double_t> TPC_neutrons_tracks_E;/**Beast PVs **/
      std::vector<Double_t> TPC_neutrons_phi;/**Beast PVs **/
      std::vector<Double_t> TPC_neutrons_theta;/**Beast PVs **/
      std::vector<Double_t> TPC_neutrons_length;/**Beast PVs **/
      std::vector<UInt_t> TPC_alphas_top_N;/**Beast PVs **/
      std::vector<UInt_t> TPC_alphas_bot_N;/**Beast PVs **/
      std::vector<Double_t> TPC_alphas_top_tracks_dEdx;/**Beast PVs **/
      std::vector<Double_t> TPC_alphas_bot_tracks_dEdx;/**Beast PVs **/
      std::vector<UInt_t> TPC_xrays_N;/**Beast PVs **/
      std::vector<Double_t> TPC_xrays_sumE;/**Beast PVs **/

      /**
       * Clear the structure: set elements to zero
       */
      void clear()
      {
        ts = 0;
        SKB_HER_injectionFlag.clear();
        SKB_LER_injectionFlag.clear();
        SKB_status.clear();
        SKB_HER_injectionRate.clear();
        SKB_LER_injectionRate.clear();
        SKB_HER_beamLife.clear();
        SKB_LER_beamLife.clear();
        SKB_LER_current.clear();
        SKB_HER_current.clear();
        SKB_LER_injectionEfficiency.clear();
        SKB_HER_injectionEfficiency.clear();
        SKB_beamLoss_ionChambers_mean.clear();
        SKB_beamLoss_PINdiodes_mean.clear();
        SKB_beamLoss_nearCollimators.clear();
        SKB_beamLoss_aroundMasks.clear();
        SKB_LER_injectionCharge.clear();
        SKB_HER_injectionCharge.clear();
        SKB_HER_beamLife_2.clear();
        SKB_LER_beamLife_2.clear();
        SKB_LER_injectionRepetitionRate.clear();
        SKB_HER_injectionRepetitionRate.clear();
        SKB_injection_BucketSelection.clear();
        SKB_injection_AutoFill_Mode.clear();
        SKB_injection_AutoFill_Nbunches.clear();
        SKB_LER_injectionNumberOfBunches.clear();
        SKB_HER_injectionNumberOfBunches.clear();
        SKB_injection_OneBunch_Nbunches.clear();
        SKB_LER_abort.clear();
        SKB_HER_abort.clear();
        SKB_LER_pressures.clear();
        SKB_HER_pressures.clear();
        SKB_LER_pressure_average.clear();
        SKB_HER_pressure_average.clear();
        SKB_HER_collimatorPositions_DMM.clear();
        SKB_HER_collimatorPositions_mm.clear();
        SKB_HER_collimatorPositions_X.clear();
        SKB_HER_collimatorPositions_Y.clear();
        SKB_HER_collimatorPosition_fromBeam.clear();
        SKB_LER_collimatorPositions_DMM.clear();
        SKB_LER_collimatorPositions_mm.clear();
        SKB_LER_collimatorPositions_X.clear();
        SKB_LER_collimatorPositions_Y.clear();
        SKB_LER_collimatorPosition_fromBeam.clear();
        SKB_HER_beamSize_xray_X.clear();
        SKB_HER_beamSize_xray_Y.clear();
        SKB_LER_beamSize_xray_X.clear();
        SKB_LER_beamSize_xray_Y.clear();
        SKB_LER_beamSize_SR_X.clear();
        SKB_LER_beamSize_SR_Y.clear();
        SKB_HER_beamSize_SR_X.clear();
        SKB_HER_beamSize_SR_Y.clear();
        SKB_HER_integratedCurrent.clear();
        SKB_LER_integratedCurrent.clear();
        SKB_LER_partialPressures_D06.clear();
        SKB_LER_partialPressures_D02.clear();
        SKB_LER_pressures_local.clear();

        PIN_dose.clear();
        BGO_energy.clear();
        HE3_rate.clear();
        CSI_sumE.clear();
        CSI_hitRate.clear();
        DIA_dose.clear();
        TPC_neutrons_N.clear();
        TPC_neutrons_tracks_E.clear();
        TPC_neutrons_phi.clear();
        TPC_neutrons_theta.clear();
        TPC_neutrons_length.clear();
        TPC_alphas_top_N.clear();
        TPC_alphas_bot_N.clear();
        TPC_alphas_top_tracks_dEdx.clear();
        TPC_alphas_bot_tracks_dEdx.clear();
        TPC_xrays_N.clear();
        TPC_xrays_sumE.clear();
      }

    };

  } // BEAST namespace
} // Belle2 namespace

#endif

