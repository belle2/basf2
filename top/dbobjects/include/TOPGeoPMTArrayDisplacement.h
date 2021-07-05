/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBase.h>

namespace Belle2 {

  /**
   * Displacement parameters of MCP-PMT array.
   * To displace: rotation first, then translation
   */
  class TOPGeoPMTArrayDisplacement: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoPMTArrayDisplacement()
    {}

    /**
     * Full constructor
     * @param x translation in x
     * @param y translation in y
     * @param alpha rotation angle around z
     * @param name object name
     */
    TOPGeoPMTArrayDisplacement(double x, double y, double alpha,
                               const std::string& name = "TOPPMTArrayDisplacement"):
      TOPGeoBase(name), m_x(x), m_y(y), m_alpha(alpha)
    {}

    /**
     * Returns translation in x
     * @return translation in x
     */
    double getX() const {return m_x / s_unit;}

    /**
     * Returns translation in y
     * @return translation in y
     */
    double getY() const {return m_y / s_unit;}

    /**
     * Returns rotation angle (around z)
     * @return rotation angle
     */
    double getAlpha() const {return m_alpha;}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const override {return true;}

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "PMT array displacement parameters") const override;

  private:

    float m_x = 0; /**< translation in x */
    float m_y = 0; /**< translation in y */
    float m_alpha = 0; /**< rotation angle around z */

    ClassDefOverride(TOPGeoPMTArrayDisplacement, 1); /**< ClassDef */

  };

} // end namespace Belle2
