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

  class GearDir;


  /**
  * The Class for VXD Envelope parameters
  */
  class VXDRotationSolidPar: public TObject {
  public:
    //! Default constructor
    VXDRotationSolidPar() {}
    //! Constructor using Gearbox
    explicit VXDRotationSolidPar(const GearDir& content) { read(content); }
    //! Destructor
    ~VXDRotationSolidPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

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
    std::list< std::pair<double, double> > getInnerPoints() const { return m_innerPoints; }
    /** get outer XZ points */
    std::list< std::pair<double, double> > getOuterPoints() const { return m_outerPoints; }

  private:
    std::string m_name;
    std::string m_material;
    std::string m_color;
    double m_minPhi;
    double m_maxPhi;
    //! List of points in the ZX plane
    std::list< std::pair<double, double> > m_innerPoints;
    std::list< std::pair<double, double> > m_outerPoints;
    bool m_exists {false};

    ClassDef(VXDRotationSolidPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2

