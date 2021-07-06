/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

        float segmentRates[18] = {0}; /**< hit rates per HAPD [Hz] for 18 segments of arich */
        float averageRate = 0; /**< total detector average hit rate per HAPD [Hz] */
        int numEvents = 0; /**< number of events accumulated */
        bool valid = false;  /**< status: true = rates valid */

        /**
         * normalize accumulated hits to single event
         */
        void normalize()
        {
          if (numEvents == 0) return;
          for (auto& segmentRate : segmentRates) segmentRate /= numEvents;
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
       * Sets number of active hapds in each segment
       */
      void setActiveHapds();

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
      double m_activeHapds[18] = {0}; /**< number of active HAPDS in each segment */
      double m_activeTotal = 0; /**< total number of active HAPDS */
      int  m_segmentMap[420] = {0}; /**< mapping from module ID to segments */
    };

  } // Background namespace
} // Belle2 namespace
