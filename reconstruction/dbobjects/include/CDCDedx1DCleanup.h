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

#include <framework/logging/Logger.h>

#include <TObject.h>
#include <cmath>

namespace Belle2 {

  /**
   *   dE/dx wire gain calibration constants
   */

  class CDCDedx1DCleanup: public TObject {

  public:

    /**
     * Default constructor
     */
    CDCDedx1DCleanup(): m_version(0), m_nbins(100), m_onedgains() {};

    /**
     * Constructor
     */
    CDCDedx1DCleanup(short version, short nbins, std::vector<std::vector<double>>& onedgains): m_version(version), m_nbins(nbins),
      m_onedgains(onedgains) {};

    /**
     * Destructor
     */
    ~CDCDedx1DCleanup() {};

    /** Get the version for the 1D cleanup
     */
    short getVersion() const {return m_version; };

    /** Get the number of entrance angle bins
     */
    short getNBins() const {return m_nbins; };

    /** Return dE/dx mean value for given entrance angle
     * @param continuous layer number
     * @param entrance angle (-pi/2 to pi/2)
     */
    double getMean(int layer, double enta) const
    {
      if (layer > 56) B2ERROR("No such layer!");

      // assume rotational symmetry
      if (enta < -3.1416 / 2.0) enta += 3.1416 / 2.0;
      if (enta > 3.1416 / 2.0) enta -= 3.1416 / 2.0;

      int bin = std::floor((sin(enta) + 1.0) / (2.0 / m_nbins));
      if (bin < 0 || bin >= m_nbins) {
        B2WARNING("Problem with CDC dE/dx 2D binning");
        return 1.0;
      }
      return m_onedgains[0][bin];
    };

  private:
    /** dE/dx cleanup correction versus entrance angle
    may be different for different layers, so store as a vector of vectors
    keep a version number to identify which layers are valid */
    short m_version; /**< version number for 1D cleanup correction */
    short m_nbins;  /**< number of entrance angle bins */
    std::vector<std::vector<double>> m_onedgains; /**< dE/dx means in entrance angle bins */

    ClassDef(CDCDedx1DCleanup, 2); /**< ClassDef */
  };
} // end namespace Belle2
