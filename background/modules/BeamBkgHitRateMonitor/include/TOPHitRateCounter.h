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
#include <top/dataobjects/TOPDigit.h>
#include <TTree.h>


namespace Belle2 {
  namespace Background {

    /**
     * Class for monitoring beam background hit rates of TOP
     */
    class TOPHitRateCounter: public HitRateBase {

    public:

      /**
       * tree structure
       */
      struct TreeStruct {
        float slotRates[16] = {0}; /**< hit rates of the modules (slots) */
        float totalRate = 0; /**< total detector hit rate */
        bool valid = false;  /**< status: true = rates valid, false on clear() */

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
       */
      TOPHitRateCounter()
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

      // class parameters: to be set via constructor or setters

      // tree structure
      TreeStruct m_rates; /**< tree variables */

      // collections
      StoreArray<TOPDigit> m_digits;  /**< collection of digits */

      // DB payloads

      // other

    };

  } // Background namespace
} // Belle2 namespace
