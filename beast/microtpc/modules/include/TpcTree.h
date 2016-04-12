/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TPCTREE_H
#define TPCTREE_H

#include <TTree.h>

namespace Belle2 {
  namespace TPC {

    /**
     * Structure for the output of test beam simulation to a root file (TTree "tpc")
     * https://belle2.cc.kek.jp/~twiki/bin/view/Detector/TPC/BeamTestJune2013ROOTDataFormats
     */
    struct TpcTree {
      enum {MaxRoi = 30000}; /**< max number of hits */

      Int_t runNum;          /**< run number */
      Int_t eventNum;        /**< event number */
      Short_t eventflag;     /**< 1:good event, 0:rejected, -1:junk */
      Short_t eventtag_tpc;  /**< tag for matching events btw. TPC and LEPS DAQ systems */

      /**ASIC variables**/
      int m_evtnb;                   /**<TPC event number.*/
      int m_pxhits;                  /**<ASIC pixel number fired.*/
      int m_column[MaxRoi];          /**<ASIC column number 1-336.*/
      int m_row[MaxRoi];             /**<ASIC row number 1-80.*/
      int m_tot[MaxRoi];             /**<ASIC Time-Over-Threshod 1-13.*/
      int m_bcid[MaxRoi];            /**<ASIC Bunch Crossing 1-256.*/
      int m_timestamp_nb;            /**<Nb of readout in one event.*/
      double m_timestamp_start[10];  /**<Readout timestamp start.*/
      double m_timestamp_stop[10];   /**<Readout timestamp stop.*/

      /**TPC slow control variables**/
      Float_t m_Temperature[4];    /**<PIN diode temperature sensor nearby TPC.*/
      Float_t m_Pressure1;         /**<Pressure gauge 1 of TPC 1 and 2 before 17:00 on MAR 29 (JST) or 1459238400 and 2 and 3 after.*/
      Float_t m_Pressure2;         /**<Pressure gauge 2 of TPC 3 and 4 before 17:00 on MAR 29 (JST) or 1459238400 and 1 and 4 after.*/
      Float_t m_Flow1;             /**<Flow gauge 1 of TPC 2 and 3.*/
      Float_t m_Flow2;             /**<Flow gauge 1 of TPC 1 and 4. NB: NOT WORKING(?).*/
      Float_t m_SetFlow;           /**<Flow controller, flow set.*/
      Float_t m_GetFlow;           /**<Flow controller, flow measured at the flow controller point.*/

      /**SuperKEKB variables**/
      Double_t m_IHER;             /**<SKG HER current.*/
      Double_t m_ILER;             /**<SKG LER current.*/
      Double_t m_tauHER;           /**<SKG HER life time.*/
      Double_t m_tauLER;           /**<SKG LER life time.*/
      Double_t m_PHER;             /**<SKG HER average pressure.*/
      Double_t m_PLER;             /**<SKG LER average pressure.*/
      Int_t m_flagHER;             /**<SKG HER injection flag.*/
      Int_t m_flagLER;             /**<SKG LER injection flag.*/

      /**
       * Clear the structure: set elements to zero
       */
      void clear()
      {

        m_pxhits = 0;
        m_evtnb = 0;
        for (int i = 0; i < 30000; i++) {
          m_column[i] = 0;
          m_row[i] = 0;
          m_tot[i] = 0;
          m_bcid[i] = 0;
        }
        m_timestamp_nb = 0;
        for (int i = 0; i < 10; i++) {
          m_timestamp_start[i] = 0;
          m_timestamp_stop[i] = 0;
        }

        for (int i = 0; i < 4; i++)
          m_Temperature[i] = 0;
        m_Pressure1 = 0;
        m_Pressure2 = 0;
        m_Flow1 = 0;
        m_Flow2 = 0;
        m_SetFlow = 0;
        m_GetFlow = 0;

        m_IHER = 0;
        m_ILER = 0;
        m_tauHER = 0;
        m_tauLER = 0;
        m_PHER = 0;
        m_PLER = 0;
        m_flagHER = 0;
        m_flagLER = 0;
      }

    };

  } // TPC namespace
} // Belle2 namespace

#endif

