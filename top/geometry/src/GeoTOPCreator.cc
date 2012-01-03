/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/geometry/GeoTOPCreator.h>
#include <top/geometry/TOPGeometryPar.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <top/simulation/SensitivePMT.h>
#include <top/simulation/SensitiveTrack.h>
#include <simulation/background/BkgSensitiveDetector.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4AssemblyVolume.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4OpticalSurface.hh>
//Shapes
#include <G4Trd.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polyhedra.hh>
#include <G4Polycone.hh>
#include <G4SubtractionSolid.hh>
#include <G4UserLimits.hh>
#include <G4Material.hh>

#include <G4TwoVector.hh>
#include <G4ThreeVector.hh>
#include <G4ExtrudedSolid.hh>
#include <G4UnionSolid.hh>
#include <G4Sphere.hh>
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4Colour.hh>

using namespace std;
using namespace boost;


namespace Belle2 {

  using namespace geometry;

  namespace TOP {

    //-----------------------------------------------------------------
    //                 Register the Creator
    //-----------------------------------------------------------------

    geometry::CreatorFactory<GeoTOPCreator> GeoTOPFactory("TOPCreator");

    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoTOPCreator::GeoTOPCreator(): isBeamBkgStudy(0)
    {
      m_sensitivePMT = new SensitivePMT();
      m_sensitiveTrack = new SensitiveTrack();
      m_topgp = new TOPGeometryPar();
    }

    GeoTOPCreator::~GeoTOPCreator()
    {
      if (m_sensitivePMT) delete m_sensitivePMT;
      if (m_sensitiveTrack) delete m_sensitiveTrack;
      if (m_topgp) delete m_topgp;
    }


    void GeoTOPCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes type)
    {

      isBeamBkgStudy = content.getInt("BeamBackgroundStudy");

      /*! Build detector segment */

      m_topgp = TOPGeometryPar::Instance();
      m_topgp->Initialize(content);


      G4LogicalVolume* air = buildSupport(content);
      G4AssemblyVolume* bar = buildBar(content);

      G4RotationMatrix* rota = new G4RotationMatrix(0, 0, 0);
      G4ThreeVector transa(0, 0, 0);

      bar->MakeImprint(air, transa, rota, 100, false);




      G4double Radius = m_topgp->getRadius() + m_topgp->getQthickness() / 2.0;
      G4int Nbars = m_topgp->getNbars();

      for (G4int i = 0; i < Nbars; i++) {

        G4double phi = i * 2 * M_PI / ((double)Nbars);

        G4RotationMatrix rot(M_PI / 2.0, M_PI / 2.0, -phi);
        G4ThreeVector trans(Radius * cos(phi), Radius * sin(phi), 0);

        new G4PVPlacement(G4Transform3D(rot, trans), air, "PlacedBox", &topVolume, false, i + 1);

      }



    }



