/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

//#include "bklm/geometry/Module.h"

#include <vector>
#include <map>

#include "CLHEP/Vector/ThreeVector.h"

#include <TObject.h>
#include <string>
#include <cmath>
#include <vector>

//#include <TROOT.h>
//#include <TClass.h>

static const int NLAYER = 15;
static const int NSCINTLAYER = 2;
static const int NSECTOR = 8;
static const int NSCINT = 100;
static const int BKLM_INNER = 1;
static const int BKLM_OUTER = 2;
static const int BKLM_FORWARD = 1;
static const int BKLM_BACKWARD = 2;

namespace Belle2 {

  class GearDir;

  /**
  * The Class for BKLM geometry.
  */

  class BKLMGeometryPar: public TObject {

  public:

    //!Default constructor
    BKLMGeometryPar() {};

    //! Constructor
    explicit BKLMGeometryPar(const GearDir&);

    //! Destructor
    ~BKLMGeometryPar();

    //! Returen BKLM geometry version
    int getVersion() const {return m_version; }

    //! set BKLM geometry version
    void setVersion(int version) { m_version = version; }

    //! Get the overlap-check flag for the geometry builder
    bool doOverlapCheck(void) const { return m_DoOverlapCheck == 1 ? true : false; }

    //! Get the flag for doing beamBackgroundStudy
    bool doBeamBackgroundStudy(void) const { return (m_beamBackgroundStudy == 1 ? true : false); }

    //! Clear all geometry parameters
    //void clear();

    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

    //! Determine if the sensitive detectors in a given layer are RPCs (=true) or scintillators (=false)
    //bool hasRPCs(int layer) const {return m_HasRPCs[layer]; }
    bool hasRPCs(int layer) const;

    //! Set flag to indicate whether layer contains RPCs (true) or scintillators (false)
    void setHasRPCs(int layer, bool hasRPC) { m_HasRPCs[layer] = hasRPC; }

    //! Get the inner radius of specified layer
    double getLayerInnerRadius(int layer) const;

    //! Get the outer radius of specified layer
    double getLayerOuterRadius(int layer) const;

    //! Get the size (dx,dy,dz) of the gap [=slot] of specified layer
    const CLHEP::Hep3Vector getGapHalfSize(int layer, bool hasChimney) const;

    //! Get the size (dx,dy,dz) of the detector module of specified layer
    const CLHEP::Hep3Vector getModuleHalfSize(int layer, bool hasChimney) const;

    //! Get the size (dx,dy,dz) of the detector module's interior volume 1
    const CLHEP::Hep3Vector getModuleInteriorHalfSize1(int layer, bool hasChimney) const;

    //! Get the size (dx,dy,dz) of the scintillator detector module's polystyrene filler
    const CLHEP::Hep3Vector getModuleInteriorHalfSize2(int layer, bool hasChimney) const;

    //! Get the size (dx,dy,dz) of the detector module's electrode of specified layer
    const CLHEP::Hep3Vector getElectrodeHalfSize(int layer, bool hasChimney) const;

    //! Get the size (dx,dy,dz) of the detector module's gas gaps of specified layer
    const CLHEP::Hep3Vector getGasHalfSize(int layer, bool hasChimney) const;

    //! Get the size (dx,dy,dz) of the scintillator detector module's air filler
    const CLHEP::Hep3Vector getAirHalfSize(int layer, bool hasChimney) const;

    //! Get the size (dx,dy,dz) of the scintillator detector module's scintillator envelope
    const CLHEP::Hep3Vector getScintEnvelopeHalfSize(int layer, bool hasChimney) const;

    //! Get the shift (dx,dy,dz) of the scintillator detector module's scintillator envelope within its enclosure
    const CLHEP::Hep3Vector getScintEnvelopeOffset(int layer, bool hasChimney) const;

    //! Get the radial offset of the scintillator detector module's active envelope due to difference in polystyrene-sheet thicknesses
    double getPolystyreneOffsetX(void) const;

    //! Get the thickness of the inactive TiO2-polystyrene coating on top (broad) surface of a scintillator strip
    double getScintTiO2ThicknessTop(void) const { return m_ScintTiO2ThicknessTop; }

    //! Set the thickness of the inactive TiO2-polystyrene coating on top (broad) surface of a scintillator strip
    void setScintTiO2ThicknessTop(double scintTiO2ThicknessTop)  { m_ScintTiO2ThicknessTop = scintTiO2ThicknessTop; }

    //! Get the thickness of the inactive TiO2-polystyrene coating on side (short) surface of a scintillator strip
    double getScintTiO2ThicknessSide(void) const { return m_ScintTiO2ThicknessSide; }

    //! Set the thickness of the inactive TiO2-polystyrene coating on side (short) surface of a scintillator strip
    void setScintTiO2ThicknessSide(double scintTiO2ThicknessSide) {m_ScintTiO2ThicknessSide = scintTiO2ThicknessSide; }

    //! Get length along z of the chimney hole
    double getChimneyLength(void) const {return m_ChimneyLength;}

