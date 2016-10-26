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
    //! Destructor
    ~SVDEndringsTypePar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

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
    //! Destructor
    ~SVDEndringsPar() {}
    //! Get geometry parameters from Gearbox
    void read(int, const GearDir&);

  private:

    std::string m_material;
    double m_length;
    double m_gapWidth;
    double m_baseThickness;

    std::vector<SVDEndringsTypePar> m_types;

    ClassDef(SVDEndringsPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };


} // end of namespace Belle2

