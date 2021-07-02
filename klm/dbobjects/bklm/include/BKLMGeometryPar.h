/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>

/* ROOT headers. */
#include <TObject.h>

/* C++ headers. */
#include <string>
#include <vector>

static const int NSCINTLAYER = 2;
static const int NPHISCINT = 48;
static const int NZSCINT = 54;
static const int NSTATION = 7;
static const int BKLM_INNER = 1;
static const int BKLM_OUTER = 2;

namespace Belle2 {

  class GearDir;

  /**
  * The Class for BKLM geometry
  */

  class BKLMGeometryPar: public TObject {

  public:

    //! Default constructor
    BKLMGeometryPar()
    {
    }

    //! Constructor using Gearbox
    explicit BKLMGeometryPar(const GearDir&);

    //! Destructor
    ~BKLMGeometryPar();

    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

    //! Get the beam background study flag
    bool doBeamBackgroundStudy(void) const
    {
      return m_DoBeamBackgroundStudy;
    }

    //! Set the beam background study flag
    void doBeamBackgroundStudy(bool flag)
    {
      m_DoBeamBackgroundStudy = flag;
    }

    //! Determine if the sensitive detectors in a given layer are RPCs (=true) or scintillators (=false)
    bool hasRPCs(int layer) const
    {
      return m_HasRPCs[layer - 1];
    }

    //! Set flag to indicate whether layer contains RPCs (true) or scintillators (false)
    void hasRPCs(int layer, bool flag)
    {
      m_HasRPCs[layer] = flag;
    }

    //! Get the thickness of the inactive TiO2-polystyrene coating on top (broad) surface of a scintillator strip
    double getScintTiO2ThicknessTop(void) const
    {
      return m_ScintTiO2ThicknessTop;
    }

    //! Set the thickness of the inactive TiO2-polystyrene coating on top (broad) surface of a scintillator strip
    void setScintTiO2ThicknessTop(double x)
    {
      m_ScintTiO2ThicknessTop = x;
    }

    //! Get the thickness of the inactive TiO2-polystyrene coating on side (short) surface of a scintillator strip
    double getScintTiO2ThicknessSide(void) const
    {
      return m_ScintTiO2ThicknessSide;
    }

    //! Set the thickness of the inactive TiO2-polystyrene coating on side (short) surface of a scintillator strip
    void setScintTiO2ThicknessSide(double x)
    {
      m_ScintTiO2ThicknessSide = x;
    }

    //! Get length along z of the chimney hole
    double getChimneyLength(void) const
    {
      return m_ChimneyLength;
    }

    //! Set length along z of the chimney hole
    void setChimneyLength(double x)
    {
      m_ChimneyLength = x;
    }

    //! Get width of the chimney hole
    double getChimneyWidth(void) const
    {
      return m_ChimneyWidth;
    }

    //! Set width of the chimney hole
    void setChimneyWidth(double x)
    {
      m_ChimneyWidth = x;
    }

    //! Get the height of the entire volume of a scintillator strip (including TiO2 coating)
    double getScintHeight(void) const
    {
      return  m_ScintHeight;
    }

    //! Set the height of the entire volume of a scintillator strip (including TiO2 coating)
    void setScintHeight(double x)
    {
      m_ScintHeight = x;
    }

    //! Get the width of the entire volume of a scintillator strip (including TiO2 coating)
    double getScintWidth(void) const
    {
      return m_ScintWidth;
    }

    //! Set the width of the entire volume of a scintillator strip (including TiO2 coating)
    void setScintWidth(double x)
    {
      m_ScintWidth = x;
    }

    //! Get the radius of the cylindrical central bore in a scintillator strip
    double getScintBoreRadius(void) const
    {
      return m_ScintBoreRadius;
    }

    //! Set the radius of the cylindrical central bore in a scintillator strip
    void setScintBoreRadius(double x)
    {
      m_ScintBoreRadius = x;
    }

    //! Get the radius of the cylindrical central WLS fiber in a scintillator strip
    double getScintFiberRadius(void) const
    {
      return m_ScintFiberRadius;
    }

    //! Set the radius of the cylindrical central WLS fiber in a scintillator strip
    void setScintFiberRadius(double x)
    {
      m_ScintFiberRadius = x;
    }

    //! Get the global rotation angle about z of the entire BKLM
    double getRotation(void) const
    {
      return m_Rotation;
    }

    //! Set the global rotation angle about z of the entire BKLM
    void setRotation(double x)
    {
      m_Rotation = x;
    }

    //! Get the rotation angle for a sector
    double getSectorRotation(int section, int sector) const
    {
      return m_SectorRotation[section][sector - 1];
    }

    //! Set the rotation angle for a sector
    void setSectorRotation(int section, int sector, double x)
    {
      m_SectorRotation[section][sector - 1] = x;
    }

    //! Get the global shift along a of the entire BKLM
    double getOffsetZ(void) const
    {
      return m_OffsetZ;
    }