    G4AssemblyVolume* GeoTOPCreator::buildBar(const GearDir& content)
    {

      /*!  Read parameters  */

      //! get width of the quartz bar
      G4double Qwidth = m_topgp->getQwidth();
      //! get thickness of the quartz bar
      G4double Qthickness = m_topgp->getQthickness();
      //! get Backward position of qurtz bar
      G4double Bposition = m_topgp->getBposition();
      //! get length of first quartz bar segment which at one side is positioned at Bposition
      G4double Length1 = m_topgp->getLength1();
      //! get length of second quartz bar segment which at one side is positioned at Bposition+Length1
      G4double Length2 = m_topgp->getLength2();
      //! get length segment to which the mirror is attached
      G4double Length3 = m_topgp->getLength3();
      //! get length wedge segment
      G4double WLength = m_topgp->getWLength();
      //! get width wedge segment
      G4double Wwidth = m_topgp->getWwidth();
      //! get wedge extension down
      G4double Wextdown = m_topgp->getWextdown();
      //! get width of glue between wedge and segment 1
      G4double Gwidth1 = m_topgp->getGwidth1();
      //! get width of glue between segment 1 segmen 2
      G4double Gwidth2 = m_topgp->getGwidth2();
      //! get width of glue between segment 3 and mirror segment
      G4double Gwidth3 = m_topgp->getGwidth3();


      //! dispacement along quartz width for mirror axis
      G4double Mirposx = m_topgp->getMirposx();
      //! dispacement along quartz whickness for mirror axis
      G4double Mirposy = m_topgp->getMirposy();
      //! get mirror layer thickness
      G4double Mirthickness = m_topgp->getMirthickness();
      //! get radius of spherical mirror
      G4double Mirradius = m_topgp->getMirradius();


      //!calculate the total length and width of the qurtz bar

      G4double length = Length1 + Length2 + Length3 + Gwidth2 + Gwidth3;

      /*!  Read materials  */

      GearDir materialNames(content, "Bars");
      string quartzName = materialNames.getString("BarMaterial", "TOPSiO2");
      string glueName = materialNames.getString("Glue/GlueMaterial", "TOPGlue");

      G4Material* quartzMaterial = Materials::get(quartzName);
      G4Material* glueMaterial = Materials::get(glueName);

      if (!quartzMaterial) { B2FATAL("Material '" << quartzName << "', required for TOP quarz bars could not be found!");}
      if (!glueMaterial) { B2FATAL("Material '" << quartzName << "', required for gluing of TOP quarz bars could not be found!");}

      /*!  Build raw bar  */

      G4Box* bar = new G4Box("bar", length / 2.0, Qthickness / 2.0 , Qwidth / 2.0);

      G4LogicalVolume* qbar = new G4LogicalVolume(bar, quartzMaterial, "cuttest");
      qbar->SetSensitiveDetector(m_sensitiveTrack);

      /*!  Build shapes of mirror  */


      G4double   pRmin = Mirradius;
      G4double   pRmax = Mirradius + Mirthickness * 100.0;

      G4double   phimax = atan(Qthickness / Mirradius); // cca 0.2 deg
      G4double   thetamax = atan(Qwidth / 2.0 / Mirradius); // cca 2deg


      G4double   pSPhi = M_PI / 2.0 - phimax * 0.5;
      G4double   pDPhi = phimax * 2.0;
      G4double   pSTheta = M_PI / 2.0 - thetamax * 1.5;
      G4double   pDTheta = 3 * thetamax ;

      G4Sphere* mirsphere = new G4Sphere("mirror_sphere_segment", pRmin, pRmax, pSPhi, pDPhi, pSTheta, pDTheta);

      G4RotationMatrix rotmir(M_PI / 2.0 + phimax, 0, 0);
      G4ThreeVector transmir(-Mirradius + length / 2.0, Mirposy, Mirposx);

      G4IntersectionSolid* mirrorshape = new G4IntersectionSolid("bar*mirsphere", bar, mirsphere, G4Transform3D(rotmir, transmir));


      /*!  Position mirror  */

      G4LogicalVolume* mirror = new G4LogicalVolume(mirrorshape, quartzMaterial, "mirror");

      GearDir surface(content, "Mirror/Surface");
      if (surface) {
        Materials& materials = Materials::getInstance();
        G4OpticalSurface* optSurf = materials.createOpticalSurface(surface);
        new G4LogicalSkinSurface("mirrorsSurface", mirror, optSurf);
      } else B2WARNING("TOP mirrors surface has no specified optical properties. No photons will be reflected.");

      G4Transform3D dumb = G4Translate3D(0, 0, 0);
      new G4PVPlacement(dumb, mirror, "TOP.mirror", qbar, false, 1);


      /*!  Build glue joints and assign material  */


      G4Box* box2 = new G4Box("glue_2", Gwidth2 / 2.0, Qthickness / 2.0, Qwidth / 2.0);
      G4Box* box3 = new G4Box("glue_3", Gwidth3 / 2.0, Qthickness / 2.0, Qwidth / 2.0);

      G4LogicalVolume* gbox2 = new G4LogicalVolume(box2, glueMaterial, "gbox2");
      G4LogicalVolume* gbox3 = new G4LogicalVolume(box3, glueMaterial, "gbox3");
      gbox2->SetSensitiveDetector(m_sensitiveTrack);
      gbox2->SetSensitiveDetector(m_sensitiveTrack);


      /*!  Place glue joints */

      G4Transform3D tglue2 = G4Translate3D(-length / 2.0 + Length1 + Gwidth2 / 2.0 , 0, 0);
      G4Transform3D tglue3 = G4Translate3D(-length / 2.0 + Length1 + Gwidth2 + Length2 + Gwidth3 / 2.0 , 0, 0);


      new G4PVPlacement(tglue2, gbox2, "TOP.gbox2", qbar, false, 1);
      new G4PVPlacement(tglue3, gbox3, "TOP.gbox3", qbar, false, 1);



      /*!  Build wedge  */


      std::vector<G4TwoVector> polygon;

      polygon.push_back(G4TwoVector(-length / 2.0, Qthickness / 2.0));
      polygon.push_back(G4TwoVector(-length / 2.0, -Qthickness / 2.0));
      polygon.push_back(G4TwoVector(-length / 2.0 - Gwidth1, -Qthickness / 2.0));
      polygon.push_back(G4TwoVector(-length / 2.0 - Gwidth1 - WLength, -Qthickness / 2.0 - Wextdown));
      polygon.push_back(G4TwoVector(-length / 2.0 - Gwidth1 - WLength, Qthickness / 2.0));
      polygon.push_back(G4TwoVector(-length / 2.0 - Gwidth1, Qthickness / 2.0));


      G4ExtrudedSolid* wedge = new  G4ExtrudedSolid("wedge", polygon, Wwidth / 2.0, G4TwoVector(0.0, 0.0), 1.0, G4TwoVector(0.0, 0.0), 1.0);

      G4LogicalVolume* qwedge = new G4LogicalVolume(wedge, quartzMaterial, "qwedge");

      /*! Add glue to wedge */


      G4Box* box1 = new G4Box("glue_1", Gwidth3 / 2.0, Qthickness / 2.0, Wwidth / 2.0);
      G4LogicalVolume* gbox1 = new G4LogicalVolume(box1, glueMaterial, "gbox1");
      gbox1->SetSensitiveDetector(m_sensitiveTrack);

      G4Transform3D tglue1 = G4Translate3D(-length / 2.0 - Gwidth1 / 2.0 , 0, 0);

      new G4PVPlacement(tglue1, gbox1, "TOP.gbox1", qwedge, false, 1);


      /*! Putt all segment into the Assembly volume */

      //            G4RotationMatrix* rotasem = new G4RotationMatrix(M_PI/2.0, M_PI/2.0,0);
      G4RotationMatrix* rotasem = new G4RotationMatrix(0, 0, 0);
      G4ThreeVector trnsasem(length / 2.0 + Bposition, 0, 0);

      G4AssemblyVolume* assemblyDetector = new G4AssemblyVolume();
      assemblyDetector->AddPlacedVolume(qbar, trnsasem, rotasem);
      assemblyDetector->AddPlacedVolume(qwedge, trnsasem, rotasem);

      G4LogicalVolume* stack = buildPMTstack(content);

      G4double dz = Bposition - WLength - Gwidth1 - (m_topgp->getdGlue() + m_topgp->getWinthickness() + m_topgp->getMsizez() + m_topgp->getBotthickness()) / 2.0;
      G4double dx = (-Wextdown) / 2.0;

      //            G4RotationMatrix* rotsta = new G4RotationMatrix(0,0,-M_PI/2.0);
      G4RotationMatrix* rotsta = new G4RotationMatrix(M_PI / 2.0, -M_PI / 2.0, -M_PI / 2.0);
      G4ThreeVector trnssta(dz, dx, 0);


      assemblyDetector->AddPlacedVolume(stack, trnssta, rotsta);


      return assemblyDetector;



    }



