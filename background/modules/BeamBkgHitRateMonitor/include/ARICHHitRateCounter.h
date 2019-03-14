/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <background/modules/BeamBkgHitRateMonitor/HitRateBase.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>
#include <arich/dataobjects/ARICHHit.h>
#include <arich/dbobjects/ARICHChannelMask.h>
#include <arich/dbobjects/ARICHModulesInfo.h>
#include <TTree.h>
#include <map>


namespace Belle2 {
  namespace Background {

    /**
     * Class for monitoring beam background hit rates of ARICH
     */
    class ARICHHitRateCounter: public HitRateBase {

    public:

      /**
       * tree structure
       */
      struct TreeStruct {

        float hapdRates[420] = {0}; /**< hit rates per HAPD [Hz] */
        float averageRate = 0; /**< total detector average hit rate */
        int numEvents = 0; /**< number of events accumulated */
        bool valid = false;  /**< status: true = rates valid */

        /**
         * normalize accumulated hits to single event
         */
        void normalize()
        {
          if (numEvents == 0) return;
          for (auto& hapdRate : hapdRates) hapdRate /= numEvents;
          averageRate /= numEvents;
        }

      };

      /**
       * Constructor
       */
      ARICHHitRateCounter()
      {}

      /**
       * Class initializer: set branch addresses and other staf
       * @param tree a valid TTree pointer
       */
      virtual void initialize(TTree* tree) override;

      /**
       * Clear time-stamp buffer to prepare for 'accumulate'
       */
      virtual void clear() override;

      /**
       * Accumulate hits
       * @param timeStamp time stamp
       */
      virtual void accumulate(unsigned timeStamp) override;

      /**
       * Normalize accumulated hits (e.g. transform to rates)
       * @param timeStamp time stamp
       */
      virtual void normalize(unsigned timeStamp) override;

    private:

      /**
       * Sets fractions of active channels
       */
      void setActiveFractions();

      // tree structure
      TreeStruct m_rates; /**< tree variables */

      // buffer
      std::map<unsigned, TreeStruct> m_buffer; /**< average rates in time stamps */

      // collections
      StoreArray<ARICHHit> m_hits;  /**< collection of digits */

      // DB payloads
      DBObjPtr<ARICHChannelMask> m_channelMask; /**< channel mask */
      DBObjPtr<ARICHModulesInfo> m_modulesInfo; /**< HAPD modules info */

      // other
      double m_activeFractions[420] = {0}; /**< fractions of active channels in HAPDs */
      double m_activeTotal = 0; /**< total fraction of active channels */
    };

  } // Background namespace
} // Belle2 namespace
