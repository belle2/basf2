/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/bklm/geometry/GeometryPar.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/KLMChannelIndex.h>
#include <klm/dbobjects/bklm/BKLMAlignment.h>

/* Belle 2 headers. */
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>
#include <simulation/background/BkgSensitiveDetector.h>

using namespace std;
using namespace Belle2::bklm;

GeometryPar* GeometryPar::m_Instance = nullptr;

GeometryPar* GeometryPar::instance(void)
{
  if (m_Instance)
    return m_Instance;
  B2FATAL("instance() called without initialization");
  return nullptr;  // never reached
}

GeometryPar* GeometryPar::instance(const GearDir& content)
{
  if (!m_Instance)
    m_Instance = new GeometryPar(content);
  return m_Instance;
}

GeometryPar* GeometryPar::instance(const BKLMGeometryPar& element)
{
  if (!m_Instance)
    m_Instance = new GeometryPar(element);
  return m_Instance;
}

GeometryPar::GeometryPar(const GearDir& content) :
  m_DoBeamBackgroundStudy(false),
  m_BkgSensitiveDetector(nullptr),
  m_NSector(BKLMElementNumbers::getMaximalSectorNumber()),
  m_NLayer(BKLMElementNumbers::getMaximalLayerNumber())
{
  clear();
  read(content);
  calculate();
}

GeometryPar::GeometryPar(const BKLMGeometryPar& element) :
  m_DoBeamBackgroundStudy(false),
  m_BkgSensitiveDetector(nullptr),
  m_NSector(BKLMElementNumbers::getMaximalSectorNumber()),
  m_NLayer(BKLMElementNumbers::getMaximalLayerNumber())
{
  clear();
  readFromDB(element);
  calculate();
}

GeometryPar::~GeometryPar()
{
  clear();
}

void GeometryPar::clear()
{
  for (std::map<int, Module*>::iterator m = m_Modules.begin(); m != m_Modules.end(); ++m) { delete m->second; }
  m_Modules.clear();

  m_Alignments.clear();
  m_Displacements.clear();
}

