/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPPmtQE.h>
#include <framework/logging/Logger.h>
#include <algorithm>

using namespace std;

namespace Belle2 {

  const std::vector<float>& TOPPmtQE::getQE(unsigned pmtPixel) const
  {
    pmtPixel--;
    if (pmtPixel < c_NumPmtPixels) return m_QE[pmtPixel];

    B2ERROR("TOPPmtQE::getQE: invalid pixel ID, returning empty vector");
    m_envelopeQE.clear();
    return m_envelopeQE;
  }

  double TOPPmtQE::getQE(unsigned pmtPixel, double lambda) const
  {
    pmtPixel--;
    if (pmtPixel < c_NumPmtPixels) return interpolate(lambda, m_QE[pmtPixel]);
    return 0;
  }

  const std::vector<float>& TOPPmtQE::getEnvelopeQE() const
  {
    setEnvelopeQE();
    return m_envelopeQE;
  }

  double TOPPmtQE::getEnvelopeQE(double lambda) const
  {
    setEnvelopeQE();
    return interpolate(lambda, m_envelopeQE);
  }

  double TOPPmtQE::getEfficiency(unsigned pmtPixel, double lambda, bool BfieldOn) const
  {
    if (BfieldOn) {
      return getQE(pmtPixel, lambda) * m_CE_withB;
    } else {
      return getQE(pmtPixel, lambda) * m_CE_noB;
    }
  }

  double TOPPmtQE::getCE(bool BfieldOn) const
  {
    if (BfieldOn) {
      return m_CE_withB;
    } else {
      return m_CE_noB;
    }
  }

  double TOPPmtQE::getLambdaLast() const
  {
    size_t size = 0;
    for (unsigned pixel = 0; pixel < c_NumPmtPixels; pixel++) {
      const auto& QE = m_QE[pixel];
      size = std::max(size, QE.size());
    }
    return m_lambdaFirst + (size - 1) * m_lambdaStep;
  }

  double TOPPmtQE::getLambdaLast(unsigned pmtPixel) const
  {
    pmtPixel--;
    if (pmtPixel < c_NumPmtPixels) {
      return m_lambdaFirst + (m_QE[pmtPixel].size() - 1) * m_lambdaStep;
    }
    return 0;
  }

  void TOPPmtQE::setEnvelopeQE() const
  {
    if (!m_envelopeQE.empty()) return;

    size_t size = 0;
    for (unsigned pixel = 0; pixel < c_NumPmtPixels; pixel++) {
      const auto& QE = m_QE[pixel];
      size = std::max(size, QE.size());
    }
    m_envelopeQE.resize(size, 0);
    for (unsigned pixel = 0; pixel < c_NumPmtPixels; pixel++) {
      const auto& QE = m_QE[pixel];
      for (size_t i = 0; i < QE.size(); i++) {
        m_envelopeQE[i] = std::max(m_envelopeQE[i], QE[i]);
      }
    }
  }

  double TOPPmtQE::interpolate(double lambda, const std::vector<float>& QE) const
  {
    double dlam = lambda - m_lambdaFirst;
    if (dlam < 0 or dlam > (QE.size() - 1) * m_lambdaStep) return 0;
    unsigned i = int(dlam / m_lambdaStep);
    if (i > QE.size() - 2) return QE.back();
    return QE[i] + (QE[i + 1] - QE[i]) / m_lambdaStep * (dlam - i * m_lambdaStep);
  }


} // end Belle2 namespace

