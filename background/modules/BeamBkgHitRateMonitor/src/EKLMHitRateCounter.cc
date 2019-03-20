/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Kirill Chilikin                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <background/modules/BeamBkgHitRateMonitor/EKLMHitRateCounter.h>

using namespace Belle2::Background;

void EKLMHitRateCounter::initialize(TTree* tree)
{
  // register collection(s) as optional, your detector might be excluded in DAQ
  m_digits.isOptional();
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());

  // set branch address
  tree->Branch("eklm", &m_rates,
               "sectorRates[104]/F:averageRate/F:numEvents/I:valid/O");
}

void EKLMHitRateCounter::clear()
{
  m_buffer.clear();
}

void EKLMHitRateCounter::accumulate(unsigned timeStamp)
{
  // check if data are available
  if (not m_digits.isValid())
    return;

  // get buffer element
  auto& rates = m_buffer[timeStamp];

  // increment event counter
  rates.numEvents++;

  // accumulate hits
  int n = m_digits.getEntries();
  for (int i = 0; i < n; ++i) {
    EKLMDigit* eklmDigit = m_digits[i];
    if (!eklmDigit->isGood())
      continue;
    int sector = m_ElementNumbers->sectorNumber(
                   eklmDigit->getEndcap(), eklmDigit->getLayer(),
                   eklmDigit->getSector()) - 1;
    rates.sectorRates[sector] += 1;
    rates.averageRate += 1;
  }

  // set flag to true to indicate the rates are valid
  rates.valid = true;
}

void EKLMHitRateCounter::normalize(unsigned timeStamp)
{
  // copy buffer element
  m_rates = m_buffer[timeStamp];

  if (not m_rates.valid) return;

  // normalize
  m_rates.normalize();

  /* Normalize the hit rate per 1 strip. */
  int n = m_ElementNumbers->getMaximalSectorGlobalNumber();
  for (int i = 0; i < n; ++i) {
    m_rates.sectorRates[i] /= (m_ElementNumbers->getMaximalStripNumber() *
                               m_ElementNumbers->getMaximalPlaneNumber());
  }
}
