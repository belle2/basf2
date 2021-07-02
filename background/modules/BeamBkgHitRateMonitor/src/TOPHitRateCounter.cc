/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgHitRateMonitor/TOPHitRateCounter.h>
#include <top/geometry/TOPGeometryPar.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {
  namespace Background {

    void TOPHitRateCounter::initialize(TTree* tree)
    {
      // register collection(s) as optional, your detector might be excluded in DAQ
      m_digits.isOptional();

      // set branch address
      tree->Branch("top", &m_rates, "slotRates[16]/F:averageRate/F:numEvents/I:valid/O");

      // create control histograms
      m_hits = new TH1F("top_hits", "time distribution of hits; digit.time [ns]",
                        1000, -500, 500);
      m_hitsInWindow = new TH1F("top_hitsInWindow",
                                "time distribution of hits; digit.time [ns]",
                                100, m_timeOffset - m_timeWindow / 2,
                                m_timeOffset + m_timeWindow / 2);

      // check parameters
      if (m_timeWindow <= 0) B2FATAL("invalid time window for TOP: " << m_timeWindow);

      // set fractions of active channels
      setActiveFractions();
    }

    void TOPHitRateCounter::clear()
    {
      m_buffer.clear();
    }

    void TOPHitRateCounter::accumulate(unsigned timeStamp)
    {
      // check if data are available
      if (not m_digits.isValid()) return;

      // get buffer element
      auto& rates = m_buffer[timeStamp];

      // increment event counter
      rates.numEvents++;

      // accumulate hits (weighted by efficiency correction)
      const auto* topgp = TOP::TOPGeometryPar::Instance();
      for (const auto& digit : m_digits) {
        if (digit.getHitQuality() != TOPDigit::c_Good) continue;
        m_hits->Fill(digit.getTime());

        if (fabs(digit.getTime() - m_timeOffset) > m_timeWindow / 2) continue;
        m_hitsInWindow->Fill(digit.getTime());

        auto effi = topgp->getRelativePixelEfficiency(digit.getModuleID(),
                                                      digit.getPixelID());
        float wt = std::min(1.0 / effi, 10.0);
        rates.slotRates[digit.getModuleID() - 1] += wt;
        rates.averageRate += wt;
      }

      // set flag to true to indicate the rates are valid
      rates.valid = true;

    }

    void TOPHitRateCounter::normalize(unsigned timeStamp)
    {
      // copy buffer element
      m_rates = m_buffer[timeStamp];

      if (not m_rates.valid) return;

      // normalize
      m_rates.normalize();

      // convert rates to [MHz/PMT]
      for (auto& slotRate : m_rates.slotRates) {
        slotRate /= m_timeWindow / Unit::us * 32; // 32 PMT's per slot
      }
      m_rates.averageRate /= m_timeWindow / Unit::us * 32 * 16; // 16 slots

      // check if DB object has changed and if so set fractions of active channels
      if (m_channelMask.hasChanged()) setActiveFractions();

      // correct rates for masked-out channels
      for (int m = 0; m < 16; m++) {
        double fraction = m_activeFractions[m];
        if (fraction > 0) {
          m_rates.slotRates[m] /= fraction;
        } else {
          m_rates.slotRates[m] = 0;
        }
      }
      m_rates.averageRate /= m_activeTotal; // we don't expect full TOP to be masked-out

    }

    void TOPHitRateCounter::setActiveFractions()
    {
      if (not m_channelMask.isValid()) {
        for (auto& fraction : m_activeFractions) fraction = 1;
        m_activeTotal = 1;
        B2WARNING("TOPHitRateCounter: no valid channel mask - active fractions set to 1");
        return;
      }

      for (int m = 0; m < 16; m++) {
        m_activeFractions[m] = m_channelMask->getActiveFraction(m + 1);
      }
      m_activeTotal = m_channelMask->getActiveFraction();

    }


  } // Background namespace
} // Belle2 namespace