// Get BKLM geometry parameters from Gearbox (no calculations here)
void GeometryPar::read(const GearDir& content)
{

  char name[80] = "";
  m_DoBeamBackgroundStudy = content.getBool("BeamBackgroundStudy");
  m_Rotation = content.getAngle("Rotation");
  m_OffsetZ = content.getLength("OffsetZ");
  m_Phi = content.getLength("Phi");
  m_NSector = content.getNumberNodes("Sectors/Forward/Sector");
  if (m_NSector > BKLMElementNumbers::getMaximalSectorNumber()) { // array-bounds check
    B2FATAL("BKLM GeometryPar::read(): sector array size exceeded:"
            << LogVar("# of sectors", m_NSector)
            << LogVar("array size", BKLMElementNumbers::getMaximalSectorNumber()));
  }
  m_SolenoidOuterRadius = content.getLength("SolenoidOuterRadius");
  m_OuterRadius = content.getLength("OuterRadius");
  m_HalfLength = content.getLength("HalfLength");
  m_NLayer = content.getNumberNodes("Layers/Layer");
  if (m_NLayer > BKLMElementNumbers::getMaximalLayerNumber()) { // array-bounds check
    B2FATAL("BKLM GeometryPar::read(): layer array size exceeded:"
            << LogVar("# of layers", m_NLayer)
            << LogVar("array size", BKLMElementNumbers::getMaximalLayerNumber()));
  }

  m_IronNominalHeight = content.getLength("Layers/IronNominalHeight");
  m_IronActualHeight = content.getLength("Layers/IronActualHeight");
  m_Gap1NominalHeight = content.getLength("Layers/Layer[@layer=\"1\"]/GapNominalHeight");
  m_GapNominalHeight = content.getLength("Layers/GapNominalHeight");
  m_Gap1InnerRadius = content.getLength("Layers/InnerRadius");
  m_Gap1IronWidth = content.getLength("Layers/Layer[@layer=\"1\"]/GapIronWidth");
  m_GapIronWidth = content.getLength("Layers/GapIronWidth");
  m_GapLength = content.getLength("Layers/GapLength");

  m_NZStrips = content.getInt("Layers/NZStrips");
  m_NZStripsChimney = content.getInt("Layers/NZStripsChimney");
  m_NZScints = content.getInt("Layers/NZScintillators");
  m_NZScintsChimney = content.getInt("Layers/NZScintillatorsChimney");
  if (m_NZScints > NZSCINT) { // array-bounds check
    B2FATAL("BKLM GeometryPar::read(): z-scint array size exceeded:"
            << LogVar("# of z scintillators", m_NZScints)
            << LogVar("array size", NZSCINT));
  }
  if (m_NZScintsChimney > NZSCINT) { // array-bounds check
    B2FATAL("BKLM GeometryPar::read(): chimney sector z-scint array size exceeded:"
            << LogVar("# of z scintillators", m_NZScintsChimney)
            << LogVar("array size", NZSCINT));
  }

  m_ModuleLength = content.getLength("Module/Length");
  m_ModuleLengthChimney = content.getLength("Module/LengthChimney");
  m_ModuleCoverHeight = content.getLength("Module/CoverHeight");
  m_ModuleCopperHeight = content.getLength("Module/CopperHeight");
  m_ModuleFoamHeight = content.getLength("Module/FoamHeight");
  m_ModuleMylarHeight = content.getLength("Module/MylarHeight");
  m_ModuleGlassHeight = content.getLength("Module/GlassHeight");
  m_ModuleGasHeight = content.getLength("Module/GasHeight");
  m_ModuleFrameWidth = content.getLength("Module/FrameWidth");
  m_ModuleFrameThickness = content.getLength("Module/FrameThickness");
  m_ModuleGasSpacerWidth = content.getLength("Module/SpacerWidth");
  m_ModulePolystyreneInnerHeight = content.getLength("Module/PolystyreneInnerHeight");
  m_ModulePolystyreneOuterHeight = content.getLength("Module/PolystyreneOuterHeight");
  m_ScintWidth = content.getLength("Module/Scintillator/Width");
  m_ScintHeight = content.getLength("Module/Scintillator/Height");
  m_ScintBoreRadius = content.getLength("Module/Scintillator/BoreRadius");
  m_ScintFiberRadius = content.getLength("Module/Scintillator/FiberRadius");
  m_ScintTiO2ThicknessTop = content.getLength("Module/Scintillator/TiO2ThicknessTop");
  m_ScintTiO2ThicknessSide = content.getLength("Module/Scintillator/TiO2ThicknessSide");

  m_ChimneyLength = content.getLength("Chimney/Length");
  m_ChimneyWidth = content.getLength("Chimney/Width");
  m_ChimneyCoverThickness = content.getLength("Chimney/CoverThickness");
  m_ChimneyHousingInnerRadius = content.getLength("Chimney/HousingInnerRadius");
  m_ChimneyHousingOuterRadius = content.getLength("Chimney/HousingOuterRadius");
  m_ChimneyShieldInnerRadius = content.getLength("Chimney/ShieldInnerRadius");
  m_ChimneyShieldOuterRadius = content.getLength("Chimney/ShieldOuterRadius");
  m_ChimneyPipeInnerRadius = content.getLength("Chimney/PipeInnerRadius");
  m_ChimneyPipeOuterRadius = content.getLength("Chimney/PipeOuterRadius");

  m_RibThickness = content.getLength("RibThickness");
  m_CablesWidth = content.getLength("CablesWidth");
  m_BraceWidth = content.getLength("BraceWidth");
  m_BraceWidthChimney = content.getLength("BraceWidthChimney");

  m_SupportPlateWidth = content.getLength("SupportPlateWidth");
  m_SupportPlateHeight = content.getLength("SupportPlateHeight");
  m_SupportPlateLength = content.getLength("SupportPlateLength");
  m_SupportPlateLengthChimney = content.getLength("SupportPlateLengthChimney");

  m_BracketWidth = content.getLength("BracketWidth");
  m_BracketThickness = content.getLength("BracketThickness");
  m_BracketLength = content.getLength("BracketLength");
  m_BracketRibWidth = content.getLength("BracketRibWidth");
  m_BracketRibThickness = content.getLength("BracketRibThickness");
  m_BracketInnerRadius = content.getLength("BracketInnerRadius");
  m_BracketInset = content.getLength("BracketInset");
  m_BracketCutoutDphi = content.getAngle("BracketCutoutDphi");

  m_NReadoutStation = content.getNumberNodes("Readout/Stations/Station");
  if (m_NReadoutStation > NSTATION) { // array-bounds check
    B2FATAL("BKLM GeometryPar::read(): readout stations array size exceeded:"
            << LogVar("# of readout stations", m_NReadoutStation)
            << LogVar("array size", NSTATION));
  }
  for (int station = 1; station <= m_NReadoutStation; ++station) {
    sprintf(name, "/Readout/Stations/Station[@station=\"%d\"]", station);
    GearDir stationContent = content;
    stationContent.append(name);
    m_ReadoutStationIsPhi[station - 1] = stationContent.getBool("IsPhi");
    m_ReadoutStationPosition[station - 1] = stationContent.getLength("Position");
  }
  m_ReadoutContainerLength = content.getLength("Readout/Container/Length");
  m_ReadoutContainerWidth = content.getLength("Readout/Container/Width");
  m_ReadoutContainerHeight = content.getLength("Readout/Container/Height");
  m_ReadoutCarrierLength = content.getLength("Readout/Carrier/Length");
  m_ReadoutCarrierWidth = content.getLength("Readout/Carrier/Width");
  m_ReadoutCarrierHeight = content.getLength("Readout/Carrier/Height");
  m_ReadoutPreamplifierLength = content.getLength("Readout/Preamplifier/Length");
  m_ReadoutPreamplifierWidth = content.getLength("Readout/Preamplifier/Width");
  m_ReadoutPreamplifierHeight = content.getLength("Readout/Preamplifier/Height");
  m_ReadoutPreamplifierPosition = content.getArray("Readout/Preamplifier/Position");
  m_ReadoutConnectorsLength = content.getLength("Readout/Connectors/Length");
  m_ReadoutConnectorsWidth = content.getLength("Readout/Connectors/Width");
  m_ReadoutConnectorsHeight = content.getLength("Readout/Connectors/Height");
  m_ReadoutConnectorsPosition = content.getLength("Readout/Connectors/Position");
  m_MPPCHousingRadius = content.getLength("Readout/MPPC/Housing/Radius");
  m_MPPCHousingLength = content.getLength("Readout/MPPC/Housing/Length");
  m_MPPCLength = content.getLength("Readout/MPPC/Sensor/Length");
  m_MPPCWidth = content.getLength("Readout/MPPC/Sensor/Width");
  m_MPPCHeight = content.getLength("Readout/MPPC/Sensor/Height");
  // by-layer values that are common for all sectors and forward/backward
  for (int layer = 1; layer <= m_NLayer; ++layer) {
    sprintf(name, "/Layers/Layer[@layer=\"%d\"]", layer);
    GearDir layerContent = content;
    layerContent.append(name);
    m_HasRPCs[layer - 1] = layerContent.getBool("HasRPCs");
    m_NPhiStrips[layer - 1] = layerContent.getInt("PhiStrips/NStrips");
    m_PhiStripWidth[layer - 1] = layerContent.getLength("PhiStrips/Width");
    m_ZStripWidth[layer - 1] = layerContent.getLength("ZStrips/Width");
    m_ScintEnvelopeOffsetSign[layer - 1] = layerContent.getInt("ScintEnvelopeOffsetSign");
    m_NPhiScints[layer - 1] = layerContent.getInt("PhiScintillators/NScints", 0);
    if (m_NPhiScints[layer - 1] > NPHISCINT) { // array-bounds check
      B2FATAL("BKLM GeometryPar::read(): phi-scint array size exceeded:"
              << LogVar("in zero-based layer", layer - 1)
              << LogVar("# phi scintillators", m_NPhiScints[layer - 1])
              << LogVar("array size", NPHISCINT));
    }
    for (int scint = 1; scint <= m_NZScints; ++scint) {
      sprintf(name, "/ZScintillators/Scint[@scint=\"%d\"]", scint);
      GearDir scintContent(layerContent);
      scintContent.append(name);
      m_ZScintDLength[layer - 1][scint - 1] = scintContent.getLength("DLength", 0.0);
    }
  }
  // values that depend on fb/sector/layer
  for (int section = 0; section <= BKLMElementNumbers::getMaximalSectionNumber(); ++section) {
    bool isForward = (section == BKLMElementNumbers::c_ForwardSection);
    for (int sector = 1; sector <= m_NSector; ++sector) {
      sprintf(name, "/Sectors/%s/Sector[@sector=\"%d\"]", (isForward ? "Forward" : "Backward"), sector);
      GearDir sectorContent(content);
      sectorContent.append(name);
      m_SectorRotation[section][sector - 1] = sectorContent.getAngle("Phi");
      for (int layer = 1; layer <= m_NLayer; ++layer) {
        GearDir layerContent(sectorContent);
        sprintf(name, "/Layer[@layer=\"%d\"]", layer);
        layerContent.append(name);
        m_LocalReconstructionShiftX[section][sector - 1][layer - 1] = layerContent.getLength("ReconstructionShift/X");
        m_LocalReconstructionShiftY[section][sector - 1][layer - 1] = layerContent.getLength("ReconstructionShift/Y");
        m_LocalReconstructionShiftZ[section][sector - 1][layer - 1] = layerContent.getLength("ReconstructionShift/Z");
        m_IsFlipped[section][sector - 1][layer - 1] = layerContent.getBool("Flip", false);
      }
    }
  }
}

