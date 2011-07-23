/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/geobklm/BKLMGeometryPar.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

BKLMGeometryPar* BKLMGeometryPar::m_Instance = NULL;

BKLMGeometryPar* BKLMGeometryPar::instance()
{
  if (!m_Instance) m_Instance = new BKLMGeometryPar();
  return m_Instance;
}

BKLMGeometryPar::BKLMGeometryPar()
{
  clear();
  read();
}

BKLMGeometryPar::~BKLMGeometryPar()
{
}

void BKLMGeometryPar::clear()
{
}

void BKLMGeometryPar::read()
{
  // Get Gearbox parameters for BKLM
  // NOTE:  Layer id="n" is referenced here as Layer[n+1]
  GearDir content = Gearbox::Instance().getContent("BKLM");

  m_Rotation = content.getParamAngle("Rotation");
  m_OffsetZ = content.getParamLength("OffsetZ");
  m_Phi = content.getParamLength("Phi");
  m_NSector = content.getNumberNodes("Sectors/Forward/Sector");
  m_SolenoidOuterRadius = content.getParamLength("SolenoidOuterRadius");
  m_OuterRadius = content.getParamLength("OuterRadius");
  m_HalfLength = content.getParamLength("HalfLength");
  m_NLayer = content.getNumberNodes("Layers/Layer");

  m_IronNominalHeight = content.getParamLength("Layers/IronNominalHeight");
  m_IronActualHeight = content.getParamLength("Layers/IronActualHeight");

  m_Gap0NominalHeight = content.getParamLength("Layers/Layer[1]/GapNominalHeight");
  m_Gap0ActualHeight = m_Gap0NominalHeight + (m_IronNominalHeight - m_IronActualHeight) / 2.0;

  m_GapNominalHeight = content.getParamLength("Layers/GapNominalHeight");
  m_GapActualHeight = m_GapNominalHeight + (m_IronNominalHeight - m_IronActualHeight);

  m_Layer0Height = m_IronNominalHeight + m_Gap0NominalHeight;
  m_LayerHeight = m_IronNominalHeight + m_GapNominalHeight;

  m_Gap0InnerRadius = content.getParamLength("Layers/InnerRadius");
  m_GapInnerRadius = m_Gap0InnerRadius + m_Layer0Height - m_LayerHeight;

  m_Gap0IronWidth = content.getParamLength("Layers/Layer[1]/GapIronWidth");
  m_GapIronWidth = content.getParamLength("Layers/GapIronWidth");

  m_GapLength = content.getParamLength("Layers/GapLength");

  m_ModuleLength = content.getParamLength("Module/Length");
  m_ModuleLengthChimney = content.getParamLength("Module/LengthChimney");
  m_ModuleCoverHeight = content.getParamLength("Module/CoverHeight");
  m_ModuleCopperHeight = content.getParamLength("Module/CopperHeight");
  m_ModuleFoamHeight = content.getParamLength("Module/FoamHeight");
  m_ModuleMylarHeight = content.getParamLength("Module/MylarHeight");
  m_ModuleGlassHeight = content.getParamLength("Module/GlassHeight");
  m_ModuleGasHeight = content.getParamLength("Module/GasHeight");
  m_ModuleReadoutHeight = m_ModuleFoamHeight + (m_ModuleCopperHeight + m_ModuleMylarHeight) * 2.0;
  m_ModuleHeight = (m_ModuleCoverHeight + m_ModuleReadoutHeight + m_ModuleGasHeight + m_ModuleGlassHeight * 2.0) * 2.0;
  m_ModuleFrameWidth = content.getParamLength("Module/FrameWidth");
  m_ModuleGasSpacerWidth = content.getParamLength("Module/SpacerWidth");
  m_ModuleElectrodeBorder = content.getParamLength(" Module/ElectrodeBorder");

  m_ChimneyLength = content.getParamLength("Chimney/Length");
  m_ChimneyWidth = content.getParamLength("Chimney/Width");
  m_ChimneyCoverThickness = content.getParamLength("Chimney/CoverThickness");
  m_ChimneyHousingInnerRadius = content.getParamLength("Chimney/HousingInnerRadius");
  m_ChimneyHousingOuterRadius = content.getParamLength("Chimney/HousingOuterRadius");
  m_ChimneyShieldInnerRadius = content.getParamLength("Chimney/ShieldInnerRadius");
  m_ChimneyShieldOuterRadius = content.getParamLength("Chimney/ShieldOuterRadius");
  m_ChimneyPipeInnerRadius = content.getParamLength("Chimney/PipeInnerRadius");
  m_ChimneyPipeOuterRadius = content.getParamLength("Chimney/PipeOuterRadius");
  m_RibThickness = content.getParamLength("RibThickness");
  m_CablesWidth = content.getParamLength("CablesWidth");
  m_BraceWidth = content.getParamLength("BraceWidth");
  m_BraceWidthChimney = content.getParamLength("BraceWidthChimney");
  m_SupportPlateWidth = content.getParamLength("SupportPlateWidth");
  m_SupportPlateHeight = content.getParamLength("SupportPlateHeight");

  double phiStripWidth[m_NLayer];
  double phiStripLength[m_NLayer];
  int    phiStripNumber[m_NLayer];
  int    phiStripMin[m_NLayer];
  int    phiStripMax[m_NLayer];
  double zStripWidth[m_NLayer];
  double zStripLength[m_NLayer];
  int    zStripNumber[m_NLayer];
  char name[40];
  for (int layer = 0; layer < m_NLayer; ++layer) {
    sprintf(name, "Layers/Layer[%d]", layer + 1);
    GearDir layerContent(content);
    layerContent.append(name);
    phiStripWidth[layer] = layerContent.getParamLength("/PhiStrips/Width");
    phiStripLength[layer] = layerContent.getParamLength("/PhiStrips/Length");
    phiStripNumber[layer] = layerContent.getParamIntValue("/PhiStrips/NStrips");
    phiStripMin[layer] = (layer == 0 ? 1 : 0);
    phiStripMax[layer] = (layer == 0 ? phiStripNumber[layer] - 2 : phiStripNumber[layer] - 1);
    zStripWidth[layer] = layerContent.getParamLength("/ZStrips/Width");
    zStripLength[layer] = layerContent.getParamLength("/ZStrips/Length");
    zStripNumber[layer] = layerContent.getParamIntValue("/ZStrips/NStrips");
  }

  CLHEP::Hep3Vector rSector(m_GapInnerRadius + 0.5 * m_GapNominalHeight, 0.0, 0.0);
  for (int fb = 0; fb < 2; ++fb) {
    CLHEP::Hep3Vector deltaZ(0.0, 0.0, (fb == 0 ? m_ModuleElectrodeBorder : -m_ModuleElectrodeBorder));
    for (int sector = 0; sector < m_NSector; ++sector) {
      sprintf(name, "Sectors/%s/Sector[%d]", (fb == 0 ? "Forward" : "Backward"), sector + 1);
      GearDir sectorContent(content);
      sectorContent.append(name);
      CLHEP::Hep3Vector shift(sectorContent.getParamLength("/Shift/X"),
                              sectorContent.getParamLength("/Shift/Y"),
                              sectorContent.getParamLength("/Shift/Z"));
      CLHEP::HepRotation rotation;
      if (fb == 1) rotation.rotateX(M_PI);
      rotation.rotateZ(sectorContent.getParamAngle("/Phi"));
      CLHEP::Hep3Vector translation(0.0, 0.0, m_OffsetZ);
      translation += rotation(rSector) + deltaZ;
      BKLMSector* pSector = new BKLMSector(fb, sector, m_NLayer, shift, translation, rotation);
      m_Sectors.push_back(pSector);
      for (int layer = 0; layer < m_NLayer; ++layer) {
        int zStripMin = 0;
        int zStripMax = zStripNumber[layer] - 1;
        if ((fb == 1) && (sector == 2)) zStripMax = 33;
        GearDir layerContent(sectorContent);
        sprintf(name, "Layer[%d]", layer + 1);
        layerContent.append(name);
        CLHEP::Hep3Vector lshift(sectorContent.getParamLength("/Shift/X"),
                                 sectorContent.getParamLength("/Shift/Y"),
                                 sectorContent.getParamLength("/Shift/Z"));
        double dr = (layer == 0 ? (m_GapNominalHeight - m_Gap0NominalHeight) * 0.5 : layer * m_LayerHeight);
        pSector->addModule(new BKLMModule(fb, sector, layer, lshift, dr, pSector,
                                          phiStripWidth[layer], phiStripLength[layer],
                                          phiStripNumber[layer], phiStripMin[layer], phiStripMax[layer],
                                          zStripWidth[layer], zStripLength[layer],
                                          zStripNumber[layer], zStripMin, zStripMax));
      }
    }
  }

}

