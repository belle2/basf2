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
      //      m_GeoPar = &parameters;



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

          B2INFO("Mass of " << side << " layer " << iLayer
                 << " = " << logiShieldLayer->GetMass() / CLHEP::g << ".kg.");

          _mass += (logiShieldLayer->GetMass() / CLHEP::g);
        }

        B2INFO("Total mass of side " << side << " = " << _mass << " kg");
      }

    }
  }
}
