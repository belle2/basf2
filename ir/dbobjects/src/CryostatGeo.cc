/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/gearbox/GearDir.h>
#include <ir/dbobjects/CryostatGeo.h>

using namespace std;
using namespace Belle2;

void CryostatGeo::initialize(const GearDir& content)
{
  //------------------------------
  // Get Cryostat geometry parameters from the gearbox
  //------------------------------

  addParameter("LimitStepLength", content.getInt("LimitStepLength"));

  std::vector<std::string> names = {"CrossingAngle", "TubeR", "TubeR2", "TubeL", "A1spc1", "A1spc2", "B1spc1", "B1spc2", "D1spc1", "E1spc1", "C1wal1", "F1wal1"};

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
