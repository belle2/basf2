/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SUPERKEKBTREE_H
#define SUPERKEKBTREE_H

#include <TTree.h>

namespace Belle2 {
  namespace SuperKEKB {

    /**
     * Structure for the input of the data SUPERKEKB (TTree "Data_SUPERKEKB")
     */
    struct SuperKEKBTree {

      /**SuperKEKB PVs **/
      double ts;/** SuperKEKB PVs */
      double CGLINJ_BEAM_GATE_STATUS;/** SuperKEKB PVs */
      double CGHINJ_BEAM_GATE_STATUS;/** SuperKEKB PVs */
      double CO_KEKB_KEKB_Status_FLG;/** SuperKEKB PVs */
      double CGHINJ_RATE_AVE;/** SuperKEKB PVs */
      double CGLINJ_RATE_AVE;/** SuperKEKB PVs */
      double COpLER_BEAM_LIFE;/** SuperKEKB PVs */
      double COeHER_BEAM_LIFE;/** SuperKEKB PVs */
      double BM_DCCT_HCUR;/** SuperKEKB PVs */
      double BM_DCCT_LCUR;/** SuperKEKB PVs */
      double CGLINJ_EFFICIENCY;/** SuperKEKB PVs */
      double CGHINJ_EFFICIENCY;/** SuperKEKB PVs */
      double BM_BLM_D01_ADC_MEAN;/** SuperKEKB PVs */
      double BM_BLM_TB4_ADC_MEAN;/** SuperKEKB PVs */
      double BM_BLM_D07_ADC_MEAN;/** SuperKEKB PVs */
      double BM_BLM_D10_ADC_MEAN;/** SuperKEKB PVs */
      double CG_BTP_BPM_CHARGE_AVE;/** SuperKEKB PVs */
      double CG_BTE_BPM_CHARGE_AVE;/** SuperKEKB PVs */
      double BMHDCCT_LIFE;/** SuperKEKB PVs */
      double BMLDCCT_LIFE;/** SuperKEKB PVs */
      double LIiEV_BEAM_REP_READ_KBP;/** SuperKEKB PVs */
      double LIiEV_BEAM_REP_READ_KBE;/** SuperKEKB PVs */
      double TM_BKT_config_SNAM;/** SuperKEKB PVs */
      double TM_BKT_AUTO_FILL_Mode;/** SuperKEKB PVs */
      double TM_BKT_AUTO_FILL_Nbunch;/** SuperKEKB PVs */
      double TM_BKT_ONE_BUNCH_LER_InjBucket;/** SuperKEKB PVs */
      double TM_BKT_ONE_BUNCH_HER_InjBucket;/** SuperKEKB PVs */
      double TM_BKT_ONE_BUNCH_Nbunch;/** SuperKEKB PVs */
      double CGLSAFE_MR_ABORT;/** SuperKEKB PVs */
      double CGHSAFE_MR_ABORT;/** SuperKEKB PVs */

      double VALCCG_LER_PRES_AVG;/** SuperKEKB PVs */
      double VALCCG_D01_LER_PRES_AVG;/** SuperKEKB PVs */
      double VALCCG_D02_LER_PRES_AVG;/** SuperKEKB PVs */
      double VALCCG_D03_LER_PRES_AVG;/** SuperKEKB PVs */
      double VALCCG_D04_LER_PRES_AVG;/** SuperKEKB PVs */
      double VALCCG_D05_LER_PRES_AVG;/** SuperKEKB PVs */
      double VALCCG_D06_LER_PRES_AVG;/** SuperKEKB PVs */
      double VALCCG_D07_LER_PRES_AVG;/** SuperKEKB PVs */
      double VALCCG_D08_LER_PRES_AVG;/** SuperKEKB PVs */
      double VALCCG_D09_LER_PRES_AVG;/** SuperKEKB PVs */
      double VALCCG_D10_LER_PRES_AVG;/** SuperKEKB PVs */
      double VALCCG_D11_LER_PRES_AVG;/** SuperKEKB PVs */
      double VALCCG_D12_LER_PRES_AVG;/** SuperKEKB PVs */

