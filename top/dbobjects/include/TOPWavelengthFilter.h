/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Rikuya Okuto                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBase.h>
#include <TObject.h>
#include <vector>

namespace Belle2 {

  /**
   * Bulk transmission of wavelength filter
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
     * @param trEffi bulk transmission data points
     * @param name object name
     */
    TOPWavelengthFilter(float lambdaFirst, float lambdaStep,
                        const std::vector<float>& trEffi,
                        const std::string& name = "TOPWavelengthFilter"): TOPGeoBase(name),
      m_lambdaFirst(lambdaFirst), m_lambdaStep(lambdaStep), m_transmissions(trEffi)
    {}

    /**
     * Returns bulk transmission data points
     * @return bulk transmission data points
     */
    const std::vector<float>& getBulkTransmissions() const {return m_transmissions;}

    /**
     * Returns bulk transmission at given wavelength using linear interpolation
     * @param lambda photon wavelength in [nm]
     * @return bulk transmission
     */
    float getBulkTransmission(double lambda) const;

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
    bool isConsistent() const;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Wavelength filter bulk transmission") const;


  private:

    float m_lambdaFirst = 0; /**< wavelength of the first data point [nm] */
    float m_lambdaStep = 0;  /**< wavelength step [nm] */
    std::vector<float> m_transmissions; /**< bulk transmission data points */

    ClassDef(TOPWavelengthFilter, 1); /**< ClassDef */

  };

} // end namespace Belle2


