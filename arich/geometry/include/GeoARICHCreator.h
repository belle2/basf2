/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOARICHCREATOR_H_
#define GEOARICHCREATOR_H_

#include <arich/geometry/ARICHGeometryPar.h>
#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>

#include <G4Transform3D.hh>
class G4LogicalVolume;
class G4AssemblyVolume;
class G4Polycone;
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

      /**
       * Get Alignment for given component from the database
       * @param  component Name of the component to align
       * @return Transformation matrix for component, idendity if component
       *         could not be found
       */

      G4LogicalVolume* buildModule(GearDir Module);
      G4LogicalVolume* buildModulePCB(GearDir Module);
      G4LogicalVolume* buildModuleSupportPlate(GearDir Support);
      G4LogicalVolume* buildMirrors(GearDir Mirrors);
      double getAvgRINDEX(G4Material* material);
      void createSimple(const GearDir& content, G4LogicalVolume& topVolume);
    protected:
      //      GearDir m_alignment;
      // GearDir m_components;
      // std::map<std::string, GeoPXDComponent> m_componentCache;
      //GeoPXDLadder m_ladder;
      SensitiveDetector* m_sensitive;
      SensitiveAero* m_sensitiveAero;
      ARICHGeometryPar* m_arichgp;
    };

  }
}

#endif /* GEOARICHCREATOR_H_ */
