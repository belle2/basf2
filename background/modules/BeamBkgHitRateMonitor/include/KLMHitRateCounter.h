/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* C++ headers. */
#include <map>

/* External headers. */
#include <TTree.h>

/* Belle2 headers. */
#include <background/modules/BeamBkgHitRateMonitor/HitRateBase.h>
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMModuleArrayIndex.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <klm/dbobjects/KLMChannelStatus.h>

namespace Belle2 {

  namespace Background {

    /**
     * Class for monitoring beam background hit rates of EKLM.
     */
    class KLMHitRateCounter: public HitRateBase {

    public:

      /**
       * Tree data structure.
       */
      struct TreeStruct {

        /** Hit rates in each module. */
        float moduleRates[KLMElementNumbers::getTotalModuleNumber()] = {0};

        /** Total detector average hit rate. */
        float averageRate = 0;

        /** Number of accumulated events. */
        int numEvents = 0;

        /** Whether the rates are valid. */
        bool valid = false;

        /**
         * Normalize accumulated hits to single event.
         */
        void normalize()
        {
          if (numEvents == 0)
            return;
          for (int i = 0; i < KLMElementNumbers::getTotalModuleNumber(); ++i)
            moduleRates[i] /= numEvents;
          averageRate /= numEvents;
        }

      };

      /**
       * Constructor.
       */
      KLMHitRateCounter() {};

      /**
       * Class initializer.
       * @param[in,out] tree Data tree.
       */
      virtual void initialize(TTree* tree) override;

      /**
       * Clear time-stamp buffer to prepare for 'accumulate'.
       */
      virtual void clear() override;

      /**
       * Accumulate hits.
       * @param[in] timeStamp Time stamp.
       */
      virtual void accumulate(unsigned timeStamp) override;

      /**
       * Normalize accumulated hits (i.e. transform to rates).
       * @param[in] timeStamp Time stamp.
       */
      virtual void normalize(unsigned timeStamp) override;

    private:

      /** Tree data. */
      TreeStruct m_rates;

      /** Buffer. */
      std::map<unsigned, TreeStruct> m_buffer;

      /** KLM digits. */
      StoreArray<KLMDigit> m_digits;

      /** KLM element numbers. */
      const KLMElementNumbers* m_ElementNumbers = nullptr;

      /** KLM module array index. */
      const KLMModuleArrayIndex* m_ModuleArrayIndex = nullptr;

      /** KLM channel status. */
      DBObjPtr<KLMChannelStatus> m_ChannelStatus;

    };

  }

}
