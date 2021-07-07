/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <framework/logging/Logger.h>
#include <vector>

#include <TRandom.h>

namespace Belle2 {

  /**
   * Parameterized TTS for each PMT pixel
   */
  class TOPPmtTTSPar : public TObject {

  public:
    /**
     * Gaussian distribution parameters
     */
    struct Gaussian {
      float fraction = 0; /**< area normalization */
      float mean = 0;     /**< peak position [ns] */
      float sigma = 0;    /**< peak width [ns] */
    };

    /**
     * number of PMT pixels
     */
    enum {c_NumPmtPixels = 16};

    /**
     * Default constructor
     */
    TOPPmtTTSPar()
    {}

    /**
     * Full constructor
     * @param serialNumber serial number
     */
    explicit TOPPmtTTSPar(const std::string& serialNumber):
      m_serialNumber(serialNumber)
    {}


    /**
     * Append struct gauss
     * @param pmtPixel PMT pixel number (1-based)
     * @param gaus gaussian to be appended
     */
    void appendGaussian(unsigned pmtPixel, const Gaussian& gaus)
    {
      pmtPixel--;
      if (pmtPixel >= c_NumPmtPixels) {
        B2ERROR("TOPPmtTTSPar::appendGaussian: invalid PMT pixel "
                << LogVar("PMT pixel", pmtPixel + 1));
        return;
      }
      m_gaussians[pmtPixel].push_back(gaus);
    }


    /**
     * Append gaussian using its parameters (frac, mean, sigma)
     * @param pmtPixel PMT pixel number (1-based)
     * @param fraction normalization fraction of the gaussian
     * @param mean mean of the gaussian
     * @param sigma sigma of the gaussian
     */
    void appendGaussian(unsigned pmtPixel, double fraction, double mean, double sigma)
    {
      Gaussian gaus;
      gaus.fraction = fraction;
      gaus.mean = mean;
      gaus.sigma = sigma;
      appendGaussian(pmtPixel, gaus);
    }

    /**
     * Returns number of PMT pixels
     * @return number of pixels
     */
    int getNumOfPixels() const {return c_NumPmtPixels;}

    /**
     * Returns PMT serial number
     * @return serial number
     */
    const std::string& getSerialNumber() const {return m_serialNumber;}


    /**
     * Returns vector of gaussians
     * @param pmtPixel PMT pixel number (1-based)
     * @return vector of gaussians
     */
    const std::vector<Gaussian>& getGaussians(unsigned pmtPixel) const
    {
      pmtPixel--;
      if (pmtPixel >= c_NumPmtPixels) {
        B2ERROR("TOPPmtTTSPar::getGaussians: invalid PMT pixel. "
                "Returning data for pixel 1."
                << LogVar("PMT pixel", pmtPixel + 1));
        pmtPixel = 0;
      }
      return m_gaussians[pmtPixel];
    }


    /**
     * Returns a random number, generated according to the distribution
     * @param pmtPixel PMT pixel number (1-based)
     * @return random time or 0 for invalid pixel
     */
    double getRandomTime(unsigned pmtPixel) const
    {
      pmtPixel--;
      if (pmtPixel >= c_NumPmtPixels) return 0;

      double prob = gRandom->Rndm();
      double s = 0;
      for (const auto& gaus : m_gaussians[pmtPixel]) {
        s = s + gaus.fraction;
        if (prob < s) {
          return gRandom->Gaus(gaus.mean, gaus.sigma);
        }
      }
      return 0;  // this should not happen, if fractions are properly normalized
    }


    /**
     * Normalizes the gaussian fractions to unity
     */
    void normalizeFractions()
    {
      double sum = 0;
      for (int ich = 0 ; ich < c_NumPmtPixels ; ich++) {
        for (const auto& gaus : m_gaussians[ich]) {
          sum = sum + gaus.fraction;
        }
        if (sum == 0) return;
        for (auto& gaus : m_gaussians[ich]) {
          gaus.fraction = gaus.fraction / sum;
        }
      }
      return;
    }



  private:

    std::string m_serialNumber;                          /**< serial number, e.g. JTxxxx */
    std::vector<Gaussian> m_gaussians[c_NumPmtPixels];    /**< TTS distribution composed of Gaussians */

    ClassDef(TOPPmtTTSPar, 2); /**< ClassDef */

  };

} // end namespace Belle2


