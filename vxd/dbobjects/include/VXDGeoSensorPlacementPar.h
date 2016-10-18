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


namespace Belle2 {

  class GearDir;

  /**
  * The Class for VXD global paramter payload
  */

  class VXDGeoSensorPlacementPar: public TObject {

  public:
    //! Default constructor
    VXDGeoSensorPlacementPar() {}
    //! Constructor using Gearbox
    explicit VXDGeoSensorPlacementPar(const GearDir& content) { read(content); }
    //! Destructor
    ~VXDGeoSensorPlacementPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

  private:
    int m_id;
    std::string m_type;
    double m_length;
    bool m_flipU;
    bool m_flipV;
    bool m_flipW;

    ClassDef(VXDGeoSensorPlacementPar, 5);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