    //! Set the global shift along a of the entire BKLM
    void setOffsetZ(double x)
    {
      m_OffsetZ = x;
    }

    //! Get the starting angle of the BKLM's polygon shape
    double getPhi(void) const
    {
      return m_Phi;
    }

    //! Set the starting angle of the BKLM's polygon shape
    void setPhi(double x)
    {
      m_Phi = x;
    }

    //! Get the outer radius of the solenoid
    double getSolenoidOuterRadius(void) const
    {
      return m_SolenoidOuterRadius;
    }

    //! Set the outer radius of the solenoid
    void setSolenoidOuterRadius(double x)
    {
      m_SolenoidOuterRadius = x;
    }

    //! Get the number of sectors of the BKLM
    int getNSector(void) const
    {
      return m_NSector;
    }

    //! Set the number of sectors of the BKLM
    void setNSector(int n)
    {
      m_NSector = n;
    }

    //! Get the half-length along z of the BKLM
    double getHalfLength(void) const
    {
      return m_HalfLength;
    }

    //! Set the half-length along z of the BKLM
    void setHalfLength(double x)
    {
      m_HalfLength = x;
    }

    //! Get the radius of the inscribed circle of the outer polygon
    double getOuterRadius(void) const
    {
      return m_OuterRadius;
    }

    //! Set the radius of the inscribed circle of the outer polygon
    void setOuterRadius(double x)
    {
      m_OuterRadius = x;
    }

    //! Get the number of modules in one sector
    int getNLayer(void) const
    {
      return m_NLayer;
    }

    //! Set the number of modules in one sector
    void setNLayer(double n)
    {
      m_NLayer = n;
    }

    //! Get the nominal height of a layer's structural iron
    double getIronNominalHeight(void) const
    {
      return m_IronNominalHeight;
    }

    //! Set the nominal height of a layer's structural iron
    void setIronNominalHeight(double x)
    {
      m_IronNominalHeight = x;
    }

    //! Get the actual height of a layer's structural iron
    double getIronActualHeight(void) const
    {
      return m_IronActualHeight;
    }

    //! Set the actual height of a layer's structural iron
    void setIronActualHeight(double x)
    {
      m_IronActualHeight = x;
    }

    //! Get the radius of the inner tangent circle of gap 0 (innermost)
    double getGap1InnerRadius(void) const
    {
      return m_Gap1InnerRadius;
    }

    //! Set the radius of the inner tangent circle of gap 0 (innermost)
    void setGap1InnerRadius(double x)
    {
      m_Gap1InnerRadius = x;
    }

    //! Get the nominal height of the innermost gap
    double getGap1NominalHeight(void) const
    {
      return m_Gap1NominalHeight;
    }

    //! Set the nominal height of the innermost gap
    void setGap1NominalHeight(double x)
    {
      m_Gap1NominalHeight = x;
    }

    //! Get the width (at the outer radius) of the adjacent structural iron on either side of innermost gap
    double getGap1IronWidth(void) const
    {
      return m_Gap1IronWidth;
    }

    //! Set the width (at the outer radius) of the adjacent structural iron on either side of innermost gap
    void setGap1IronWidth(double x)
    {
      m_Gap1IronWidth = x;
    }

    //! Get the length along z of the module gap
    double getGapLength(void) const
    {
      return m_GapLength;
    }

    //! Set the length along z of the module gap
    void setGapLength(double x)
    {
      m_GapLength = x;
    }

    //! Get the nominal height of the outer gaps
    double getGapNominalHeight(void) const
    {
      return m_GapNominalHeight;
    }

    //! Set the nominal height of the outer gaps
    void setGapNominalHeight(double x)
    {
      m_GapNominalHeight = x;
    }

    //! Get the width (at the outer radius) of the adjacent structural iron on either side of a gap
    double getGapIronWidth(void) const
    {
      return m_GapIronWidth;
    }

    //! Set the width (at the outer radius) of the adjacent structural iron on either side of a gap
    void setGapIronWidth(double x)
    {
      m_GapIronWidth = x;
    }

    //! Get the number of z-measuring cathode strips in an RPC module (no chimney)
    int getNZStrips(void) const
    {
      return m_NZStrips;
    }

    //! Set the number of z-measuring cathode strips in an RPC module (no chimney)
    void setNZStrips(int n)
    {
      m_NZStrips = n;
    }

    //! Get the number of z-measuring cathode strips in an RPC module (with chimney)
    int getNZStripsChimney(void) const
    {
      return m_NZStripsChimney;
    }

    //! Set the number of z-measuring cathode strips in an RPC module (with chimney)
    void setNZStripsChimney(int n)
    {
      m_NZStripsChimney = n;
    }

    //! Get the number of phi-measuring cathode strips in an RPC module
    int getNPhiStrips(int layer) const
    {
      return m_NPhiStrips[layer - 1];
    }

    //! Set the number of phi-measuring cathode strips in an RPC module
    void setNPhiStrips(int layer, int n)
    {
      m_NPhiStrips[layer - 1] = n;
    }

