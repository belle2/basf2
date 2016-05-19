/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBase.h>
#include <vector>

namespace Belle2 {

  /**
   * Nominal time transition spread of PMT
   */
  class TOPNominalTTS: public TOPGeoBase {
  public:

    /**
     * Gaussian distribution parameters
     */
    struct Gauss {
      float fraction = 0; /**< area normalization */
      float position = 0; /**< peak position [ns] */
      float sigma = 0;    /**< peak width [ns] */
    };

    /**
     * Default constructor
     */
    TOPNominalTTS()
    {}

    /**
     * Append Gaussian
     * @param norm normalization (area)
     * @param mean mean value [ns]
     * @param sigma sigma (width) [ns]
     */
    void appendGaussian(double norm, double mean, double sigma);

    /**
     * Normalize the distribution (fractions)
     * @return value used to normalize fractions
     */
    double normalize();

    /**
     * Returns TTS
     * @return TTS parametrization
     */
    const std::vector<Gauss>& getTTS() const {return m_tts;}

    /**
     * Generate time according to TTS distribution
     * @return time [ns]
     */
    double generateTTS() const;

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Nominal TTS distribution") const;


  private:

    std::vector<Gauss> m_tts; /**< TTS distribution composed of a sum of Gaussians */
    bool m_normalized = false; /**< normalization flag */

    ClassDef(TOPNominalTTS, 1); /**< ClassDef */

  };

} // end namespace Belle2