    //! Set length along z of the chimney hole
    void setChimneyLength(double chimneyLength) {m_ChimneyLength = chimneyLength;}

    //! Get width of the chimney hole
    double getChimneyWidth(void) const {return m_ChimneyWidth;}

    //! Set width of the chimney hole
    void setChimneyWidth(double chimneyWidth) {m_ChimneyWidth = chimneyWidth;}

    //! Get half of the height of the entire volume of a scintillator strip (including TiO2 coating)
    double getScintHalfHeight(void) const { return 0.5 * m_ScintHeight; }

    //! Get the height of the entire volume of a scintillator strip (including TiO2 coating)
    double getScintHeight(void) const { return  m_ScintHeight; }

    //! Set the height of the entire volume of a scintillator strip (including TiO2 coating)
    void setScintHeight(double scintHeight) {  m_ScintHeight = scintHeight; }

    //! Get half of the height of the entire volume of a scintillator strip (including TiO2 coating)
    double getScintHalfWidth(void) const { return 0.5 * m_ScintWidth; }

    //! Get the height of the entire volume of a scintillator strip (including TiO2 coating)
    double getScintWidth(void) const { return m_ScintWidth; }

    //! Set the height of the entire volume of a scintillator strip (including TiO2 coating)
    void setScintWidth(double scintWidth) { m_ScintWidth = scintWidth; }

    //! Get the radius of the cylindrical central bore in a scintillator strip
    double getScintBoreRadius(void) const { return m_ScintBoreRadius; }

    //! Set the radius of the cylindrical central bore in a scintillator strip
    void setScintBoreRadius(double scintBoreRadius) { m_ScintBoreRadius = scintBoreRadius; }

    //! Get the radius of the cylindrical central WLS fiber in a scintillator strip
    double getScintFiberRadius(void) const { return m_ScintFiberRadius; }

    //! Set the radius of the cylindrical central WLS fiber in a scintillator strip
    void setScintFiberRadius(double scintFiberRadius) { m_ScintFiberRadius = scintFiberRadius; }

    //! Get the radial midpoint of the gap of specified layer
    double getGapMiddleRadius(int layer) const;

    //! Get the radial midpoint of the detector module of specified layer
    double getModuleMiddleRadius(int layer) const;

    //! Get the radial midpoint of the detector module's active volume of specified layer
    double getActiveMiddleRadius(int layer) const;

    //! Get the global rotation angle about z of the entire BKLM
    double getRotation(void) const { return m_Rotation; }

    //! Get the rotation angle for a sector
    double getSectorRotation(int sector) const {return m_SectorRotation[sector];}

    //! Set the rotation angle for a sector
    void setSectorRotation(int sector, double sectorRotation) {m_SectorRotation[sector] = sectorRotation; }

    //! Set the global rotation angle about z of the entire BKLM
    void setRotation(double rotation)  { m_Rotation = rotation; }

    //! Get the global shift along a of the entire BKLM
    double getOffsetZ(void) const { return m_OffsetZ; }

    //! Set the global shift along a of the entire BKLM
    void setOffsetZ(double offsetZ) { m_OffsetZ = offsetZ; }

    //! Get the starting angle of the BKLM's polygon shape
    double getPhi(void) const { return m_Phi; }

    //! Set the starting angle of the BKLM's polygon shape
    void setPhi(double phi) { m_Phi = phi; }

    //! Get the outer radius of the solenoid
    double getSolenoidOuterRadius(void) const { return m_SolenoidOuterRadius; }

    //! Set the outer radius of the solenoid
    void setSolenoidOuterRadius(double solenoidOuterRadius) {m_SolenoidOuterRadius = solenoidOuterRadius; }

    //! Get the number of sectors of the BKLM
    int getNSector(void) const { return m_NSector; }

    //! Set the number of sectors of the BKLM
    void setNSector(int nSector) { m_NSector = nSector; }

    //! Get the half-length along z of the BKLM
    double getHalfLength(void) const { return m_HalfLength; }

    //! Set the half-length along z of the BKLM
    void setHalfLength(double halfLength) { m_HalfLength = halfLength; }

    //! Get the radius of the inscribed circle of the outer polygon
    double getOuterRadius(void) const { return m_OuterRadius; }

    //! Set the radius of the inscribed circle of the outer polygon
    void setOuterRadius(double outerRadius) { m_OuterRadius = outerRadius; }

    //! Get the number of modules in one sector
    int getNLayer(void) const { return m_NLayer; }

    //! Set the number of modules in one sector
    void setNLayer(double nLayer) { m_NLayer = nLayer; }

    //! Get the nominal height of a layer's structural iron
    double getIronNominalHeight(void) const { return m_IronNominalHeight; }

    //! Set the nominal height of a layer's structural iron
    void setIronNominalHeight(double ironNominalHeight) { m_IronNominalHeight = ironNominalHeight; }

    //! Get the actual height of a layer's structural iron
    double getIronActualHeight(void) const { return m_IronActualHeight; }

