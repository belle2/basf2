/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMGEOMETRYPAR_H
#define BKLMGEOMETRYPAR_H

#include "bklm/geometry/Module.h"
#include "bklm/dbobjects/BKLMGeometryPar.h"

#include <vector>
#include <map>

#include "CLHEP/Vector/ThreeVector.h"

namespace Belle2 {

  class GearDir;
  class BkgSensitiveDetector;

  namespace bklm {

    //! Provides BKLM geometry parameters for simulation, reconstruction etc (from Gearbox or DataBase)
    /*! Length is measured along the z axis.
        Height is measured in the r-phi plane along a radial axis at the centre of a polygon side.
        Width is measured in the r-phi plane along the tangent to a polygon side.
    */
    class GeometryPar {

    public:

      //! Static method to get a reference to the singleton GeometryPar instance
      static GeometryPar* instance(void);

      //! Static method to initialize and then get a reference to the singleton GeometryPar instance
      static GeometryPar* instance(const GearDir&);

      //! Static method to initialize and then get a reference to the singleton GeometryPar instance from database
      static GeometryPar* instance(const BKLMGeometryPar&);

      //! Get the beam background study flag
      bool doBeamBackgroundStudy(void) const { return m_DoBeamBackgroundStudy; }

      //! Get the beam background study flag
      BkgSensitiveDetector* getBkgSensitiveDetector(void) const { return m_BkgSensitiveDetector; }

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

      //! Get the sign (+/-1) of scintillator-envelope's shift along y axis within its enclosing module for MPPC placement
      //! -1: shift envelope along -y to place MPPCs at +y, +1: shift envelope along +y to place MPPCs at -y
      int getScintEnvelopeOffsetSign(int layer) const { return m_ScintEnvelopeOffsetSign[layer - 1]; }

      //! Get the radial offset of the scintillator detector module's active envelope due to difference in polystyrene-sheet thicknesses
      double getPolystyreneOffsetX(void) const;

      //! Get the thickness of the inactive TiO2-polystyrene coating on top (broad) surface of a scintillator strip
      double getScintTiO2ThicknessTop(void) const { return m_ScintTiO2ThicknessTop; }

      //! Get the thickness of the inactive TiO2-polystyrene coating on side (short) surface of a scintillator strip
      double getScintTiO2ThicknessSide(void) const { return m_ScintTiO2ThicknessSide; }

      //! Get the height of the entire volume of a scintillator strip (including TiO2 coating)
      double getScintHalfHeight(void) const { return 0.5 * m_ScintHeight; }

      //! Get the height of the entire volume of a scintillator strip (including TiO2 coating)
      double getScintHalfWidth(void) const { return 0.5 * m_ScintWidth; }

      //! Get the radius of the cylindrical central bore in a scintillator strip
      double getScintBoreRadius(void) const { return m_ScintBoreRadius; }

      //! Get the radius of the cylindrical central WLS fiber in a scintillator strip
      double getScintFiberRadius(void) const { return m_ScintFiberRadius; }

      //! Get the radial midpoint of the gap of specified layer
      double getGapMiddleRadius(int layer) const;

      //! Get the radial midpoint of the detector module of specified layer
      double getModuleMiddleRadius(int layer) const;

      //! Get the radial midpoint of the detector module's active volume of specified layer
      double getActiveMiddleRadius(int fb, int sector, int layer) const;

      //! Get the flip (180-degrees about z axis) of a particular BKLM module
      bool getModuleFlip(int fb, int sector, int layer) const { return m_IsFlipped[fb - 1][sector - 1][layer - 1]; }

      //! Get the global rotation angle about z of the entire BKLM
      double getRotation(void) const { return m_Rotation; }

      //! Get the global shift along a of the entire BKLM
      double getOffsetZ(void) const { return m_OffsetZ; }

      //! Get the starting angle of the BKLM's polygon shape
      double getPhi(void) const { return m_Phi; }

      //! Get the outer radius of the solenoid
      double getSolenoidOuterRadius(void) const { return m_SolenoidOuterRadius; }

