/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <limits>

namespace Belle2 {

  /**
   *   This object contain parameters of the Time-Walk correction
   *   function of the ECL. General correction contain 7 parameters
   *
   *
   */
  class ECLTimeWalkCorrection : public TObject {
  public:

    /**
     * Default constructor
     */
    ECLTimeWalkCorrection() :
      m_par(c_expectedParametersCount)
    {
    }

    /**
     * Destructor
     */
    ~ECLTimeWalkCorrection() {}

    /** return nth parameter of the time-walk correction.  */
    double getParam(int n) const
    {
      return m_par.at(n);
    }
    /** set nth parameter of the time-walk correction.  */
    void setParam(int n, double newval)
    {
      if (n < 0 || n >= c_expectedParametersCount) {
        B2FATAL("Parameter index should be within [0, " <<
                c_expectedParametersCount - 1 << "]." <<
                LogVar("parameter index", n));
      }
      m_par[n] = newval;
    }

    /**
     * Return std::vector containing all parameters of the time walk correction function
     * @see m_par
     * @see m_parFormat
     */
    std::vector<double> getParams() const
    {
      return m_par;
    }
    /**
     * Set std::vector containing all parameters of the time walk correction function
     * @see m_par
     * @see m_parFormat
     */
    void setParams(const std::vector<double>& new_params)
    {
      if (new_params.size() != c_expectedParametersCount) {
        B2FATAL("Count of parameters must always be equal to " <<
                c_expectedParametersCount <<
                LogVar("count of parameters", new_params.size()));
      }
      m_par = new_params;
    }

    /**
     * Get parameter format ID
     */
    int getParamFormat()
    {
      return m_parFormat;
    }

    /**
     * Set parameter format ID
     */
    void setParamFormat(int newval)
    {
      m_parFormat = newval;
    }

  private:
    /**
     * Format of the correction parameters
     * 0 - same set of (c_expectedParametersCount) parameters for all Cell IDs
     * (for now, only 0 is supported)
     */
    int m_parFormat = 0;
    /** List of time walk correction parameters */
    std::vector<double> m_par;

    /**
     * Number of values expected to be in the m_par vector
     */
    const static int c_expectedParametersCount = 6;

    ClassDef(ECLTimeWalkCorrection, 1); /**< ClassDef */
  };
} // end namespace Belle2