    //! Get the number of z-measuring scintillators in a scintillator module (no chimney)
    int getNZScints(void) const
    {
      return m_NZScints;
    }

    //! Set the number of z-measuring scintillators in a scintillator module (no chimney)
    void setNZScints(int n)
    {
      m_NZScints = n;
    }

    //! Get the number of z-measuring scintillators in a scintillator module (with chimney)
    int getNZScintsChimney(void) const
    {
      return m_NZScintsChimney;
    }

    //! Set the number of z-measuring scintillators in a scintillator module (with chimney)
    void setNZScintsChimney(int n)
    {
      m_NZScintsChimney = n;
    }

    //! Get the number of phi-measuring scintillators in a scintillator module
    int getNPhiScints(int layer) const
    {
      return m_NPhiScints[layer - 1];
    }

    //! Set the number of phi-measuring scintillators in an scintillator module
    void setNPhiScints(int layer, int n)
    {
      m_NPhiScints[layer - 1] = n;
    }

    //! Get the sign of shift of scintillator envelope along local y axis (-1: MPPCs on left, +1: MPPCs on right)
    //! *for scintillator layers only*
    double getScintEnvelopeOffsetSign(int layer) const
    {
      return m_ScintEnvelopeOffsetSign[layer - 1];
    }

    //! Set the sign of shift of scintillator envelope along local y axis (-1: MPPCs on left, +1: MPPCs on right)
    //! *for scintillator layers only*
    void setScintEnvelopeOffsetSign(int layer, int n)
    {
      m_ScintEnvelopeOffsetSign[layer - 1] = n;
    }

    //! Get width of the phi strips on each layer
    double getPhiStripWidth(int layer) const
    {
      return m_PhiStripWidth[layer - 1];
    }

    //! Set width of the phi strips on each layer
    void setPhiStripWidth(int layer, double x)
    {
      m_PhiStripWidth[layer - 1] = x;
    }

    //! Get width of the z strips on each layer
    double getZStripWidth(int layer) const
    {
      return m_ZStripWidth[layer - 1];
    }

    //! Set width of the z strips on each layer
    void setZStripWidth(int layer, double x)
    {
      m_ZStripWidth[layer - 1] = x;
    }

    //! Get shortening of nominal length of the z scintillators
    //! *for scintillator layers only*
    double getZScintDLength(int layer, int scint) const
    {
      return m_ZScintDLength[layer - 1][scint - 1];
    }

    //! Set shortening of nominal length of the z scintillators
    //! *for scintillator layers only*
    void setZScintDLength(int layer, int scint, double x)
    {
      m_ZScintDLength[layer - 1][scint - 1] = x;
    }

    //! Get the length along z of the module
    double getModuleLength(void) const
    {
      return m_ModuleLength;
    }

    //! Set the length along z of the module
    void setModuleLength(double x)
    {
      m_ModuleLength = x;
    }

    //! Get the length along z of the module
    double getModuleLengthChimney(void) const
    {
      return m_ModuleLengthChimney;
    }

    //! Set the length along z of the module
    void setModuleLengthChimney(double x)
    {
      m_ModuleLengthChimney = x;
    }

    //! Get the height of the module's aluminum cover (2 per module)
    double getModuleCoverHeight(void) const
    {
      return m_ModuleCoverHeight;
    }

    //! Set the height of the module's aluminum cover (2 per module)
    void setModuleCoverHeight(double x)
    {
      m_ModuleCoverHeight = x;
    }

    //! Get the height of the module's readout or ground copper plane (4 per module)
    double getModuleCopperHeight(void) const
    {
      return m_ModuleCopperHeight;
    }

    //! Set the height of the module's readout or ground copper plane (4 per module)
    void setModuleCopperHeight(double x)
    {
      m_ModuleCopperHeight = x;
    }

    //! Get the height of the module's transmission-line foam (2 per module)
    double getModuleFoamHeight(void) const
    {
      return m_ModuleFoamHeight;
    }

    //! Set the height of the module's transmission-line foam (2 per module)
    void setModuleFoamHeight(double x)
    {
      m_ModuleFoamHeight = x;
    }

    //! Get the height of the module's insulating mylar (2 per module)
    double getModuleMylarHeight(void) const
    {
      return m_ModuleMylarHeight;
    }

    //! Set the height of the module's insulating mylar (2 per module)
    void setModuleMylarHeight(double x)
    {
      m_ModuleMylarHeight = x;
    }

    //! Get the height of the module's glass electrode (4 per module)
    double getModuleGlassHeight(void) const
    {
      return m_ModuleGlassHeight;
    }

    //! Set the height of the module's glass electrode (4 per module)
    void setModuleGlassHeight(double moduleGlassHeight)
    {
      m_ModuleGlassHeight = moduleGlassHeight;
    }

    //! Get the height of the module's gas gap (2 per module)
    double getModuleGasHeight(void) const
    {
      return m_ModuleGasHeight;
    }

