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
#include <background/modules/BeamBkgHitRateMonitor/TOPHitRateCounter.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {
  namespace Background {

    void TOPHitRateCounter::initialize(TTree* tree)
    {
      // register collection(s) as optional, your detector might be excluded in DAQ
      m_digits.isOptional();

      // set branch address
      tree->Branch("top", &m_rates, "slotRates[16]/F:totalRate/F:valid/O");

    }

    void TOPHitRateCounter::clear()
    {
      m_rates.clear();
    }

    void TOPHitRateCounter::accumulate()
    {
      // check if data are available
      if (not m_digits.isValid()) return;

      // accumulate hits
      for (const auto& digit : m_digits) {
        if (digit.getHitQuality() != TOPDigit::c_Good) continue;
        m_rates.slotRates[digit.getModuleID() - 1] += 1;
        m_rates.totalRate += 1;
      }

      // set flag to true to indicate the rates are valid
      m_rates.valid = true;

    }

    void TOPHitRateCounter::normalize()
    {
    }


  } // Background namespace
} // Belle2 namespace

