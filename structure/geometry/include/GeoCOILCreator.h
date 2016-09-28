/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOCOILCREATOR_H
#define GEOCOILCREATOR_H

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <string>
#include <vector>

class G4LogicalVolume;
class G4VisAttributes;
//class G4Polycone

namespace Belle2 {

  namespace coil {

    //!  The GeoCoilCreator class.
    /*!
       The creator for the the Belle II coil.
    */
    class GeoCOILCreator : public geometry::CreatorBase {

    public:

      //! Constructor of the GeoCOILCreator class.
      GeoCOILCreator();

      //! The destructor of the GeoCOILCreator class.
      virtual ~GeoCOILCreator();

      //! Creates the ROOT Objects for the coil geometry.
      /*!
      \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
      */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

    protected:

    private:

      //! Vector of pointers to G4VisAttributes
      std::vector<G4VisAttributes*> m_VisAttributes;

    };
  }
}

#endif /* GEOCOILCREATOR_H */
