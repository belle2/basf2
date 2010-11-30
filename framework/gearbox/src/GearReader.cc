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

#include <TGeoManager.h>

using namespace Belle2;
using namespace std;


TGeoMaterial* GearReader::readMaterial(GearDir& gearDir)
{
  return NULL;
}


//=========================================================
//                  Protected methods
//=========================================================

TGeoElement* GearReader::readElementSection(GearDir& elementContent, double& weight)
{
  //Check if the GearDir points to an <Element> node
  if (elementContent.getNodeName() != "Element") {
    B2ERROR("Can't read the <Element> section specified by '" << elementContent.getDirPath() << "'. The path is not pointing to an <Element> node !");
    return NULL;
  }

  weight = readWeightAttribute(elementContent);
  string matName = readNameAttribute(elementContent);

  if (matName.empty()) {
    B2ERROR("The <Element> specified by '" << elementContent.getDirPath() << "' has no name !");
    return NULL;
  }

  int    atomNumber = static_cast<int>(elementContent.getParamNumValue("AtomNumber"));
  double massNumber = elementContent.getParamNumValue("MassNumber");

  TGeoElement* geoElem = new TGeoElement(matName.c_str(), matName.c_str(), atomNumber, massNumber);
  return geoElem;
}


TGeoMaterial* GearReader::readMaterialSection(GearDir& materialContent, double& weight)
{
  //Check if the GearDir points to a <Material> node
  if (materialContent.getNodeName() != "Material") {
    B2ERROR("Can't read the <Material> section specified by '" << materialContent.getDirPath() << "'. The path is not pointing to a <Material> node !");
    return NULL;
  }

  weight = readWeightAttribute(materialContent);
  string matName = readNameAttribute(materialContent);

  if (matName.empty()) {
    B2ERROR("The <Material> specified by '" << materialContent.getDirPath() << "' has no name !");
    return NULL;
  }

  TGeoMaterial* geoMat = NULL;

  //Check if the first parameter is available, if not try to get the material from gGeoManager
  if (materialContent.isParamAvailable("AtomNumber")) {

    double atomNumber  = materialContent.getParamNumValue("AtomNumber");
    double massNumber  = materialContent.getParamNumValue("MassNumber");
    double density     = materialContent.getParamDensity("Density");

    //Build root material
    geoMat = new TGeoMaterial(matName.c_str(), massNumber, atomNumber, density);

    //If the RadLength or InterLength is not given, they are computed by Root using the G3 formula
    if ((materialContent.isParamAvailable("RadLength")) && (materialContent.isParamAvailable("InterLength"))) {
      double radlength   = materialContent.getParamNumValue("RadLength");   //radiation length
      double interlength = materialContent.getParamNumValue("InterLength"); //interaction length
      geoMat->SetRadLen(radlength, interlength);
    }

  } else {
    geoMat = gGeoManager->GetMaterial(matName.c_str());
    if (geoMat == NULL) B2ERROR("Material " << matName << " could not be found in gGeoManager !")
    }

  return geoMat;
}


TGeoMixture* GearReader::readMixtureSection(GearDir& mixtureContent, double& weight)
{
  //Check if the GearDir points to a <Mixture> node
  if (mixtureContent.getNodeName() != "Mixture") {
    B2ERROR("Can't read the <Mixture> section specified by '" << mixtureContent.getDirPath() << "'. The path is not pointing to a <Mixture> node !");
    return NULL;
  }

  weight = readWeightAttribute(mixtureContent);
  string matName = readNameAttribute(mixtureContent);

  if (matName.empty()) {
    B2ERROR("The <Mixture> specified by '" << mixtureContent.getDirPath() << "' has no name !");
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
