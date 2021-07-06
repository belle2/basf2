/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* This is just a short example header to show how to implement the geometry
 * from the database. For simplicity this is just in one cc file, the division
 * into suitable header and cc files is left as exercise for the reader.
 *
 * The old CreatorBase had only one public member called
 * create(). Now we have three:
 *
 * 1. create() to create the Geometry from the Gearbox as before
 * 2. createFromDB() to create the Geometry from the Database
 * 3. createPayloads() to create Database configuration from Gearbox
 *
 * (1) will be deprecated in the future and (3) will probably be split into a
 * separate class once (1) is gone. For now we require all three to be
 * implemented.
 *
 * The following is just an example how all three interface members should be
 * implemented using two private members: One to convert the Gearbox parameters
 * to an Parameter object and one to Create the Geometry from this object. This
 * has not been done more generic to a) allow for more then one parameter
 * object to be created from the Gearbox parameters and b) don't bloat the code
 * with endless templates. But all Creators should follow this scheme.
 *
 * If you don't I do have are a very particular set of skills, skills I have
 * acquired over a very long career. Skills that make me a nightmare for people
 * like you. I will look for you, I will find you and I will annoy you until
 * you fix it.
 */

#include <geometry/CreatorBase.h>
#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>

namespace Belle2 {
  /** Class containing all the parameters needed to create the geometry and
   * suitable to save into a ROOT file to be used from the Database */
  class MyDBPayloadClass: public TObject {
    /** Database objects need a dictionary */
    ClassDef(MyDBPayloadClass, 1);
  };

  /** Very simple Creator class which actually does not do anything but shows
   * how creators should implement loading the geometry from database */
  class MyDBCreator: public geometry::CreatorBase {
  private:
    /** Create a parameter object from the Gearbox XML parameters. If more
     * than one object is created these could be assigned to members or you
     * could return a tuple. */
    MyDBPayloadClass createConfiguration(const GearDir& param);
    /** Create the geometry from a parameter object. If more than one object
     * is needed these could instead be taken from member variables or the
     * number of parameters could be increased. */
    void createGeometry(const MyDBPayloadClass& parameters, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
  public:
    /** The old create member: create the configuration object(s) on the fly
     * and call the geometry creation routine. If more than one object is
     * needed adjust accordingly */
    virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
    {
      MyDBPayloadClass config = createConfiguration(content);
      createGeometry(config, topVolume, type);
    }

    /** Create the configuration objects and save them in the Database.  If
     * more than one object is needed adjust accordingly */
    virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override
    {
      DBImportObjPtr<MyDBPayloadClass> importObj;
      importObj.construct(createConfiguration(content));
      importObj.import(iov);
    }

    /** Create the geometry from the Database */
    virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
    {
      DBObjPtr<MyDBPayloadClass> dbObj;
      if (!dbObj) {
        // Check that we found the object and if not report the problem
        B2FATAL("No configuration for " << name << " found.");
      }
      createGeometry(*dbObj, topVolume, type);
    }
  };
}; //end of namespace Belle2

