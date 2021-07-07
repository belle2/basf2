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
   *   dE/dx sigma (versus beta-gamma) parameterization constants
   */

  class CDCDedxSigmaPars: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCDedxSigmaPars(): m_sigmapars() {};

    /**
     * Constructor
     */
    CDCDedxSigmaPars(short version, const std::vector<double>& sigmapars): m_version(version), m_sigmapars(sigmapars) {};

    /**
     * Destructor
     */
    ~CDCDedxSigmaPars() {};

    /** Get the number of parameters
     */
    double getSize() const {return m_sigmapars.size(); };

    /** Get the version for the sigma parameterization
     */
    int getVersion() const {return m_version; };

    /** Return vector of sigma parameters
     * @return vector of sigma parameters
     */
    std::vector<double> getSigmaPars() const {return m_sigmapars; };

    /** Return specific sigma parameters
     * @return specific sigma parameters
     */
    double getSigmaPar(int par) const {return m_sigmapars[par]; };

    /** Set version number
     * @param version
     */
    void setVersion(short version) {m_version = version; };

    /** Set sigma parameter
     * @param par parameter value
     * @param value parameter key
     */
    void setSigmaPar(int par, double value) {m_sigmapars[par] = value; };

  private:
    short m_version{ -1}; /**< version number for sigma parameterization */
    std::vector<double> m_sigmapars; /**< dE/dx resolution parameters */

    ClassDef(CDCDedxSigmaPars, 2); /**< ClassDef */
  };
} // end namespace Belle2
