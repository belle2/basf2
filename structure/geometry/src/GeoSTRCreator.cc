/**************************************************************************
 *  BASF2 (Belle Analysis Framework 2)                                    *
 *  Copyright(C) 2010 - Belle II Collaboration                            *
 *                                                                        *
 *  Author: The Belle II Collaboration                                    *
 *  Contributors:                                                         *
 *                                                                        *
 *                                                                        *
 *  This software is provided "as is" without any warranty.               *
 **************************************************************************/

#include <structure/geometry/GeoSTRCreator.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>

#include <G4NistManager.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Tubs.hh>
#include <G4Polycone.hh>
#include <G4Polyhedra.hh>
#include <G4Box.hh>
#include <G4SubtractionSolid.hh>
#include <G4Transform3D.hh>
#include <G4UserLimits.hh>
#include <G4VisAttributes.hh>

using namespace std;
using namespace boost;

namespace Belle2 {

  using namespace geometry;

  namespace structure {
    //-----------------------------------------------------------------
    //                 Register the Creator
    //-----------------------------------------------------------------

    geometry::CreatorFactory<GeoSTRCreator> GeoSTRFactory("STRCreator");
    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoSTRCreator::GeoSTRCreator()
    {

    }


    GeoSTRCreator::~GeoSTRCreator()
    {

    }

    void GeoSTRCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes)
    {

      GearDir cPolePieceL(content, "PolePieceL");
      double PolePieceL_minZ(0), PolePieceL_maxZ(0);
      G4Polycone* geo_PolePieceL = geometry::createPolyCone("geo_PolePieceL_name", cPolePieceL, PolePieceL_minZ, PolePieceL_maxZ);
      string strMat_PolePieceL = cPolePieceL.getString("Material", "Air");
      G4Material* mat_PolePieceL = Materials::get(strMat_PolePieceL);
      G4LogicalVolume* logi_PolePieceL = new G4LogicalVolume(geo_PolePieceL, mat_PolePieceL, "logi_PolePieceL_name");
      setColor(*logi_PolePieceL, "#CC0000");
      //setVisibility(*logi_PolePieceL, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0),
                        logi_PolePieceL, "phys_PolePieceL", &topVolume, false, 0);

      GearDir cPolePieceR(content, "PolePieceR");
      double PolePieceR_minZ(0), PolePieceR_maxZ(0);
      G4Polycone* geo_PolePieceR =
        geometry::createPolyCone("geo_PolePieceR_name",
                                 cPolePieceR, PolePieceR_minZ, PolePieceR_maxZ);
      string strMat_PolePieceR = cPolePieceR.getString("Material", "Air");
      G4Material* mat_PolePieceR = Materials::get(strMat_PolePieceR);
      G4LogicalVolume* logi_PolePieceR  = new G4LogicalVolume(geo_PolePieceR, mat_PolePieceR, "logi_PolePieceR_name");
      setColor(*logi_PolePieceR, "#CC0000");
      //setVisibility(*logi_PolePieceR, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0),
                        logi_PolePieceR, "phys_PolePieceR", &topVolume, false, 0);

      //Default value: Belle shield. Defined in STR.xml as e.g.:
      //      <ForwardShieldType>FWD0</ForwardShieldType>
      string strForwardShield = content.getString("ForwardShieldType", "FWD0");
      string strBackwardShield = content.getString("BackwardShieldType", "BWD0");
      BuildShield(content, topVolume, strForwardShield);
      BuildShield(content, topVolume, strBackwardShield);

    }

    float GeoSTRCreator::BuildShield(const GearDir& content, G4LogicalVolume& topVolume, string side)
    {

      string gearPath = (format("%1%/Layers/Layer") % side).str();
      int nFwdShieldLayers = content.getNumberNodes(gearPath);
      float _mass = 0;

      //Build the volumes
      for (int iLayer = 1 ; iLayer <= nFwdShieldLayers ; ++iLayer) {

        //Thread the strings
        string layerPath   = (format("/%1%[%2%]/") % gearPath % iLayer).str();
        string shapeName   = (format("%1%Layer_%2%") % side % iLayer).str();
        string logiVolName = (format("logi%1%Layer_%2%") % side % iLayer).str();
        string physVolName = (format("phys%1%Layer_%2%") % side % iLayer).str();

        // Connect the appropriate Gearbox path
        GearDir layerContent(content);
        layerContent.append(layerPath);

        // read and create material
        string strMatLayer = layerContent.getString("Material", "Air");
        G4Material* LayerMat = Materials::get(strMatLayer);

        // Create the shapw
        const std::vector<GearDir> planes = layerContent.getNodes("Plane");
        int nPlanes = planes.size();
        B2INFO("Number of planes on side  " << side << " layer " << iLayer
               << " : " << nPlanes);


        double _minZ(0), _maxZ(0);
        G4Polycone* LayerShape = geometry::createPolyCone(shapeName.c_str(),
                                                          layerContent, _minZ, _maxZ);
        // Create logical volume
        G4LogicalVolume* logiShieldLayer = new G4LogicalVolume(LayerShape, LayerMat, logiVolName, 0, 0, 0);

        //Place physical volume
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logiShieldLayer, physVolName, &topVolume, false, 0);

        B2INFO("Mass of " << side << " layer " << iLayer
               << " = " << logiShieldLayer->GetMass() / CLHEP::kg << ".kg.");

        _mass += (logiShieldLayer->GetMass() / CLHEP::kg);
      }

      B2INFO("Total mass of " << side << " = " << _mass << " kg");

      return _mass;
    }
  }
}
