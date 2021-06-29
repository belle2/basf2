/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
#include <TObject.h>

#include <cmath>
#include <vector>

namespace Belle2 {

  /** class to contain the Cluster Position Error Formulae*/
  class SVDPositionErrorFunction : public TObject {

  public:

    /** typedef of the return value of the position error function*/
    typedef double (SVDPositionErrorFunction::*posErrFunction)(double, int) const;

    /** returns the position error, depending on the cluster SNR and size*/
    double getPositionError(double snr, int size) const
    {
      posErrFunction f = m_implementations[m_current];
      return (this->*f)(snr, size) ;
    }

    /** constructor */
    SVDPositionErrorFunction()
    {
      // The m_implementations vector is static.
      // We have to initialize it just once.
      if (m_implementations.size() == 0) {
        m_implementations.push_back(&SVDPositionErrorFunction::v0);
        //m_implementations.push_back(
        //  &SVDPositionErrorFunction::betterVersion);
      }

      m_current = m_implementations.size() - 1;

    };

    /** allows to choose the function version */
    void set_current(int current)
    {
      m_current = current;
    }

    //SETTERS FOR function ID = 0 (v0, CoGOnly)
    /** set the a1 cluster size dependent parameters*/
    void set_a1(double* a1, int maxSize)
    {
      if (maxSize != maxClusterSize)
        B2ERROR("please provide the parameters for exactly a max cluster size = " << maxClusterSize <<
                ", i.e. >= " << maxClusterSize << "share the same parameters");
      else
        for (int i = 0; i < maxClusterSize; i++)
          m_a1[i] = a1[i];
    }
    /** set the a2 cluster size dependent parameters*/
    void set_a2(double* a2, int maxSize)
    {
      if (maxSize != maxClusterSize)
        B2ERROR("please provide the parameters for exactly a max cluster size = " << maxClusterSize <<
                ", i.e. >= " << maxClusterSize << "share the same parameters");
      else
        for (int i = 0; i < maxClusterSize; i++)
          m_a2[i] = a2[i];
    }
    /** set the b1 cluster size dependent parameters*/
    void set_b1(double* b1, int maxSize)
    {
      if (maxSize != maxClusterSize)
        B2ERROR("please provide the parameters for exactly a max cluster size = " << maxClusterSize <<
                ", i.e. >= " << maxClusterSize << "share the same parameters");
      else
        for (int i = 0; i < maxClusterSize; i++)
          m_b1[i] = b1[i];
    }
    /** set the c1 cluster size dependent parameters*/
    void set_c1(double* c1, int maxSize)
    {
      if (maxSize != maxClusterSize)
        B2ERROR("please provide the parameters for exactly a max cluster size = " << maxClusterSize <<
                ", i.e. >= " << maxClusterSize << "share the same parameters");
      else
        for (int i = 0; i < maxClusterSize; i++)
          m_c1[i] = c1[i];
    }
    /** set the cluster size dependent scaleFactors*/
    void set_scaleFactor(double* scaleFactor, int maxSize)
    {
      if (maxSize != maxClusterSize)
        B2ERROR("please provide the scaleFactors for exactly a max cluster size = " << maxClusterSize <<
                ", i.e. >= " << maxClusterSize << "share the same scale factor");
      else
        for (int i = 0; i < maxClusterSize; i++)
          m_scaleFactor[i] = scaleFactor[i];
    }


    /** copy constructor */
    SVDPositionErrorFunction(const Belle2::SVDPositionErrorFunction& a);

    /** operator = */
    SVDPositionErrorFunction& operator=(const Belle2::SVDPositionErrorFunction& a);

  private:

    /** total number of cluster sizes */
    static const int maxClusterSize = 5;

    /** scale factors */
    float m_scaleFactor[ maxClusterSize ] = {1, 1, 1, 1, 1};

    /** function parameters & implementations*/

    /** ID = {0}, HIKARU: error is the sum in quadrature of
     * A = a1/(x+a2)
     * B = b1 * x
     * C = c1
     * with x = S_cl / N_cl * sqrt(size)
     */
    double m_a1[ maxClusterSize ] = {3, 3, 3, 3, 3}; /**< size dependent a1-parameter*/
    double m_a2[ maxClusterSize ] = {3, 0, 0, 0, 0}; /**< size dependent a2-parameter*/
    double m_b1[ maxClusterSize ] = {0}; /**< size dependent b1-parameter*/
    double m_c1[ maxClusterSize ] = {0}; /**< size dependent c1-parameter*/

    /** v0 implementation
     * @param clSNR cluster SNR
     * @param clSize cluster size
     * @return final cluster position error, including scale factor
     */
    double v0(double clSNR, int clSize) const
    {
      if (clSize > maxClusterSize - 1)
        clSize = maxClusterSize - 1;

      double x = sqrt(clSize) * clSNR;

      double A = m_a1[clSize - 1] / (x + m_a2[clSize - 1]);
      double B = m_b1[clSize - 1] * x;
      double C = m_c1[clSize - 1];

      return sqrt(A * A + B * B + C * C) * m_scaleFactor[clSize - 1];
    };

    /** current function ID */
    int m_current;

    /** vector of functions for position error, we use the m_current*/
    static std::vector < posErrFunction > m_implementations; //! Do not stream this, please throw it in the WC

    ClassDef(SVDPositionErrorFunction, 1);
  };

}
