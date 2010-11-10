/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/GeoMaterials.h>
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <framework/logging/Logger.h>

#include <cmath>

#include <TMath.h>
#include <TVector3.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------

GeoMaterials regGeoMaterials;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoMaterials::GeoMaterials() : CreatorBase("GeoMaterials")
{
  setDescription("Creates the materials for the Belle II detector.");
}


GeoMaterials::~GeoMaterials()
{

}


void GeoMaterials::create(GearDir& content)
{
  m_matIndex = 0;
  createMaterialSingle(content);
  createMaterialMixture(content);
}


//=========================================================
//                  Private methods
//=========================================================

void GeoMaterials::createMaterialSingle(GearDir& content)
{
  //Get number of materials in the material set
  if (!content.isPathValid("Material")) return;

  GearDir matContent(content, "Material");
  int nMat = matContent.getNumberNodes();
  double weight = -1.0;

  //Loop over all materials and create TGeoMaterial and TGeoMedium objects
  for (int iMat = 1; iMat <= nMat; ++iMat) {
    GearDir matContentIdx(matContent, iMat);

    //This might look like a memory leak, but ROOT takes care of deleting the object.
    TGeoMaterial* currMaterial = readMaterial(matContentIdx, weight);
    new TGeoMedium(currMaterial->GetName(), m_matIndex++, currMaterial);
  }
}


void GeoMaterials::createMaterialMixture(GearDir& content)
{
  //Get number of mixtures in the material set
  if (!content.isPathValid("Mixture")) return;

  GearDir mixContent(content, "Mixture");
  int nMix = mixContent.getNumberNodes();
  double weight = -1.0;

  //Loop over all mixtures and create TGeoMixture and TGeoMedium objects
  for (int iMix = 1; iMix <= nMix; ++iMix) {
    GearDir mixContentIdx(mixContent, iMix);

    //1) Get the number of Elements and Materials
    int nElem = 0;
    GearDir elements(mixContentIdx);
    if (mixContentIdx.isPathValid("Elements/Element")) {
      elements.append("Elements/Element");
      nElem = elements.getNumberNodes();
    }

    int nMat = 0;
    GearDir materials(mixContentIdx);
    if (mixContentIdx.isPathValid("Elements/Material")) {
      materials.append("Elements/Material");
      nMat = materials.getNumberNodes();
    }

    //2) Create Mixture
    string mixName = mixContentIdx.getParamString("attribute::name");
    TGeoMixture *geoMix = new TGeoMixture(mixName.c_str(), nElem + nMat);

    //3) Loop over all elements
    for (int iElem = 1; iElem <= nElem; ++iElem) {
      GearDir elemContentIdx(elements, iElem);
      TGeoElement* currElement = readElement(elemContentIdx, weight);
      geoMix->AddElement(currElement, weight);
    }

    //4) Loop over all materials
    for (int iMat = 1; iMat <= nMat; ++iMat) {
      GearDir matContentIdx(materials, iMat);
      TGeoMaterial* currMaterial = readMaterial(matContentIdx, weight);

      if (weight < 0.0) {
        B2ERROR("The material " << currMaterial->GetName() << " has no weight defined. It was not added to the mixture " << geoMix->GetName())
        continue;
      }

      geoMix->AddElement(currMaterial, weight);
    }

    //3) Finalize
    double density = mixContentIdx.getParamNumValue("Density");
    geoMix->SetDensity(density);

    //This might look like a memory leak, but ROOT takes care of deleting the object.
    new TGeoMedium(mixName.c_str(), m_matIndex++, geoMix);
  }
}


//==========================================================
//                 Private methods
//==========================================================

TGeoMaterial* GeoMaterials::readMaterial(GearDir& materialContent, double& weight)
{
  //Collect information
  if (materialContent.isParamAvailable("attribute::weight")) {
    weight = materialContent.getParamNumValue("attribute::weight");
  } else weight = -1.0;

  string matName     = materialContent.getParamString("attribute::name");
  TGeoMaterial* geoMat = NULL;

  //Check if the first parameter is available, if not try to get the material from gGeoManager
  if (materialContent.isParamAvailable("AtomNumber")) {

    double atomNumber  = materialContent.getParamNumValue("AtomNumber");
    double massNumber  = materialContent.getParamNumValue("MassNumber");
    double density     = materialContent.getParamNumValue("Density");
    double radlength   = materialContent.getParamNumValue("RadLength");   //radiation length
    double interlength = materialContent.getParamNumValue("InterLength"); //interaction length

    //Build root material
    geoMat = new TGeoMaterial(matName.c_str(), massNumber, atomNumber, density);
    geoMat->SetRadLen(radlength, interlength);

  } else {
    geoMat = gGeoManager->GetMaterial(matName.c_str());
    if (geoMat == NULL) B2ERROR("Material " << matName << " could not be found in gGeoManager !")
    }

  return geoMat;
}


TGeoElement* GeoMaterials::readElement(GearDir& elementContent, double& weight)
{
  //Collect information
  weight            = elementContent.getParamNumValue("attribute::weight");
  string elemName   = elementContent.getParamString("attribute::name");
  int    atomNumber = static_cast<int>(elementContent.getParamNumValue("AtomNumber"));
  double massNumber = elementContent.getParamNumValue("MassNumber");

  //Build root element
  TGeoElement* geoElem = new TGeoElement(elemName.c_str(), elemName.c_str(), atomNumber, massNumber);

  return geoElem;
}
