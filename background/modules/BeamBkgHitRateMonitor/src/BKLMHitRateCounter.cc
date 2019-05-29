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

void BKLMHitRateCounter::initialize(TTree* tree)
{
  // register collection(s) as optional, your detector might be excluded in DAQ
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
    //if (!digit.isGood())
    //  continue;
    int globalLayer = digit.getLayer() - 1;
    globalLayer += (digit.getSector() - 1) * BKLMElementNumbers::getMaximalLayerNumber();
    globalLayer += digit.isForward() * BKLMElementNumbers::getMaximalSectorNumber() * BKLMElementNumbers::getMaximalLayerNumber();
    rates.sectorRates[globalLayer] += 1;
    rates.averageRate += 1;
  }

  // set flag to true to indicate the rates are valid
  rates.valid = true;
}

void BKLMHitRateCounter::normalize(unsigned timeStamp)
{
  // copy buffer element
  m_rates = m_buffer[timeStamp];

  if (!m_rates.valid) return;

  // normalize
  m_rates.normalize();

  /* Normalize the hit rate per 1 strip. */
  for (int i = 0; i < m_maxGlobalLayer; ++i) {
    int activeStrips = getActiveStripsBKLMLayer(i);
    if (activeStrips == 0)
      m_rates.sectorRates[i] = 0;
    else {
      m_rates.sectorRates[i] /= activeStrips;
      if ((i % BKLMElementNumbers::getMaximalLayerNumber()) > 2) {
        // The layer is an RPC-layer: there are two digits per hit
        // so it's better to divide by 2 the rate
        m_rates.sectorRates[i] /= 2;
      }
    }
  }
}

int BKLMHitRateCounter::getActiveStripsBKLMLayer(int globalLayer) const
{
  int active = 0;

  int layer = (globalLayer % BKLMElementNumbers::getMaximalLayerNumber()) + 1;
  int sector = ((globalLayer / BKLMElementNumbers::getMaximalLayerNumber()) % BKLMElementNumbers::getMaximalSectorNumber()) + 1;
  int isForward = ((globalLayer / BKLMElementNumbers::getMaximalLayerNumber()) / BKLMElementNumbers::getMaximalSectorNumber()) %
                  (BKLMElementNumbers::getMaximalLayerNumber() + 1);

  for (int plane = 0; plane < BKLMElementNumbers::getMaximalPlaneNumber(); ++plane) {
    for (int strip = 1; strip <= BKLMElementNumbers::getNStrips(isForward, sector, layer, plane); ++strip) {
      const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
      uint16_t channel = elementNumbers->channelNumberBKLM(isForward, layer, sector, plane, strip);
      enum KLMChannelStatus::ChannelStatus status = m_ChannelStatus->getChannelStatus(channel);
      if (status == KLMChannelStatus::c_Unknown)
        B2FATAL("Incomplete KLM channel status data.");
      if (status != KLMChannelStatus::c_Dead)
        active++;
    }
  }
  return active;
}
