/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/geometry/GeometryPar.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace CLHEP;

namespace Belle2 {

  namespace bklm {

    GeometryPar* GeometryPar::m_Instance = NULL;

    GeometryPar* GeometryPar::instance(void)
    {
      if (m_Instance) return m_Instance;
      B2FATAL("GeometryPar::getInstance() called without initialization")
      return NULL;  // never reached
    }

    GeometryPar* GeometryPar::instance(const GearDir& content)
    {
      if (!m_Instance) m_Instance = new GeometryPar(content);
      return m_Instance;
    }

    GeometryPar::GeometryPar(const GearDir& content)
    {
      clear();
      read(content);
    }

    GeometryPar::~GeometryPar()
    {
    }

    void GeometryPar::clear()
    {
    }

    void GeometryPar::read(const GearDir& content)
    {
      // Get Gearbox parameters for BKLM

      m_DoOverlapCheck = content.getInt("DoOverlapCheck");
      m_Rotation = content.getAngle("Rotation");
      m_OffsetZ = content.getLength("OffsetZ");
      m_Phi = content.getLength("Phi");
      m_NSector = content.getNumberNodes("Sectors/Forward/Sector");
      m_SolenoidOuterRadius = content.getLength("SolenoidOuterRadius");
      m_OuterRadius = content.getLength("OuterRadius");
      m_HalfLength = content.getLength("HalfLength");
      m_NLayer = content.getNumberNodes("Layers/Layer");
      if (m_NLayer > NLAYER) { // for array m_HasRPCs[]
        B2FATAL("BKLM GeometryPar::read(): # of layers (" << m_NLayer << ") exceeds array size NLAYER (" << NLAYER << ")")
      }

      m_IronNominalHeight = content.getLength("Layers/IronNominalHeight");
      m_IronActualHeight = content.getLength("Layers/IronActualHeight");

      m_Gap1NominalHeight = content.getLength("Layers/Layer[@layer=\"1\"]/GapNominalHeight");
      m_Gap1ActualHeight = m_Gap1NominalHeight + (m_IronNominalHeight - m_IronActualHeight) / 2.0;

      m_GapNominalHeight = content.getLength("Layers/GapNominalHeight");
      m_GapActualHeight = m_GapNominalHeight + (m_IronNominalHeight - m_IronActualHeight);

      m_Layer1Height = m_IronNominalHeight + m_Gap1NominalHeight;
      m_LayerHeight = m_IronNominalHeight + m_GapNominalHeight;

      m_Gap1InnerRadius = content.getLength("Layers/InnerRadius");
      m_GapInnerRadius = m_Gap1InnerRadius + m_Layer1Height - m_LayerHeight;

      m_Gap1IronWidth = content.getLength("Layers/Layer[@layer=\"1\"]/GapIronWidth");
      m_GapIronWidth = content.getLength("Layers/GapIronWidth");

      m_GapLength = content.getLength("Layers/GapLength");

      m_ModuleLength = content.getLength("Module/Length");
      m_ModuleLengthChimney = content.getLength("Module/LengthChimney");
      m_ModuleCoverHeight = content.getLength("Module/CoverHeight");
      m_ModuleCopperHeight = content.getLength("Module/CopperHeight");
      m_ModuleFoamHeight = content.getLength("Module/FoamHeight");
      m_ModuleMylarHeight = content.getLength("Module/MylarHeight");
      m_ModuleGlassHeight = content.getLength("Module/GlassHeight");
      m_ModuleGasHeight = content.getLength("Module/GasHeight");
      m_ModuleReadoutHeight = m_ModuleFoamHeight + (m_ModuleCopperHeight + m_ModuleMylarHeight) * 2.0;
      m_ModuleHeight = (m_ModuleCoverHeight + m_ModuleReadoutHeight + m_ModuleGasHeight + m_ModuleGlassHeight * 2.0) * 2.0;
      m_ModuleFrameWidth = content.getLength("Module/FrameWidth");
      m_ModuleGasSpacerWidth = content.getLength("Module/SpacerWidth");
      m_ModuleElectrodeBorder = content.getLength(" Module/ElectrodeBorder");

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

      m_HasRPCs[0] = false;
      double phiStripWidth[NLAYER + 1] = { 0.0 };
      double phiStripLength[NLAYER + 1] = { 0.0 };
      int    phiStripNumber[NLAYER + 1] = { 0 };
      int    phiStripMin[NLAYER + 1] = { 0 };
      int    phiStripMax[NLAYER + 1] = { 0 };
      double zStripWidth[NLAYER + 1] = { 0.0 };
      double zStripLength[NLAYER + 1] = { 0.0 };
      int    zStripNumber[NLAYER + 1] = { 0 };
      char name[40] = { '\0' };
      for (int layer = 1; layer <= m_NLayer; ++layer) {
        sprintf(name, "/Layers/Layer[@layer=\"%d\"]", layer);
        GearDir layerContent(content);
        layerContent.append(name);
        m_HasRPCs[layer] = layerContent.getInt("HasRPCs");
        phiStripWidth[layer] = layerContent.getLength("PhiStrips/Width");
        phiStripLength[layer] = layerContent.getLength("PhiStrips/Length");
        phiStripNumber[layer] = layerContent.getInt("PhiStrips/NStrips");
        phiStripMin[layer] = (layer == 1 ? 2 : 1);
        phiStripMax[layer] = (layer == 1 ? phiStripNumber[layer] - 1 : phiStripNumber[layer]);
        zStripWidth[layer] = layerContent.getLength("ZStrips/Width");
        zStripLength[layer] = layerContent.getLength("ZStrips/Length");
        zStripNumber[layer] = layerContent.getInt("ZStrips/NStrips");
      }

      Hep3Vector rSector(m_GapInnerRadius + 0.5 * m_GapNominalHeight, 0.0, 0.0);
      for (int fb = 0; fb < 2; ++fb) {
        bool isForward = (fb == 0);
        Hep3Vector deltaZ(0.0, 0.0, (isForward ? m_ModuleElectrodeBorder : -m_ModuleElectrodeBorder));
        for (int sector = 1; sector <= m_NSector; ++sector) {
          sprintf(name, "/Sectors/%s/Sector[@sector=\"%d\"]", (isForward ? "Forward" : "Backward"), sector);
          GearDir sectorContent(content);
          sectorContent.append(name);
          Hep3Vector shift(sectorContent.getLength("Shift/X"),
                           sectorContent.getLength("Shift/Y"),
                           sectorContent.getLength("Shift/Z"));
          HepRotation rotation;
          if (~isForward) rotation.rotateX(M_PI);
          rotation.rotateZ(sectorContent.getAngle("/Phi"));
          Hep3Vector translation(0.0, 0.0, m_OffsetZ);
          translation += rotation(rSector) + deltaZ;
          Sector* pSector = new Sector(isForward, sector, m_NLayer, shift, translation, rotation);
          m_Sectors.push_back(pSector);
          for (int layer = 1; layer <= m_NLayer; ++layer) {
            int zStripMin = 1;
            int zStripMax = zStripNumber[layer];
            if ((!isForward) && (sector == 3)) zStripMax = sectorContent.getInt("ZStrips/NStrips");
            GearDir layerContent(sectorContent);
            sprintf(name, "/Layer[@layer=\"%d\"]", layer);
            layerContent.append(name);
            Hep3Vector lshift(layerContent.getLength("Shift/X"),
                              layerContent.getLength("Shift/Y"),
                              layerContent.getLength("Shift/Z"));
            double dr = (layer == 1 ? (m_GapNominalHeight - m_Gap1NominalHeight) * 0.5 : (layer - 1) * m_LayerHeight);
            pSector->addModule(new Module(isForward, sector, layer, lshift, dr, pSector,
                                          phiStripWidth[layer], phiStripLength[layer],
                                          phiStripNumber[layer], phiStripMin[layer], phiStripMax[layer],
                                          zStripWidth[layer], zStripLength[layer],
                                          zStripNumber[layer], zStripMin, zStripMax));
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

    const Hep3Vector GeometryPar::getGapHalfSize(int layer, bool flag) const
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
      double dz = 0.5 * (flag ? m_GapLength - m_ChimneyLength : m_GapLength);
      return Hep3Vector(dx, r * tan(M_PI / m_NSector) - ds, dz);

    }

    const Hep3Vector GeometryPar::getModuleHalfSize(int layer, bool flag) const
    {
      Hep3Vector size = getGapHalfSize(layer, flag);
      size.setX(0.5 * m_ModuleHeight);
      size.setZ(0.5 * (flag ? m_ModuleLengthChimney : m_ModuleLength));
      return size;
    }

    const Hep3Vector GeometryPar::getReadoutHalfSize(int layer, bool flag) const
    {
      Hep3Vector size = getModuleHalfSize(layer, flag);
      size.setX(m_ModuleReadoutHeight + 2.0 * m_ModuleGlassHeight + m_ModuleGasHeight);
      size.setY(size.y() - m_ModuleFrameWidth);
      size.setZ(size.z() - m_ModuleFrameWidth);
      return size;
    }

    const Hep3Vector GeometryPar::getElectrodeHalfSize(int layer, bool flag) const
    {
      Hep3Vector size = getReadoutHalfSize(layer, flag);
      size.setX(2.0 * m_ModuleGlassHeight + m_ModuleGasHeight);
      return size;
    }

    const Hep3Vector GeometryPar::getGasHalfSize(int layer, bool flag) const
    {
      Hep3Vector size = getElectrodeHalfSize(layer, flag);
      size.setX(0.5 * m_ModuleGasHeight);
      size.setY(size.y() - m_ModuleGasSpacerWidth);
      size.setZ(size.z() - m_ModuleGasSpacerWidth);
      return size;
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
        return m_Gap1InnerRadius + 0.5 * m_Gap1ActualHeight;
      }
      return m_GapInnerRadius + 0.5 * m_GapNominalHeight + m_LayerHeight * (layer - 1);
    }

    const Hep3Vector GeometryPar::getChimneyHalfSize(int layer) const
    {
      return Hep3Vector(0.5 * (getLayerOuterRadius(layer) - getLayerInnerRadius(layer)),
                        0.5 * m_ChimneyWidth,
                        0.5 * (m_ChimneyLength - m_ChimneyCoverThickness));
    }

    const Hep3Vector GeometryPar::getChimneyPosition(int layer) const
    {
      return Hep3Vector(0.5 * (getLayerOuterRadius(layer) + getLayerInnerRadius(layer)),
                        0.0,
                        0.5 * (m_GapLength - m_ChimneyLength - m_ChimneyCoverThickness));
    }

    const Hep3Vector GeometryPar::getSupportPlateHalfSize(bool flag) const
    {
      Hep3Vector size;
      size.setX(0.5 * m_SupportPlateHeight);
      size.setY(0.5 * m_SupportPlateWidth);
      if (flag) {
        size.setZ(0.5 * m_SupportPlateLengthChimney);
      } else {
        size.setZ(0.5 * m_SupportPlateLength);
      }
      return size;
    }

    double GeometryPar::getBracketZPosition(int bracket, bool flag) const
    {
      double z = m_BracketInset - 0.5 * m_GapLength;
      if (bracket == 0) return z;
      if (flag) {
        return m_SupportPlateLengthChimney - m_GapLength - z;
      } else {
        return (bracket == 1 ? 0.0 : -z);
      }
    }

    bool GeometryPar::hasRPCs(int layer) const
    {
      if ((layer <= 0) || (layer > m_NLayer)) return true;
      return m_HasRPCs[layer];
    }

    const Sector* GeometryPar::findSector(bool isForward, int sector) const
    {
      vector<Sector*>::const_iterator iS;
      for (iS = m_Sectors.begin(); iS != m_Sectors.end(); ++iS) {
        if (((*iS)->isForward() == isForward) && ((*iS)->getSector() == sector)) {
          break;
        }
      }
      return (*iS);
    }
  } // end of namespace bklm

} // end of namespace Belle2