// Get BKLM geometry parameters from dbobject (no calculations here)
void GeometryPar::readFromDB(const BKLMGeometryPar& element)
{

  m_DoBeamBackgroundStudy = element.doBeamBackgroundStudy();
  m_Rotation = element.getRotation();
  m_OffsetZ = element.getOffsetZ();
  m_Phi = element.getPhi();
  m_NSector = element.getNSector(); // array-bounds check has already been done
  m_SolenoidOuterRadius = element.getSolenoidOuterRadius();
  m_OuterRadius = element.getOuterRadius();
  m_HalfLength = element.getHalfLength();
  m_NLayer = element.getNLayer(); // array-bounds check has already been done

  m_IronNominalHeight = element.getIronNominalHeight();
  m_IronActualHeight = element.getIronActualHeight();
  m_Gap1NominalHeight = element.getGap1NominalHeight();
  m_GapNominalHeight = element.getGapNominalHeight();
  m_Gap1InnerRadius = element.getGap1InnerRadius();
  m_Gap1IronWidth = element.getGap1IronWidth();
  m_GapIronWidth = element.getGapIronWidth();
  m_GapLength = element.getGapLength();

  m_NZStrips = element.getNZStrips();
  m_NZStripsChimney = element.getNZStripsChimney();
  m_NZScints = element.getNZScints(); // array-bounds check has already been done
  m_NZScintsChimney = element.getNZScintsChimney(); // array-bounds check has already been done

  m_ModuleLength = element.getModuleLength();
  m_ModuleLengthChimney = element.getModuleLengthChimney();
  m_ModuleCoverHeight = element.getModuleCoverHeight();
  m_ModuleCopperHeight = element.getModuleCopperHeight();
  m_ModuleFoamHeight = element.getModuleFoamHeight();
  m_ModuleMylarHeight = element.getModuleMylarHeight();
  m_ModuleGlassHeight = element.getModuleGlassHeight();
  m_ModuleGasHeight = element.getModuleGasHeight();
  m_ModuleFrameWidth = element.getModuleFrameWidth();
  m_ModuleFrameThickness = element.getModuleFrameThickness();
  m_ModuleGasSpacerWidth = element.getModuleGasSpacerWidth();
  m_ModulePolystyreneInnerHeight = element.getModulePolystyreneInnerHeight();
  m_ModulePolystyreneOuterHeight = element.getModulePolystyreneOuterHeight();
  m_ScintWidth = element.getScintWidth();
  m_ScintHeight = element.getScintHeight();
  m_ScintBoreRadius = element.getScintBoreRadius();
  m_ScintFiberRadius = element.getScintFiberRadius();
  m_ScintTiO2ThicknessTop = element.getScintTiO2ThicknessTop();
  m_ScintTiO2ThicknessSide = element.getScintTiO2ThicknessSide();

  m_ChimneyLength = element.getChimneyLength();
  m_ChimneyWidth = element.getChimneyWidth();
  m_ChimneyCoverThickness = element.getChimneyCoverThickness();
  m_ChimneyHousingInnerRadius = element.getChimneyHousingInnerRadius();
  m_ChimneyHousingOuterRadius = element.getChimneyHousingOuterRadius();
  m_ChimneyShieldInnerRadius = element.getChimneyShieldInnerRadius();
  m_ChimneyShieldOuterRadius = element.getChimneyShieldOuterRadius();
  m_ChimneyPipeInnerRadius = element.getChimneyPipeInnerRadius();
  m_ChimneyPipeOuterRadius = element.getChimneyPipeOuterRadius();

  m_RibThickness = element.getRibThickness();
  m_CablesWidth = element.getCablesWidth();
  m_BraceWidth = element.getBraceWidth();
  m_BraceWidthChimney = element.getBraceWidthChimney();

  m_SupportPlateWidth = element.getSupportPlateWidth();
  m_SupportPlateHeight = element.getSupportPlateHeight();
  m_SupportPlateLength = element.getSupportPlateLength();
  m_SupportPlateLengthChimney = element.getSupportPlateLengthChimney();

  m_BracketWidth = element.getBracketWidth();
  m_BracketThickness = element.getBracketThickness();
  m_BracketLength = element.getBracketLength();
  m_BracketRibWidth = element.getBracketRibWidth();
  m_BracketRibThickness = element.getBracketRibThickness();
  m_BracketInnerRadius = element.getBracketInnerRadius();
  m_BracketInset = element.getBracketInset();
  m_BracketCutoutDphi = element.getBracketCutoutDphi();

  m_NReadoutStation = element.getNReadoutStation(); // array-bounds check has already been done
  for (int station = 1; station <= m_NReadoutStation; ++station) {
    m_ReadoutStationIsPhi[station - 1] = element.getReadoutStationIsPhi(station);
    m_ReadoutStationPosition[station - 1] = element.getReadoutStationPosition(station);
  }
  m_ReadoutContainerLength = element.getReadoutContainerLength();
  m_ReadoutContainerWidth = element.getReadoutContainerWidth();
  m_ReadoutContainerHeight = element.getReadoutContainerHeight();
  m_ReadoutCarrierLength = element.getReadoutCarrierLength();
  m_ReadoutCarrierWidth = element.getReadoutCarrierWidth();
  m_ReadoutCarrierHeight = element.getReadoutCarrierHeight();
  m_ReadoutPreamplifierLength = element.getReadoutPreamplifierLength();
  m_ReadoutPreamplifierWidth = element.getReadoutPreamplifierWidth();
  m_ReadoutPreamplifierHeight = element.getReadoutPreamplifierHeight();
  for (int preamp = 1; preamp <= element.getNReadoutPreamplifierPosition(); ++preamp) {
    m_ReadoutPreamplifierPosition.push_back(element.getReadoutPreamplifierPosition(preamp));
  }
  m_ReadoutConnectorsLength = element.getReadoutConnectorsLength();
  m_ReadoutConnectorsWidth = element.getReadoutConnectorsWidth();
  m_ReadoutConnectorsHeight = element.getReadoutConnectorsHeight();
  m_ReadoutConnectorsPosition = element.getReadoutConnectorsPosition();
  m_MPPCHousingRadius = element.getMPPCHousingRadius();
  m_MPPCHousingLength = element.getMPPCHousingLength();
  m_MPPCLength = element.getMPPCLength();
  m_MPPCWidth = element.getMPPCWidth();
  m_MPPCHeight = element.getMPPCHeight();

  // by-layer values that are common for all sectors and forward/backward
  for (int layer = 1; layer <= m_NLayer; ++layer) {
    m_HasRPCs[layer - 1] = element.hasRPCs(layer);
    m_NPhiStrips[layer - 1] = element.getNPhiStrips(layer);
    m_PhiStripWidth[layer - 1] = element.getPhiStripWidth(layer);
    m_ZStripWidth[layer - 1] = element.getZStripWidth(layer);
    m_NPhiScints[layer - 1] = (m_HasRPCs[layer - 1] ? 0 : element.getNPhiScints(layer)); // array-bounds check has already been done
    m_ScintEnvelopeOffsetSign[layer - 1] = (m_HasRPCs[layer - 1] ? 0 : element.getScintEnvelopeOffsetSign(layer));
    for (int scint = 1; scint <= m_NZScints; ++scint) {
      m_ZScintDLength[layer - 1][scint - 1] = (m_HasRPCs[layer - 1] ? 0.0 : element.getZScintDLength(layer, scint));
    }
  }
  // values that depend on fb/sector/layer
  for (int section = 0; section <= BKLMElementNumbers::getMaximalSectionNumber(); ++section) {
    for (int sector = 1; sector <= m_NSector; ++sector) {
      m_SectorRotation[section][sector - 1] = element.getSectorRotation(section, sector);
      for (int layer = 1; layer <= m_NLayer; ++layer) {
        m_LocalReconstructionShiftX[section][sector - 1][layer - 1] = element.getLocalReconstructionShiftX(section, sector, layer);
        m_LocalReconstructionShiftY[section][sector - 1][layer - 1] = element.getLocalReconstructionShiftY(section, sector, layer);
        m_LocalReconstructionShiftZ[section][sector - 1][layer - 1] = element.getLocalReconstructionShiftZ(section, sector, layer);
        m_IsFlipped[section][sector - 1][layer - 1] = false;
        if (layer <= NSCINTLAYER) {
          m_IsFlipped[section][sector - 1][layer - 1] = element.isFlipped(section, sector, layer);
        }
      }
    }
  }


}