      double VAHCCG_HER_PRES_AVG;/** SuperKEKB PVs */
      double VAHCCG_D01_HER_PRES_AVG;/** SuperKEKB PVs */
      double VAHCCG_D02_HER_PRES_AVG;/** SuperKEKB PVs */
      double VAHCCG_D03_HER_PRES_AVG;/** SuperKEKB PVs */
      double VAHCCG_D04_HER_PRES_AVG;/** SuperKEKB PVs */
      double VAHCCG_D05_HER_PRES_AVG;/** SuperKEKB PVs */
      double VAHCCG_D06_HER_PRES_AVG;/** SuperKEKB PVs */
      double VAHCCG_D07_HER_PRES_AVG;/** SuperKEKB PVs */
      double VAHCCG_D08_HER_PRES_AVG;/** SuperKEKB PVs */
      double VAHCCG_D09_HER_PRES_AVG;/** SuperKEKB PVs */
      double VAHCCG_D10_HER_PRES_AVG;/** SuperKEKB PVs */
      double VAHCCG_D11_HER_PRES_AVG;/** SuperKEKB PVs */
      double VAHCCG_D12_HER_PRES_AVG;/** SuperKEKB PVs */

      //SKB HER collimator absolute positions as measured by DMM
      double VAHCLM_D09H1_PM_POS;/** SuperKEKB PVs */
      double VAHCLM_D09H2_PM_POS;/** SuperKEKB PVs */
      double VAHCLM_D09H3_PM_POS;/** SuperKEKB PVs */
      double VAHCLM_D09H4_PM_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V1_PM_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V2_PM_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V3_PM_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V4_PM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H1_PM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H2_PM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H3_PM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H4_PM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V1_PM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V2_PM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V3_PM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V4_PM_POS;/** SuperKEKB PVs */

      //SKB HER beam position in X
      double VAHCLM_D09H1_BMX_POS;/** SuperKEKB PVs */
      double VAHCLM_D09H2_BMX_POS;/** SuperKEKB PVs */
      double VAHCLM_D09H3_BMX_POS;/** SuperKEKB PVs */
      double VAHCLM_D09H4_BMX_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V1_BMX_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V2_BMX_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V3_BMX_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V4_BMX_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H1_BMX_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H2_BMX_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H3_BMX_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H4_BMX_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V1_BMX_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V2_BMX_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V3_BMX_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V4_BMX_POS;/** SuperKEKB PVs */

      //SKB HER beam position in Y
      double VAHCLM_D09H1_BMY_POS;/** SuperKEKB PVs */
      double VAHCLM_D09H2_BMY_POS;/** SuperKEKB PVs */
      double VAHCLM_D09H3_BMY_POS;/** SuperKEKB PVs */
      double VAHCLM_D09H4_BMY_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V1_BMY_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V2_BMY_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V3_BMY_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V4_BMY_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H1_BMY_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H2_BMY_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H3_BMY_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H4_BMY_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V1_BMY_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V2_BMY_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V3_BMY_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V4_BMY_POS;/** SuperKEKB PVs */

      //SKB HER collimator position with respect to beam
      double VAHCLM_D09H1_DIF_POS;/** SuperKEKB PVs */
      double VAHCLM_D09H2_DIF_POS;/** SuperKEKB PVs */
      double VAHCLM_D09H3_DIF_POS;/** SuperKEKB PVs */
      double VAHCLM_D09H4_DIF_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V1_DIF_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V2_DIF_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V3_DIF_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V4_DIF_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H1_DIF_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H2_DIF_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H3_DIF_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H4_DIF_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V1_DIF_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V2_DIF_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V3_DIF_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V4_DIF_POS;/** SuperKEKB PVs */

      //SKB LER collimator absolute positions as measured by DMM
      double VALCLM_D06H4IN_CSS_NOWPOS_DMM;/** SuperKEKB PVs */
      double VALCLM_D06H4OUT_CSS_NOWPOS_DMM;/** SuperKEKB PVs */
      double VALCLM_D06H3IN_CSS_NOWPOS_DMM;/** SuperKEKB PVs */
      double VALCLM_D06H3OUT_CSS_NOWPOS_DMM;/** SuperKEKB PVs */

      //SKB LER beam position in X
      double VALCLM_D06H4IN_CSS_NOWPOS_BMX_POS;/** SuperKEKB PVs */
      double VALCLM_D06H4OUT_CSS_NOWPOS_BMX_POS;/** SuperKEKB PVs */
      double VALCLM_D06H3IN_CSS_NOWPOS_BMX_POS;/** SuperKEKB PVs */
      double VALCLM_D06H3OUT_CSS_NOWPOS_BMX_POS;/** SuperKEKB PVs */