      //! Get the number of sectors of the BKLM
      int getNSector(void) const { return m_NSector; }

      //! Get the half-length along z of the BKLM
      double getHalfLength(void) const { return m_HalfLength; }

      //! Get the radius of the inscribed circle of the outer polygon
      double getOuterRadius(void) const { return m_OuterRadius; }

      //! Get the number of modules in one sector
      int getNLayer(void) const { return m_NLayer; }

      //! Get the nominal height of a layer's structural iron
      double getIronNominalHeight(void) const { return m_IronNominalHeight; }

      //! Get the actual height of a layer's structural iron
      double getIronActualHeight(void) const { return m_IronActualHeight; }

      //! Get the radius of the inner tangent circle of gap 0 (innermost)
      double getGap1InnerRadius(void) const { return m_Gap1InnerRadius; }

      //! Get the nominal height of the innermost gap
      double getGap1NominalHeight(void) const { return m_Gap1NominalHeight; }

      //! Get the actual height of the innermost gap
      double getGap1ActualHeight(void) const { return m_Gap1ActualHeight; }

      //! Get the width (at the outer radius) of the adjacent structural iron on either side of innermost gap
      double getGap1IronWidth(void) const { return m_Gap1IronWidth; }

      //! Get the length along z of the module gap
      double getGapLength(void) const { return m_GapLength; }

      //! Get the nominal height of the outer gaps
      double getGapNominalHeight(void) const { return m_GapNominalHeight; }

      //! Get the actual height of the outer gaps
      double getGapActualHeight(void) const { return m_GapActualHeight; }

      //! Get the width (at the outer radius) of the adjacent structural iron on either side of a gap
      double getGapIronWidth(void) const { return m_GapIronWidth; }

      //! Get the radius of the inner tangent circle of gap 1 (next-to-innermost)
      double getGapInnerRadius(void) const { return m_GapInnerRadius; }

      //! Get the number of z-measuring cathode strips in an RPC module
      int getNZStrips(bool isChimney) const { return (isChimney ? m_NZStripsChimney : m_NZStrips); }

      //! Get the number of phi-measuring cathode strips in an RPC module
      int getNPhiStrips(int layer) const;

      //! Get the number of z-measuring scintillators in a scintillator module
      int getNZScints(bool isChimney) const { return (isChimney ? m_NZScintsChimney : m_NZScints); }

      //! Get the number of phi-measuring scintillators in a scintillator module
      int getNPhiScints(int layer) const;

      //! Get the length along z of the module
      double getModuleLength(void) const { return m_ModuleLength; }

      //! Get the length along z of the module
      double getModuleLengthChimney(void) const { return m_ModuleLengthChimney; }

      //! Get the height of the module's aluminum cover (2 per module)
      double getModuleCoverHeight(void) const { return m_ModuleCoverHeight; }

      //! Get the height of the module's readout or ground copper plane (4 per module)
      double getModuleCopperHeight(void) const { return m_ModuleCopperHeight; }

      //! Get the height of the module's transmission-line foam (2 per module)
      double getModuleFoamHeight(void) const { return m_ModuleFoamHeight; }

      //! Get the height of the module's insulating mylar (2 per module)
      double getModuleMylarHeight(void) const { return m_ModuleMylarHeight; }

      //! Get the height of the module's glass electrode (4 per module)
      double getModuleGlassHeight(void) const { return m_ModuleGlassHeight; }

      //! Get the height of the module's gas gap (2 per module)
      double getModuleGasHeight(void) const { return m_ModuleGasHeight; }

      //! Get the height of the module
      double getModuleHeight(void) const { return m_ModuleHeight; }

      //! Get the width of the module's perimeter aluminum frame
      double getModuleFrameWidth(void) const { return m_ModuleFrameWidth; }

      //! Get the thickness of the module's perimeter aluminum frame
      double getModuleFrameThickness(void) const { return m_ModuleFrameThickness; }

      //! Get the width of the module's gas-gap's perimeter spacer
      double getModuleGasSpacerWidth(void) const { return m_ModuleGasSpacerWidth; }

