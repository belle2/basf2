/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/SVDGeometryPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;



// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void SVDGeometryPar::read(const GearDir& content)
{
  B2INFO("Parsing SVD.xml to create SVDGeometryPar payload");
}
