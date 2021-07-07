/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/geometry/TOPGeometryPar.h>

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/logging/LogSystem.h>
#include <framework/geometry/BFieldManager.h>
#include <geometry/Materials.h>
#include <iostream>
#include <TSpline.h>
#include <algorithm>
#include <set>
#include <cmath>

using namespace std;

namespace Belle2 {

  namespace TOP {

    TOPGeometryPar* TOPGeometryPar::s_instance = 0;
    const double TOPGeometryPar::c_hc = 1239.84193; // [eV*nm]

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

      finalizeInitialization();

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
      if ((*m_geoDB)->getWavelengthFilter().getName().empty()) {
        m_oldPayload = true;
        B2WARNING("TOPGeometry: obsolete payload revision (pixel independent PDE) - please, check global tag");
      }
      if ((*m_geoDB)->getTTSes().empty()) {
        B2WARNING("TOPGeometry: obsolete payload revision (nominal TTS only) - please, check global tag");
      }
      if ((*m_geoDB)->arePDETuningFactorsEmpty()) {
        B2WARNING("TOPGeometry: old payload revision (before bugfix and update of optical properties)");
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

      finalizeInitialization();

    }

    void TOPGeometryPar::finalizeInitialization()
    {
      // set B field flag
      m_BfieldOn = (BFieldManager::getField(0, 0, 0).Mag() / Unit::T) > 0.1;

      // add call backs for PMT data
      m_pmtInstalled.addCallback(this, &TOPGeometryPar::clearCache);
      m_pmtQEData.addCallback(this, &TOPGeometryPar::clearCache);

      // print geometry if the debug level for 'top' is set 10000
      const auto& logSystem = LogSystem::Instance();
      if (logSystem.isLevelEnabled(LogConfig::c_Debug, 10000, "top")) {
        getGeometry()->print();
        if (m_oldPayload) {
          cout << "Envelope QE same as nominal quantum efficiency" << endl << endl;
          return;
        }
        if (m_envelopeQE.isEmpty()) setEnvelopeQE();
        m_envelopeQE.print("Envelope QE");
      }
    }

