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

  //! The GeoESTRCreator class.
  /*!
    The creator for the Belle II Endcap Structure
  */
  class GeoESTRCreator : public geometry::CreatorBase {

  public:

    //! Constructor of the GeoESTRCreator class.
    GeoESTRCreator();

    //! Destructor of the GeoESTRCreator class.
    virtual ~GeoESTRCreator();

    //! Creates the ROOT Objects for the structure geometry.
    /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
    */
    virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

  private:

  };

} // end of namespace Belle2

#endif /* GEOESTRCREATOR_H */