    //! Set the actual height of a layer's structural iron
    void setIronActualHeight(double ironActualHeight) { m_IronActualHeight = ironActualHeight; }

    //! Get the radius of the inner tangent circle of gap 0 (innermost)
    double getGap1InnerRadius(void) const { return m_Gap1InnerRadius; }

    //! Set the radius of the inner tangent circle of gap 0 (innermost)
    void setGap1InnerRadius(double gap1InnerRadius) { m_Gap1InnerRadius = gap1InnerRadius; }

    //! Get the nominal height of the innermost gap
    double getGap1NominalHeight(void) const { return m_Gap1NominalHeight; }

    //! Set the nominal height of the innermost gap
    void setGap1NominalHeight(double gap1NominalHeight) { m_Gap1NominalHeight = gap1NominalHeight; }

    //! Get the actual height of the innermost gap
    double getGap1ActualHeight(void) const { return m_Gap1ActualHeight; }

    //! Set the actual height of the innermost gap
    void setGap1ActualHeight(double gap1ActualHeight) { m_Gap1ActualHeight = gap1ActualHeight; }

    //! Get the height of layer 1,the innermost layer
    double getLayer1Height(void) const { return m_Layer1Height; }

    //! Set the actual height of layer 1, the innermost layer
    void setLayer1Height(double Height) { m_Layer1Height = Height; }

    //! Get the height of layer
    double getLayerHeight(void) const { return m_LayerHeight; }

    //! Set the actual height of layer
    void setLayerHeight(double Height) { m_LayerHeight = Height; }

    //! Get the width (at the outer radius) of the adjacent structural iron on either side of innermost gap
    double getGap1IronWidth(void) const { return m_Gap1IronWidth; }

    //! Set the width (at the outer radius) of the adjacent structural iron on either side of innermost gap
    void setGap1IronWidth(double gap1IronWidth) { m_Gap1IronWidth = gap1IronWidth; }

    //! Get the length along z of the module gap
    double getGapLength(void) const { return m_GapLength; }

    //! Set the length along z of the module gap
    void setGapLength(double gapLength) { m_GapLength = gapLength; }

    //! Get the nominal height of the outer gaps
    double getGapNominalHeight(void) const { return m_GapNominalHeight; }

    //! Set the nominal height of the outer gaps
    void setGapNominalHeight(double gapNominalHeight) { m_GapNominalHeight = gapNominalHeight; }

    //! Get the actual height of the outer gaps
    double getGapActualHeight(void) const { return m_GapActualHeight; }

    //! Set the actual height of the outer gaps
    void setGapActualHeight(double gapActualHeight) { m_GapActualHeight = gapActualHeight; }

    //! Get the width (at the outer radius) of the adjacent structural iron on either side of a gap
    double getGapIronWidth(void) const { return m_GapIronWidth; }

    //! Set the width (at the outer radius) of the adjacent structural iron on either side of a gap
    void setGapIronWidth(double gapIronWidth) { m_GapIronWidth = gapIronWidth; }

    //! Get the radius of the inner tangent circle of gap 1 (next-to-innermost)
    double getGapInnerRadius(void) const { return m_GapInnerRadius; }

    //! Set the radius of the inner tangent circle of gap 1 (next-to-innermost)
    void setGapInnerRadius(double gapInnerRadius) { m_GapInnerRadius = gapInnerRadius; }

    //! Get the number of z-measuring cathode strips in an RPC module
    int getNZStrips(bool isChimney) const { return (isChimney ? m_NZStripsChimney : m_NZStrips); }

    //! Set the number of z-measuring cathode strips in an RPC module
    void setNZStrips(int nZStrips) { m_NZStrips = nZStrips; }

    //! Set the number of z-measuring cathode strips in an RPC module in Chimney sector
    void setNZStripsChimney(int nZStripsChimney) { m_NZStripsChimney = nZStripsChimney; }

    //! Get the number of phi-measuring cathode strips in an RPC module
    int getNPhiStrips(int layer) const { return m_NPhiStrips[layer]; }

    //! set the number of phi-measuring cathode strips in an RPC module
    void setNPhiStrips(int layer, int nPhiStrips) {m_NPhiStrips[layer] = nPhiStrips;}

    //! Get the number of z-measuring scintillators in a scintillator module
    int getNZScints(bool isChimney) const { return (isChimney ? m_NZScintsChimney : m_NZScints); }

    //! Set the number of z-measuring scintillators in a scintillator module
    void setNZScints(int nZScints) { m_NZScints = nZScints; }

    //! Set the number of z-measuring scintillators in a scintillator module
    void setNZScintsChimney(int nZScintsChimney) { m_NZScintsChimney = nZScintsChimney; }

    //! Get the number of phi-measuring scintillators in a scintillator module
    //int getNPhiScints(int layer) const{return m_NPhiScints[layer]; }
    int getNPhiScints(int layer) const;

    //! set the number of phi-measuring scintillators in an scintillator module
    void setNPhiScints(int layer, int nPhiScints) {m_NPhiScints[layer] = nPhiScints;}

