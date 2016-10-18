/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/VXDAlignmentComponentPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;



// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void VXDAlignmentComponentPar::read(const GearDir& params)
{
  m_dU = params.getLength("du");
  m_dV = params.getLength("dv");
  m_dW = params.getLength("dw");
  m_alpha = params.getAngle("alpha");
  m_beta = params.getAngle("beta");
  m_gamma = params.getAngle("gamma");
}





