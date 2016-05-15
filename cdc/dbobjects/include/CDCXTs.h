/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <iostream>

#include <TObject.h>

namespace Belle2 {

  /**
   * Database object for xt-relations.
   */
  class CDCXTs: public TObject {
  public:

    enum {c_nSLayers = 56, c_nAlphaPoints = 19, c_nThetaPoints = 7, c_nXTParams = 9};
    /**
     * Default constructor
     */
    CDCXTs(): m_alphaPoints(), m_thetaPoints(), m_xt() {}

    /**
     * Set alpha-angle point (rad)
     */
    void setAlphaPoint(unsigned short i, double alpha)
    {
      m_alphaPoints[i] = alpha;
    }

    /**
     * Set theta-angle point (rad)
     */
    void setThetaPoint(unsigned short i, double theta)
    {
      m_thetaPoints[i] = theta;
    }

    /**
     * Set xt parameters
     */
    void setXTParam(unsigned short iCLayer, unsigned short LR, unsigned short iAlpha, unsigned short iTheta, unsigned short i,
                    double param)
    {
      m_xt[iCLayer][LR][iAlpha][iTheta][i] = param;
    }

    /**
     * Multiply the factor
     */
    void MultiplyFactor(unsigned short iCLayer, unsigned short LR, unsigned short iAlpha, unsigned short iTheta, unsigned short i,
                        double fact)
    {
      m_xt[iCLayer][LR][iAlpha][iTheta][i] *= fact;
    }

    /**
     * Copy XT params.
     */
    void copyXTParam(unsigned short iCLayer, unsigned short LR, unsigned short iAlpha, unsigned short iTheta0, unsigned short iTheta1)
    {
      for (unsigned short i = 0; i < c_nXTParams; ++i) {
        m_xt[iCLayer][LR][iAlpha][iTheta1][i] = m_xt[iCLayer][LR][iAlpha][iTheta0][i];
      }
    }


    /**
     * Get alpha-angle point (rad)
     */
    double getAlphaPoint(unsigned short i)
    {
      return m_alphaPoints[i];
    }

    /**
     * Get theta-angle point (rad)
     */
    double getThetaPoint(unsigned short i)
    {
      return m_thetaPoints[i];
    }

    /**
     * Get xt parameter
     */
    double getXTParam(unsigned short iCLayer, unsigned short LR, unsigned short iAlpha, unsigned short iTheta, unsigned short i)
    {
      return m_xt[iCLayer][LR][iAlpha][iTheta][i];
    }

    /**
     * Print all contents
     */
    void dump()
    {
      std::cout << " " << std::endl;
      std::cout << "Contents of xt db" << std::endl;
      std::cout << "alpha points" << std::endl;

      for (unsigned short i = 0; i < c_nAlphaPoints; ++i) {
        std::cout << " " << m_alphaPoints[i];
      }
      std::cout << " " << std::endl;

      std::cout << " " << std::endl;
      std::cout << "theta points" << std::endl;

      for (unsigned short i = 0; i < c_nThetaPoints; ++i) {
        std::cout << " " << m_thetaPoints[i];
      }
      std::cout << " " << std::endl;

      std::cout << " " << std::endl;
      std::cout << "coefficients for xt" << std::endl;

      for (unsigned short iCL = 0; iCL < c_nSLayers; ++iCL) {
        for (unsigned short LR = 0; LR < 2; ++LR) {
          for (unsigned short iA = 0; iA < c_nAlphaPoints; ++iA) {
            for (unsigned short iT = 0; iT < c_nThetaPoints; ++iT) {
              for (unsigned short i = 0; i < c_nXTParams; ++i) {
                std::cout << " " << m_xt[iCL][LR][iA][iT][i];
              }
              std::cout << " " << std::endl;
            }
          }
        }
      }
    }

    /*
    void init()
    {
      for (unsigned short i = 0; i < c_nAlphaPoints; ++i) {
        m_alphaPoints[i] = 0.;
      }

      for (unsigned short i = 0; i < c_nThetaPoints; ++i) {
        m_thetaPoints[i] = 0.;
      }

      for (unsigned short iCL = 0; iCL < c_nSLayers; ++iCL) {
        for (unsigned short LR = 0; LR < 2; ++LR) {
          for (unsigned short iA = 0; iA < c_nAlphaPoints; ++iA) {
            for (unsigned short iT = 0; iT < c_nThetaPoints; ++iT) {
              for (unsigned short i = 0; i < c_nXTParams; ++i) {
                m_xt[iCL][LR][iA][iT][i] = 0.;
              }
            }
          }
        }
      }
    }
    */

  private:

    float m_alphaPoints[c_nAlphaPoints];  /*!< alpha sampling points for xt (rad) */
    float m_thetaPoints[c_nThetaPoints];  /*!< theta sampling points for xt (rad) */
    float m_xt[c_nSLayers][2][c_nAlphaPoints][c_nThetaPoints][c_nXTParams];  /*!< XT-relation coefficients for each layer, Left/Right, entrance angle and polar angle.  */

    ClassDef(CDCXTs, 1); /**< ClassDef */
  };

} // end namespace Belle2
