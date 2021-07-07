/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgHitRateMonitor/ARICHHitRateCounter.h>

// framework aux
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {
  namespace Background {

    void ARICHHitRateCounter::initialize(TTree* tree)
    {
      // register collection(s) as optional, your detector might be excluded in DAQ
      m_hits.isOptional();

      // set branch address
      tree->Branch("arich", &m_rates, "segmentRates[18]/F:averageRate/F:numEvents/I:valid/O");

      // make map of modules to 18 segments
      // (first hapd ring is 6 segments (by sector), for the rest, each segment merges 3 hapd rings (again by sector))
      int nModInRing[7] = {0, 42, 90, 144, 204, 270, 342};
      int iRing = 0;
      for (int i = 0; i < 420; i++) {
        if (i == nModInRing[iRing + 1]) iRing++;
        int segment = (i - nModInRing[iRing]) / (7 + iRing);
        if (iRing > 0) segment += 6;
        if (iRing > 3) segment += 6;
        m_segmentMap[i] = segment;
      }

      // set fractions of active channels
      setActiveHapds();
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
        if (hit.getModule() < 1 || hit.getModule() > 420) continue;
        auto effi = m_modulesInfo->getChannelQE(hit.getModule(), hit.getChannel());
        float wt = std::min(1.0 / effi, 100.);
        rates.segmentRates[m_segmentMap[hit.getModule() - 1]] += wt;
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
      if (m_channelMask.hasChanged()) setActiveHapds();

      for (int iSegment = 0; iSegment < 18; iSegment++) {
        double nHapds = m_activeHapds[iSegment];
        if (nHapds > 0) m_rates.segmentRates[iSegment] /= nHapds;
        else m_rates.segmentRates[iSegment] = 0;
      }
      m_rates.averageRate /= m_activeTotal;
    }

    void ARICHHitRateCounter::setActiveHapds()
    {
      for (auto& nactive : m_activeHapds) nactive = 0;

      if (not m_channelMask.isValid()) {
        for (unsigned imod = 1; imod < 421; imod++)  m_activeHapds[m_segmentMap[imod - 1]] += 1.;
        m_activeTotal = 420;
        B2WARNING("ARICHHitRateCounter: no valid channel mask - all HAPDs set to active");
        return;
      }

      int nactiveTotal = 0;
      for (unsigned imod = 1; imod < 421; imod++) {
        int nactive = 0;
        for (unsigned ichn = 0; ichn < 144; ichn++) {
          if (m_channelMask->isActive(imod, ichn)) nactive++;
        }
        nactiveTotal += nactive;
        m_activeHapds[m_segmentMap[imod - 1]] += (float)nactive / 144.;
      }
      m_activeTotal = (float)nactiveTotal / 144.;
    }

  } // Background namespace
} // Belle2 namespace

