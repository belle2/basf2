/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/geoecl/GeoECLBelleII.h>
#include <ecl/simecl/ECLSensitiveDetector.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>

#include <string>
#include <iostream>
#include <iomanip>


#include <TGeoArb8.h>

#include <TMath.h>
#include <TVector3.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>
#include <TGeoPcon.h>
#define PI 3.14159265358979323846
//#define PI 3.14159265
using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------

GeoECLBelleII regGeoECLBelleII;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoECLBelleII::GeoECLBelleII() : CreatorBase("ECLBelleII")
{
  setDescription("Creates the TGeo objects for the ECL geometry of the Belle II detector.");

  //The ECL subdetector uses the "SD_" prefix to flag its sensitive volumes
  addSensitiveDetector("SD_", new ECLSensitiveDetector("ECLSensitiveDetector"));
}


GeoECLBelleII::~GeoECLBelleII()
{

}

void GeoECLBelleII::create(GearDir& content)
{
  TGeoRotation* geoRot = new TGeoRotation("ECLRot", 90.0, 0.0, 0.0);
  TGeoVolumeAssembly* volECL = addSubdetectorGroup("ECL", new TGeoCombiTrans(0.0, 0.0, 0.0, geoRot));

  //Add the TGeo creating code here

  TGeoMixture *matCsI = new TGeoMixture("CsI", 2, 4.510);
  TGeoElementTable *eltable = gGeoManager->GetElementTable();
  TGeoElement *cesium = eltable->GetElement(53);
  TGeoElement *iodine = eltable->GetElement(55);
  matCsI->AddElement(cesium, 0.4885);
  matCsI->AddElement(iodine, 0.5115);

  makeENDCAP(content, volECL);
  makeBarrel(content, volECL);

}


