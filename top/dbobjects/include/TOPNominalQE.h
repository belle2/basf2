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
   * Nominal quantum efficiency of PMT
   */
  class TOPNominalQE : public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPNominalQE()
    {}

    /**
     * Full constructor
     * @param lambdaFirst wavelenght of the first data point [nm]
     * @param lambdaStep wavelength step [nm]
     * @param CE collection efficiency
     * @param qe quantum efficiency data points
     * @param name object name
     */
    TOPNominalQE(float lambdaFirst, float lambdaStep, float CE,
                 const std::vector<float>& qe,
                 const std::string& name = "TOPNominalQE"): TOPGeoBase(name),
      m_lambdaFirst(lambdaFirst), m_lambdaStep(lambdaStep), m_CE(CE), m_QE(qe)
    {}

    void set(float lambdaFirst, float lambdaStep, float CE,
             const std::vector<float>& qe,
             const std::string& name)
    {

      m_lambdaFirst = lambdaFirst;
      m_lambdaStep = lambdaStep;
      m_CE = CE;
      m_QE = qe;
      setName(name);
    }

    /**
     * Returns quantum efficiency data points
     * @return quantum efficiency data points
     */
    const std::vector<float>& getQE() const {return m_QE;}

    /**
     * Returns quantum efficiency at given photon energy using linear interpolation
     * @param energy photon energy in [eV]
     * @return quantum efficiency
     */
    double getQE(double energy) const;

    /**
     * Returns quantum times collection efficiency at given photon energy
     * using linear interpolation
     * @param energy photon energy in [eV]
     * @return quantum efficiency
     */
    double getEfficiency(double energy) const {return getQE(energy) * m_CE;}

    /**
     * Returns wavelenght of the first data point
     * @return wavelength in [nm]
     */
    double getLambdaFirst() const {return m_lambdaFirst;}

    /**
     * Returns wavelenght step
     * @return wavelength step in [nm]
     */
    double getLambdaStep() const {return m_lambdaStep;}

    /**
     * Returns collection efficiency
     * @return collection efficiency
     */
    double getCE() const {return m_CE;}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Nominal quantum efficiency") const;


  private:

    float m_lambdaFirst = 0; /**< wavelength of the first data point [nm] */
    float m_lambdaStep = 0;  /**< wavelength step [nm] */
    float m_CE = 0;          /**< collection efficiency */
    std::vector<float> m_QE; /**< QE data points */

    ClassDef(TOPNominalQE, 1); /**< ClassDef */

  };

} // end namespace Belle2