    //! Set the height of the module's gas gap (2 per module)
    void setModuleGasHeight(double x)
    {
      m_ModuleGasHeight = x;
    }

    //! Get the width of the module's perimeter aluminum frame
    double getModuleFrameWidth(void) const
    {
      return m_ModuleFrameWidth;
    }

    //! Set the width of the module's perimeter aluminum frame
    void setModuleFrameWidth(double x)
    {
      m_ModuleFrameWidth = x;
    }

    //! Get the thickness of the module's perimeter aluminum frame
    double getModuleFrameThickness(void) const
    {
      return m_ModuleFrameThickness;
    }

    //! Set the thickness of the module's perimeter aluminum frame
    void setModuleFrameThickness(double x)
    {
      m_ModuleFrameThickness = x;
    }

    //! Get the width of the module's gas-gap's perimeter spacer
    double getModuleGasSpacerWidth(void) const
    {
      return m_ModuleGasSpacerWidth;
    }

    //! Set the width of the module's gas-gap's perimeter spacer
    void setModuleGasSpacerWidth(double x)
    {
      m_ModuleGasSpacerWidth = x;
    }

    //! Get the height of the inner polystyrene-filler sheet
    double getModulePolystyreneInnerHeight(void) const
    {
      return m_ModulePolystyreneInnerHeight;
    }

    //! Set the height of the inner polystyrene-filler sheet
    void setModulePolystyreneInnerHeight(double x)
    {
      m_ModulePolystyreneInnerHeight = x;
    }

    //! Get the height of the outer polystyrene-filler sheet
    double getModulePolystyreneOuterHeight(void) const
    {
      return m_ModulePolystyreneOuterHeight;
    }

    //! Set the height of the outer polystyrene-filler sheet
    void setModulePolystyreneOuterHeight(double x)
    {
      m_ModulePolystyreneOuterHeight = x;
    }

    //! Get the thickness of the chimney cover plate
    double getChimneyCoverThickness(void) const
    {
      return m_ChimneyCoverThickness;
    }

    //! Set the thickness of the chimney cover plate
    void setChimneyCoverThickness(double x)
    {
      m_ChimneyCoverThickness = x;
    }

    //! Get the inner radius of the chimney housing
    double getChimneyHousingInnerRadius(void) const
    {
      return m_ChimneyHousingInnerRadius;
    }

    //! Set the inner radius of the chimney housing
    void setChimneyHousingInnerRadius(double x)
    {
      m_ChimneyHousingInnerRadius = x;
    }

    //! Get the outer radius of the chimney housing
    double getChimneyHousingOuterRadius(void) const
    {
      return m_ChimneyHousingOuterRadius;
    }

    //! Set the outer radius of the chimney housing
    void setChimneyHousingOuterRadius(double x)
    {
      m_ChimneyHousingOuterRadius = x;
    }

    //! Get the inner radius of the chimney radiation shield
    double getChimneyShieldInnerRadius(void) const
    {
      return m_ChimneyShieldInnerRadius;
    }

    //! Set the inner radius of the chimney radiation shield
    void setChimneyShieldInnerRadius(double x)
    {
      m_ChimneyShieldInnerRadius = x;
    }

    //! Get the outer radius of the chimney radiation shield
    double getChimneyShieldOuterRadius(void) const
    {
      return m_ChimneyShieldOuterRadius;
    }

    //! Set the outer radius of the chimney radiation shield
    void setChimneyShieldOuterRadius(double x)
    {
      m_ChimneyShieldOuterRadius = x;
    }

    //! Get the inner radius of the chimney pipe
    double getChimneyPipeInnerRadius(void) const
    {
      return m_ChimneyPipeInnerRadius;
    }

    //! Set the inner radius of the chimney pipe
    void setChimneyPipeInnerRadius(double x)
    {
      m_ChimneyPipeInnerRadius = x;
    }

    //! Get the outer radius of the chimney pipe
    double getChimneyPipeOuterRadius(void) const
    {
      return m_ChimneyPipeOuterRadius;
    }

    //! Set the outer radius of the chimney pipe
    void setChimneyPipeOuterRadius(double x)
    {
      m_ChimneyPipeOuterRadius = x;
    }

    //! Get the thickness of the radial rib that supports the solenoid / inner detectors
    double getRibThickness(void) const
    {
      return m_RibThickness;
    }

    //! Set the thickness of the radial rib that supports the solenoid / inner detectors
    void setRibThickness(double x)
    {
      m_RibThickness = x;
    }

    //! Get the width of the cable-services channel at each end
    double getCablesWidth(void) const
    {
      return m_CablesWidth;
    }

    //! Set the width of the cable-services channel at each end
    void setCablesWidth(double x)
    {
      m_CablesWidth = x;
    }

    //! Get the width of the brace in the middle of the cable-services channel
    double getBraceWidth(void) const
    {
      return m_BraceWidth;
    }