      //SKB LER beam position in Y
      double VALCLM_D06H4IN_CSS_NOWPOS_BMY_POS;/** SuperKEKB PVs */
      double VALCLM_D06H4OUT_CSS_NOWPOS_BMY_POS;/** SuperKEKB PVs */
      double VALCLM_D06H3IN_CSS_NOWPOS_BMY_POS;/** SuperKEKB PVs */
      double VALCLM_D06H3OUT_CSS_NOWPOS_BMY_POS;/** SuperKEKB PVs */

      //SKB LER collimator position with respect to beam
      double VALCLM_D06H4IN_CSS_NOWPOS_DIF_POS;/** SuperKEKB PVs */
      double VALCLM_D06H4OUT_CSS_NOWPOS_DIF_POS;/** SuperKEKB PVs */
      double VALCLM_D06H3IN_CSS_NOWPOS_DIF_POS;/** SuperKEKB PVs */
      double VALCLM_D06H3OUT_CSS_NOWPOS_DIF_POS;/** SuperKEKB PVs */

      double VAHCLM_D09H1_CLM_POS;/** SuperKEKB PVs */
      double VAHCLM_D09H2_CLM_POS;/** SuperKEKB PVs */
      double VAHCLM_D09H3_CLM_POS;/** SuperKEKB PVs */
      double VAHCLM_D09H4_CLM_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V1_CLM_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V2_CLM_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V3_CLM_POS;/** SuperKEKB PVs */
      double VAHCLM_D09V4_CLM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H1_CLM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H2_CLM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H3_CLM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12H4_CLM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V1_CLM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V2_CLM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V3_CLM_POS;/** SuperKEKB PVs */
      double VAHCLM_D12V4_CLM_POS;/** SuperKEKB PVs */

      double VALCLM_D06H3IN_CSS_CLM_POS;/** SuperKEKB PVs */
      double VALCLM_D06H3OUT_CSS_CLM_POS;/** SuperKEKB PVs */
      double VALCLM_D06H4IN_CSS_CLM_POS;/** SuperKEKB PVs */
      double VALCLM_D06H4OUT_CSS_CLM_POS;/** SuperKEKB PVs */

      //Beam size monitors
      double BMHSRM_BEAM_SIGMAX;/** SuperKEKB PVs */
      double BMHSRM_BEAM_SIGMAY;/** SuperKEKB PVs */
      double BMLSRM_BEAM_SIGMAX;/** SuperKEKB PVs */
      double BMLSRM_BEAM_SIGMAY;/** SuperKEKB PVs */
      double BMHXRM_BEAM_SIGMAY;/** SuperKEKB PVs */
      double BMLXRM_BEAM_SIGMAY;/** SuperKEKB PVs */
      double BMHXRM_BEAM_SIGMAX;/** SuperKEKB PVs */
      double BMLXRM_BEAM_SIGMAX;/** SuperKEKB PVs */

      //Integrated beam dose in A*hours
      double VAHBMD_BEAMDOSE_VAL;/** SuperKEKB PVs */
      double VALBMD_BEAMDOSE_VAL;/** SuperKEKB PVs */

      //Bunch number
      double CGHINJ_BKSEL_NOB_SET;/** SuperKEKB PVs */
      double CGLINJ_BKSEL_NOB_SET;/** SuperKEKB PVs */

      double VALCCG_D02_L18_PRES;/** SuperKEKB PVs */
      double VALCCG_D02_L19_PRES;/** SuperKEKB PVs */
      double VALCCG_D02_L20_PRES;/** SuperKEKB PVs */
      double VALCCG_D02_L21_PRES;/** SuperKEKB PVs */
      double VALCCG_D02_L22_PRES;/** SuperKEKB PVs */
      double VALCCG_D02_L23_PRES;/** SuperKEKB PVs */
      double VALCCG_D02_L24_PRES;/** SuperKEKB PVs */
      double VALCCG_D02_L25_PRES;/** SuperKEKB PVs */
      double VALCCG_D02_L26_PRES;/** SuperKEKB PVs */
      double VALCCG_D07_L00_PRES;/** SuperKEKB PVs */
      double VALCCG_D07_L01_PRES;/** SuperKEKB PVs */
      double VALCCG_D07_L02_PRES;/** SuperKEKB PVs */
      double VALCCG_D10_L01_PRES;/** SuperKEKB PVs */
      double VALCCG_D10_L02_PRES;/** SuperKEKB PVs */
      double VALCCG_D10_L03_PRES;/** SuperKEKB PVs */
      double VALCCG_D10_L04_PRES;/** SuperKEKB PVs */
      double VALCCG_D10_L05_PRES;/** SuperKEKB PVs */
      double VALCCG_D10_L06_PRES;/** SuperKEKB PVs */
      double VALCCG_D10_L07_PRES;/** SuperKEKB PVs */
      double VALCCG_D10_L08_PRES;/** SuperKEKB PVs */
      double VALCCG_D11_L19_PRES;/** SuperKEKB PVs */
      double VALCCG_D11_L20_PRES;/** SuperKEKB PVs */
      double VALCCG_D11_L21_PRES;/** SuperKEKB PVs */
      double VALCCG_D11_L22_PRES;/** SuperKEKB PVs */
      double VALCCG_D11_L23_PRES;/** SuperKEKB PVs */
      double VALCCG_D11_L24_PRES;/** SuperKEKB PVs */
      double VALCCG_D11_L25_PRES;/** SuperKEKB PVs */

