/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 * Major revision: 2016                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/geometry/TOPGeometryPar.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <geometry/Materials.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  namespace TOP {

    TOPGeometryPar* TOPGeometryPar::s_instance = 0;

    TOPGeometryPar::~TOPGeometryPar()
    {
      if (m_geo) delete m_geo;
      if (m_geoDB) delete m_geoDB;
      s_instance = 0;
    }


    TOPGeometryPar* TOPGeometryPar::Instance()
    {
      if (!s_instance) {
        s_instance = new TOPGeometryPar();
      }
      return s_instance;
    }


    void TOPGeometryPar::Initialize(const GearDir& content)
    {

      m_fromDB = false;
      m_valid = false;

      if (m_geo) delete m_geo;
      m_geo = createConfiguration(content);
      if (!m_geo->isConsistent()) {
        B2ERROR("TOPGeometryPar::createConfiguration: geometry not consistently defined");
        return;
      }

      readOldQBB(content);

      GearDir frontEndMapping(content, "FrontEndMapping");
      m_frontEndMapper.initialize(frontEndMapping);
      if (!m_frontEndMapper.isValid()) {
        return;
      }

      GearDir channelMapping0(content, "ChannelMapping[@type='IRS3B']");
      m_channelMapperIRS3B.initialize(channelMapping0);
      if (!m_channelMapperIRS3B.isValid()) {
        return;
      }

      GearDir channelMapping1(content, "ChannelMapping[@type='IRSX']");
      m_channelMapperIRSX.initialize(channelMapping1);
      if (!m_channelMapperIRSX.isValid()) {
        return;
      }

      m_valid = true;
    }


    void TOPGeometryPar::Initialize()
    {
      m_fromDB = true;
      m_valid = false;

      if (m_geoDB) delete m_geoDB;
      m_geoDB = new DBObjPtr<TOPGeometry>();

      if (!m_geoDB->isValid()) {
        B2ERROR("TOPGeometry: no payload found in database");
        return;
      }

      // Make sure that we abort as soon as the geometry changes
      m_geoDB->addCallback([]() {
        B2FATAL("Geometry cannot change during processing, "
                "aborting (component TOP)");
      });

      m_frontEndMapper.initialize();
      if (!m_frontEndMapper.isValid()) {
        B2ERROR("TOPFrontEndMaps: no payload found in database");
        return;
      }

      m_channelMapperIRSX.initialize();
      if (!m_channelMapperIRSX.isValid()) {
        B2ERROR("TOPChannelMaps: no payload found in database");
        return;
      }

      m_valid = true;
    }


    const TOPGeometry* TOPGeometryPar::getGeometry() const
    {
      if (!m_valid) B2FATAL("No geometry available for TOP");

      TOPGeometry::useBasf2Units();
      if (m_fromDB) {
        return &(**m_geoDB);
      } else {
        return m_geo;
      }
    }


    TOPGeometry* TOPGeometryPar::createConfiguration(const GearDir& content)
    {
      TOPGeometry* geo = new TOPGeometry("TOPGeometryIdealized");

      // PMT array

      GearDir pmtParams(content, "PMTs/Module");
      TOPGeoPMT pmt(pmtParams.getLength("ModuleXSize"),
                    pmtParams.getLength("ModuleYSize"),
                    pmtParams.getLength("ModuleZSize") +
                    pmtParams.getLength("WindowThickness") +
                    pmtParams.getLength("BottomThickness"));
      pmt.setWallThickness(pmtParams.getLength("ModuleWall"));
      pmt.setWallMaterial(pmtParams.getString("wallMaterial"));
      pmt.setFillMaterial(pmtParams.getString("fillMaterial"));
      pmt.setSensVolume(pmtParams.getLength("SensXSize"),
                        pmtParams.getLength("SensYSize"),
                        pmtParams.getLength("SensThickness"),
                        pmtParams.getString("sensMaterial"));
      pmt.setNumPixels(pmtParams.getInt("PadXNum"),
                       pmtParams.getInt("PadYNum"));
      pmt.setWindow(pmtParams.getLength("WindowThickness"),
                    pmtParams.getString("winMaterial"));
      pmt.setBottom(pmtParams.getLength("BottomThickness"),
                    pmtParams.getString("botMaterial"));

      auto& materials = geometry::Materials::getInstance();
      GearDir reflEdgeSurfParams(pmtParams, "reflectiveEdge/Surface");
      pmt.setReflEdge(pmtParams.getLength("reflectiveEdge/width"),
                      pmtParams.getLength("reflectiveEdge/thickness"),
                      materials.createOpticalSurfaceConfig(reflEdgeSurfParams));

      GearDir arrayParams(content, "PMTs");
      TOPGeoPMTArray pmtArray(arrayParams.getInt("nPMTx"),
                              arrayParams.getInt("nPMTy"),
                              arrayParams.getLength("Xgap"),
                              arrayParams.getLength("Ygap"),
                              arrayParams.getString("stackMaterial"),
                              pmt);
      geo->setPMTArray(pmtArray);

      // modules

      GearDir barParams(content, "Bars");
      GearDir barSurfParams(barParams, "Surface");
      auto barSurface = materials.createOpticalSurfaceConfig(barSurfParams);
      double sigmaAlpha = barSurfParams.getDouble("SigmaAlpha");

      TOPGeoBarSegment bar1(barParams.getLength("QWidth"),
                            barParams.getLength("QThickness"),
                            barParams.getLength("QBar1Length"),
                            barParams.getString("BarMaterial"));
      bar1.setGlue(barParams.getLength("Glue/Thicknes2"),
                   barParams.getString("Glue/GlueMaterial"));
      bar1.setSurface(barSurface, sigmaAlpha);
      bar1.setName(bar1.getName() + "1");

      TOPGeoBarSegment bar2(barParams.getLength("QWidth"),
                            barParams.getLength("QThickness"),
                            barParams.getLength("QBar2Length"),
                            barParams.getString("BarMaterial"));
      bar2.setGlue(barParams.getLength("Glue/Thicknes1"),
                   barParams.getString("Glue/GlueMaterial"));
      bar2.setSurface(barSurface, sigmaAlpha);
      bar2.setName(bar2.getName() + "2");

      TOPGeoMirrorSegment mirror(barParams.getLength("QWidth"),
                                 barParams.getLength("QThickness"),
                                 barParams.getLength("QBarMirror"),
                                 barParams.getString("BarMaterial"));
      mirror.setGlue(barParams.getLength("Glue/Thicknes3"),
                     barParams.getString("Glue/GlueMaterial"));
      mirror.setSurface(barSurface, sigmaAlpha);
      GearDir mirrorParams(content, "Mirror");
      mirror.setRadius(mirrorParams.getLength("Radius"));
      mirror.setCenterOfCurvature(mirrorParams.getLength("Xpos"),
                                  mirrorParams.getLength("Ypos"));
      GearDir mirrorSurfParams(mirrorParams, "Surface");
      mirror.setCoating(mirrorParams.getLength("mirrorThickness"),
                        mirrorParams.getString("Material"),
                        materials.createOpticalSurfaceConfig(mirrorSurfParams));

      TOPGeoPrism prism(barParams.getLength("QWedgeWidth"),
                        barParams.getLength("QThickness"),
                        barParams.getLength("QWedgeLength"),
                        barParams.getLength("QWedgeDown") +
                        barParams.getLength("QThickness"),
                        barParams.getLength("QWedgeFlat"),
                        barParams.getString("BarMaterial"));
      prism.setGlue(arrayParams.getLength("dGlue"),
                    arrayParams.getString("glueMaterial"));
      prism.setSurface(barSurface, sigmaAlpha);

      double R = barParams.getLength("Radius") + barParams.getLength("QThickness") / 2;
      double phi = barParams.getLength("Phi0");
      double backwardZ = barParams.getLength("QZBackward");
      int numModules = barParams.getInt("Nbar");
      for (int i = 0; i < numModules; i++) {
        unsigned id = i + 1;
        TOPGeoModule module(id, R, phi, backwardZ);
        module.setName(addNumber(module.getName(), id));
        module.setBarSegment1(bar1);
        module.setBarSegment2(bar2);
        module.setMirrorSegment(mirror);
        module.setPrism(prism);
        // module.setModuleCNumber(num);
        // module.setPMTArrayDisplacement(arrayDispl);
        // module.setModuleDisplacement(moduleDispl);
        geo->appendModule(module);
        phi += 2 * M_PI / numModules;
      }

      // front-end electronics geometry

      GearDir feParams(content, "FrontEndGeo");
      GearDir fbParams(feParams, "FrontBoard");
      TOPGeoBoardStack boardStack;
      boardStack.setFrontBoard(fbParams.getLength("width"),
                               fbParams.getLength("height"),
                               fbParams.getLength("thickness"),
                               fbParams.getLength("gap"),
                               fbParams.getLength("y"),
                               fbParams.getString("material"));
      GearDir bsParams(feParams, "BoardStack");
      boardStack.setBoardStack(bsParams.getLength("width"),
                               bsParams.getLength("height"),
                               bsParams.getLength("length"),
                               bsParams.getLength("gap"),
                               bsParams.getLength("y"),
                               bsParams.getString("material"),
                               bsParams.getLength("spacerWidth"),
                               bsParams.getString("spacerMaterial"));
      geo->setBoardStack(boardStack, feParams.getInt("numBoardStacks"));

      // QBB

      GearDir qbbParams(content, "QBB");
      TOPGeoQBB qbb(qbbParams.getLength("width"),
                    qbbParams.getLength("length"),
                    qbbParams.getLength("prismPosition"),
                    qbbParams.getString("material"));

      GearDir outerPanelParams(qbbParams, "outerPanel");
      TOPGeoHoneycombPanel outerPanel(outerPanelParams.getLength("width"),
                                      outerPanelParams.getLength("length"),
                                      outerPanelParams.getLength("minThickness"),
                                      outerPanelParams.getLength("maxThickness"),
                                      outerPanelParams.getLength("radius"),
                                      outerPanelParams.getLength("edgeWidth"),
                                      outerPanelParams.getLength("y"),
                                      outerPanelParams.getInt("N"),
                                      outerPanelParams.getString("material"),
                                      outerPanelParams.getString("edgeMaterial"),
                                      "TOPOuterHoneycombPanel");
      qbb.setOuterPanel(outerPanel);

      GearDir innerPanelParams(qbbParams, "innerPanel");
      TOPGeoHoneycombPanel innerPanel(innerPanelParams.getLength("width"),
                                      innerPanelParams.getLength("length"),
                                      innerPanelParams.getLength("minThickness"),
                                      innerPanelParams.getLength("maxThickness"),
                                      innerPanelParams.getLength("radius"),
                                      innerPanelParams.getLength("edgeWidth"),
                                      innerPanelParams.getLength("y"),
                                      innerPanelParams.getInt("N"),
                                      innerPanelParams.getString("material"),
                                      innerPanelParams.getString("edgeMaterial"),
                                      "TOPInnerHoneycombPanel");
      qbb.setInnerPanel(innerPanel);

      GearDir sideRailsParams(qbbParams, "sideRails");
      TOPGeoSideRails sideRails(sideRailsParams.getLength("thickness"),
                                sideRailsParams.getLength("reducedThickness"),
                                sideRailsParams.getLength("height"),
                                sideRailsParams.getString("material"));
      qbb.setSideRails(sideRails);

      GearDir prismEnclParams(qbbParams, "prismEnclosure");
      TOPGeoPrismEnclosure prismEncl(prismEnclParams.getLength("length"),
                                     prismEnclParams.getLength("height"),
                                     prismEnclParams.getAngle("angle"),
                                     prismEnclParams.getLength("bottomThickness"),
                                     prismEnclParams.getLength("sideThickness"),
                                     prismEnclParams.getLength("backThickness"),
                                     prismEnclParams.getLength("frontThickness"),
                                     prismEnclParams.getLength("extensionThickness"),
                                     prismEnclParams.getString("material"));
      qbb.setPrismEnclosure(prismEncl);

      GearDir endPlateParams(qbbParams, "forwardEndPlate");
      TOPGeoEndPlate endPlate(endPlateParams.getLength("thickness"),
                              endPlateParams.getLength("height"),
                              endPlateParams.getString("material"),
                              "TOPForwardEndPlate");
      qbb.setEndPlate(endPlate);

      GearDir coldPlateParams(qbbParams, "coldPlate");
      TOPGeoColdPlate coldPlate(coldPlateParams.getLength("baseThickness"),
                                coldPlateParams.getString("baseMaterial"),
                                coldPlateParams.getLength("coolThickness"),
                                coldPlateParams.getLength("coolWidth"),
                                coldPlateParams.getString("coolMaterial"));
      qbb.setColdPlate(coldPlate);

      geo->setQBB(qbb);

      // nominal QE

      GearDir qeParams(content, "QE");
      std::vector<float> qeData;
      for (const GearDir& Qeffi : qeParams.getNodes("Qeffi")) {
        qeData.push_back(Qeffi.getDouble(""));
      }
      TOPNominalQE nominalQE(qeParams.getLength("LambdaFirst") / Unit::nm,
                             qeParams.getLength("LambdaStep") / Unit::nm,
                             qeParams.getDouble("ColEffi"),
                             qeData);
      geo->setNominalQE(nominalQE);

      // nominal TTS

      GearDir ttsParams(content, "PMTs/TTS");
      TOPNominalTTS nominalTTS("TOPNominalTTS");
      for (const GearDir& Gauss : ttsParams.getNodes("Gauss")) {
        nominalTTS.appendGaussian(Gauss.getDouble("fraction"),
                                  Gauss.getTime("mean"),
                                  Gauss.getTime("sigma"));
      }
      nominalTTS.normalize();
      geo->setNominalTTS(nominalTTS);

      // nominal TDC

      GearDir tdcParams(content, "TDC");
      if (tdcParams) {
        TOPNominalTDC nominalTDC(tdcParams.getInt("numWindows"),
                                 tdcParams.getInt("subBits"),
                                 tdcParams.getTime("syncTimeBase"),
                                 tdcParams.getInt("numofBunches"),
                                 tdcParams.getTime("offset"),
                                 tdcParams.getTime("pileupTime"),
                                 tdcParams.getTime("doubleHitResolution"),
                                 tdcParams.getTime("timeJitter"),
                                 tdcParams.getDouble("efficiency"));
        geo->setNominalTDC(nominalTDC);
      } else {
        TOPNominalTDC nominalTDC(pmtParams.getInt("TDCbits"),
                                 pmtParams.getTime("TDCbitwidth"),
                                 pmtParams.getTime("TDCoffset", 0),
                                 pmtParams.getTime("TDCpileupTime", 0),
                                 pmtParams.getTime("TDCdoubleHitResolution", 0),
                                 pmtParams.getTime("TDCtimeJitter", 50e-3),
                                 pmtParams.getDouble("TDCefficiency", 1));
        geo->setNominalTDC(nominalTDC);
      }

      return geo;
    }


    std::string TOPGeometryPar::addNumber(const std::string& str, unsigned number)
    {
      stringstream ss;
      if (number < 10) {
        ss << str << "0" << number;
      } else {
        ss << str << number;
      }
      string out;
      ss >> out;
      return out;
    }


    void TOPGeometryPar::readOldQBB(const GearDir& content)
    {

      // Support structure

      GearDir supParams(content, "Support");
      m_PannelThickness = supParams.getLength("PannelThickness");
      m_PlateThickness = supParams.getLength("PlateThickness");
      m_LowerGap = supParams.getLength("lowerGap");
      m_UpperGap = supParams.getLength("upperGap");
      m_SideGap = supParams.getLength("sideGap");
      m_forwardGap = supParams.getLength("forwardGap");
      m_backwardGap = supParams.getLength("backGap");
      m_pannelMaterial = supParams.getString("PannelMaterial");
      m_insideMaterial = supParams.getString("FillMaterial");

      // other

      m_brokenFraction = content.getDouble("Bars/BrokenJointFraction", 0);

    }


  } // End namespace TOP
} // End namespace Belle2
