/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Alessandro Gaz                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  /**
   * Parameterized PMT gain (from laser testing at Nagoya)
   */
  class TOPPmtGainPar : public TObject {
  public:
    /**
     * number of PMT channels
     */
    enum {c_NumChannels = 16};

    /**
     * Default constructor
     */
    TOPPmtGainPar()
    {}

    /**
     * Useful constructor
     * @param serialNumber serial number
     */
    TOPPmtGainPar(const std::string& serialNumber):
      m_serialNumber(serialNumber)
    {}

    /**
     * Returns PMT serial number
     * @return serial number
     */
    const std::string& getSerialNumber() const {return m_serialNumber;}

    /**
     * Returns constant of the gain vs HV fitting function
     * @param channel channel number (1-based)
     * @return constant
     */
    float getGainConstant(unsigned channel) const
    {
      if (channel > c_NumChannels) channel = c_numChannels;
      channel--;
      return m_constant[channel];
    }

    /**
     * Returns slope of the gain vs HV fitting function
     * @param channel channel number (1-based)
     * @return slope
     */
    float getGainSlope(unsigned channel) const
    {
      if (channel > c_NumChannels) channel = c_numChannels;
      channel--;
      return m_slope[channel];
    }

    /**
     * Returns ratio of gains between 1.5T and 0T
     * @param channel channel number (1-based)
     * @return gain ratio
     */
    float getGainRatio(unsigned channel) const
    {
      if (channel > c_NumChannels) channel = c_numChannels;
      channel--;
      return m_ratio[channel];
    }

    /**
     * Returns nominal HV (corresponding to a gain of 5x10^5)
     * @return HV
     */
    float getNominalHV() const {return m_HV;}


  private:

    std::string m_serialNumber;          /**< serial number, e.g. JTxxxx */
    float m_constant[c_NumChannels] = 0; /**< constant */
    float m_slope[c_NumChannels] = 0;    /**< slope */
    float m_ratio[c_NumChannels] = 0;    /**< ratio of gains at B = 1.5 T and B = 0 */
    float m_HV = 0;                      /**< high voltage setting to obtain the gain of 5e5 */

    ClassDef(TOPPmtGainPar, 1); /**< ClassDef */

  };

} // end namespace Belle2