      //! Get the size of the border between a detector module's perimeter and electrode
      double getModuleElectrodeBorder(void) const { return m_ModuleElectrodeBorder; }

      //! Get the size of the chimney hole in the specified layer
      const CLHEP::Hep3Vector getChimneyHalfSize(int layer) const;

      //! Get the position of the chimney hole in the specified layer
      const CLHEP::Hep3Vector getChimneyPosition(int layer) const;

      //! Get the thickness of the chimney cover plate
      double getChimneyCoverThickness(void) const { return m_ChimneyCoverThickness; }

      //! Get the inner radius of the chimney housing
      double getChimneyHousingInnerRadius(void) const { return m_ChimneyHousingInnerRadius; }

      //! Get the outer radius of the chimney housing
      double getChimneyHousingOuterRadius(void) const { return m_ChimneyHousingOuterRadius; }

      //! Get the inner radius of the chimney radiation shield
      double getChimneyShieldInnerRadius(void) const { return m_ChimneyShieldInnerRadius; }

      //! Get the outer radius of the chimney radiation shield
      double getChimneyShieldOuterRadius(void) const { return m_ChimneyShieldOuterRadius; }

      //! Get the inner radius of the chimney pipe
      double getChimneyPipeInnerRadius(void) const { return m_ChimneyPipeInnerRadius; }

      //! Get the outer radius of the chimney pipe
      double getChimneyPipeOuterRadius(void) const { return m_ChimneyPipeOuterRadius; }

      //! Get the thickness of the radial rib that supports the solenoid / inner detectors
      double getRibThickness(void) const { return m_RibThickness; }

      //! Get the width of the cable-services channel at each end
      double getCablesWidth(void) const { return m_CablesWidth; }

      //! Get the width of the brace in the middle of the cable-services channel
      double getBraceWidth(void) const { return m_BraceWidth; }

      //! Get the width of the brace in the middle of the cable-services channel in the chimney sector
      double getBraceWidthChimney(void) const { return m_BraceWidthChimney; }

      //! Get the size of the layer-0 support plate
      const CLHEP::Hep3Vector getSupportPlateHalfSize(bool) const;

      //! Get the width of the layer-0 support plate's bracket
      double getBracketWidth(void) const { return m_BracketWidth; }

      //! Get the thickness of the layer-0 support plate's bracket
      double getBracketThickness(void) const { return m_BracketThickness; }

      //! Get the length of the layer-0 support plate's bracket
      double getBracketLength(void) const { return m_BracketLength; }

      //! Get the width of the layer-0 support plate's bracket's rib
      double getBracketRibWidth(void) const { return m_BracketRibWidth; }

      //! Get the thickness of the layer-0 support plate's bracket's rib
      double getBracketRibThickness(void) const { return m_BracketRibThickness; }

      //! Get the inner radius of the layer-0 support plate's bracket
      double getBracketInnerRadius(void) const { return m_BracketInnerRadius; }

      //! Get the position of a layer-0 support plate's bracket
      double getBracketZPosition(int, bool) const;

      //! Get the angular width of the layer-0 support plate's bracket's cutout
      double getBracketCutoutDphi(void) const { return m_BracketCutoutDphi; }

      //! Get the number of preamplifier readout stations
      int getNReadoutStation(void) const { return m_NReadoutStation; }

      //! Get the selector for phi (true) or z (false) readout station
      bool getReadoutStationIsPhi(int station) const { return m_ReadoutStationIsPhi[station - 1]; }

      //! Get the position of each readout station
      double getReadoutStationPosition(int station) const { return m_ReadoutStationPosition[station - 1]; }

      //! Get the size (dx,dy,dz) of the readout container
      const CLHEP::Hep3Vector getReadoutContainerHalfSize(void) const;

      //! Get the size (dx,dy,dz) of the carrier card
      const CLHEP::Hep3Vector getReadoutCarrierHalfSize(void) const;

      //! Get the size (dx,dy,dz) of the preamplifier card
      const CLHEP::Hep3Vector getReadoutPreamplifierHalfSize(void) const;

