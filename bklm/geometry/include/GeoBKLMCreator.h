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

      //! Build and put elements within enclosing logical volumes
      void putEndsInEnvelope(G4LogicalVolume*);
      void putSectorsInEnd(G4LogicalVolume*, int);
      void putCapInSector(G4LogicalVolume*, bool);
      void putInnerRegionInSector(G4LogicalVolume*, int, bool);
      void putVoidInInnerRegion(G4LogicalVolume*, int, bool);
      void putLayer0SupportInInnerVoid(G4LogicalVolume*, bool);
      void putLayer0BracketsInInnerVoid(G4LogicalVolume*, bool);
      void putLayersInSector(G4LogicalVolume*, int, int, bool);
      void putChimneyInLayer(G4LogicalVolume*, int);
      void putGapInLayer(G4LogicalVolume*, int, int, int, bool);
      void putRPCModuleInGap(G4LogicalVolume*, int, int, int, bool);
      void putScintModuleInGap(G4LogicalVolume*, int, int, int, bool);

      //! get shape corresponding to the solenoid (for subtraction)
      G4Tubs* solenoidCutout(void);

      //! Pointer to the BKLM geometry accessor
      GeometryPar* m_GeoPar;

      //! Pointer to the BKLM SensitiveDetector processor
      SensitiveDetector* m_sensitive;

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
