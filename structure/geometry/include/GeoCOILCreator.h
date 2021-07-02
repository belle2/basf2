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

#include <framework/database/IntervalOfValidity.h>

#include <structure/dbobjects/COILGeometryPar.h>

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


      //! Creates the Geant4 objects for the structure geometry from Gearbox
      /*!
      This is the 'old' way of building the geometry from Gearbox (xml files). Expected to be deprecated in the future.
        \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
      \param topVolume Geant4 logical top volume.
      \param type Geometry type.
      */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override;

      /** Create the configuration objects and save them in the Database.  If
       * more than one object is needed adjust accordingly */
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override;

      /** Create the geometry from the Database */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override;


    protected:

    private:

      //! Creates a parameter object from the Gearbox XML parameters.
      /*! If more  than one object is created these could be assigned to members or you
       * could return a tuple.
      \param param A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
       */
      COILGeometryPar readConfiguration(const GearDir& param);


      //! Creates the ROOT Objects for the coil geometry.
      /*!
      \param parameters A database object containing the geometry information
      \param topVolume A Geant4 volume where to place the item
      */
      virtual void createGeometry(const COILGeometryPar& parameters, G4LogicalVolume& topVolume, geometry::GeometryTypes);

      //! Vector of pointers to G4VisAttributes
      std::vector<G4VisAttributes*> m_VisAttributes;

    };
  }
}

#endif /* GEOCOILCREATOR_H */
