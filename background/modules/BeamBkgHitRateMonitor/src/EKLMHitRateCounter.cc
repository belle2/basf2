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
#include <background/modules/BeamBkgHitRateMonitor/EKLMHitRateCounter.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {
  namespace Background {

    void EKLMHitRateCounter::initialize(TTree* tree)
    {
      // register collection(s) as optional, your detector might be excluded in DAQ
      m_digits.isOptional();

      // set branch address
      tree->Branch("eklm", &m_rates, "totalRate/F:valid/O");

    }

    void EKLMHitRateCounter::clear()
    {
      m_rates.clear();
    }

    void EKLMHitRateCounter::accumulate()
    {
      // check if data are available
      if (not m_digits.isValid()) return;

      // accumulate hits
      /* either count all */
      m_rates.totalRate = m_digits.getEntries();
      /* or count selected ones only
      for(const auto& digit: m_digits) {
      // select digits to count here first, usualy count good ones only
      m_rates.totalRate += 1;
      }
      */

      // set flag to true to indicate the rates are valid
      m_rates.valid = true;

    }

    void EKLMHitRateCounter::normalize()
    {
    }


  } // Background namespace
} // Belle2 namespace

