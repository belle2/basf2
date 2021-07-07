/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/gearbox/GearDir.h>
#include <ir/dbobjects/BeamPipeGeo.h>

using namespace std;
using namespace Belle2;

void BeamPipeGeo::initialize(const GearDir& content)
{
  //------------------------------
  // Get BeamPipe geometry parameters from the gearbox
  //------------------------------

  GearDir cSafety(content, "Safety/");
  addParameter("Safety.L1", cSafety.getLength("L1"));

  addParameter("LimitStepLength", content.getInt("LimitStepLength"));

  std::vector<std::string> names = {"Lv1SUS", "Lv2OutTi", "Lv2OutBe", "Lv2InBe", "Lv2Paraf", "Lv2Vacuum", "Lv3AuCoat", "Lv1TaFwd", "Lv2VacFwd", "Lv1TaBwd", "Lv2VacBwd", "Flange", "AreaTubeFwd", "Lv1TaLERUp", "Lv2VacLERUp", "Lv1TaHERDwn", "Lv2VacHERDwn", "AreaTubeBwd", "Lv1TaHERUp", "Lv2VacHERUp", "Lv1TaLERDwn", "Lv2VacLERDwn"};


  for (auto name : names) {
    GearDir sect(content, name + "/");
    addParameters(sect, name);
  }

}
