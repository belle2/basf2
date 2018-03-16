/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <sstream>
#include <cmath>
#include <string.h>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

//#include <geant4/G4LogicalVolume.hh>

// Geant4
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4AssemblyVolume.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4OpticalSurface.hh>
// Geant4 Shapes
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polyhedra.hh>
#include <G4SubtractionSolid.hh>
#include <G4Material.hh>

#include <Python.h>

#include <arich/geometry/GeoARICHBtestCreator.h>
#include <arich/geometry/ARICHGeometryPar.h>
#include <arich/geometry/ARICHBtestGeometryPar.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
// Framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>

#include <arich/simulation/SensitiveDetector.h>
#include <arich/simulation/SensitiveAero.h>

using namespace std;
using namespace boost;

namespace Belle2 {

  using namespace geometry;

  namespace arich {

    //-----------------------------------------------------------------
    //                 Register the Creator
    //-----------------------------------------------------------------

    geometry::CreatorFactory<GeoARICHBtestCreator> GeoARICHBtestFactory("ARICHBtestCreator");

    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoARICHBtestCreator::GeoARICHBtestCreator():
      m_sensitive(new SensitiveDetector),
      m_sensitiveAero(new SensitiveAero),
      m_runno(0),
      m_neve(0),
      m_rotation(0),
      m_rx(0),
      m_ry(0),
      m_aerosupport(0),
      m_aerogeldx(0),
      m_framedx(0),
      m_rotation1(0),
      m_configuration(0)
    {};

    GeoARICHBtestCreator::~GeoARICHBtestCreator()
    {

    }