    G4LogicalVolume* GeoTOPCreator::buildSupport(const GearDir& content)
    {

      /*!  Read parameters  */

      //! get width of the quartz bar
      G4double Qwidth = m_topgp->getQwidth();
      //! get thickness of the quartz bar
      G4double Qthickness = m_topgp->getQthickness();
      //! get Backward position of qurtz bar
      G4double Bposition = m_topgp->getBposition();
      //! get length of first quartz bar segment which at one side is positioned at Bposition
      G4double Length1 = m_topgp->getLength1();
      //! get length of second quartz bar segment which at one side is positioned at Bposition+Length1
      G4double Length2 = m_topgp->getLength2();
      //! get length segment to which the mirror is attached
      G4double Length3 = m_topgp->getLength3();
      //! get length wedge segment
      G4double WLength = m_topgp->getWLength();
      //! get wedge extension down
      G4double Wextdown = m_topgp->getWextdown();
      //! get width of glue between wedge and segment 1
      G4double Gwidth1 = m_topgp->getGwidth1();
      //! get width of glue between segment 1 segmen 2
      G4double Gwidth2 = m_topgp->getGwidth2();
      //! get width of glue between segment 3 and mirror segment
      G4double Gwidth3 = m_topgp->getGwidth3();

      //!calculate the total length and width of the qurtz bar

      G4double length = Length1 + Length2 + Length3 + Gwidth2 + Gwidth3;

      G4double lengthw = WLength + Gwidth1;

      //! get module wall size in Z
      G4double MWsizez = m_topgp->getMsizez();
      //! get window thickness
      G4double Winthickness = m_topgp->getWinthickness();
      //! get window thickness
      G4double Botthickness = m_topgp->getBotthickness();

      //! get thickness of glue between PMTs and wedge
      G4double dGlue = m_topgp->getdGlue();

      G4double dz = Winthickness + MWsizez + Botthickness + dGlue;

      /*!  Build wedge  */

      G4double dx = 1;

      std::vector<G4TwoVector> polygon;

      polygon.push_back(G4TwoVector(Bposition - Gwidth1, Qthickness / 2.0 + dx));
      polygon.push_back(G4TwoVector(Bposition + length + dx, Qthickness / 2.0 + dx));
      polygon.push_back(G4TwoVector(Bposition + length + dx, -Qthickness / 2.0 - dx));
      polygon.push_back(G4TwoVector(Bposition, -Qthickness / 2.0 - dx));
      polygon.push_back(G4TwoVector(Bposition - Gwidth1, -Qthickness / 2.0 - dx));
      polygon.push_back(G4TwoVector(Bposition - lengthw, -Qthickness / 2.0 - Wextdown - 3 * dx));
      polygon.push_back(G4TwoVector(Bposition - lengthw - dz - dx, -Qthickness / 2.0 - Wextdown - 3 * dx));
      polygon.push_back(G4TwoVector(Bposition - lengthw - dz - dx, Qthickness / 2.0 + 5 * dx));
      polygon.push_back(G4TwoVector(Bposition - lengthw, Qthickness / 2.0 + 3 * dx));

      G4ExtrudedSolid* airshape = new  G4ExtrudedSolid("Air", polygon, Qwidth / 2.0 + dx / 2.0, G4TwoVector(0.0, 0.0), 1.0, G4TwoVector(0.0, 0.0), 1.0);


      GearDir materialNames(content, "Detector/Module");
      string fillName = materialNames.getString("fillMaterial", "TOPAir");
      G4Material* fillMaterial = Materials::get(fillName);
      if (!fillMaterial) { B2FATAL("Material '" << fillName << "' missing!");}

      G4LogicalVolume* air = new G4LogicalVolume(airshape, fillMaterial, "Air");

      return air;



    }