// Calculate derived quantities from the database-defined values
void GeometryPar::calculate(void)
{
  if (m_DoBeamBackgroundStudy) {
    B2INFO("BKLM::GeometryPar: DoBeamBackgroundStudy is enabled");
    m_BkgSensitiveDetector = new BkgSensitiveDetector("BKLM");
  } else {
    B2DEBUG(20, "BKLM::GeometryPar: DoBeamBackgroundStudy is disabled");
  }
  m_Gap1ActualHeight = m_Gap1NominalHeight + (m_IronNominalHeight - m_IronActualHeight) / 2.0;
  m_GapActualHeight = m_GapNominalHeight + (m_IronNominalHeight - m_IronActualHeight);
  m_Layer1Height = m_IronNominalHeight + m_Gap1NominalHeight;
  m_LayerHeight = m_IronNominalHeight + m_GapNominalHeight;
  m_GapInnerRadius = m_Gap1InnerRadius + m_Layer1Height - m_LayerHeight;
  m_ModuleReadoutHeight = m_ModuleFoamHeight + (m_ModuleCopperHeight + m_ModuleMylarHeight) * 2.0;
  m_ModuleHeight = (m_ModuleCoverHeight + m_ModuleReadoutHeight + m_ModuleGasHeight + m_ModuleGlassHeight * 2.0) * 2.0;

  // set up displaced geometry
  readDisplacedGeoFromDB();
  // set up ReconstructionAlignment, so that those information can pass to Modules
  readAlignmentFromDB();

  for (int section = 0; section <= BKLMElementNumbers::getMaximalSectionNumber(); ++section) {
    bool isForward = (section == BKLMElementNumbers::c_ForwardSection);
    for (int sector = 1; sector <= m_NSector; ++sector) {
      bool hasChimney = (!isForward) && (sector == BKLMElementNumbers::c_ChimneySector);
      int nZStrips = (hasChimney ? m_NZStripsChimney : m_NZStrips);
      int nZScints = (hasChimney ? m_NZScintsChimney : m_NZScints);
      CLHEP::HepRotation rotation;
      if (!isForward)
        rotation.rotateX(M_PI);
      rotation.rotateZ(m_SectorRotation[section][sector - 1]);
      for (int layer = 1; layer <= m_NLayer; ++layer) {
        bool isFlipped = m_IsFlipped[section][sector - 1][layer - 1];
        CLHEP::Hep3Vector localReconstructionShift(m_LocalReconstructionShiftX[section][sector - 1][layer - 1],
                                                   m_LocalReconstructionShiftY[section][sector - 1][layer - 1],
                                                   m_LocalReconstructionShiftZ[section][sector - 1][layer - 1]);
        double dx = getActiveMiddleRadius(section, sector, layer);
        double dz = getModuleHalfSize(layer, hasChimney).z() - getModuleInteriorHalfSize2(layer, hasChimney).z();
        CLHEP::Hep3Vector localOrigin(dx, 0.0, dz);
        int moduleID = BKLMElementNumbers::moduleNumber(
                         section, sector, layer);
        if (m_HasRPCs[layer - 1]) {
          localOrigin.setZ(localOrigin.z() + getModuleInteriorHalfSize1(layer, hasChimney).z() - getGasHalfSize(layer, hasChimney).z());
          Module* pModule = new Module(m_PhiStripWidth[layer - 1],
                                       (layer == 1 ? 2 : 1),
                                       m_NPhiStrips[layer - 1] - (layer == 1 ? 1 : 0),
                                       m_ZStripWidth[layer - 1],
                                       nZStrips,
                                       CLHEP::Hep3Vector(0.0, 0.0, m_OffsetZ) + rotation(localOrigin),
                                       localReconstructionShift,
                                       rotation
                                      );
          pModule->setDisplacedGeo(getModuleDisplacedGeo(section, sector, layer));
          pModule->setAlignment(getModuleAlignment(section, sector, layer));
          m_Modules.insert(std::pair<int, Module*>(moduleID, pModule));
        } else {
          double dy = getScintEnvelopeOffset(layer, hasChimney).y() * getScintEnvelopeOffsetSign(layer) * (isFlipped ? -1.0 : 1.0);
          localOrigin.setY(dy);
          Module* pModule = new Module(m_ScintWidth,
                                       m_NPhiScints[layer - 1],
                                       getScintEnvelopeOffsetSign(layer),
                                       nZScints,
                                       CLHEP::Hep3Vector(0.0, 0.0, m_OffsetZ) + rotation(localOrigin),
                                       localReconstructionShift,
                                       rotation,
                                       isFlipped
                                      );
          pModule->setDisplacedGeo(getModuleDisplacedGeo(section, sector, layer));
          pModule->setAlignment(getModuleAlignment(section, sector, layer));
          m_Modules.insert(std::pair<int, Module*>(moduleID, pModule));
          double base = -0.5 * (m_NPhiScints[layer - 1] + 1) * m_ScintWidth;
          for (int scint = 1; scint <= m_NPhiScints[layer - 1]; ++scint) {
            double length = nZScints * m_ScintWidth;
            if (length > m_MaximalPhiStripLength)
              m_MaximalPhiStripLength = length;
            pModule->addPhiScint(scint,
                                 length,
                                 0.0,
                                 base + scint * m_ScintWidth
                                );
          }
          base = -0.5 * (nZScints + 1) * m_ScintWidth;
          for (int scint = 1; scint <= nZScints; ++scint) {
            int scint0 = m_NZScints - getNZScints(hasChimney);
            double dLength = m_ZScintDLength[layer - 1][scint0 + scint - 1];
            double length = m_NPhiScints[layer - 1] * m_ScintWidth + dLength;
            if (length > m_MaximalZStripLength)
              m_MaximalZStripLength = length;
            pModule->addZScint(scint,
                               length,
                               -0.5 * dLength,
                               base + scint * m_ScintWidth
                              );
          }
        }
      }
    }
  }

}