    //! get the number of phi-measuring scintillators in an scintillator module
    double getPhiScintsOffsetSign(int layer) const {return m_PhiScintsOffsetSign[layer]; }

    //! set the number of phi-measuring scintillators in an scintillator module
    void setPhiScintsOffsetSign(int layer, int phiScintsOffsetSign) {m_PhiScintsOffsetSign[layer] = phiScintsOffsetSign;}

    //! Get the length along z of the module
    double getModuleLength(void) const { return m_ModuleLength; }

    //! Set the length along z of the module
    void setModuleLength(double moduleLength) { m_ModuleLength = moduleLength; }

    //! Get the length along z of the module
    double getModuleLengthChimney(void) const { return m_ModuleLengthChimney; }

    //! Set the length along z of the module
    void setModuleLengthChimney(double moduleLengthChimney) { m_ModuleLengthChimney = moduleLengthChimney; }

    //! Get the height of the module's aluminum cover (2 per module)
    double getModuleCoverHeight(void) const { return m_ModuleCoverHeight; }

    //! Set the height of the module's aluminum cover (2 per module)
    void setModuleCoverHeight(double moduleCoverHeight) { m_ModuleCoverHeight = moduleCoverHeight; }

    //! Get the height of the module's readout or ground copper plane (4 per module)
    double getModuleCopperHeight(void) const { return m_ModuleCopperHeight; }

    //! Set the height of the module's readout or ground copper plane (4 per module)
    void setModuleCopperHeight(double moduleCopperHeight) { m_ModuleCopperHeight = moduleCopperHeight; }

    //! Get the height of the module's transmission-line foam (2 per module)
    double getModuleFoamHeight(void) const { return m_ModuleFoamHeight; }

    //! Set the height of the module's transmission-line foam (2 per module)
    void setModuleFoamHeight(double moduleFoamHeight) { m_ModuleFoamHeight = moduleFoamHeight; }

    //! Get the height of the module's insulating mylar (2 per module)
    double getModuleMylarHeight(void) const { return m_ModuleMylarHeight; }

    //! Set the height of the module's insulating mylar (2 per module)
    void setModuleMylarHeight(double moduleMylarHeight) { m_ModuleMylarHeight = moduleMylarHeight; }

    //! Get the height of a detector module's readout
    double getModuleReadoutHeight(void) const {return m_ModuleReadoutHeight;}

    //! Set the height of a detector module's readout
    void setModuleReadoutHeight(double ModuleReadoutHeight) { m_ModuleReadoutHeight = ModuleReadoutHeight;}

    //! Get the height of the module's glass electrode (4 per module)
    double getModuleGlassHeight(void) const { return m_ModuleGlassHeight; }

    //! Set the height of the module's glass electrode (4 per module)
    void setModuleGlassHeight(double moduleGlassHeight) { m_ModuleGlassHeight = moduleGlassHeight; }

    //! Get the height of the module's gas gap (2 per module)
    double getModuleGasHeight(void) const { return m_ModuleGasHeight; }

    //! Set the height of the module's gas gap (2 per module)
    void setModuleGasHeight(double moduleGasHeight) { m_ModuleGasHeight = moduleGasHeight; }

    //! Get the height of the module
    double getModuleHeight(void) const { return m_ModuleHeight; }

    //! Set the height of the module
    void setModuleHeight(double moduleHeight) { m_ModuleHeight = moduleHeight; }

    //! Get the width of the module's perimeter aluminum frame
    double getModuleFrameWidth(void) const { return m_ModuleFrameWidth; }

    //! Set the width of the module's perimeter aluminum frame
    void setModuleFrameWidth(double moduleFrameWidth) { m_ModuleFrameWidth = moduleFrameWidth; }

    //! Get the thickness of the module's perimeter aluminum frame
    double getModuleFrameThickness(void) const { return m_ModuleFrameThickness; }

    //! Set the thickness of the module's perimeter aluminum frame
    void setModuleFrameThickness(double moduleFrameThickness) { m_ModuleFrameThickness = moduleFrameThickness; }

    //! Get the width of the module's gas-gap's perimeter spacer
    double getModuleGasSpacerWidth(void) const { return m_ModuleGasSpacerWidth; }

    //! Set the width of the module's gas-gap's perimeter spacer
    void setModuleGasSpacerWidth(double moduleGasSpacerWidth) { m_ModuleGasSpacerWidth = moduleGasSpacerWidth; }

    //! Get the size of the border between a detector module's perimeter and electrode
    double getModuleElectrodeBorder(void) const { return m_ModuleElectrodeBorder; }

    //! Set the size of the border between a detector module's perimeter and electrode
    void setModuleElectrodeBorder(double moduleElectrodeBorder) { m_ModuleElectrodeBorder = moduleElectrodeBorder; }

    //! Get the height of the inner polystyrene-filler sheet
    double getModulePolystyreneInnerHeight(void) const { return m_ModulePolystyreneInnerHeight; }

