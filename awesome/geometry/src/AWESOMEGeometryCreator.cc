/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <awesome/geometry/AWESOMEGeometryCreator.h>

/* Belle2 haders. */
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <geometry/utilities.h>

/** Geant4 headers. */
#include <G4Box.hh>
#include <G4PVPlacement.hh>
#include <G4Transform3D.hh>
#include <G4UserLimits.hh>

using namespace Belle2;
using namespace Belle2::AWESOME;

/** Register the AWESOME geometry creator. */
geometry::CreatorFactory<AWESOMEGeometryCreator> AWESOMEFactory("AWESOMECreator");

AWESOMEGeometryCreator::AWESOMEGeometryCreator() :
  m_sensitiveDetector{nullptr}
{
  m_sensitiveDetector = new AWESOMESensitiveDetector();
}

AWESOMEGeometryCreator::~AWESOMEGeometryCreator()
{
  if (m_sensitiveDetector)
    delete m_sensitiveDetector;
}

void AWESOMEGeometryCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type)
{
  /* Let's get the stepsize parameter with a default value of 5 µm. */
  double stepSize = content.getLength("stepSize", 5 * Unit::um);
  /* Now get the array. Notice that the default framework unit is cm,
   * so the values will be automatically converted. */
  B2INFO("Contents of bar:");
  for (double value : content.getArray("bar")) {
    B2INFO(LogVar("Value", value));
  }
  /* Let's loop over all the Active nodes. */
  for (const GearDir& activeParams : content.getNodes("Active")) {
    /* Lets create a Box with dimensions taken from the parameters.
     * WARNING: Geant4 is in mm so you have to convert the parameters. */
    G4Box* activeShape = new G4Box("ActiveBox",
                                   activeParams.getLength("width") / Unit::mm / 2.0,
                                   activeParams.getLength("length") / Unit::mm / 2.0,
                                   activeParams.getLength("height") / Unit::mm / 2.0
                                  );
    std::string material = activeParams.getString("Material");
    G4LogicalVolume* active = new G4LogicalVolume(activeShape,  geometry::Materials::get(material),
                                                  "ActiveVolume", 0, m_sensitiveDetector);
    /* setColor is an utility function which allows to easily set the color for visualization. */
    geometry::setColor(*active, "#006699");
    /* Le's limit the Geant4 stepsize inside the volume. */
    active->SetUserLimits(new G4UserLimits(stepSize));
    /* And finally place the volume in the world one. */
    new G4PVPlacement(G4Translate3D(0, 0, activeParams.getLength("z") / Unit::mm),
                      active, "ActivePlacement", &topVolume, false, 1);
  }
}
