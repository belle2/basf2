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
    CDCDedxHadronCor(short version, const std::vector<double>& hadronpars): m_version(version), m_hadronpars(hadronpars) {};

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
     * @param par parameter key
     * @param value parameter value
     */
    void setHadronPar(int par, double value) {m_hadronpars[par] = value; };

  private:
    short m_version{ -1}; /**< version number for hadron saturation parameterization */
    std::vector<double> m_hadronpars; /**< dE/dx hadron correction parameters */

    ClassDef(CDCDedxHadronCor, 2); /**< ClassDef */
  };
} // end namespace Belle2
