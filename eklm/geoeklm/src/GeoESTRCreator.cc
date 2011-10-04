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

#include <eklm/geoeklm/GeoESTRCreator.h>
//#include <eklm/geoeklm/StructureEndcap.h>

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

namespace Belle2 {

  using namespace geometry;

  //  namespace eklm {
  //-----------------------------------------------------------------
  //                 Register the Creator
  //-----------------------------------------------------------------

  geometry::CreatorFactory<GeoESTRCreator> GeoESTRFactory("ESTRCreator");
  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  GeoESTRCreator::GeoESTRCreator()
  {

  }


  GeoESTRCreator::~GeoESTRCreator()
  {

  }

  void GeoESTRCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type)
  {

  }

  //  }
}

