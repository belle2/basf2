/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/gearbox/GearDir.h>
#include <ir/dbobjects/FarBeamLineGeo.h>

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

  std::string straightSections;
  for (const GearDir& straight : content.getNodes("Straight")) {
    std::string name = straight.getString("@name");
    addParameters(straight, name);
    if (!straightSections.empty()) straightSections += " ";
    straightSections += name;
  }
  addParameter("Straight", straightSections);

  std::string bendingSections;
  for (const GearDir& bend : content.getNodes("Bending")) {
    std::string name = bend.getString("@name");
    addParameters(bend, name);
    if (!bendingSections.empty()) bendingSections += " ";
    bendingSections += name;
  }
  addParameter("Bending", bendingSections);

  std::string collimators;
  for (const GearDir& coll : content.getNodes("Collimator")) {
    std::string name = coll.getString("@name");
    addParameters(coll, name);
    if (!collimators.empty()) collimators += " ";
    collimators += name;
  }
  addParameter("Collimator", collimators);

  std::string collimatorShields;
  for (const GearDir& collShield : content.getNodes("CollimatorShield")) {
    std::string name = collShield.getString("@name");
    addParameters(collShield, name);
    if (!collimatorShields.empty()) collimatorShields += " ";
    collimatorShields += name;
  }
  addParameter("CollimatorShield", collimatorShields);
}