    void TOPGeometryPar::clearCache()
    {
      m_envelopeQE.clear();
      m_pmts.clear();
      m_relEfficiencies.clear();
      m_pmtTypes.clear();
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

    double TOPGeometryPar::getPMTEfficiencyEnvelope(double energy) const
    {
      double lambda = c_hc / energy;

      if (m_oldPayload) { // filter transmittance is included in nominal QE, return it!
        return getGeometry()->getNominalQE().getEfficiency(lambda);
      }

      if (m_envelopeQE.isEmpty()) setEnvelopeQE();
      return m_envelopeQE.getEfficiency(lambda);
    }

    double TOPGeometryPar::getPMTEfficiency(double energy,
                                            int moduleID, int pmtID,
                                            double x, double y) const
    {
      const auto* geo = getGeometry();
      if (!geo->isModuleIDValid(moduleID)) return 0;

      double lambda = c_hc / energy;

      if (m_oldPayload) { // filter transmittance is included in nominal QE, return it!
        return geo->getNominalQE().getEfficiency(lambda);
      }

      if (m_pmts.empty()) mapPmtQEToPositions();

      int id = getUniquePmtID(moduleID, pmtID);
      const auto* pmtQE = m_pmts[id];
      if (!pmtQE) return 0;

      const auto& pmtArray = geo->getModule(moduleID).getPMTArray();
      auto pmtPixel = pmtArray.getPMT().getPixelID(x, y);
      if (pmtPixel == 0) return 0;

      auto pixelID = pmtArray.getPixelID(pmtID, pmtPixel);
      auto channel = getChannelMapper().getChannel(pixelID);

      double RQE = geo->getPDETuningFactor(getPMTType(moduleID, pmtID));
      if (m_channelRQE.isValid()) RQE *= m_channelRQE->getRQE(moduleID, channel);

      return pmtQE->getEfficiency(pmtPixel, lambda, m_BfieldOn) * RQE;

    }


    double TOPGeometryPar::getRelativePixelEfficiency(int moduleID, int pixelID) const
    {

      auto channel = getChannelMapper().getChannel(pixelID);
      auto pmtID = getChannelMapper().getPmtID(pixelID);

      double RQE = getGeometry()->getPDETuningFactor(getPMTType(moduleID, pmtID));
      if (m_channelRQE.isValid()) RQE *= m_channelRQE->getRQE(moduleID, channel);

      double thrEffi = 1.0;
      if (m_thresholdEff.isValid()) thrEffi = m_thresholdEff->getThrEff(moduleID, channel);

      if (m_oldPayload) { // nominal QE is used
        return RQE * thrEffi;
      }

      if (m_relEfficiencies.empty()) prepareRelEfficiencies();

      int id = getUniquePixelID(moduleID, pixelID);
      return m_relEfficiencies[id] * RQE * thrEffi;
    }


    unsigned TOPGeometryPar::getPMTType(int moduleID, int pmtID) const
    {
      if (m_pmtTypes.empty()) mapPmtTypeToPositions();

      int id = getUniquePmtID(moduleID, pmtID);
      return m_pmtTypes[id];
    }


    const TOPNominalTTS& TOPGeometryPar::getTTS(int moduleID, int pmtID) const
    {
      auto pmtType = getPMTType(moduleID, pmtID);
      return getGeometry()->getTTS(pmtType);
    }


    void TOPGeometryPar::setEnvelopeQE() const
    {
      if (m_pmtQEData.getEntries() == 0) {
        B2ERROR("DBArray TOPPmtQEs is empty");
        return;
      }

      double lambdaFirst = 0;
      for (const auto& pmt : m_pmtQEData) {
        if (pmt.getLambdaFirst() > 0) {
          lambdaFirst = pmt.getLambdaFirst();
          break;
        }
      }
      if (lambdaFirst == 0) {
        B2ERROR("DBArray TOPPmtQEs: lambdaFirst of all PMT found to be less or equal 0");
        return;
      }
      for (const auto& pmt : m_pmtQEData) {
        if (pmt.getLambdaFirst() > 0) {
          lambdaFirst = std::min(lambdaFirst, pmt.getLambdaFirst());
        }
      }

      double lambdaStep = 0;
      for (const auto& pmt : m_pmtQEData) {
        if (pmt.getLambdaStep() > 0) {
          lambdaStep = pmt.getLambdaStep();
          break;
        }
      }
      if (lambdaStep == 0) {
        B2ERROR("DBArray TOPPmtQEs: lambdaStep of all PMT found to be less or equal 0");
        return;
      }
      for (const auto& pmt : m_pmtQEData) {
        if (pmt.getLambdaStep() > 0) {
          lambdaStep = std::min(lambdaStep, pmt.getLambdaStep());
        }
      }

      std::map<std::string, const TOPPmtInstallation*> map;
      for (const auto& pmt : m_pmtInstalled) {
        map[pmt.getSerialNumber()] = &pmt;
      }
      const auto* geo = getGeometry();

      std::vector<float> envelopeQE;
      for (const auto& pmt : m_pmtQEData) {
        float ce = pmt.getCE(m_BfieldOn);
        auto pmtInstalled = map[pmt.getSerialNumber()];
        if (pmtInstalled) ce *= geo->getPDETuningFactor(pmtInstalled->getType());
        if (pmt.getLambdaFirst() == lambdaFirst and pmt.getLambdaStep() == lambdaStep) {
          const auto& envelope = pmt.getEnvelopeQE();
          if (envelopeQE.size() < envelope.size()) {
            envelopeQE.resize(envelope.size() - envelopeQE.size(), 0);
          }
          for (size_t i = 0; i < std::min(envelopeQE.size(), envelope.size()); i++) {
            envelopeQE[i] = std::max(envelopeQE[i], envelope[i] * ce);
          }
        } else {
          double lambdaLast = pmt.getLambdaLast();
          int nExtra = (lambdaLast - lambdaFirst) / lambdaStep + 1 - envelopeQE.size();
          if (nExtra > 0) envelopeQE.resize(nExtra, 0);
          for (size_t i = 0; i < envelopeQE.size(); i++) {
            float qe = pmt.getEnvelopeQE(lambdaFirst + lambdaStep * i);
            envelopeQE[i] = std::max(envelopeQE[i], qe * ce);
          }
        }
      }

      m_envelopeQE.set(lambdaFirst, lambdaStep, 1.0, envelopeQE, "EnvelopeQE");

      B2INFO("TOPGeometryPar: envelope of PMT dependent QE has been set");

    }


    void TOPGeometryPar::mapPmtQEToPositions() const
    {
      m_pmts.clear();

      std::map<std::string, const TOPPmtQE*> map;
      for (const auto& pmt : m_pmtQEData) {
        map[pmt.getSerialNumber()] = &pmt;
      }
      for (const auto& pmt : m_pmtInstalled) {
        int id = getUniquePmtID(pmt.getSlotNumber(), pmt.getPosition());
        m_pmts[id] = map[pmt.getSerialNumber()];
      }

      B2INFO("TOPGeometryPar: QE of PMT's mapped to positions, size = " << m_pmts.size());
    }


    void TOPGeometryPar::mapPmtTypeToPositions() const
    {
      for (const auto& pmt : m_pmtInstalled) {
        int id = getUniquePmtID(pmt.getSlotNumber(), pmt.getPosition());
        m_pmtTypes[id] = pmt.getType();
      }

      B2INFO("TOPGeometryPar: PMT types mapped to positions, size = "
             << m_pmtTypes.size());


      std::set<unsigned> types;
      for (const auto& pmt : m_pmtInstalled) {
        types.insert(pmt.getType());
      }
      const auto* geo = getGeometry();
      for (const auto& type : types) {
        if (geo->getTTS(type).getPMTType() != type) {
          B2WARNING("No TTS found for an installed PMT type. Nominal one will be used."
                    << LogVar("PMT type", type));
        }
      }

    }


    void TOPGeometryPar::prepareRelEfficiencies() const
    {
      m_relEfficiencies.clear();
      if (m_pmts.empty()) mapPmtQEToPositions();

      const auto* geo = getGeometry();

      const auto& nominalQE = geo->getNominalQE();
      double s0 = integralOfQE(nominalQE.getQE(), nominalQE.getCE(),
                               nominalQE.getLambdaFirst(), nominalQE.getLambdaStep());

      for (const auto& module : geo->getModules()) {
        auto moduleID = module.getModuleID();
        const auto& pmtArray = module.getPMTArray();
        int numPMTs = pmtArray.getSize();
        int numPMTPixels = pmtArray.getPMT().getNumPixels();
        for (int pmtID = 1; pmtID <= numPMTs; pmtID++) {
          const auto* pmtQE = m_pmts[getUniquePmtID(moduleID, pmtID)];
          for (int pmtPixel = 1; pmtPixel <= numPMTPixels; pmtPixel++) {
            double s = 0;
            if (pmtQE) {
              s = integralOfQE(pmtQE->getQE(pmtPixel), pmtQE->getCE(m_BfieldOn),
                               pmtQE->getLambdaFirst(), pmtQE->getLambdaStep());
            }
            auto pixelID = pmtArray.getPixelID(pmtID, pmtPixel);
            auto id = getUniquePixelID(moduleID, pixelID);
            m_relEfficiencies[id] = s / s0;
          }
        }
      }

      B2INFO("TOPGeometryPar: pixel relative quantum efficiencies have been set, size = "
             << m_relEfficiencies.size());
    }


    double TOPGeometryPar::integralOfQE(const std::vector<float>& qe, double ce,
                                        double lambdaFirst, double lambdaStep) const
    {
      if (qe.empty()) return 0;

      double s = 0;
      double lambda = lambdaFirst;
      double f1 = qe[0] / (lambda * lambda);
      for (size_t i = 1; i < qe.size(); i++) {
        lambda += lambdaStep;
        double f2 = qe[i] / (lambda * lambda);
        s += (f1 + f2) / 2;
        f1 = f2;
      }
      return s * c_hc * lambdaStep * ce;
    }


    TOPGeometry* TOPGeometryPar::createConfiguration(const GearDir& content)
    {
      TOPGeometry* geo = new TOPGeometry("TOPGeometry");

      // PMT array

      GearDir pmtParams(content, "PMTs/PMT");
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
      pmtArray.setSiliconeCookie(arrayParams.getLength("siliconeCookie/thickness"),
                                 arrayParams.getString("siliconeCookie/material"));
      pmtArray.setWavelengthFilter(arrayParams.getLength("wavelengthFilter/thickness"),
                                   arrayParams.getString("wavelengthFilter/material"));
      pmtArray.setAirGap(arrayParams.getLength("airGap", 0));
      double decoupledFraction = arrayParams.getDouble("decoupledFraction", 0);

      // modules

      GearDir moduleParams(content, "Modules");
      GearDir glueParams(moduleParams, "Glue");
      int numModules = moduleParams.getNumberNodes("Module");
      for (int slotID = 1; slotID <= numModules; slotID++) {
        std::string gearName = "Module[@slotID='" + std::to_string(slotID) + "']";
        GearDir slotParams(moduleParams, gearName);
        TOPGeoModule module(slotID,
                            slotParams.getLength("Radius"),
                            slotParams.getAngle("Phi"),
                            slotParams.getLength("BackwardZ"));
        int cNumber = slotParams.getInt("ConstructionNumber");
        module.setModuleCNumber(cNumber);
        module.setName(addNumber(module.getName(), cNumber));

        auto prism = createPrism(content, slotParams.getString("Prism"));
        prism.setName(addNumber(prism.getName(), cNumber));
        module.setPrism(prism);

        auto barSegment2 = createBarSegment(content, slotParams.getString("BarSegment2"));
        barSegment2.setName(addNumber(barSegment2.getName() + "2-", cNumber));
        barSegment2.setGlue(glueParams.getLength("Thicknes1"),
                            glueParams.getString("GlueMaterial"));
        module.setBarSegment2(barSegment2);

        auto barSegment1 = createBarSegment(content, slotParams.getString("BarSegment1"));
        barSegment1.setName(addNumber(barSegment1.getName() + "1-", cNumber));
        barSegment1.setGlue(glueParams.getLength("Thicknes2"),
                            glueParams.getString("GlueMaterial"));
        module.setBarSegment1(barSegment1);

        auto mirror = createMirrorSegment(content, slotParams.getString("Mirror"));
        mirror.setName(addNumber(mirror.getName(), cNumber));
        mirror.setGlue(glueParams.getLength("Thicknes3"),
                       glueParams.getString("GlueMaterial"));
        module.setMirrorSegment(mirror);

        module.setPMTArray(pmtArray);
        if (decoupledFraction > 0) module.generateDecoupledPMTs(decoupledFraction);

        geo->appendModule(module);
      }

      // displaced geometry (if defined)

      GearDir displacedGeometry(content, "DisplacedGeometry");
      if (displacedGeometry) {
        if (displacedGeometry.getInt("SwitchON") != 0) {
          B2WARNING("TOP: displaced geometry is activated");
          for (const GearDir& slot : displacedGeometry.getNodes("Slot")) {
            int moduleID = slot.getInt("@ID");
            if (!geo->isModuleIDValid(moduleID)) {
              B2WARNING("TOPGeometryPar: DisplacedGeometry.xml: invalid moduleID."
                        << LogVar("moduleID", moduleID));
              continue;
            }
            TOPGeoModuleDisplacement moduleDispl(slot.getLength("x"),
                                                 slot.getLength("y"),
                                                 slot.getLength("z"),
                                                 slot.getAngle("alpha"),
                                                 slot.getAngle("beta"),
                                                 slot.getAngle("gamma"));
            auto& module = const_cast<TOPGeoModule&>(geo->getModule(moduleID));
            module.setModuleDisplacement(moduleDispl);
          }
        }
      }

      // displaced PMT arrays (if defined)

      GearDir displacedPMTArrays(content, "DisplacedPMTArrays");
      if (displacedPMTArrays) {
        if (displacedPMTArrays.getInt("SwitchON") != 0) {
          B2WARNING("TOP: displaced PMT arrays are activated");
          for (const GearDir& slot : displacedPMTArrays.getNodes("Slot")) {
            int moduleID = slot.getInt("@ID");
            if (!geo->isModuleIDValid(moduleID)) {
              B2WARNING("TOPGeometryPar: DisplacedPMTArrays.xml: invalid moduleID."
                        << LogVar("moduleID", moduleID));
              continue;
            }
            TOPGeoPMTArrayDisplacement arrayDispl(slot.getLength("x"),
                                                  slot.getLength("y"),
                                                  slot.getAngle("alpha"));
            auto& module = const_cast<TOPGeoModule&>(geo->getModule(moduleID));
            module.setPMTArrayDisplacement(arrayDispl);
          }
        }
      }

      // broken glues (if any)

      GearDir brokenGlues(content, "BrokenGlues");
      if (brokenGlues) {
        if (brokenGlues.getInt("SwitchON") != 0) {
          auto material = brokenGlues.getString("material");
          for (const GearDir& slot : brokenGlues.getNodes("Slot")) {
            int moduleID = slot.getInt("@ID");
            if (!geo->isModuleIDValid(moduleID)) {
              B2WARNING("TOPGeometryPar: BrokenGlues.xml: invalid moduleID."
                        << LogVar("moduleID", moduleID));
              continue;
            }
            auto& module = const_cast<TOPGeoModule&>(geo->getModule(moduleID));
            for (const GearDir& glue : slot.getNodes("Glue")) {
              int glueID = glue.getInt("@ID");
              double fraction = glue.getDouble("fraction");
              if (fraction <= 0) continue;
              double angle = glue.getAngle("angle");
              module.setBrokenGlue(glueID, fraction, angle, material);
            }
          }
        }
      }

      // peel-off cookies (if any)

      GearDir peelOff(content, "PeelOffCookies");
      if (peelOff) {
        if (peelOff.getInt("SwitchON") != 0) {
          auto material = peelOff.getString("material");
          double thickness = peelOff.getLength("thickness");
          for (const GearDir& slot : peelOff.getNodes("Slot")) {
            int moduleID = slot.getInt("@ID");
            if (!geo->isModuleIDValid(moduleID)) {
              B2WARNING("TOPGeometryPar: PeelOffCookiess.xml: invalid moduleID."
                        << LogVar("moduleID", moduleID));
              continue;
            }
            auto& module = const_cast<TOPGeoModule&>(geo->getModule(moduleID));
            module.setPeelOffRegions(thickness, material);
            for (const GearDir& region : slot.getNodes("Region")) {
              int regionID = region.getInt("@ID");
              double fraction = region.getDouble("fraction");
              if (fraction <= 0) continue;
              double angle = region.getAngle("angle");
              module.appendPeelOffRegion(regionID, fraction, angle);
            }
          }
        }
      }

      // front-end electronics geometry

      GearDir feParams(content, "FrontEndGeo");
      GearDir fbParams(feParams, "FrontBoard");
      TOPGeoFrontEnd frontEnd;
      frontEnd.setFrontBoard(fbParams.getLength("width"),
                             fbParams.getLength("height"),
                             fbParams.getLength("thickness"),
                             fbParams.getLength("gap"),
                             fbParams.getLength("y"),
                             fbParams.getString("material"));
      GearDir hvParams(feParams, "HVBoard");
      frontEnd.setHVBoard(hvParams.getLength("width"),
                          hvParams.getLength("length"),
                          hvParams.getLength("thickness"),
                          hvParams.getLength("gap"),
                          hvParams.getLength("y"),
                          hvParams.getString("material"));
      GearDir bsParams(feParams, "BoardStack");
      frontEnd.setBoardStack(bsParams.getLength("width"),
                             bsParams.getLength("height"),
                             bsParams.getLength("length"),
                             bsParams.getLength("gap"),
                             bsParams.getLength("y"),
                             bsParams.getString("material"),
                             bsParams.getLength("spacerWidth"),
                             bsParams.getString("spacerMaterial"));
      geo->setFrontEnd(frontEnd, feParams.getInt("numBoardStacks"));

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

      // PMT type dependent TTS

      GearDir pmtTTSParams(content, "TTSofPMTs");
      for (const GearDir& ttsPar : pmtTTSParams.getNodes("TTSpar")) {
        int type = ttsPar.getInt("type");
        double tuneFactor = ttsPar.getDouble("PDEtuneFactor");
        TOPNominalTTS tts("TTS of " + ttsPar.getString("@name") + " PMT");
        tts.setPMTType(type);
        for (const GearDir& Gauss : ttsPar.getNodes("Gauss")) {
          tts.appendGaussian(Gauss.getDouble("fraction"),
                             Gauss.getTime("mean"),
                             Gauss.getTime("sigma"));
        }
        tts.normalize();
        geo->appendTTS(tts);
        geo->appendPDETuningFactor(type, tuneFactor);
      }

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
        nominalTDC.setADCBits(tdcParams.getInt("adcBits"));
        nominalTDC.setAveragePedestal(tdcParams.getInt("averagePedestal"));
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

      // single photon signal shape

      GearDir shapeParams(content, "SignalShape");
      if (shapeParams) {
        GearDir noiseBandwidth(shapeParams, "noiseBandwidth");
        TOPSignalShape signalShape(shapeParams.getArray("sampleValues"),
                                   geo->getNominalTDC().getSampleWidth(),
                                   shapeParams.getTime("tailTimeConstant"),
                                   noiseBandwidth.getDouble("pole1") / 1000,
                                   noiseBandwidth.getDouble("pole2") / 1000);
        geo->setSignalShape(signalShape);
      }

      // calibration pulse shape

      GearDir calpulseParams(content, "CalPulseShape");
      if (calpulseParams) {
        GearDir noiseBandwidth(calpulseParams, "noiseBandwidth");
        TOPSignalShape shape(calpulseParams.getArray("sampleValues"),
                             geo->getNominalTDC().getSampleWidth(),
                             calpulseParams.getTime("tailTimeConstant"),
                             noiseBandwidth.getDouble("pole1") / 1000,
                             noiseBandwidth.getDouble("pole2") / 1000);
        geo->setCalPulseShape(shape);
      }

      // wavelength filter bulk transmittance

      std::string materialNode = "Materials/Material[@name='TOPWavelengthFilterIHU340']";
      GearDir filterMaterial(content, materialNode);
      if (!filterMaterial) {
        B2FATAL("TOPGeometry: " << materialNode << " not found");
      }
      GearDir property(filterMaterial, "Property[@name='ABSLENGTH']");
      if (!property) {
        B2FATAL("TOPGeometry: " << materialNode << ", Property ABSLENGTH not found");
      }
      int numNodes = property.getNumberNodes("value");
      if (numNodes > 1) {
        double conversion = Unit::convertValue(1, property.getString("@unit", "GeV"));
        std::vector<double> energies;
        std::vector<double> absLengths;
        for (int i = 0; i < numNodes; i++) {
          GearDir value(property, "value", i + 1);
          energies.push_back(value.getDouble("@energy") * conversion / Unit::eV);// [eV]
          absLengths.push_back(value.getDouble() * Unit::mm); // [cm]
        }
        TSpline3 spline("absLen", energies.data(), absLengths.data(), energies.size());
        double lambdaFirst = c_hc / energies.back();
        double lambdaLast = c_hc / energies[0];
        double lambdaStep = 5; // [nm]
        int numSteps = (lambdaLast - lambdaFirst) / lambdaStep + 1;
        const double filterThickness = arrayParams.getLength("wavelengthFilter/thickness");
        std::vector<float> bulkTransmittances;
        for (int i = 0; i < numSteps; i++) {
          double wavelength = lambdaFirst + lambdaStep * i;
          double energy = c_hc / wavelength;
          double absLen = spline.Eval(energy);
          bulkTransmittances.push_back(exp(-filterThickness / absLen));
        }
        TOPWavelengthFilter filter(lambdaFirst, lambdaStep, bulkTransmittances);
        geo->setWavelengthFilter(filter);
      } else {
        B2FATAL("TOPGeometry: " << materialNode
                << ", Property ABSLENGTH has less than 2 nodes");
      }

      return geo;
    }


    TOPGeoBarSegment TOPGeometryPar::createBarSegment(const GearDir& content,
                                                      const std::string& SN)
    {
      // dimensions and material
      GearDir params(content, "QuartzBars/QuartzBar[@SerialNumber='" + SN + "']");
      TOPGeoBarSegment bar(params.getLength("Width"),
                           params.getLength("Thickness"),
                           params.getLength("Length"),
                           params.getString("Material"));
      bar.setVendorData(params.getString("Vendor"), SN);

      // optical surface
      std::string surfaceName = params.getString("OpticalSurface");
      double sigmaAlpha = params.getDouble("SigmaAlpha");
      GearDir surfaceParams(content, "Modules/Surface[@name='" + surfaceName + "']");
      auto& materials = geometry::Materials::getInstance();
      auto quartzSurface = materials.createOpticalSurfaceConfig(surfaceParams);
      bar.setSurface(quartzSurface, sigmaAlpha);

      return bar;
    }


    TOPGeoMirrorSegment TOPGeometryPar::createMirrorSegment(const GearDir& content,
                                                            const std::string& SN)
    {
      // dimensions and material
      GearDir params(content, "Mirrors/Mirror[@SerialNumber='" + SN + "']");
      TOPGeoMirrorSegment mirror(params.getLength("Width"),
                                 params.getLength("Thickness"),
                                 params.getLength("Length"),
                                 params.getString("Material"));
      mirror.setVendorData(params.getString("Vendor"), SN);
      mirror.setRadius(params.getLength("Radius"));
      mirror.setCenterOfCurvature(params.getLength("Xpos"), params.getLength("Ypos"));

      // mirror reflective coating
      auto& materials = geometry::Materials::getInstance();
      GearDir coatingParams(params, "Surface");
      mirror.setCoating(params.getLength("mirrorThickness"), "Al",
                        materials.createOpticalSurfaceConfig(coatingParams));

      // optical surface
      std::string surfaceName = params.getString("OpticalSurface");
      double sigmaAlpha = params.getDouble("SigmaAlpha");
      GearDir surfaceParams(content, "Modules/Surface[@name='" + surfaceName + "']");
      auto quartzSurface = materials.createOpticalSurfaceConfig(surfaceParams);
      mirror.setSurface(quartzSurface, sigmaAlpha);

      return mirror;
    }


    TOPGeoPrism TOPGeometryPar::createPrism(const GearDir& content,
                                            const std::string& SN)
    {
      // dimensions and material
      GearDir params(content, "Prisms/Prism[@SerialNumber='" + SN + "']");
      TOPGeoPrism prism(params.getLength("Width"),
                        params.getLength("Thickness"),
                        params.getLength("Length"),
                        params.getLength("ExitThickness"),
                        0.,
                        params.getString("Material"));
      prism.setAngle(params.getAngle("Angle"));
      prism.setVendorData(params.getString("Vendor"), SN);

      // optical surface
      std::string surfaceName = params.getString("OpticalSurface");
      double sigmaAlpha = params.getDouble("SigmaAlpha");
      GearDir surfaceParams(content, "Modules/Surface[@name='" + surfaceName + "']");
      auto& materials = geometry::Materials::getInstance();
      auto quartzSurface = materials.createOpticalSurfaceConfig(surfaceParams);
      prism.setSurface(quartzSurface, sigmaAlpha);

      return prism;
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

    double TOPGeometryPar::refractiveIndex(double lambda) const
    {
      // parameters of SellMeier equation (Matsuoka-san, 24.11.2018)
      // from the specs of Corning HPFS 7980
      // https://www.corning.com/media/worldwide/csm/documents/5bf092438c5546dfa9b08e423348317b.pdf
      double b[] = {0.683740494, 0.420323613, 0.585027480};
      double c[] = {0.00460352869, 0.0133968856, 64.4932732};

      double x = pow(lambda * 0.001, 2);
      double y = 1;
      for (int i = 0; i < 3; i++) {
        y += b[i] * x / (x - c[i]);
      }
      return sqrt(y);
    }

    double TOPGeometryPar::getPhaseIndex(double energy) const
    {
      double lambda = c_hc / energy;
      return refractiveIndex(lambda);
    }

    double TOPGeometryPar::getGroupIndex(double energy) const
    {
      double lambda = c_hc / energy;
      double dl = 1.0; // [nm]
      double n = refractiveIndex(lambda);
      double dndl = (refractiveIndex(lambda + dl / 2) - refractiveIndex(lambda - dl / 2)) / dl;
      return n / (1 + lambda / n * dndl);
    }

  } // End namespace TOP
} // End namespace Belle2
