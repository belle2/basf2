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

class G4VSolid;
class G4Box;
class G4Tubs;
class G4LogicalVolume;
class G4VSensitiveDetector;
class G4String;

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
      virtual void create(const GearDir&, G4LogicalVolume&, geometry::GeometryTypes type);

    protected:

    private:

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

    };

  } // end of namespace bklm

}  // end of namespace Belle2

#endif // GEOBKLMCREATOR_H