      double BM_DCCT_HLIFE;/** SuperKEKB PVs */
      double BM_DCCT_LLIFE;/** SuperKEKB PVs */


      double VALRGA_D06_MV1_SEQ_MAS_01;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_02;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_03;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_04;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_05;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_06;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_07;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_08;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_09;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_10;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_11;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_12;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_13;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_14;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_15;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_16;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_17;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_18;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_19;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_20;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_21;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_22;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_23;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_24;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_25;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_26;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_27;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_28;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_29;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_30;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_31;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_32;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_33;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_34;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_35;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_36;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_37;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_38;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_39;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_40;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_41;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_42;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_43;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_44;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_45;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_46;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_47;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_48;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_49;/** SuperKEKB PVs */
      double VALRGA_D06_MV1_SEQ_MAS_50;/** SuperKEKB PVs */


      double VALRGA_D02_MV1_SEQ_MAS_01;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_02;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_03;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_04;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_05;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_06;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_07;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_08;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_09;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_10;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_11;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_12;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_13;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_14;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_15;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_16;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_17;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_18;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_19;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_20;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_21;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_22;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_23;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_24;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_25;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_26;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_27;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_28;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_29;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_30;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_31;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_32;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_33;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_34;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_35;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_36;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_37;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_38;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_39;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_40;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_41;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_42;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_43;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_44;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_45;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_46;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_47;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_48;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_49;/** SuperKEKB PVs */
      double VALRGA_D02_MV1_SEQ_MAS_50;/** SuperKEKB PVs */

