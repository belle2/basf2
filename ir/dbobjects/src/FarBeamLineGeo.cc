/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <ir/dbobjects/FarBeamLineGeo.h>

#include <cmath>

using namespace std;
using namespace Belle2;

void FarBeamLineGeo::initialize(const GearDir& content)
{
  //------------------------------
  // Get FarBeamLine geometry parameters from the gearbox
  //------------------------------

  addParameter("LimitStepLength", content.getInt("LimitStepLength"));

  std::vector<std::string> names = {"TubeR", "TubeL", "GateShield", "PolyShieldR", "PolyShieldL", "ConcreteShieldR", "ConcreteShieldL"};

  for (auto name : names) {
    GearDir sect(content, name + "/");
    addParameters(sect, name);
  }

  for (const GearDir& straight : content.getNodes("Straight")) {
    std::string name = straight.getString("@name");
    addParameters(straight, name);
    addParameter("Straight", name);
  }

  for (const GearDir& bend : content.getNodes("Bending")) {
    std::string name = bend.getString("@name");
    addParameters(bend, name);
    addParameter("Bending", name);
  }

}
