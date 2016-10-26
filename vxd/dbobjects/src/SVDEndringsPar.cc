/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/SVDEndringsPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;


// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void SVDEndringsPar::read(int layer, const GearDir& support)
{
  GearDir endrings(support, (boost::format("Endrings/Layer[@id='%1%']") % layer).str());

  m_material      = support.getString("Endrings/Material");
  m_length        = support.getLength("Endrings/length");
  m_gapWidth      = support.getLength("Endrings/gapWidth");
  m_baseThickness = support.getLength("Endrings/baseThickness");

  //Create  the endrings
  for (const GearDir& endring : endrings.getNodes("Endring")) {
    m_types.push_back(SVDEndringsTypePar(endring));
  }

}

// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void SVDEndringsTypePar::read(const GearDir& endring)
{
  m_name = endring.getString("@name");
  m_z = endring.getLength("z");
  m_baseRadius = endring.getLength("baseRadius");
  m_innerRadius = endring.getLength("innerRadius");
  m_outerRadius = endring.getLength("outerRadius");
  m_horizontalBarWidth = endring.getLength("horizontalBar");
  m_verticalBarWidth = endring.getLength("verticalBar");
}

