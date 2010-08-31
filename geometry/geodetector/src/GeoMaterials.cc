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
#include <geometry/gearbox/GearDir.h>
#include <framework/datastore/Units.h>

#include <cmath>
#include <boost/format.hpp>

#include <TMath.h>
#include <TVector3.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>


using namespace std;
using namespace boost;
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
  int nMat = content.getNumberNodes("Material");

  //Loop over all materials and create TGeoMaterial and TGeoMedium objects
  for (int iMat = 1; iMat <= nMat; ++iMat) {
    GearDir matContent(content);
    matContent.append((format("Material[%1%]/") % (iMat)).str());

    //1) Collect information
    std::string matName = matContent.getParamString("attribute::name");
    double atomNumber   = matContent.getParamNumValue("AtomNumber");
    double massNumber   = matContent.getParamNumValue("MassNumber");
    double density      = matContent.getParamNumValue("Density");
    double radlength    = matContent.getParamNumValue("RadLength");   //radiation length
    double interlength  = matContent.getParamNumValue("InterLength"); //interaction length

    //2) Build root material
    TGeoMaterial* geoMat = new TGeoMaterial(matName.c_str(), massNumber, atomNumber, density);
    geoMat->SetRadLen(radlength, interlength);

    //This might look like a memory leak, but ROOT takes care of deleting the object.
    new TGeoMedium(matName.c_str(), m_matIndex++, geoMat);
  }
}


void GeoMaterials::createMaterialMixture(GearDir& content)
{
  //Get number of mixtures in the material set
  int nMix = content.getNumberNodes("Mixture");

  //Loop over all mixtures and create TGeoMixture and TGeoMedium objects
  for (int iMix = 1; iMix <= nMix; ++iMix) {
    GearDir mixContent(content);
    mixContent.append((format("Mixture[%1%]/") % (iMix)).str());

    //1) Create Mixture
    int nElem = mixContent.getNumberNodes("Elements/Element");
    std::string mixName = mixContent.getParamString("attribute::name");
    TGeoMixture *geoMix = new TGeoMixture(mixName.c_str(), nElem);

    //2) Loop over all elements
    for (int iElem = 1; iElem <= nElem; ++iElem) {
      GearDir elemContent(mixContent);
      elemContent.append((format("Elements/Element[%1%]/") % (iElem)).str());

      double weight       = elemContent.getParamNumValue("attribute::weight");
      double atomNumber   = elemContent.getParamNumValue("AtomNumber");
      double massNumber   = elemContent.getParamNumValue("MassNumber");

      //Build root material
      geoMix->AddElement(massNumber, atomNumber, weight);
    }

    //3) Finalize
    double density = mixContent.getParamNumValue("Density");
    geoMix->SetDensity(density);

    //This might look like a memory leak, but ROOT takes care of deleting the object.
    new TGeoMedium(mixName.c_str(), m_matIndex++, geoMix);
  }
}
