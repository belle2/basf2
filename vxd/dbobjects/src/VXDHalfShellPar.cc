/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/VXDHalfShellPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;

// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void VXDHalfShellPar::read(const GearDir& shell)
{
  m_shellName =  shell.getString("@name");
  m_shellAngle = shell.getAngle("shellAngle", 0);

  B2INFO("Reading half shell parameters: name " << m_shellName);

  for (const GearDir& layer : shell.getNodes("Layer")) {
    int layerID = layer.getInt("@id");
    for (const GearDir& ladder : layer.getNodes("Ladder")) {
      int ladderID = ladder.getInt("@id");
      double phi = ladder.getAngle("phi", 0);
      m_layers[layerID].push_back(std::pair<int, double>(ladderID, phi));
      B2INFO("Reading layerID " << layerID << " ladderID " << ladderID << " shell angle " << phi);
    }
  }
}


