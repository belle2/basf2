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
#include <ir/dbobjects/CryostatGeo.h>

#include <cmath>

using namespace std;
using namespace Belle2;

void CryostatGeo::initialize(const GearDir& content)
{
  //------------------------------
  // Get Cryostat geometry parameters from the gearbox
  //------------------------------

  addParameter("LimitStepLength", content.getInt("LimitStepLength"));

  std::vector<std::string> names = {"TubeR", "TubeR2", "TubeL", "A1spc1", "A1spc2", "B1spc1", "B1spc2", "D1spc1", "E1spc1", "C1wal1", "F1wal1"};

  for (auto name : names) {
    GearDir sect(content, name + "/");
    addParameters(sect, name);
  }

  std::string straightSections;
  for (const GearDir& straight : content.getNodes("Straight")) {
    std::string name = straight.getString("@name");
    addParameters(straight, name);
    if (!straightSections.empty()) straightSections += " ";
    straightSections += name;
  }
  addParameter("Straight", straightSections);
}