    //! Set the width of the brace in the middle of the cable-services channel
    void setBraceWidth(double x)
    {
      m_BraceWidth = x;
    }

    //! Get the width of the brace in the middle of the cable-services channel in the chimney sector
    double getBraceWidthChimney(void) const
    {
      return m_BraceWidthChimney;
    }

    //! Set the width of the brace in the middle of the cable-services channel in the chimney sector
    void setBraceWidthChimney(double x)
    {
      m_BraceWidthChimney = x;
    }

    //! Get width of the innermost-module support plate
    double getSupportPlateWidth(void) const
    {
      return m_SupportPlateWidth;
    }

    //! Set width of the innermost-module support plate
    void setSupportPlateWidth(double x)
    {
      m_SupportPlateWidth = x;
    }

    //! Get height of the innermost-module support plate
    double getSupportPlateHeight(void) const
    {
      return m_SupportPlateHeight;
    }

    //! Set height of the innermost-module support plate
    void setSupportPlateHeight(double x)
    {
      m_SupportPlateHeight = x;
    }

    //! Get length of the innermost-module support plate
    double getSupportPlateLength(void) const
    {
      return m_SupportPlateLength;
    }

    //! Set length of the innermost-module support plate
    void setSupportPlateLength(double x)
    {
      m_SupportPlateLength = x;
    }

    //! Get length of the innermost-module support plate in the chimney sector
    double getSupportPlateLengthChimney(void) const
    {
      return m_SupportPlateLengthChimney;
    }

    //! Set length of the innermost-module support plate in the chimney sector
    void setSupportPlateLengthChimney(double x)
    {
      m_SupportPlateLengthChimney = x;
    }

    //! Get the width of the layer-0 support plate's bracket
    double getBracketWidth(void) const
    {
      return m_BracketWidth;
    }

    //! Set the width of the layer-0 support plate's bracket
    void setBracketWidth(double x)
    {
      m_BracketWidth = x;
    }

    //! Get the thickness of the layer-0 support plate's bracket
    double getBracketThickness(void) const
    {
      return m_BracketThickness;
    }

    //! Set the thickness of the layer-0 support plate's bracket
    void setBracketThickness(double x)
    {
      m_BracketThickness = x;
    }

    //! Get the length of the layer-0 support plate's bracket
    double getBracketLength(void) const
    {
      return m_BracketLength;
    }

    //! Set the length of the layer-0 support plate's bracket
    void setBracketLength(double x)
    {
      m_BracketLength = x;
    }

    //! Get the width of the layer-0 support plate's bracket's rib
    double getBracketRibWidth(void) const
    {
      return m_BracketRibWidth;
    }

    //! Set the width of the layer-0 support plate's bracket's rib
    void setBracketRibWidth(double x)
    {
      m_BracketRibWidth = x;
    }

    //! Get the thickness of the layer-0 support plate's bracket's rib
    double getBracketRibThickness(void) const
    {
      return m_BracketRibThickness;
    }

    //! Set the thickness of the layer-0 support plate's bracket's rib
    void setBracketRibThickness(double x)
    {
      m_BracketRibThickness = x;
    }

    //! Get distance from support plate's end of bracket
    double getBracketInset(void)const
    {
      return m_BracketInset;
    }

    //! Set distance from support plate's end of bracket
    void setBracketInset(double x)
    {
      m_BracketInset = x;
    }

    //! Get the inner radius of the layer-0 support plate's bracket
    double getBracketInnerRadius(void) const
    {
      return m_BracketInnerRadius;
    }

    //! Set the inner radius of the layer-0 support plate's bracket
    void setBracketInnerRadius(double x)
    {
      m_BracketInnerRadius = x;
    }

    //! Get the angular width of the layer-0 support plate's bracket's cutout
    double getBracketCutoutDphi(void) const
    {
      return m_BracketCutoutDphi;
    }

    //! Set the angular width of the layer-0 support plate's bracket's cutout
    void setBracketCutoutDphi(double x)
    {
      m_BracketCutoutDphi = x;
    }

    //! Get the number of preamplifier readout stations
    int getNReadoutStation(void) const
    {
      return m_NReadoutStation;
    }

    //! Get the selector for phi (true) or z (false) readout station
    bool getReadoutStationIsPhi(int station) const
    {
      return m_ReadoutStationIsPhi[station - 1];
    }

    //! Get the position of each readout station along its relevant axis
    double getReadoutStationPosition(int station) const
    {
      return m_ReadoutStationPosition[station - 1];
    }

    //! Get the length of the readout station's container
    double getReadoutContainerLength(void) const
    {
      return m_ReadoutContainerLength;
    }

    //! Get the width of the readout station's container
    double getReadoutContainerWidth(void) const
    {
      return m_ReadoutContainerWidth;
    }

    //! Get the height of the readout station's container
    double getReadoutContainerHeight(void) const
    {
      return m_ReadoutContainerHeight;
    }

    //! Get the length of the readout carrier card
    double getReadoutCarrierLength(void) const
    {
      return m_ReadoutCarrierLength;
    }