    G4LogicalVolume* GeoTOPCreator::buildPMTstack(const GearDir& content)
    {

      /*! Get PMT dimensions */

      //! get module side X
      G4double dx = m_topgp->getMsizex();
      //! get module side Y
      G4double dy = m_topgp->getMsizey();
      //! get module wall size in Z
      G4double MWsizez = m_topgp->getMsizez();
      //! get window thickness
      G4double Winthickness = m_topgp->getWinthickness();
      //! get window thickness
      G4double Botthickness = m_topgp->getBotthickness();

      //! get gap between PMTs in x direction
      G4double Xgap = m_topgp->getXgap();
      //! get gap between PMTs in x direction
      G4double Ygap = m_topgp->getYgap();
      //! get number of PMTs in one row
      G4int  Npmtx = m_topgp->getNpmtx();
      //! get number of PMTs in one column
      G4int Npmty = m_topgp->getNpmty();
      //! get thickness of glue between PMTs and wedge
      G4double dGlue = m_topgp->getdGlue();

      G4double dz = Winthickness + MWsizez + Botthickness;

      G4double Qthickness = m_topgp->getQthickness();
      G4double Wwidth = m_topgp->getWwidth();
      G4double Wextdown = m_topgp->getWextdown();


      GearDir materialNames(content, "Detector/Module");
      string fillName = materialNames.getString("fillMaterial", "TOPAir");
      G4Material* fillMaterial = Materials::get(fillName);
      if (!fillMaterial) { B2FATAL("Material '" << fillName << "' missing!");}


      /*! Build stack */

      G4double x = Wwidth;
      G4double y = Qthickness + Wextdown;

      G4Box* box = new G4Box("box", x / 2.0, y / 2.0, (dz + dGlue) / 2.0);
      G4LogicalVolume* stack = new G4LogicalVolume(box, fillMaterial, "stack");


      G4LogicalVolume* PMT = buildPMT(content);

      for (G4int ix = 0; ix < Npmtx; ix++) {
        for (G4int iy = 0; iy < Npmty; iy++) {

          G4Transform3D tpmt = G4Translate3D(-x / 2.0 + dx / 2.0 + (dx + Xgap) * ix , -y / 2.0 + dy / 2.0 + (dy + Ygap) * iy, -dGlue / 2.0);

          new G4PVPlacement(tpmt, PMT, "TOP.window", stack, false, (ix + 1) + (iy * Npmtx));
        }
      }



      return stack;

    }