    void GeoARICHBtestCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes)
    {

      B2INFO("GeoARICHBtestCreator::create");
      StoreObjPtr<EventMetaData> eventMetaDataPtr;

      int run = 68;
      PyObject* m = PyImport_AddModule(strdup("__main__"));
      if (m) {
        PyObject* v = PyObject_GetAttrString(m, strdup("runno"));
        if (v) {
          run = PyLong_AsLong(v);
          Py_DECREF(v);
        }
        B2INFO("GeoARICHBtestCreator::create runno = " << run);
      }

      B2INFO("eventMetaDataPtr run:" << run);
      // eventMetaDataPtr->setEndOfData();




      string Type = content.getString("@type", "");

      char nodestr[100];
      sprintf(nodestr, "run[runno=%d]", run);
      if (Type == "beamtest") {
        BOOST_FOREACH(const GearDir & runparam, content.getNodes(nodestr)) {
          m_runno       = runparam.getInt("runno", -1);
          m_author      = runparam.getString("author", "");
          m_neve        = runparam.getInt("neve", -1);
          m_runtype     = runparam.getString("calibration", "pion");
          m_hapdID      = runparam.getString("setup1", "unknown");
          m_aerogelID   = runparam.getString("aerogel1", "unknown");
          m_mirrorID    = runparam.getString("mirror", "unknown");
          m_rotation    = runparam.getDouble("rotation", 0);
          m_rx          = runparam.getDouble("positionx", 0);
          m_ry          = runparam.getDouble("positiony", 0);
          m_mytype      = runparam.getString("type1", "unknown");
          m_daqqa       = runparam.getString("daqqa1", "unknown");
          m_comment     = runparam.getString("comment1", "unknown");
          m_datum       = runparam.getString("datum", "unknown");


          B2INFO("runno    : " << m_runno);
          B2INFO("author   : " << m_author);
          B2INFO("neve     : " << m_neve);
          B2INFO("runtype  : " << m_runtype);
          B2INFO("hapdID   : " << m_hapdID);
          B2INFO("aerogelID: " << m_aerogelID);
          B2INFO("mirrorID : " << m_mirrorID);
          B2INFO("rotation : " << m_rotation);
          B2INFO("rx       : " << m_rx);
          B2INFO("ry       : " << m_ry);
          B2INFO("runtype  : " << m_mytype);
          B2INFO("daqqa    : " << m_daqqa);
          B2INFO("comment  : " << m_comment);
          B2INFO("datum    : " << m_datum);


        }
        string aerogelname;
        sprintf(nodestr, "setup/aerogel/row[@id=\"%s\"]", m_aerogelID.c_str());

        GearDir runparam(content, nodestr);
        B2INFO("id    : " << runparam.getString("@id", ""));
        BOOST_FOREACH(const GearDir & aeroparam, runparam.getNodes("aerogel")) {
          aerogelname       = aeroparam.getString(".", "");
          string stype       = aeroparam.getString("@type", "");
          B2INFO(stype << " aerogelname    : " << aerogelname);
          sprintf(nodestr, "setup/aerogelinfo/row[@id=\"%s\"]", aerogelname.c_str());
          GearDir infoparam(content, nodestr);

          double agelrefind    =  infoparam.getDouble("refind", 1);
          double ageltrlen     =  infoparam.getLength("trlen", 0);
          double agelthickness =  infoparam.getLength("thickness", 0);
          if (stype != string("left")) {
            m_ageltrlen.push_back(ageltrlen);
            m_agelrefind.push_back(agelrefind);
            m_agelthickness.push_back(agelthickness);
          }
          B2INFO("refind   : " <<  agelrefind);
          B2INFO("trlen    : " <<  ageltrlen / Unit::mm);
          B2INFO("thickness    : " << agelthickness / Unit::mm);

        }
        int size = m_hapdID.size();

        m_aerosupport = 0;
        if (size > 0) {
          char agelsupport = m_hapdID.at(size - 1);
          if (agelsupport == 'a') m_aerosupport = 1;
          if (agelsupport == 'b') m_aerosupport = 2;
        }

        if (m_aerosupport) size--;
        sprintf(nodestr, "setup/hapd/row[@id=\"%s\"]", m_hapdID.substr(0, size).c_str());
        B2INFO("nodestr    : " << nodestr);
        B2INFO("aerogelsupport    : " << m_aerosupport);
        GearDir hapdparam(content, nodestr);
        //BOOST_FOREACH(const GearDir & runparam, content.getNodes(nodestr)) {
        m_aerogeldx     =  hapdparam.getLength("aerogeldx", 0);
        m_framedx       =  hapdparam.getLength("framedx", 0) * CLHEP::mm / Unit::mm ;
        m_rotation1     =  hapdparam.getDouble("rotation", 0);
        m_configuration =  hapdparam.getInt("setup", 0);
        m_comment1      =  hapdparam.getString("comment", "");


        B2INFO("aerogeldx  : " <<  m_aerogeldx);
        B2INFO("framedx  : "   <<  m_framedx);
        B2INFO("rotation  : "  <<  m_rotation1);
        B2INFO("configuration  : "  <<  m_configuration);
        B2INFO("comment  : "  << m_comment);
        //}

        GearDir setup(content, "setup");

        createBtestGeometry(setup, topVolume);
      }
    }

    double GeoARICHBtestCreator::getAvgRINDEX(G4Material* material)
    {
      G4MaterialPropertiesTable* mTable = material->GetMaterialPropertiesTable();
      if (!mTable) return 0;
      G4MaterialPropertyVector* mVector =  mTable->GetProperty("RINDEX");
      if (!mVector) return 0;
      G4bool b;
      return mVector->GetValue(2 * Unit::eV / Unit::MeV, b);
    }

