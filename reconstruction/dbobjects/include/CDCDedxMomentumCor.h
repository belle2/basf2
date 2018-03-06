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

  class CDCDedxMomentumCor: public TObject {

  public:

    /**
     * Default constructor
     */
    CDCDedxMomentumCor(): m_nbins(100), m_momcor() {};

    /**
     * Constructor
     */
    CDCDedxMomentumCor(short nbins, std::vector<double>& momcor): m_nbins(nbins), m_momcor(momcor) {};

    /**
     * Destructor
     */
    ~CDCDedxMomentumCor() {};

    /** Get the number of momentum correction
     */
    short getNBins() const {return m_nbins; };

    /** Get the momentum correction
     */
    std::vector<double> getMomCor() const {return m_momcor; };

    /** Return dE/dx mean value for given cos(theta)
     * @param cos(theta)
     */
    double getMean(double mom) const
    {
      if (std::abs(mom) > 10.0) return 0;

      // gains are stored at the center of the bins
      // find the bin center immediately preceding this value of mom
      double binsize = 10.0 / m_nbins;
      int bin = std::floor(mom / binsize);

      return m_momcor[bin];
    };

  private:
    short m_nbins; /**< number of momentum bins */
    std::vector<double> m_momcor; /**< dE/dx gains in momentum bins */

    ClassDef(CDCDedxMomentumCor, 1); /**< ClassDef */
  };
} // end namespace Belle2
