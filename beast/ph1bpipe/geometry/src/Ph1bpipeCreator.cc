/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/ph1bpipe/geometry/Ph1bpipeCreator.h>
#include <beast/ph1bpipe/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <framework/gearbox/GearDir.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4VisAttributes.hh>

//Shapes
#include <G4Box.hh>
#include <G4EllipticalTube.hh>
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4IntersectionSolid.hh"
#include <G4UserLimits.hh>
#include "G4Tubs.hh"
#include "G4Trd.hh"

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PH1BPIPE detector */
  namespace ph1bpipe {

    // Register the creator
    /** Creator creates the phase 1 beam pipe for |s| < 4 m geometry */
    geometry::CreatorFactory<Ph1bpipeCreator> Ph1bpipeFactory("PH1BPIPECreator");

    Ph1bpipeCreator::Ph1bpipeCreator(): m_sensitive(0)
    {
      //m_sensitive = new SensitiveDetector();
    }

    Ph1bpipeCreator::~Ph1bpipeCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void Ph1bpipeCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {

      m_sensitive = new SensitiveDetector();

      //Beam pipes 6 pillars
      double pillar_height = 61.9 * CLHEP::cm / 2.;
      double pillar_length = 3. * CLHEP::cm;
      double pillar_width = pillar_length;
      G4VSolid* s_bppillar = new G4Box("s_bppillar", pillar_length, pillar_height, pillar_width);
      G4LogicalVolume* l_bppillar = new G4LogicalVolume(s_bppillar,  geometry::Materials::get("Al") , "l_bppillar", 0, 0);
      G4VisAttributes* white = new G4VisAttributes(G4Colour(1, 1, 1));
      white->SetForceAuxEdgeVisible(true);
      l_bppillar->SetVisAttributes(white);
      double y_offset = -76. * CLHEP::cm + pillar_height;
      G4ThreeVector Pillarpos = G4ThreeVector(0, y_offset, -154.0 * CLHEP::cm);
      new G4PVPlacement(0, Pillarpos, l_bppillar, "p_bppilar1", &topVolume, false, 1);
      Pillarpos = G4ThreeVector(0, y_offset, 154.0 * CLHEP::cm);
      new G4PVPlacement(0, Pillarpos, l_bppillar, "p_bppilar2", &topVolume, false, 1);
      Pillarpos = G4ThreeVector(11.4175236299 * CLHEP::cm, y_offset, -227.758203051 * CLHEP::cm);
      new G4PVPlacement(0, Pillarpos, l_bppillar, "p_bppilar3", &topVolume, false, 1);
      Pillarpos = G4ThreeVector(-10.4976636985 * CLHEP::cm, y_offset, 227.758203051 * CLHEP::cm);
      new G4PVPlacement(0, Pillarpos, l_bppillar, "p_bppilar4", &topVolume, false, 1);
      Pillarpos = G4ThreeVector(-8.71250756316 * CLHEP::cm , y_offset, -209.819190072 * CLHEP::cm);
      new G4PVPlacement(0, Pillarpos, l_bppillar, "p_bppilar5", &topVolume, false, 1);
      Pillarpos = G4ThreeVector(8.71248973173 * CLHEP::cm, y_offset, 209.819190072 * CLHEP::cm);
      new G4PVPlacement(0, Pillarpos, l_bppillar, "p_bppilar6", &topVolume, false, 1);

      //Central beam pipe reinforcement
      double x_reih = 2. * CLHEP::cm / 2.;
      double y_reih = 2.3 * CLHEP::cm / 2.;
      double z_reih = 48. * CLHEP::cm / 2.;
      G4VSolid* s_reih = new G4Box("s_reih", x_reih, y_reih, z_reih);
      G4LogicalVolume* l_reih = new G4LogicalVolume(s_reih,  geometry::Materials::get("Al") , "l_reih", 0, 0);
      l_reih->SetVisAttributes(white);
      G4ThreeVector Reihpos = G4ThreeVector(72.8780869619 * CLHEP::mm, 0, 1.35841468498 * CLHEP::mm);
      new G4PVPlacement(0, Reihpos, l_reih, "p_Reih1", &topVolume, false, 1);
      Reihpos = G4ThreeVector(-72.8780869619 * CLHEP::mm, 0, 1.35841468498 * CLHEP::mm);
      new G4PVPlacement(0, Reihpos, l_reih, "p_Reih2", &topVolume, false, 1);

      double x_reiv = 2. * CLHEP::cm / 2.;
      double y_reiv = 5.2 * CLHEP::cm / 2.;
      double z_reiv = 140. * CLHEP::cm / 2.;
      G4VSolid* s_reiv = new G4Box("s_reiv", x_reiv, y_reiv, z_reiv);
      G4LogicalVolume* l_reiv = new G4LogicalVolume(s_reiv,  geometry::Materials::get("Al") , "l_reiv", 0, 0);
      l_reiv->SetVisAttributes(white);
      //G4ThreeVector Reivpos = G4ThreeVector(0, -77.5018052955 * CLHEP::mm, 0);
      G4ThreeVector Reivpos = G4ThreeVector(0, -83.0 * CLHEP::mm, 0);
      new G4PVPlacement(0, Reivpos, l_reiv, "p_Reiv1", &topVolume, false, 1);
      //Reivpos = G4ThreeVector(0, 77.4981947045 * CLHEP::mm,0 );
      Reivpos = G4ThreeVector(0, 83.0 * CLHEP::mm, 0);
      new G4PVPlacement(0, Reivpos, l_reiv, "p_Reiv2", &topVolume, false, 1);

      /*
      Central beampipe +- pipe_hz = 20cm
      Flanges of centra BP  endcap_hz = 2.2cm thick
      Distance of the end of xshape from IP xpipe_hz1 = 200cm
      Length of far beampipe backward xpipePOS_hz = 168.1cm
      Length of far beampipe backward xpipeMIN_hz = 141.3cm
      */

      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * CLHEP::um);
//    flag_limitStep = true for SynRad simulation
      //double stepMax = 50. *CLHEP::mm;
      //cout << " stepMax = " << stepMax << endl;
      //bool flag_limitStep = false;
//      bool flag_limitStep = true;

///      double stepSize = content.getLength("stepSize", 5*CLHEP::um);
      double SafetyLength = 0.1 * CLHEP ::cm;

      double xpipe_hz1 = content.getLength("xpipe_hz1") * CLHEP::cm;
      double pipe_hz = content.getLength("pipe_hz") * CLHEP::cm;
      double pipe_outerRadius_x = content.getLength("pipe_outerRadius_x") * CLHEP::cm;
      double pipe_outerRadius_y = content.getLength("pipe_outerRadius_y") * CLHEP::cm;
      double pipe_innerRadius_x = content.getLength("pipe_innerRadius_x") * CLHEP::cm;
      double pipe_innerRadius_y = content.getLength("pipe_innerRadius_y") * CLHEP::cm;
      double xpipe_innerRadius = content.getLength("xpipe_innerRadius") * CLHEP::cm;
      double xpipe_outerRadius = content.getLength("xpipe_outerRadius") * CLHEP::cm;
      double pipe_innerRadiusTiN_x = content.getLength("pipe_innerRadiusTiN_x") * CLHEP::cm;
      double pipe_innerRadiusTiN_y = content.getLength("pipe_innerRadiusTiN_y") * CLHEP::cm;
      double xpipe_innerRadiusTiN = content.getLength("xpipe_innerRadiusTiN") * CLHEP::cm;
      double endcap_hz = content.getLength("endcap_hz") * CLHEP::cm;
      double endcap_outerRadius = content.getLength("endcap_outerRadius") * CLHEP::cm;
      double xpipePOS_hz = content.getLength("xpipePOS_hz") * CLHEP::cm;
      double xpipeMIN_hz = content.getLength("xpipeMIN_hz") * CLHEP::cm;

      //create ph1bpipe volume
      double startAngle = 0.*CLHEP::deg;
      double spanningAngle = 360.*CLHEP::deg;
      double A1 = 0.0415 * CLHEP::rad;
//      //cout << " A1 = " << A1 << endl;
      double OBtemp = sqrt(pow(xpipe_hz1, 2) + pow(xpipe_innerRadius / 2., 2));
      // cout << " OBtemp = " << OBtemp << endl;
      double BCtemp = OBtemp * sin(A1);
      // cout << " BCtemp = " << BCtemp << endl;
      double ABtemp = sqrt(pow(pipe_hz, 2) + pow(OBtemp, 2) - 2 * pipe_hz * OBtemp * cos(A1));
      // cout << " ABtemp = " << ABtemp << endl;
      double A2 = asin(BCtemp / ABtemp) * CLHEP::rad;
      // cout << " A2 = " << A2 << endl;
      double xpipe_hz = ABtemp - 2 * endcap_hz / cos(A2);  // Length of xshape pipe
      // cout << " xpipe_hz = " << xpipe_hz << endl;
      double xcont2 = BCtemp + xpipe_outerRadius;                // +Z size of AluCont_f trapezoid
      double xcont1 = xpipe_outerRadius + 2 * endcap_hz * tan(A2); //-Z size of AluCont_f trapezoid
      double xcont3 = xcont2 + xpipePOS_hz * sin(A1);            // +Z size of AluCont_fp trapezoid
      double xcont4 = xcont3 - 2 * xpipe_outerRadius;            // +Z size of AluCont_fps trapezoid
      double xcont5 = xcont2 - 2 * xpipe_outerRadius;            // -Z size of AluCont_fps trapezoid
      double xcont3M = xcont2 + xpipeMIN_hz * sin(A1);            // +Z size of AluCont_bp trapezoid
      double xcont4M = xcont3M - 2 * xpipe_outerRadius;            // +Z size of AluCont_bps trapezoid
      // cout << " xcont2 = " << xcont2 << "  xcont1 = " << xcont1 << endl;
      // cout << " xcont3 = " << xcont3 << "  xcont4 = " << xcont4 << "  xcont5 = " << xcont5 << endl;
      // cout << " xcont3M = " << xcont3M << "  xcont4M = " << xcont4M << endl;
      double dxtr = 0.5 * (ABtemp + 2 * endcap_hz / cos(A2)) * sin(2 * A2);
      double dztr = (ABtemp + 2 * endcap_hz / cos(A2)) * sin(A2) * sin(A2);
      // cout << "dxtr  = " << dxtr << "  dztr   = " << dztr << endl;

      string matPipe = content.getString("MaterialPipe");
      string matTiN = content.getString("MaterialTiN");
      string vacPipe = content.getString("MatVacuum");
      G4double tubinR = 0.0 * CLHEP::cm;

      G4VSolid* s_PH1BPIPE = new G4EllipticalTube("s_PH1BPIPE",
                                                  pipe_outerRadius_x,
                                                  pipe_outerRadius_y,
                                                  pipe_hz);
      G4LogicalVolume* l_PH1BPIPE = new G4LogicalVolume(s_PH1BPIPE, geometry::Materials::get(matPipe), "l_PH1BPIPE", 0, 0);
      //Lets limit the Geant4 stepsize inside the volume
      l_PH1BPIPE->SetUserLimits(new G4UserLimits(stepSize));
      //position central ph1bpipe volume
      G4ThreeVector PH1BPIPEpos = G4ThreeVector(
                                    content.getLength("x_ph1bpipe") * CLHEP::cm,
                                    content.getLength("y_ph1bpipe") * CLHEP::cm,
                                    content.getLength("z_ph1bpipe") * CLHEP::cm
                                  );
      new G4PVPlacement(0, PH1BPIPEpos, l_PH1BPIPE, "p_PH1BPIPE", &topVolume, false, 0);

      G4VSolid* s_PH1BPIPETiN = new G4EllipticalTube("s_PH1BPIPETiN",
                                                     pipe_innerRadius_x,
                                                     pipe_innerRadius_y,
                                                     pipe_hz);
      G4LogicalVolume* l_PH1BPIPETiN = new G4LogicalVolume(s_PH1BPIPETiN, geometry::Materials::get(matTiN), "l_PH1BPIPETiN", 0, 0);
      new G4PVPlacement(0, PH1BPIPEpos, l_PH1BPIPETiN, "p_PH1BPIPETiN", l_PH1BPIPE , false, 0);

      G4VSolid* s_PH1BPIPEV = new G4EllipticalTube("s_PH1BPIPEV",
                                                   pipe_innerRadiusTiN_x,
                                                   pipe_innerRadiusTiN_y,
                                                   pipe_hz);
      G4VSolid* s_PH1BPIPEVac = new G4IntersectionSolid("s_PH1BPIPEVac", s_PH1BPIPE, s_PH1BPIPEV);
      G4LogicalVolume* l_PH1BPIPEVac = new G4LogicalVolume(s_PH1BPIPEVac, geometry::Materials::get(vacPipe), "l_PH1BPIPEVac", 0, 0);
      //if (flag_limitStep) l_PH1BPIPEVac->SetUserLimits(new G4UserLimits(stepMax));
      new G4PVPlacement(0, PH1BPIPEpos, l_PH1BPIPEVac, "p_PH1BPIPEVac", l_PH1BPIPETiN , false, 0);

      //create central endcaps
      G4VSolid* s_PH1BPIPEendcap = new G4Tubs("s_PH1BPIPEendcap", 0.,
                                              endcap_outerRadius,
                                              endcap_hz,
                                              startAngle, spanningAngle);
      G4LogicalVolume* l_PH1BPIPEendcapTop = new G4LogicalVolume(s_PH1BPIPEendcap, geometry::Materials::get(matPipe),
                                                                 "l_PH1BPIPEendcapTop", 0, 0);
      G4LogicalVolume* l_PH1BPIPEendcapBot = new G4LogicalVolume(s_PH1BPIPEendcap, geometry::Materials::get(matPipe),
                                                                 "l_PH1BPIPEendcapBot", 0, 0);
      //position central endcaps
      G4ThreeVector PH1BPIPEendcapposTop = G4ThreeVector(
                                             content.getLength("x_ph1bpipe") * CLHEP::cm,
                                             content.getLength("y_ph1bpipe") * CLHEP::cm,
                                             content.getLength("z_ph1bpipe") * CLHEP::cm + pipe_hz + endcap_hz
                                           );

      G4ThreeVector PH1BPIPEendcapposBot = G4ThreeVector(
                                             content.getLength("x_ph1bpipe") * CLHEP::cm,
                                             content.getLength("y_ph1bpipe") * CLHEP::cm,
                                             content.getLength("z_ph1bpipe") * CLHEP::cm - pipe_hz - endcap_hz
                                           );
      new G4PVPlacement(0, PH1BPIPEendcapposTop, l_PH1BPIPEendcapTop, "p_PH1BPIPEendcapTop", &topVolume, false, 0);
      new G4PVPlacement(0, PH1BPIPEendcapposBot, l_PH1BPIPEendcapBot, "p_PH1BPIPEendcapBot", &topVolume, false, 0);

      G4VSolid* s_PH1BPIPEendcapTiN = new G4EllipticalTube("s_PH1BPIPEendcapTiN",
                                                           pipe_innerRadius_x,
                                                           pipe_innerRadius_y,
                                                           endcap_hz);
      G4LogicalVolume* l_PH1BPIPEendcapTiNTop = new G4LogicalVolume(s_PH1BPIPEendcapTiN, geometry::Materials::get(matTiN),
          "l_PH1BPIPEendcapTiNTop", 0, 0);
      G4LogicalVolume* l_PH1BPIPEendcapTiNBot = new G4LogicalVolume(s_PH1BPIPEendcapTiN, geometry::Materials::get(matTiN),
          "l_PH1BPIPEendcapTiNBot", 0, 0);
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), l_PH1BPIPEendcapTiNTop, "p_PH1BPIPEendcapTiNTop", l_PH1BPIPEendcapTop , false, 0);
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), l_PH1BPIPEendcapTiNBot, "p_PH1BPIPEendcapTiNBot", l_PH1BPIPEendcapBot , false, 0);

      G4VSolid* s_PH1BPIPEendcapV = new G4EllipticalTube("s_PH1BPIPEendcapV",
                                                         pipe_innerRadiusTiN_x,
                                                         pipe_innerRadiusTiN_y,
                                                         endcap_hz);
      G4VSolid* s_PH1BPIPEendcapVac = new G4IntersectionSolid("s_PH1BPIPEendcapVac", s_PH1BPIPEendcap , s_PH1BPIPEendcapV);
      G4LogicalVolume* l_PH1BPIPEendcapVac = new G4LogicalVolume(s_PH1BPIPEendcapVac, geometry::Materials::get(vacPipe),
                                                                 "l_PH1BPIPEendcapVac", 0, 0);
      //if (flag_limitStep) l_PH1BPIPEendcapVac->SetUserLimits(new G4UserLimits(stepMax));
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), l_PH1BPIPEendcapVac, "p_PH1BPIPEendcapVacTop", l_PH1BPIPEendcapTiNTop, false, 0);
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), l_PH1BPIPEendcapVac, "p_PH1BPIPEendcapVacBot", l_PH1BPIPEendcapTiNBot, false, 0);

      //create x shape tubes forward
      // Alu trapezoid containing x-shape tube
      G4VSolid* AluCont_f = new G4Trd("AluCont_f", xcont1, xcont2, xcont1, xcont1, xpipe_hz / 2.0 * cos(A2));
      G4LogicalVolume* logi_AluCont_f = new G4LogicalVolume(AluCont_f, geometry::Materials::get(matPipe), "logi_AluCont_f", 0 , 0);
      new G4PVPlacement(0, G4ThreeVector(0, 0, pipe_hz + endcap_hz * 2.0 + 0.5 * xpipe_hz * cos(A2)),
                        logi_AluCont_f, "phys_AluCont_f", &topVolume, false, 0);
