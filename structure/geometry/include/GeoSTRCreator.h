/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOSTRCREATOR_H
#define GEOSTRCREATOR_H

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <string>
#include <vector>

class G4LogicalVolume;
//class G4Polycone

namespace Belle2 {

  namespace structure {

    //!  The GeoSTRCreator class.
    /*!
       The creator for the Belle II Structure.
    */
    class GeoSTRCreator : public geometry::CreatorBase {

    public:

      //! Constructor of the GeoSTRCreator class.
      GeoSTRCreator();

      //! The destructor of the GeoSTRCreator class.
      virtual ~GeoSTRCreator();

      //! Creates the ROOT Objects for the structure geometry.
      /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
      */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

    protected:

    private:

    };
  }
}

#endif /* GEOSTRCREATOR_H */
