/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/top/geometry/GeoTOPTBCreator.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <cmath>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4AssemblyVolume.hh>
//Shapes
#include <G4Trd.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polyhedra.hh>
#include <G4Polycone.hh>
#include <G4SubtractionSolid.hh>
#include <G4UserLimits.hh>
#include <G4Material.hh>

#include <G4TwoVector.hh>
#include <G4ThreeVector.hh>
#include <G4ExtrudedSolid.hh>
#include <G4UnionSolid.hh>
#include <G4Sphere.hh>
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4Colour.hh>

using namespace std;

namespace Belle2 {

  using namespace geometry;

  namespace TOPTB {

    /** Register the creator */
    geometry::CreatorFactory<GeoTOPTBCreator> GeoTOPFactory("TOPTBCreator");


    GeoTOPTBCreator::GeoTOPTBCreator()
    {
    }


    GeoTOPTBCreator::~GeoTOPTBCreator()
    {
    }


    void GeoTOPTBCreator::create(const GearDir& content, G4LogicalVolume& topVolume,
                                 GeometryTypes)
    {

      cout << "creating TOPTB geometry" << endl;

    }


  } // namespace TOPTB
} // namespace Belle2


