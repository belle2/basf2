/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBase.h>
#include <TVector3.h>
#include <TRotation.h>

namespace Belle2 {

  /**
   * Displacement parameters of a TOP module.
   * To displace module: rotation first (around x, then y, then z), then translation
   * (e.g. going from local to nominal frame)
   */
  class TOPGeoModuleDisplacement: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoModuleDisplacement()
    {}

    /**
     * Full constructor
     */
    TOPGeoModuleDisplacement(double x, double y, double z,
                             double alpha, double beta, double gamma):
      m_x(x), m_y(y), m_z(z), m_alpha(alpha), m_beta(beta), m_gamma(gamma)
    {}

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
    double getAlpha() const {return m_alpha;}

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
     * Returns translation vector (always in Basf2 units!)
     * @return translation vector
     */
    TVector3 getTranslation() const {return TVector3(m_x, m_y, m_z);}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const {return true;}

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Module displacement parameters") const;

  private:

    float m_x = 0; /**< translation in x */
    float m_y = 0; /**< translation in y */
    float m_z = 0; /**< translation in z */
    float m_alpha = 0; /**< rotation angle around x */
    float m_beta = 0;  /**< rotation angle around y */
    float m_gamma = 0; /**< rotation angle around z */

    ClassDef(TOPGeoModuleDisplacement, 1); /**< ClassDef */

  };

} // end namespace Belle2