    //! Set the height of the inner polystyrene-filler sheet
    void setModulePolystyreneInnerHeight(double modulePolystyreneInnerHeight) { m_ModulePolystyreneInnerHeight = modulePolystyreneInnerHeight; }

    //! Get the height of the outer polystyrene-filler sheet
    double getModulePolystyreneOuterHeight(void) const { return m_ModulePolystyreneOuterHeight; }

    //! Set the height of the outer polystyrene-filler sheet
    void setModulePolystyreneOuterHeight(double modulePolystyreneOuterHeight) { m_ModulePolystyreneOuterHeight = modulePolystyreneOuterHeight; }

    //! Get the size of the chimney hole in the specified layer
    const CLHEP::Hep3Vector getChimneyHalfSize(int layer) const;

    //! Get the position of the chimney hole in the specified layer
    const CLHEP::Hep3Vector getChimneyPosition(int layer) const;

    //! Get the thickness of the chimney cover plate
    double getChimneyCoverThickness(void) const { return m_ChimneyCoverThickness; }

    //! Set the thickness of the chimney cover plate
    void setChimneyCoverThickness(double chimneyCoverThickness) { m_ChimneyCoverThickness = chimneyCoverThickness; }

    //! Get the inner radius of the chimney housing
    double getChimneyHousingInnerRadius(void) const { return m_ChimneyHousingInnerRadius; }

    //! Set the inner radius of the chimney housing
    void setChimneyHousingInnerRadius(double chimneyHousingInnerRadius) { m_ChimneyHousingInnerRadius = chimneyHousingInnerRadius; }

    //! Get the outer radius of the chimney housing
    double getChimneyHousingOuterRadius(void) const { return m_ChimneyHousingOuterRadius; }

    //! Set the outer radius of the chimney housing
    void setChimneyHousingOuterRadius(double chimneyHousingOuterRadius) { m_ChimneyHousingOuterRadius = chimneyHousingOuterRadius; }

    //! Get the inner radius of the chimney radiation shield
    double getChimneyShieldInnerRadius(void) const { return m_ChimneyShieldInnerRadius; }

    //! Set the inner radius of the chimney radiation shield
    void setChimneyShieldInnerRadius(double chimneyShieldInnerRadius) { m_ChimneyShieldInnerRadius = chimneyShieldInnerRadius; }

    //! Get the outer radius of the chimney radiation shield
    double getChimneyShieldOuterRadius(void) const { return m_ChimneyShieldOuterRadius; }

    //! Set the outer radius of the chimney radiation shield
    void setChimneyShieldOuterRadius(double chimneyShieldOuterRadius) { m_ChimneyShieldOuterRadius = chimneyShieldOuterRadius; }

    //! Get the inner radius of the chimney pipe
    double getChimneyPipeInnerRadius(void) const { return m_ChimneyPipeInnerRadius; }

    //! Set the inner radius of the chimney pipe
    void setChimneyPipeInnerRadius(double chimneyPipeInnerRadius) { m_ChimneyPipeInnerRadius = chimneyPipeInnerRadius; }

    //! Get the outer radius of the chimney pipe
    double getChimneyPipeOuterRadius(void) const { return m_ChimneyPipeOuterRadius; }

    //! Set the outer radius of the chimney pipe
    void setChimneyPipeOuterRadius(double chimneyPipeOuterRadius) { m_ChimneyPipeOuterRadius = chimneyPipeOuterRadius; }

    //! Get the thickness of the radial rib that supports the solenoid / inner detectors
    double getRibThickness(void) const { return m_RibThickness; }

    //! Set the thickness of the radial rib that supports the solenoid / inner detectors
    void setRibThickness(double ribThickness) { m_RibThickness = ribThickness; }

    //! Get the width of the cable-services channel at each end
    double getCablesWidth(void) const { return m_CablesWidth; }

    //! Set the width of the cable-services channel at each end
    void setCablesWidth(double cablesWidth) {m_CablesWidth = cablesWidth; }

    //! Get the width of the brace in the middle of the cable-services channel
    double getBraceWidth(void) const { return m_BraceWidth; }

    //! Set the width of the brace in the middle of the cable-services channel
    void setBraceWidth(double braceWidth) { m_BraceWidth = braceWidth; }

    //! Get the width of the brace in the middle of the cable-services channel in the chimney sector
    double getBraceWidthChimney(void) const { return m_BraceWidthChimney; }

    //! Set the width of the brace in the middle of the cable-services channel in the chimney sector
    void setBraceWidthChimney(double braceWidthChimney) { m_BraceWidthChimney = braceWidthChimney; }

    //! Get width of the innermost-module support plate
    double getSupportPlateWidth(void) const { return m_SupportPlateWidth; }

    //! Set width of the innermost-module support plate
    void setSupportPlateWidth(double supportPlateWidth) { m_SupportPlateWidth = supportPlateWidth; }

    //! Get height of the innermost-module support plate
    double getSupportPlateHeight(void) const { return m_SupportPlateHeight; }

