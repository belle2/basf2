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

namespace Belle2 {

  /**
   *   dE/dx hadron saturation parameterization constants
   */

  class CDCDedxHadronCor: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCDedxHadronCor(): m_hadronpars() {};

    /**
     * Constructor
     */
    CDCDedxHadronCor(short version, std::vector<double>& hadronpars): m_version(version), m_hadronpars(hadronpars) {};

    /**
     * Destructor
     */
    ~CDCDedxHadronCor() {};

    /** Get the number of parameters
     */
    double getSize() const {return m_hadronpars.size(); };

    /** Get the version for the hadron parameterization
     */
    short getVersion() const {return m_version; };

    /** Return vector of hadron parameters
     * @return vector of hadron parameters
     */
    std::vector<double> getHadronPars() const {return m_hadronpars; };

    /** Return specific hadron parameter
     * @return hadron parameter
     */
    double getHadronPar(int par) const {return m_hadronpars[par]; };

    /** Set version number
     * @param version
     */
    void setVersion(short version) {m_version = version; };

    /** Set a parameter
     * @param parameter key
     * @param parameter value
     */
    void setHadronPar(int par, double value) {m_hadronpars[par] = value; };

  private:
    short m_version; /**< version number for hadron saturation parameterization */
    std::vector<double> m_hadronpars; /**< dE/dx hadron correction parameters */

    ClassDef(CDCDedxHadronCor, 1); /**< ClassDef */
  };
} // end namespace Belle2
