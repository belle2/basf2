/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
     * Constructor with name
     * @param name object name
     */
    explicit TOPNominalTTS(const std::string& name): TOPGeoBase(name)
    {}

    /**
     * Append Gaussian
     * @param norm normalization (area)
     * @param mean mean value [ns]
     * @param sigma sigma (width) [ns]
     */
    void appendGaussian(double norm, double mean, double sigma);

    /**
     * Set type of PMT (see TOPPmtObsoleteData::EType for the defined types)
     * @param type PMT type
     */
    void setPMTType(unsigned type) {m_type = type;}

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
     * Returns PMT type (see TOPPmtObsoleteData::EType for the defined types)
     * @return PMT type
     */
    unsigned getPMTType() const {return m_type;}

    /**
     * Generate time according to TTS distribution
     * @return time [ns]
     */
    double generateTTS() const;

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Nominal TTS distribution") const override;


  private:

    std::vector<Gauss> m_tts; /**< TTS distribution composed of a sum of Gaussians */
    bool m_normalized = false; /**< normalization flag */
    unsigned m_type = 0; /**< PMT type (see TOPPmtObsoleteData::EType) */

    ClassDefOverride(TOPNominalTTS, 2); /**< ClassDef */

  };

} // end namespace Belle2
