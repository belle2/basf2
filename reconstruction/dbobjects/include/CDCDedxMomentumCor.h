/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    CDCDedxMomentumCor(): m_momcor() {};

    /**
     * Constructor
     */
    explicit CDCDedxMomentumCor(const std::vector<double>& momcor): m_momcor(momcor) {};

    /**
     * Destructor
     */
    ~CDCDedxMomentumCor() {};

    /**
     * Combine payloads
     **/
    CDCDedxMomentumCor& operator*=(CDCDedxMomentumCor const& rhs)
    {
      if (m_momcor.size() != rhs.getSize()) {
        B2WARNING("Momentum correction parameters do not match, cannot merge!");
        return *this;
      }
      std::vector<double> rhsgains = rhs.getMomCor();
      for (unsigned int bin = 0; bin < m_momcor.size(); ++bin) {
        m_momcor[bin] *= rhsgains[bin];
      }
      return *this;
    }

    /** Get the number of bins for the momentum correction
     */
    unsigned int getSize() const { return m_momcor.size(); };

    /** Get the momentum correction
     */
    std::vector<double> getMomCor() const {return m_momcor; };

    /** Return dE/dx mean value for given bin
     * @param bin for const from bin number
     */
    double getMean(unsigned int bin) const
    {
      if (bin > m_momcor.size()) return 1.0;
      else return m_momcor[bin];
    }

    /** Return dE/dx mean value for given cos(theta)
     * @param mom for const from momemtum value
     */
    double getMean(double mom) const
    {
      if (std::abs(mom) > 10.0) return 0;

      // gains are stored at the center of the bins
      // find the bin center immediately preceding this value of mom
      double binsize = 10.0 / m_momcor.size();
      int bin = std::floor(mom / binsize);

      return m_momcor[bin];
    };

  private:

    std::vector<double> m_momcor; /**< dE/dx gains in momentum bins */

    ClassDef(CDCDedxMomentumCor, 5); /**< ClassDef */
  };
} // end namespace Belle2
