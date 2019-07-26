/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Giacomo De Pietro                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <background/modules/BeamBkgHitRateMonitor/BKLMHitRateCounter.h>

using namespace Belle2::Background;

BKLMHitRateCounter::BKLMHitRateCounter()
{
  m_klmElementNumbers = &(KLMElementNumbers::Instance());
}

void BKLMHitRateCounter::initialize(TTree* tree)
{
  // register collection(s) as optional: BKLM might be excluded in DAQ
  m_digits.isOptional();

  // set branch address
  std::string branches;
  branches =
    "sectorRates[" +
    std::to_string(static_cast<int>(m_maxGlobalLayer)) +
    "]/F:averageRate/F:numEvents/I:valid/O";
  tree->Branch("bklm", &m_rates, branches.c_str());
}

void BKLMHitRateCounter::clear()
{
  m_buffer.clear();
}

void BKLMHitRateCounter::accumulate(unsigned timeStamp)
{
  // check if data are available
  if (!m_digits.isValid())
    return;

  // get buffer element
  auto& rates = m_buffer[timeStamp];

  // increment event counter
  rates.numEvents++;

  // accumulate hits
  for (const BKLMDigit& digit : m_digits) {
    // Discard scintillators hits below the theshdold
    if (!digit.inRPC() && !digit.isAboveThreshold())
      continue;

    int layerGlobal = BKLMElementNumbers::layerGlobalNumber(digit.getForward(), digit.getSector(), digit.getLayer());
    rates.layerRates[layerGlobal]++;
    rates.averageRate++;
  }

  // set flag to true to indicate the rates are valid
  rates.valid = true;
}

void BKLMHitRateCounter::normalize(unsigned timeStamp)
{
  // copy buffer element
  m_rates = m_buffer[timeStamp];

  if (!m_rates.valid)
    return;

  // normalize
  m_rates.normalize();

  // Normalize the hit rate per one strip.
  for (int layerGlobal = 0; layerGlobal < m_maxGlobalLayer; ++layerGlobal) {
    int activeStrips = getActiveStripsBKLMLayer(layerGlobal);
    if (activeStrips == 0)
      m_rates.layerRates[layerGlobal] = 0;
    else {
      m_rates.layerRates[layerGlobal] /= activeStrips;
      if ((layerGlobal % BKLMElementNumbers::getMaximalLayerNumber()) >= 2) {
        // The layer is an RPC-layer: there are two digits per "real" hit
        // so it's better to divide by 2 the rate
        m_rates.layerRates[layerGlobal] /= 2;
      }
    }
  }
}

int BKLMHitRateCounter::getActiveStripsBKLMLayer(int layerGlobal) const
{
  int active = 0;

  int forward, sector, layer;
  BKLMElementNumbers::layerGlobalNumberToElementNumbers(layerGlobal, &forward, &sector, &layer);

  for (int plane = 0; plane < BKLMElementNumbers::getMaximalPlaneNumber(); ++plane) {
    for (int strip = 1; strip <= BKLMElementNumbers::getNStrips(forward, sector, layer, plane); ++strip) {
      uint16_t channel = m_klmElementNumbers->channelNumberBKLM(forward, sector, layer, plane, strip);
      enum KLMChannelStatus::ChannelStatus status = m_ChannelStatus->getChannelStatus(channel);

      // Ignore the unknown and dead channels
      if (status == KLMChannelStatus::c_Unknown)
        B2FATAL("Incomplete KLM channel status data.");
      if (status != KLMChannelStatus::c_Dead)
        active++;
    }
  }
  return active;
}
