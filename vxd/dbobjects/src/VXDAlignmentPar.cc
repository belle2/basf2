/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/VXDAlignmentPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;

// Get VXD geometry parameters from Gearbox
void VXDAlignmentPar::read(const std::string& component, const GearDir& alignment)
{
  B2INFO("Reading alignment data for component " << component);

  string path = (boost::format("Align[@component='%1%']/") % component).str();
  GearDir params(alignment, path);
  if (!params) {
    B2WARNING("Could not find alignment parameters for component " << component);
    return;
  }
  m_dU = params.getLength("du") / Unit::mm;
  m_dV = params.getLength("dv") / Unit::mm;
  m_dW = params.getLength("dw") / Unit::mm;
  m_alpha = params.getAngle("alpha");
  m_beta  = params.getAngle("beta");
  m_gamma = params.getAngle("gamma");
}





