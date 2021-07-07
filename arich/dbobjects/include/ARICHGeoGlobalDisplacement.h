/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <arich/dbobjects/ARICHGeoBase.h>

namespace Belle2 {

  /**
   * Global displacement parameters for ARICH.
   */
  class ARICHGeoGlobalDisplacement: public ARICHGeoBase {
  public:

    /**
     * Default constructor
     */
    ARICHGeoGlobalDisplacement()
    {}

    /**
     * Full constructor
     * @param x displacement in x
     * @param y displacement in y
     * @param z displacement in z
     * @param alpha rotation angle around x
     * @param beta rotation angle around y
     * @param gamma rotation angle around z
     */
    ARICHGeoGlobalDisplacement(double x, double y, double z,
                               double alpha, double beta, double gamma):
      m_x(x), m_y(y), m_z(z),
      m_alpha(alpha), m_beta(beta), m_gamma(gamma)
    {}

    ~ARICHGeoGlobalDisplacement()
    {}

    /**
     * Sets translation in x
     * @param x translation in x
     */
    void setX(double x) {m_x = x;}

    /**
     * Sets translation in y
     * @param y translation in y
     */
    void setY(double y) {m_y = y;}

    /**
     * Sets translation in z
     * @param z translation in z
     */
    void setZ(double z) {m_z = z;}

    /**
     * Sets rotation around x
     * @param alpha rotation angle around x
     */
    void setAlpha(double alpha) {m_alpha = alpha;}


    /**
     * Sets rotation around y
     * @param alpha rotation angle around y
     */
    void setBeta(double beta) {m_beta = beta;}


    /**
     * Sets rotation around z
     * @param alpha rotation angle around z
     */
    void setGamma(double gamma) {m_gamma = gamma;}

    /**
     * Returns translation in x
     * @return x coordinate
     */
    double getX() const {return m_x / s_unit;}

    /**
     * Returns translation in y
     * @return y coordinate
     */
    double getY() const {return m_y / s_unit;}

    /**
     * Returns translation in z
     * @return z coordinate
     */
    double getZ() const {return m_z / s_unit;}

    /**
     * Returns rotation angle around x
     * @return rotation angle
     */
    double getAlpha() const {return  m_alpha;}

    /**
     * Returns rotation angle around y
     * @return rotation angle
     */
    double getBeta() const {return m_beta;}

    /**
     * Returns rotation angle around z
     * @return rotation angle
     */
    double getGamma() const {return m_gamma;}


  private:

    float m_x = 0; /**< translation in x */
    float m_y = 0; /**< translation in y */
    float m_z = 0; /**< translation in z */
    float m_alpha = 0; /**< rotation angle around x */
    float m_beta = 0;  /**< rotation angle around y */
    float m_gamma = 0; /**< rotation angle around z */

    ClassDef(ARICHGeoGlobalDisplacement, 1); /**< ClassDef */

  };

} // end namespace Belle2
