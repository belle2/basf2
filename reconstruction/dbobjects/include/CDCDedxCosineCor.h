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

  class CDCDedxCosineCor: public TObject {

  public:

    /**
     * Default constructor
     */
    CDCDedxCosineCor(): m_nbins(100), m_cosgains() {};

    /**
     * Constructor
     */
    CDCDedxCosineCor(short nbins, std::vector<double>& cosgains): m_nbins(nbins), m_cosgains(cosgains) {};

    /**
     * Destructor
     */
    ~CDCDedxCosineCor() {};

    /** Get the number of cosine correction
     */
    short getNBins() const {return m_nbins; };

    /** Return dE/dx mean value for given cos(theta)
     * @param cos(theta)
     */
    double getMean(double costh) const
    {
      if (std::abs(costh) > 1) return 0;

      // gains are stored at the center of the bins
      // find the bin center immediately preceding this value of costh
      double binsize = 2.0 / m_nbins;
      int bin = std::floor((costh - 0.5 * binsize + 1.0) / binsize);

      // extrapolation
      // extrapolate backward for lowest half-bin and center positive half-bin
      // extrapolate forward for highest half-bin and center negative half-bin
      int thisbin = bin, nextbin = bin + 1;
      double frac = ((costh - 0.5 * binsize + 1.0) / binsize) - bin;
      if ((costh + 1) < (binsize / 2) || (costh > 0 && std::abs(costh) < (binsize / 2))) {
        thisbin = bin + 1; nextbin = bin + 2;
      } else {
        if ((costh - 1) > -1.0 * (binsize / 2) || (costh < 0 && std::abs(costh) < (binsize / 2))) {
          thisbin = bin - 1; nextbin = bin;
        }
      }

      if (thisbin < 0 || nextbin >= m_nbins) {
        B2WARNING("Problem with extrapolation of CDC dE/dx cosine correction");
        return 1.0;
      }
      return ((m_cosgains[nextbin] - m_cosgains[thisbin]) * frac + m_cosgains[thisbin]);
    };

  private:
    short m_nbins; /**< number of cosine bins */
    std::vector<double> m_cosgains; /**< dE/dx gains in cos(theta) bins */

    ClassDef(CDCDedxCosineCor, 2); /**< ClassDef */
  };
} // end namespace Belle2
