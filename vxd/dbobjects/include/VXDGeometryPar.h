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
#include <vector>

#include <vxd/dbobjects/VXDGlobalPar.h>
#include <vxd/dbobjects/VXDAlignmentPar.h>
#include <vxd/dbobjects/VXDEnvelopePar.h>
#include <vxd/dbobjects/VXDHalfShellPar.h>

#include <vxd/dbobjects/VXDGeoComponentPar.h>
#include <vxd/dbobjects/VXDGeoSensorPar.h>
#include <vxd/dbobjects/VXDGeoLadderPar.h>
#include <vxd/dbobjects/VXDGeoRadiationSensorsPar.h>
#include <vxd/dbobjects/VXDSensorInfoBasePar.h>

namespace Belle2 {

  class GearDir;


  /**
  * The Class for VXD geometry
  */

  class VXDGeometryPar: public TObject {
  public:
    //! Default constructor
    VXDGeometryPar() {}
    //! Constructor using Gearbox
    explicit VXDGeometryPar(const std::string& prefix, const GearDir& content) { read(prefix, content); }
    //! Destructor
    virtual ~VXDGeometryPar() {}
    //! Get geometry parameters from Gearbox
    void read(const std::string&, const GearDir&);

    /**
     * Return vector of VXDGeoPlacements with all the components defined inside a given path
     */
    std::vector<VXDGeoPlacementPar> getSubComponents(GearDir path);

    /**
     * Read the sensor definitions from the database
     * @param sensor Reference to the database containing the parameters
     */
    virtual VXDSensorInfoBasePar* createSensorInfo(const GearDir& sensor) = 0;

    /**
     * Create support structure for VXD Half Shell, that means everything
     * thagt does not depend on layer or sensor alignment
     * @param support Reference to the database containing the parameters
     */
    virtual void createHalfShellSupport(GearDir support) = 0;

    /**
     * Create support structure for a VXD Layer
     * @param layer Layer ID to create the support for
     * @param support Reference to the database containing the parameters
     */
    virtual void createLayerSupport(int layer, GearDir support) = 0;

    /**
     * Create support structure for a VXD Ladder
     * @param layer Layer ID to create the support for
     * @param support Reference to the database containing the parameters
     */
    virtual void createLadderSupport(int layer, GearDir support) = 0;

    /**
     * Read parameters for given layer and store in m_ladders
     */
    virtual void setCurrentLayer(int layer, GearDir components);

  private:

    /** Prefix to prepend to all volume names */
    std::string m_prefix {""};
    /** Container for a few general parameters */
    VXDGlobalPar m_globals;
    /** Alignment parameters for all components */
    std::map<std::string, VXDAlignmentPar> m_alignment;
    /** Detector envelope parameters */
    VXDEnvelopePar m_envelope;
    /** Container for half shells, can be used to loop over sensors */
    std::vector<VXDHalfShellPar> m_halfShells;
    /** Cache of all previously created components */
    std::map<std::string, VXDGeoComponentPar> m_componentCache;
    /** Map containing Information about all defined sensor types */
    std::map<std::string, VXDGeoSensorPar> m_sensorMap;
    /** Diamond radiation sensor "sub creator" */
    VXDGeoRadiationSensorsPar m_radiationsensors;
    /** Parameters of the detector ladders */
    std::map<int, VXDGeoLadderPar> m_ladders;


    ClassDef(VXDGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

