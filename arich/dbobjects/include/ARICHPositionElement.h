/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <arich/dbobjects/ARICHGeoBase.h>
#include <framework/gearbox/Unit.h>
#include <TVector3.h>
#include <TRotation.h>
#include <iostream>

namespace Belle2 {

  /**
   * Position element for ARICH. Holding parameters for displacement and alignment of arich components
   */
  class ARICHPositionElement: public ARICHGeoBase {
  public:

    /**
     * Default constructor
     */
    ARICHPositionElement()
    {}

    /**
     * Full constructor
     * @param x translation in x
     * @param y translation in y
     * @param z translation in z
     * @param alpha rotation angle around x
     * @param beta rotation angle around y
     * @param gamma rotation angle around z
     * @param name object name
     */
    ARICHPositionElement(double x, double y, double z,
                         double alpha, double beta, double gamma, const std::string& name = "ARICHAlignment"):
      ARICHGeoBase(name),
      m_x(x), m_y(y), m_z(z),
      m_alpha(alpha), m_beta(beta), m_gamma(gamma)
    {}

    ~ARICHPositionElement()
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
     * Sets x,y from r,phi
     * @param r radius
     * @param phi phi angle
     */
    void setRPhi(double r, double phi)
    {
      m_x = r * cos(phi / Unit::rad);
      m_y = r * sin(phi / Unit::rad);
    }

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
     * Shift the existing values of parameters
     * @param dx translation in x
     * @param dy translation in y
     * @param dz translation in z
     * @param dalpha rotation angle around x
     * @param dbeta rotation angle around y
     * @param dgamma rotation angle around z
     */
    void addShift(double dx, double dy, double dz, double dalpha, double dbeta, double dgamma)
    {
      m_x += dx;
      m_y += dy;
      m_z += dz;
      m_alpha += dalpha;
      m_beta += dbeta;
      m_gamma += dgamma;
    };

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

    /**
     * Returns rotation matrix
     * @return rotation matrix
     */
    TRotation getRotation() const
    {
      TRotation rot;
      rot.RotateX(m_alpha).RotateY(m_beta).RotateZ(m_gamma);
      return rot;
    }

    /**
     * Returns translation vector (always in basf2 units!)
     * @return translation vector
     */
    TVector3 getTranslation() const {return TVector3(m_x, m_y, m_z);}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const override {return true;}

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Parmeters of position element") const override
    {
      std::cout << title << std::endl;
      std::cout << "translations (x,y,z): " << m_x << " " << m_y << " " << m_z << std::endl;
      std::cout << "rotations (x,y,z axis): " << m_alpha << " " << m_beta << " " << m_gamma << std::endl;

    }

  private:

    float m_x = 0; /**< translation in x */
    float m_y = 0; /**< translation in y */
    float m_z = 0; /**< translation in z */
    float m_alpha = 0; /**< rotation angle around x */
    float m_beta = 0;  /**< rotation angle around y */
    float m_gamma = 0; /**< rotation angle around z */

    ClassDefOverride(ARICHPositionElement, 1); /**< ClassDef */

  };

} // end namespace Belle2
