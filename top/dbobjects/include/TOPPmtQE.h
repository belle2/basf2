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
    enum {c_NumChannels = 16};

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
     * @param CE collection efficiency
     */
    TOPPmtQE(const std::string& serialNumber, float lambdaFirst, float lambdaStep, float CE):
      m_serialNumber(serialNumber), m_lambdaFirst(lambdaFirst), m_lambdaStep(lambdaStep), m_CE(CE)
    {}

    /**
     * Set quantum efficiency data points for a given channel
     * @param channel channel number (1-based)
     * @param qe quantum efficiency data points
     */
    void setQE(unsigned channel, const std::vector<float>& qe)
    {
      channel--;
      if (channel < c_NumChannels) m_QE[channel] = qe;
    }

    /**
     * Returns PMT serial number
     * @return serial number
     */
    const std::string& getSerialNumber() const {return m_serialNumber;}

    /**
     * Returns quantum efficiency for a given channel and wavelength using linear interpolation
     * @param channel channel number (1-based)
     * @param lambda wavelength in [nm]
     * @return quantum efficiency
     */
    float getQE(unsigned channel, float lambda) const
    {
      if (channel > c_NumChannels) channel = c_NumChannels;
      channel--;

      int vsize = m_QE[channel].size();
      float lambdaLast = m_lambdaFirst + m_lambdaStep * (vsize - 1);

      if (lambda < m_lambdaFirst) lambda = m_lambdaFirst;
      if (lambda > lambdaLast) lambda = lambdaLast;
      int ilLow = (int)(lambda - m_lambdaFirst) / m_lambdaStep;

      float lambdaLow  = m_lambdaFirst + ilLow * m_lambdaStep;
      float lambdaHigh = m_lambdaFirst + (ilLow + 1) * m_lambdaStep;

      float qe = (m_QE[channel].at(ilLow) * (lambdaHigh - lambda) + m_QE[channel].at(ilLow + 1) * (lambda - lambdaLow)) / m_lambdaStep;
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
     * Returns collection efficiency
     * @return collection efficiency
     */
    float getCE() const {return m_CE;}


  private:

    std::string m_serialNumber; /**< serial number, e.g. JTxxxx */
    std::vector<float> m_QE[c_NumChannels]; /**< QE data points */
    float m_lambdaFirst = 0; /**< wavelength of the first data point [nm] */
    float m_lambdaStep = 0;  /**< wavelength step [nm] */
    float m_CE = 0;          /**< relative collection efficiency */

    ClassDef(TOPPmtQE, 1); /**< ClassDef */

  };

} // end namespace Belle2