double GeometryPar::getLayerInnerRadius(int layer) const
{
  if (layer == 1) {
    return m_Gap1InnerRadius;
  }
  if (layer > m_NLayer) {
    return m_OuterRadius;
  }
  return m_GapInnerRadius - (m_IronNominalHeight - m_IronActualHeight) / 2.0 + m_LayerHeight * (layer - 1);
}

double GeometryPar::getLayerOuterRadius(int layer) const
{
  return getLayerInnerRadius(layer + 1);
}

const CLHEP::Hep3Vector GeometryPar::getGapHalfSize(int layer, bool hasChimney) const
{
  double r, ds, dx;
  if (layer == 1) {
    r = m_Gap1InnerRadius + m_Gap1NominalHeight;
    ds = m_Gap1IronWidth;
    dx = 0.5 * m_Gap1ActualHeight;
  } else {
    r = m_GapInnerRadius + m_GapNominalHeight + m_LayerHeight * (layer - 1);
    ds = m_GapIronWidth;
    dx = 0.5 * m_GapActualHeight;
  }
  double dz = 0.5 * (hasChimney ? m_GapLength - m_ChimneyLength : m_GapLength);
  return CLHEP::Hep3Vector(dx, r * tan(M_PI / m_NSector) - ds, dz);

}

const CLHEP::Hep3Vector GeometryPar::getModuleHalfSize(int layer, bool hasChimney) const
{
  CLHEP::Hep3Vector size = getGapHalfSize(layer, hasChimney);
  size.setX(0.5 * m_ModuleHeight);
  size.setZ(0.5 * (hasChimney ? m_ModuleLengthChimney : m_ModuleLength));
  return size;
}

