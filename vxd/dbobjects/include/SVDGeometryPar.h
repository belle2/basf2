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

#include <vxd/dbobjects/SVDSensorInfoPar.h>
#include <vxd/dbobjects/VXDGeometryPar.h>

#include <vxd/dbobjects/SVDCoolingPipesPar.h>
#include <vxd/dbobjects/SVDSupportRibsPar.h>
#include <vxd/dbobjects/SVDEndringsPar.h>
#include <vxd/dbobjects/VXDRotationSolidPar.h>


namespace Belle2 {

  class GearDir;


  /**
  * The Class for VXD geometry
  */

  class SVDGeometryPar: public VXDGeometryPar {

  public:
    //! Default constructor
    SVDGeometryPar() {}
    //! Constructor using Gearbox
    explicit SVDGeometryPar(const GearDir& content) :  VXDGeometryPar(content) {  }
    //! Destructor
    ~SVDGeometryPar();
    //! Get geometry parameters from Gearbox
    //void read(const GearDir&);

    /**
     * Read the sensor definitions from the database
     * @param sensors Reference to the database containing the parameters
     */
    VXDSensorInfoBasePar* createSensorInfo(const GearDir& sensor);

    /**
     * Create support structure for VXD Half Shell, that means everything
     * thagt does not depend on layer or sensor alignment
     * @param support Reference to the database containing the parameters
     */
    void createHalfShellSupport(GearDir support);

    /**
     * Create support structure for a VXD Layer
     * @param layer Layer ID to create the support for
     * @param support Reference to the database containing the parameters
     */
    void createLayerSupport(int layer, GearDir support);

    /**
     * Create support structure for a VXD Ladder
     * @param layer Layer ID to create the support for
     * @param support Reference to the database containing the parameters
     */
    void createLadderSupport(int layer, GearDir support);

  private:

    /** Vector of pointers to SensorInfo objects */
    std::vector<SVDSensorInfoPar*> m_SensorInfo;

    /** SVD Half Shell parameters */
    std::vector<VXDRotationSolidPar> m_halfShell;

    /** SVD Endrings parameters */
    std::map<int, SVDEndringsPar> m_endrings;

    /** SVD Cooling Pipes parameters */
    std::map<int, SVDCoolingPipesPar> m_coolingPipes;

    /** SVD SupportRibs parameters */
    std::map<int, SVDSupportRibsPar> m_supportRibs;

    ClassDef(SVDGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

