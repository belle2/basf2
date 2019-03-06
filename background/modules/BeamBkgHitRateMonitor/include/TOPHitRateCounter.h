/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <background/modules/BeamBkgHitRateMonitor/HitRateBase.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dbobjects/TOPCalChannelMask.h>
#include <TTree.h>
#include <TH1F.h>


namespace Belle2 {
  namespace Background {

    /**
     * Class for monitoring beam background hit rates of TOP.
     *
     * Hit rates are given in MHz per PMT and are already corrected for different pixel
     * efficiencies and masked-out pixels.
     */
    class TOPHitRateCounter: public HitRateBase {

    public:

      /**
       * tree structure
       */
      struct TreeStruct {
        float slotRates[16] = {0}; /**< hit rates per PMT of the modules (slots) [MHz] */
        float totalRate = 0; /**< average hit rate per PMT [MHz] */
        bool valid = false;  /**< status: true = rates valid, set to false on clear() */

        /**
         * clear variables
         */
        void clear()
        {
          for (auto& slotRate : slotRates) slotRate = 0;
          totalRate = 0;
          valid = false;
        }

      };

      /**
       * Constructor
       * @param timeOffset time offset to be subtracted from hit times [ns]
       * @param timeWindow time window (full size) in which to count hits [ns]
       */
      TOPHitRateCounter(double timeOffset, double timeWindow):
        m_timeOffset(timeOffset), m_timeWindow(timeWindow)
      {}

      /**
       * Class initializer: set branch addresses and other staf
       * @param tree a valid TTree pointer
       */
      virtual void initialize(TTree* tree) override;

      /**
       * Clear the tree structure and other relevant variables to prepare for 'accumulate'
       */
      virtual void clear()  override;

      /**
       * Accumulate hits
       */
      virtual void accumulate()  override;

      /**
       * Normalize accumulated hits (e.g. transform to rates)
       */
      virtual void normalize()  override;

    private:

      /**
       * Sets fractions of active channels
       */
      void setActiveFractions();

      // class parameters: to be set via constructor or setters
      double m_timeOffset = 0; /**< time offset of hits [ns] */
      double m_timeWindow = 0; /**< time window in which to count hits [ns] */

      // tree structure
      TreeStruct m_rates; /**< tree variables */

      // collections
      StoreArray<TOPDigit> m_digits;  /**< collection of digits */

      // DB payloads
      DBObjPtr<TOPCalChannelMask> m_channelMask; /**< channel mask */

      // control histograms: to check if time offset and window size are correctly set
      TH1F* m_hits = 0; /**< time distribution of hits in wide range */
      TH1F* m_hitsInWindow = 0; /**< time distribution of hits inside timeWindow */

      // other
      double m_activeFractions[16] = {0}; /**< fractions of active channels in slots */
      double m_activeTotal = 0; /**< total fraction of active channels */

    };

  } // Background namespace
} // Belle2 namespace
