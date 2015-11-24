/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kenji Inami, Marko Staric, Matthew Barrett               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CRT2014TREE_H
#define CRT2014TREE_H

#include <TTree.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Structure for the output of cosmic test simulation to a root file (TTree "top")
     * https://belle2.cc.kek.jp/~twiki/bin/view/Detector/TOP/BeamTestJune2013ROOTDataFormats
     * The cosmic test format is the same as the format used for the LEPS beam test, except for the
     * non-TOP module comonents, i.e. trigger modules, drift tubes, used for triggering and tracking.
     */

    struct TopTree {
      enum {MaxROI = 392}; /**< Maximum number of hits (392 = 4 SCRODS * 98 max hits per SCROD) */
      enum {MaxDTHits = 100}; /**< Maximum number of hits in each drift tube module */

      Int_t runNum;         /**< Run number */
      Int_t eventNum;       /**< Event number */
      Short_t eventflag;    /**< 1:good event, 0:rejected, -1:junk */

      Short_t nhit;                /**< Number of hits */
      Short_t pmtid_mcp[MaxROI];   /**< PMT ID (Belle II numbering, 1-based) */
      Short_t ch_mcp[MaxROI];      /**< PMT channel ID (Belle II numbering, 1-based) */
      Float_t tdc_mcp[MaxROI];     /**< raw TDC time, unit="ps" */
      Float_t tdc0_mcp[MaxROI];    /**< raw TDC time, unit="ps" */
      Float_t adc0_mcp[MaxROI];    /**< Pulse height, unit="ADC counts" */
      Short_t pmtflag_mcp[MaxROI]; /**< 1:good/main, 0:cross-talk/charge shared 2nd, -1:no hit/bad ch */




      Float_t trig12;              /**< Trigger12 (coincident hits on trigger paddles 1 and 2) */
      Float_t trig13;              /**< Trigger13 (coincident hits on trigger paddles 1 and 3) */
      Float_t trig14;              /**< Trigger14 (coincident hits on trigger paddles 1 and 4) */

      Int_t nDT1;                        /**< Number of hits in drift tube module 1 */
      Int_t nDT2;                        /**< Number of hits in drift tube module 2 */
      Int_t nDT3;                        /**< Number of hits in drift tube module 3 */
      Int_t nDT4;                        /**< Number of hits in drift tube module 4 */
      Int_t DT1ch[MaxDTHits];            /**< Channel numbers of hits in drift tube module 1 */
      Int_t DT2ch[MaxDTHits];            /**< Channel numbers of hits in drift tube module 2 */
      Int_t DT3ch[MaxDTHits];            /**< Channel numbers of hits in drift tube module 3 */
      Int_t DT4ch[MaxDTHits];            /**< Channel numbers of hits in drift tube module 4 */
      Int_t DT1tdc[MaxDTHits];           /**< Raw tdc values of hits in drift tube module 1 */
      Int_t DT2tdc[MaxDTHits];           /**< Raw tdc values of hits in drift tube module 2 */
      Int_t DT3tdc[MaxDTHits];           /**< Raw tdc values of hits in drift tube module 3 */
      Int_t DT4tdc[MaxDTHits];           /**< Raw tdc values of hits in drift tube module 4 */

      Int_t TOF1tdc[4];            /**< Raw tdc values of TOF module 1 */
      Int_t TOF1adc[4];            /**< Raw adc values of TOF module 1 */
      Int_t TOF2tdc[4];            /**< Raw tdc values of TOF module 2 */
      Int_t TOF2adc[4];            /**< Raw adc values of TOF module 2 */
      Int_t TOF3tdc[4];            /**< Raw tdc values of TOF module 3 */
      Int_t TOF3adc[4];            /**< Raw adc values of TOF module 3 */
      Int_t TOF4tdc[4];            /**< Raw tdc values of TOF module 4 */
      Int_t TOF4adc[4];            /**< Raw adc values of TOF module 4 */

      /**
       * Clear the structure: set elements to zero
       */
      void clear()
      {
        runNum = 0;
        eventNum = 0;
        eventflag = 0;
        nhit = 0;

        for (int i(0); i < MaxROI; ++i) {
          pmtid_mcp[i]   = 0;
          ch_mcp[i]      = 0;
          tdc_mcp[i]     = 0;
          tdc0_mcp[i]    = 0;
          adc0_mcp[i]    = 0;
          pmtflag_mcp[i] = 0;
        }

        trig12 = 0;
        trig13 = 0;
        trig14 = 0;

        nDT1 = 0;
        nDT2 = 0;
        nDT3 = 0;
        nDT4 = 0;
        for (int i(0); i < MaxDTHits; ++i) {
          DT1ch[i]  = 0;
          DT2ch[i]  = 0;
          DT3ch[i]  = 0;
          DT4ch[i]  = 0;
          DT1tdc[i] = 0;
          DT2tdc[i] = 0;
          DT3tdc[i] = 0;
          DT4tdc[i] = 0;
        }

        for (int i(0); i < 4; ++i) {
          TOF1tdc[i] = 0;
          TOF1adc[i] = 0;
          TOF2tdc[i] = 0;
          TOF1adc[i] = 0;
          TOF3tdc[i] = 0;
          TOF3adc[i] = 0;
          TOF4tdc[i] = 0;
          TOF4adc[i] = 0;
        }

      }

    };

  } // TOP namespace
} // Belle2 namespace

#endif

