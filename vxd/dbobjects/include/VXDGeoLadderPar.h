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

#include <vxd/dbobjects/VXDGeoSensorPar.h>


namespace Belle2 {

  class GearDir;



  /**
  * The Class for VXD Ladder payload
  */
  class VXDGeoLadderPar: public TObject {
  public:
    //! Default constructor
    VXDGeoLadderPar() {}
    //! Constructor using Gearbox
    explicit VXDGeoLadderPar(const GearDir& content) { read(content); }
    //! Destructor
    ~VXDGeoLadderPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);
  private:
    int m_layer;
    double m_shift;
    double m_radius;
    double m_slantedAngle;
    double m_slantedRadius;
    double m_glueOversize;
    std::string m_glueMaterial;

    std::vector<VXDGeoSensorPar> m_sensors;

    ClassDef(VXDGeoLadderPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

