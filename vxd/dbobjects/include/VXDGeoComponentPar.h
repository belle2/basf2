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
  * The Class for VXD geometry component
  */
  class VXDGeoComponentPar: public TObject {

  public:

    //! Default constructor
    VXDGeoComponentPar() {}
    //! Constructor using Gearbox
    explicit VXDGeoComponentPar(const GearDir& content) { read(content); }
    //! Destructor
    ~VXDGeoComponentPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

  private:
    std::string m_material;
    std::string m_color;
    double m_width;
    double m_width2;
    double m_length;
    double m_height;
    double m_angle;

    ClassDef(VXDGeoComponentPar, 5);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

