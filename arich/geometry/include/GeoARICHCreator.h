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

      G4LogicalVolume* buildModule(GearDir Module);
      G4LogicalVolume* buildModulePCB(GearDir Module);
      G4LogicalVolume* buildModuleSupportPlate(GearDir Support);
      G4LogicalVolume* buildMirrors(GearDir Mirrors);
      double getAvgRINDEX(G4Material* material);
      void createSimple(const GearDir& content, G4LogicalVolume& topVolume);

    protected:
      SensitiveDetector* m_sensitive;
      SensitiveAero* m_sensitiveAero;
      int isBeamBkgStudy;
    };

  }
}

#endif /* GEOARICHCREATOR_H */
