/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kenji Inami, Marko Staric                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPTREE_H
#define TOPTREE_H

#include <TTree.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Structure for the output of test beam simulation to a root file (TTree "top")
     * https://belle2.cc.kek.jp/~twiki/bin/view/Detector/TOP/BeamTestJune2013ROOTDataFormats
     *
     * Belle II numbering scheme (looking from the mirror towards the PMTs):
     * PMT numbering:
     *
     *   +-------------~~~~~~~---------+
     *   |17 18 19 20          30 31 32|
     *   |1  2  3  4           5  6  7 |    note: wedge extends towards lower PMT row
     *   +-------------~~~~~~~---------+
     *
     * PMT channel numbering:
     *
     *   IRS3B readout      CFD readout
     *   +-----------+     +-----------+
     *   |13 14 15 16|     |     4     |
     *   |9  10 11 12|     |     3     |
     *   |5  6  7  8 |     |     2     |
     *   |1  2  3  4 |     |     1     |
     *   +-----------+     +-----------+
     *
     * channelID = (PMT-1)*16+PMTch for IRS3B readout
     * channelID = (PMT-1)*4+PMTch  for CFD readout
     *
     * Coordinate frame for tracker data: ?
     *
     */
    struct TopTree {
      enum {MaxROI = 512}; /**< max number of hits */

      Int_t runNum;         /**< run number */
      Int_t eventNum;       /**< event number */
      Short_t eventflag;    /**< 1:good event, 0:rejected, -1:junk */
      Short_t eventtag_top; /**< tag for matching events btw. TOP and LEPS DAQ systems */

      Short_t nhit;                /**< number of hits */
      Short_t pmtid_mcp[MaxROI];   /**< PMT ID (Belle II numbering, 1-based) */
      Short_t ch_mcp[MaxROI];      /**< PMT channel ID (Belle II numbering, 1-based) */
      Float_t tdc0_mcp[MaxROI];    /**< raw TDC time, unit="ps" */
      Float_t adc0_mcp[MaxROI];    /**< pulse height, unit="ADC counts" */
      Short_t pmtflag_mcp[MaxROI]; /**< 1:good/main, 0:cross-talk/charge shared 2nd, -1:no hit/bad ch */

      Int_t rf[4];                 /**< RF values (50ps/bin) */
      Float_t rf_time;             /**< time obtained from the rf values, unit="ps" */
      Short_t bunchNum;            /**< SPring-8 bunch number */

      Int_t trigS_tdc[2]; /**< trigger counters S TDC (25ps/bin), 0:upstream, 1:downstr */
      Int_t trigS_adc[2]; /**< trigger counters S ADC (0.25pC/bin) */
      Int_t trigM_tdc[2]; /**< trigger counters M TDC (25ps/bin), 0:upstream, 1:downstr */
      Int_t trigM_adc[2]; /**< trigger counters M ADC (0.25pC/bin) */
      Int_t timing_tdc;   /**< timing counter TDC (25ps/bin) */
      Int_t timing_adc;   /**< timing counter ADC (0.25pC/bin) */
      Int_t veto_adc[2];  /**< veto counters ADC (0.25pC/bin), 0:left, 1:right */
      Int_t ratemon;      /**< rate monitor */

      Float_t tdc0_ch[512];    /**< copy of tdc0_mcp, index=channelID-1 (Belle II) */
      Float_t adc0_ch[512];    /**< copy of adc0_mcp, index=channelID-1 (Belle II) */
      Int_t pmtflag_ch[512];   /**< copy of pmtflag_mcp, index=channelID-1 (Belle II) */
      Float_t tdc_mcp[MaxROI]; /**< calibrated & t0 corrected times, unit="ps" */
      Float_t tdc_ch[512];     /**< copy of tdc_mcp, index=channelID-1 (Belle II) */

      Float_t trk_x[2];    /**< sciFi x, unit="mm", 0:upstream, 1:downstream tracker */
      Float_t trk_z_x[2];  /**< sciFi z of x-measuring fibers, unit="mm" */
      Int_t trk_qual_x[2]; /**< quality flag of x-measuring fibers, meaning? */

      Float_t trk_y[2];    /**< sciFi y, unit="mm", 0:upstream, 1:downstream tracker */
      Float_t trk_z_y[2];  /**< sciFi z of y-measuring fibers, unit="mm" */
      Int_t trk_qual_y[2]; /**< quality flag of y-measuring fibers */

      Float_t trk1_x; /**< track position x at upstream tracker, unit="mm" */
      Float_t trk1_y; /**< track position y at upstream tracker, unit="mm" */
      Float_t trk1_z; /**< track position z at upstream tracker, unit="mm" */

      Float_t trk2_x; /**< track position x at downstream tracker, unit="mm" */
      Float_t trk2_y; /**< track position y at downstream tracker, unit="mm" */
      Float_t trk2_z; /**< track position z at downstream tracker, unit="mm" */

      Float_t trk_top_x; /**< track position x at top, unit="mm" */
      Float_t trk_top_y; /**< track position y at top, unit="mm" */
      Float_t trk_top_z; /**< track position z at top, unit="mm" */

      /**
       * Clear the structure: set elements to zero
       */
      void clear() {
        runNum = 0;
        eventNum = 0;
        eventflag = 0;
        eventtag_top = 0;
        nhit = 0;
        for (int i = 0; i < 4; i++) rf[i] = 0;
        rf_time = 0;
        bunchNum = 0;
        for (int i = 0; i < 2; i++) {
          trigS_tdc[i] = 0;
          trigS_adc[i] = 0;
          trigM_tdc[i] = 0;
          trigM_adc[i] = 0;
          veto_adc[i] = 0;
        }
        timing_tdc = 0;
        timing_adc = 0;
        ratemon = 0;
        for (int i = 0; i < 512; i++) {
          tdc0_ch[i] = 0;
          adc0_ch[i] = 0;
          pmtflag_ch[i] = 0;
          tdc_ch[i] = 0;
        }
        for (int i = 0; i < 2; i++) {
          trk_x[i] = 0;
          trk_z_x[i] = 0;
          trk_qual_x[i] = 0;
          trk_y[i] = 0;
          trk_z_y[i] = 0;
          trk_qual_y[i] = 0;
        }
        trk1_x = 0; trk1_y = 0; trk1_z = 0;
        trk2_x = 0; trk2_y = 0; trk2_z = 0;
        trk_top_x = 0; trk_top_y = 0; trk_top_z = 0;
      }

    };

  } // TOP namespace
} // Belle2 namespace

#endif

