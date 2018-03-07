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
     * number of PMT pixeld
     */
    enum {c_NumPmtPixels = 16};

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
     * Sets the data for a given PMT pmtPixel
     * @param pmtPixel pmtPixel number (1-based)
     * @param constant fitting function constant (gain vs HV)
     * @param slope fitting function slope (gain vs HV)
     * @param ratio ratio of gains at B = 1.5 T and B = 0
     */
    void setPmtPixelData(unsigned pmtPixel, double constant, double slope, double ratio)
    {
      pmtPixel--;
      if (pmtPixel >= c_NumPmtPixels) return;
      m_constant[pmtPixel] = constant;
      m_slope[pmtPixel] = slope;
      m_ratio[pmtPixel] = ratio;
    }

    /**
     * Sets the high voltage at gain of 5x10^5, without B field
     * @param HV high voltage [V]
     */
    void setNominalHV0(float HV) {m_HV_noB = HV;}

    /**
     * Sets the high voltage at gain of 5x10^5, with B field
     * @param HV high voltage [V]
     */
    void setNominalHV(float HV) {m_HV_withB = HV;}

    /**
     * Returns PMT serial number
     * @return serial number
     */
    const std::string& getSerialNumber() const {return m_serialNumber;}

    /**
     * Returns constant of the gain vs HV fitting function
     * @param pmtPixel pmtPixel number (1-based)
     * @return constant
     */
    double getConstant(unsigned pmtPixel) const
    {
      pmtPixel--;
      if (pmtPixel >= c_NumPmtPixels) return 0;
      return m_constant[pmtPixel];
    }

    /**
     * Returns slope of the gain vs HV fitting function
     * @param pmtPixel pmtPixel number (1-based)
     * @return slope
     */
    double getSlope(unsigned pmtPixel) const
    {
      pmtPixel--;
      if (pmtPixel >= c_NumPmtPixels) return 0;
      return m_slope[pmtPixel];
    }

    /**
     * Returns ratio of gains between 1.5T and 0T
     * @param pmtPixel pmtPixel number (1-based)
     * @return gain ratio
     */
    double getRatio(unsigned pmtPixel) const
    {
      pmtPixel--;
      if (pmtPixel >= c_NumPmtPixels) return 0;
      return m_ratio[pmtPixel];
    }

    /**
     * Returns nominal HV (corresponding to a gain of 5x10^5 at B = 0)
     * @return HV in [V]
     */
    float getNominalHV0() const {return m_HV_noB;}

    /**
     * Returns nominal HV (corresponding to a gain of 5x10^5 at B = 1.5T)
     * @return HV in [V]
     */
    float getNominalHV() const {return m_HV_withB;}

    /**
     * Returns pmtPixel gain at B = 0 for a given high voltage
     * @param pmtPixel pmtPixel number (1-based)
     * @param HV high voltage [V]
     * @return gain
     */
    double getGain0(unsigned pmtPixel, double HV) const
    {
      pmtPixel--;
      if (pmtPixel >= c_NumPmtPixels) return 0;
      return exp(m_constant[pmtPixel] + m_slope[pmtPixel] * HV) * 1.0e6;
    }

    /**
     * Returns pmtPixel gain at B = 1.5 T for a given high voltage
     * @param pmtPixel pmtPixel number (1-based)
     * @param HV high voltage [V]
     * @return gain
     */
    double getGain(unsigned pmtPixel, float HV) const
    {
      return getGain0(pmtPixel, HV) * getRatio(pmtPixel);
    }

    /**
     * Print the class content
     */
    void print() const;


  private:

    std::string m_serialNumber;             /**< serial number, e.g. JTxxxx */
    float m_constant[c_NumPmtPixels] = {0}; /**< constant */
    float m_slope[c_NumPmtPixels] = {0};    /**< slope */
    float m_ratio[c_NumPmtPixels] = {0};    /**< ratio of gains at B = 1.5 T and B = 0 */
    float m_HV_noB = 0;                     /**< high voltage for the gain of 5x10^5, no B field */
    float m_HV_withB = 0;                   /**< high voltage for the gain of 5x10^5, with B field */

    ClassDef(TOPPmtGainPar, 3); /**< ClassDef */

  };

} // end namespace Belle2


