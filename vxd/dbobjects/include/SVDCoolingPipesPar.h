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
  * The Class for SVD Cooling Pipes
  */

  class SVDCoolingPipesPar: public TObject {

  public:

    //! Default constructor
    SVDCoolingPipesPar() {}
    //! Constructor using Gearbox
    explicit SVDCoolingPipesPar(int layer, const GearDir& support) { read(layer, support); }
    //! Get geometry parameters from Gearbox
    void read(int, const GearDir&);
    /** get material */
    const std::string& getMaterial() const { return m_material; }
    /** get outer diameter */
    double getOuterDiameter() const { return m_outerDiameter; }
    /** get wall thickness */
    double getWallThickness() const { return m_wallThickness; }
    /** get nPipes */
    int getNPipes() const { return m_nPipes; }
    /** get start phi */
    double getStartPhi() const { return m_startPhi; }
    /** get delta phi */
    double getDeltaPhi() const { return m_deltaPhi; }
    /** get radius */
    double getRadius() const { return m_radius; }
    /** get zstart */
    double getZStart() const { return m_zstart; }
    /** get zend */
    double getZEnd() const { return m_zend; }
    /** get deltal */
    double getDeltaL() const { return m_deltaL; }

  private:
    std::string m_material;
    double m_outerDiameter;
    double m_wallThickness;

    int m_nPipes {0};
    double m_startPhi {0};
    double m_deltaPhi {0};
    double m_radius {0};
    double m_zstart {0};
    double m_zend {0};
    double m_deltaL {0};


    ClassDef(SVDCoolingPipesPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };


} // end of namespace Belle2

