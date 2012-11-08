/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOESTRCREATOR_H
#define GEOESTRCREATOR_H

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <vector>
#include <string>

namespace Belle2 {

  /**
   * Creator of the Belle II Endcap Structure.
   * @details
   * This class does not actually create the geometry;
   * it is created by the GeoEKLMBelleII class.
   */
  class GeoESTRCreator : public geometry::CreatorBase {

  public:

    /**
     * Constructor.
     */
    GeoESTRCreator();

    /**
     * Destructor of the GeoESTRCreator class.
     */
    ~GeoESTRCreator();

    /**
     * Function to actually create the geometry, has to be overridden by
     * derived classes
     * @param content   GearDir pointing to the parameters which
     *        should be used for construction
     * @param topVolume Top volume in which the geometry has to be
     *        placed
     * @param type    Type of geometry to be build
     */
    void create(const GearDir& content, G4LogicalVolume& topVolume,
                geometry::GeometryTypes type);

  private:

  };

} // end of namespace Belle2

#endif /* GEOESTRCREATOR_H */
