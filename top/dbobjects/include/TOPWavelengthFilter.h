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
   * Bulk transmittance of wavelength filter
   */
  class TOPWavelengthFilter : public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPWavelengthFilter()
    {}

    /**
     * Full constructor
     * @param lambdaFirst wavelength of the first data point [nm]
     * @param lambdaStep wavelength step [nm]
     * @param trEffi bulk transmittance data points
     * @param name object name
     */
    TOPWavelengthFilter(float lambdaFirst, float lambdaStep,
                        const std::vector<float>& trEffi,
                        const std::string& name = "TOPWavelengthFilter"): TOPGeoBase(name),
      m_lambdaFirst(lambdaFirst), m_lambdaStep(lambdaStep), m_transmittances(trEffi)
    {}

    /**
     * Returns bulk transmittance data points
     * @return bulk transmittance data points
     */
    const std::vector<float>& getBulkTransmittances() const {return m_transmittances;}

    /**
     * Returns bulk transmittance at given wavelength using linear interpolation
     * @param lambda photon wavelength in [nm]
     * @return bulk transmittance
     */
    float getBulkTransmittance(double lambda) const;

    /**
     * Returns wavelength of the first data point
     * @return wavelength in [nm]
     */
    float getLambdaFirst() const {return m_lambdaFirst;}

    /**
     * Returns wavelength step
     * @return wavelength step in [nm]
     */
    float getLambdaStep() const {return m_lambdaStep;}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const override;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Wavelength filter bulk transmittance") const override;


  private:

    float m_lambdaFirst = 0; /**< wavelength of the first data point [nm] */
    float m_lambdaStep = 0;  /**< wavelength step [nm] */
    std::vector<float> m_transmittances; /**< bulk transmittance data points */

    ClassDefOverride(TOPWavelengthFilter, 1); /**< ClassDef */

  };

} // end namespace Belle2


