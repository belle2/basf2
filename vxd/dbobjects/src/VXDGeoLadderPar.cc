/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/VXDGeoLadderPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;



// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void VXDGeoLadderPar::read(const GearDir& paramsLadder)
{
  m_layer = 0;
  m_shift = paramsLadder.getLength("shift");
  m_radius = paramsLadder.getLength("radius");
  m_slantedAngle = paramsLadder.getAngle("slantedAngle", 0);
  m_slantedRadius = paramsLadder.getLength("slantedRadius", 0);
  m_glueOversize = paramsLadder.getLength("Glue/oversize", 0);
  m_glueMaterial = paramsLadder.getString("Glue/Material", "");
}





