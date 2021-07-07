/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Belle2 headers. */
#include <background/modules/BeamBkgHitRateMonitor/KLMHitRateCounter.h>

using namespace Belle2::Background;

void KLMHitRateCounter::initialize(TTree* tree)
{
  // register collection(s) as optional, your detector might be excluded in DAQ
  m_digits.isOptional();
  m_ElementNumbers = &(KLMElementNumbers::Instance());
  m_ModuleArrayIndex = &(KLMModuleArrayIndex::Instance());

  // set branch address
  std::string branches;
  branches =
    "moduleRates[" +
    std::to_string(KLMElementNumbers::getTotalModuleNumber()) +
    "]/F:averageRate/F:numEvents/I:valid/O";
  tree->Branch("klm", &m_rates, branches.c_str());
}

void KLMHitRateCounter::clear()
{
  m_buffer.clear();
}

void KLMHitRateCounter::accumulate(unsigned timeStamp)
{
  // check if data are available
  if (not m_digits.isValid())
    return;

  // get buffer element
  auto& rates = m_buffer[timeStamp];

  // increment event counter
  rates.numEvents++;

  // accumulate hits
  for (const KLMDigit& klmDigit : m_digits) {
    if (!klmDigit.inRPC() && !klmDigit.isGood())
      continue;
    uint16_t moduleNumber =
      m_ElementNumbers->moduleNumber(
        klmDigit.getSubdetector(), klmDigit.getSection(), klmDigit.getSector(),
        klmDigit.getLayer());
    int module = m_ModuleArrayIndex->getIndex(moduleNumber);
    if (module >= 0 and module < KLMElementNumbers::getTotalModuleNumber()) {
      rates.moduleRates[module] += 1;
    } else {
      B2ERROR("KLMHitRateCounter: module number out of range"
              << LogVar("module", module));
    }
    rates.averageRate += 1;
  }

  // set flag to true to indicate the rates are valid
  rates.valid = true;
}

void KLMHitRateCounter::normalize(unsigned timeStamp)
{
  // copy buffer element
  m_rates = m_buffer[timeStamp];

  if (not m_rates.valid) return;

  // normalize
  m_rates.normalize();

  /* Normalize the hit rate per 1 strip. */
  for (int i = 0; i < KLMElementNumbers::getTotalModuleNumber(); ++i) {
    uint16_t module = m_ModuleArrayIndex->getNumber(i);
    int activeStrips = m_ChannelStatus->getActiveStripsInModule(module);
    if (activeStrips == 0)
      m_rates.moduleRates[i] = 0;
    else
      m_rates.moduleRates[i] /= activeStrips;
  }
}
