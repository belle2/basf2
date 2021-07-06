/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/bklm/BKLMGeometryPar.h>

/* Belle 2 headers. */
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;

BKLMGeometryPar::BKLMGeometryPar(const GearDir& content)
{
  read(content);
}

BKLMGeometryPar::~BKLMGeometryPar()
{
}

// Get BKLM geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void BKLMGeometryPar::read(const GearDir& content)
{
  if (!content) {
    B2FATAL("The GearDir to look for BKLM geometry parameters is not valid.");
    return;
  }
  char name[80] = "";
  GearDir data(content);
  data.append("/BKLM");
  m_DoBeamBackgroundStudy = data.getBool("BeamBackgroundStudy");
  m_Rotation = data.getAngle("Rotation");
  m_OffsetZ = data.getLength("OffsetZ");
  m_Phi = data.getLength("Phi");
  m_NSector = data.getNumberNodes("Sectors/Forward/Sector");
  if (m_NSector > BKLMElementNumbers::getMaximalSectorNumber()) { // array-bounds check
    B2FATAL("BKLMGeometryPar::read(): sectors array size exceeded:"
            << LogVar("# of sectors", m_NSector)
            << LogVar("array size", BKLMElementNumbers::getMaximalSectorNumber()));
  }
  m_SolenoidOuterRadius = data.getLength("SolenoidOuterRadius");
  m_OuterRadius = data.getLength("OuterRadius");
  m_HalfLength = data.getLength("HalfLength");
  m_NLayer = data.getNumberNodes("Layers/Layer");
  if (m_NLayer > BKLMElementNumbers::getMaximalLayerNumber()) { // array-bounds check
    B2FATAL("BKLMGeometryPar::read(): layer array size exceeded:"
            << LogVar("# of layers", m_NLayer)
            << LogVar("array size", BKLMElementNumbers::getMaximalLayerNumber()));
  }

  m_IronNominalHeight = data.getLength("Layers/IronNominalHeight");
  m_IronActualHeight = data.getLength("Layers/IronActualHeight");
  m_Gap1NominalHeight = data.getLength("Layers/Layer[@layer=\"1\"]/GapNominalHeight");
  m_GapNominalHeight = data.getLength("Layers/GapNominalHeight");
  m_Gap1InnerRadius = data.getLength("Layers/InnerRadius");
  m_Gap1IronWidth = data.getLength("Layers/Layer[@layer=\"1\"]/GapIronWidth");
  m_GapIronWidth = data.getLength("Layers/GapIronWidth");
  m_GapLength = data.getLength("Layers/GapLength");

  m_NZStrips = data.getInt("Layers/NZStrips");
  m_NZStripsChimney = data.getInt("Layers/NZStripsChimney");
  m_NZScints = data.getInt("Layers/NZScintillators");
  m_NZScintsChimney = data.getInt("Layers/NZScintillatorsChimney");
  m_NZScintsChimney = data.getInt("Layers/NZScintillatorsChimney");
  if (m_NZScints > NZSCINT) { // array-bounds check
    B2FATAL("BKLMGeometryPar::read(): z-scint array size exceeded:"
            << LogVar("# of z scintillators", m_NZScints)
            << LogVar("array size", NZSCINT));
  }
  if (m_NZScintsChimney > NZSCINT) { // array-bounds check
    B2FATAL("BKLMGeometryPar::read(): chimney sector z-scint array size exceeded:"
            << LogVar("# of z scintillators", m_NZScintsChimney)
            << LogVar("array size", NZSCINT));
  }

  m_ModuleLength = data.getLength("Module/Length");
  m_ModuleLengthChimney = data.getLength("Module/LengthChimney");
  m_ModuleCoverHeight = data.getLength("Module/CoverHeight");
  m_ModuleCopperHeight = data.getLength("Module/CopperHeight");
  m_ModuleFoamHeight = data.getLength("Module/FoamHeight");
  m_ModuleMylarHeight = data.getLength("Module/MylarHeight");
  m_ModuleGlassHeight = data.getLength("Module/GlassHeight");
  m_ModuleGasHeight = data.getLength("Module/GasHeight");
  m_ModuleFrameWidth = data.getLength("Module/FrameWidth");
  m_ModuleFrameThickness = data.getLength("Module/FrameThickness");
  m_ModuleGasSpacerWidth = data.getLength("Module/SpacerWidth");
  m_ModulePolystyreneInnerHeight = data.getLength("Module/PolystyreneInnerHeight");
  m_ModulePolystyreneOuterHeight = data.getLength("Module/PolystyreneOuterHeight");
  m_ScintWidth = data.getLength("Module/Scintillator/Width");
  m_ScintHeight = data.getLength("Module/Scintillator/Height");
  m_ScintBoreRadius = data.getLength("Module/Scintillator/BoreRadius");
  m_ScintFiberRadius = data.getLength("Module/Scintillator/FiberRadius");
  m_ScintTiO2ThicknessTop = data.getLength("Module/Scintillator/TiO2ThicknessTop");
  m_ScintTiO2ThicknessSide = data.getLength("Module/Scintillator/TiO2ThicknessSide");

  m_ChimneyLength = data.getLength("Chimney/Length");
  m_ChimneyWidth = data.getLength("Chimney/Width");
  m_ChimneyCoverThickness = data.getLength("Chimney/CoverThickness");
  m_ChimneyHousingInnerRadius = data.getLength("Chimney/HousingInnerRadius");
  m_ChimneyHousingOuterRadius = data.getLength("Chimney/HousingOuterRadius");
  m_ChimneyShieldInnerRadius = data.getLength("Chimney/ShieldInnerRadius");
  m_ChimneyShieldOuterRadius = data.getLength("Chimney/ShieldOuterRadius");
  m_ChimneyPipeInnerRadius = data.getLength("Chimney/PipeInnerRadius");
  m_ChimneyPipeOuterRadius = data.getLength("Chimney/PipeOuterRadius");

  m_RibThickness = data.getLength("RibThickness");
  m_CablesWidth = data.getLength("CablesWidth");
  m_BraceWidth = data.getLength("BraceWidth");
  m_BraceWidthChimney = data.getLength("BraceWidthChimney");

  m_SupportPlateWidth = data.getLength("SupportPlateWidth");
  m_SupportPlateHeight = data.getLength("SupportPlateHeight");
  m_SupportPlateLength = data.getLength("SupportPlateLength");
  m_SupportPlateLengthChimney = data.getLength("SupportPlateLengthChimney");

  m_BracketWidth = data.getLength("BracketWidth");
  m_BracketThickness = data.getLength("BracketThickness");
  m_BracketLength = data.getLength("BracketLength");
  m_BracketRibWidth = data.getLength("BracketRibWidth");
  m_BracketRibThickness = data.getLength("BracketRibThickness");
  m_BracketInnerRadius = data.getLength("BracketInnerRadius");
  m_BracketInset = data.getLength("BracketInset");
  m_BracketCutoutDphi = data.getAngle("BracketCutoutDphi");

  m_NReadoutStation = data.getNumberNodes("Readout/Stations/Station");
  if (m_NReadoutStation > NSTATION) { // array-bounds check
    B2FATAL("BKLMGeometryPar::read(): readout stations array size exceeded:"
            << LogVar("# of readout stations", m_NReadoutStation)
            << LogVar("array size", NSTATION));
  }
  for (int station = 1; station <= m_NReadoutStation; ++station) {
    sprintf(name, "/Readout/Stations/Station[@station=\"%d\"]", station);
    GearDir stationContent = data;
    stationContent.append(name);
    m_ReadoutStationIsPhi[station - 1] = stationContent.getBool("IsPhi");
    m_ReadoutStationPosition[station - 1] = stationContent.getLength("Position");
  }
  m_ReadoutContainerLength = data.getLength("Readout/Container/Length");
  m_ReadoutContainerWidth = data.getLength("Readout/Container/Width");
  m_ReadoutContainerHeight = data.getLength("Readout/Container/Height");
  m_ReadoutCarrierLength = data.getLength("Readout/Carrier/Length");
  m_ReadoutCarrierWidth = data.getLength("Readout/Carrier/Width");
  m_ReadoutCarrierHeight = data.getLength("Readout/Carrier/Height");
  m_ReadoutPreamplifierLength = data.getLength("Readout/Preamplifier/Length");
  m_ReadoutPreamplifierWidth = data.getLength("Readout/Preamplifier/Width");
  m_ReadoutPreamplifierHeight = data.getLength("Readout/Preamplifier/Height");
  m_ReadoutPreamplifierPosition = data.getArray("Readout/Preamplifier/Position");
  m_ReadoutConnectorsLength = data.getLength("Readout/Connectors/Length");
  m_ReadoutConnectorsWidth = data.getLength("Readout/Connectors/Width");
  m_ReadoutConnectorsHeight = data.getLength("Readout/Connectors/Height");
  m_ReadoutConnectorsPosition = data.getLength("Readout/Connectors/Position");
  m_MPPCHousingRadius = data.getLength("Readout/MPPC/Housing/Radius");
  m_MPPCHousingLength = data.getLength("Readout/MPPC/Housing/Length");
  m_MPPCLength = data.getLength("Readout/MPPC/Sensor/Length");
  m_MPPCWidth = data.getLength("Readout/MPPC/Sensor/Width");
  m_MPPCHeight = data.getLength("Readout/MPPC/Sensor/Height");

  // by-layer values that are common for all sectors and forward/backward
  for (int layer = 1; layer <= m_NLayer; ++layer) {
    sprintf(name, "/Layers/Layer[@layer=\"%d\"]", layer);
    GearDir layerContent = data;
    layerContent.append(name);
    m_HasRPCs[layer - 1] = layerContent.getBool("HasRPCs");
    m_NPhiStrips[layer - 1] = layerContent.getInt("PhiStrips/NStrips");
    m_PhiStripWidth[layer - 1] = layerContent.getLength("PhiStrips/Width");
    m_ZStripWidth[layer - 1] = layerContent.getLength("ZStrips/Width");
    if (layer <= NSCINTLAYER) {
      m_ScintEnvelopeOffsetSign[layer - 1] = layerContent.getInt("ScintEnvelopeOffsetSign");
      m_NPhiScints[layer - 1] = layerContent.getInt("PhiScintillators/NScints", 0);
      if (m_NPhiScints[layer - 1] > NPHISCINT) { // array-bounds check
        B2FATAL("BKLMGeometryPar::read(): phi-scint array size exceeded:"
                << LogVar("in zero-based layer", layer - 1)
                << LogVar("# of phi scintillators", m_NPhiScints[layer - 1])
                << LogVar("array size", NPHISCINT));
      }
      for (int scint = 1; scint <= m_NZScints; ++scint) {
        sprintf(name, "/ZScintillators/Scint[@scint=\"%d\"]", scint);
        GearDir scintContent(layerContent);
        scintContent.append(name);
        m_ZScintDLength[layer - 1][scint - 1] = scintContent.getLength("DLength", 0.0);
      }
    }
  }

  // values that depend on fb/sector/layer
  for (int section = 0; section <= BKLMElementNumbers::getMaximalSectionNumber(); ++section) {
    bool isForward = (section == BKLMElementNumbers::c_ForwardSection);
    for (int sector = 1; sector <= m_NSector; ++sector) {
      sprintf(name, "/Sectors/%s/Sector[@sector=\"%d\"]", (isForward ? "Forward" : "Backward"), sector);
      GearDir sectorContent(data);
      sectorContent.append(name);
      m_SectorRotation[section][sector - 1] = sectorContent.getAngle("Phi");
      for (int layer = 1; layer <= m_NLayer; ++layer) {
        GearDir layerContent(sectorContent);
        sprintf(name, "/Layer[@layer=\"%d\"]", layer);
        layerContent.append(name);
        m_LocalReconstructionShiftX[section][sector - 1][layer - 1] = layerContent.getLength("ReconstructionShift/X");
        m_LocalReconstructionShiftY[section][sector - 1][layer - 1] = layerContent.getLength("ReconstructionShift/Y");
        m_LocalReconstructionShiftZ[section][sector - 1][layer - 1] = layerContent.getLength("ReconstructionShift/Z");
        if (layer <= NSCINTLAYER) {
          m_IsFlipped[section][sector - 1][layer - 1] = layerContent.getBool("Flip", false);
        }
      }
    }
  }

}
