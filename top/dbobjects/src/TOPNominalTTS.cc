/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPNominalTTS.h>
#include <framework/logging/Logger.h>
#include <iostream>
#include <TRandom.h>

using namespace std;

namespace Belle2 {

  void TOPNominalTTS::appendGaussian(double norm, double mean, double sigma)
  {
    if (norm <= 0) {
      B2ERROR("TOPNominalTTS::appendGaussian: normalization is non-positive number");
      return;
    }
    Gauss gauss;
    gauss.fraction = norm;
    gauss.position = mean;
    gauss.sigma = sigma;
    m_tts.push_back(gauss);
    m_normalized = false;
  }

  double TOPNominalTTS::normalize()
  {
    float sum = 0;
    for (const auto& tts : m_tts) sum += tts.fraction;
    if (sum == 0) return 0;
    for (auto& tts : m_tts) tts.fraction /= sum;
    m_normalized = true;
    return sum;
  }

  double TOPNominalTTS::generateTTS() const
  {
    double prob = gRandom->Rndm();
    double s = 0;
    for (const auto& tts : m_tts) {
      s = s + tts.fraction;
      if (prob < s) {
        return gRandom->Gaus(tts.position, tts.sigma);
      }
    }
    return 0;
  }


  bool TOPNominalTTS::isConsistent() const
  {
    if (m_tts.empty()) return false;
    if (!m_normalized) return false;
    return true;
  }


  void TOPNominalTTS::print(const std::string& title) const
  {
    TOPGeoBase::printUnderlined(title);
    if (m_tts.empty()) {
      cout << " -> vector is empty!" << endl;
      return;
    }
    cout << " PMT type: " << m_type << endl;

    int i = 0;
    for (const auto& tts : m_tts) {
      i++;
      cout << " Gaussian " << i << ": ";
      cout << tts.fraction << ", ";
      cout << tts.position << " ns, ";
      cout << tts.sigma << " ns" << endl;
    }
    if (!m_normalized) cout << " warning: fractions not normalized" << endl;
  }

} // end Belle2 namespace
