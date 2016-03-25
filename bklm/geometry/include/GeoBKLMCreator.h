/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOBKLMCREATOR_H
#define GEOBKLMCREATOR_H

#include <geometry/CreatorBase.h>
#include <bklm/geometry/GeometryPar.h>
#include <bklm/dbobjects/BKLMGeometryPar.h>
#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>

class G4VSolid;
class G4Box;
class G4Tubs;
class G4Polyhedra;
class G4LogicalVolume;
class G4VSensitiveDetector;
class G4String;
class G4VisAttributes;

namespace Belle2 {

  class GearDir;

  namespace bklm {

    class GeometryPar;
    class Sector;
    class Module;

    //!   This class creates the BKLM geometry of the Belle II detector

    class GeoBKLMCreator : public geometry::CreatorBase {

    public:

      //! Constructor of the GeoBKLMCreator class
      GeoBKLMCreator();

      //! Destructor of the GeoBKLMCreator class
      virtual ~GeoBKLMCreator();

      //! Creates the objects for the BKLM geometry
      //! virtual void create(const GearDir&, G4LogicalVolume&, geometry::GeometryTypes type);
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        BKLMGeometryPar config = createConfiguration(content);
        createGeometry(config, topVolume, type);
      }

      //! Create the configuration objects and save them in the Database.
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override
      {
        DBImportObjPtr<BKLMGeometryPar> importObj;
        importObj.construct(createConfiguration(content));
        importObj.import(iov);
      }

      //! Create the geometry from the Database
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        DBObjPtr<BKLMGeometryPar> dbObj;
        if (!dbObj) {
          //! Check that we found the object and if not report the problem
          B2FATAL("No configuration for " << name << " found.");
        }
        createGeometry(*dbObj, topVolume, type);
      }

    protected:

    private:

      //! Create a parameter object from the Gearbox XML parameters.
      BKLMGeometryPar createConfiguration(const GearDir& param)
      {
        BKLMGeometryPar bklmGeometryPar;
        //bklmGeometryPar.setVersion(0);
        bklmGeometryPar.read(param);
        return bklmGeometryPar;
      };

      //! Create the geometry from a parameter object.
      void createGeometry(const BKLMGeometryPar& parameters, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

      //! Put the forward and backward ends into the BKLM envelope
      void putEndsInEnvelope(G4LogicalVolume*);

      //! Put each sector into the forward or backward end
      void putSectorsInEnd(G4LogicalVolume*, int);

      //! Put the cap (at maximum |z|) into each sector
      void putCapInSector(G4LogicalVolume*, bool);

      //! Put the inner-radius region into each sector
      void putInnerRegionInSector(G4LogicalVolume*, bool, bool);

      //! Put the air void into the inner-radius region
      void putVoidInInnerRegion(G4LogicalVolume*, bool, bool);

      //! Put the layer-0 support plate into the inner region's air void (sectors 1..5 only)
      void putLayer1SupportInInnerVoid(G4LogicalVolume*, bool);

      //! Put the layer-0 support plate's brackets into the inner region's air void (sectors 1..5 only)
      void putLayer1BracketsInInnerVoid(G4LogicalVolume*, bool);

      //! Put the layers into each sector
      void putLayersInSector(G4LogicalVolume*, bool, bool);

      //! Put the solenoid's cooling chimney into the backward top sector
      void putChimneyInLayer(G4LogicalVolume*, int);

      //! Put the module (and enclosing air gap) into each layer
      void putModuleInLayer(G4LogicalVolume*, const Module*, int, bool);

      //! Put the RPCs into each detector module's interior (module is itself in an air gap)
      void putRPCsInInterior(G4LogicalVolume*, int, bool);

      //! Put the scintillators into each detector module's interior (module is itself in an air gap)
      void putScintsInInterior(G4LogicalVolume*, const Module*, int, bool);

      //! get pointer to sector logical volume
      G4LogicalVolume* getSectorLogical(int, bool, bool);

      //! Get pointer to scintillator logical volume
      G4LogicalVolume* getScintLogical(double, double, double);

      //! get shape corresponding to the solenoid (for subtraction)
      G4Tubs* getSolenoidTube(void);

      //! convert G4VSolid's name to corresponding G4LogicalVolume name
      G4String logicalName(G4VSolid*);

      //! convert G4LogicalVolume's name to corresponding G4PhysicalVolume name
      G4String physicalName(G4LogicalVolume*);

      //! Pointer to the BKLM geometry accessor
      GeometryPar* m_GeoPar;

      //! Pointer to the BKLM SensitiveDetector processor
      G4VSensitiveDetector* m_Sensitive;

      //! Angular extent of one sector
      double m_SectorDphi;

      //! Half-length of one sector
      double m_SectorDz;

      //! Radial displacement of polygon to create an azimuthal iron rib
      double m_RibShift;

      //! Pointer to solid for cap
      G4Polyhedra* m_CapSolid;

      //! Pointer to logical volumes for cap
      G4LogicalVolume* m_CapLogical[2];

      //! Pointer to solid for inner iron
      G4VSolid* m_InnerIronSolid;

      //! Pointer to logical volumes for inner iron
      G4LogicalVolume* m_InnerIronLogical[4];

      //! Pointer to solid for inner air
      G4VSolid* m_InnerAirSolid;

      //! Pointer to logical volumes for inner air
      G4LogicalVolume* m_InnerAirLogical[4];

      //! Pointer to logical volumes for support structure
      G4LogicalVolume* m_SupportLogical[2];

      //! Pointer to logical volume for bracket
      G4LogicalVolume* m_BracketLogical;

      //! Pointers to solids for iron in each layer
      G4Polyhedra* m_LayerIronSolid[NLAYER + 1];

      //! Pointers to logical volumes for iron in each layer
      G4LogicalVolume* m_LayerIronLogical[2 * (NLAYER + 1)];

      //! Pointer to solid for sector's enclosing tube
      G4Tubs* m_SectorTube;

      //! Pointers to logical volumes for each sector
      G4LogicalVolume* m_SectorLogical[NSECTOR];

      //! Pointer to solid for solenoid
      G4Tubs* m_SolenoidTube;

      //! Pointers to logical volumes for scintillator strips
      std::vector<G4LogicalVolume*> m_ScintLogicals;

      //! Vector of pointers to G4VisAttributes objects
      std::vector<G4VisAttributes*> m_VisAttributes;

      //! Vector of pointers to G4String objects (volume names)
      std::vector<G4String*> m_Names;

    };

  } // end of namespace bklm

}  // end of namespace Belle2

#endif // GEOBKLMCREATOR_H