void GeoECLBelleII::makeBarrel(GearDir& content, TGeoVolumeAssembly* group)
{

  TGeoMixture *matCsI = new TGeoMixture("CsI", 2, 4.510);
  TGeoElementTable *eltable = gGeoManager->GetElementTable();
  TGeoElement *cesium = eltable->GetElement(53);
  TGeoElement *iodine = eltable->GetElement(55);
  matCsI->AddElement(cesium, 0.4885);
  matCsI->AddElement(iodine, 0.5115);
  matCsI->Print();
  TGeoMedium *CsI = new TGeoMedium("CsI", 1, matCsI);

  double k_BLL;
  double k_Ba;
  double k_Bb;
  double k_BA;
  double k_Bh;
  double k_BH;
  double k_gamA;
  double k_alpha;
  double k_gamH;
  double k_z_TILTED;
  double k_phi_TILTED;
  double k_perpC;
  double k_phiC;
  double k_zC;
  double fin_thickness = 0.05 ;
  double wrap_thickness = 0.025 ;

  double zsign = 1.;


  GearDir detcontent(content);
  fin_thickness = detcontent.getParamLength("k_fin_thickness");
  wrap_thickness = detcontent.getParamLength("k_wrap_thickness");
  int    nblock = int(detcontent.getParamNumValue("k_barrel_Nblock"));


  detcontent.append("BarrelCrystals/");
  int Ncounters = detcontent.getNumberNodes("BarrelCrystal");
  B2INFO("detcontent_barrel = " << detcontent.getDirPath().c_str() << " with " << Ncounters << "nodes");


  TGeoVolume *boxcopy[8736];


//        for (int iBrCry = 1 ; iBrCry <= Ncounters ; ++iBrCry) {
  for (int iBrCry = 1 ; iBrCry <= 46 ; ++iBrCry) {//46=29+17

    char stemp[100];
    GearDir layerContent(content);

    if (iBrCry >= 30) {zsign = -1; }

    sprintf(stemp, "BarrelCrystals/BarrelCrystal[%d]/", iBrCry);
    layerContent.append(stemp);
    k_BLL = layerContent.getParamLength("K_BLL");
    k_Ba = layerContent.getParamLength("K_Ba") ;
    k_Bb = layerContent.getParamLength("K_Bb") ;
    k_Bh = layerContent.getParamLength("K_Bh") ;
    k_BA = layerContent.getParamLength("K_BA") ;
    k_BH = layerContent.getParamLength("K_BH") ;
    k_z_TILTED = layerContent.getParamAngle("K_z_TILTED") ;
    k_phi_TILTED = layerContent.getParamAngle("K_phi_TILTED") ;
    k_perpC = layerContent.getParamLength("K_perpC") ;
    k_phiC = layerContent.getParamAngle("K_phiC") ;
    k_zC = layerContent.getParamLength("K_zC") ;



    k_gamA = atan((k_Bb - k_Ba) / 2 / k_Bh) * 180 / PI;
    k_alpha = atan(k_BLL / k_Bh) * 180 / PI ;
    k_gamH = atan((k_BH - k_Bh) / 2 / k_BLL) * 180 / PI ;

//          double wDx1=k_Ba/2;//same as cDx1
//          double wDx2=k_Bb/2;//same as cDx2
    double wDy1 = k_Bh / 2 + wrap_thickness * (1 / cos(k_gamH * PI / 180) - tan(k_gamH * PI / 180));
//          double wDx3=k_BA/2;//same as cDx3
//          double wDx4=k_Bb/2*k_BA/k_Ba;//same as cDx4
    double wDy2 = k_BH / 2 + wrap_thickness * (1 / cos(k_gamH * PI / 180) + tan(k_gamH * PI / 180));
    double wDz = k_BLL / 2 + wrap_thickness;
    k_alpha = atan(wDz / wDy1) * 180 / PI ;
    k_gamH = atan((wDy2 - wDy1) / 2 / wDz) * 180 / PI ;

    double cDx1 = k_Ba / 2;
    double cDx2 = k_Bb / 2;
    double cDy1 = k_Bh / 2;
    double cDx3 = k_BA / 2;
    double cDx4 = k_Bb / 2 * k_BA / k_Ba;
    double cDy2 = k_BH / 2;
    double cDz = k_BLL / 2;

    double vtx[15];
    vtx[0] = cDx1; vtx[1] = cDy1;//HepPoint3D tmp0( cDx1, cDy1,-cDz);
    vtx[2] = cDx2; vtx[3] = -cDy1;//HepPoint3D tmp1( cDx2,-cDy1,-cDz);
    vtx[4] = -cDx2; vtx[5] = -cDy1;//HepPoint3D tmp2(-cDx2,-cDy1,-cDz);
    vtx[6] = -cDx1; vtx[7] = cDy1;//HepPoint3D tmp3(-cDx1, cDy1,-cDz);
    vtx[8] = cDx3; vtx[9] = cDy2;//HepPoint3D tmp4( cDx3, cDy2, cDz);
    vtx[10] = cDx4; vtx[11] = -cDy2;//HepPoint3D tmp5( cDx4,-cDy2, cDz);
    vtx[12] = -cDx4; vtx[13] = -cDy2;//HepPoint3D tmp6(-cDx4,-cDy2, cDz);
    vtx[14] = -cDx3; vtx[15] = cDy2;//HepPoint3D tmp7(-cDx3, cDy2, cDz);


    TGeoRotation r00, tilt_z, tilt_phi, pos_phi;
    r00.RotateZ(90.*zsign);//m1 = HepRotateZ3D(90./180*PI*zsign);
    tilt_z.RotateY(k_z_TILTED);//m2 = HepRotateY3D( (90-crystal_rotate*zsign)/180*PI );
    tilt_phi.RotateZ(k_phi_TILTED);//m3 = HepRotateZ3D(k_phi_TILTED);
    TGeoTranslation  position("gTrans", k_perpC, 0, k_zC);
    pos_phi.RotateZ(k_phiC);//


    for (int iBox = 0; iBox < nblock; ++iBox) {//total 72

      int cry_id1 = 2 * nblock * (iBrCry - 1) + 2 * iBox + 1152;
      int cry_id2 = 2 * nblock * (iBrCry - 1) + 2 * iBox + 1 + 1152;
      sprintf(stemp, "SD_cid_%04d", cry_id1);
//      boxcopy[cry_id1] = gGeoManager->MakeArb8(stemp, CsI, cDz, vtx);
      boxcopy[cry_id1] = gGeoManager->MakeTrap(stemp, CsI, cDz , 0 , 0, cDy1, cDx2, cDx1, 0, cDy2 , cDx4, cDx3, 0);

      sprintf(stemp, "SD_cid_%04d", cry_id2);
//      boxcopy[cry_id2] = gGeoManager->MakeArb8(stemp, CsI, cDz, vtx);
      boxcopy[cry_id2] = gGeoManager->MakeTrap(stemp, CsI, cDz , 0 , 0, cDy1, cDx2, cDx1, 0, cDy2 , cDx4, cDx3, 0);
      TGeoRotation    r("gRot", 0.0, 0.0, 360.*iBox / nblock);
      TGeoRotation    rr("gRot", 0.0, 0.0, 360.*iBox / nblock - 2.494688);

      group->AddNode(boxcopy[cry_id1], cry_id1
                     , new TGeoHMatrix(r*pos_phi*position*tilt_phi*tilt_z*r00));
      group->AddNode(boxcopy[cry_id2], cry_id2
                     , new TGeoHMatrix(rr*pos_phi*position*tilt_phi*tilt_z*r00));

    }


  }//iBrCry46


}
void GeoECLBelleII::makeENDCAP(GearDir& content, TGeoVolumeAssembly* group)
{
  TGeoMixture *matCsI = new TGeoMixture("CsI", 2, 4.510);
  TGeoElementTable *eltable = gGeoManager->GetElementTable();
  TGeoElement *cesium = eltable->GetElement(53);
  TGeoElement *iodine = eltable->GetElement(55);
  matCsI->AddElement(cesium, 0.4885);
  matCsI->AddElement(iodine, 0.5115);
  matCsI->Print();
  TGeoMedium *CsI = new TGeoMedium("CsI", 1, matCsI);

  //TGeoMaterial *matAl = new TGeoMaterial("Al", 26.98, 13, 2.7);
  //TGeoMedium *Al = new TGeoMedium("Root Material", 2, matAl);  // not used (by T.Hara)




  GearDir detcontent_ecap(content);
  int    nblock = int(detcontent_ecap.getParamNumValue("k_endcap_Nblock"));
  double halflength = double(detcontent_ecap.getParamLength("k_Crystall_HalfLength"));

  detcontent_ecap.append("EndCapCrystals/");
  int Ncounters_ecap = detcontent_ecap.getNumberNodes("EndCapCrystal");
  B2INFO("detcontent_ecap = " << detcontent_ecap.getDirPath().c_str() << " with " << Ncounters_ecap << "nodes");

  double h1, h2, bl1, bl2, tl1, tl2, alpha1, alpha2, Rphi1, Rphi2, Rtheta, Pr, Ptheta, Pphi;
  char stemp_ecap[100];
  //TGeoVolume *counterbox[200]; // not used (by T.Hara)
  TGeoVolume *boxcopy[8736];
  int box_id = 0;
  int box_id1 = 0;

  for (int i = 1 ; i <= Ncounters_ecap ; ++i) {
    if (i % 100 == 0) B2INFO("ECL counter " << i << "read");
    GearDir counter(detcontent_ecap);
    sprintf(stemp_ecap, "EndCapCrystal[%d]/", i);
    counter.append(stemp_ecap);
//              printf ("HalfLength = %f\n", counter.getParamLength("HalfLength"));

    h1 = counter.getParamLength("K_h1");
    h2 = counter.getParamLength("K_h2");
    bl1 = counter.getParamLength("K_bl1");
    bl2 = counter.getParamLength("K_bl2");
    tl1 = counter.getParamLength("K_tl1");
    tl2 = counter.getParamLength("K_tl2");
    alpha1 = counter.getParamAngle("K_alpha1");
    alpha2 = counter.getParamAngle("K_alpha2");
    Ptheta = counter.getParamAngle("K_Ptheta") ;
    Rphi1 = counter.getParamAngle("K_Rphi1") ;
    Rtheta = counter.getParamAngle("K_Ptheta") ;
    Rphi2 = counter.getParamAngle("K_Rphi2")  ;
    Pr = counter.getParamLength("K_Pr");
    Ptheta = counter.getParamAngle("K_Ptheta") ;
    Pphi = counter.getParamAngle("K_Pphi") ;



    /*
       fDz = 15;
       fTheta = 0;
       fPhi = 0;
       fH1 = ;
       fH2 = h2;
       fBl1 = bl1;
       fBl2 = bl2;
       fTl1 = tl1;
       fTl2 = tl2;
       fAlpha1 = alpha1;
       fAlpha2 = alpha2;
       Double_t tx = TMath::Tan(theta*TMath::DegToRad())*TMath::Cos(phi*TMath::DegToRad());
       Double_t ty = TMath::Tan(theta*TMath::DegToRad())*TMath::Sin(phi*TMath::DegToRad());
       Double_t ta1 = TMath::Tan(alpha1*TMath::DegToRad());
       Double_t ta2 = TMath::Tan(alpha2*TMath::DegToRad());
       fXY[0][0] = -dz*tx-h1*ta1-bl1;    fXY[0][1] = -dz*ty-h1;
       fXY[1][0] = -dz*tx+h1*ta1-tl1;    fXY[1][1] = -dz*ty+h1;
       fXY[2][0] = -dz*tx+h1*ta1+tl1;    fXY[2][1] = -dz*ty+h1;
       fXY[3][0] = -dz*tx-h1*ta1+bl1;    fXY[3][1] = -dz*ty-h1;
       fXY[4][0] = dz*tx-h2*ta2-bl2;    fXY[4][1] = dz*ty-h2;
       fXY[5][0] = dz*tx+h2*ta2-tl2;    fXY[5][1] = dz*ty+h2;
       fXY[6][0] = dz*tx+h2*ta2+tl2;    fXY[6][1] = dz*ty+h2;
       fXY[7][0] = dz*tx-h2*ta2+bl2;    fXY[7][1] = dz*ty-h2;
    */



    TGeoRotation m1, m2, m3;
    m1.RotateZ(Rphi1);
    m2.RotateY(Rtheta);//m2 = HepRotateY3D( (90-crystal_rotate*zsign)/180*PI );
    m3.RotateZ(Rphi2);//m3 = HepRotateZ3D(k_phi_TILTED);
    TGeoTranslation  position("gTrans", Pr*sin(Ptheta*PI / 180)*cos(Pphi*PI / 180)  , Pr*sin(Ptheta*PI / 180)*sin(Pphi*PI / 180) , Pr*cos(Ptheta*PI / 180));


    if (i <= 72) {
      for (int iBox = 0; iBox < nblock; ++iBox) {//total 16
//   for (int iBox = 0; iBox < 1; ++iBox) {//total 16
        int cry_id = box_id * nblock + iBox;
        sprintf(stemp_ecap, "SD_cid_%04d", cry_id);
//        boxcopy[cry_id] = gGeoManager->MakeArb8(stemp_ecap, CsI, halflength, vtx);
        boxcopy[cry_id] = gGeoManager->MakeTrap(stemp_ecap, CsI, halflength , 0 , 0, h1 ,   bl1, tl1 , alpha2 , h2   , bl2, tl2, alpha2);
        TGeoRotation    r("gRot", 0.0, 0.0, 360.*iBox / nblock);
        group->AddNode(boxcopy[cry_id], cry_id, new TGeoHMatrix(r* position*m3*m2*m1));

      }
      box_id++;

    }

    if (i > 72) {
      for (int iBox = 0; iBox < nblock; ++iBox) {//total 16
        int cry_id = box_id1 * nblock + iBox + 7776;
        sprintf(stemp_ecap, "SD_cid_%04d", cry_id);
        boxcopy[cry_id] = gGeoManager->MakeTrap(stemp_ecap, CsI, halflength , 0 , 0, h1 ,   bl1, tl1 , alpha2, h2   , bl2, tl2, alpha2);
        TGeoRotation    r("gRot", 0.0, 0.0, 360.*iBox / nblock);
        group->AddNode(boxcopy[cry_id], cry_id, new TGeoHMatrix(r* position*m3*m2*m1));
      }
      box_id1++;
    }

  }//Ncounters
  /*
     double Ffin_vtx[15];//12.01 && 31.36 from document
     Ffin_vtx[0] = 196.3*tan(12.398*PI/180);Ffin_vtx[1]= fin_thickness/2;
     Ffin_vtx[2] = 196.3*tan(12.398*PI/180);Ffin_vtx[3]= -fin_thickness/2;
     Ffin_vtx[4] = 196.3*tan(31.62*PI/180);Ffin_vtx[5]= fin_thickness/2;
     Ffin_vtx[6] = 196.3*tan(31.62*PI/180);Ffin_vtx[7]= -fin_thickness/2;
     Ffin_vtx[8] = 227.0*tan(12.398*PI/180);Ffin_vtx[9]= fin_thickness/2;
     Ffin_vtx[10]= 227.0*tan(12.398*PI/180);Ffin_vtx[11]= -fin_thickness/2;
     Ffin_vtx[12]= 227.0*tan(31.62*PI/180);Ffin_vtx[13]= fin_thickness/2;
     Ffin_vtx[14]= 227.0*tan(31.62*PI/180);Ffin_vtx[15]= -fin_thickness/2;

     TGeoVolume* Front_finPhi = gGeoManager->
     MakeArb8 ("Front_finPhi",Al,(227-196.3)/2, Ffin_vtx);

     double Rfin_vtx[15];//12.01 && 31.36 from document
     Rfin_vtx[0] = 131.42*tan(25.05*PI/180);Rfin_vtx[1]= fin_thickness/2;
     Rfin_vtx[2] = 131.42*tan(25.05*PI/180);Rfin_vtx[3]= -fin_thickness/2;
     Rfin_vtx[4] = 131.42*tan(49.87*PI/180);Rfin_vtx[5]= fin_thickness/2;
     Rfin_vtx[6] = 131.42*tan(49.87*PI/180);Rfin_vtx[7]= -fin_thickness/2;
     Rfin_vtx[8] = 102.32*tan(25.05*PI/180);Rfin_vtx[9]= fin_thickness/2;
     Rfin_vtx[10]= 102.32*tan(25.05*PI/180);Rfin_vtx[11]= -fin_thickness/2;
     Rfin_vtx[12]= 102.32*tan(49.87*PI/180);Rfin_vtx[13]= fin_thickness/2;
     Rfin_vtx[14]= 102.32*tan(49.87*PI/180);Rfin_vtx[15]= -fin_thickness/2;

     TGeoVolume* Rare_finPhi = gGeoManager->
     MakeArb8 ("Rare_finPhi",Al,(131.42-102.32)/2, Rfin_vtx);


  //  for (int iBox=0; iBox<1; ++iBox) {//total 16
    for (int iBox=0; iBox<nblock; ++iBox) {//total 16

      TGeoRotation    r("gRot",0.0,0.0,360.*iBox/16);
      TGeoTranslation  front_fin_z("gTrans",0,0,196.3+ (227-196.3)/2);
      group->AddNode(Front_finPhi, 100004+iBox+16*0, new TGeoHMatrix(r*front_fin_z));
      TGeoTranslation  rare_fin_z("gTrans",0,0,-102.32 -(131.42-102.32)/2);
      group->AddNode(Rare_finPhi , 100004+iBox+16*1, new TGeoHMatrix(r*rare_fin_z));
    }

      TGeoTranslation  in_cyl_z("gTrans",0,0,(-102.16+196.16)/2);
      TGeoVolume* Inner_cylinder = gGeoManager->MakeTube("in_cyl",Al,125.5,125.65,(102.16+196.16)/2);


      TGeoTranslation  F_container_z("gTrans",0,0,0);
      TGeoVolume* Ffin_container = gGeoManager->MakePcon("Ffin_container",Al,0,360,4);
      TGeoPcon *Fpcon = (TGeoPcon*)(Ffin_container->GetShape());
      Fpcon->DefineSection(0,196. ,196. *tan(12.398*PI/180),196.  *tan(31.62*PI/180));
      Fpcon->DefineSection(1,196.3,196.3*tan(12.398*PI/180),196.3 *tan(31.62*PI/180)+0.16);
      Fpcon->DefineSection(2,196.3+0.00000001,196.3*tan(31.62 *PI/180),196.3 *tan(31.62*PI/180)+0.16);
      Fpcon->DefineSection(3,227.0,227.0*tan(31.62 *PI/180),227.0 *tan(31.62*PI/180)+0.16);

      TGeoTranslation  R_container_z("gTrans",0,0,0);
      TGeoVolume* Rfin_container = gGeoManager->MakePcon("Rfin_container",Al,0,360,4);
      TGeoPcon *Rpcon = (TGeoPcon*)(Rfin_container->GetShape());
      Rpcon->DefineSection(0,-131.42,131.42 *tan(49.87 *PI/180),131.42  *tan(49.87*PI/180)+0.16);
      Rpcon->DefineSection(1,-102.32,102.32 *tan(49.87 *PI/180),102.32  *tan(49.87*PI/180)+0.16);
      Rpcon->DefineSection(2,-102.32+0.00000001,102.32 *tan(25.05 *PI/180),102.32  *tan(49.87*PI/180));
      Rpcon->DefineSection(3,-102.02,102.02 *tan(25.05 *PI/180),102.02  *tan(49.87*PI/180));


      group->AddNode(Inner_cylinder, 100001, new TGeoHMatrix(in_cyl_z));
      group->AddNode(Ffin_container, 100002, new TGeoHMatrix(F_container_z));
      group->AddNode(Rfin_container, 100003, new TGeoHMatrix(R_container_z));
  */

}