      //! Get the number of preamplifier positions along the length of the carrier card
      int getNReadoutPreamplifierPosition(void) const { return m_ReadoutPreamplifierPosition.size(); }

      //! Get the position of a preamplifier along the length of the carrier card
      double getReadoutPreamplifierPosition(int preamp) const { return m_ReadoutPreamplifierPosition[preamp - 1]; }

      //! Get the size (dx,dy,dz) of the readout connectors pair
      const CLHEP::Hep3Vector getReadoutConnectorsHalfSize(void) const;

      //! Get the position of the readout connectors pair along the length of the carrier card
      double getReadoutConnectorsPosition(void) const { return m_ReadoutConnectorsPosition; }

      //! Get the MPPC housing radius
      double getMPPCHousingRadius(void) const { return m_MPPCHousingRadius; }

      //! Get the MPPC housing half-length
      double getMPPCHousingHalfLength(void) const { return 0.5 * m_MPPCHousingLength; }

      //! Get the MPPC half-length
      double getMPPCHalfLength(void) const { return 0.5 * m_MPPCLength; }

      //! Get the MPPC half-width
      double getMPPCHalfWidth(void) const { return 0.5 * m_MPPCWidth; }

      //! Get the MPPC half-height
      double getMPPCHalfHeight(void) const { return 0.5 * m_MPPCHeight; }

      //! Determine if the sensitive detectors in a given layer are RPCs (=true) or scintillators (=false)
      bool hasRPCs(int layer) const;

      //! Get the pointer to the definition of a module
      const Module* findModule(bool isForward, int sector, int layer) const;

      //! Get the pointer to the definition of a module
      const Module* findModule(int layer, bool hasChimney) const;

      //! Get the alignment transformation of a module
      const HepGeom::Transform3D getModuleAlignment(bool isForward, int sector, int layer) const;

      //! Get the displacement transformation of a module
      const HepGeom::Transform3D getModuleDisplacedGeo(bool isForward, int sector, int layer) const;

    private:

      //! Hidden constructor
      explicit GeometryPar(const GearDir&);

      //! Hidden constructor from database
      explicit GeometryPar(const Belle2::BKLMGeometryPar&);

      //! Hidden copy constructor
      GeometryPar(GeometryPar&);

      //! Hidden copy assignment
      GeometryPar& operator=(const GeometryPar&);

      //! Hidden destructor
      ~GeometryPar();

      //! Clear all geometry parameters
      void clear();

      //! Get geometry parameters from Gearbox
      void read(const GearDir&);

      //! Get geometry parameters from Conditions Database
      void readFromDB(const BKLMGeometryPar&);

      //! Calculate additional geometry parameters
      void calculate();

      //! Initialize and Updates alignment parameters from DB for reconstruction, that is for Module construction
      //! and registers itself for subsequent updates of DB objects to keep the hierarchy up-to-date.
      //! note that alignment are supposed to only for reconstruction correction, but NOT the geometry constructor
      void readAlignmentFromDB();

      //! Initialize and Updates displacements parameters from DB for geometry constructor,
      //! registers itself for subsequent updates of DB objects to keep the hierarchy up-to-date.
      void readDisplacedGeoFromDB();

      //! Convert 6 rigid body params (alignment/displacement) to corresponding Transform3D
      //! Angles in radians, length units in centimeters.
      //! three angles are defined as the intrinsic rotations, that is around u (alpha) --> v' (beta) --> w'' (gamma) axis
      //! note this is equivalent with extrinsic rotation with the order w (gamma)--> v(beta) --> u (alpha)
      HepGeom::Transform3D getTransformFromRigidBodyParams(double dU, double dV, double dW, double dAlpha, double dBeta, double dGamma);

      //! Flag for enabling beam background study (=use bkg sensitive-detector function too)
      bool m_DoBeamBackgroundStudy;

      //! Pointer to object that creates BeamBkgHits for BKLM
      BkgSensitiveDetector* m_BkgSensitiveDetector;

