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
     * https://belle2.cc.kek.jp/~twiki/bin/view/Detector/SUPERKEKB/ROOTDataFormats
     */
    struct SuperKEKBTree {

      /**SuperKEKB PVs **/
      double ts;
      double CGLINJ_BEAM_GATE_STATUS;
      double CGHINJ_BEAM_GATE_STATUS;
      double CO_KEKB_KEKB_Status_FLG;
      double CGHINJ_RATE_AVE;
      double CGLINJ_RATE_AVE;
      double COpLER_BEAM_LIFE;
      double COeHER_BEAM_LIFE;
      double BM_DCCT_HCUR;
      double BM_DCCT_LCUR;
      double CGLINJ_EFFICIENCY;
      double CGHINJ_EFFICIENCY;
      double BM_BLM_D01_ADC_MEAN;
      double BM_BLM_TB4_ADC_MEAN;
      double BM_BLM_D07_ADC_MEAN;
      double BM_BLM_D10_ADC_MEAN;
      double CG_BTP_BPM_CHARGE_AVE;
      double CG_BTE_BPM_CHARGE_AVE;
      double BMHDCCT_LIFE;
      double BMLDCCT_LIFE;
      double LIiEV_BEAM_REP_READ_KBP;
      double LIiEV_BEAM_REP_READ_KBE;
      double TM_BKT_config_SNAM;
      double TM_BKT_AUTO_FILL_Mode;
      double TM_BKT_AUTO_FILL_Nbunch;
      double TM_BKT_ONE_BUNCH_LER_InjBucket;
      double TM_BKT_ONE_BUNCH_HER_InjBucket;
      double TM_BKT_ONE_BUNCH_Nbunch;
      double CGLSAFE_MR_ABORT;
      double CGHSAFE_MR_ABORT;

      double VALCCG_LER_PRES_AVG;
      double VALCCG_D01_LER_PRES_AVG;
      double VALCCG_D02_LER_PRES_AVG;
      double VALCCG_D03_LER_PRES_AVG;
      double VALCCG_D04_LER_PRES_AVG;
      double VALCCG_D05_LER_PRES_AVG;
      double VALCCG_D06_LER_PRES_AVG;
      double VALCCG_D07_LER_PRES_AVG;
      double VALCCG_D08_LER_PRES_AVG;
      double VALCCG_D09_LER_PRES_AVG;
      double VALCCG_D10_LER_PRES_AVG;
      double VALCCG_D11_LER_PRES_AVG;
      double VALCCG_D12_LER_PRES_AVG;

      double VAHCCG_HER_PRES_AVG;
      double VAHCCG_D01_HER_PRES_AVG;
      double VAHCCG_D02_HER_PRES_AVG;
      double VAHCCG_D03_HER_PRES_AVG;
      double VAHCCG_D04_HER_PRES_AVG;
      double VAHCCG_D05_HER_PRES_AVG;
      double VAHCCG_D06_HER_PRES_AVG;
      double VAHCCG_D07_HER_PRES_AVG;
      double VAHCCG_D08_HER_PRES_AVG;
      double VAHCCG_D09_HER_PRES_AVG;
      double VAHCCG_D10_HER_PRES_AVG;
      double VAHCCG_D11_HER_PRES_AVG;
      double VAHCCG_D12_HER_PRES_AVG;

      //SKB HER collimator absolute positions as measured by DMM
      double VAHCLM_D09H1_PM_POS;
      double VAHCLM_D09H2_PM_POS;
      double VAHCLM_D09H3_PM_POS;
      double VAHCLM_D09H4_PM_POS;
      double VAHCLM_D09V1_PM_POS;
      double VAHCLM_D09V2_PM_POS;
      double VAHCLM_D09V3_PM_POS;
      double VAHCLM_D09V4_PM_POS;
      double VAHCLM_D12H1_PM_POS;
      double VAHCLM_D12H2_PM_POS;
      double VAHCLM_D12H3_PM_POS;
      double VAHCLM_D12H4_PM_POS;
      double VAHCLM_D12V1_PM_POS;
      double VAHCLM_D12V2_PM_POS;
      double VAHCLM_D12V3_PM_POS;
      double VAHCLM_D12V4_PM_POS;

      //SKB HER beam position in X
      double VAHCLM_D09H1_BMX_POS;
      double VAHCLM_D09H2_BMX_POS;
      double VAHCLM_D09H3_BMX_POS;
      double VAHCLM_D09H4_BMX_POS;
      double VAHCLM_D09V1_BMX_POS;
      double VAHCLM_D09V2_BMX_POS;
      double VAHCLM_D09V3_BMX_POS;
      double VAHCLM_D09V4_BMX_POS;
      double VAHCLM_D12H1_BMX_POS;
      double VAHCLM_D12H2_BMX_POS;
      double VAHCLM_D12H3_BMX_POS;
      double VAHCLM_D12H4_BMX_POS;
      double VAHCLM_D12V1_BMX_POS;
      double VAHCLM_D12V2_BMX_POS;
      double VAHCLM_D12V3_BMX_POS;
      double VAHCLM_D12V4_BMX_POS;

      //SKB HER beam position in Y
      double VAHCLM_D09H1_BMY_POS;
      double VAHCLM_D09H2_BMY_POS;
      double VAHCLM_D09H3_BMY_POS;
      double VAHCLM_D09H4_BMY_POS;
      double VAHCLM_D09V1_BMY_POS;
      double VAHCLM_D09V2_BMY_POS;
      double VAHCLM_D09V3_BMY_POS;
      double VAHCLM_D09V4_BMY_POS;
      double VAHCLM_D12H1_BMY_POS;
      double VAHCLM_D12H2_BMY_POS;
      double VAHCLM_D12H3_BMY_POS;
      double VAHCLM_D12H4_BMY_POS;
      double VAHCLM_D12V1_BMY_POS;
      double VAHCLM_D12V2_BMY_POS;
      double VAHCLM_D12V3_BMY_POS;
      double VAHCLM_D12V4_BMY_POS;

      //SKB HER collimator position with respect to beam
      double VAHCLM_D09H1_DIF_POS;
      double VAHCLM_D09H2_DIF_POS;
      double VAHCLM_D09H3_DIF_POS;
      double VAHCLM_D09H4_DIF_POS;
      double VAHCLM_D09V1_DIF_POS;
      double VAHCLM_D09V2_DIF_POS;
      double VAHCLM_D09V3_DIF_POS;
      double VAHCLM_D09V4_DIF_POS;
      double VAHCLM_D12H1_DIF_POS;
      double VAHCLM_D12H2_DIF_POS;
      double VAHCLM_D12H3_DIF_POS;
      double VAHCLM_D12H4_DIF_POS;
      double VAHCLM_D12V1_DIF_POS;
      double VAHCLM_D12V2_DIF_POS;
      double VAHCLM_D12V3_DIF_POS;
      double VAHCLM_D12V4_DIF_POS;

      //SKB LER collimator absolute positions as measured by DMM
      double VALCLM_D06H4IN_CSS_NOWPOS_DMM;
      double VALCLM_D06H4OUT_CSS_NOWPOS_DMM;
      double VALCLM_D06H3IN_CSS_NOWPOS_DMM;
      double VALCLM_D06H3OUT_CSS_NOWPOS_DMM;

      //SKB LER beam position in X
      double VALCLM_D06H4IN_CSS_NOWPOS_BMX_POS;
      double VALCLM_D06H4OUT_CSS_NOWPOS_BMX_POS;
      double VALCLM_D06H3IN_CSS_NOWPOS_BMX_POS;
      double VALCLM_D06H3OUT_CSS_NOWPOS_BMX_POS;

      //SKB LER beam position in Y
      double VALCLM_D06H4IN_CSS_NOWPOS_BMY_POS;
      double VALCLM_D06H4OUT_CSS_NOWPOS_BMY_POS;
      double VALCLM_D06H3IN_CSS_NOWPOS_BMY_POS;
      double VALCLM_D06H3OUT_CSS_NOWPOS_BMY_POS;

      //SKB LER collimator position with respect to beam
      double VALCLM_D06H4IN_CSS_NOWPOS_DIF_POS;
      double VALCLM_D06H4OUT_CSS_NOWPOS_DIF_POS;
      double VALCLM_D06H3IN_CSS_NOWPOS_DIF_POS;
      double VALCLM_D06H3OUT_CSS_NOWPOS_DIF_POS;

      //Beam size monitors
      double BMHSRM_BEAM_SIGMAX;
      double BMHSRM_BEAM_SIGMAY;
      double BMLSRM_BEAM_SIGMAX;
      double BMLSRM_BEAM_SIGMAY;
      double BMHXRM_BEAM_SIGMAY;
      double BMLXRM_BEAM_SIGMAY;
      double BMHXRM_BEAM_SIGMAX;
      double BMLXRM_BEAM_SIGMAX;

      //Integrated beam dose in A*hours
      double VAHBMD_BEAMDOSE_VAL;
      double VALBMD_BEAMDOSE_VAL;

      //Bunch number
      double CGHINJ_BKSEL_NOB_SET;
      double CGLINJ_BKSEL_NOB_SET;

      double VALCCG_D02_L18_PRES;
      double VALCCG_D02_L19_PRES;
      double VALCCG_D02_L20_PRES;
      double VALCCG_D02_L21_PRES;
      double VALCCG_D02_L22_PRES;
      double VALCCG_D02_L23_PRES;
      double VALCCG_D02_L24_PRES;
      double VALCCG_D02_L25_PRES;
      double VALCCG_D02_L26_PRES;
      double VALCCG_D07_L00_PRES;
      double VALCCG_D07_L01_PRES;
      double VALCCG_D07_L02_PRES;
      double VALCCG_D10_L01_PRES;
      double VALCCG_D10_L02_PRES;
      double VALCCG_D10_L03_PRES;
      double VALCCG_D10_L04_PRES;
      double VALCCG_D10_L05_PRES;
      double VALCCG_D10_L06_PRES;
      double VALCCG_D10_L07_PRES;
      double VALCCG_D10_L08_PRES;
      double VALCCG_D11_L19_PRES;
      double VALCCG_D11_L20_PRES;
      double VALCCG_D11_L21_PRES;
      double VALCCG_D11_L22_PRES;
      double VALCCG_D11_L23_PRES;
      double VALCCG_D11_L24_PRES;
      double VALCCG_D11_L25_PRES;
      double VALCLM_D06H3OUT_CSS_CLM_POS;
      double VALCLM_D06H3IN_CSS_CLM_POS;
      double VALCLM_D06H4OUT_CSS_CLM_POS;
      double VALCLM_D06H4IN_CSS_CLM_POS;
      double BM_DCCT_HLIFE;
      double BM_DCCT_LLIFE;

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
      }

    };

  } // SUPERKEKB namespace
} // Belle2 namespace

#endif

