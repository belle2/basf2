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
#include <vector>


namespace Belle2 {

  class GearDir;

  /**
  * The Class for VXD Endrings Layer
  */

  class SVDEndringsLayerPar: public TObject {

  public:

    //! Default constructor
    SVDEndringsLayerPar() {}
    //! Constructor using Gearbox
    explicit SVDEndringsLayerPar(const GearDir& content) { read(content); }
    //! Destructor
    ~SVDEndringsLayerPar() {}
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

    ClassDef(SVDEndringsLayerPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for VXD Endring
  */

  class SVDEndringsPar: public TObject {

  public:

    //! Default constructor
    SVDEndringsPar() {}
    //! Constructor using Gearbox
    explicit SVDEndringsPar(const GearDir& content) { read(content); }
    //! Destructor
    ~SVDEndringsPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

  private:

    std::string m_material;
    double m_length;
    double m_gapWidth;
    double m_baseThickness;

    std::vector<SVDEndringsLayerPar> m_layers;

    ClassDef(SVDEndringsPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };


} // end of namespace Belle2