    G4LogicalVolume* GeoTOPCreator::buildPMT(const GearDir& content)
    {

      /*! Get PMT dimensions */

      //! get module side X
      G4double Msizex = m_topgp->getMsizex();
      //! get module side Y
      G4double Msizey = m_topgp->getMsizey();
      //! get module wall size in Z
      G4double MWsizez = m_topgp->getMsizez();
      //! get active area size in X
      G4double Asizex = m_topgp->getAsizex();
      //! get active area size in y
      G4double Asizey = m_topgp->getAsizey();
      //! get active area size in z
      G4double Asizez = m_topgp->getAsizez();
      //! get window thickness
      G4double Winthickness = m_topgp->getWinthickness();
      //! get window thickness
      G4double Botthickness = m_topgp->getBotthickness();


      /*! Get PMT material */

      GearDir materialNames(content, "Detector/Module");
      string caseName = materialNames.getString("wallMaterial", "TOPPMTCase");
      string sensName = materialNames.getString("sensMaterial", "TOPPMTBialkali");
      string winName = materialNames.getString("winMaterial", "TOPBorosilicateGlass");
      string botName = materialNames.getString("botMaterial", "TOPPMTBottom");
      string fillName = materialNames.getString("fillMaterial", "TOPAir");


      G4Material* caseMaterial = Materials::get(caseName);
      G4Material* sensMaterial = Materials::get(sensName);
      G4Material* winMaterial = Materials::get(winName);
      G4Material* botMaterial = Materials::get(botName);
      G4Material* fillMaterial = Materials::get(fillName);


      if (!caseMaterial) { B2FATAL("Material '" << caseName << "' missing!");}
      if (!sensMaterial) { B2FATAL("Material '" << sensName << "' missing!");}
      if (!winMaterial) { B2FATAL("Material '" << winName << "' missing!");}
      if (!botMaterial) { B2FATAL("Material '" << botName << "' missing!");}
      if (!fillMaterial) { B2FATAL("Material '" << fillName << "' missing!");}


      /*! Build PMT segments */
      G4Box* box = new G4Box("box", Msizex / 2., Msizey / 2., (MWsizez + Winthickness + Botthickness) / 2.);
      G4LogicalVolume* PMT = new G4LogicalVolume(box, fillMaterial, "PMT");

      G4Box* box1 = new G4Box("window", Msizex / 2., Msizey / 2., Winthickness / 2.);
      G4LogicalVolume* window = new G4LogicalVolume(box1, winMaterial, "window");



      G4Box* box4 = new G4Box("bottom", Msizex / 2., Msizey / 2., Botthickness / 2.);
      G4LogicalVolume* bottom = new G4LogicalVolume(box4, botMaterial, "bottom");
      if (isBeamBkgStudy) bottom->SetSensitiveDetector(new BkgSensitiveDetector("TOP"));

      G4Box* box2 = new G4Box("box2", Msizex / 2., Msizey / 2., MWsizez  / 2.);
      G4Box* box3 = new G4Box("box3", Asizex / 2., Asizey / 2., MWsizez  / 2. + 0.1);
      G4SubtractionSolid* wallbox = new G4SubtractionSolid("box2-box3", box2, box3, G4Transform3D());
      G4LogicalVolume* wall = new G4LogicalVolume(wallbox, caseMaterial, "wall");

      G4Box* sensBox = new G4Box("sensbox", Asizex / 2., Asizey / 2., Asizez  / 2.);
      G4LogicalVolume* lmoduleSens = new G4LogicalVolume(sensBox, sensMaterial, "moduleSensitive");
      lmoduleSens->SetSensitiveDetector(m_sensitivePMT);

      /*! Place PMT segments */

      G4Transform3D twin = G4Translate3D(0 , 0, (MWsizez + Botthickness) / 2.);
      G4Transform3D tbot = G4Translate3D(0 , 0, -(MWsizez + Winthickness) / 2.);
      G4Transform3D twall = G4Translate3D(0 , 0, -(Winthickness - Botthickness) / 2.);
      G4Transform3D tsens = G4Translate3D(0 , 0, (MWsizez - Winthickness + Botthickness) / 2. - (Asizez / 2.));

      setColor(*window, "rgb(0.776,0.886,1.0,1.0)");
      new G4PVPlacement(twin, window, "TOP.window", PMT, false, 1);

      setColor(*bottom, "rgb(1.0,1.0,1.0,1.0)");
      new G4PVPlacement(tbot, bottom, "TOP.bottom", PMT, false, 1);

      setColor(*wall, "rgb(0.47,0.47,0.47,1.0)");
      new G4PVPlacement(twall, wall, "TOP.wall", PMT, false, 1);

      setColor(*lmoduleSens, "rgb(0.0,0.0,0.0,1.0)");
      new G4PVPlacement(tsens, lmoduleSens, "TOP.moduleSensitive", PMT, false, 1);


      return PMT;
    }

  }
}
