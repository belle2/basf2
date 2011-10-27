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

class G4LogicalVolume;
class G4Tubs;

namespace Belle2 {

  class GearDir;
  class BkgSensitiveDetector;

  namespace bklm {

    class GeometryPar;
    class SensitiveDetector;

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
      void putInnerRegionInSector(G4LogicalVolume*, int, bool);

      //! Put the air void into the inner-radius region
      void putVoidInInnerRegion(G4LogicalVolume*, int, bool);

      //! Put the layer-0 support plate into the inner region's air void (sectors 0..4 only)
      void putLayer0SupportInInnerVoid(G4LogicalVolume*, bool);

      //! Put the layer-0 support plate's brackets into the inner region's air void (sectors 0..4 only)
      void putLayer0BracketsInInnerVoid(G4LogicalVolume*, bool);

      //! Put the layers into each sector
      void putLayersInSector(G4LogicalVolume*, int, int, bool);

      //! Put the solenoid's cooling chimney into the backward top sector
      void putChimneyInLayer(G4LogicalVolume*, int);

      //! Put the air gap into each layer
      void putGapInLayer(G4LogicalVolume*, int, int, int, bool);

      //! Put the RPC module into each air gap
      void putRPCModuleInGap(G4LogicalVolume*, int, int, int, bool);

      //! Put the scintillator module into each air gap
      void putScintModuleInGap(G4LogicalVolume*, int, int, int, bool);

      //! get shape corresponding to the solenoid (for subtraction)
      G4Tubs* solenoidCutout(void);

      //! Pointer to the BKLM geometry accessor
      GeometryPar* m_GeoPar;

      //! Pointer to the BKLM SensitiveDetector processor
      SensitiveDetector* m_sensitive;

      //! Pointer to the BKLM BkgSensitiveDetector processor
      BkgSensitiveDetector* m_bkgsensitive;

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