    //! Set height of the innermost-module support plate
    void setSupportPlateHeight(double supportPlateHeight) { m_SupportPlateHeight = supportPlateHeight; }

    //! Get length of the innermost-module support plate
    double getSupportPlateLength(void) const { return m_SupportPlateLength; }

    //! Set length of the innermost-module support plate
    void setSupportPlateLength(double supportPlateLength) { m_SupportPlateLength = supportPlateLength; }

    //! Get length of the innermost-module support plate in the chimney sector
    double getSupportPlateLengthChimney(void) const { return m_SupportPlateLengthChimney; }

    //! Set length of the innermost-module support plate in the chimney sector
    void setSupportPlateLengthChimney(double supportPlateLengthChimney) { m_SupportPlateLengthChimney = supportPlateLengthChimney; }

    //! Get the size of the layer-0 support plate
    const CLHEP::Hep3Vector getSupportPlateHalfSize(bool) const;

    //! Get the width of the layer-0 support plate's bracket
    double getBracketWidth(void) const { return m_BracketWidth; }

    //! Set the width of the layer-0 support plate's bracket
    void setBracketWidth(double bracketWidth) { m_BracketWidth = bracketWidth; }

    //! Get the thickness of the layer-0 support plate's bracket
    double getBracketThickness(void) const { return m_BracketThickness; }

    //! Set the thickness of the layer-0 support plate's bracket
    void setBracketThickness(double bracketThickness) { m_BracketThickness = bracketThickness; }

    //! Get the length of the layer-0 support plate's bracket
    double getBracketLength(void) const { return m_BracketLength; }

    //! Set the length of the layer-0 support plate's bracket
    void setBracketLength(double bracketLength) { m_BracketLength = bracketLength; }

    //! Get the width of the layer-0 support plate's bracket's rib
    double getBracketRibWidth(void) const { return m_BracketRibWidth; }

    //! Set the width of the layer-0 support plate's bracket's rib
    void setBracketRibWidth(double bracketRibWidth) { m_BracketRibWidth = bracketRibWidth; }

    //! Get the thickness of the layer-0 support plate's bracket's rib
    double getBracketRibThickness(void) const { return m_BracketRibThickness; }

    //! Set the thickness of the layer-0 support plate's bracket's rib
    void setBracketRibThickness(double bracketRibThickness) { m_BracketRibThickness = bracketRibThickness; }

    //! Get distance from support plate's end of bracket
    double getBracketInset(void)const {return m_BracketInset; }

    //! Set distance from support plate's end of bracket
    void setBracketInset(double bracketInset) {m_BracketInset = bracketInset; }

    //! Get the inner radius of the layer-0 support plate's bracket
    double getBracketInnerRadius(void) const { return m_BracketInnerRadius; }

    //! Set the inner radius of the layer-0 support plate's bracket
    void setBracketInnerRadius(double bracketInnerRadius) { m_BracketInnerRadius = bracketInnerRadius; }

    //! Get the position of a layer-0 support plate's bracket
    double getBracketZPosition(int, bool) const;

    //! Get the angular width of the layer-0 support plate's bracket's cutout
    double getBracketCutoutDphi(void) const { return m_BracketCutoutDphi; }

    //! Set the angular width of the layer-0 support plate's bracket's cutout
    void setBracketCutoutDphi(double bracketCutoutDphi) { m_BracketCutoutDphi = bracketCutoutDphi; }

    //! Get reconstructionShift of each layer along x  in local system. displacement, not alignment here
    double getLocalReconstructionShiftX(int sector, int layer) const {return m_LocalReconstructionShift_X[sector][layer];}

    //! Set reconstructionShift of each layer along x  in local system. displacement, not alignment here
    void setLocalReconstructionShiftX(int sector, int layer, double localReconstructionShift) {m_LocalReconstructionShift_X[sector][layer] = localReconstructionShift;}

    //! Get reconstructionShift of each layer along y  in local system. displacement, not alignment here
    double getLocalReconstructionShiftY(int sector, int layer) const {return m_LocalReconstructionShift_Y[sector][layer];}

    //! Set reconstructionShift of each layer along y  in local system. displacement, not alignment here
    void setLocalReconstructionShiftY(int sector, int layer, double localReconstructionShift) {m_LocalReconstructionShift_Y[sector][layer] = localReconstructionShift;}

    //! Get reconstructionShift of each layer along z  in local system. displacement, not alignment here
    double getLocalReconstructionShiftZ(int sector, int layer) const {return m_LocalReconstructionShift_Z[sector][layer];}

    //! Set reconstructionShift of each layer along z  in local system. displacement, not alignment here
    void setLocalReconstructionShiftZ(int sector, int layer, double localReconstructionShift) {m_LocalReconstructionShift_Z[sector][layer] = localReconstructionShift;}

    //!get the number of the phi strips on each layer.
    int getPhiStripNumber(int layer) const {return m_PhiStripNumber[layer]; }