const CLHEP::Hep3Vector GeometryPar::getModuleInteriorHalfSize1(int layer, bool hasChimney) const
{
  CLHEP::Hep3Vector size = getModuleHalfSize(layer, hasChimney);
  size.setX(size.x() - m_ModuleCoverHeight);
  size.setY(size.y() - m_ModuleFrameWidth);
  size.setZ(size.z() - m_ModuleFrameWidth);
  return size;
}

const CLHEP::Hep3Vector GeometryPar::getModuleInteriorHalfSize2(int layer, bool hasChimney) const
{
  CLHEP::Hep3Vector size = getModuleHalfSize(layer, hasChimney);
  size.setX(size.x() - m_ModuleFrameThickness - m_ModuleCoverHeight);
  size.setY(size.y() - m_ModuleFrameThickness);
  size.setZ(size.z() - m_ModuleFrameThickness);
  return size;
}

const CLHEP::Hep3Vector GeometryPar::getElectrodeHalfSize(int layer, bool hasChimney) const
{
  CLHEP::Hep3Vector size = getModuleInteriorHalfSize1(layer, hasChimney);
  size.setX(2.0 * m_ModuleGlassHeight + m_ModuleGasHeight);
  return size;
}

const CLHEP::Hep3Vector GeometryPar::getGasHalfSize(int layer, bool hasChimney) const
{
  CLHEP::Hep3Vector size = getElectrodeHalfSize(layer, hasChimney);
  size.setX(0.5 * m_ModuleGasHeight);
  size.setY(size.y() - m_ModuleGasSpacerWidth);
  size.setZ(size.z() - m_ModuleGasSpacerWidth);
  return size;
}

const CLHEP::Hep3Vector GeometryPar::getAirHalfSize(int layer, bool hasChimney) const
{
  CLHEP::Hep3Vector size = getModuleInteriorHalfSize2(layer, hasChimney);
  size.setX(m_ScintHeight);
  return size;
}

