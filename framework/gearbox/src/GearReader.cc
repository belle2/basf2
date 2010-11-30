/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/GearReader.h>

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;


TGeoMaterial* GearReader::readMaterial(GearDir& gearDir)
{
  return NULL;
}


//=========================================================
//                  Protected methods
//=========================================================

TGeoElement* GearReader::readElementSection(GearDir& gearDir, double& weight)
{
  weight = readWeightAttribute(gearDir);
  string matName = readNameAttribute(gearDir);

  if (matName.empty()) {
    B2ERROR("The <Element> specified by '" << gearDir.getDirPath() << "' has no name !");
    return NULL;
  }

}


TGeoMaterial* GearReader::readMaterialSection(GearDir& gearDir, double& weight)
{
  weight = readWeightAttribute(gearDir);
  string matName = readNameAttribute(gearDir);

  if (matName.empty()) {
    B2ERROR("The <Material> specified by '" << gearDir.getDirPath() << "' has no name !");
    return NULL;
  }

}


TGeoMixture* GearReader::readMixtureSection(GearDir& gearDir, double& weight)
{
  weight = readWeightAttribute(gearDir);
  string matName = readNameAttribute(gearDir);

  if (matName.empty()) {
    B2ERROR("The <Mixture> specified by '" << gearDir.getDirPath() << "' has no name !");
    return NULL;
  }

}


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
