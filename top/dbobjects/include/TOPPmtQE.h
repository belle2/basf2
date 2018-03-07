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
#include <vector>

namespace Belle2 {

  /**
   * Nagoya measurements of quantum efficiency
   */
  class TOPPmtQE : public TObject {
  public:
    /**
     * number of PMT channels
     */
    enum {c_NumPmtPixels = 16};

    /**
     * Default constructor
     */
    TOPPmtQE()
    {}

    /**
     * Useful constructor
     * @param serialNumber serial number
     * @param lambdaFirst wavelenght of the first data point [nm]
     * @param lambdaStep wavelength step [nm]
     * @param CE collection efficiency at B = 0T
     * @param CE collection efficiency at B = 1.5T
     */
    TOPPmtQE(const std::string& serialNumber, float lambdaFirst, float lambdaStep, float CE0, float CE):
      m_serialNumber(serialNumber), m_lambdaFirst(lambdaFirst), m_lambdaStep(lambdaStep), m_CE_noB(CE0), m_CE_withB(CE)
    {}

    /**
     * Set quantum efficiency data points for a given pmtPixel
     * @param pmtPixel pmtPixel number (1-based)
     * @param qe quantum efficiency data points
     */
    void setQE(unsigned pmtPixel, const std::vector<float>& qe)
    {
      pmtPixel--;
      if (pmtPixel < c_NumPmtPixels) m_QE[pmtPixel] = qe;
    }

    /**
     * Returns PMT serial number
     * @return serial number
     */
    const std::string& getSerialNumber() const {return m_serialNumber;}

    /**
     * Returns quantum efficiency for a given pmtPixel and wavelength using linear interpolation
     * @param pmtPixel pmtPixel number (1-based)
     * @param lambda wavelength in [nm]
     * @return quantum efficiency
     */
    float getQE(unsigned pmtPixel, float lambda) const
    {
      if (pmtPixel > c_NumPmtPixels) pmtPixel = c_NumPmtPixels;
      pmtPixel--;

      int vsize = m_QE[pmtPixel].size();
      float lambdaLast = m_lambdaFirst + m_lambdaStep * (vsize - 1);

      if (lambda < m_lambdaFirst) lambda = m_lambdaFirst;
      if (lambda > lambdaLast) lambda = lambdaLast;
      int ilLow = (int)(lambda - m_lambdaFirst) / m_lambdaStep;

      float lambdaLow  = m_lambdaFirst + ilLow * m_lambdaStep;
      float lambdaHigh = m_lambdaFirst + (ilLow + 1) * m_lambdaStep;

      float qe = (m_QE[pmtPixel].at(ilLow) * (lambdaHigh - lambda) + m_QE[pmtPixel].at(ilLow + 1) * (lambda - lambdaLow)) / m_lambdaStep;
      return qe;
    }

    /**
     * Returns wavelenght of the first data point
     * @return wavelength in [nm]
     */
    float getLambdaFirst() const {return m_lambdaFirst;}

    /**
     * Returns wavelenght step
     * @return wavelength step in [nm]
     */
    float getLambdaStep() const {return m_lambdaStep;}

    /**
     * Returns collection efficiency (for no B field)
     * @return collection efficiency
     */
    float getCE0() const {return m_CE_noB;}

    /**
     * Returns collection efficiency (for B field = 1.5T)
     * @return collection efficiency
     */
    float getCE() const {return m_CE_withB;}


  private:

    std::string m_serialNumber; /**< serial number, e.g. JTxxxx */
    std::vector<float> m_QE[c_NumPmtPixels]; /**< QE data points */
    float m_lambdaFirst = 0; /**< wavelength of the first data point [nm] */
    float m_lambdaStep = 0;  /**< wavelength step [nm] */
    float m_CE_noB = 0;      /**< relative collection efficiency, without B field */
    float m_CE_withB = 0;    /**< relative collection efficiency, with B field */

    ClassDef(TOPPmtQE, 2); /**< ClassDef */

  };

} // end namespace Belle2


