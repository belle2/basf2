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
    void set_a1(double a1_1, double a1_2, double a1_3, double a1_4, double a1_5)
    {
      m_a1[0] = a1_1;
      m_a1[1] = a1_2;
      m_a1[2] = a1_3;
      m_a1[3] = a1_4;
      m_a1[4] = a1_5;
    }
    /** set the a2 cluster size dependent parameters*/
    void set_a2(double a2_1, double a2_2, double a2_3, double a2_4, double a2_5)
    {
      m_a2[0] = a2_1;
      m_a2[1] = a2_2;
      m_a2[2] = a2_3;
      m_a2[3] = a2_4;
      m_a2[4] = a2_5;
    }
    /** set the b1 cluster size dependent parameters*/
    void set_b1(double b1_1, double b1_2, double b1_3, double b1_4, double b1_5)
    {
      m_b1[0] = b1_1;
      m_b1[1] = b1_2;
      m_b1[2] = b1_3;
      m_b1[3] = b1_4;
      m_b1[4] = b1_5;
    }

    /** set the c1 cluster size dependent parameters*/
    void set_c1(double c1_1, double c1_2, double c1_3, double c1_4, double c1_5)
    {
      m_c1[0] = c1_1;
      m_c1[1] = c1_2;
      m_c1[2] = c1_3;
      m_c1[3] = c1_4;
      m_c1[4] = c1_5;
    }


    /** copy constructor */
    SVDPositionErrorFunction(const Belle2::SVDPositionErrorFunction& a);

    /** operator = */
    SVDPositionErrorFunction& operator=(const Belle2::SVDPositionErrorFunction& a);

  private:

    /** total number of cluster sizes */
    static const int maxClusterSize = 5;

    /** function parameters & implementations*/

    /** ID = {0}, rel05: error is the sum in quadrature of
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

      double x = sqrt(clSize) * clSNR;

      if (clSize > maxClusterSize)
        clSize = maxClusterSize;

      double A = m_a1[clSize - 1] / (x + m_a2[clSize - 1]);
      double B = m_b1[clSize - 1] * x;
      double C = m_c1[clSize - 1];

      return sqrt(A * A + B * B + C * C);
    };

    /** current function ID */
    int m_current;

    /** vector of functions for position error, we use the m_current*/
    static std::vector < posErrFunction > m_implementations; //! Do not stream this, please throw it in the WC

    ClassDef(SVDPositionErrorFunction, 2);
  };

}