      /**
       * Clear the structure: set elements to zero
       */
      void clear()
      {
        ts = 0;
        CGLINJ_BEAM_GATE_STATUS = 0;
        CGHINJ_BEAM_GATE_STATUS = 0;
        CO_KEKB_KEKB_Status_FLG = 0;
        CGHINJ_RATE_AVE = 0;
        CGLINJ_RATE_AVE = 0;
        COpLER_BEAM_LIFE = 0;
        COeHER_BEAM_LIFE = 0;
        BM_DCCT_HCUR = 0;
        BM_DCCT_LCUR = 0;
        CGLINJ_EFFICIENCY = 0;
        CGHINJ_EFFICIENCY = 0;
        BM_BLM_D01_ADC_MEAN = 0;
        BM_BLM_TB4_ADC_MEAN = 0;
        BM_BLM_D07_ADC_MEAN = 0;
        BM_BLM_D10_ADC_MEAN = 0;
        CG_BTP_BPM_CHARGE_AVE = 0;
        CG_BTE_BPM_CHARGE_AVE = 0;
        BMHDCCT_LIFE = 0;
        BMLDCCT_LIFE = 0;
        LIiEV_BEAM_REP_READ_KBP = 0;
        LIiEV_BEAM_REP_READ_KBE = 0;
        TM_BKT_config_SNAM = 0;
        TM_BKT_AUTO_FILL_Mode = 0;
        TM_BKT_AUTO_FILL_Nbunch = 0;
        TM_BKT_ONE_BUNCH_LER_InjBucket = 0;
        TM_BKT_ONE_BUNCH_HER_InjBucket = 0;
        TM_BKT_ONE_BUNCH_Nbunch = 0;
        CGLSAFE_MR_ABORT = 0;
        CGHSAFE_MR_ABORT = 0;

        VALCCG_LER_PRES_AVG = 0;
        VALCCG_D01_LER_PRES_AVG = 0;
        VALCCG_D02_LER_PRES_AVG = 0;
        VALCCG_D03_LER_PRES_AVG = 0;
        VALCCG_D04_LER_PRES_AVG = 0;
        VALCCG_D05_LER_PRES_AVG = 0;
        VALCCG_D06_LER_PRES_AVG = 0;
        VALCCG_D07_LER_PRES_AVG = 0;
        VALCCG_D08_LER_PRES_AVG = 0;
        VALCCG_D09_LER_PRES_AVG = 0;
        VALCCG_D10_LER_PRES_AVG = 0;
        VALCCG_D11_LER_PRES_AVG = 0;
        VALCCG_D12_LER_PRES_AVG = 0;

        VAHCCG_HER_PRES_AVG = 0;
        VAHCCG_D01_HER_PRES_AVG = 0;
        VAHCCG_D02_HER_PRES_AVG = 0;
        VAHCCG_D03_HER_PRES_AVG = 0;
        VAHCCG_D04_HER_PRES_AVG = 0;
        VAHCCG_D05_HER_PRES_AVG = 0;
        VAHCCG_D06_HER_PRES_AVG = 0;
        VAHCCG_D07_HER_PRES_AVG = 0;
        VAHCCG_D08_HER_PRES_AVG = 0;
        VAHCCG_D09_HER_PRES_AVG = 0;
        VAHCCG_D10_HER_PRES_AVG = 0;
        VAHCCG_D11_HER_PRES_AVG = 0;
        VAHCCG_D12_HER_PRES_AVG = 0;

        //SKB HER collimator absolute positions as measured by DMM
        VAHCLM_D09H1_PM_POS = 0;
        VAHCLM_D09H2_PM_POS = 0;
        VAHCLM_D09H3_PM_POS = 0;
        VAHCLM_D09H4_PM_POS = 0;
        VAHCLM_D09V1_PM_POS = 0;
        VAHCLM_D09V2_PM_POS = 0;
        VAHCLM_D09V3_PM_POS = 0;
        VAHCLM_D09V4_PM_POS = 0;
        VAHCLM_D12H1_PM_POS = 0;
        VAHCLM_D12H2_PM_POS = 0;
        VAHCLM_D12H3_PM_POS = 0;
        VAHCLM_D12H4_PM_POS = 0;
        VAHCLM_D12V1_PM_POS = 0;
        VAHCLM_D12V2_PM_POS = 0;
        VAHCLM_D12V3_PM_POS = 0;
        VAHCLM_D12V4_PM_POS = 0;

        //SKB HER beam position in X
        VAHCLM_D09H1_BMX_POS = 0;
        VAHCLM_D09H2_BMX_POS = 0;
        VAHCLM_D09H3_BMX_POS = 0;
        VAHCLM_D09H4_BMX_POS = 0;
        VAHCLM_D09V1_BMX_POS = 0;
        VAHCLM_D09V2_BMX_POS = 0;
        VAHCLM_D09V3_BMX_POS = 0;
        VAHCLM_D09V4_BMX_POS = 0;
        VAHCLM_D12H1_BMX_POS = 0;
        VAHCLM_D12H2_BMX_POS = 0;
        VAHCLM_D12H3_BMX_POS = 0;
        VAHCLM_D12H4_BMX_POS = 0;
        VAHCLM_D12V1_BMX_POS = 0;
        VAHCLM_D12V2_BMX_POS = 0;
        VAHCLM_D12V3_BMX_POS = 0;
        VAHCLM_D12V4_BMX_POS = 0;

        //SKB HER beam position in Y
        VAHCLM_D09H1_BMY_POS = 0;
        VAHCLM_D09H2_BMY_POS = 0;
        VAHCLM_D09H3_BMY_POS = 0;
        VAHCLM_D09H4_BMY_POS = 0;
        VAHCLM_D09V1_BMY_POS = 0;
        VAHCLM_D09V2_BMY_POS = 0;
        VAHCLM_D09V3_BMY_POS = 0;
        VAHCLM_D09V4_BMY_POS = 0;
        VAHCLM_D12H1_BMY_POS = 0;
        VAHCLM_D12H2_BMY_POS = 0;
        VAHCLM_D12H3_BMY_POS = 0;
        VAHCLM_D12H4_BMY_POS = 0;
        VAHCLM_D12V1_BMY_POS = 0;
        VAHCLM_D12V2_BMY_POS = 0;
        VAHCLM_D12V3_BMY_POS = 0;
        VAHCLM_D12V4_BMY_POS = 0;

        //SKB HER collimator position with respect to beam
        VAHCLM_D09H1_DIF_POS = 0;
        VAHCLM_D09H2_DIF_POS = 0;
        VAHCLM_D09H3_DIF_POS = 0;
        VAHCLM_D09H4_DIF_POS = 0;
        VAHCLM_D09V1_DIF_POS = 0;
        VAHCLM_D09V2_DIF_POS = 0;
        VAHCLM_D09V3_DIF_POS = 0;
        VAHCLM_D09V4_DIF_POS = 0;
        VAHCLM_D12H1_DIF_POS = 0;
        VAHCLM_D12H2_DIF_POS = 0;
        VAHCLM_D12H3_DIF_POS = 0;
        VAHCLM_D12H4_DIF_POS = 0;
        VAHCLM_D12V1_DIF_POS = 0;
        VAHCLM_D12V2_DIF_POS = 0;
        VAHCLM_D12V3_DIF_POS = 0;
        VAHCLM_D12V4_DIF_POS = 0;

        //SKB LER collimator absolute positions as measured by DMM
        VALCLM_D06H4IN_CSS_NOWPOS_DMM = 0;
        VALCLM_D06H4OUT_CSS_NOWPOS_DMM = 0;
        VALCLM_D06H3IN_CSS_NOWPOS_DMM = 0;
        VALCLM_D06H3OUT_CSS_NOWPOS_DMM = 0;

        //SKB LER beam position in X
        VALCLM_D06H4IN_CSS_NOWPOS_BMX_POS = 0;
        VALCLM_D06H4OUT_CSS_NOWPOS_BMX_POS = 0;
        VALCLM_D06H3IN_CSS_NOWPOS_BMX_POS = 0;
        VALCLM_D06H3OUT_CSS_NOWPOS_BMX_POS = 0;

        //SKB LER beam position in Y
        VALCLM_D06H4IN_CSS_NOWPOS_BMY_POS = 0;
        VALCLM_D06H4OUT_CSS_NOWPOS_BMY_POS = 0;
        VALCLM_D06H3IN_CSS_NOWPOS_BMY_POS = 0;
        VALCLM_D06H3OUT_CSS_NOWPOS_BMY_POS = 0;

        //SKB LER collimator position with respect to beam
        VALCLM_D06H4IN_CSS_NOWPOS_DIF_POS = 0;
        VALCLM_D06H4OUT_CSS_NOWPOS_DIF_POS = 0;
        VALCLM_D06H3IN_CSS_NOWPOS_DIF_POS = 0;
        VALCLM_D06H3OUT_CSS_NOWPOS_DIF_POS = 0;

        //Beam size monitors
        BMHXRM_BEAM_SIGMAY = 0;
        BMLXRM_BEAM_SIGMAY = 0;
        BMHXRM_BEAM_SIGMAX = 0;
        BMLXRM_BEAM_SIGMAX = 0;
        BMHSRM_BEAM_SIGMAX = 0;
        BMHSRM_BEAM_SIGMAY = 0;
        BMLSRM_BEAM_SIGMAX = 0;
        BMLSRM_BEAM_SIGMAY = 0;

        //Integrated beam dose in A*hours
        VAHBMD_BEAMDOSE_VAL = 0;
        VALBMD_BEAMDOSE_VAL = 0;

        //Bunch number
        CGHINJ_BKSEL_NOB_SET = 0;
        CGLINJ_BKSEL_NOB_SET = 0;

        VALCCG_D02_L18_PRES = 0;
        VALCCG_D02_L19_PRES = 0;
        VALCCG_D02_L20_PRES = 0;
        VALCCG_D02_L21_PRES = 0;
        VALCCG_D02_L22_PRES = 0;
        VALCCG_D02_L23_PRES = 0;
        VALCCG_D02_L24_PRES = 0;
        VALCCG_D02_L25_PRES = 0;
        VALCCG_D02_L26_PRES = 0;
        VALCCG_D07_L00_PRES = 0;
        VALCCG_D07_L01_PRES = 0;
        VALCCG_D07_L02_PRES = 0;
        VALCCG_D10_L01_PRES = 0;
        VALCCG_D10_L02_PRES = 0;
        VALCCG_D10_L03_PRES = 0;
        VALCCG_D10_L04_PRES = 0;
        VALCCG_D10_L05_PRES = 0;
        VALCCG_D10_L06_PRES = 0;
        VALCCG_D10_L07_PRES = 0;
        VALCCG_D10_L08_PRES = 0;
        VALCCG_D11_L19_PRES = 0;
        VALCCG_D11_L20_PRES = 0;
        VALCCG_D11_L21_PRES = 0;
        VALCCG_D11_L22_PRES = 0;
        VALCCG_D11_L23_PRES = 0;
        VALCCG_D11_L24_PRES = 0;
        VALCCG_D11_L25_PRES = 0;
        VALCLM_D06H3OUT_CSS_CLM_POS = 0;
        VALCLM_D06H3IN_CSS_CLM_POS = 0;
        VALCLM_D06H4OUT_CSS_CLM_POS = 0;
        VALCLM_D06H4IN_CSS_CLM_POS = 0;
        BM_DCCT_HLIFE = 0;
        BM_DCCT_LLIFE = 0;

        VAHCLM_D09H1_CLM_POS = 0;
        VAHCLM_D09H2_CLM_POS = 0;
        VAHCLM_D09H3_CLM_POS = 0;
        VAHCLM_D09H4_CLM_POS = 0;
        VAHCLM_D09V1_CLM_POS = 0;
        VAHCLM_D09V2_CLM_POS = 0;
        VAHCLM_D09V3_CLM_POS = 0;
        VAHCLM_D09V4_CLM_POS = 0;
        VAHCLM_D12H1_CLM_POS = 0;
        VAHCLM_D12H2_CLM_POS = 0;
        VAHCLM_D12H3_CLM_POS = 0;
        VAHCLM_D12H4_CLM_POS = 0;
        VAHCLM_D12V1_CLM_POS = 0;
        VAHCLM_D12V2_CLM_POS = 0;
        VAHCLM_D12V3_CLM_POS = 0;
        VAHCLM_D12V4_CLM_POS = 0;


        VALRGA_D06_MV1_SEQ_MAS_01 = 0;
        VALRGA_D06_MV1_SEQ_MAS_02 = 0;
        VALRGA_D06_MV1_SEQ_MAS_03 = 0;
        VALRGA_D06_MV1_SEQ_MAS_04 = 0;
        VALRGA_D06_MV1_SEQ_MAS_05 = 0;
        VALRGA_D06_MV1_SEQ_MAS_06 = 0;
        VALRGA_D06_MV1_SEQ_MAS_07 = 0;
        VALRGA_D06_MV1_SEQ_MAS_08 = 0;
        VALRGA_D06_MV1_SEQ_MAS_09 = 0;
        VALRGA_D06_MV1_SEQ_MAS_10 = 0;
        VALRGA_D06_MV1_SEQ_MAS_11 = 0;
        VALRGA_D06_MV1_SEQ_MAS_12 = 0;
        VALRGA_D06_MV1_SEQ_MAS_13 = 0;
        VALRGA_D06_MV1_SEQ_MAS_14 = 0;
        VALRGA_D06_MV1_SEQ_MAS_15 = 0;
        VALRGA_D06_MV1_SEQ_MAS_16 = 0;
        VALRGA_D06_MV1_SEQ_MAS_17 = 0;
        VALRGA_D06_MV1_SEQ_MAS_18 = 0;
        VALRGA_D06_MV1_SEQ_MAS_19 = 0;
        VALRGA_D06_MV1_SEQ_MAS_20 = 0;
        VALRGA_D06_MV1_SEQ_MAS_21 = 0;
        VALRGA_D06_MV1_SEQ_MAS_22 = 0;
        VALRGA_D06_MV1_SEQ_MAS_23 = 0;
        VALRGA_D06_MV1_SEQ_MAS_24 = 0;
        VALRGA_D06_MV1_SEQ_MAS_25 = 0;
        VALRGA_D06_MV1_SEQ_MAS_26 = 0;
        VALRGA_D06_MV1_SEQ_MAS_27 = 0;
        VALRGA_D06_MV1_SEQ_MAS_28 = 0;
        VALRGA_D06_MV1_SEQ_MAS_29 = 0;
        VALRGA_D06_MV1_SEQ_MAS_30 = 0;
        VALRGA_D06_MV1_SEQ_MAS_31 = 0;
        VALRGA_D06_MV1_SEQ_MAS_32 = 0;
        VALRGA_D06_MV1_SEQ_MAS_33 = 0;
        VALRGA_D06_MV1_SEQ_MAS_34 = 0;
        VALRGA_D06_MV1_SEQ_MAS_35 = 0;
        VALRGA_D06_MV1_SEQ_MAS_36 = 0;
        VALRGA_D06_MV1_SEQ_MAS_37 = 0;
        VALRGA_D06_MV1_SEQ_MAS_38 = 0;
        VALRGA_D06_MV1_SEQ_MAS_39 = 0;
        VALRGA_D06_MV1_SEQ_MAS_40 = 0;
        VALRGA_D06_MV1_SEQ_MAS_41 = 0;
        VALRGA_D06_MV1_SEQ_MAS_42 = 0;
        VALRGA_D06_MV1_SEQ_MAS_43 = 0;
        VALRGA_D06_MV1_SEQ_MAS_44 = 0;
        VALRGA_D06_MV1_SEQ_MAS_45 = 0;
        VALRGA_D06_MV1_SEQ_MAS_46 = 0;
        VALRGA_D06_MV1_SEQ_MAS_47 = 0;
        VALRGA_D06_MV1_SEQ_MAS_48 = 0;
        VALRGA_D06_MV1_SEQ_MAS_49 = 0;
        VALRGA_D06_MV1_SEQ_MAS_50 = 0;

        VALRGA_D02_MV1_SEQ_MAS_01 = 0;
        VALRGA_D02_MV1_SEQ_MAS_02 = 0;
        VALRGA_D02_MV1_SEQ_MAS_03 = 0;
        VALRGA_D02_MV1_SEQ_MAS_04 = 0;
        VALRGA_D02_MV1_SEQ_MAS_05 = 0;
        VALRGA_D02_MV1_SEQ_MAS_06 = 0;
        VALRGA_D02_MV1_SEQ_MAS_07 = 0;
        VALRGA_D02_MV1_SEQ_MAS_08 = 0;
        VALRGA_D02_MV1_SEQ_MAS_09 = 0;
        VALRGA_D02_MV1_SEQ_MAS_10 = 0;
        VALRGA_D02_MV1_SEQ_MAS_11 = 0;
        VALRGA_D02_MV1_SEQ_MAS_12 = 0;
        VALRGA_D02_MV1_SEQ_MAS_13 = 0;
        VALRGA_D02_MV1_SEQ_MAS_14 = 0;
        VALRGA_D02_MV1_SEQ_MAS_15 = 0;
        VALRGA_D02_MV1_SEQ_MAS_16 = 0;
        VALRGA_D02_MV1_SEQ_MAS_17 = 0;
        VALRGA_D02_MV1_SEQ_MAS_18 = 0;
        VALRGA_D02_MV1_SEQ_MAS_19 = 0;
        VALRGA_D02_MV1_SEQ_MAS_20 = 0;
        VALRGA_D02_MV1_SEQ_MAS_21 = 0;
        VALRGA_D02_MV1_SEQ_MAS_22 = 0;
        VALRGA_D02_MV1_SEQ_MAS_23 = 0;
        VALRGA_D02_MV1_SEQ_MAS_24 = 0;
        VALRGA_D02_MV1_SEQ_MAS_25 = 0;
        VALRGA_D02_MV1_SEQ_MAS_26 = 0;
        VALRGA_D02_MV1_SEQ_MAS_27 = 0;
        VALRGA_D02_MV1_SEQ_MAS_28 = 0;
        VALRGA_D02_MV1_SEQ_MAS_29 = 0;
        VALRGA_D02_MV1_SEQ_MAS_30 = 0;
        VALRGA_D02_MV1_SEQ_MAS_31 = 0;
        VALRGA_D02_MV1_SEQ_MAS_32 = 0;
        VALRGA_D02_MV1_SEQ_MAS_33 = 0;
        VALRGA_D02_MV1_SEQ_MAS_34 = 0;
        VALRGA_D02_MV1_SEQ_MAS_35 = 0;
        VALRGA_D02_MV1_SEQ_MAS_36 = 0;
        VALRGA_D02_MV1_SEQ_MAS_37 = 0;
        VALRGA_D02_MV1_SEQ_MAS_38 = 0;
        VALRGA_D02_MV1_SEQ_MAS_39 = 0;
        VALRGA_D02_MV1_SEQ_MAS_40 = 0;
        VALRGA_D02_MV1_SEQ_MAS_41 = 0;
        VALRGA_D02_MV1_SEQ_MAS_42 = 0;
        VALRGA_D02_MV1_SEQ_MAS_43 = 0;
        VALRGA_D02_MV1_SEQ_MAS_44 = 0;
        VALRGA_D02_MV1_SEQ_MAS_45 = 0;
        VALRGA_D02_MV1_SEQ_MAS_46 = 0;
        VALRGA_D02_MV1_SEQ_MAS_47 = 0;
        VALRGA_D02_MV1_SEQ_MAS_48 = 0;
        VALRGA_D02_MV1_SEQ_MAS_49 = 0;
        VALRGA_D02_MV1_SEQ_MAS_50 = 0;
      }

    };

  } // SUPERKEKB namespace
} // Belle2 namespace

#endif

