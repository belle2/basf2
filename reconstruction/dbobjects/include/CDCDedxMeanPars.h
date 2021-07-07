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
    CDCDedxMeanPars(short version, const std::vector<double>& meanpars): m_version(version), m_meanpars(meanpars) {};

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
     * @param par parameter key
     * @param value parameter value
     */
    void setMeanPar(int par, double value) {m_meanpars[par] = value; };

  private:
    short m_version{ -1}; /**< version number for mean parameterization */
    std::vector<double> m_meanpars; /**< dE/dx mean parameters */

    ClassDef(CDCDedxMeanPars, 2); /**< ClassDef */
  };
} // end namespace Belle2
