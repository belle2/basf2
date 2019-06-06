/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Giacomo De Pietro                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* C++ headers. */
#include <map>

/* External headers. */
#include <TTree.h>

/* Belle2 headers. */
#include <background/modules/BeamBkgHitRateMonitor/HitRateBase.h>
#include <bklm/dataobjects/BKLMDigit.h>
#include <bklm/dataobjects/BKLMElementNumbers.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dbobjects/KLMChannelStatus.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  namespace Background {

    /**
     * Class for monitoring beam background hit rates of BKLM.
     */
    class BKLMHitRateCounter: public HitRateBase {

    public:

      /**
       * Tree data structure.
       */
      struct TreeStruct {

        /** Hit rates in each layer. */
        float layerRates[240] = {0};

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
          for (int i = 0; i < m_maxGlobalLayer; ++i)
            layerRates[i] /= numEvents;
          averageRate /= numEvents;
        }

      };

      /**
       * Constructor.
       */
      BKLMHitRateCounter() {};

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

      /**
       * Get number of active strips in the specified BKLM global layer.
       * @param[in] layerGlobal Layer global number.
       */
      int getActiveStripsBKLMLayer(int layerGlobal) const;

    private:

      /** Tree data. */
      TreeStruct m_rates;

      /** Buffer. */
      std::map<unsigned, TreeStruct> m_buffer;

      /** Total number of layers. */
      static constexpr int m_maxGlobalLayer = BKLMElementNumbers::getMaximalSectorGlobalNumber();

      /** BKLM digits. */
      StoreArray<BKLMDigit> m_digits;

      /** KLM channel status. */
      DBObjPtr<KLMChannelStatus> m_ChannelStatus;
    };

  }

}
