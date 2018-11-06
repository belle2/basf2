/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/dbobjects/BKLMGeometryPar.h>
#include <framework/gearbox/Gearbox.h>
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

  char name[80] = "";
  m_DoBeamBackgroundStudy = content.getBool("BeamBackgroundStudy");
  m_Rotation = content.getAngle("Rotation");
  m_OffsetZ = content.getLength("OffsetZ");
  m_Phi = content.getLength("Phi");
  m_NSector = content.getNumberNodes("Sectors/Forward/Sector");
  if (m_NSector > NSECTOR) { // array-bounds check
    B2FATAL("BKLMGeometryPar::read(): sectors array size exceeded:"
            << LogVar("# of sectors", m_NSector)
            << LogVar("array size", NSECTOR));
  }
  m_SolenoidOuterRadius = content.getLength("SolenoidOuterRadius");
  m_OuterRadius = content.getLength("OuterRadius");
  m_HalfLength = content.getLength("HalfLength");
  m_NLayer = content.getNumberNodes("Layers/Layer");
  if (m_NLayer > NLAYER) { // array-bounds check
    B2FATAL("BKLMGeometryPar::read(): layer array size exceeded:"
            << LogVar("# of layers", m_NLayer)
            << LogVar("array size", NLAYER));
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
  m_NZScintsChimney = content.getInt("Layers/NZScintillatorsChimney");
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
    B2FATAL("BKLMGeometryPar::read(): readout stations array size exceeded:"
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
  for (int fb = BKLM_FORWARD; fb <= BKLM_BACKWARD; ++fb) {
    bool isForward = (fb == BKLM_FORWARD);
    for (int sector = 1; sector <= m_NSector; ++sector) {
      sprintf(name, "/Sectors/%s/Sector[@sector=\"%d\"]", (isForward ? "Forward" : "Backward"), sector);
      GearDir sectorContent(content);
      sectorContent.append(name);
      m_SectorRotation[fb - 1][sector - 1] = sectorContent.getAngle("Phi");
      for (int layer = 1; layer <= m_NLayer; ++layer) {
        GearDir layerContent(sectorContent);
        sprintf(name, "/Layer[@layer=\"%d\"]", layer);
        layerContent.append(name);
        m_LocalReconstructionShiftX[fb - 1][sector - 1][layer - 1] = layerContent.getLength("ReconstructionShift/X");
        m_LocalReconstructionShiftY[fb - 1][sector - 1][layer - 1] = layerContent.getLength("ReconstructionShift/Y");
        m_LocalReconstructionShiftZ[fb - 1][sector - 1][layer - 1] = layerContent.getLength("ReconstructionShift/Z");
        if (layer <= NSCINTLAYER) {
          m_IsFlipped[fb - 1][sector - 1][layer - 1] = layerContent.getBool("Flip", false);
        }
      }
    }
  }

}