    //!get width of the phi strips on each layer.
    double getPhiStripWidth(int layer) const {return m_PhiStripWidth[layer]; }

    //!set width of the phi strips on each layer.
    void setPhiStripWidth(int layer, double width) {m_PhiStripWidth[layer] = width;}

    //!get width of the z strips on each layer.
    double getZStripWidth(int layer) const {return m_ZStripWidth[layer]; }

    //!set width of the z strips on each layer.
    void setZStripWidth(int layer, double width) {m_ZStripWidth[layer] = width;}

    //! get dLength of the phi scintillators.
    double getPhiScintsDlength(int layer, int sci) const {return m_PhiScintsDlength[layer][sci]; }

    //! set dLength of the phi scintillators.
    void setPhiScintsDlength(int layer, int sci, double phiScintsDlength) {m_PhiScintsDlength[layer][sci] = phiScintsDlength; }

    //! get dLength of the z scintillators.
    double getZScintsDlength(int layer, int sci) const {return m_ZScintsDlength[layer][sci]; }

    //! set dLength of the z scintillators.
    void setZScintsDlength(int layer, int sci, double zScintsDlength) {m_ZScintsDlength[layer][sci] = zScintsDlength; }

    //! Get the pointer to the definition of a module
    //const bklm::Module* findModule(bool isForward, int sector, int layer) const;

    //! Get the pointer to the definition of a module
    //const bklm::Module* findModule(int layer, bool hasChimney) const;

    //! Get  comment
    std::string getBKLMGeometryParComment() const {return m_comment; }

    //! Set comment
    void setBKLMGeometryParComment(const std::string& comment) {m_comment = comment;}

  private:

    //! geometry version
    int m_version;

    //! Flag for enabling overlap-check during geometry construction
    int m_DoOverlapCheck;

    //! Flag for beam Background Study
    int m_beamBackgroundStudy;

    //! global rotation about z of the BKLM
    double m_Rotation;

    //! global rotation of a sector
    double m_SectorRotation[NSECTOR + 1];

    //! global offset along z of the BKLM
    double m_OffsetZ;

    //! starting angle of the polygon shape
    double m_Phi;

    //! number of sectors (=8 : octagonal)
    int m_NSector;

    //! outer radius of the solenoid
    double m_SolenoidOuterRadius;

    //! radius of the circle tangent to the sides of the outer polygon
    double m_OuterRadius;

    //! half-length along z of the BKLM
    double m_HalfLength;

    //! number of layers in one sector
    int m_NLayer;

    //! nominal height of a layer's structural iron
    double m_IronNominalHeight;

    //! actual height of a layer's stuctural iron
    double m_IronActualHeight;

    //! radius of the inner tangent circle of the innermost gap
    double m_Gap1InnerRadius;

    //! nominal height of the innermost gap
    double m_Gap1NominalHeight;

    //! actual height of the innermost gap
    double m_Gap1ActualHeight;

    //! height of layer 0
    double m_Layer1Height;

    //! height of a layer
    double m_LayerHeight;

    //! variable for width (at the outer radius) of the adjacent structural iron on either side of innermost gap
    double m_Gap1IronWidth;

    //! length along z of each gap
    double m_GapLength;

    //! nominal height of outer gaps
    double m_GapNominalHeight;

    //! actual height of outer gaps
    double m_GapActualHeight;

    //! width (at the outer radius) of the adjacent structural iron on either side of a gap
    double m_GapIronWidth;

    //! radius of the inner tangent circle of virtual gap 0 (assuming equal-height layers)
    double m_GapInnerRadius;

    //! Number of phi-readout RPC strips in each layer
    int m_NPhiStrips[NLAYER + 1];

    //! Number of phi-readout scintillators in each layer
    int m_NPhiScints[NLAYER + 1];

    //! number of z-measuring cathode strips in a standard RPC module
    int m_NZStrips;

    //! number of z-measuring cathode strips in a chimney-sector RPC module
    int m_NZStripsChimney;

    //! number of z-measuring scintillators in a standard scintillator module
    int m_NZScints;

    //! number of z-measuring scintillators in a chimney-sector scintillator module
    int m_NZScintsChimney;

    //! Sign (+/-1) of scintillator envelope's shift along phi axis within its enclosing module
    int m_PhiScintsOffsetSign[NLAYER + 1];

    //! length along z of the module
    double m_ModuleLength;

    //! length along z of the module in the chimney sector
    double m_ModuleLengthChimney;

    //! height of a detector module's aluminum cover
    double m_ModuleCoverHeight;

    //! height of a detector module's copper readout or ground plane
    double m_ModuleCopperHeight;

    //! height of a detector module's transmission-line foam
    double m_ModuleFoamHeight;

    //! height of a detector module's mylar insulation
    double m_ModuleMylarHeight;

    //! height of a detector module's readout
    double m_ModuleReadoutHeight;

    //! height of a detector module's glass electrode
    double m_ModuleGlassHeight;

    //! height of a detector module's gas gap
    double m_ModuleGasHeight;

