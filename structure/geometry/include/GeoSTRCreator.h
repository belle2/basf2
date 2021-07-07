/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef GEOSTRCREATOR_H
#define GEOSTRCREATOR_H

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>

#include <structure/dbobjects/STRGeometryPar.h>

#include <string>

class G4LogicalVolume;

namespace Belle2 {

  namespace structure {

    //!  The GeoSTRCreator class to create the Belle2 structure geometry
    /*!
       The creator for the Belle II structural elements such as radiation shields and pole piece
    */
    class GeoSTRCreator : public geometry::CreatorBase {

    public:

      //! The Constructor of the GeoSTRCreator class.
      GeoSTRCreator();

      //! The destructor of the GeoSTRCreator class.
      virtual ~GeoSTRCreator();

      //! Creates the Geant4 objects for the structure geometry from Gearbox
      /*!
      This is the 'old' way of building the geometry from Gearbox (xml files). Expected to be deprecated in the future.
        \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
      \param topVolume Geant4 logical top volume.
      \param type Geometry type.
      */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        STRGeometryPar config = createConfiguration(content);
        createGeometry(config, topVolume, type);
      }

      /** Create the configuration objects and save them in the Database.  If
       * more than one object is needed adjust accordingly */
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override
      {
        DBImportObjPtr<STRGeometryPar> importObj;
        importObj.construct(createConfiguration(content));
        importObj.import(iov);
      }

      /** Create the geometry from the Database */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type)\
      override
      {
        DBObjPtr<STRGeometryPar> dbObj;
        if (!dbObj) {
          // Check that we found the object and if not report the problem
          B2FATAL("No configuration for " << name << " found.");
        }
        createGeometry(*dbObj, topVolume, type);
      }


    protected:

    private:
      /** Create a parameter object from the Gearbox XML parameters. If more
       * than one object is created these could be assigned to members or you
       * could return a tuple. */
      STRGeometryPar createConfiguration(const GearDir& param);


      /** Create the geometry from a parameter object. If more than one object
       * is needed these could instead be taken from member variables or the
       * number of parameters could be increased. */
      void createGeometry(const STRGeometryPar& parameters, G4LogicalVolume& topVolume,
                          geometry::GeometryTypes type);


      /** Read xml files from GearDir for one of the ECL shields
       *
       */
      void readShield(const GearDir& content, STRGeometryPar& parameters, std::string side);


      /** Read xml files from GearDir for one of the Pole Pieces
       *
       */
      void readPole(const GearDir& content, STRGeometryPar& parameters, std::string side);


    };
  }
}

#endif /* GEOSTRCREATOR_H */
