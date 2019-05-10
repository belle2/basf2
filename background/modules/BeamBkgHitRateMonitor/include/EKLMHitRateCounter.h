/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Kirill Chilikin                            *
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
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dataobjects/ElementNumbersSingleton.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <klm/dbobjects/KLMChannelStatus.h>

namespace Belle2 {

  namespace Background {

    /**
     * Class for monitoring beam background hit rates of EKLM.
     */
    class EKLMHitRateCounter: public HitRateBase {

    public:

      /**
       * Tree data structure.
       */
      struct TreeStruct {

        /** Hit rates in each sector. */
        float sectorRates[EKLMElementNumbers::getMaximalSectorGlobalNumber()] = {0};

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
          for (int i = 0; i < EKLMElementNumbers::getMaximalSectorGlobalNumber(); ++i)
            sectorRates[i] /= numEvents;
          averageRate /= numEvents;
        }

      };

      /**
       * Constructor.
       */
      EKLMHitRateCounter() {};

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

      /** EKLM digits. */
      StoreArray<EKLMDigit> m_digits;  /**< collection of digits */

      /** EKLM element numbers. */
      const EKLM::ElementNumbersSingleton* m_ElementNumbers = nullptr;

      /** KLM channel status. */
      DBObjPtr<KLMChannelStatus> m_ChannelStatus;

    };

  }

}
