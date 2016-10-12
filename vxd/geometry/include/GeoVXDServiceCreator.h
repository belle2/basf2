/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Benjamin Schwenker                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOVXDSERVICECREATOR_H
#define GEOVXDSERVICECREATOR_H

#include <geometry/CreatorBase.h>

#include <framework/logging/Logger.h>
#include <vxd/dbobjects/VXDServiceGeometryPar.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>

class G4Material;

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the VXD */
  namespace VXD {
    /** The creator for the VXD Service geometry of the Belle II detector. */
    class GeoVXDServiceCreator : public geometry::CreatorBase {
    private:
      //! Create a parameter object from the Gearbox XML parameters.
      VXDServiceGeometryPar createConfiguration(const GearDir& param)
      {
        VXDServiceGeometryPar vxdServiceGeometryPar;
        vxdServiceGeometryPar.read(param);
        return vxdServiceGeometryPar;
      };

      //! Create the geometry from a parameter object.
      void createGeometry(const VXDServiceGeometryPar& parameters, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    public:
      /** Default constructor is sufficient in this case */
      GeoVXDServiceCreator() = default;

      /**
       * Creates the Geant4 Objects for the VXD Service geometry.
       * @param content A reference to the content part of the parameter
       *                description, which should to be used to create the Geant4
       *                objects.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

      /** Create the configuration objects and save them in the Database.  If
       * more than one object is needed adjust accordingly */
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override
      {
        DBImportObjPtr<VXDServiceGeometryPar> importObj;
        importObj.construct(createConfiguration(content));
        importObj.import(iov);
      }

      /** Create the geometry from the Database */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        DBObjPtr<VXDServiceGeometryPar> dbObj;
        if (!dbObj) {
          // Check that we found the object and if not report the problem
          B2FATAL("No configuration for " << name << " found.");
        }
        createGeometry(*dbObj, topVolume, type);
      }

    private:
      /** Create the dock boxes */
      void createDockBoxes(const GearDir& content, G4LogicalVolume& topVolume);

      /** Default Material, inherited from topVolume */
      G4Material* m_defaultMaterial {0};
    };
  } // namespace VXD
} //namespace Belle2

#endif /* GEOVXDSERVICECREATOR_H */