//      //cout << " AluCont_f z pos = " << pipe_hz + endcap_hz*2.0 + xpipe_hz*cos(A2)/2. << endl;
//      //cout << " AluCont_f zmax pos = " << pipe_hz + endcap_hz*2.0 + xpipe_hz*cos(A2) << endl;

      // create x part
      G4VSolid* s_Xshapef_11 = new G4Tubs("s_Xshapef_11",
                                          tubinR,
                                          xpipe_innerRadius,
                                          xpipe_hz,
                                          startAngle, spanningAngle);
      G4VSolid* s_Xshapef_12 = new G4Tubs("s_Xshapef_12",
                                          tubinR,
                                          xpipe_innerRadius * 2.0,
                                          xpipe_hz / 2.0 ,
                                          startAngle, spanningAngle);
      G4VSolid* s_Xshapef_21 = new G4Tubs("s_Xshapef_21",
                                          tubinR,
                                          xpipe_innerRadiusTiN,
                                          xpipe_hz,
                                          startAngle, spanningAngle);
      G4VSolid* s_Xshapef_22 = new G4Tubs("s_Xshapef_22",
                                          tubinR,
                                          xpipe_innerRadiusTiN * 2.0,
                                          xpipe_hz / 2.0 ,
                                          startAngle, spanningAngle);
      //create TiN layer on

      G4VSolid* s_XshapefTiN_11 = new G4Tubs("s_XshapefTiN_11",
                                             tubinR,
                                             xpipe_innerRadius,
                                             xpipe_hz,
                                             startAngle, spanningAngle);
      G4VSolid* s_XshapefTiN_12 = new G4Tubs("s_XshapefTiN_12",
                                             tubinR,
                                             xpipe_innerRadius * 2.0,
                                             xpipe_hz / 2.0 ,
                                             startAngle, spanningAngle);
      G4VSolid* s_XshapefTiN_21 = new G4Tubs("s_XshapefTiN_21",
                                             tubinR,
                                             xpipe_innerRadius,
                                             xpipe_hz,
                                             startAngle, spanningAngle);
      G4VSolid* s_XshapefTiN_22 = new G4Tubs("s_XshapefTiN_22",
                                             tubinR,
                                             xpipe_innerRadius * 2.0,
                                             xpipe_hz / 2.0 ,
                                             startAngle, spanningAngle);
      //Slanted tube1
      G4Transform3D transform_sXshapef_12 = G4Translate3D(0., 0., 0.);
      transform_sXshapef_12 = transform_sXshapef_12 * G4RotateY3D(-A2);
      G4IntersectionSolid* Xshapef1 = new G4IntersectionSolid("Xshapef1", s_Xshapef_11, s_Xshapef_12, transform_sXshapef_12);
      //Slanted tube2
      G4Transform3D transform_sXshapef_22 = G4Translate3D(0., 0., 0.);
      transform_sXshapef_22 = transform_sXshapef_22 * G4RotateY3D(A2);
      G4IntersectionSolid* Xshapef2 = new G4IntersectionSolid("Xshapef2", s_Xshapef_21, s_Xshapef_22, transform_sXshapef_22);
      //Slanted tube1TiN
      G4Transform3D transform_sXshapefTiN_12 = G4Translate3D(0., 0., 0.);
      transform_sXshapefTiN_12 = transform_sXshapefTiN_12 * G4RotateY3D(-A2);
      G4IntersectionSolid* XshapefTiN1 = new G4IntersectionSolid("XshapefTiN1", s_XshapefTiN_11, s_XshapefTiN_12,
                                                                 transform_sXshapefTiN_12);
      //Slanted tube2TiN
      G4Transform3D transform_sXshapefTiN_22 = G4Translate3D(0., 0., 0.);
      transform_sXshapefTiN_22 = transform_sXshapefTiN_22 * G4RotateY3D(A2);
      G4IntersectionSolid* XshapefTiN2 = new G4IntersectionSolid("XshapefTiN2", s_XshapefTiN_21, s_XshapefTiN_22,
                                                                 transform_sXshapefTiN_22);

      // (sXshapeTiN2 + sXshapeTiN1)
      G4Transform3D transform_XshapefTiN1 = G4Translate3D(dxtr, 0. , -dztr);
      transform_XshapefTiN1 = transform_XshapefTiN1 * G4RotateY3D(2.*A2);
      G4UnionSolid* XshapeTiNForwx = new G4UnionSolid("XshapeTiNForwx", XshapefTiN2 , XshapefTiN1 , transform_XshapefTiN1);
      // Place XshapeTiNForwx into logi_AluCont_f
      G4RotationMatrix* yRot = new G4RotationMatrix;
      yRot->rotateY(A2);
      G4ThreeVector transform_XshapeTiNForwx(-dxtr / 2., 0., 0.);
      G4IntersectionSolid* XshapeTiNForw = new G4IntersectionSolid("XshapeTiNForw", AluCont_f, XshapeTiNForwx, yRot,
          transform_XshapeTiNForwx);
      G4LogicalVolume* l_XshapeTiNForw = new G4LogicalVolume(XshapeTiNForw, geometry::Materials::get(matTiN), "l_XshapeTiNForw", 0 , 0);
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), l_XshapeTiNForw, "p_XshapeTiNForw", logi_AluCont_f , false, 0);

      // sXshape2 + sXshape1
      G4Transform3D transform_Xshapef1 = G4Translate3D(dxtr , 0. , -dztr);
      transform_Xshapef1 = transform_Xshapef1 * G4RotateY3D(2.*A2);
      G4UnionSolid* XshapeForwx = new G4UnionSolid("XshapeForwx", Xshapef2 , Xshapef1 , transform_Xshapef1);
      // Place XshapeForwx into l_XshapeTiNForw
      G4Transform3D transform_XshapeForwx = G4Translate3D(-dxtr / 2., 0., 0.);
      transform_XshapeForwx = transform_XshapeForwx * G4RotateY3D(-A2);
      G4IntersectionSolid* XshapeForw = new G4IntersectionSolid("XshapeForw", AluCont_f, XshapeForwx, transform_XshapeForwx);
      G4LogicalVolume* l_XshapeForw = new G4LogicalVolume(XshapeForw, geometry::Materials::get(vacPipe), "l_XshapeForw", 0 , 0);
      //if (flag_limitStep) l_XshapeForw ->SetUserLimits(new G4UserLimits(stepMax));
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), l_XshapeForw, "p_XshapeForw", l_XshapeTiNForw , false, 0);

      //create far forward parts
      // Alu trapezoid containing far parts
      G4VSolid* AluCont_fpO = new G4Trd("AluCont_fpO", xcont2, xcont3, xcont1, xcont1, xpipePOS_hz / 2.0 * cos(A1));
      G4VSolid* AluCont_fps = new G4Trd("AluCont_fps", xcont5, xcont4, xcont1, xcont1, xpipePOS_hz / 2.0 * cos(A1));
      G4Transform3D transform_AluCont_fps = G4Translate3D(0., 0., 0.);
      G4SubtractionSolid* AluCont_fp = new G4SubtractionSolid("AluCont_fp", AluCont_fpO, AluCont_fps, transform_AluCont_fps);
      G4LogicalVolume* logi_AluCont_fp = new G4LogicalVolume(AluCont_fp, geometry::Materials::get(matPipe), "logi_AluCont_fp", 0 , 0);
      new G4PVPlacement(0, G4ThreeVector(0, 0, pipe_hz + endcap_hz * 2.0 + xpipe_hz * cos(A2) + xpipePOS_hz / 2.0 * cos(A1)),
                        logi_AluCont_fp, "phys_AluCont_fp", &topVolume, false, 0);
      //HER far forward part
      //slanted tubes Vacuum
      G4VSolid* s_XshapefPOS_11 = new G4Tubs("s_XshapefPOS11",
                                             tubinR,
                                             xpipe_innerRadius,
                                             xpipePOS_hz,
                                             startAngle, spanningAngle);
      G4VSolid* s_XshapefPOS_12 = new G4Tubs("s_XshapefPOS12",
                                             tubinR,
                                             xpipe_innerRadius * 2,
                                             xpipePOS_hz / 2. - 0.7568202457 * SafetyLength,
                                             startAngle, spanningAngle);
      G4Transform3D transform_sXshapefPOS_12 = G4Translate3D(0., 0., 0.);
      transform_sXshapefPOS_12 = transform_sXshapefPOS_12 * G4RotateY3D(-A1);
      G4IntersectionSolid* XshapefPOS1 = new G4IntersectionSolid("XshapefPOS1", s_XshapefPOS_11, s_XshapefPOS_12,
                                                                 transform_sXshapefPOS_12);
      //put HER vacuum part in AluCont_fp
      G4LogicalVolume* l_XshapefPOS1 = new G4LogicalVolume(XshapefPOS1, geometry::Materials::get(vacPipe), "l_XshapefPOS1", 0 , 0);
      //if (flag_limitStep) l_XshapefPOS1 ->SetUserLimits(new G4UserLimits(stepMax));
      G4RotationMatrix* yRotP = new G4RotationMatrix;
      yRotP->rotateY(-A1);
      new G4PVPlacement(yRotP, G4ThreeVector(2 * endcap_hz * tan(A2) + xpipe_hz * sin(A2) + xpipePOS_hz / 2.*sin(A1), 0., 0.),
                        l_XshapefPOS1 , "p_XshapefPOS1", logi_AluCont_fp, false, 0);
      //LER far forward part
      //slanted tubes TiN
      G4VSolid* s_XshapefPOS_21 = new G4Tubs("s_XshapefPOS21",
                                             tubinR,
                                             xpipe_innerRadius,
                                             xpipePOS_hz,
                                             startAngle, spanningAngle);
      G4VSolid* s_XshapefPOS_22 = new G4Tubs("s_XshapefPOS22",
                                             tubinR,
                                             xpipe_innerRadius * 2,
                                             xpipePOS_hz / 2. - 0.7568202457 * SafetyLength,
                                             startAngle, spanningAngle);
      G4Transform3D transform_sXshapefPOS_22 = G4Translate3D(0., 0., 0.);
      transform_sXshapefPOS_22 = transform_sXshapefPOS_22 * G4RotateY3D(A1);
      G4IntersectionSolid* XshapefPOS2 = new G4IntersectionSolid("XshapefPOS2", s_XshapefPOS_21, s_XshapefPOS_22,
                                                                 transform_sXshapefPOS_22);
      //put HER TiN part in AluCont_fp
      G4LogicalVolume* l_XshapefPOS2 = new G4LogicalVolume(XshapefPOS2, geometry::Materials::get(matTiN), "l_XshapefPOS2", 0 , 0);
      G4RotationMatrix* yRotM = new G4RotationMatrix;
      yRotM->rotateY(A1);
      new G4PVPlacement(yRotM, G4ThreeVector(-(2 * endcap_hz * tan(A2) + xpipe_hz * sin(A2) + xpipePOS_hz / 2.*sin(A1)), 0., 0.),
                        l_XshapefPOS2 , "p_XshapefPOS2", logi_AluCont_fp, false, 0);
      //slanted tubes Vacuum
      G4VSolid* s_XshapefPOS_31 = new G4Tubs("s_XshapefPOS31",
                                             tubinR,
                                             xpipe_innerRadiusTiN,
                                             xpipePOS_hz,
                                             startAngle, spanningAngle);
      G4VSolid* s_XshapefPOS_32 = new G4Tubs("s_XshapefPOS32",
                                             tubinR,
                                             xpipe_innerRadiusTiN * 2,
                                             xpipePOS_hz / 2. - 0.7568202457 * SafetyLength,
                                             startAngle, spanningAngle);
      G4Transform3D transform_sXshapefPOS_32 = G4Translate3D(0., 0., 0.);
      transform_sXshapefPOS_32 = transform_sXshapefPOS_32 * G4RotateY3D(A1);
      G4IntersectionSolid* XshapefPOS3 = new G4IntersectionSolid("XshapefPOS3", s_XshapefPOS_31, s_XshapefPOS_32,
                                                                 transform_sXshapefPOS_32);
      //put HER vacuum part into l_XshapefPOS2
      G4LogicalVolume* l_XshapefPOS3 = new G4LogicalVolume(XshapefPOS3, geometry::Materials::get(vacPipe), "l_XshapefPOS3", 0 , 0);
      //if (flag_limitStep) l_XshapefPOS3 ->SetUserLimits(new G4UserLimits(stepMax));
      new G4PVPlacement(0, G4ThreeVector(0. , 0., 0.), l_XshapefPOS3 , "p_XshapefPOS3", l_XshapefPOS2, false, 0);

      // create x-shape tubes backward
      // Alu trapezoid containing x-shape tube
      G4VSolid* AluCont_b = new G4Trd("AluCont_b", xcont2, xcont1, xcont1, xcont1, xpipe_hz / 2.0 * cos(A2));
      G4LogicalVolume* logi_AluCont_b = new G4LogicalVolume(AluCont_b, geometry::Materials::get(matPipe), "logi_AluCont_b", 0 , 0);
      new G4PVPlacement(0, G4ThreeVector(0, 0, -pipe_hz - endcap_hz * 2.0 - xpipe_hz * cos(A2) / 2.), logi_AluCont_b, "phys_AluCont_b",
                        &topVolume, false, 1);
      //cout << " logi_AluCont_b z  = " << -pipe_hz - endcap_hz*2.0 - xpipe_hz*cos(A2)/2. << endl;
      //cout << " logi_AluCont_b zmax  = " << -pipe_hz - endcap_hz*2.0 - xpipe_hz*cos(A2) << endl;

      // create x part
      G4VSolid* s_Xshapeb_11 = new G4Tubs("s_Xshapeb_11",
                                          tubinR,
                                          xpipe_innerRadiusTiN,
                                          xpipe_hz,
                                          startAngle, spanningAngle);
      G4VSolid* s_Xshapeb_12 = new G4Tubs("s_Xshapeb_12",
                                          tubinR,
                                          xpipe_innerRadiusTiN * 2.0,
                                          xpipe_hz / 2.0,
                                          startAngle, spanningAngle);
      G4VSolid* s_Xshapeb_21 = new G4Tubs("s_Xshapeb_21",
                                          tubinR,
                                          xpipe_innerRadius,
                                          xpipe_hz,
                                          startAngle, spanningAngle);
      G4VSolid* s_Xshapeb_22 = new G4Tubs("s_Xshapeb_22",
                                          tubinR,
                                          xpipe_innerRadius * 2.0,
                                          xpipe_hz / 2.0,
                                          startAngle, spanningAngle);
      //create TiN layer on
      G4VSolid* s_XshapebTiN_11 = new G4Tubs("s_XshapebTiN_11",
                                             tubinR,
                                             xpipe_innerRadius,
                                             xpipe_hz,
                                             startAngle, spanningAngle);
      G4VSolid* s_XshapebTiN_12 = new G4Tubs("s_XshapebTiN_12",
                                             tubinR,
                                             xpipe_innerRadius * 2.0,
                                             xpipe_hz / 2.0,
                                             startAngle, spanningAngle);
      G4VSolid* s_XshapebTiN_21 = new G4Tubs("s_XshapebTiN_21",
                                             tubinR,
                                             xpipe_innerRadius,
                                             xpipe_hz,
                                             startAngle, spanningAngle);
      G4VSolid* s_XshapebTiN_22 = new G4Tubs("s_XshapebTiN_22",
                                             tubinR,
                                             xpipe_innerRadius * 2.0,
                                             xpipe_hz / 2.0,
                                             startAngle, spanningAngle);
      //Slanted tube1
      G4Transform3D transform_sXshapeb_12 = G4Translate3D(0., 0., 0.);
      transform_sXshapeb_12 = transform_sXshapeb_12 * G4RotateY3D(A2);
      G4IntersectionSolid* Xshapeb1 = new G4IntersectionSolid("Xshapeb1", s_Xshapeb_11, s_Xshapeb_12, transform_sXshapeb_12);
      //Slanted tube2
      G4Transform3D transform_sXshapeb_22 = G4Translate3D(0., 0., 0.);
      transform_sXshapeb_22 = transform_sXshapeb_22 * G4RotateY3D(-A2);
      G4IntersectionSolid* Xshapeb2 = new G4IntersectionSolid("Xshapeb2", s_Xshapeb_21, s_Xshapeb_22, transform_sXshapeb_22);
      //Slanted tube1TiN
      G4Transform3D transform_sXshapebTiN_12 = G4Translate3D(0., 0., 0.);
      transform_sXshapebTiN_12 = transform_sXshapebTiN_12 * G4RotateY3D(A2);
      G4IntersectionSolid* XshapebTiN1 = new G4IntersectionSolid("XshapebTiN1", s_XshapebTiN_11, s_XshapebTiN_12,
                                                                 transform_sXshapebTiN_12);
      //Slanted tube2TiN
      G4Transform3D transform_sXshapebTiN_22 = G4Translate3D(0., 0., 0.);
      transform_sXshapebTiN_22 = transform_sXshapebTiN_22 * G4RotateY3D(-A2);
      G4IntersectionSolid* XshapebTiN2 = new G4IntersectionSolid("XshapebTiN2", s_XshapebTiN_21, s_XshapebTiN_22,
                                                                 transform_sXshapebTiN_22);

      // (sXshapeTiN2 + sXshapeTiN1)
      G4Transform3D transform_XshapebTiN1 = G4Translate3D(dxtr, 0. , dztr);
      //cout << "dxtr= " << dxtr << "  dztr= " << dztr << endl;;
      transform_XshapebTiN1 = transform_XshapebTiN1 * G4RotateY3D(-2.*A2);
      G4UnionSolid* XshapeTiNBackwx = new G4UnionSolid("XshapeTiNBackwx", XshapebTiN2 , XshapebTiN1 , transform_XshapebTiN1);
      // Place XshapeTiNBackwx into logi_AluCont_b
      G4Transform3D transform_XshapeTiNBackwx = G4Translate3D(-dxtr / 2., 0., 0.);
      transform_XshapeTiNBackwx = transform_XshapeTiNBackwx * G4RotateY3D(A2);
      G4IntersectionSolid* XshapeTiNBackw = new G4IntersectionSolid("XshapeTiNBackw", AluCont_b, XshapeTiNBackwx,
          transform_XshapeTiNBackwx);
      G4LogicalVolume* l_XshapeTiNBackw = new G4LogicalVolume(XshapeTiNBackw, geometry::Materials::get(matTiN), "l_XshapeTiNBackw", 0 ,
                                                              0);
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), l_XshapeTiNBackw, "p_XshapeTiNBackw", logi_AluCont_b , false, 0);

      // sXshape2 + sXshape1
      G4Transform3D transform_Xshapeb1 = G4Translate3D(dxtr, 0. , dztr);
      transform_Xshapeb1 = transform_Xshapeb1 * G4RotateY3D(-2.*A2);
      G4UnionSolid* XshapeBackwx = new G4UnionSolid("XshapebBackwx", Xshapeb2 , Xshapeb1 , transform_Xshapeb1);
      G4Transform3D transform_XshapeBackwx = G4Translate3D(-dxtr / 2., 0., 0.);
      transform_XshapeBackwx = transform_XshapeBackwx * G4RotateY3D(A2);
      G4IntersectionSolid* XshapeBackw = new G4IntersectionSolid("XshapeBackw", AluCont_b, XshapeBackwx, transform_XshapeBackwx);
      G4LogicalVolume* l_XshapeBackw = new G4LogicalVolume(XshapeBackw, geometry::Materials::get(vacPipe), "l_XshapeBackw", 0 , 0);
      //if (flag_limitStep) l_XshapeBackw->SetUserLimits(new G4UserLimits(stepMax));
      // Place XshapeBackw
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), l_XshapeBackw, "p_XshapeBackw", l_XshapeTiNBackw , false, 0);

      //create far backward parts
      // Alu trapezoid containing far parts
      G4VSolid* AluCont_bpO = new G4Trd("AluCont_bpO", xcont3M, xcont2, xcont1, xcont1, xpipeMIN_hz / 2.0 * cos(A1));
      G4VSolid* AluCont_bps = new G4Trd("AluCont_bps", xcont4M, xcont5, xcont1, xcont1, xpipeMIN_hz / 2.0 * cos(A1));
      G4Transform3D transform_AluCont_bps = G4Translate3D(0., 0., 0.);
      G4SubtractionSolid* AluCont_bp = new G4SubtractionSolid("AluCont_bp", AluCont_bpO, AluCont_bps, transform_AluCont_bps);
      G4LogicalVolume* logi_AluCont_bp = new G4LogicalVolume(AluCont_bp, geometry::Materials::get(matPipe), "logi_AluCont_bp", 0 , 0);
      new G4PVPlacement(0, G4ThreeVector(0, 0, -(pipe_hz + endcap_hz * 2.0 + xpipe_hz * cos(A2) + xpipeMIN_hz / 2.0 * cos(A1))),
                        logi_AluCont_bp, "phys_AluCont_bp", &topVolume, false, 0);
      //HER far backward part
      //slanted tubes Vacuum
      G4VSolid* s_XshapebMIN_11 = new G4Tubs("s_XshapebMIN11",
                                             tubinR,
                                             xpipe_innerRadius,
                                             xpipeMIN_hz,
                                             startAngle, spanningAngle);
      G4VSolid* s_XshapebMIN_12 = new G4Tubs("s_XshapebMIN12",
                                             tubinR,
                                             xpipe_innerRadius * 2,
                                             xpipeMIN_hz / 2. - 0.525641366 * SafetyLength,
                                             startAngle, spanningAngle);
      G4Transform3D transform_sXshapebMIN_12 = G4Translate3D(0., 0., 0.);
      transform_sXshapebMIN_12 = transform_sXshapebMIN_12 * G4RotateY3D(-A1);
      G4IntersectionSolid* XshapebMIN1 = new G4IntersectionSolid("XshapebMIN1", s_XshapebMIN_11, s_XshapebMIN_12,
                                                                 transform_sXshapebMIN_12);
      //put HER vacuum part in AluCont_bp
      G4LogicalVolume* l_XshapebMIN1 = new G4LogicalVolume(XshapebMIN1, geometry::Materials::get(vacPipe), "l_XshapebMIN1", 0 , 0);
      //if (flag_limitStep) l_XshapebMIN1 ->SetUserLimits(new G4UserLimits(stepMax));
      new G4PVPlacement(yRotP, G4ThreeVector(-(2 * endcap_hz * tan(A2) + xpipe_hz * sin(A2) + xpipeMIN_hz / 2.*sin(A1)), 0., 0.),
                        l_XshapebMIN1 , "p_XshapebMIN1", logi_AluCont_bp, false, 0);
      //LER far backward part
      //slanted tubes TiN
      G4VSolid* s_XshapebMIN_21 = new G4Tubs("s_XshapebMIN21",
                                             tubinR,
                                             xpipe_innerRadius,
                                             xpipeMIN_hz,
                                             startAngle, spanningAngle);
      G4VSolid* s_XshapebMIN_22 = new G4Tubs("s_XshapebMIN22",
                                             tubinR,
                                             xpipe_innerRadius * 2,
                                             xpipeMIN_hz / 2. - 0.525641366 * SafetyLength,
                                             startAngle, spanningAngle);
      G4Transform3D transform_sXshapebMIN_22 = G4Translate3D(0., 0., 0.);
      transform_sXshapebMIN_22 = transform_sXshapebMIN_22 * G4RotateY3D(A1);
      G4IntersectionSolid* XshapebMIN2 = new G4IntersectionSolid("XshapebMIN2", s_XshapebMIN_21, s_XshapebMIN_22,
                                                                 transform_sXshapebMIN_22);
      //put HER TiN part in AluCont_bp
      G4LogicalVolume* l_XshapebMIN2 = new G4LogicalVolume(XshapebMIN2, geometry::Materials::get(matTiN), "l_XshapebMIN2", 0 , 0);
      new G4PVPlacement(yRotM, G4ThreeVector(2 * endcap_hz * tan(A2) + xpipe_hz * sin(A2) + xpipeMIN_hz / 2.*sin(A1), 0., 0.),
                        l_XshapebMIN2 , "p_XshapebMIN2", logi_AluCont_bp, false, 0);
      //slanted tubes Vacuum
      G4VSolid* s_XshapebMIN_31 = new G4Tubs("s_XshapebMIN31",
                                             tubinR,
                                             xpipe_innerRadiusTiN,
                                             xpipeMIN_hz,
                                             startAngle, spanningAngle);
      G4VSolid* s_XshapebMIN_32 = new G4Tubs("s_XshapebMIN32",
                                             tubinR,
                                             xpipe_innerRadiusTiN * 2,
                                             xpipeMIN_hz / 2. - 0.525641366 * SafetyLength,
                                             startAngle, spanningAngle);
      G4Transform3D transform_sXshapebMIN_32 = G4Translate3D(0., 0., 0.);
      transform_sXshapebMIN_32 = transform_sXshapebMIN_32 * G4RotateY3D(A1);
      G4IntersectionSolid* XshapebMIN3 = new G4IntersectionSolid("XshapebMIN3", s_XshapebMIN_31, s_XshapebMIN_32,
                                                                 transform_sXshapebMIN_32);
      //put HER vacuum part in AluCont_bp
      G4LogicalVolume* l_XshapebMIN3 = new G4LogicalVolume(XshapebMIN3, geometry::Materials::get(vacPipe), "l_XshapebMIN3", 0 , 0);
      //if (flag_limitStep) l_XshapebMIN3 ->SetUserLimits(new G4UserLimits(stepMax));
      new G4PVPlacement(0, G4ThreeVector(0. , 0., 0.), l_XshapebMIN3 , "p_XshapebMIN3", l_XshapebMIN2, false, 0);

    }
  } // ph1bpipe namespace
} // Belle2 namespace
