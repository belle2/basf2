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

#include <vxd/dbobjects/PXDSensorInfoPar.h>
#include <vxd/dbobjects/VXDGeometryPar.h>

namespace Belle2 {

  class GearDir;


  /**
  * The Class for VXD geometry
  */

  class PXDGeometryPar: public VXDGeometryPar {
  public:
    //! Default constructor
    PXDGeometryPar() {}
    //! Constructor using Gearbox
    explicit PXDGeometryPar(const GearDir& content) : VXDGeometryPar(content) {  }
    //! Destructor
    ~PXDGeometryPar();
    //! Get geometry parameters from Gearbox
    //void read(const GearDir&);

    /**
     * Read the sensor definitions from the database
     * @param sensors Reference to the database containing the parameters
       */
    virtual VXDSensorInfoBasePar* createSensorInfo(const GearDir& sensor);

  private:

    /** Vector of points to SensorInfo objects */
    std::vector<PXDSensorInfoPar*> m_SensorInfo;

    ClassDef(PXDGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

