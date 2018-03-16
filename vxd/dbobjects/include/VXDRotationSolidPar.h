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
#include <list>
#include <vector>


namespace Belle2 {


  /**
  * The Class for VXD Envelope parameters
  */
  class VXDRotationSolidPar: public TObject {
  public:
    /** Constructor */
    VXDRotationSolidPar(const std::string& name, const std::string& material, const std::string& color,
                        double minPhi, double maxPhi, bool exists):
      m_name(name), m_material(material), m_color(color), m_minPhi(minPhi), m_maxPhi(maxPhi), m_exists(exists)
    {}

    /** Constructor */
    VXDRotationSolidPar():
      m_name(""), m_material(""), m_color(""), m_minPhi(0), m_maxPhi(0), m_exists(false)
    {}

    /** returns if rotation solid has inner points */
    bool getExists() const { return  m_exists; }
    /** get name of the envelope */
    const std::string& getName() const { return m_name; }
    /** get name of the material */
    const std::string& getMaterial() const { return m_material; }
    /** get color */
    const std::string& getColor() const { return m_color; }
    /** get min Phi */
    double getMinPhi() const { return m_minPhi; }
    /** get max Phi */
    double getMaxPhi() const { return m_maxPhi; }
    /** get inner XZ points */
    const std::list< std::pair<double, double> >& getInnerPoints() const { return m_innerPoints; }
    /** get outer XZ points */
    const std::list< std::pair<double, double> >& getOuterPoints() const { return m_outerPoints; }
    /** get inner XZ points */
    std::list< std::pair<double, double> >& getInnerPoints()  { return m_innerPoints; }
    /** get outer XZ points */
    std::list< std::pair<double, double> >& getOuterPoints()  { return m_outerPoints; }

  private:
    /** Name of rotation solid (envelope)*/
    std::string m_name;
    /** Name of the material */
    std::string m_material;
    /** Name of color */
    std::string m_color;
    /** Minimum phi angle */
    double m_minPhi;
    /** Maximum phi angle */
    double m_maxPhi;
    /** Flags rotation solid (envelope) exists */
    bool m_exists;
    //! List of inner points in the ZX plane
    std::list< std::pair<double, double> > m_innerPoints;
    //! List of outer points in the ZX plane
    std::list< std::pair<double, double> > m_outerPoints;

    ClassDef(VXDRotationSolidPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2

