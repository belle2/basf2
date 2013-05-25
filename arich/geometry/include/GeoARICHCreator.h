/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOARICHCREATOR_H
#define GEOARICHCREATOR_H

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>


class G4LogicalVolume;
class G4Material;

namespace Belle2 {
  namespace arich {

    class SensitiveDetector;
    class SensitiveAero;

    /** The creator for the PXD geometry of the Belle II detector.   */
    class GeoARICHCreator : public geometry::CreatorBase {

    public:

      /** Constructor of the GeoPXDCreator class. */
      GeoARICHCreator();

      /** The destructor of the GeoPXDCreator class. */
      virtual ~GeoARICHCreator();

      /**
       * Creates the ROOT Objects for the ARICH geometry.
       * @param content A reference to the content part of the parameter
       *                description, which should to be used to create the ROOT
       *                objects.
       */

      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

      //! build the HAPD modules
      G4LogicalVolume* buildModule(GearDir Module);
      //! build PCB of the modules
      G4LogicalVolume* buildModulePCB(GearDir Module);
      //! build the support plate of the module s
      G4LogicalVolume* buildModuleSupportPlate(GearDir Support);
      //! build mirrors
      G4LogicalVolume* buildMirrors(GearDir Mirrors);
      //! get average refractive index of the material
      double getAvgRINDEX(G4Material* material);
      //! creates simple geometry
      void createSimple(const GearDir& content, G4LogicalVolume& topVolume);

    protected:
      //! pointer to sensitive detector
      SensitiveDetector* m_sensitive;
      //! pointer to sensitive aerogel - used instead of tracking
      SensitiveAero* m_sensitiveAero;
      //! flag the beam background study
      int isBeamBkgStudy;
    };

  }
}

#endif /* GEOARICHCREATOR_H */
