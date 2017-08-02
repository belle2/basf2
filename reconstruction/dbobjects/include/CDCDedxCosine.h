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
#include <map>

namespace Belle2 {

  /**
   *   dE/dx wire gain calibration constants
   */

  class CDCDedxCosine: public TObject {

  public:

    /**
     * Default constructor
     */
    CDCDedxCosine(): m_means() {};

    /**
     * Constructor
     */
    CDCDedxCosine(std::map<double, double>& means): m_means(means) {};

    /**
     * Destructor
     */
    ~CDCDedxCosine() {};

    /** Return bin centers as a function of cos(theta)
     * @return vector of bin centers
     */
    std::vector<double> getCosThetaBins() const
    {
      std::vector<double> binedges;
      for (std::map<double, double>::const_iterator it = m_means.begin(); it != m_means.end(); ++it)
        binedges.push_back(it->first);
      return binedges;
    };

    /** Return dE/dx mean value for given cos(theta)
     * @return dE/dx mean value
     */
    double getMean(double costh) const
    {
      double mean = m_means.at(costh);
      return mean;
    };

  private:
    /** dE/dx gains in cos(theta) bins: key is low edge of bin */
    std::map<double, double> m_means;

    ClassDef(CDCDedxCosine, 2); /**< ClassDef */
  };
} // end namespace Belle2