    //! Get the width of the readout carrier card
    double getReadoutCarrierWidth(void) const
    {
      return m_ReadoutCarrierWidth;
    }

    //! Get the height of the readout carrier card
    double getReadoutCarrierHeight(void) const
    {
      return m_ReadoutCarrierHeight;
    }

    //! Get the length of the preamplifier card
    double getReadoutPreamplifierLength(void) const
    {
      return m_ReadoutPreamplifierLength;
    }

    //! Get the width of the preamplifier card
    double getReadoutPreamplifierWidth(void) const
    {
      return m_ReadoutPreamplifierWidth;
    }

    //! Get the height of the preamplifier card
    double getReadoutPreamplifierHeight(void) const
    {
      return m_ReadoutPreamplifierHeight;
    }

    //! Get the number of preamplifier positions along the length of the carrier card
    int getNReadoutPreamplifierPosition(void) const
    {
      return m_ReadoutPreamplifierPosition.size();
    }

    //! Get the position of a preamplifier along the length of the carrier card
    double getReadoutPreamplifierPosition(int preamp) const
    {
      return m_ReadoutPreamplifierPosition[preamp - 1];
    }

    //! Get the length of the readout connectors pair
    double getReadoutConnectorsLength(void) const
    {
      return m_ReadoutConnectorsLength;
    }

    //! Get the width of the readout connectors pair
    double getReadoutConnectorsWidth(void) const
    {
      return m_ReadoutConnectorsWidth;
    }

    //! Get the height of the readout connectors pair
    double getReadoutConnectorsHeight(void) const
    {
      return m_ReadoutConnectorsHeight;
    }

    //! Get the position of the readout connectors pair along the length of the carrier card
    double getReadoutConnectorsPosition(void) const
    {
      return m_ReadoutConnectorsPosition;
    }

    //! Get the MPPC housing radius
    double getMPPCHousingRadius(void) const
    {
      return m_MPPCHousingRadius;
    }

    //! Get the MPPC housing length
    double getMPPCHousingLength(void) const
    {
      return m_MPPCHousingLength;
    }

    //! Get the MPPC length
    double getMPPCLength(void) const
    {
      return m_MPPCLength;
    }

    //! Get the MPPC width
    double getMPPCWidth(void) const
    {
      return m_MPPCWidth;
    }

    //! Get the MPPC height
    double getMPPCHeight(void) const
    {
      return m_MPPCHeight;
    }

    //! Get reconstruction dx in local system. displacement, not alignment
    double getLocalReconstructionShiftX(int section, int sector, int layer) const
    {
      return m_LocalReconstructionShiftX[section][sector - 1][layer - 1];
    }

    //! Set reconstruction dx in local system. displacement, not alignment
    void setLocalReconstructionShiftX(int section, int sector, int layer, double x)
    {
      m_LocalReconstructionShiftX[section][sector - 1][layer - 1] = x;
    }

    //! Get reconstruction dy in local system. displacement, not alignment
    double getLocalReconstructionShiftY(int section, int sector, int layer) const
    {
      return m_LocalReconstructionShiftY[section][sector - 1][layer - 1];
    }

    //! Set reconstruction dy in local system. displacement, not alignment
    void setLocalReconstructionShiftY(int section, int sector, int layer, double x)
    {
      m_LocalReconstructionShiftY[section][sector - 1][layer - 1] = x;
    }

    //! Get reconstruction dz in local system. displacement, not alignment
    double getLocalReconstructionShiftZ(int section, int sector, int layer) const
    {
      return m_LocalReconstructionShiftZ[section][sector - 1][layer - 1];
    }

    //! Set reconstruction dz in local system. displacement, not alignment
    void setLocalReconstructionShiftZ(int section, int sector, int layer, double x)
    {
      m_LocalReconstructionShiftZ[section][sector - 1][layer - 1] = x;
    }

    //! Get the z-phi planes flip (i.e., rotation by 180 degrees about z axis)
    //! True: z plane is inner, close to IP. False: phi-plane is inner, close to IP
    //! *for scintillator layers only*
    bool isFlipped(int section, int sector, int layer) const
    {
      return m_IsFlipped[section][sector - 1][layer - 1];
    }

    //! Set the z-phi planes flip (i.e., rotation by 180 degrees about z axis)
    //! True: z plane is inner, close to IP. False: phi-plane is inner, close to IP
    //! *for scintillator layers only*
    void isFlipped(int section, int sector, int layer, bool flag)
    {
      m_IsFlipped[section][sector - 1][layer - 1] = flag;
    }

    //! Get comment
    std::string getBKLMGeometryParComment() const
    {
      return m_comment;
    }

    //! Set comment
    void setBKLMGeometryParComment(const std::string& s)
    {
      m_comment = s;
    }

  private:

    //! Flag for enabling beam background study (=alternate sensitive-detector function)
    bool m_DoBeamBackgroundStudy;