const double BKLMGeometryPar::getLayerInnerRadius(int layer) const
{
  if (layer == 0) {
    return m_Gap0InnerRadius;
  }
  return m_GapInnerRadius - (m_IronNominalHeight - m_IronActualHeight) / 2.0 + m_LayerHeight * layer;
}

const double BKLMGeometryPar::getLayerOuterRadius(int layer) const
{
  int nextLyr = layer + 1;
  if (nextLyr == m_NLayer) {
    return m_OuterRadius;
  }
  return getLayerInnerRadius(nextLyr);
}

const CLHEP::Hep3Vector BKLMGeometryPar::getGapSize(int layer, bool flag) const
{
  double r, ds, dx;
  if (layer == 0) {
    r = m_Gap0InnerRadius + m_Gap0NominalHeight;
    ds = m_Gap0IronWidth;
    dx = m_Gap0ActualHeight / 2.0;
  } else {
    r = m_GapInnerRadius + m_GapNominalHeight + m_LayerHeight * layer;
    ds = m_GapIronWidth;
    dx = m_GapActualHeight / 2.0;
  }
  double dz = (flag ? m_GapLength - m_ChimneyLength : m_GapLength) / 2.0;
  return CLHEP::Hep3Vector(dx, r * tan(M_PI / m_NSector) - ds, dz);

}

