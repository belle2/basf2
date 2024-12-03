/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <vector>
#include <cmath>
#include <framework/logging/Logger.h>
#include <iostream>

namespace Belle2 {

  /**
   *   dE/dx special large cosine calibration to fix
   *   bending shoulder at large costh
   */

  class CDCDedxCosineEdge: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCDedxCosineEdge(): m_largeCos() {};

    /**
     * Constructor
     * @param largecosth calibration constants
     */
    explicit CDCDedxCosineEdge(const std::vector<std::vector<double>>& largecosth): m_largeCos(largecosth) {};

    /**
     * Destructor
     */
    ~CDCDedxCosineEdge() {};

    /**
     * Get the number of bins of requested side
     * @param side <0 for backward and >0 for forward
     * @return number of bins
     */
    int getSize(const int side) const
    {
      if (side < 0) {
        if (m_largeCos.empty()) return 0;
        return m_largeCos[0].size();
      } else {
        if (m_largeCos.size() < 2) return 0;
        return m_largeCos[1].size();
      }
    };


    /**
     * Return calibration constant for given side and bin #
     * @param side <0 for backward and >0 for forward
     * @param ibin is bin number for given costh
     * @return calibration constant
     */
    double getMean(int side, unsigned int ibin) const
    {
      if (m_largeCos.size() < 2) {
        B2ERROR("CDCDedxCosineEdge: vector-of-vectors too short, returning 1");
        return 1.0;
      }

      const std::vector<double>& temp = side < 0 ?  m_largeCos[0] : m_largeCos[1];
      if (ibin < temp.size()) return temp[ibin];

      B2WARNING("CDCDedxCosineEdge: invalid bin number, returning 1");
      return 1.0;
    };

    /**
     * Return calibration constant for cosine value
     * @param costh is a signed cosine theta value so by definition it recog it's side
     * @return calibration constant
     */
    double getMean(double costh)const
    {
      if (m_largeCos.size() < 2) {
        B2ERROR("CDCDedxCosineEdge: vector-of-vectors too short, returning 1");
        return 1.0;
      }

      const std::vector<double>& temp = costh < 0 ?  m_largeCos[0] : m_largeCos[1];
      if (temp.size() < 2) {
        B2ERROR("CDCDedxCosineEdge: vector too short, returning 1");
        return 1.0;
      }

      double coslow = 0.0, coshigh = 0.0;
      if (costh < 0) {
        coslow = -0.870; coshigh = -0.850; //this is hardcoded and fixed
      } else {
        coslow = 0.950; coshigh = 0.960; //this is hardcoded and fixed
      }

      //don't do anything for other cosine range
      if (costh < coslow or costh > coshigh) {
        return 1.0;
      }

      if (costh <= -0.866 or costh >= 0.9575) return 1.0;

      // gains are stored at the center of the bins
      // find the bin center immediately preceding this value of costh
      int nbins = int(temp.size());
      double binsize = (coshigh - coslow) / nbins;
      int bin = std::floor((costh - 0.5 * binsize - coslow) / binsize);
      if (bin < 0) bin = 0;
      else if (bin > nbins - 2) bin = nbins - 2;

      // extrapolate backward for lowest half-bin and center positive half-bin
      // extrapolate forward for highest half-bin and center negative half-bin
      // MS: not clear why this is needed
      if (temp[bin + 1] - temp[bin] < -0.6) bin++;
      else if (temp[bin + 1] - temp[bin] > 0.6) bin--;

      if (bin < 0 or bin + 1 >= nbins) {
        B2WARNING("CDCDedxCosineEdge:no constants for costh: " << costh << " as it is not in range");
        return 1.0;
      }

      double frac = ((costh - 0.5 * binsize - coslow) / binsize) - bin;
      return ((temp[bin + 1] - temp[bin]) * frac + temp[bin]);
    }

    /**
     * Return specific large cosine constants on give side
     * @param side <0 for backward and >0 for forward
     * @param ibin is bin number for given costh
    */
    double getCosEdgePar(int side, unsigned int ibin) const
    {

      std::vector<double> temp;
      double coslow = 0.0, coshigh = 0.0;
      if (side < 0) {
        if (m_largeCos.size() > 0) temp = m_largeCos[0];
        coslow = -0.870; coshigh = -0.850;
      } else if (side > 0) {
        if (m_largeCos.size() > 1) temp = m_largeCos[1];
        coslow = 0.950; coshigh = 0.960;
      } else {
        B2ERROR("CDCDedxCosineEdge:choose > 0 for forward and <0 for backward side");
        return  -99.0;
      }

      if (ibin >= temp.size()) {
        B2ERROR("CDCDedxCosineEdge:Problem with bin index: choose 0 and " << temp.size() - 1); //
        return -99.0;
      }

      if (temp.size() == 0) return -99.0;
      double bw = abs(coshigh - coslow) / temp.size();
      double bc = coslow  + (0.5 + ibin) * bw; //bin centre
      std::cout << "Par # " << ibin << ", costh bin centre = " << bc << ", const =" << temp[ibin] << std::endl;
      return temp[ibin];
    };


    /**
     * Print large cosine constants array on requested side
     * @param side <0 for backward and >0 for forward constants
    */
    void printCosEdgePars(int side)
    {
      std::vector<double> temp;
      double coslow = 0.0, coshigh = 0.0;
      if (side < 0) {
        if (m_largeCos.size() > 0) temp = m_largeCos[0];
        coslow = -0.870; coshigh = -0.850;
      } else if (side > 0) {
        if (m_largeCos.size() > 1) temp = m_largeCos[1];
        coslow = 0.950; coshigh = 0.960;
      } else {
        B2ERROR("CDCDedxCosineEdge:choose > 0 for forward and <0 for backward side");
        return;
      }

      if (temp.size() == 0) return;
      double bw = abs(coshigh - coslow) / temp.size();
      B2INFO("Printing parameters (0=backward and 1=forward): " << side << ", nPars = " << temp.size());
      for (unsigned int ibin = 0; ibin < temp.size(); ibin++) {
        double bc = coslow  + (0.5 + ibin) * bw; //bin centre
        std::cout << "Par # " << ibin << ", costh bin centre = " << bc << ", const = " << temp[ibin] << std::endl;
      }
    };


    /**
     * Set specific hadron parameter
     * @param side 0 for backward and 1 for forward
     * @param ibin number starts from 0
     * @param value of parameter to set
     */
    void setCosthEdgePar(int side, unsigned int ibin, double value)
    {
      if (m_largeCos.size() < 2) {
        B2ERROR("CDCDedxCosineEdge: vector-of-vectors too short, value not set");
        return;
      }

      int iside = side < 0 ? 0 : 1;
      if (ibin >= m_largeCos[iside].size()) {
        B2ERROR("CDCDedxCosineEdge: invalid bin number, value not set");
        return;
      }

      m_largeCos[iside][ibin] = value;
      std::cout << "Par # " << ibin << ", const = " << m_largeCos[iside][ibin] << std::endl;
    };


  private:
    std::vector<std::vector<double>> m_largeCos; /**< large cosine calibration constants */
    ClassDef(CDCDedxCosineEdge, 1); /**< ClassDef */
  };
} // end namespace Belle2
