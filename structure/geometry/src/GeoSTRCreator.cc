/**************************************************************************
 *  BASF2 (Belle Analysis Framework 2)                                    *
 *  Copyright(C) 2016 - Belle II Collaboration                            *
 *                                                                        *
 *  Author: The Belle II Collaboration                                    *
 *  Contributors: Alexandre BEAULIEU                                      *
 *                                                                        *
 *                                                                        *
 *  This software is provided "as is" without any warranty.               *
 **************************************************************************/

#include <structure/geometry/GeoSTRCreator.h>
#include "structure/dbobjects/STRGeometryPar.h"

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>

#include <framework/logging/Logger.h>

#include <math.h> // PI
#include <boost/format.hpp>
#include <iostream>
#include <fstream>

#include <G4NistManager.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Polycone.hh>
#include <G4VisAttributes.hh>


using namespace std;

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



    void GeoSTRCreator::createGeometry(const STRGeometryPar& parameters, G4LogicalVolume& topVolume, GeometryTypes)
    {




      for (int iShield = 0; iShield < parameters.NECLSHIELDS; iShield++) {
        float  _mass = 0;
        string side;
        if (iShield == parameters.FWD_ECLSHIELD) {
          side = "FWD_Shield";
        } else if (iShield == parameters.BWD_ECLSHIELD) {
          side = "BWD_Shield";
        } else {
          B2FATAL("Only 2 ECL shields should be defined. Can't retrieve info for shield #" << iShield);
        }

        int nLayers = parameters.getNLayers(iShield);

        for (int iLayer = 0; iLayer < nLayers; iLayer++) {
          G4Material* LayerMat = Materials::get(parameters.getLayerMaterial(iShield, iLayer));
          int nPlanes = parameters.getLayerNPlanes(iShield, iLayer);

          //Thread the strings
          string shapeName   = (boost::format("%1%Layer_%2%") % side % (iLayer + 1)).str();
          string logiVolName = (boost::format("logi%1%Layer_%2%") % side % (iLayer + 1)).str();
          string physVolName = (boost::format("phys%1%Layer_%2%") % side % (iLayer + 1)).str();

          G4Polycone* LayerShape = new G4Polycone(shapeName.c_str(), 0, 2 * M_PI, nPlanes,
                                                  parameters.getLayerPlaneZ(iShield, iLayer),
                                                  parameters.getLayerPlaneInnerRadius(iShield, iLayer),
                                                  parameters.getLayerPlaneOuterRadius(iShield, iLayer));

          // Create logical volume
          G4LogicalVolume* logiShieldLayer = new G4LogicalVolume(LayerShape, LayerMat, logiVolName, 0, 0, 0);

          //Place physical volume
          new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logiShieldLayer, physVolName, &topVolume, false, 0);

          B2DEBUG(1, "Mass of " << side << " layer " << iLayer
                  << " = " << logiShieldLayer->GetMass() / CLHEP::kg << ".kg.");

          _mass += (logiShieldLayer->GetMass() / CLHEP::kg);
        }

        B2DEBUG(1, "Total mass of side " << side << " = " << _mass << " kg");
      }

      //Place pole pieces
      for (int iPole = 0; iPole < parameters.NPOLEPIECES; iPole++) {

        string side;
        if (iPole == parameters.FWD_POLEPIECE) {
          side = "PolePieceR";
        } else if (iPole == parameters.BWD_POLEPIECE) {
          side = "PolePieceL";
        } else {
          B2FATAL("Only 2 pole pieces should be defined. Can't retrieve info for pole #" << iPole);
        }


        G4Material* PoleMat = Materials::get(parameters.getPoleMaterial(iPole));
        int nPlanes = parameters.getPoleNPlanes(iPole);

        //Thread the strings
        string shapeName   = (boost::format("%1%") % side).str();
        string logiVolName = (boost::format("logi%1%") % side).str();
        string physVolName = (boost::format("phys%1%") % side).str();

        G4Polycone* PoleShape = new G4Polycone(shapeName.c_str(), 0, 2 * M_PI, nPlanes,
                                               parameters.getPolePlaneZ(iPole),
                                               parameters.getPolePlaneInnerRadius(iPole),
                                               parameters.getPolePlaneOuterRadius(iPole));

        // Create logical volume
        G4LogicalVolume* logiPole = new G4LogicalVolume(PoleShape, PoleMat, logiVolName, 0, 0, 0);

        //Place physical volume
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logiPole, physVolName, &topVolume, false, 0);

        B2DEBUG(1, "Total mass of " << side << " = " << logiPole->GetMass() / CLHEP::kg  << " kg");
      }


    }

    STRGeometryPar GeoSTRCreator::createConfiguration(const GearDir& param)
    {
      STRGeometryPar strGeometryPar;

      // Get STR geometry parameters from Gearbox (no calculations here)
      readShield(param, strGeometryPar, "FWD_Shield");
      readShield(param, strGeometryPar, "BWD_Shield");
      readPole(param, strGeometryPar, "PolePieceL");
      readPole(param, strGeometryPar, "PolePieceR");


      return strGeometryPar;
    };

    void GeoSTRCreator::readPole(const GearDir& content, STRGeometryPar& parameters, std::string side)
    {
      // Check if method was called using the correct name for the shields
      std::size_t foundF = side.find("PolePieceR");
      std::size_t foundB = side.find("PolePieceL");

      int iPole;
      if (foundF != std::string::npos) { iPole = parameters.FWD_POLEPIECE; }
      else if (foundB != std::string::npos) { iPole = parameters.BWD_POLEPIECE; }
      else { B2FATAL("No data for the Pole Piece requested " << side << "(not found)");}


      //Thread the strings
      std::string polePath   = (boost::format("/%1%/") % side).str();

      // Connect the appropriate Gearbox path
      GearDir poleContent(content);
      poleContent.append(polePath);

      // Retrieve material material
      parameters.setPoleMaterial(iPole, poleContent.getString("Material", "Air"));

      // Read the shape parameters
      const std::vector<GearDir> planes = poleContent.getNodes("Plane");
      parameters.setPoleNPlanes(iPole, planes.size());
      B2DEBUG(1, "Number of planes on side  " << side << " : " << planes.size());

      for (unsigned int iPlane = 0; iPlane < planes.size(); iPlane++) {
        parameters.setPolePlaneZ(iPole, iPlane, planes.at(iPlane).getLength("posZ") / Unit::mm);
        parameters.setPolePlaneInnerRadius(iPole, iPlane, planes.at(iPlane).getLength("innerRadius") / Unit::mm);
        parameters.setPolePlaneOuterRadius(iPole, iPlane, planes.at(iPlane).getLength("outerRadius") / Unit::mm);

      }
    }

    void GeoSTRCreator::readShield(const GearDir& content, STRGeometryPar& parameters, std::string side)
    {

      // Check if method was called using the correct name for the shields
      std::size_t foundF = side.find("FWD_Shield");
      std::size_t foundB = side.find("BWD_Shield");

      int iShield;
      if (foundF != std::string::npos) { iShield = parameters.FWD_ECLSHIELD; }
      else if (foundB != std::string::npos) { iShield = parameters.BWD_ECLSHIELD; }
      else { B2FATAL("No data for the ECL shield called " << side << "(not found)");}


      std::string gearPath = (boost::format("%1%/Layers/Layer") % side).str();

      // Retrieve the number of layers in the shield
      int nLayers = content.getNumberNodes(gearPath);
      parameters.setNLayers(iShield, nLayers);


      for (int iLayer = 0 ; iLayer < nLayers ; ++iLayer) {
        //Thread the strings
        std::string layerPath   = (boost::format("/%1%[%2%]/") % gearPath % (iLayer + 1)).str();

        // Connect the appropriate Gearbox path
        GearDir layerContent(content);
        layerContent.append(layerPath);

        // Retrieve material material
        parameters.setLayerMaterial(iShield, iLayer, layerContent.getString("Material", "Air"));

        // Read the shape parameters
        const std::vector<GearDir> planes = layerContent.getNodes("Plane");
        parameters.setLayerNPlanes(iShield, iLayer, planes.size());
        B2DEBUG(1, "Number of planes on side  " << side << " layer " << iLayer
                << " : " << planes.size());

        for (unsigned int iPlane = 0; iPlane < planes.size(); iPlane++) {
          parameters.setLayerPlaneZ(iShield, iLayer, iPlane, planes.at(iPlane).getLength("posZ") / Unit::mm);
          parameters.setLayerPlaneInnerRadius(iShield, iLayer, iPlane, planes.at(iPlane).getLength("innerRadius") / Unit::mm);
          parameters.setLayerPlaneOuterRadius(iShield, iLayer, iPlane, planes.at(iPlane).getLength("outerRadius") / Unit::mm);
        }
      }
    }
  }
}
