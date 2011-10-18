/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Christian Oswald,           *
 *               Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/geometry/SupportCreator.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/foreach.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Polycone.hh>

using namespace std;
using namespace boost;

namespace Belle2 {

  using namespace geometry;
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PXD */
  namespace SVD {

    //-----------------------------------------------------------------
    //                 Register the Creator
    //-----------------------------------------------------------------

    geometry::CreatorFactory<SupportCreator> SupportFactory("SVDSupportCreator");

    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    SupportCreator::SupportCreator()
    {
    }

    SupportCreator::~SupportCreator()
    {
    }

    void SupportCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes type)
    {
      double minZ(0), maxZ(0);
      BOOST_FOREACH(const GearDir &component, content.getNodes("Polycone")) {
        string name = component.getString("Name");
        string material = component.getString("Material");

        G4Polycone* solid  = geometry::createPolyCone(name, component, minZ, maxZ);
        G4LogicalVolume* volume = new G4LogicalVolume(solid, Materials::get(material), name);
        setColor(*volume, component.getString("Color"));
        new G4PVPlacement(G4Transform3D(), volume, name, &topVolume, false, 1);
      }
      BOOST_FOREACH(const GearDir &component, content.getNodes("RotationSolid")) {
        string name = component.getString("Name");
        string material = component.getString("Material");

        G4Polycone* solid  = geometry::createRotationSolid(name, component, minZ, maxZ);
        G4LogicalVolume* volume = new G4LogicalVolume(solid, Materials::get(material), name);
        setColor(*volume, component.getString("Color"));
        new G4PVPlacement(G4Transform3D(), volume, name, &topVolume, false, 1);
      }

    }

  }
}
