/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <map>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <TObject.h>

namespace Belle2 {

  /**
   * Database object for scale factors on alpha for CDC hit charge asymmetry. Needed only for MC production.
   */
  class CDCAlphaScaleFactorForAsymmetry: public TObject {
  public:
    /**
     * Constants
     */
    enum {c_nLayers    = 56,  /**< no. of layers */
          c_nAlphaBins = 150, /**< no. of alpha angle bins per layer*/
         };


    /**
     * Default constructor
     */
    CDCAlphaScaleFactorForAsymmetry() {}


    /**
     * Set the factors in the list
     * @param inputScales factors
     */
    void setScaleFactors(const std::array<std::array<float, c_nAlphaBins>, c_nLayers>& inputScales)
    {
      m_Scales = inputScales;
    }

    /**
     * Get the no. of entries in the list
     */
    unsigned short getEntries() const
    {
      return c_nLayers * c_nAlphaBins ;
    }

    /**
     * Get the whole list
     */
    std::array<std::array<float, c_nAlphaBins>, c_nLayers> getScaleFactors() const
    {
      return m_Scales;
    }

    /**
     * Get the factors for the iCLayer
     * @param  iCLayer layerID
     * @return scale factors for the iCLayer
     */
    std::array<float, c_nAlphaBins> getScaleFactors(unsigned short iCLayer) const
    {
      if (iCLayer >= c_nLayers)
        B2FATAL("Required iCLayer is invalid ! Should be 0 to 55 .");
      return m_Scales[iCLayer] ;
    }

    /**
     * Get the factor for one hit
     * @param iCLayer layerID
     * @param alpha alpha value
     * @return the scale factor for this hit
     */
    double getScaleFactor(unsigned short iCLayer, double alpha) const
    {
      if (alpha > (M_PI / 2)) alpha = alpha - M_PI;
      if (alpha < -(M_PI / 2)) alpha = alpha + M_PI;
      int alpha_bin = trunc(alpha / c_AlphaBinWidth);
      if (alpha_bin > c_nAlphaBins or alpha_bin < -c_nAlphaBins) alpha_bin = c_nAlphaBins - 1 ;
      if (alpha_bin < 0) alpha_bin = -1 * (alpha_bin);
      return m_Scales[iCLayer][alpha_bin];
    }

    int getNBins() const
    {
      return c_nAlphaBins ;
    }



    /**
     * Print all contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "Scale factor list" << std::endl;
      std::cout << "#entries= " << c_nLayers* c_nAlphaBins << std::endl;
      std::cout << "the ratio of data to MC for the hit efficiency of positively charged tracks comparing to negitively charged tracks" <<
                std::endl;

      for (unsigned short iLayer = 0; iLayer < c_nLayers; iLayer++) {
        std::cout << "Scale factors for Layer " << iLayer << " : " << std::endl;
        for (unsigned short i = 0; i < c_nAlphaBins; ++i) {
          std::cout << " Alpha in ( " << i * 0.01 << " , " << (i + 1) * 0.01 << " ), Scale factor: "  << m_Scales[iLayer][i] << std::endl;
        }
      }
    }

  private:
    std::array<std::array<float, c_nAlphaBins>, c_nLayers> m_Scales; /**< scale factors */
    double c_AlphaBinWidth = 0.01; /**< bin width on alpha */
    ClassDef(CDCAlphaScaleFactorForAsymmetry, 1); /**< ClassDef */
  };

} // end namespace Belle2