    G4LogicalVolume* GeoARICHBtestCreator::buildModule(GearDir Module)
    {

      // get detector module parameters

      // get module materials
      string wallMat =  Module.getString("wallMaterial");
      string winMat =  Module.getString("windowMaterial");
      string botMat =  Module.getString("Bottom/material");
      G4Material* wallMaterial = Materials::get(wallMat);
      G4Material* windowMaterial = Materials::get(winMat);
      G4Material* bottomMaterial = Materials::get(botMat);
      G4Material* boxFill = Materials::get("ARICH_Vacuum");

      // check that module window material has specified refractive index
      double wref = getAvgRINDEX(windowMaterial);
      if (!wref) B2WARNING("Material '" << winMat <<
                             "', required for ARICH photon detector window as no specified refractive index. Continuing, but no photons in ARICH will be detected.");
      ARICHGeometryPar* m_arichgp = ARICHGeometryPar::Instance();
      m_arichgp->setWindowRefIndex(wref);
      // get module dimensions
      double modXsize = Module.getLength("moduleXSize") / Unit::mm;
      double modZsize = Module.getLength("moduleZSize") / Unit::mm;
      double wallThick = Module.getLength("moduleWallThickness") / Unit::mm;
      double winThick = Module.getLength("windowThickness") / Unit::mm ;
      double sensXsize = m_arichgp->getSensitiveSurfaceSize() / Unit::mm;
      double botThick =  Module.getLength("Bottom/thickness") / Unit::mm;

      // some trivial checks of overlaps
      if (sensXsize > modXsize - 2 * wallThick)
        B2FATAL("ARICH photon detector module: Sensitive surface is too big. Doesn't fit into module box.");
      if (winThick + botThick > modZsize)
        B2FATAL("ARICH photon detector module: window + bottom thickness larger than module thickness.");

      // module master volume
      G4Box* moduleBox = new G4Box("Box", modXsize / 2., modXsize / 2., modZsize / 2.);
      G4LogicalVolume* lmoduleBox = new G4LogicalVolume(moduleBox, boxFill, "moduleBox");

      // build and place module wall
      G4Box* tempBox = new G4Box("tempBox", modXsize / 2. - wallThick, modXsize / 2. - wallThick,
                                 modZsize / 2. + 0.1); // Dont't care about "+0.1", needs to be there.
      G4SubtractionSolid* moduleWall = new G4SubtractionSolid("Box-tempBox", moduleBox, tempBox);
      G4LogicalVolume* lmoduleWall = new G4LogicalVolume(moduleWall, wallMaterial, "moduleWall");
      setColor(*lmoduleWall, "rgb(1.0,0.0,0.0,1.0)");
      new G4PVPlacement(G4Transform3D(), lmoduleWall, "moduleWall", lmoduleBox, false, 1);

      // build module window
      G4Box* winBox = new G4Box("winBox", modXsize / 2. - wallThick, modXsize / 2. - wallThick, winThick / 2.);
      G4LogicalVolume* lmoduleWin = new G4LogicalVolume(winBox, windowMaterial, "moduleWindow");
      setColor(*lmoduleWin, "rgb(0.7,0.7,0.7,1.0)");
      G4Transform3D transform = G4Translate3D(0., 0., (-modZsize + winThick) / 2.);
      new G4PVPlacement(transform, lmoduleWin, "moduleWindow", lmoduleBox, false, 1);

      // build module bottom
      G4Box* botBox = new G4Box("botBox", modXsize / 2. - wallThick, modXsize / 2. - wallThick, botThick / 2.);
      G4LogicalVolume* lmoduleBot = new G4LogicalVolume(botBox, bottomMaterial, "moduleBottom");
      // if (isBeamBkgStudy) lmoduleBot->SetSensitiveDetector(new BkgSensitiveDetector("ARICH", 1));
      setColor(*lmoduleBot, "rgb(0.0,1.0,0.0,1.0)");
      G4Transform3D transform1 = G4Translate3D(0., 0., (modZsize - botThick) / 2.);
      // add surface optical properties if specified
      Materials& materials = Materials::getInstance();
      GearDir bottomParam(Module, "Bottom/Surface");
      if (bottomParam) {
        G4OpticalSurface* optSurf = materials.createOpticalSurface(bottomParam);
        new G4LogicalSkinSurface("bottomSurface", lmoduleBot, optSurf);
      } else B2INFO("ARICH: No optical properties are specified for detector module bottom surface.");
      new G4PVPlacement(transform1, lmoduleBot, "moduleBottom", lmoduleBox, false, 1);

      // build sensitive surface
      G4Box* sensBox = new G4Box("sensBox", sensXsize / 2., sensXsize / 2., 0.1 * Unit::mm);
      G4LogicalVolume* lmoduleSens = new G4LogicalVolume(sensBox, boxFill, "moduleSensitive");
      lmoduleSens->SetSensitiveDetector(m_sensitive);
      setColor(*lmoduleSens, "rgb(0.5,0.5,0.5,1.0)");
      G4Transform3D transform2 = G4Translate3D(0., 0., (-modZsize + 0.1) / 2. + winThick);
      new G4PVPlacement(transform2, lmoduleSens, "moduleSensitive", lmoduleBox, false, 1);

      // module is build, return module logical volume
      return lmoduleBox;
    }


