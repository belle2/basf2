/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/VXDServiceGeometryPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;


// Get  parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void VXDServiceGeometryPar::read(const GearDir& content)
{

  m_recordBackground = content.getBool("RecordBackground", false);

  GearDir content2(content, "DockBoxes");

  // Read parameters to creates boxes
  for (const GearDir& boxtype : content2.getNodes("BoxType")) {
    m_vxdBoxTypes.push_back(VXDBoxTypesPar(boxtype));
  }

  // Read paramater to place boxes
  for (const GearDir& position : content2.getNodes("Position")) {
    m_vxdBoxPositions.push_back(VXDBoxPositionsPar(position));
  }
}

// Read parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void VXDBoxPositionsPar::read(const GearDir& position)
{
  m_minZ = position.getLength("minZ");
  m_maxZ =  position.getLength("maxZ");
  m_minR = position.getLength("minR");
  m_maxR = position.getLength("maxR");
  m_divisions = position.getInt("divisions");
  m_name = position.getString("@name");

  for (const GearDir& slot : position.getNodes("slots")) {
    m_slotTypes.push_back(VXDSlotTypesPar(slot));
  }
}

// Read parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void VXDBoxTypesPar::read(const GearDir& boxtype)
{
  m_width = boxtype.getLength("width");
  m_length = boxtype.getLength("length");
  m_height = boxtype.getLength("height");
  m_material = boxtype.getString("material");
  m_name = boxtype.getString("@name");
  m_identifier = boxtype.getInt("@identifier", 0);
}

// Read parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void VXDSlotTypesPar::read(const GearDir& slots)
{
  m_type = slots.getString("@type");
  for (const double slot : slots.getArray("")) {
    m_slots.push_back(slot);
  }
}


