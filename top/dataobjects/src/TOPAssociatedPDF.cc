
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dataobjects/TOPAssociatedPDF.h>
#include <TRandom.h>


namespace Belle2 {

  const TOPAssociatedPDF::PDFPeak* TOPAssociatedPDF::getSinglePeak() const
  {
    if (m_weights.empty()) return 0;

    float sum = m_BGWeight + m_deltaRayWeight;
    for (const auto& w : m_weights) sum += w;
    float prob = sum * gRandom->Rndm();
    float cumul = 0;
    for (size_t i = 0; i < m_weights.size(); i++) {
      cumul += m_weights[i];
      if (prob < cumul) return &m_peaks[i];
    }
    return 0;
  }

}