      //! Global rotation about z of the BKLM
      double m_Rotation;

      //! Global rotation angle of a sector
      double m_SectorRotation[2][NSECTOR];

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

      //! Radius of the inner tangent circle of the innermost gap
      double m_Gap1InnerRadius;

      //! Nominal height of the innermost gap
      double m_Gap1NominalHeight;

      //! Actual height of the innermost gap
      double m_Gap1ActualHeight;

      //! Height of layer 0: internal use only
      double m_Layer1Height;

      //! Height of a layer: internal use only
      double m_LayerHeight;

      //! Width (at the outer radius) of the adjacent structural iron on either side of innermost gap
      double m_Gap1IronWidth;

      //! Length along z of each gap
      double m_GapLength;

      //! Nominal height of outer gaps
      double m_GapNominalHeight;

      //! Actual height of outer gaps
      double m_GapActualHeight;

      //! Width (at the outer radius) of the adjacent structural iron on either side of a gap
      double m_GapIronWidth;

      //! Radius of the inner tangent circle of virtual gap 0 (assuming equal-height layers)
      double m_GapInnerRadius;

      //! Number of phi-readout RPC strips in each layer
      int m_NPhiStrips[NLAYER];

      //! Number of phi-readout scintillators in each layer
      int m_NPhiScints[NLAYER];

      //! number of z-measuring cathode strips in a standard RPC module
      int m_NZStrips;

      //! number of z-measuring cathode strips in a chimney-sector RPC module
      int m_NZStripsChimney;

      //! number of z-measuring scintillators in a standard scintillator module
      int m_NZScints;

      //! number of z-measuring scintillators in a chimney-sector scintillator module
      int m_NZScintsChimney;

      //! Sign (+/-1) of scintillator-envelope's shift along y axis within its enclosing module for MPPC placement
      //! -1: shift envelope along -y to place MPPCs at +y, +1: shift envelope along +y to place MPPCs at -y
      int m_ScintEnvelopeOffsetSign[NLAYER];

      //! Width of the phi strips on each layer
      double m_PhiStripWidth[NLAYER];

      //! Width of the z strips on each layer
      double m_ZStripWidth[NLAYER];

      //! Shortening of the nominal length of the z scintillators
      double m_ZScintDLength[NLAYER][NZSCINT];

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

      //! Number of preamplifier readout stations
      int m_NReadoutStation;

      //! Selector for phi (true) or z (false) readout station
      bool m_ReadoutStationIsPhi[NSTATION];

      //! Position of each readout station along its relevant axis
      double m_ReadoutStationPosition[NSTATION];

      //! Length of the readout station's container
      double m_ReadoutContainerLength;

      //! Width of the readout station's container
      double m_ReadoutContainerWidth;

      //! Height of the readout station's container
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

      //! Flag to indicate whether layer contains RPCs (true) or scintillators (false)
      bool m_HasRPCs[NLAYER];

      //! Reconstruction dx in local system. displacement, not alignment
      double m_LocalReconstructionShiftX[2][NSECTOR][NLAYER];

      //! Reconstruction dy in local system. displacement, not alignment
      double m_LocalReconstructionShiftY[2][NSECTOR][NLAYER];

      //! Reconstruction dz in local system. displacement, not alignment
      double m_LocalReconstructionShiftZ[2][NSECTOR][NLAYER];

      //! Flag to indicate whether a module is flipped (true) or not (false) by 180 degrees about the z axis
      bool m_IsFlipped[2][NSECTOR][NLAYER];

      //! map of <volumeIDs, pointers to defined modules>
      std::map<int, Module*> m_Modules;

      //! map of <volumeIDs, alignment Transform3D>
      std::map<int, HepGeom::Transform3D> m_Alignments;

      //! map of <volumeIDs, displacement Transform3D>
      std::map<int, HepGeom::Transform3D> m_Displacements;

      //! static pointer to the singleton instance of this class
      static GeometryPar* m_Instance;

    };

  } // end of namespace bklm

} // end of namespace Belle2

#endif // BKLMGEOMETRYPAR_H
