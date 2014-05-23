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
#include <bklm/dataobjects/BKLMStatus.h>

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
      B2FATAL("getInstance() called without initialization")
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

      m_NZStrips = content.getInt("Layers/NZStrips");
      m_NZStripsChimney = content.getInt("Layers/NZStripsChimney");
      m_NZScints = content.getInt("Layers/NZScintillators");
      m_NZScintsChimney = content.getInt("Layers/NZScintillatorsChimney");

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

      char name[40] = { '\0' };
      for (int fb = 1; fb <= 2; ++fb) {
        bool isForward = (fb == 1);
        for (int sector = 1; sector <= m_NSector; ++sector) {
          bool hasChimney = (!isForward) && (sector == 3);
          int nZStrips = (hasChimney ? m_NZStripsChimney : m_NZStrips);
          int nZScints = (hasChimney ? m_NZScintsChimney : m_NZScints);
          sprintf(name, "/Sectors/%s/Sector[@sector=\"%d\"]", (isForward ? "Forward" : "Backward"), sector);
          GearDir sectorContent(content);
          sectorContent.append(name);
          HepRotation rotation;
          if (!isForward) rotation.rotateX(M_PI);
          rotation.rotateZ(sectorContent.getAngle("/Phi"));
          for (int layer = 1; layer <= m_NLayer; ++layer) {
            GearDir layerContent(sectorContent);
            sprintf(name, "/Layer[@layer=\"%d\"]", layer);
            layerContent.append(name);
            Hep3Vector localReconstructionShift(layerContent.getLength("ReconstructionShift/X"),
                                                layerContent.getLength("ReconstructionShift/Y"),
                                                layerContent.getLength("ReconstructionShift/Z"));
            sprintf(name, "/Layers/Layer[@layer=\"%d\"]", layer);
            layerContent = content;
            layerContent.append(name);
            m_HasRPCs[layer] = layerContent.getBool("HasRPCs");
            m_NPhiScints[layer] = layerContent.getInt("PhiScintillators/NScints", 0);
            m_PhiScintsOffsetSign[layer] = layerContent.getInt("PhiScintillators/OffsetSign", 1.0);
            m_ZScintsOffsetSign[layer] = layerContent.getInt("ZScintillators/OffsetSign", 1.0);
            Hep3Vector localOrigin(getActiveMiddleRadius(layer), 0.0, m_ModuleFrameThickness);
            int moduleID = (isForward ? BKLM_END_MASK : 0)
                           | ((sector - 1) << BKLM_SECTOR_BIT)
                           | ((layer - 1) << BKLM_LAYER_BIT);
            if (m_HasRPCs[layer]) {
              int phiStripNumber = layerContent.getInt("PhiStrips/NStrips");
              localOrigin.setZ(localOrigin.z() + m_ModuleGasSpacerWidth);
              Module* pModule = new Module(layerContent.getLength("PhiStrips/Width"),
                                           (layer == 1 ? 2 : 1),
                                           (layer == 1 ? phiStripNumber - 1 : phiStripNumber),
                                           layerContent.getLength("ZStrips/Width"),
                                           nZStrips,
                                           Hep3Vector(0.0, 0.0, m_OffsetZ) + rotation(localOrigin),
                                           localReconstructionShift,
                                           rotation
                                          );
              m_Modules.insert(std::pair<int, Module*>(moduleID, pModule));
            } else {
              localOrigin.setY(localOrigin.y() + getScintEnvelopeOffset(layer, hasChimney).y());
              Module* pModule = new Module(m_ScintWidth,
                                           m_NPhiScints[layer],
                                           nZScints,
                                           Hep3Vector(0.0, 0.0, m_OffsetZ) + rotation(localOrigin),
                                           localReconstructionShift,
                                           rotation
                                          );
              m_Modules.insert(std::pair<int, Module*>(moduleID, pModule));
              double base = -0.5 * (m_NPhiScints[layer] + 1) * m_ScintWidth;
              for (int scint = 1; scint <= m_NPhiScints[layer]; ++scint) {
                sprintf(name, "/PhiScintillators/Scint[@scint=\"%d\"]", scint);
                GearDir scintContent(layerContent);
                scintContent.append(name);
                double dLength = scintContent.getLength("DLength", 0.0);
                pModule->addPhiScint(scint,
                                     nZScints * m_ScintWidth + dLength,
                                     0.5 * fabs(dLength) * scintContent.getInt("OffsetSign", 0),
                                     base + scint * m_ScintWidth
                                    );
              }
              base = -0.5 * (nZScints + 1) * m_ScintWidth;
              for (int scint = 1; scint <= nZScints; ++scint) {
                sprintf(name, "/ZScintillators/Scint[@scint=\"%d\"]", scint);
                GearDir scintContent(layerContent);
                scintContent.append(name);
                double dLength = scintContent.getLength("DLength", 0.0);
                pModule->addZScint(scint,
                                   m_NPhiScints[layer] * m_ScintWidth + dLength,
                                   0.5 * fabs(dLength) * scintContent.getInt("OffsetSign", 0),
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

    const Hep3Vector GeometryPar::getGapHalfSize(int layer, bool hasChimney) const
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
      return Hep3Vector(dx, r * tan(M_PI / m_NSector) - ds, dz);

    }

    const Hep3Vector GeometryPar::getModuleHalfSize(int layer, bool hasChimney) const
    {
      Hep3Vector size = getGapHalfSize(layer, hasChimney);
      size.setX(0.5 * m_ModuleHeight);
      size.setZ(0.5 * (hasChimney ? m_ModuleLengthChimney : m_ModuleLength));
      return size;
    }

    const Hep3Vector GeometryPar::getModuleInteriorHalfSize1(int layer, bool hasChimney) const
    {
      Hep3Vector size = getModuleHalfSize(layer, hasChimney);
      size.setX(m_ModuleReadoutHeight + 2.0 * m_ModuleGlassHeight + m_ModuleGasHeight);
      size.setY(size.y() - m_ModuleFrameWidth);
      size.setZ(size.z() - m_ModuleFrameWidth);
      return size;
    }

    const Hep3Vector GeometryPar::getModuleInteriorHalfSize2(int layer, bool hasChimney) const
    {
      Hep3Vector size = getModuleHalfSize(layer, hasChimney);
      size.setX(size.x() - m_ModuleFrameThickness - m_ModuleCoverHeight);
      size.setY(size.y() - m_ModuleFrameThickness);
      size.setZ(size.z() - m_ModuleFrameThickness);
      return size;
    }

    const Hep3Vector GeometryPar::getElectrodeHalfSize(int layer, bool hasChimney) const
    {
      Hep3Vector size = getModuleInteriorHalfSize1(layer, hasChimney);
      size.setX(2.0 * m_ModuleGlassHeight + m_ModuleGasHeight);
      return size;
    }

    const Hep3Vector GeometryPar::getGasHalfSize(int layer, bool hasChimney) const
    {
      Hep3Vector size = getElectrodeHalfSize(layer, hasChimney);
      size.setX(0.5 * m_ModuleGasHeight);
      size.setY(size.y() - m_ModuleGasSpacerWidth);
      size.setZ(size.z() - m_ModuleGasSpacerWidth);
      return size;
    }

    const Hep3Vector GeometryPar::getAirHalfSize(int layer, bool hasChimney) const
    {
      Hep3Vector size = getModuleInteriorHalfSize2(layer, hasChimney);
      size.setX(m_ScintHeight);
      return size;
    }

    const Hep3Vector GeometryPar::getScintEnvelopeHalfSize(int layer, bool hasChimney) const
    {
      return Hep3Vector(0.5 * m_ScintHeight,
                        0.5 * m_ScintWidth * getNPhiScints(layer),
                        0.5 * m_ScintWidth * getNZScints(hasChimney)
                       );
    }

    int GeometryPar::getNPhiScints(int layer) const
    {
      if ((layer <= 0) || (layer > m_NLayer)) return 0;
      return m_NPhiScints[layer];
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
        return m_Gap1InnerRadius + 0.5 * m_Gap1ActualHeight;
      }
      return m_GapInnerRadius + 0.5 * m_GapNominalHeight + m_LayerHeight * (layer - 1);
    }

    double GeometryPar::getActiveMiddleRadius(int layer) const
    {
      // place the active radius at the midplane of the innermost sensitive volume
      // (same as inner-plane positioning in GeoBKLMCreator.cc)
      double r = getModuleMiddleRadius(layer);
      if (hasRPCs(layer)) {
        r -= (getModuleGlassHeight() + 0.5 * getModuleGasHeight());
      } else {
        r -= (0.5 * m_ScintHeight - getPolystyreneOffsetX());
      }
      return r;
    }

    const Hep3Vector GeometryPar::getScintEnvelopeOffset(int layer, bool hasChimney) const
    {
      Hep3Vector airHalfSize = getAirHalfSize(layer, hasChimney);
      Hep3Vector envelopeHalfSize = getScintEnvelopeHalfSize(layer, hasChimney);
      Hep3Vector offset((airHalfSize.x() - envelopeHalfSize.x()),
                        (airHalfSize.y() - envelopeHalfSize.y()) * m_PhiScintsOffsetSign[layer],
                        (airHalfSize.z() - envelopeHalfSize.z()) * m_ZScintsOffsetSign[layer]);
      return offset;
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

    const Hep3Vector GeometryPar::getSupportPlateHalfSize(bool hasChimney) const
    {
      Hep3Vector size;
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
      if (bracket == 0) return z;
      if (hasChimney) {
        return m_SupportPlateLengthChimney - m_GapLength - z;
      } else {
        return (bracket == 1 ? 0.0 : -z);
      }
    }

    bool GeometryPar::hasRPCs(int layer) const
    {
      if ((layer <= 0) || (layer > m_NLayer)) return false;
      return m_HasRPCs[layer];
    }

    const Module* GeometryPar::findModule(bool isForward, int sector, int layer) const
    {
      int moduleID = (isForward ? BKLM_END_MASK : 0)
                     | ((sector - 1) << BKLM_SECTOR_BIT)
                     | ((layer - 1) << BKLM_LAYER_BIT);
      map<int, Module*>::const_iterator iM = m_Modules.find(moduleID);
      return (iM == m_Modules.end() ? NULL : iM->second);
    }

    const Module* GeometryPar::findModule(int layer, bool hasChimney) const
    {
      int moduleID = ((layer - 1) << BKLM_LAYER_BIT);
      if (hasChimney) { // Chimney module is in backward sector 3
        moduleID |= ((3 - 1) << BKLM_SECTOR_BIT);
      }
      map<int, Module*>::const_iterator iM = m_Modules.find(moduleID);
      return (iM == m_Modules.end() ? NULL : iM->second);
    }


  } // end of namespace bklm

} // end of namespace Belle2