    //! Global rotation angle about z of the BKLM
    double m_Rotation;

    //! Global rotation angle of a sector
    double m_SectorRotation[2][BKLMElementNumbers::getMaximalSectorNumber()];

    //! Global offset along z of the BKLM
    double m_OffsetZ;

    //! Starting angle of the polygon shape
    double m_Phi;

    //! Number of sectors (=8 : octagonal)
    int m_NSector;

    //! Outer radius of the solenoid
    double m_SolenoidOuterRadius;

    //! Radius of the circle tangent to the sides of the outer polygon
    double m_OuterRadius;

    //! Half-length along z of the BKLM
    double m_HalfLength;

    //! Number of layers in one sector
    int m_NLayer;

    //! Nominal height of a layer's structural iron
    double m_IronNominalHeight;

    //! Actual height of a layer's stuctural iron
    double m_IronActualHeight;

    //! Nominal height of the innermost gap
    double m_Gap1NominalHeight;

    //! Nominal height of outer gaps
    double m_GapNominalHeight;

    //! Radius of the inner tangent circle of the innermost gap
    double m_Gap1InnerRadius;

    //! Width (at the outer radius) of the adjacent structural iron on either side of innermost gap
    double m_Gap1IronWidth;

    //! Width (at the outer radius) of the adjacent structural iron on either side of a gap
    double m_GapIronWidth;

    //! Length along z of each gap
    double m_GapLength;

    //! Number of z-measuring cathode strips in a standard RPC module
    int m_NZStrips;

    //! Number of z-measuring cathode strips in a chimney-sector RPC module
    int m_NZStripsChimney;

    //! Number of z-measuring scintillators in a standard scintillator module
    int m_NZScints;

    //! Number of z-measuring scintillators in a chimney-sector scintillator module
    int m_NZScintsChimney;

    //! Flag to indicate whether layer contains RPCs (true) or scintillators (false)
    bool m_HasRPCs[BKLMElementNumbers::getMaximalLayerNumber()];

    //! Number of phi-readout RPC strips in each layer
    int m_NPhiStrips[BKLMElementNumbers::getMaximalLayerNumber()];

    //! Width of the phi strips on each layer
    double m_PhiStripWidth[BKLMElementNumbers::getMaximalLayerNumber()];

    //! Width of the z strips on each layer
    double m_ZStripWidth[BKLMElementNumbers::getMaximalLayerNumber()];

    //! Sign (+/-1) of scintillator-envelope's shift along y axis within its enclosing module for MPPC placement
    //! -1: shift envelope along -y to place MPPCs at +y, +1: shift envelope along +y to place MPPCs at -y
    //! *for scintillator layers only*
    int m_ScintEnvelopeOffsetSign[NSCINTLAYER];

    //! Number of phi-readout scintillators in each layer
    //! *for scintillator layers only*
    int m_NPhiScints[NSCINTLAYER];

    //! Shortening of the nominal length of the z scintillators
    //! *for scintillator layers only*
    double m_ZScintDLength[NSCINTLAYER][NZSCINT];

    //! Length along z of the module
    double m_ModuleLength;

    //! Length along z of the module in the chimney sector
    double m_ModuleLengthChimney;

    //! Height of a detector module's aluminum cover
    double m_ModuleCoverHeight;

    //! Height of a detector module's copper readout or ground plane
    double m_ModuleCopperHeight;

    //! Height of a detector module's transmission-line foam
    double m_ModuleFoamHeight;

    //! Height of a detector module's mylar insulation
    double m_ModuleMylarHeight;

    //! Height of a detector module's glass electrode
    double m_ModuleGlassHeight;

    //! Height of a detector module's gas gap
    double m_ModuleGasHeight;

    //! Width of a detector module's frame ("C" shape - width of horizontal leg)
    double m_ModuleFrameWidth;

    //! Thickness of a detector module's frame ("C" shape - thickness of vertical leg)
    double m_ModuleFrameThickness;

    //! Width of a detector module's spacer
    double m_ModuleGasSpacerWidth;

    //! Height of the inner polystyrene-filler sheet
    double m_ModulePolystyreneInnerHeight;

    //! Height of the outer polystyrene-filler sheet
    double m_ModulePolystyreneOuterHeight;

    //! Width of one scintillator strip (cm), including the TiO2 coating
    double m_ScintWidth;

    //! Height of one scintillator strip (cm), including the TiO2 coating
    double m_ScintHeight;

    //! Radius (cm) of the central bore in the scintillator strip
    double m_ScintBoreRadius;

    //! Radius (cm) of the central WLS fiber in the scintillator strip
    double m_ScintFiberRadius;

    //! Thickness (cm) of the TiO2 coating on the top (and bottom) of the scintillator strip
    double m_ScintTiO2ThicknessTop;

    //! Thickness (cm) of the TiO2 coating on the left (and right) side of the scintillator strip
    double m_ScintTiO2ThicknessSide;

    //! Length along z of the chimney hole
    double m_ChimneyLength;

