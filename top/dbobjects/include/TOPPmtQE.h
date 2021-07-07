/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
     * number of PMT pixels
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
     * @param CE0 collection efficiency at B = 0T
     * @param CE collection efficiency at B = 1.5T
     */
    TOPPmtQE(const std::string& serialNumber,
             float lambdaFirst, float lambdaStep, float CE0, float CE):
      m_serialNumber(serialNumber), m_lambdaFirst(lambdaFirst), m_lambdaStep(lambdaStep),
      m_CE_noB(CE0), m_CE_withB(CE)
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
     * Returns quantum efficiency data points for a given pixel
     * @param pmtPixel pmtPixel number (1-based)
     * @return quantum efficiency data points (empty vector for invalid pixel)
     */
    const std::vector<float>& getQE(unsigned pmtPixel) const;

    /**
     * Returns quantum efficiency for a given pixel and wavelength,
     * using linear interpolation.
     * @param pmtPixel pmtPixel number (1-based)
     * @param lambda wavelength in [nm]
     * @return quantum efficiency
     */
    double getQE(unsigned pmtPixel, double lambda) const;

    /**
     * Returns envelope quantum efficiency data points (maximum over pixels)
     * @return envelope quantum efficiency data points
     */
    const std::vector<float>& getEnvelopeQE() const;

    /**
     * Returns envelope quantum efficiency for a given wavelength (maximum over pixels),
     * using linear interpolation.
     * @param lambda wavelength in [nm]
     * @return quantum efficiency
     */
    double getEnvelopeQE(double lambda) const;

    /**
     * Returns quantum times collection efficiency for a given pixel and wavelength,
     * using linear interpolation.
     * @param pmtPixel pmtPixel number (1-based)
     * @param lambda wavelength in [nm]
     * @param BfieldOn true for magnetic field being ON
     * @return quantum efficiency
     */
    double getEfficiency(unsigned pmtPixel, double lambda, bool BfieldOn) const;

    /**
     * Returns wavelenght of the first data point
     * @return wavelength in [nm]
     */
    double getLambdaFirst() const {return m_lambdaFirst;}

    /**
     * Returns wavelenght of the last data point (maximal of pixels)
     * @return wavelength in [nm]
     */
    double getLambdaLast() const;

    /**
     * Returns wavelenght of the last data point for a given pixel
     * @param pmtPixel pmtPixel number (1-based)
     * @return wavelength in [nm]
     */
    double getLambdaLast(unsigned pmtPixel) const;

    /**
     * Returns wavelenght step
     * @return wavelength step in [nm]
     */
    double getLambdaStep() const {return m_lambdaStep;}

    /**
     * Returns collection efficiency
     * @param BfieldOn true for magnetic field being ON
     * @return collection efficiency
     */
    double getCE(bool BfieldOn) const;


  private:

    /**
     * Sets envelope quantum efficiency.
     */
    void setEnvelopeQE() const;

    /**
     * Interpolate between QE datapoints (linear interpolation).
     * @param lambda wavelength in [nm]
     * @param QE quantum efficiency data points
     * @return quantum efficiency at lambda
     */
    double interpolate(double lambda, const std::vector<float>& QE) const;

    std::string m_serialNumber; /**< serial number, e.g. JTxxxx */
    std::vector<float> m_QE[c_NumPmtPixels]; /**< QE data points */
    float m_lambdaFirst = 0; /**< wavelength of the first data point [nm] */
    float m_lambdaStep = 0;  /**< wavelength step [nm] */
    float m_CE_noB = 0;      /**< relative collection efficiency, without B field */
    float m_CE_withB = 0;    /**< relative collection efficiency, with B field */

    /** cache for envelope QE */
    mutable std::vector<float> m_envelopeQE; //! don't write out

    ClassDef(TOPPmtQE, 3); /**< ClassDef */

  };

} // end namespace Belle2


