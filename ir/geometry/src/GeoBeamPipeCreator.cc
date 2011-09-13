/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ir/geometry/GeoBeamPipeCreator.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <ir/simulation/SensitiveDetector.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4AssemblyVolume.hh>

//Shapes
#include <G4Trd.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polycone.hh>
#include <G4SubtractionSolid.hh>
#include <G4UserLimits.hh>

using namespace std;
using namespace boost;

namespace Belle2 {


  using namespace geometry;

  namespace ir {

    //-----------------------------------------------------------------
    //                 Register the Creator
    //-----------------------------------------------------------------

    geometry::CreatorFactory<GeoBeamPipeCreator> GeoBeamPipeFactory("BeamPipeCreator");

    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoBeamPipeCreator::GeoBeamPipeCreator()
    {
    }

    GeoBeamPipeCreator::~GeoBeamPipeCreator()
    {
      //Delete all sensitive detectors
      BOOST_FOREACH(SensitiveDetector* sensitive, m_sensitive) {
        delete sensitive;
      }
      m_sensitive.clear();
    }

    void GeoBeamPipeCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes type)
    {

      //##############
      //#   Beam Pipe


    }
  }
}
