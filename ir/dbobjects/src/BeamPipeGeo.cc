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
#include <ir/dbobjects/BeamPipeGeo.h>

#include <cmath>

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

  std::vector<std::string> names = {"Lv1SUS", "Lv2OutBe", "Lv2InBe", "Lv2Paraf", "Lv2Vacuum", "Lv3AuCoat", "Lv1TaFwd", "Lv2VacFwd", "Lv1TaBwd", "Lv2VacBwd", "Flange",
                                    "AreaTubeFwd", "Lv1TaLERUp", "Lv2VacLERUp", "Lv1TaHERDwn", "Lv2VacHERDwn", "AreaTubeBwd", "Lv1TaHERUp", "Lv2VacHERUp", "Lv1TaLERDwn", "Lv2VacLERDwn"
                                   };

  for (auto name : names) {
    addParameters(content, name);
  }

}