    G4Material*  GeoARICHBtestCreator::createAerogel(const char* aeroname, double RefractiveIndex, double AerogelTransmissionLength)
    {


      G4double   density = (RefractiveIndex - 1) / 0.21 * CLHEP::g / CLHEP::cm3;
      B2INFO("Creating ARICH " << aeroname << " n=" << RefractiveIndex << " density=" << density / CLHEP::g * CLHEP::cm3 << " g/cm3");
      Materials& materials = Materials::getInstance();
      G4Material* _aerogel = new G4Material(aeroname, density, 4);
      _aerogel->AddElement(materials.getElement("O")  ,   0.665);
      _aerogel->AddElement(materials.getElement("H")  ,   0.042);
      _aerogel->AddElement(materials.getElement("Si") ,   0.292);
      _aerogel->AddElement(materials.getElement("C")  ,   0.001);


      const G4double AerogelAbsorbtionLength = 1000 * Unit::mm;

      const G4int NBins = 40;
      G4double MomentumBins[NBins];

      G4double  AerogelRindex[NBins];
      G4double  AerogelAbsorption[NBins];
      G4double  AerogelRayleigh[NBins];

      G4double MaxPhotonEnergy = 5 * CLHEP::eV;
      G4double MinPhotonEnergy = 1.5 * CLHEP::eV;

      for (G4int i = 0; i < NBins; i++) {

        const G4double energy = float(i) / NBins * (MaxPhotonEnergy - MinPhotonEnergy) + MinPhotonEnergy;

        MomentumBins[i]  =  energy;
        AerogelRindex[i]    = RefractiveIndex;
        AerogelAbsorption[i] = AerogelAbsorbtionLength;

        const G4double Lambda0 = 400 * 1e-6 * CLHEP::mm;
        const G4double Lambda  = 1240 * CLHEP::eV / energy * 1e-6 * CLHEP::mm;
        G4double x = Lambda / Lambda0;
        AerogelRayleigh[i]  = AerogelTransmissionLength * x * x * x * x;
      }


      G4MaterialPropertiesTable* AeroProperty = new G4MaterialPropertiesTable();
      AeroProperty->AddProperty("RINDEX"   , MomentumBins, AerogelRindex    , NBins);
      AeroProperty->AddProperty("ABSLENGTH", MomentumBins, AerogelAbsorption, NBins);
      AeroProperty->AddProperty("RAYLEIGH" , MomentumBins, AerogelRayleigh, NBins);


      _aerogel->SetMaterialPropertiesTable(AeroProperty);


      return _aerogel;
    }


