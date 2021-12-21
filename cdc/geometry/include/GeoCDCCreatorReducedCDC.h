/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <geometry/CreatorBase.h>

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>

#include <cdc/simulation/CDCSensitiveDetector.h>
#include <cdc/dbobjects/CDCGeometry.h>

class G4LogicalVolume;
class G4VPhysicalVolume;
class G4VisAttributes;
class G4UserLimits;

namespace Belle2 {

  class BkgSensitiveDetector;

  namespace CDC {

    //!  The GeoCDCCreatorReducedCDC class.
    /*!
       The creator for the CDC geometry of the Belle II detector.
    */
    class GeoCDCCreatorReducedCDC : public geometry::CreatorBase {

    public:

      //! Constructor of the GeoCDCCreatorReducedCDC class.
      GeoCDCCreatorReducedCDC();

      //! The destructor of the GeoCDCCreatorReducedCDC class.
      ~GeoCDCCreatorReducedCDC();

      //! Creates the ROOT Objects for the CDC geometry.
      /*!
        \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
      \param topVolume Geant4 top logical volume.
      \param type Geometry type.
      */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        CDCGeometry config = createConfiguration(content);
        createGeometry(config, topVolume, type);
      }

      /**
       * Create geometry from DB.
       */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        DBObjPtr<CDCGeometry> geo;
        if (!geo) {
          B2FATAL("No configuration for " << name << " found.");
        }
        createGeometry(*geo, topVolume, type);
      }

      /**
       * Create payloads.
       */
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override
      {
        DBImportObjPtr<CDCGeometry> importObj;
        importObj.construct(createConfiguration(content));
        importObj.import(iov);
      }

      /**
       * Create CDC covers from gear box.
       */
      void createCovers(const GearDir& content);

      /**
       * Create CDC covers from DB.
       */
      void createCovers(const CDCGeometry& geom);

      /**
       * Create neutron shield from gearbox.
       */
      void createNeutronShields(const GearDir& content);


      /**
       * Create neutron shield from DB.
       */
      void createNeutronShields(const CDCGeometry& geom);

      /**
       * Create CDC cover2s from gear box.
       */
      void createCover2s(const GearDir& content);

      /**
       * Create CDC cover2s from DB.
       */
      void createCover2s(const CDCGeometry& geom);

      /**
       * Create G4Cone.
       */
      void createCone(const double rmin1, const double rmax1,
                      const double rmin2, const double rmax2,
                      const double thick, const double posz,
                      const int id, G4Material* med, const std::string& name);
      /**
       * Create G4Box.
       */
      void createBox(const double length, const double height,
                     const double thick, const double x,
                     const double y, const double z,
                     const int id, G4Material* med,
                     const std::string& name);
      /**
       * Create G4Tube.
       */
      void createTube(const double rmin, const double rmax,
                      const double thick, const double posZ,
                      const int id, G4Material* med,
                      const std::string& name);
      /**
       * Create G4Torus.
       */
      void createTorus(const double rmin1, const double rmax1,
                       const double thick, const double posZ,
                       const int id, G4Material* med,
                       const std::string& name);

      /**
       * Create G4Tube2.
       */
      void createTube2(const double rmin, const double rmax,
                       const double phis, const double phie,
                       const double thick, const double posZ,
                       const int id, G4Material* med,
                       const std::string& name);

    protected:

    private:
      /**
       * Create DB object of CDC geometry from gearbox.
       */
      CDCGeometry createConfiguration(const GearDir& param)
      {
        CDCGeometry cdcGeometry;
        cdcGeometry.read(param);
        return cdcGeometry;
      }
      /**
       * Create G4 geometry of CDC.
       */
      void createGeometry(const CDCGeometry& parameters, G4LogicalVolume& topVolume,
                          geometry::GeometryTypes type);
      /**
       * Create the B-field mapper geometry (tentative function)
       */
      void createMapper(G4LogicalVolume& topVolume);

      //! CDC G4 logical volume.
      G4LogicalVolume* m_logicalCDC;

      //! CDC G4 physical volume.
      G4VPhysicalVolume* m_physicalCDC;

      //! Sensitive detector
      CDCSensitiveDetector* m_sensitive = nullptr;

      //! Sensitive detector for background studies
      BkgSensitiveDetector* m_bkgsensitive = nullptr;

      //! Vector of pointers to G4VisAttributes
      std::vector<G4VisAttributes*> m_VisAttributes;

      //! Vector of pointers to G4UserLimits
      std::vector<G4UserLimits*> m_userLimits;

    };

  } // end of cdc namespace
} // end of Belle2 namespace
