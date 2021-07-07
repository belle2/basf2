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
  * The Class for SVD Cooling Pipes
  */

  class SVDCoolingPipesPar: public TObject {

  public:

    /** Constructor */
    SVDCoolingPipesPar(const std::string& material, double outerDiameter, double wallThickness, int nPipes,
                       double startPhi, double deltaPhi, double radius, double zstart, double zend, double deltaL = 0):
      m_material(material), m_outerDiameter(outerDiameter), m_wallThickness(wallThickness), m_nPipes(nPipes),
      m_startPhi(startPhi), m_deltaPhi(deltaPhi), m_radius(radius), m_zstart(zstart), m_zend(zend), m_deltaL(deltaL)
    {}
    /** Constructor */
    SVDCoolingPipesPar():
      m_material(""), m_outerDiameter(0.), m_wallThickness(0.), m_nPipes(0),
      m_startPhi(0), m_deltaPhi(0), m_radius(0), m_zstart(0), m_zend(0), m_deltaL(0)
    {}

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
    /** set deltal */
    void setDeltaL(double deltaL)  { m_deltaL = deltaL; }

  private:
    std::string m_material; /**< material*/
    double m_outerDiameter; /**< outer diameter*/
    double m_wallThickness; /**< wall thickness*/

    int m_nPipes; /**<n pipes*/
    double m_startPhi; /**< start phi angle*/
    double m_deltaPhi; /**< delta phi angle*/
    double m_radius; /**<radius*/
    double m_zstart; /**< start z*/
    double m_zend; /**< end z*/
    double m_deltaL; /**< delta L*/


    ClassDef(SVDCoolingPipesPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };


} // end of namespace Belle2