const CLHEP::Hep3Vector GeometryPar::getScintEnvelopeHalfSize(int layer, bool hasChimney) const
{
  return CLHEP::Hep3Vector(0.5 * m_ScintHeight,
                           0.5 * m_ScintWidth * getNPhiScints(layer),
                           0.5 * m_ScintWidth * getNZScints(hasChimney)
                          );
}

int GeometryPar::getNPhiScints(int layer) const
{
  if ((layer <= 0) || (layer > m_NLayer))
    return 0;
  return m_NPhiScints[layer - 1];
}

double GeometryPar::getPolystyreneOffsetX(void) const
{
  return 0.5 * (m_ModulePolystyreneInnerHeight - m_ModulePolystyreneOuterHeight);
}

double GeometryPar::getGapMiddleRadius(int layer) const
{
  if (layer == 1) {
    return m_Gap1InnerRadius + 0.5 * m_Gap1ActualHeight;
  }
  return m_GapInnerRadius + 0.5 * m_GapActualHeight + m_LayerHeight * (layer - 1);
}

double GeometryPar::getModuleMiddleRadius(int layer) const
{
  if (layer == 1) {
    return m_Gap1InnerRadius + 0.5 * m_Gap1NominalHeight;
  }
  return m_GapInnerRadius + 0.5 * m_GapNominalHeight + m_LayerHeight * (layer - 1);
}

double GeometryPar::getActiveMiddleRadius(int section, int sector, int layer) const
{
  // place the active radius midway between the two readout planes
  // (same as positioning in GeoBKLMCreator.cc)
  double dx = getModuleMiddleRadius(layer) - getGapMiddleRadius(layer);
  int s1 = sector - 1;
  int s2 = m_NSector / 2;
  if (s1 % s2 == 0) {
    dx = 0.0;
  } else if (s1 > s2) {
    dx = -dx;
  }
  double r = getGapMiddleRadius(layer) + dx;
  if (!hasRPCs(layer)) {
    if (m_IsFlipped[section][sector - 1][layer - 1]) {
      r -= getPolystyreneOffsetX();
    } else {
      r += getPolystyreneOffsetX();
    }
  }
  return r;
}

const CLHEP::Hep3Vector GeometryPar::getScintEnvelopeOffset(int layer, bool hasChimney) const
{
  CLHEP::Hep3Vector airHalfSize = getAirHalfSize(layer, hasChimney);
  CLHEP::Hep3Vector envelopeHalfSize = getScintEnvelopeHalfSize(layer, hasChimney);
  CLHEP::Hep3Vector offset((airHalfSize.x() - envelopeHalfSize.x()),
                           (airHalfSize.y() - envelopeHalfSize.y()),
                           -(airHalfSize.z() - envelopeHalfSize.z()));
  return offset;
}

const CLHEP::Hep3Vector GeometryPar::getChimneyHalfSize(int layer) const
{
  return CLHEP::Hep3Vector(0.5 * (getLayerOuterRadius(layer) - getLayerInnerRadius(layer)),
                           0.5 * m_ChimneyWidth,
                           0.5 * (m_ChimneyLength - m_ChimneyCoverThickness));
}

const CLHEP::Hep3Vector GeometryPar::getChimneyPosition(int layer) const
{
  return CLHEP::Hep3Vector(0.5 * (getLayerOuterRadius(layer) + getLayerInnerRadius(layer)),
                           0.0,
                           0.5 * (m_GapLength - m_ChimneyLength - m_ChimneyCoverThickness));
}

const CLHEP::Hep3Vector GeometryPar::getSupportPlateHalfSize(bool hasChimney) const
{
  CLHEP::Hep3Vector size;
  size.setX(0.5 * m_SupportPlateHeight);
  size.setY(0.5 * m_SupportPlateWidth);
  if (hasChimney) {
    size.setZ(0.5 * m_SupportPlateLengthChimney);
  } else {
    size.setZ(0.5 * m_SupportPlateLength);
  }
  return size;
}

double GeometryPar::getBracketZPosition(int bracket, bool hasChimney) const
{
  double z = m_BracketInset - 0.5 * m_GapLength;
  if (bracket == 0)
    return z;
  if (hasChimney) {
    return m_SupportPlateLengthChimney - m_GapLength - z;
  } else {
    return (bracket == 1 ? 0.0 : -z);
  }
}

const CLHEP::Hep3Vector GeometryPar::getReadoutContainerHalfSize(void) const
{
  return CLHEP::Hep3Vector(0.5 * m_ReadoutContainerWidth,
                           0.5 * m_ReadoutContainerLength,
                           0.5 * m_ReadoutContainerHeight
                          );
}

const CLHEP::Hep3Vector GeometryPar::getReadoutCarrierHalfSize(void) const
{
  return CLHEP::Hep3Vector(0.5 * m_ReadoutCarrierWidth,
                           0.5 * m_ReadoutCarrierLength,
                           0.5 * m_ReadoutCarrierHeight
                          );
}

const CLHEP::Hep3Vector GeometryPar::getReadoutPreamplifierHalfSize(void) const
{
  return CLHEP::Hep3Vector(0.5 * m_ReadoutPreamplifierWidth,
                           0.5 * m_ReadoutPreamplifierLength,
                           0.5 * m_ReadoutPreamplifierHeight
                          );
}

const CLHEP::Hep3Vector GeometryPar::getReadoutConnectorsHalfSize(void) const
{
  return CLHEP::Hep3Vector(0.5 * m_ReadoutConnectorsWidth,
                           0.5 * m_ReadoutConnectorsLength,
                           0.5 * m_ReadoutConnectorsHeight
                          );
}

