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
     */
    explicit CDCDedxCosineEdge(const std::vector<std::vector<double>>& largecosth): m_largeCos(largecosth) {};

    /**
     * Destructor
     */
    ~CDCDedxCosineEdge() {};

    /**
     * Get the number of bins of requested side
     * @param side <0 for backward and >0 for forward
     */
    int getSize(const int side) const
    {
      if (side < 0) {
        return m_largeCos[0].size();
      } else if (side > 0) {
        return m_largeCos[1].size();
      } else {
        B2ERROR("CDCDedxCosineEdge:choose >0 for forward and <0 for backward side");
        return 0;
      }
    };


    /** return calibration constant for given side and bin #
     * @param side <0 for backward and >0 for forward
     * @param ibin is bin number for given costh
     */
    double getMean(int side, unsigned int ibin) const
    {

      std::vector<double> temp;
      if (side < 0) {
        temp = m_largeCos[0];
      } else if (side > 0) {
        temp = m_largeCos[1];
      } else {
        B2ERROR("CDCDedxCosineEdge:choose > 0 for forward and <0 for backward side");
        return 1.0;
      }

      if (ibin >= temp.size()) { //index starts from zero
        B2WARNING("CDCDedxADCNonLinearity:returning (1.0) uncorrected ADC as bin: " <<  ibin << " is not in range");
        return 1.0;
      }
      return temp[ibin];
    };

    /** return calibration constant for cosine value
      * @param costh is a signed cosine theta value so by definitioi it recog it's side
      */
    double getMean(double costh)const
    {
      double coslow = 0.0, coshigh = 0.0;
      std::vector<double> temp;
      if (costh < 0) {
        temp = m_largeCos[0];
        coslow = -0.870; coshigh = -0.850; //this is hardcoded and fixed
      } else if (costh > 0) {
        temp = m_largeCos[1];
        coslow = 0.950; coshigh = 0.960; //this is hardcoded and fixed
      } else {
        B2ERROR("CDCDedxCosineEdge:choose > 0 for forward and <0 for backward side");
        return 1.0;
      }

      //don't do anything for other cosine range
      if (costh < coslow || costh > coshigh) {
        B2WARNING("CDCDedxCosineEdge:outside range (" << costh << ")choose in between " << coslow << " and " << coshigh);
        return 1.0;
      }

      double bw = abs(coshigh - coslow) / temp.size();
      unsigned int ibin = int((costh - coslow) / bw);

      if (ibin >= temp.size()) {
        B2WARNING("CDCDedxCosineEdge:no constants for bin #: " << ibin << " as it is not in range");
        return 1.0;
      }

      //nothing but a protection only
      if (temp[ibin] <= 0.0)return 1.0;

      return temp[ibin];
    }

    /** return specific large cosine constants on give side
     * @param side <0 for backward and >0 for forward
     * @param ibin is bin number for given costh
    */
    double getCosEdgePar(int side, unsigned int ibin) const
    {

      std::vector<double> temp;
      double coslow = 0.0, coshigh = 0.0;
      if (side < 0) {
        temp = m_largeCos[0];
        coslow = -0.870; coshigh = -0.850;
      } else if (side > 0) {
        temp = m_largeCos[1];
        coslow = 0.950; coshigh = 0.960;
      } else {
        B2ERROR("CDCDedxCosineEdge:choose > 0 for forward and <0 for backward side");
        return  -99.0;
      }

      if (ibin >= temp.size()) {
        B2ERROR("CDCDedxCosineEdge:Problem with bin index: choose 0 and " << temp.size() - 1); //
        return -99.0;
      }

      if (temp.size() == 0)return -99.0;
      double bw = abs(coshigh - coslow) / temp.size();
      double bc = coslow  + (0.5 + ibin) * bw; //bin centre
      std::cout << "Par # " << ibin << ", costh bin centre = " << bc << ", const =" << temp[ibin] << std::endl;
      return temp[ibin];
    };


    /** print large cosine constants array on requested side
     * @param side <0 for backward and >0 for forward constants
    */
    void printCosEdgePars(int side)
    {
      std::vector<double> temp;
      double coslow = 0.0, coshigh = 0.0;
      if (side < 0) {
        temp = m_largeCos[0];
        coslow = -0.870; coshigh = -0.850;
      } else if (side > 0) {
        temp = m_largeCos[1];
        coslow = 0.950; coshigh = 0.960;
      } else {
        B2ERROR("CDCDedxCosineEdge:choose > 0 for forward and <0 for backward side");
        return;
      }

      if (temp.size() == 0)return;
      double bw = abs(coshigh - coslow) / temp.size();
      B2INFO("Printing parameters (0=backward and 1=forward): " << side << ", nPars = " << temp.size());
      for (unsigned int ibin = 0; ibin < temp.size(); ibin++) {
        double bc = coslow  + (0.5 + ibin) * bw; //bin centre
        std::cout << "Par # " << ibin << ", costh bin centre = " << bc << ", const = " << temp[ibin] << std::endl;
      }
      temp.clear();
    };


    /** set specific hadron parameter
     * @param side 0 for backward and 1 for forward
     * @param ibin number starts from 0
     * @param value of parameter to set
     */
    void setCosthEdgePar(int side, unsigned int ibin, double value)
    {
      int iside = -99.0;
      if (side < 0) {
        iside = 0;
      } else if (side > 0) {
        iside = 1;
      } else {
        B2ERROR("CDCDedxCosineEdge:choose >0 for forward and <0 for backward side");
        return;
      }

      if (ibin >= m_largeCos[iside].size()) {
        B2ERROR("CDCDedxCosineEdge:Problem with bin index: choose 0 and " << m_largeCos[iside].size() - 1); //
        return;
      }

      m_largeCos[iside][ibin] = value;
      std::cout << "Par # " << ibin << ", const = " << m_largeCos[iside][ibin] << std::endl;
    };


  private:
    std::vector<std::vector<double>> m_largeCos; /**< ADC vs corrected ADC mapping */
    ClassDef(CDCDedxCosineEdge, 1); /**< ClassDef */
  };
} // end namespace Belle2
