/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dataobjects/TOPSimHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <framework/datastore/StoreArray.h>


#include <map>
#include <vector>

namespace Belle2 {
  namespace TOP {
    /**
     * Time digitization of simulated hits in a single electronic channel
     */
    class TimeDigitizer {

    public:

      /**
       * constructor
       */
      TimeDigitizer(int barID, int channelID): m_barID(barID), m_channelID(channelID)
      {}

      /**
       * add time of simulated hit
       * @param t time of simulated hit
       * @param simHit pointer to simulated hit or NULL
       */
      void addTimeOfHit(double t, const TOPSimHit* simHit = NULL)
      {
        m_times.insert(std::pair<double, const TOPSimHit*>(t, simHit));
      }

      /**
       * Get bar ID
       * @return bar ID
       */
      int getBarID() const { return m_barID; }

      /**
       * Get channel ID
       * @return software channel ID
       */
      int getChannelID() const { return m_channelID; }

      /**
       *
       */
      unsigned getUniqueID() const {return m_channelID + (m_barID << 16);}

      /**
       * Do time digitization using simplified pile-up and double-hit-resolution model.
       * As a result, the digitized hits are appended to TOPDigits and the relations to
       * TOPSimHits and MCParticles are set with proper weights.
       * @param digits a reference to TOPDigits
       * @param sigma a r.m.s. of an additional time jitter due to electronics
       */
      void digitize(StoreArray<TOPDigit>& digits, double sigma = 0.0);

    private:

      int m_barID;               /**< bar ID (1-based) */
      int m_channelID;           /**< software channel ID (1-based) */
      std::multimap<double, const TOPSimHit*> m_times; /**< sorted hit times */

    };

  } // TOP namespace
} // Belle2 namespace