bool GeometryPar::hasRPCs(int layer) const
{
  if ((layer <= 0) || (layer > m_NLayer))
    return false;
  return m_HasRPCs[layer - 1];
}

const Module* GeometryPar::findModule(int section, int sector, int layer) const
{
  int moduleID = BKLMElementNumbers::moduleNumber(section, sector, layer);
  map<int, Module*>::const_iterator iM = m_Modules.find(moduleID);
  return (iM == m_Modules.end() ? nullptr : iM->second);
}

const HepGeom::Transform3D GeometryPar::getModuleAlignment(int section, int sector, int layer) const
{
  int moduleID = BKLMElementNumbers::moduleNumber(section, sector, layer);
  map<int, HepGeom::Transform3D>::const_iterator iA = m_Alignments.find(moduleID);
  return (iA == m_Alignments.end() ? HepGeom::Transform3D() : iA->second);
}

const HepGeom::Transform3D GeometryPar::getModuleDisplacedGeo(int section, int sector, int layer) const
{
  int moduleID = BKLMElementNumbers::moduleNumber(section, sector, layer);
  map<int, HepGeom::Transform3D>::const_iterator iDis = m_Displacements.find(moduleID);
  return (iDis == m_Displacements.end() ? HepGeom::Transform3D() : iDis->second);
}

void GeometryPar::readAlignmentFromDB()
{
  DBObjPtr<BKLMAlignment> bklmAlignment;
  if (!bklmAlignment.isValid())
    B2FATAL("No BKLM alignment data.");
  KLMChannelIndex bklmModules(KLMChannelIndex::c_IndexLevelLayer);
  for (KLMChannelIndex bklmModule = bklmModules.beginBKLM();
       bklmModule != bklmModules.endBKLM(); ++bklmModule) {
    KLMModuleNumber module = bklmModule.getKLMModuleNumber();
    const KLMAlignmentData* alignmentData =
      bklmAlignment->getModuleAlignment(module);
    if (alignmentData == nullptr)
      B2FATAL("Incomplete BKLM alignment data.");
    HepGeom::Transform3D alignment;
    alignment = getTransformFromRigidBodyParams(
                  alignmentData->getDeltaU(),
                  alignmentData->getDeltaV(),
                  alignmentData->getDeltaW(),
                  alignmentData->getDeltaAlpha(),
                  alignmentData->getDeltaBeta(),
                  alignmentData->getDeltaGamma());
    int moduleID = BKLMElementNumbers::moduleNumber(
                     bklmModule.getSection(), bklmModule.getSector(),
                     bklmModule.getLayer());
    m_Alignments.insert(std::pair<int, HepGeom::Transform3D>(moduleID, alignment));
  }
  // Add callback to itself.
  bklmAlignment.addCallback(this, &bklm::GeometryPar::readAlignmentFromDB);
}

void GeometryPar::readDisplacedGeoFromDB()
{
  DBObjPtr<BKLMAlignment> bklmDisplacement("BKLMDisplacement");
  if (!bklmDisplacement.isValid())
    B2FATAL("No BKLM displaced geometry data in database!");
  KLMChannelIndex bklmModules(KLMChannelIndex::c_IndexLevelLayer);
  for (KLMChannelIndex bklmModule = bklmModules.beginBKLM();
       bklmModule != bklmModules.endBKLM(); ++bklmModule) {
    KLMModuleNumber module = bklmModule.getKLMModuleNumber();
    const KLMAlignmentData* displacementData =
      bklmDisplacement->getModuleAlignment(module);
    if (displacementData == nullptr)
      B2FATAL("Incomplete BKLM displacement data.");
    HepGeom::Transform3D displacement;
    displacement = getTransformFromRigidBodyParams(
                     displacementData->getDeltaU(),
                     displacementData->getDeltaV(),
                     displacementData->getDeltaW(),
                     displacementData->getDeltaAlpha(),
                     displacementData->getDeltaBeta(),
                     displacementData->getDeltaGamma());
    int moduleID = BKLMElementNumbers::moduleNumber(
                     bklmModule.getSection(), bklmModule.getSector(),
                     bklmModule.getLayer());
    m_Displacements.insert(std::pair<int, HepGeom::Transform3D>(moduleID, displacement));
  }
  // Add callback to itself.
  bklmDisplacement.addCallback(this, &bklm::GeometryPar::readDisplacedGeoFromDB);
}

HepGeom::Transform3D GeometryPar::getTransformFromRigidBodyParams(double dU, double dV, double dW, double dAlpha, double dBeta,
    double dGamma)
{

  CLHEP::HepRotation dy = CLHEP::HepRotationY(-dAlpha);
  CLHEP::HepRotation dz = CLHEP::HepRotationZ(-dBeta);
  CLHEP::HepRotation dx = CLHEP::HepRotationX(-dGamma);
  CLHEP::Hep3Vector shift(dW, dU, dV);

  //we do dx-->dz-->dy ( local w-->v-->u), because angles are definded as intrinsic rotations u-->v'-->w''
  //the equivalent one is extrinsic rotation with the order w (gamma)--> v(beta) --> u (alpha)
  //and then we map it to global rotation x -> z -> y axis
  return HepGeom::Transform3D(dy * dz * dx, shift);
}