    //! Width of the chimney hole
    double m_ChimneyWidth;

    //! Thickness of the chimney's iron cover plate
    double m_ChimneyCoverThickness;

    //! Inner radius of the chimney housing
    double m_ChimneyHousingInnerRadius;

    //! Outer radius of the chimney housing
    double m_ChimneyHousingOuterRadius;

    //! Inner radius of the chimney shield
    double m_ChimneyShieldInnerRadius;

    //! Outer radius of the chimney shield
    double m_ChimneyShieldOuterRadius;

    //! Inner radius of the chimney pipe
    double m_ChimneyPipeInnerRadius;

    //! Outer radius of the chimney pipe
    double m_ChimneyPipeOuterRadius;

    //! Thickness of the radial rib that supports the solenoid / inner detectors
    double m_RibThickness;

    //! Width of the cable-services channel at each end
    double m_CablesWidth;

    //! Width of the central brace in the middle of the cable-services channel
    double m_BraceWidth;

    //! Width of the central brace in the middle of the cable-services channel in the chimney sector
    double m_BraceWidthChimney;

    //! Width of the innermost-module support plate
    double m_SupportPlateWidth;

    //! Height of the innermost-module support plate
    double m_SupportPlateHeight;

    //! Length of the innermost-module support plate
    double m_SupportPlateLength;

    //! Length of the innermost-module support plate in the chimney sector
    double m_SupportPlateLengthChimney;

    //! Width of the innermost-module support plate's bracket
    double m_BracketWidth;

    //! Thickness of the innermost-module support plate's bracket
    double m_BracketThickness;

    //! Length of the innermost-module support plate's bracket
    double m_BracketLength;

    //! Width of the innermost-module support plate's bracket's rib
    double m_BracketRibWidth;

    //! Thickness of the innermost-module support plate's bracket's rib
    double m_BracketRibThickness;

    //! Inner radius of the innermost-module support plate's bracket
    double m_BracketInnerRadius;

    //! Distance from support plate's end of bracket
    double m_BracketInset;

    //! Angular width of the innermost-module support plate's bracket's cutout
    double m_BracketCutoutDphi;

    //! Number of preamplifier readout stations
    int m_NReadoutStation;

    //! Selector for phi (true) or z (false) readout station
    bool m_ReadoutStationIsPhi[NSTATION];

    //! Position of each readout station along its relevant axis
    double m_ReadoutStationPosition[NSTATION];

    //! Length of the readout station
    double m_ReadoutContainerLength;

    //! Width of the readout station
    double m_ReadoutContainerWidth;

    //! Height of the readout station
    double m_ReadoutContainerHeight;

    //! Length of the readout carrier card
    double m_ReadoutCarrierLength;

    //! Width of the readout carrier card
    double m_ReadoutCarrierWidth;

    //! Height of the readout carrier card
    double m_ReadoutCarrierHeight;

    //! Length of the preamplifier card
    double m_ReadoutPreamplifierLength;

    //! Width of the preamplifier card
    double m_ReadoutPreamplifierWidth;

    //! Height of the preamplifier card
    double m_ReadoutPreamplifierHeight;

    //! Positions of the preamplifiers along the length of the carrier card
    std::vector<double> m_ReadoutPreamplifierPosition;

    //! Length of the readout connectors pair
    double m_ReadoutConnectorsLength;

    //! Width of the readout connectors pair
    double m_ReadoutConnectorsWidth;

    //! Height of the readout connectors pair
    double m_ReadoutConnectorsHeight;

    //! Position of the readout connectors pair along the length of the carrier card
    double m_ReadoutConnectorsPosition;

    //! MPPC housing radius
    double m_MPPCHousingRadius;

    //! MPPC housing length
    double m_MPPCHousingLength;

    //! MPPC length
    double m_MPPCLength;

    //! MPPC width
    double m_MPPCWidth;

    //! MPPC height
    double m_MPPCHeight;

    //! Reconstruction dx in local system. displacement, not alignment
    double m_LocalReconstructionShiftX[2][BKLMElementNumbers::getMaximalSectorNumber()][BKLMElementNumbers::getMaximalLayerNumber()];

    //! Reconstruction dy in local system. displacement, not alignment
    double m_LocalReconstructionShiftY[2][BKLMElementNumbers::getMaximalSectorNumber()][BKLMElementNumbers::getMaximalLayerNumber()];

    //! Reconstruction dz in local system. displacement, not alignment
    double m_LocalReconstructionShiftZ[2][BKLMElementNumbers::getMaximalSectorNumber()][BKLMElementNumbers::getMaximalLayerNumber()];

    //! Flag of z-phi planes flip *for scintillator layers only*
    bool m_IsFlipped[2][BKLMElementNumbers::getMaximalSectorNumber()][NSCINTLAYER];

    //! Optional comment
    std::string m_comment;

    /** Class version. */
    ClassDef(BKLMGeometryPar, 6);

  };
} // end of namespace Belle2
