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
#include <math.h>

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
    explicit TOPPmtGainPar(const std::string& serialNumber):
      m_serialNumber(serialNumber)
    {}

    /**
     * Sets the data for a given PMT channel
     * @param channel channel number (1-based)
     * @param constant fitting function constant (gain vs HV)
     * @param slope fitting function slope (gain vs HV)
     * @param ratio ratio of gains at B = 1.5 T and B = 0
     */
    void setChannelData(unsigned channel, double constant, double slope, double ratio)
    {
      channel--;
      if (channel >= c_NumChannels) return;
      m_constant[channel] = constant;
      m_slope[channel] = slope;
      m_ratio[channel] = ratio;
    }

    /**
     * Sets the high voltage at gain of 2.5x10^5, without B field
     * @param HV high voltage [V]
     */
    void setNominalHV0(int HV) {m_HV_noB = HV;}

    /**
     * Sets the high voltage at gain of 2.5x10^5, with B field
     * @param HV high voltage [V]
     */
    void setNominalHV(int HV) {m_HV_withB = HV;}

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
    double getConstant(unsigned channel) const
    {
      channel--;
      if (channel >= c_NumChannels) return 0;
      return m_constant[channel];
    }

    /**
     * Returns slope of the gain vs HV fitting function
     * @param channel channel number (1-based)
     * @return slope
     */
    double getSlope(unsigned channel) const
    {
      channel--;
      if (channel >= c_NumChannels) return 0;
      return m_slope[channel];
    }

    /**
     * Returns ratio of gains between 1.5T and 0T
     * @param channel channel number (1-based)
     * @return gain ratio
     */
    double getRatio(unsigned channel) const
    {
      channel--;
      if (channel >= c_NumChannels) return 0;
      return m_ratio[channel];
    }

    /**
     * Returns nominal HV (corresponding to a gain of 2.5x10^5 at B = 0)
     * @return HV in [V]
     */
    int getNominalHV0() const {return m_HV_noB;}

    /**
     * Returns nominal HV (corresponding to a gain of 2.5x10^5 at B = 1.5T)
     * @return HV in [V]
     */
    int getNominalHV() const {return m_HV_withB;}

    /**
     * Returns channel gain at B = 0 for a given high voltage
     * @param channel channel number (1-based)
     * @param HV high voltage [V]
     * @return gain
     */
    double getGain0(unsigned channel, double HV) const
    {
      channel--;
      if (channel >= c_NumChannels) return 0;
      return exp(m_constant[channel] + m_slope[channel] * HV) * 1.0e6;
    }

    /**
     * Returns channel gain at B = 1.5 T for a given high voltage
     * @param channel channel number (1-based)
     * @param HV high voltage [V]
     * @return gain
     */
    double getGain(unsigned channel, double HV) const
    {
      return getGain0(channel, HV) * getRatio(channel);
    }

    /**
     * Print the class content
     */
    void print() const;


  private:

    std::string m_serialNumber;            /**< serial number, e.g. JTxxxx */
    float m_constant[c_NumChannels] = {0}; /**< constant */
    float m_slope[c_NumChannels] = {0};    /**< slope */
    float m_ratio[c_NumChannels] = {0};    /**< ratio of gains at B = 1.5 T and B = 0 */
    int m_HV_noB = 0;                      /**< high voltage for the gain of 2.5x10^5, no B field */
    int m_HV_withB = 0;                    /**< high voltage for the gain of 2.5x10^5, with B field */

    ClassDef(TOPPmtGainPar, 2); /**< ClassDef */

  };

} // end namespace Belle2


