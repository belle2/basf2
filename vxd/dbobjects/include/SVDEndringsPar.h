/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <string>
#include <map>


namespace Belle2 {

  class GearDir;

  /**
  * The Class for SVD Endring Type
  */

  class SVDEndringsTypePar: public TObject {

  public:

    //! Default constructor
    SVDEndringsTypePar() {}
    //! Constructor using Gearbox
    explicit SVDEndringsTypePar(const GearDir& endring) { read(endring); }
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);
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

    std::string m_name;
    double m_z;
    double m_baseRadius;
    double m_innerRadius;
    double m_outerRadius;
    double m_horizontalBarWidth;
    double m_verticalBarWidth;

    ClassDef(SVDEndringsTypePar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for SVD Endring
  */

  class SVDEndringsPar: public TObject {

  public:

    //! Default constructor
    SVDEndringsPar() {}
    //! Constructor using Gearbox
    explicit SVDEndringsPar(int layer, const GearDir& support) { read(layer, support); }
    //! Get geometry parameters from Gearbox
    void read(int, const GearDir&);
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

  private:

    std::string m_material;
    double m_length;
    double m_gapWidth;
    double m_baseThickness;

    std::vector<SVDEndringsTypePar> m_types;

    ClassDef(SVDEndringsPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };


} // end of namespace Belle2

