/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <string>


namespace Belle2 {



  /**
  * The Class for SVD Endring Type
  */
  class SVDEndringsTypePar: public TObject {

  public:
    /** Constructor */
    SVDEndringsTypePar(const std::string& name, double z, double baseRadius, double innerRadius, double outerRadius,
                       double horizontalBarWidth, double verticalBarWidth):
      m_name(name), m_z(z), m_baseRadius(baseRadius), m_innerRadius(innerRadius), m_outerRadius(outerRadius),
      m_horizontalBarWidth(horizontalBarWidth), m_verticalBarWidth(verticalBarWidth)
    {}
    /** Constructor */
    SVDEndringsTypePar():
      m_name(""), m_z(0), m_baseRadius(0), m_innerRadius(0), m_outerRadius(0),
      m_horizontalBarWidth(0), m_verticalBarWidth(0)
    {}

    /** get name */
    const std::string& getName() const { return m_name; }
    /** get z */
    double getZ() const { return m_z; }
    /** get base radius */
    double getBaseRadius() const { return m_baseRadius; }
    /** get inner radius */
    double getInnerRadius() const { return m_innerRadius; }
    /** get outer radius */
    double getOuterRadius() const { return m_outerRadius; }
    /** get horizontal bar width */
    double getHorizontalBarWidth() const { return m_horizontalBarWidth; }
    /** get vertical bar width */
    double getVerticalBarWidth() const { return m_verticalBarWidth; }

  private:

    std::string m_name; /**< name*/
    double m_z; /**< z*/
    double m_baseRadius; /**< base radius*/
    double m_innerRadius; /**< inner radius*/
    double m_outerRadius; /**< outer radius*/
    double m_horizontalBarWidth; /**< horizontal bar width*/
    double m_verticalBarWidth; /**< vertical bar width*/

    ClassDef(SVDEndringsTypePar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for SVD Endring
  */

  class SVDEndringsPar: public TObject {

  public:
    /** Constructor */
    SVDEndringsPar(const std::string& material, double length, double gapWidth, double baseThickness):
      m_material(material),  m_length(length), m_gapWidth(gapWidth), m_baseThickness(baseThickness)
    {}
    /** Constructor */
    SVDEndringsPar():
      m_material(""),  m_length(0), m_gapWidth(0), m_baseThickness(0)
    {}

    /** get material */
    const std::string& getMaterial() const { return m_material; }
    /** get length */
    double getLength() const { return m_length; }
    /** get gap width */
    double getGapWidth() const { return m_gapWidth; }
    /** get base thickness */
    double getBaseThickness() const { return m_baseThickness; }
    /** get types (forward/backward) */
    const std::vector<SVDEndringsTypePar>& getTypes() const { return m_types; }
    /** get types (forward/backward) */
    std::vector<SVDEndringsTypePar>& getTypes() { return m_types; }

  private:

    std::string m_material; /**< material name */
    double m_length; /**< length*/
    double m_gapWidth; /**< gap width*/
    double m_baseThickness; /**< base thickness*/

    std::vector<SVDEndringsTypePar> m_types; /**< vector of types*/

    ClassDef(SVDEndringsPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };


} // end of namespace Belle2