const CLHEP::Hep3Vector BKLMGeometryPar::getModuleSize(int layer, bool flag) const
{
  CLHEP::Hep3Vector size = getGapSize(layer, flag);
  size.setX(m_ModuleHeight / 2.0);
  if (flag) {
    size.setZ(m_ModuleLengthChimney / 2.0);
  } else {
    size.setZ(m_ModuleLength / 2.0);
  }
  return size;
}

const CLHEP::Hep3Vector BKLMGeometryPar::getReadoutSize(int layer, bool flag) const
{
  CLHEP::Hep3Vector size = getModuleSize(layer, flag);
  size.setX(m_ModuleReadoutHeight + m_ModuleGlassHeight * 2.0 + m_ModuleGasHeight);
  size.setY(size.y() - m_ModuleFrameWidth);
  size.setZ(size.z() - m_ModuleFrameWidth);
  return size;
}

const CLHEP::Hep3Vector BKLMGeometryPar::getElectrodeSize(int layer, bool flag) const
{
  CLHEP::Hep3Vector size = getReadoutSize(layer, flag);
  size.setX(m_ModuleGlassHeight * 2.0 + m_ModuleGasHeight);
  return size;
}

const CLHEP::Hep3Vector BKLMGeometryPar::getGasSize(int layer, bool flag) const
{
  CLHEP::Hep3Vector size = getElectrodeSize(layer, flag);
  size.setX(m_ModuleGasHeight * 0.5);
  size.setY(size.y() - m_ModuleGasSpacerWidth);
  size.setZ(size.z() - m_ModuleGasSpacerWidth);
  return size;
}

const double BKLMGeometryPar::getGapMiddleRadius(int layer) const
{
  if (layer == 0) {
    return m_Gap0InnerRadius + m_Gap0ActualHeight / 2.0;
  }
  return m_GapInnerRadius + m_GapActualHeight / 2.0 + m_LayerHeight * layer;
}

const double BKLMGeometryPar::getModuleMiddleRadius(int layer) const
{
  if (layer == 0) {
    return m_Gap0InnerRadius + m_Gap0ActualHeight / 2.0;
  }
  return m_GapInnerRadius + m_GapNominalHeight / 2.0 + m_LayerHeight * layer;
}

const CLHEP::Hep3Vector BKLMGeometryPar::getChimneySize(int layer) const
{
  return CLHEP::Hep3Vector((getLayerOuterRadius(layer) - getLayerInnerRadius(layer)) / 2.0, m_ChimneyWidth / 2.0, m_ChimneyLength / 2.0);
}

const CLHEP::Hep3Vector BKLMGeometryPar::getChimneyPosition(int layer) const
{
  return CLHEP::Hep3Vector((getLayerOuterRadius(layer) + getLayerInnerRadius(layer)) / 2.0,
                           0.0,
                           m_GapLength - m_ChimneyLength / 2.0);
}

const CLHEP::Hep3Vector BKLMGeometryPar::getSupportPlateSize(bool flag) const
{
  CLHEP::Hep3Vector size;
  size.setX(m_SupportPlateHeight / 2.0);
  size.setY(m_SupportPlateWidth / 2.0);
  if (flag) {
    size.setZ((m_ModuleLengthChimney + 1.6) / 2.0);
  } else {
    size.setZ(m_GapLength / 2.0);
  }
  return size;
}

const BKLMSector* BKLMGeometryPar::findSector(int frontBack, int sector) const
{
  std::vector<BKLMSector*>::const_iterator iS;
  for (iS = m_Sectors.begin(); iS != m_Sectors.end(); ++iS) {
    if (((*iS)->getFrontBack() == frontBack) && ((*iS)->getSector() == sector)) {
      break;
    }
  }
  return (*iS);
}
