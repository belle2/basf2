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
      double ts;
      std::vector<Float_t> SKB_HER_injectionFlag;
      std::vector<Float_t> SKB_LER_injectionFlag;
      std::vector<Float_t> SKB_status;
      std::vector<Float_t> SKB_HER_injectionRate;
      std::vector<Float_t> SKB_LER_injectionRate;
      std::vector<Float_t> SKB_HER_beamLife;
      std::vector<Float_t> SKB_LER_beamLife;
      std::vector<Float_t> SKB_LER_current;
      std::vector<Float_t> SKB_HER_current;
      std::vector<Float_t> SKB_LER_injectionEfficiency;
      std::vector<Float_t> SKB_HER_injectionEfficiency;
      std::vector<Float_t> SKB_IP_beamLoss_IonChambers;
      std::vector<Float_t> SKB_IP_beamLoss_PINdiodes;
      std::vector<Float_t> SKB_LER_beamLoss_NearCollimators;
      std::vector<Float_t> SKB_HER_beamLoss_NearCollimators;
      std::vector<Float_t> SKB_LER_injectionCharge;
      std::vector<Float_t> SKB_HER_injectionCharge;
      std::vector<Float_t> SKB_HER_beamLife_2;
      std::vector<Float_t> SKB_LER_beamLife_2;
      std::vector<Float_t> SKB_LER_injection_RepetitionRate;
      std::vector<Float_t> SKB_HER_injection_RepetitionRate;
      std::vector<Float_t> SKB_injection_BucketSelection;
      std::vector<Float_t> SKB_injection_AutoFill_Mode;
      std::vector<Float_t> SKB_injection_AutoFill_Nbunches;
      std::vector<Float_t> SKB_LER_injection_OneBunch_BucketNumber;
      std::vector<Float_t> SKB_HER_injection_OneBunch_BucketNumber;
      std::vector<Float_t> SKB_injection_OneBunch_Nbunches;
      std::vector<Float_t> SKB_LER_abort;
      std::vector<Float_t> SKB_HER_abort;
      std::vector<Float_t> SKB_LER_pressure;
      std::vector<Float_t> SKB_HER_pressure;
      std::vector<Float_t> SKB_LER_averagePressure;
      std::vector<Float_t> SKB_HER_averagePressure;
      std::vector<Float_t> SKB_HER_collimatorAbsolutePosition;
      std::vector<Float_t> SKB_HER_collimatorBeamPositionX;
      std::vector<Float_t> SKB_HER_collimatorBeamPositionY;
      std::vector<Float_t> SKB_HER_collimatorDistanceToBeam;
      std::vector<Float_t> SKB_LER_collimatorBeamPositionX;
      std::vector<Float_t> SKB_LER_collimatorBeamPositionY;
      std::vector<Float_t> SKB_LER_collimatorDistanceToBeam;
      std::vector<Float_t> SKB_HER_BeamSize_xray_X;
      std::vector<Float_t> SKB_HER_BeamSize_xray_Y;
      std::vector<Float_t> SKB_LER_BeamSize_xray_X;
      std::vector<Float_t> SKB_LER_BeamSize_xray_Y;
      std::vector<Float_t> SKB_LER_BeamSize_SR_X;
      std::vector<Float_t> SKB_LER_BeamSize_SR_Y;
      std::vector<Float_t> SKB_HER_BeamSize_SR_X;
      std::vector<Float_t> SKB_HER_BeamSize_SR_Y;
      std::vector<Float_t> SKB_HER_integratedCurrent;
      std::vector<Float_t> SKB_LER_integratedCurrent;

      std::vector<Float_t> PIN_dose;
      std::vector<Float_t> BGO_energy;
      std::vector<Float_t> HE3_rate;
      std::vector<Float_t> CSI_sumE;
      std::vector<Float_t> CSI_hitRate;
      std::vector<Float_t> DIA_dose;
      std::vector<UInt_t> TPC_neutrons_N;
      std::vector<Float_t> TPC_neutrons_tracks_E;
      std::vector<Float_t> TPC_neutrons_phi;
      std::vector<Float_t> TPC_neutrons_theta;
      std::vector<Float_t> TPC_neutrons_length;
      std::vector<UInt_t> TPC_alphas_top_N;
      std::vector<UInt_t> TPC_alphas_bot_N;
      std::vector<Float_t> TPC_alphas_top_tracks_dEdx;
      std::vector<Float_t> TPC_alphas_bot_tracks_dEdx;
      std::vector<UInt_t> TPC_xrays_N;
      std::vector<Float_t> TPC_xrays_sumE;

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
        SKB_IP_beamLoss_IonChambers.clear();
        SKB_IP_beamLoss_PINdiodes.clear();
        SKB_LER_beamLoss_NearCollimators.clear();
        SKB_HER_beamLoss_NearCollimators.clear();
        SKB_LER_injectionCharge.clear();
        SKB_HER_injectionCharge.clear();
        SKB_HER_beamLife_2.clear();
        SKB_LER_beamLife_2.clear();
        SKB_LER_injection_RepetitionRate.clear();
        SKB_HER_injection_RepetitionRate.clear();
        SKB_injection_BucketSelection.clear();
        SKB_injection_AutoFill_Mode.clear();
        SKB_injection_AutoFill_Nbunches.clear();
        SKB_LER_injection_OneBunch_BucketNumber.clear();
        SKB_HER_injection_OneBunch_BucketNumber.clear();
        SKB_injection_OneBunch_Nbunches.clear();
        SKB_LER_abort.clear();
        SKB_HER_abort.clear();
        SKB_LER_pressure.clear();
        SKB_HER_pressure.clear();
        SKB_LER_averagePressure.clear();
        SKB_HER_averagePressure.clear();
        SKB_HER_collimatorAbsolutePosition.clear();
        SKB_HER_collimatorBeamPositionX.clear();
        SKB_HER_collimatorBeamPositionY.clear();
        SKB_HER_collimatorDistanceToBeam.clear();
        SKB_LER_collimatorBeamPositionX.clear();
        SKB_LER_collimatorBeamPositionY.clear();
        SKB_LER_collimatorDistanceToBeam.clear();
        SKB_HER_BeamSize_xray_X.clear();
        SKB_HER_BeamSize_xray_Y.clear();
        SKB_LER_BeamSize_xray_X.clear();
        SKB_LER_BeamSize_xray_Y.clear();
        SKB_LER_BeamSize_SR_X.clear();
        SKB_LER_BeamSize_SR_Y.clear();
        SKB_HER_BeamSize_SR_X.clear();
        SKB_HER_BeamSize_SR_Y.clear();
        SKB_HER_integratedCurrent.clear();
        SKB_LER_integratedCurrent.clear();

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

