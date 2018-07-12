/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>
#include <cmath>

namespace Belle2 {

  /**
   *   dE/dx mean (curve versus beta-gamma) parameterization constants
   */

  class CDCDedxMeanPars: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCDedxMeanPars(): m_meanpars() {};

    /**
     * Constructor
     */
    CDCDedxMeanPars(short version, std::vector<double>& meanpars): m_version(version), m_meanpars(meanpars) {};

    /**
     * Destructor
     */
    ~CDCDedxMeanPars() {};

    /** Get the number of parameters
     */
    double getSize() const {return m_meanpars.size(); };

    /** Get the version for the mean parameterization
     */
    short getVersion() const {return m_version; };

    /** Return vector of mean parameters
     * @return vector of mean parameters
     */
    std::vector<double> getMeanPars() const {return m_meanpars; };

    /** Return specific mean parameter
     * @return mean parameter
     */
    double getMeanPar(int par) const {return m_meanpars[par]; };

    /** Set version number
     * @param version
     */
    void setVersion(short version) {m_version = version; };

    /** Set a parameter
     * @param parameter key
     * @param parameter value
     */
    void setMeanPar(int par, double value) {m_meanpars[par] = value; };

  private:
    short m_version; /**< version number for mean parameterization */
    std::vector<double> m_meanpars; /**< dE/dx mean parameters */

    ClassDef(CDCDedxMeanPars, 1); /**< ClassDef */
  };
} // end namespace Belle2
