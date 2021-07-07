/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/dbobjects/bklm/BKLMGeometryPar.h>

/* Belle 2 headers. */
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/logging/Logger.h>
#include <geometry/CreatorBase.h>

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

    //!   This class creates the BKLM geometry of the Belle II detector

    class GeoBKLMCreator : public geometry::CreatorBase {

    public:

      //! Constructor of the GeoBKLMCreator class
      GeoBKLMCreator();

      //! Copy constructor is disabled
      GeoBKLMCreator(GeoBKLMCreator&) = delete;

      //! Assignment operator is disabled
      GeoBKLMCreator& operator=(GeoBKLMCreator&) = delete;

      //! Destructor of the GeoBKLMCreator class
      ~GeoBKLMCreator();

      //! Creates the objects for the BKLM geometry
      //! virtual void create(const GearDir&, G4LogicalVolume&, geometry::GeometryTypes type);
      void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        BKLMGeometryPar config = createConfiguration(content);
        createGeometry(config, topVolume, type);
      }

      //! Create the configuration objects and save them in the Database.
      void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override
      {
        DBImportObjPtr<BKLMGeometryPar> importObj;
        importObj.construct(createConfiguration(content));
        importObj.import(iov);
      }

      //! Create the geometry from the Database
      void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
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
      void putLayersInSector(G4LogicalVolume*, int, int, bool);

      //! Put the solenoid's cooling chimney into the backward top sector
      void putChimneyInLayer(G4LogicalVolume*, int);

      //! Put the module (and enclosing air gap) into each layer
      void putModuleInLayer(G4LogicalVolume*, int, int, int, bool, bool, int);

      //! Put the RPCs into each detector module's interior (module is itself in an air gap)
      void putRPCsInInterior(G4LogicalVolume*, int, bool);

      //! Put the scintillators into each detector module's interior (module is itself in an air gap)
      void putScintsInInterior(G4LogicalVolume*, int, int, int, bool);

      //! Get pointer to scintillator logical volume
      G4LogicalVolume* getScintLogical(double, double, double, double);

      //! Get pointer to MPPC-container logical volume
      G4LogicalVolume* getMPPCHousingLogical(void);

      //! Get pointer to readout-container logical volume
      G4LogicalVolume* getReadoutContainerLogical(void);

      //! Get shape corresponding to the solenoid (for subtraction)
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

      //! Pointer to logical volumes for cap [hasChimney]
      G4LogicalVolume* m_CapLogical[2];

      //! Pointer to solid for inner iron [hasInnerSupport | hasChimney]
      G4VSolid* m_InnerIronSolid;

      //! Pointer to logical volumes for inner iron [hasInnerSupport | hasChimney]
      G4LogicalVolume* m_InnerIronLogical[4];

      //! Pointer to solid for inner air
      G4VSolid* m_InnerAirSolid;

      //! Pointer to logical volumes for inner air [hasInnerSupport | hasChimney]
      G4LogicalVolume* m_InnerAirLogical[4];

      //! Pointer to logical volumes for support structure [hasChimney]
      G4LogicalVolume* m_SupportLogical[2];

      //! Pointer to logical volume for bracket
      G4LogicalVolume* m_BracketLogical;

      //! Pointers to solids for iron in each layer [layer-1]
      G4Polyhedra* m_LayerIronSolid[BKLMElementNumbers::getMaximalLayerNumber()];

      //! Pointers to logical volumes for iron in each layer [side/bottom/top | isFlipped | hasChimney | layer-1]
      G4LogicalVolume* m_LayerIronLogical[12 * BKLMElementNumbers::getMaximalLayerNumber()];

      //! Pointers to solids for air gap in each layer [hasChimney | layer-1]
      G4Box* m_LayerGapSolid[2 * BKLMElementNumbers::getMaximalLayerNumber()];

      //! Pointers to logical volumes for air gap in each layer [side/bottom/top | isFlipped | hasChimney | layer-1]
      G4LogicalVolume* m_LayerGapLogical[12 * BKLMElementNumbers::getMaximalLayerNumber()];

      //! Pointers to logical volumes for detector modules in each layer's air gap [hasChimney | layer-1]
      G4LogicalVolume* m_LayerModuleLogical[2 * BKLMElementNumbers::getMaximalLayerNumber()];

      //! Pointer to solid for sector's enclosing tube
      G4Tubs* m_SectorTube;

      //! Pointers to logical volumes for each sector [fb-1][sector-1]
      G4LogicalVolume* m_SectorLogical[2][BKLMElementNumbers::getMaximalSectorNumber()];

      //! Pointer to logical volume for MPPC housing
      G4LogicalVolume* m_MPPCHousingLogical;

      //! Pointer to logical volume for scint preamplifier/carrier container
      G4LogicalVolume* m_ReadoutContainerLogical;

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
