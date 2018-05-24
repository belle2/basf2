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

  class CDCDedx1DCell: public TObject {

  public:

    /**
     * Default constructor
     */
    CDCDedx1DCell(): m_version(0), m_onedgains() {};

    /**
     * Constructor
     */
    CDCDedx1DCell(short version, std::vector<std::vector<double>>& onedgains): m_version(version), m_onedgains(onedgains) {};

    /**
     * Destructor
     */
    ~CDCDedx1DCell() {};

    /**
     * Combine payloads
     **/
    CDCDedx1DCell& operator*=(CDCDedx1DCell const& rhs)
    {
      if (m_version != rhs.getVersion()) {
        B2WARNING("1D cell gain parameters do not match, cannot merge!");
        return *this;
      }
      for (unsigned int layer = 0; layer < getSize(); ++layer) {
        for (unsigned int bin = 0; bin < getNBins(layer); ++bin) {
          m_onedgains[layer][bin] *= rhs.getMean(layer, bin);
        }
      }
      return *this;
    }

    /** Get the version for the 1D cleanup
     */
    short getVersion() const { return m_version; };

    /** Get the number of bins for the entrance angle correction
     */
    unsigned int getSize() const { return m_onedgains.size(); };

    /** Get the number of bins for the entrance angle correction
     */
    unsigned int getNBins(unsigned int layer) const
    {
      if (m_onedgains.size() == 0) {
        B2ERROR("ERROR!");
      }
      if (layer < 8 && m_onedgains.size() <= 2) return m_onedgains[0].size();
      else if (m_onedgains.size() == 2) return m_onedgains[1].size();
      else return m_onedgains[layer].size();
    };

    /** Return dE/dx mean value for the given bin
     * @param bin number
     */
    double getMean(unsigned int layer, unsigned int bin) const
    {
      int mylayer = 0;
      if (layer >= 8 && m_onedgains.size() == 2) mylayer = 1;
      else if (m_onedgains.size() == 56) mylayer = layer;

      if (bin < m_onedgains[mylayer].size())
        return m_onedgains[mylayer][bin];
      else return 1.0;
    }

    /** Return dE/dx mean value for given entrance angle
     * @param continuous layer number
     * @param entrance angle (-pi/2 to pi/2)
     */
    double getMean(unsigned int layer, double enta) const
    {
      if (layer > 56) {
        B2ERROR("No such layer!");
        return 0;
      }

      int mylayer = 0;
      if (layer >= 8 && m_onedgains.size() == 2) mylayer = 1;
      else if (m_onedgains.size() == 56) mylayer = layer;

      // assume rotational symmetry
      if (enta < -3.1416 / 2.0) enta += 3.1416 / 2.0;
      if (enta > 3.1416 / 2.0) enta -= 3.1416 / 2.0;

      double binsize = (m_onedgains[mylayer].size() != 0) ? 2.0 / m_onedgains[mylayer].size() : 0.0;
      int bin = (binsize != 0.0) ? std::floor((std::sin(enta) + 1.0) / binsize) : -1;
      if (bin < 0 || (unsigned)bin >= m_onedgains[mylayer].size()) {
        B2WARNING("Problem with CDC dE/dx 1D binning!");
        return 1.0;
      }

      return m_onedgains[mylayer][bin];
    };

  private:
    /** dE/dx cleanup correction versus entrance angle
    may be different for different layers, so store as a vector of vectors
    keep a version number to identify which layers are valid */
    short m_version; /**< version number for 1D cleanup correction */
    std::vector<std::vector<double>> m_onedgains; /**< dE/dx means in entrance angle bins */

    ClassDef(CDCDedx1DCell, 2); /**< ClassDef */
  };
} // end namespace Belle2
