/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgHitRateMonitor/ARICHHitRateCounter.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {
  namespace Background {

    void ARICHHitRateCounter::initialize(TTree* tree)
    {
      // register collection(s) as optional, your detector might be excluded in DAQ
      m_hits.isOptional();

      // set branch address
      tree->Branch("arich", &m_rates, "hapdRates[420]/F:averageRate/F:numEvents/I:valid/O");

      // set fractions of active channels
      setActiveFractions();
    }

    void ARICHHitRateCounter::clear()
    {
      m_buffer.clear();
    }

    void ARICHHitRateCounter::accumulate(unsigned timeStamp)
    {
      // check if data are available
      if (not m_hits.isValid()) return;

      // get buffer element
      auto& rates = m_buffer[timeStamp];

      // increment event counter
      rates.numEvents++;

      // count and weight hits accoring to channel efficiecny
      for (const auto& hit : m_hits) {
        auto effi = m_modulesInfo->getChannelQE(hit.getModule(), hit.getChannel());
        float wt = std::min(1.0 / effi, 100.);
        rates.hapdRates[hit.getModule() - 1] += wt;
        rates.averageRate += wt;
      }

      // set flag to true to indicate the rates are valid
      rates.valid = true;

    }

    void ARICHHitRateCounter::normalize(unsigned timeStamp)
    {
      // copy buffer element
      m_rates = m_buffer[timeStamp];

      if (not m_rates.valid) return;

      // normalize
      m_rates.normalize();

      // correct rates for masked-out channels
      if (m_channelMask.hasChanged()) setActiveFractions();

      for (int imod = 0; imod < 420; imod++) {
        double fraction = m_activeFractions[imod];
        if (fraction > 0) m_rates.hapdRates[imod] /= fraction;
        else m_rates.hapdRates[imod] = 0;
      }
      m_rates.averageRate /= m_activeTotal;
    }

    void ARICHHitRateCounter::setActiveFractions()
    {

      if (not m_channelMask.isValid()) {
        for (auto& fraction : m_activeFractions) fraction = 1;
        m_activeTotal = 1;
        B2WARNING("ARICHHitRateCounter: no valid channel mask - active fractions set to 1");
        return;
      }

      int nactiveTotal = 0;
      for (unsigned imod = 1; imod < 421; imod++) {
        int nactive = 0;
        for (unsigned ichn = 0; ichn < 144; ichn++) {
          if (m_channelMask->isActive(imod, ichn)) nactive++;
        }
        nactiveTotal += nactive;
        m_activeFractions[imod - 1] = (float)nactive / 144.;
      }
      m_activeTotal = (float)nactiveTotal / 144. / 420.;
    }

  } // Background namespace
} // Belle2 namespace

