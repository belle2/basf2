/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/SVDSupportRibsPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;


// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void SVDSupportTabPar::read(const GearDir& tab)
{
  m_theta = tab.getAngle("theta");
  m_zpos = tab.getLength("z");
  m_rpos = tab.getLength("r");
}


// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void SVDSupportBoxPar::read(const GearDir& box)
{
  m_theta = box.getAngle("theta");
  m_zpos = box.getLength("z");
  m_rpos = box.getLength("r");
  m_length = box.getLength("length");
}


// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void SVDEndmountPar::read(const GearDir& endmount)
{
  m_name = endmount.getString("@name");
  m_height = endmount.getLength("height");
  m_width = endmount.getLength("width");
  m_length = endmount.getLength("length");
  m_zpos = endmount.getLength("z");
  m_rpos = endmount.getLength("r");
}


// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void SVDSupportRibsPar::read(int layer, const GearDir& support)
{
  GearDir params(support, (boost::format("SupportRibs/Layer[@id='%1%']") % layer).str());

  // Get the common values for all layers
  m_spacing = support.getLength("SupportRibs/spacing");
  m_height = support.getLength("SupportRibs/height");
  m_innerWidth = support.getLength("SupportRibs/inner/width");
  m_outerWidth = support.getLength("SupportRibs/outer/width");
  m_tabLength = support.getLength("SupportRibs/inner/tabLength");

  m_outerMaterial = support.getString("SupportRibs/outer/Material");
  m_innerMaterial = support.getString("SupportRibs/inner/Material");
  m_outerColor = support.getString("SupportRibs/outer/Color");
  m_innerColor = support.getString("SupportRibs/inner/Color");
  m_endmountMaterial = support.getString("SupportRibs/endmount/Material");

  // Get values for the layer if available
  if (params.exists("spacing")) m_spacing = params.getLength("spacing");
  if (params.exists("height")) m_height = params.getLength("height");

  for (const GearDir& box : params.getNodes("box")) {
    m_boxes.push_back(SVDSupportBoxPar(box));
  }

  for (const GearDir& tab : params.getNodes("tab")) {
    m_tabs.push_back(SVDSupportTabPar(tab));
  }

  for (const GearDir& endmount : params.getNodes("Endmount")) {
    m_endmounts.push_back(SVDEndmountPar(endmount));
  }

}