    void GeoARICHBtestCreator::createBtestGeometry(const GearDir& content, G4LogicalVolume& topWorld)
    {

      B2INFO("ARICH Btest geometry will be built.");
      ARICHGeometryPar* m_arichgp = ARICHGeometryPar::Instance();

      ARICHBtestGeometryPar* m_arichbtgp = ARICHBtestGeometryPar::Instance();

      // experimental box

      GearDir boxParams(content, "ExperimentalBox");
      double xBox = boxParams.getLength("xSize") * CLHEP::mm / Unit::mm;
      double yBox = boxParams.getLength("ySize") * CLHEP::mm / Unit::mm;
      double zBox = boxParams.getLength("zSize") * CLHEP::mm / Unit::mm;

      double xoffset = boxParams.getLength("beamcenter/x")  * CLHEP::mm  / Unit::mm;
      double yoffset = boxParams.getLength("beamcenter/y")  * CLHEP::mm  / Unit::mm;
      double zoffset = boxParams.getLength("beamcenter/z")  * CLHEP::mm  / Unit::mm - zBox / 2.;
      G4ThreeVector roffset(xoffset, yoffset, zoffset);

      TVector3 sh(boxParams.getLength("beamcenter/x"), boxParams.getLength("beamcenter/y"),
                  boxParams.getLength("beamcenter/z") - boxParams.getLength("zSize") / 2.);
      m_arichbtgp->setOffset(sh);

      string boxMat = boxParams.getString("material");
      G4Material* boxMaterial = Materials::get(boxMat);
      G4Box* expBox = new G4Box("ExperimentalBox", xBox / 2., yBox / 2., zBox / 2.);
      G4LogicalVolume* topVolume = new G4LogicalVolume(expBox, boxMaterial, "ARICH.experimentalbox");
      new G4PVPlacement(G4Transform3D(), topVolume, "ARICH.experimentalbox", &topWorld, false, 1);
      setVisibility(*topVolume, false);

      TVector3 trackingshift(content.getLength("tracking/shift/x"),
                             content.getLength("tracking/shift/y"),
                             content.getLength("tracking/shift/z"));

      char mnodestr[256];
      sprintf(mnodestr, "tracking/shift/run[@id=\"%d\"]", m_runno);
      if (content.exists(mnodestr)) {
        GearDir runtrackingshift(content, mnodestr);
        trackingshift[0] = runtrackingshift.getLength("x");
        trackingshift[1] = runtrackingshift.getLength("y");
        trackingshift[2] = runtrackingshift.getLength("z");
      }
      m_arichbtgp->setTrackingShift(trackingshift);
      ARICHTracking* m_mwpc = new ARICHTracking[4];
      m_arichbtgp->setMwpc(m_mwpc);
      BOOST_FOREACH(const GearDir & mwpc, content.getNodes("tracking/mwpc")) {
        double x = mwpc.getLength("size/x")  * CLHEP::mm / Unit::mm;
        double y = mwpc.getLength("size/y")  * CLHEP::mm / Unit::mm;
        double z = mwpc.getLength("size/z")  * CLHEP::mm / Unit::mm;

        double px = mwpc.getLength("position/x")  * CLHEP::mm / Unit::mm;
        double py = mwpc.getLength("position/y")  * CLHEP::mm / Unit::mm;
        double pz = mwpc.getLength("position/z")  * CLHEP::mm / Unit::mm;

        G4Box* mwpcBox = new G4Box("MwpcBox", x / 2., y / 2., z / 2.);
        G4LogicalVolume* mwpcVol = new G4LogicalVolume(mwpcBox, Materials::get(mwpc.getString("material"))  , "ARICH.mwpc");
        new G4PVPlacement(G4Transform3D(G4RotationMatrix(), G4ThreeVector(px, py, pz) + roffset), mwpcVol, "ARICH.mwpc", topVolume, false,
                          1);
        //setVisibility(*mwpc, true);

        int id = mwpc.getInt("@id", -1);
        B2INFO("GeoARICHBtestCreator:: MWPC ID=" << id);
        if (id < 4 && id >= 0) {
          m_mwpc[id].tdc[0]   = mwpc.getInt("tdc/y/up");
          m_mwpc[id].tdc[1]   = mwpc.getInt("tdc/y/down");
          m_mwpc[id].tdc[2]   = mwpc.getInt("tdc/x/left");
          m_mwpc[id].tdc[3]   = mwpc.getInt("tdc/x/right");
          m_mwpc[id].atdc     = mwpc.getInt("tdc/anode", 0);
          m_mwpc[id].slp[0]   = mwpc.getDouble("slope/x");
          m_mwpc[id].slp[1]   = mwpc.getDouble("slope/y");
          m_mwpc[id].offset[0] = mwpc.getDouble("offset/x");
          m_mwpc[id].offset[1] = mwpc.getDouble("offset/y");
          m_mwpc[id].cutll[0] = mwpc.getInt("tdccut/y/min");
          m_mwpc[id].cutll[1] = mwpc.getInt("tdccut/x/min");
          m_mwpc[id].cutul[0] = mwpc.getInt("tdccut/y/max");
          m_mwpc[id].cutul[1] = mwpc.getInt("tdccut/x/max");
          m_mwpc[id].pos[0] = mwpc.getDouble("position/x");
          m_mwpc[id].pos[1] = mwpc.getDouble("position/y");
          m_mwpc[id].pos[2] = mwpc.getDouble("position/z");
          // m_mwpc[id].Print();
        }

      }
      // physical position of the hapd channels

      istringstream mapstream;
      double mx, my;
      mapstream.str(content.getString("hapdmap"));
      while (mapstream >> mx >> my) {
        m_arichbtgp->AddHapdChannelPositionPair(mx, my);
      }
      mapstream.clear();

      // mapping of the electronic channels
      int ipx, ipy;
      mapstream.str(content.getString("hapdchmap"));
      while (mapstream >> ipx >> ipy) {
        m_arichbtgp->AddHapdElectronicMapPair(ipx, ipy);
      }
      // experimental frame consisting of detector plane, aerogel and mirrors

      GearDir frameParams(content, "Frame");
      double xFrame = frameParams.getLength("xSize")  * CLHEP::mm / Unit::mm;
      double yFrame = frameParams.getLength("ySize")  * CLHEP::mm / Unit::mm;
      double zFrame = frameParams.getLength("zSize")  * CLHEP::mm / Unit::mm;
      string envMat = frameParams.getString("material");

      double px = frameParams.getLength("position/x") * CLHEP::mm /  Unit::mm;
      double py = frameParams.getLength("position/y") * CLHEP::mm /  Unit::mm;
      double pz = frameParams.getLength("position/z") * CLHEP::mm /  Unit::mm;

      G4Material* envMaterial = Materials::get(envMat);


      G4Box* envBox = new G4Box("FrameBox", xFrame / 2., yFrame / 2., zFrame / 2.);
      G4LogicalVolume* lenvBox = new G4LogicalVolume(envBox, envMaterial, "ARICH.frame");
      G4ThreeVector    frameOrigin0(m_framedx + px, py, pz); // rotation point of the detector frame wrt beamcenter
      G4ThreeVector    frameOrigin = frameOrigin0 + roffset;
      G4RotationMatrix frameRotation;
      frameRotation.rotateY(-m_rotation1 * CLHEP::degree);
      G4Transform3D frameTransformation = G4Transform3D(frameRotation, frameOrigin);

      new G4PVPlacement(frameTransformation, lenvBox, "ARICH.frame", topVolume, false, 1);
      //setVisibility(*lenvBox, false);

      TVector3 rotationCenter =  TVector3(frameOrigin0.x() *  Unit::mm / CLHEP::mm, frameOrigin0.y() *  Unit::mm / CLHEP::mm,
                                          frameOrigin0.z() *  Unit::mm / CLHEP::mm);
      m_arichbtgp->setFrameRotation(m_rotation1 * CLHEP::degree);
      m_arichbtgp->setRotationCenter(rotationCenter);


      char nodestr[256];
      B2INFO(content.getPath());
      sprintf(nodestr, "PhotonDetector/setup[@id=\"%d\"]", m_configuration);
      GearDir hapdcontent(content, nodestr);
      B2INFO(hapdcontent.getPath());



      char mirrornodestr[256];
      sprintf(mirrornodestr, "Mirrors/setup[@id=\"%s\"]", m_mirrorID.c_str());

      GearDir mirrorcontent(content, mirrornodestr);
      B2INFO(mirrorcontent.getPath());

      // detectors
      m_arichgp->Initialize(hapdcontent, mirrorcontent);


      GearDir moduleParam(hapdcontent, "Detector/Module");
      G4LogicalVolume* detModule = buildModule(moduleParam);

      double detZpos = hapdcontent.getLength("Detector/Plane/zPosition") * CLHEP::mm / Unit::mm;
      double detThick = hapdcontent.getLength("Detector/Module/moduleZSize") * CLHEP::mm / Unit::mm;
      int nModules = m_arichgp->getNMCopies();

      for (int i = 1; i <= nModules; i++) {
        G4ThreeVector origin = m_arichgp->getOriginG4(i);
        origin.setZ(detZpos + detThick / 2.);
        double angle = m_arichgp->getModAngle(i);
        G4RotationMatrix Ra;
        Ra.rotateZ(angle);
        G4Transform3D trans = G4Transform3D(Ra, origin);
        new G4PVPlacement(G4Transform3D(Ra, origin), detModule, "detModule", lenvBox, false, i);
        B2INFO(nodestr << "Module " << i << " is build ");
      }
      // mask hot channels
      int npx       = m_arichgp->getDetectorXPadNumber();
      BOOST_FOREACH(const double & ch, hapdcontent.getArray("HotChannels")) {
        int channelID = (int) ch;
        int moduleID  = (npx) ? channelID / (npx * npx) : 0;
        channelID    %= (npx * npx);
        m_arichgp->setActive(moduleID, channelID, false);
        B2INFO("HotChannel " << ch << " : Module " << moduleID << "channelID " << channelID << " disabled");
      }
      // mask dead channels
      BOOST_FOREACH(const double & ch, hapdcontent.getArray("DeadChannels")) {
        int channelID = (int) ch;
        int moduleID  = (npx) ? channelID / (npx * npx) : 0;
        channelID    %= (npx * npx);
        m_arichgp->setActive(moduleID, channelID, false);
        B2INFO("DeadChannel " << ch << " : Module " << moduleID << "channelID " << channelID << " disabled");
      }
      // place aerogel tiles
      GearDir aerogelParam(content, "Aerogel");
      double sizeX = aerogelParam.getLength("tileXSize") * CLHEP::mm / Unit::mm;
      double sizeY = aerogelParam.getLength("tileYSize") * CLHEP::mm / Unit::mm;
      double posX  = aerogelParam.getLength("tileXPos")   * CLHEP::mm / Unit::mm;
      double posY  = aerogelParam.getLength("tileYPos")   * CLHEP::mm / Unit::mm;
      double posZ  = aerogelParam.getLength("tileZPos")   * CLHEP::mm / Unit::mm;
      double posZ0 = posZ;
      double meanrefind = 0;
      double meantrlen  = 0;

      // get parameter from python script
      PyObject* m = PyImport_AddModule(strdup("__main__"));
      if (m) {
        int averageagel = 0;
        PyObject* v = PyObject_GetAttrString(m, strdup("averageagel"));
        if (v) {
          averageagel = PyLong_AsLong(v);
          Py_DECREF(v);
        }
        B2INFO("Python averageagel = " << averageagel);
        m_arichbtgp->setAverageAgel(averageagel > 0);
      }

      for (unsigned int ilayer = 0; ilayer < m_agelthickness.size(); ilayer++) {
        char aeroname[100];
        sprintf(aeroname, "Aerogel%u", ilayer + 1);
        G4Material* tileMaterial = createAerogel(aeroname, m_agelrefind[ilayer],  m_ageltrlen[ilayer]);
        double sizeZ = m_agelthickness[ilayer] * CLHEP::mm / Unit::mm;

        if (!m_arichbtgp->getAverageAgel()) {
          m_arichgp->setAeroRefIndex(ilayer, m_agelrefind[ilayer]);
          m_arichgp->setAerogelZPosition(ilayer, (posZ - zFrame / 2.) * Unit::mm / CLHEP::mm);
          m_arichgp->setAerogelThickness(ilayer, sizeZ * Unit::mm / CLHEP::mm);
          m_arichgp->setAeroTransLength(ilayer, m_ageltrlen[ilayer]);
        }

        meantrlen  += sizeZ / m_ageltrlen[ilayer];
        meanrefind += m_agelrefind[ilayer];
        G4Box* tileBox = new G4Box("tileBox", sizeX / 2., sizeY / 2., sizeZ / 2.);
        G4LogicalVolume* lTile = new G4LogicalVolume(tileBox, tileMaterial, "Tile", 0, ilayer == 0 ? m_sensitiveAero : 0);
        setColor(*lTile, "rgb(0.0, 1.0, 1.0,1.0)");
        G4Transform3D trans = G4Translate3D(posX, posY, posZ + sizeZ / 2.  - zFrame / 2.);
        new G4PVPlacement(trans, lTile, "ARICH.tile", lenvBox, false, ilayer + 1);
        posZ += sizeZ;
      }
      if (m_arichbtgp->getAverageAgel() && m_agelthickness.size()) {
        B2INFO("Average aerogel will be used in the reconstruction ");
        m_arichgp->setAeroRefIndex(0, meanrefind / m_agelthickness.size());
        m_arichgp->setAerogelZPosition(0, (posZ0 - zFrame)* Unit::mm / CLHEP::mm);
        m_arichgp->setAerogelThickness(0, posZ  * Unit::mm / CLHEP::mm);
        if (meantrlen > 0 && posZ > 0) meantrlen = 1 / meantrlen / posZ;
        m_arichgp->setAeroTransLength(0, meantrlen);
      }


      // place mirrors
      GearDir mirrorsParam(mirrorcontent, "Mirrors");
      double height    = mirrorsParam.getLength("height") * CLHEP::mm / Unit::mm;
      double width     = mirrorsParam.getLength("width") * CLHEP::mm / Unit::mm;
      double thickness = mirrorsParam.getLength("thickness") * CLHEP::mm / Unit::mm;
      string mirrMat   = mirrorsParam.getString("material");
      G4Material* mirrMaterial = Materials::get(mirrMat);
      G4Box* mirrBox = new G4Box("mirrBox", thickness / 2., height / 2., width / 2.);
      G4LogicalVolume* lmirror = new G4LogicalVolume(mirrBox, mirrMaterial, "mirror");

      Materials& materials = Materials::getInstance();
      GearDir surface(mirrorsParam, "Surface");
      G4OpticalSurface* optSurf = materials.createOpticalSurface(surface);
      new G4LogicalSkinSurface("mirrorsSurface", lmirror, optSurf);
      int iMirror = 0;
      BOOST_FOREACH(const GearDir & mirror, mirrorsParam.getNodes("Mirror")) {
        double xpos = mirror.getLength("xPos") * CLHEP::mm / Unit::mm;
        double ypos = mirror.getLength("yPos") * CLHEP::mm / Unit::mm;
        double zpos = mirror.getLength("zPos") * CLHEP::mm / Unit::mm;
        double angle = mirror.getAngle("angle") / Unit::rad;
        G4ThreeVector origin(xpos, ypos, zpos + width / 2. - zFrame / 2.);
        G4RotationMatrix Ra;
        Ra.rotateZ(angle);
        G4Transform3D trans = G4Transform3D(Ra, origin);
        new G4PVPlacement(G4Transform3D(Ra, origin), lmirror, "ARICH.mirror", lenvBox, false, iMirror);
        iMirror++;
      }
      m_arichgp->Print();
      m_arichbtgp->Print();
    }


  }
}
