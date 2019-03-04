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
#include <ecl/dataobjects/ECLDigit.h>
#include <TTree.h>


namespace Belle2 {
  namespace Background {

    /**
     * Class for monitoring beam background hit rates of ECL
     */
    class ECLHitRateCounter: public HitRateBase {

    public:

      /**
       * tree structure
       */
      struct TreeStruct {

        float totalRate = 0; /**< total detector hit rate */
        bool valid = false;  /**< status: true = rates valid, false on clear() */

        /**
         * clear variables
         */
        void clear()
        {
          totalRate = 0;
          valid = false;
        }

      };

      /**
       * Constructor
       */
      ECLHitRateCounter()
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
      StoreArray<ECLDigit> m_digits;  /**< collection of digits */

      // DB payloads

      // other

    };

  } // Background namespace
} // Belle2 namespace
