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
      double BM_DCCT_HCUR;
      double BM_DCCT_LCUR;
      double CGLINJ_BEAM_GATE_STATUS;
      double CGHINJ_BEAM_GATE_STATUS;
      double VAHBMD_BEAMDOSE_VAL;
      double VALBMD_BEAMDOSE_VAL;
      double VAHCCG_HER_PRES_AVG;
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
      double BMHXRM_BEAM_SIGMAY;
      double BMLXRM_BEAM_SIGMAY;
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
      double CGLINJ_EFFICIENCY;
      double CGHINJ_EFFICIENCY;
      double COpLER_BEAM_LIFE;
      double COeHER_BEAM_LIFE;
      double BM_DCCT_HLIFE;
      double BMHDCCT_LIFE;
      double BM_DCCT_LLIFE;
      double BMLDCCT_LIFE;

      /**
       * Clear the structure: set elements to zero
       */
      void clear()
      {
        ts = 0;
        BM_DCCT_HCUR = 0;
        BM_DCCT_LCUR = 0;
        CGLINJ_BEAM_GATE_STATUS = 0;
        CGHINJ_BEAM_GATE_STATUS = 0;
        VAHBMD_BEAMDOSE_VAL = 0;
        VALBMD_BEAMDOSE_VAL = 0;
        VAHCCG_HER_PRES_AVG = 0;
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
        BMHXRM_BEAM_SIGMAY = 0;
        BMLXRM_BEAM_SIGMAY = 0;
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
        CGLINJ_EFFICIENCY = 0;
        CGHINJ_EFFICIENCY = 0;
        COpLER_BEAM_LIFE = 0;
        COeHER_BEAM_LIFE = 0;
        BM_DCCT_HLIFE = 0;
        BMHDCCT_LIFE = 0;
        BM_DCCT_LLIFE = 0;
        BMLDCCT_LIFE = 0;
      }

    };

  } // SUPERKEKB namespace
} // Belle2 namespace

#endif

