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
    CDCDedxCosineCor(): m_cosgains() {};

    /**
     * Constructor
     */
    explicit CDCDedxCosineCor(std::vector<double>& cosgains): m_cosgains(cosgains) {};

    /**
     * Destructor
     */
    ~CDCDedxCosineCor() {};

    /**
     * Combine payloads
     **/
    CDCDedxCosineCor& operator*=(CDCDedxCosineCor const& rhs)
    {
      if (m_cosgains.size() % rhs.getSize() != 0) {
        B2WARNING("Cosine gain parameters do not match, cannot merge!");
        return *this;
      }
      std::vector<double> rhsgains = rhs.getCosCor();
      int scale = std::floor(m_cosgains.size() / rhs.getSize() + 0.001);
      for (unsigned int bin = 0; bin < m_cosgains.size(); ++bin) {
        m_cosgains[bin] *= rhsgains[std::floor(bin / scale + 0.001)];
      }
      return *this;
    }

    /** Get the number of bins for the cosine correction
     */
    unsigned int getSize() const { return m_cosgains.size(); };

    /** Get the cosine correction
     */
    std::vector<double> getCosCor() const {return m_cosgains; };

    /** Return dE/dx mean value for the given bin
     * @param bin number
     */
    double getMean(unsigned int bin) const
    {
      if (bin > m_cosgains.size()) return 1.0;
      else return m_cosgains[bin];
    }

    /** Return dE/dx mean value for given cos(theta)
     * @param cos(theta)
     */
    double getMean(double costh) const
    {
      if (std::abs(costh) > 1) return 0;

      // gains are stored at the center of the bins
      // find the bin center immediately preceding this value of costh
      double binsize = 2.0 / m_cosgains.size();
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

      if (thisbin < 0 || (unsigned)nextbin >= m_cosgains.size()) {
        B2WARNING("Problem with extrapolation of CDC dE/dx cosine correction");
        return 1.0;
      }
      return ((m_cosgains[nextbin] - m_cosgains[thisbin]) * frac + m_cosgains[thisbin]);
    };

  private:
    std::vector<double> m_cosgains; /**< dE/dx gains in cos(theta) bins */

    ClassDef(CDCDedxCosineCor, 5); /**< ClassDef */
  };
} // end namespace Belle2
