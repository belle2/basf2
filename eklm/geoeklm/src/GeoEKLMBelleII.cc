/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Galina Pakhlova, Timofey Uglov                          *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/
#include <eklm/geoeklm/GeoEKLMBelleII.h>
#include <eklm/simeklm/EKLMSensetiveDetector.h>

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

  addSensitiveDetector("Strip_", new EKLMSensetiveDetector("EKLMSensetiveDetector", (2*24)*eV, 10*MeV)); //The EKLM subdetector uses the "Strip_" prefix to flag its sensitive volumes since all Strips are sensitive and vice versa.
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

  TGeoMedium* medPlastic = gGeoManager->GetMedium("Polystyrene");

  //----------------------------------------
  //         Get parameters from XML
  //----------------------------------------

  int nEndcap = 2 ;
  GearDir EndCap(content);
  content.append("EndCap/");
  double  Endcap_InnerR    = content.getParamLength("InnerR");
  double  Endcap_OuterR    = content.getParamLength("OuterR");
  double  Endcap_length    = content.getParamLength("Length");
  double  Endcap_positionX    = content.getParamLength("PositionX");
  double  Endcap_positionY    = content.getParamLength("PositionY");
  double  Endcap_positionZ    = EKLM_OffsetZ + Endcap_length / 2.0;

  //----------------------
  GearDir Layer(content);
  content.append("Layer/");
  int nLayer = 14 ;


  double  Layer_InnerR    = content.getParamLength("InnerR");
  double  Layer_OuterR    = content.getParamLength("OuterR");
  double  Layer_length    = content.getParamLength("Length");
  double  Layer_positionX    = content.getParamLength("PositionX");
  double  Layer_positionY    = content.getParamLength("PositionY");
  double  Layer_shiftZ       = content.getParamLength("ShiftZ");

  //----------------------
  GearDir Sector(content);
  content.append("Sector/");
  int     nSector = 4 ;


  double  Sector_InnerR    = content.getParamLength("InnerR");
  double  Sector_OuterR    = content.getParamLength("OuterR");
  double  Sector_length    = content.getParamLength("Length");
  double  Sector_positionX    = content.getParamLength("PositionX");
  double  Sector_positionY    = content.getParamLength("PositionY");
  double  Sector_positionZ    = content.getParamLength("PositionZ");

  //----------------------
  int     nPlane = 2 ;
  double  Plane_InnerR    = content.getParamLength("Plane/InnerR");
  double  Plane_OuterR    = content.getParamLength("Plane/OuterR");
  double  Plane_length    = content.getParamLength("Plane/Length");
  double  Plane_positionX    = content.getParamLength("Plane/PositionX");
  double  Plane_positionY    = content.getParamLength("Plane/PositionY");
  double  Plane_positionZ    = content.getParamLength("Plane/PositionZ");

  //----------------------
  int nStrip = content.getNumberNodes("Strips/Strip");


  double Strip_width     =  content.getParamLength("Strips/Width");
  double Strip_thickness =  content.getParamLength("Strips/Thickness");

  double* Strip_length    = new double[nStrip];
  double* Strip_positionX = new double[nStrip];
  double* Strip_positionY = new double[nStrip];

  for (int iStrip = 0; iStrip < nStrip; ++iStrip) {
    GearDir StripContent(content);
    StripContent.append((format("Strips/Strip[%1%]/") % (iStrip + 1)).str());

    string sStrip  = StripContent.getParamString("@id");
    int    StripID = atoi(sStrip.c_str());
    Strip_length[StripID]     = StripContent.getParamLength("Length");
    Strip_positionX[StripID]  = StripContent.getParamLength("PositionX");
    Strip_positionY[StripID]  = StripContent.getParamLength("PositionY");
  }

  //----------------------------------------
  //           Build EKLM
  //----------------------------------------

  TGeoRotation* geoRot = new TGeoRotation("EKLMRot", 0.0, 0, 0.0);
  volGrpEKLM = addSubdetectorGroup("EKLM", new TGeoCombiTrans(0.0, 0.0, 0.0, geoRot));

  //  Build EndCaps
  for (int iEndcap  = 0; iEndcap < nEndcap; ++iEndcap) {

    string Endcap_Name  = "Endcap_" + lexical_cast<string>(iEndcap);


    TGeoVolume* volEndcap =
      gGeoManager->MakeTube(Endcap_Name.c_str(), medAir,
                            Endcap_InnerR,
                            Endcap_OuterR,
                            Endcap_length / 2);
    volEndcap->SetLineColor(kRed);

    TGeoRotation* EndcapRot = new TGeoRotation("EndcapRot", 0.0, 0, 0.0);
    TGeoTranslation* EndcapTrans ;
    if (iEndcap == 0) {
      EndcapTrans = new TGeoTranslation("EndcapTrans",
                                        Endcap_positionX,
                                        Endcap_positionY,
                                        Endcap_positionZ);
    } else {
      EndcapRot->ReflectZ(true);
      EndcapTrans = new TGeoTranslation("EndcapTrans",
                                        Endcap_positionX,
                                        Endcap_positionY,
                                        -Endcap_positionZ);
    }


    volGrpEKLM->AddNode(volEndcap, iEndcap , new TGeoCombiTrans(*EndcapTrans , *EndcapRot));


    // Build and add Layers

    for (int iLayer = 0; iLayer < nLayer; ++iLayer) {

      string Layer_Name = "Layer_" + lexical_cast<string>(iLayer) + "_" + Endcap_Name;
      TGeoVolume* volLayer =     gGeoManager->MakeTube(Layer_Name.c_str(),
                                                       medAir,
                                                       Layer_InnerR,
                                                       Layer_OuterR,
                                                       Layer_length / 2.0);
      if (iLayer == 0)
        volLayer->SetLineColor(kGreen);
      else
        volLayer->SetLineColor(kBlack);
      double  Layer_positionZ  = -Endcap_length / 2.0 + (iLayer + 1) * Layer_shiftZ + (iLayer + 0.5) *  Layer_length ;
      volEndcap->AddNode(volLayer, iLayer ,
                         new TGeoTranslation(Layer_positionX,
                                             Layer_positionY,
                                             Layer_positionZ));


      // Build and add Sectors


      for (int iSector = 0; iSector < nSector; ++iSector) {
        string Sector_Name =  "Sector_" + lexical_cast<string>(iSector) + "_" + Layer_Name;
        TGeoVolume* volSector =
          gGeoManager->MakeTubs(Sector_Name.c_str(),
                                medAir,
                                Sector_InnerR,
                                Sector_OuterR,
                                Sector_length / 2.0,
                                0.0, 90.);
        volSector->SetLineColor(kRed);


        // Reflect sectors
        TGeoRotation* SectorRot = new TGeoRotation("SectorRot", 0.0, 0, 0.0);
        switch (iSector) {
          case 1:
            SectorRot->ReflectX(true);
            break;
          case 2:
            SectorRot->ReflectX(true);
            SectorRot->ReflectY(true);
            break;
          case 3:
            SectorRot->ReflectY(true);
            break;
        }

        volLayer->AddNode(volSector, iSector ,
                          new TGeoCombiTrans(("Sector_reflected_" + lexical_cast<string>(iSector)).c_str(),
                                             Sector_positionX,
                                             Sector_positionY,
                                             Sector_positionZ, SectorRot));

        // Build and add planes
        for (int iPlane = 0; iPlane < nPlane; ++iPlane) {
          string Plane_Name =   "Plane_" + lexical_cast<string>(iPlane) + "_" + Sector_Name;


          TGeoVolume* volPlane =
            gGeoManager->MakeTubs(Plane_Name.c_str(),
                                  medAir,
                                  Plane_InnerR,
                                  Plane_OuterR,
                                  Plane_length / 2.0,
                                  0.0, 90.);

          volPlane->SetLineColor(kBlue + 3);



          TGeoRotation* PlaneRot = new TGeoRotation("PlaneRot", 0.0, 0, 0.0);
          TGeoTranslation* PlaneTrans ;
          if (iPlane == 0) {
            PlaneTrans = new TGeoTranslation("PlaneTrans",
                                             Plane_positionX,
                                             Plane_positionY,
                                             Plane_positionZ);
          } else {
            PlaneRot->SetAngles(90, 90, 90, 0, 180, 0);  // rotation around X=Y axe

            PlaneTrans = new TGeoTranslation("PlaneTrans",
                                             Plane_positionX,
                                             Plane_positionY,
                                             -Plane_positionZ);
          }



          volSector->AddNode(volPlane, iPlane , new TGeoCombiTrans(*PlaneTrans , *PlaneRot));


          // Build and add strips for Plane
          for (int iStrip = 0; iStrip < nStrip; ++iStrip) {
            string Strip_Name =   "Strip_" + lexical_cast<string>(iStrip) + "_" + Plane_Name;
            TGeoVolume* volStrip =
              gGeoManager->MakeBox(Strip_Name.c_str(),
                                   medPlastic,
                                   Strip_length[iStrip] / 2.0 ,
                                   Strip_width / 2.0 ,
                                   Strip_thickness / 2.0);

            if (iStrip % 2)
              volStrip->SetLineColor(kRed);
            else
              volStrip->SetLineColor(kRed + 2);

            volPlane->AddNode(volStrip, iStrip ,
                              new TGeoTranslation(Strip_positionX[iStrip],
                                                  Strip_positionY[iStrip],
                                                  0.0));
          }
        }
      }

    }
  }
/////////////////////////////////////////////////////////////////////////////////////////////

}
