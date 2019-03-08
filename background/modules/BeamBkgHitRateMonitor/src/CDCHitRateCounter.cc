/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgHitRateMonitor/CDCHitRateCounter.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {
  namespace Background {

    void CDCHitRateCounter::initialize(TTree* tree)
    {
      // register collection(s) as optional, your detector might be excluded in DAQ
      m_digits.isOptional();

      // set branch address
      tree->Branch("cdc", &m_rates, "averageRate/F:numEvents/I:valid/O");

    }

    void CDCHitRateCounter::accumulate(unsigned timeStamp)
    {
      // check if data are available
      if (not m_digits.isValid()) return;

      // get buffer element
      auto& rates = m_buffer[timeStamp];

      // increment event counter
      rates.numEvents++;

      // accumulate hits
      /* either count all */
      rates.averageRate = m_digits.getEntries();
      /* or count selected ones only
      for(const auto& digit: m_digits) {
      // select digits to count (usualy only good ones)
         rates.averageRate += 1;
      }
      */

      // set flag to true to indicate the rates are valid
      rates.valid = true;

    }

    void CDCHitRateCounter::normalize(unsigned timeStamp)
    {
      // copy buffer element
      m_rates = m_buffer[timeStamp];

      if (not m_rates.valid) return;

      // normalize
      m_rates.normalize();

      // optionally: convert to MHz, correct for the masked-out channels etc.

    }


  } // Background namespace
} // Belle2 namespace

