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
#include <framework/gearbox/GearReader.h>
#include <framework/gearbox/Unit.h>
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
  if (content.isPathValid("Material")) createMedia(content, "Material");
  if (content.isPathValid("Mixture")) createMedia(content, "Mixture");
}


void GeoMaterials::createMedia(GearDir& content, const string nodeName)
{
  //Get the number of Materials
  int nMat = 0;
  GearDir materials(content);
  materials.append(nodeName);
  nMat = materials.getNumberNodes();

  //Loop over Materials
  for (int iMat = 1; iMat <= nMat; ++iMat) {
    GearDir matContentIdx(materials, iMat);
    matContentIdx.convertPathToNode();
    TGeoMaterial* material = GearReader::readMaterial(matContentIdx);

    //This might look like a memory leak, but ROOT takes care of deleting the object.
    new TGeoMedium(material->GetName(), m_matIndex++, material);
  }
}
