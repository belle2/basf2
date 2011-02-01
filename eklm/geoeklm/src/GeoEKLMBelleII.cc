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

#include <structure/geostructure/StructureEndcap.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

#include <cmath>
#include <boost/format.hpp>

#include <TMath.h>
#include <TVector3.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>
#include <TGeoBBox.h>
#include <TGeoPcon.h>
#include <TGeoPgon.h>
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

  //  TGeoMaterial *matFe = new TGeoMaterial("Fe", 55.845, 26, 7.87);
  //  TGeoMedium *medFe = new TGeoMedium("Iron", 1, matFe);


  //----------------------------------------
  //         Get parameters from XML
  //----------------------------------------

  int nEndcap = 2 ;
  GearDir EndCap(content);
  content.append("EndCap/");
  //  double  Endcap_InnerR    = content.getParamLength("InnerR");
  //  double  Endcap_OuterR    = content.getParamLength("OuterR");
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
//   double  Layer_positionX    = content.getParamLength("PositionX");
//   double  Layer_positionY    = content.getParamLength("PositionY");
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

  StructureEndcap EndcapMgr;
  EndcapMgr.read();

  //  Build EndCaps
  for (int iEndcap  = 0; iEndcap < nEndcap; ++iEndcap) {

    string Endcap_Name  = "Endcap_" + lexical_cast<string>(iEndcap);

    //Get Material
    TGeoMedium* strMed4EKLM = gGeoManager->GetMedium(EndcapMgr.matname().c_str());

    TGeoVolume* strTemp = gGeoManager->MakePgon("TempOct", strMed4EKLM,
                                                EndcapMgr.phi() / Unit::deg, EndcapMgr.dphi() / Unit::deg,
                                                EndcapMgr.nsides(), EndcapMgr.nBoundary());

    for (int iSet = 0; iSet < EndcapMgr.nBoundary() ; iSet++) {
      ((TGeoPgon*)strTemp->GetShape())->DefineSection(iSet, EndcapMgr.z(iSet),
                                                      EndcapMgr.rmin(iSet), EndcapMgr.rmax(iSet));
    }

//     TGeoVolume* strsubtube = gGeoManager->MakeTube("subtube", strMed4EKLM,
//                                                    EndcapMgr.rminsub(), EndcapMgr.rmaxsub(), EndcapMgr.zsub());
    TGeoCompositeShape* EndcapVessel = new TGeoCompositeShape("EKLMVessel", "TempOct-subtube");

    /*
    TGeoVolume* volEndcap =
      gGeoManager->MakeTube(Endcap_Name.c_str(), medFe,
                            Endcap_InnerR,
                            Endcap_OuterR,
                            Endcap_length / 2);
    */

    TGeoVolume* volEndcap = new TGeoVolume(Endcap_Name.c_str(), EndcapVessel, strMed4EKLM);
    volEndcap->SetLineColor(kBlue - 9);
    //volEndcap->SetLineColor(kRed);

    TGeoRotation* EndcapRot = new TGeoRotation("EndcapRot", 0.0, 0, 0.0);
    TGeoTranslation* EndcapTrans ;
    if (iEndcap == 0) {
      EndcapTrans = new TGeoTranslation("EndcapTrans",
                                        Endcap_positionX,
                                        Endcap_positionY,
                                        Endcap_positionZ); //Endcap_positionZ = 343.05cm
    } else {
      EndcapRot->ReflectZ(true);
      EndcapTrans = new TGeoTranslation("EndcapTrans",
                                        Endcap_positionX,
                                        Endcap_positionY,
                                        //-Endcap_positionZ); //Endcap_positionZ != -343.05cm : should be shifted by 47.0 x 2 = 94cm
                                        -Endcap_positionZ + 47.0 * 2.);
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
      //      volLayer->SetInvisible();
      // if (iLayer == 0)
      //   volLayer->SetLineColor(kGreen);
      // else
      //   volLayer->SetLineColor(kBlack);
      double  Layer_positionZ  = -Endcap_length / 2.0 + (iLayer + 1) * Layer_shiftZ + (iLayer + 0.5) *  Layer_length ;
      volEndcap->AddNode(volLayer, iLayer ,
                         //new TGeoTranslation(Layer_positionX,
                         //                    Layer_positionY,
                         new TGeoTranslation(0.0,
                                             0.0,
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
        //        volSector->SetLineColor(kRed);
        //  volSector->SetInvisible();

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

          //          volPlane->SetLineColor(kBlue + 3);
          //    volPlane->SetInvisible();


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
            /*
              inner end corresponds to negative local X
              outer (with SiPM) -- to positive X)
            */


            //hara            volStrip->SetInvisible();
            /*
                  if (iStrip % 2)
                    volStrip->SetLineColor(kRed);
                  else
                    volStrip->SetLineColor(kRed + 2);
            */
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
