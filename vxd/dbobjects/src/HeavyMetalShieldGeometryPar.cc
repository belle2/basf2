/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/HeavyMetalShieldGeometryPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;

// Read parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void HeavyMetalShieldPlanePar::read(const GearDir& plane)
{
  m_posZ = plane.getLength("posZ");
  m_innerRadius = plane.getLength("innerRadius");
  m_outerRadius = plane.getLength("outerRadius");
}

// Read parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void HeavyMetalShieldTypePar::read(const GearDir& shield)
{
  m_name = shield.getString("@name");
  m_minPhi = shield.getAngle("minPhi", 0);
  m_maxPhi = shield.getAngle("maxPhi", 2 * M_PI);
  m_materialName = shield.getString("Material", "Air");
  m_doCutOut = (shield.getNodes("Cutout").size() > 0);
  m_cutOutWidth = shield.getLength("Cutout/width", 0.);
  m_cutOutHeight = shield.getLength("Cutout/height", 0.);
  m_cutOutDepth = shield.getLength("Cutout/depth", 0.);

  const std::vector<GearDir> planes = shield.getNodes("Plane");
  for (const GearDir& plane : planes) {
    m_planes.push_back(HeavyMetalShieldPlanePar(plane));
  }
}

// Get  parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void HeavyMetalShieldGeometryPar::read(const GearDir& content)
{
  for (const GearDir& shield : content.getNodes("Shield")) {
    m_shields.push_back(HeavyMetalShieldTypePar(shield));
  }
}


