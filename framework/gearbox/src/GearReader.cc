/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/GearReader.h>

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <TGeoManager.h>

using namespace Belle2;
using namespace std;


double GearReader::readWeightAttribute(GearDir& gearDir)
{
  //Check if a weight attribute is defined. If it is, read its value.
  if (gearDir.isParamAvailable("attribute::weight")) {
    return gearDir.getParamNumValue("attribute::weight");
  } else return -1.0;
}


string GearReader::readNameAttribute(GearDir& gearDir)
{
  //Check if a name attribute is defined. If it is, read its value.
  if (gearDir.isParamAvailable("attribute::name")) {
    return gearDir.getParamString("attribute::name");
  } else return string("");
}


string GearReader::readUnitAttribute(GearDir& gearDir)
{
  //Check if a unit attribute is defined. If it is, read its value.
  if (gearDir.isParamAvailable("attribute::unit")) {
    return gearDir.getParamString("attribute::unit");
  } else return string("");
}
