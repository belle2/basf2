
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
    float cum = 0;
    for (size_t i = 0; i < m_weights.size(); i++) {
      cum += m_weights[i];
      if (prob < cum) return &m_peaks[i];
    }
    return 0;
  }

}
