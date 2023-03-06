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
      m_par()
    {
    }

    /**
     * Destructor
     */
    ~ECLTimeWalkCorrection() {}

    /** return nth parameter of the time-walk correction.  */
    double getParam(int n) const
    {
      return m_par[n];
    }
    /** set nth parameter of the time-walk correction.  */
    void setParam(int n, double newval)
    {
      m_par[n] = newval;
    }

    /** */
    std::vector<double> getParams() const
    {
      return m_par;
    }
    /** */
    void setParams(const std::vector<double>& new_params)
    {
      m_par = new_params;
    }

  private:
    /**
     * Format of the correction parameters
     * 0 - same set of parameters for all Cell IDs
     */
    int m_par_format = 0;
    /** List of time walk correction parameters */
    std::vector<double> m_par;

    ClassDef(ECLTimeWalkCorrection, 1); /**< ClassDef */
  };
} // end namespace Belle2

