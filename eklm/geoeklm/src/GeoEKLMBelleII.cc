/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Galina Pakhlova, Timofei Uglov                          *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/
#include <eklm/geoeklm/GeoEKLMBelleII.h>

#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>

#include <cmath>
#include <boost/format.hpp>

#include <TMath.h>
#include <TVector3.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>
#include <TGeoBBox.h>
#include <TGeoPcon.h>
#include "TGeoCompositeShape.h"
#include "TGeoTube.h"
#include "TGeoCone.h"

#include <iostream>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;
using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------

GeoEKLMBelleII regGeoEKLMBelleII;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
GeoEKLMBelleII::GeoEKLMBelleII() : CreatorBase("EKLMBelleII")
{
  setDescription("Creates the TGeo objects for the EKLM geometry of the Belle II detector.");
}
GeoEKLMBelleII::~GeoEKLMBelleII()
{
}
void GeoEKLMBelleII::create(GearDir& content)
{
  //----------------------------------------
  //         Get global parameters
  //----------------------------------------

  double  EKLM_OffsetZ  = content.getParamLength("OffsetZ");

  //----------------------------------------
  //         Get Materials
  //----------------------------------------

  TGeoMaterial* matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
  TGeoMedium*   medAir    = new TGeoMedium("medAir", 1, matVacuum);

  //----------------------------------------
  //         Get parameters from XML
  //----------------------------------------

  GearDir FwEndCap(content);
  content.append("FwEndCap/");
  double  FwEndcap_InnerR    = content.getParamLength("InnerR");
  double  FwEndcap_OuterR    = content.getParamLength("OuterR");
  double  FwEndcap_length    = content.getParamLength("Length");
  double  FwEndcap_positionX    = content.getParamLength("PositionX");
  double  FwEndcap_positionY    = content.getParamLength("PositionY");
  double  FwEndcap_positionZ    = EKLM_OffsetZ + FwEndcap_length / 2.0;

  //----------------------
  GearDir Layer(content);
  content.append("Layer/");
  int nLayer = 1 ;

  string  Layer_Name      = content.getParamString("Name");
  double  Layer_InnerR    = content.getParamLength("InnerR");
  double  Layer_OuterR    = content.getParamLength("OuterR");
  double  Layer_length    = content.getParamLength("Length");
  double  Layer_positionX    = content.getParamLength("PositionX");
  double  Layer_positionY    = content.getParamLength("PositionY");
  double  Layer_shiftZ       = content.getParamLength("ShiftZ");

  //----------------------
  GearDir Sector(content);
  content.append("Sector/");
  int     nSector = 1 ;

  string  Sector_Name      = content.getParamString("Name");
  double  Sector_InnerR    = content.getParamLength("InnerR");
  double  Sector_OuterR    = content.getParamLength("OuterR");
  double  Sector_length    = content.getParamLength("Length");
  double  Sector_positionX    = content.getParamLength("PositionX");
  double  Sector_positionY    = content.getParamLength("PositionY");
  double  Sector_positionZ    = content.getParamLength("PositionZ");

  //----------------------
  string  Xplane_Name      = content.getParamString("Xplane/Name");
  double  Xplane_InnerR    = content.getParamLength("Xplane/InnerR");
  double  Xplane_OuterR    = content.getParamLength("Xplane/OuterR");
  double  Xplane_length    = content.getParamLength("Xplane/Length");
  double  Xplane_positionX    = content.getParamLength("Xplane/PositionX");
  double  Xplane_positionY    = content.getParamLength("Xplane/PositionY");
  double  Xplane_positionZ    = content.getParamLength("Xplane/PositionZ");

  //----------------------
  string  Yplane_Name      = content.getParamString("Yplane/Name");
  double  Yplane_InnerR    = content.getParamLength("Yplane/InnerR");
  double  Yplane_OuterR    = content.getParamLength("Yplane/OuterR");
  double  Yplane_length    = content.getParamLength("Yplane/Length");
  double  Yplane_positionX    = content.getParamLength("Yplane/PositionX");
  double  Yplane_positionY    = content.getParamLength("Yplane/PositionY");
  double  Yplane_positionZ    = content.getParamLength("Yplane/PositionZ");

  //----------------------
  int nStrip = content.getNumberNodes("Strips/Strip");

  string Strips_name     =  content.getParamString("Strips/Name");
  double Strip_width     =  content.getParamLength("Strips/Width");
  double Strip_thickness =  content.getParamLength("Strips/Thickness");

  string* Strip_Name      = new string[nStrip];
  double* Strip_length    = new double[nStrip];
  double* Strip_positionX = new double[nStrip];
  double* Strip_positionY = new double[nStrip];

  for (int iStrip = 0; iStrip < nStrip; ++iStrip) {
    GearDir StripContent(content);
    StripContent.append((format("Strips/Strip[%1%]/") % (iStrip + 1)).str());

    string sStrip  = StripContent.getParamString("@id");
    int    StripID = atoi(sStrip.c_str());

    Strip_Name[StripID]       = Strips_name + "_" + sStrip;
    Strip_length[StripID]     = StripContent.getParamLength("Length");
    Strip_positionX[StripID]  = StripContent.getParamLength("PositionX");
    Strip_positionY[StripID]  = StripContent.getParamLength("PositionY");
  }

  //----------------------------------------
  //           Build EKLM
  //----------------------------------------

  TGeoRotation* geoRot = new TGeoRotation("EKLMRot", 0.0, 0, 0.0);
  TGeoVolumeAssembly* volGrpEKLM = addSubdetectorGroup("EKLM", new TGeoCombiTrans(0.0, 0.0, 0.0, geoRot));

  TGeoVolume* volFwEndcap =
    gGeoManager->MakeTube("FwEndCap", medAir,
                          FwEndcap_InnerR,
                          FwEndcap_OuterR,
                          FwEndcap_length / 2);
  volFwEndcap->SetLineColor(kRed);
  volGrpEKLM->AddNode(volFwEndcap, 1 ,
                      new TGeoTranslation(FwEndcap_positionX,
                                          FwEndcap_positionY,
                                          FwEndcap_positionZ));

  //----------------------------------------

  for (int iLayer = 0; iLayer < nLayer; ++iLayer) {
    TGeoVolume* volLayer =
      gGeoManager->MakeTube((Layer_Name + "_" + lexical_cast<string>(iLayer)).c_str(),
                            medAir,
                            Layer_InnerR,
                            Layer_OuterR,
                            Layer_length / 2.0);
    volLayer->SetLineColor(kGreen);
    double  iLayer_positionZ  = -FwEndcap_length / 2.0 + (iLayer + 1) * Layer_shiftZ + (iLayer + 0.5) *  Layer_length ;
    volFwEndcap->AddNode(volLayer, 1 ,
                         new TGeoTranslation(Layer_positionX,
                                             Layer_positionY,
                                             iLayer_positionZ));


    for (int iSector = 0; iSector < nSector; ++iSector) {
      TGeoVolume* volSector =
        gGeoManager->MakeTubs((Sector_Name + "_" + lexical_cast<string>(iSector)).c_str(),
                              medAir,
                              Sector_InnerR,
                              Sector_OuterR,
                              Sector_length / 2.0,
                              0.0, 90.);
      volSector->SetLineColor(kRed);
      volLayer->AddNode(volSector, 1 ,
                        new TGeoTranslation(Sector_positionX,
                                            Sector_positionY,
                                            Sector_positionZ));

      TGeoVolume* volXplane =
        gGeoManager->MakeTubs((Xplane_Name + "_" + lexical_cast<string>(iSector)).c_str(),
                              medAir,
                              Xplane_InnerR,
                              Xplane_OuterR,
                              Xplane_length / 2.0,
                              0.0, 90.);
      volXplane->SetLineColor(kBlue + 3);
      volSector->AddNode(volXplane, 1 ,
                         new TGeoTranslation(Xplane_positionX,
                                             Xplane_positionY,
                                             Xplane_positionZ));


      for (int iStrip = 0; iStrip < nStrip; ++iStrip) {
        TGeoVolume* volStrip =
          gGeoManager->MakeBox((Sector_Name + "_" + lexical_cast<string>(iSector) + "_" + Xplane_Name + "_" + Strip_Name[iStrip]).c_str(),
                               medAir,
                               Strip_length[iStrip] / 2.0 ,
                               Strip_width / 2.0 ,
                               Strip_thickness / 2.0);

        volStrip->SetLineColor(kRed);
        volXplane->AddNode(volStrip, 1 ,
                           new TGeoTranslation(Strip_positionX[iStrip],
                                               Strip_positionY[iStrip],
                                               0.0));
      }
      TGeoVolume* volYplane =
        gGeoManager->MakeTubs((Yplane_Name + "_" + lexical_cast<string>(iSector)).c_str(),
                              medAir,
                              Yplane_InnerR,
                              Yplane_OuterR,
                              Yplane_length / 2.0 ,
                              0.0, 90.);
      volXplane->SetLineColor(kBlue);
      volSector->AddNode(volYplane, 1 ,
                         new TGeoTranslation(Yplane_positionX,
                                             Yplane_positionY,
                                             Yplane_positionZ));

    }
  }

/////////////////////////////////////////////////////////////////////////////////////////////

}
