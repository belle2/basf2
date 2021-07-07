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
#include <TMath.h>

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
    CDCDedx1DCell(short version, const std::vector<std::vector<double>>& onedgains): m_version(version), m_onedgains(onedgains) {};

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
        double tempLayer = -1; //out of bound layer
        if (getSize() == 2)tempLayer = layer * 20; // 20 to make <8 and >=8 difference
        else if (getSize() == 56)tempLayer = layer;
        else B2ERROR("ERROR! Wrong # of constants vector array");
        for (unsigned int bin = 0; bin < getNBins(layer); ++bin) {
          m_onedgains[layer][bin] *= rhs.getMean(tempLayer, bin);
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
        return 0;
      }

      if (m_onedgains.size() == 2) {
        if (layer == 0) return m_onedgains[0].size();
        else if (layer == 1) return m_onedgains[1].size();
        else {
          B2ERROR("ERROR! const vector not found");
          return 0;
        }
      } else if (m_onedgains.size() == 56) {
        return m_onedgains[layer].size();
      } else {
        B2ERROR("ERROR! Wrong # of constants vector array: getNBins()");
        return 0;
      }
      return 0;
    };

    /** Return dE/dx mean value for the given bin
     * @param layer is layer number between 0-55
     * @param bin is enta bin number
     */
    double getMean(unsigned int layer, unsigned int bin) const
    {
      int mylayer = 0;
      if (layer >= 8 && m_onedgains.size() == 2) mylayer = 1;
      else if (m_onedgains.size() == 56) mylayer = layer;

      if (bin < m_onedgains[mylayer].size())
        return m_onedgains[mylayer][bin];
      else return 1.0;
    };

    /** Reset dE/dx mean value for the given bin
     * @param layer is layer number between 0-55
     * @param bin is enta bin number
     * @param value is constant for requested entabin and layer
    */
    void setMean(unsigned int layer, unsigned int bin, double value)
    {
      int mylayer = 0;
      if (layer >= 8 && m_onedgains.size() == 2) mylayer = 1;
      else if (m_onedgains.size() == 56) mylayer = layer;

      if (bin < m_onedgains[mylayer].size()) m_onedgains[mylayer][bin] = value;
      else m_onedgains[mylayer][bin] = 1.0;
    };

    /** Return dE/dx mean value for given entrance angle
     * @param layer continuous layer number
     * @param enta entrance angle (-pi/2 to pi/2)
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

      double piby2 =  TMath::Pi() / 2.0;
      // assume rotational symmetry
      if (enta < -piby2) enta += piby2;
      if (enta > piby2) enta -= piby2;

      double binsize = (m_onedgains[mylayer].size() != 0) ? 2.0 * piby2 / m_onedgains[mylayer].size() : 0.0;
      int bin = (binsize != 0.0) ? std::floor((enta + piby2) / binsize) : -1;
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

    ClassDef(CDCDedx1DCell, 5); /**< ClassDef */
  };
} // end namespace Belle2