    //! height of a detector module
    double m_ModuleHeight;

    //! width of a detector module's frame ("C" shape - width of horizontal leg)
    double m_ModuleFrameWidth;

    //! thickness of a detector module's frame ("C" shape - thickness of vertical leg)
    double m_ModuleFrameThickness;

    //! width of a detector module's spacer
    double m_ModuleGasSpacerWidth;

    //! size of the border between a detector module's perimeter and electrode
    double m_ModuleElectrodeBorder;

    //! height of the inner polystyrene-filler sheet
    double m_ModulePolystyreneInnerHeight;

    //! height of the outer polystyrene-filler sheet
    double m_ModulePolystyreneOuterHeight;

    //! width of one scintillator strip (cm), including the TiO2 coating
    double m_ScintWidth;

    //! height of one scintillator strip (cm), including the TiO2 coating
    double m_ScintHeight;

    //! radius (cm) of the central bore in the scintillator strip
    double m_ScintBoreRadius;

    //! radius (cm) of the central WLS fiber in the scintillator strip
    double m_ScintFiberRadius;

    //! thickness (cm) of the TiO2 coating on the top (and bottom) of the scintillator strip
    double m_ScintTiO2ThicknessTop;

    //! thickness (cm) of the TiO2 coating on the left (and right) side of the scintillator strip
    double m_ScintTiO2ThicknessSide;

    //! length along z of the chimney hole
    double m_ChimneyLength;

    //! width of the chimney hole
    double m_ChimneyWidth;

    //! thickness of the chimney's iron cover plate
    double m_ChimneyCoverThickness;

    //! inner radius of the chimney housing
    double m_ChimneyHousingInnerRadius;

    //! outer radius of the chimney housing
    double m_ChimneyHousingOuterRadius;

    //! inner radius of the chimney shield
    double m_ChimneyShieldInnerRadius;

    //! outer radius of the chimney shield
    double m_ChimneyShieldOuterRadius;

    //! inner radius of the chimney pipe
    double m_ChimneyPipeInnerRadius;

    //! outer radius of the chimney pipe
    double m_ChimneyPipeOuterRadius;

    //! thickness of the radial rib that supports the solenoid / inner detectors
    double m_RibThickness;

    //! width of the cable-services channel at each end
    double m_CablesWidth;

    //! width of the central brace in the middle of the cable-services channel
    double m_BraceWidth;

    //! width of the central brace in the middle of the cable-services channel in the chimney sector
    double m_BraceWidthChimney;

    //! width of the innermost-module support plate
    double m_SupportPlateWidth;

    //! height of the innermost-module support plate
    double m_SupportPlateHeight;

    //! length of the innermost-module support plate
    double m_SupportPlateLength;

    //! length of the innermost-module support plate in the chimney sector
    double m_SupportPlateLengthChimney;

    //! width of the innermost-module support plate's bracket
    double m_BracketWidth;

    //! thickness of the innermost-module support plate's bracket
    double m_BracketThickness;

    //! length of the innermost-module support plate's bracket
    double m_BracketLength;

    //! width of the innermost-module support plate's bracket's rib
    double m_BracketRibWidth;

    //! thickness of the innermost-module support plate's bracket's rib
    double m_BracketRibThickness;

    //! distance from support plate's end of bracket
    double m_BracketInset;

    //! inner radius of the innermost-module support plate's bracket
    double m_BracketInnerRadius;

    //! angular width of the innermost-module support plate's bracket's cutout
    double m_BracketCutoutDphi;

    //! Flag to indicate whether layer contains RPCs (true) or scintillators (false)
    bool m_HasRPCs[NLAYER + 1];

    //! ReconstructionShift of each layer along x  in local system. displacement, not alignment here
    double m_LocalReconstructionShift_X[NSECTOR + 1][NLAYER + 1];

    //! ReconstructionShift of each layer along y  in local system. displacement, not alignment here
    double m_LocalReconstructionShift_Y[NSECTOR + 1][NLAYER + 1];

    //! ReconstructionShift of each layer along z  in local system. displacement, not alignment here
    double m_LocalReconstructionShift_Z[NSECTOR + 1][NLAYER + 1];

    //!number of the phi strips on each layer.
    int m_PhiStripNumber[NLAYER + 1];

    //!width of the phi strips on each layer.
    double m_PhiStripWidth[NLAYER + 1];

    //!width of the z strips on each layer.
    double m_ZStripWidth[NLAYER + 1];

    //!dLength of the phi scintillators.
    double m_PhiScintsDlength[NSCINTLAYER + 1][NSCINT + 1];

    //!dLength of the z scintillators.
    double m_ZScintsDlength[NSCINTLAYER + 1][NSCINT + 1];

    //! map of <volumeIDs, defined modules>, internal use only
    //std::map<int, bklm::Module> m_Modules;

    //! static pointer to the singleton instance of this class
    //static GeometryPar* m_Instance;

    //! optional comment
    std::string m_comment;

    ClassDef(BKLMGeometryPar, 2);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

