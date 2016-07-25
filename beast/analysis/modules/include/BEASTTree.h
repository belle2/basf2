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
      std::vector<Float_t> SKB_LER_pressure;
      std::vector<Float_t> SKB_HER_pressure;
      std::vector<Float_t> SKB_LER_averagePressure;
      std::vector<Float_t> SKB_HER_averagePressure;
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
        SKB_LER_pressure.clear();
        SKB_HER_pressure.clear();
        SKB_LER_averagePressure.clear();
        SKB_HER_averagePressure.clear();
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

