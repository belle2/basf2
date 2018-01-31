/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Benjamin Schwenker                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOVXDMATERIALCREATOR_H
#define GEOVXDMATERIALCREATOR_H

#include <geometry/CreatorBase.h>

#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>

#include <vxd/dbobjects/VXDMaterialGeometryPar.h>

class G4Material;

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the VXD */
  namespace VXD {

    /** The creator for the VXD Material geometry of the Belle II detector. */
    class GeoVXDMaterialCreator : public geometry::CreatorBase {
    private:
      //! Create a parameter object from the Gearbox XML parameters.
      VXDMaterialGeometryPar createConfiguration(const GearDir& param);

      //! Create the geometry from a parameter object.
      void createGeometry(const VXDMaterialGeometryPar& parameters, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

      //! Create BEAST2 Material between CDC and ECL from a parameter object.
      void createBEAST2Material(const VXDMaterialGeometryPar& parameters, G4LogicalVolume*& logical_gap);

      //!  Create G4Tube.
      void createTube(const double rmin, const double rmax,
                      const double SPhi, const double DPhi,
                      const double thick, const double posZ,
                      const int id, G4Material* med,
                      const int blockid, G4LogicalVolume*& top);

      //! Create G4Cone.
      void createCone(const double rmin1, const double rmax1,
                      const double rmin2, const double rmax2,
                      const double thick, const double SPhi, const double DPhi, const double posz,
                      G4Material* med, const std::string& name, G4LogicalVolume*& top);


      //! Create BEAST2 Material between between barrel and end of ECL.
      void beast2ECLmaterial(G4LogicalVolume*& logical_gap_back, G4LogicalVolume*& logical_gap_for);

    public:
      /** Default constructor is sufficient in this case */
      GeoVXDMaterialCreator()  = default;
//      {
//     logical_gap = 0;
//     physical_gap = 0;
//      }


      /** The old create member: create the configuration object(s) on the fly
       * and call the geometry creation routine.*/
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        VXDMaterialGeometryPar config = createConfiguration(content);
        createGeometry(config, topVolume, type);
      }

      /** Create the configuration objects and save them in the Database.  If
       * more than one object is needed adjust accordingly */
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override
      {
        DBImportObjPtr<VXDMaterialGeometryPar> importObj;
        importObj.construct(createConfiguration(content));
        importObj.import(iov);
      }

      /** Create the geometry from the Database */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        DBObjPtr<VXDMaterialGeometryPar> dbObj;
        if (!dbObj) {
          // Check that we found the object and if not report the problem
          B2FATAL("No configuration for " << name << " found.");
        }
        createGeometry(*dbObj, topVolume, type);
      }

    private:

      /** Default Material, inherited from topVolume */
      G4Material* m_defaultMaterial {0};
      //! GAP G4 logical volume.
//      G4LogicalVolume* logical_gap;

      //! GAP G4 physical volume.
//     G4VPhysicalVolume* physical_gap;

    };
  } // namespace VXD
} //namespace Belle2

#endif /* GEOVXDMATERIALCREATOR_H */
