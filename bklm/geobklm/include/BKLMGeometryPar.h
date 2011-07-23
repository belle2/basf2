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

#include "bklm/geobklm/BKLMSector.h"
#include "bklm/geobklm/BKLMModule.h"

#include <vector>

#include "CLHEP/Vector/ThreeVector.h"

namespace Belle2 {

  //! Provides BKLM geometry parameters for simulation, reconstruction etc (from Gearbox)
  /*! Length is measured along the z axis.
      Height is measured in the r-phi plane along a radial axis at the centre of a polygon side.
      Width is measured in the r-phi plane along the tangent to a polygon side.
  */
  class BKLMGeometryPar {

    //! Output function
    //friend std::ostream& operator<<( std::ostream& out, const KlmGeo& geo );

  public:

    //! Static method to get a reference to the singleton BKLMGeometryPar instance
    static BKLMGeometryPar* instance();

    //! Clear all geometry parameters
    void clear();

    //! Get geometry parameters from Gearbox
    void read();

    //! Get the inner radius of specified layer
    const double getLayerInnerRadius(int layer) const;

    //! Get the outer radius of specified layer
    const double getLayerOuterRadius(int layer) const;

    //! Get the size (dx,dy,dz) of the gap [=slot] of specified layer
    const CLHEP::Hep3Vector getGapSize(int layer, bool hasChimney) const;

    //! Get the size (dx,dy,dz) of the detector module of specified layer
    const CLHEP::Hep3Vector getModuleSize(int layer, bool hasChimney) const;

    //! Get the size (dx,dy,dz) of the detector module's readout of specified layer
    const CLHEP::Hep3Vector getReadoutSize(int layer, bool hasChimney) const;

    //! Get the size (dx,dy,dz) of the detector module's electrode of specified layer
    const CLHEP::Hep3Vector getElectrodeSize(int layer, bool hasChimney) const;

    //! Get the size (dx,dy,dz) of the detector module's gas gaps of specified layer
    const CLHEP::Hep3Vector getGasSize(int layer, bool hasChimney) const;

    //! Get the radial midpoint of the gap of specified layer
    const double getGapMiddleRadius(int layer) const;

    //! Get the radial midpoint of the detector module of specified layer
    const double getModuleMiddleRadius(int layer) const;

    //! Get the global rotation angle about z of the entire BKLM
    const double getRotation(void) const { return m_Rotation; }

    //! Get the global shift along a of the entire BKLM
    const double getOffsetZ(void) const { return m_OffsetZ; }

    //! Get the starting angle of the BKLM's polygon shape
    const double getPhi(void) const { return m_Phi; }

    //! Get the outer radius of the solenoid
    const double getSolenoidOuterRadius(void) const { return m_SolenoidOuterRadius; }

    //! Get the number of sectors of the BKLM
    const int getNSector(void) const { return m_NSector; }

    //! Get the half-length along z of the BKLM
    const double getHalfLength(void) const { return m_HalfLength; }

    //! Get the radius of the inscribed circle of the outer polygon
    const double getOuterRadius(void) const { return m_OuterRadius; }

    //! Get the number of modules in one sector
    const int getNLayer(void) const { return m_NLayer; }

    //! Get the nominal height of a layer's structural iron
    const double getIronNominalHeight(void) const { return m_IronNominalHeight; }

    //! Get the actual height of a layer's structural iron
    const double getIronActualHeight(void) const { return m_IronActualHeight; }

    //! Get the radius of the inner tangent circle of gap 0 (innermost)
    const double getGap0InnerRadius(void) const { return m_Gap0InnerRadius; }

    //! Get the nominal height of the innermost gap
    const double getGap0NominalHeight(void) const { return m_Gap0NominalHeight; }

    //! Get the actual height of the innermost gap
    const double getGap0ActualHeight(void) const { return m_Gap0ActualHeight; }

    //! Get the width (at the outer radius) of the adjacent structural iron on either side of innermost gap
    const double getGap0IronWidth(void) const { return m_Gap0IronWidth; }

    //! Get the length along z of the module gap
    const double getGapLength(void) const { return m_GapLength; }

    //! Get the nominal height of the outer gaps
    const double getGapNominalHeight(void) const { return m_GapNominalHeight; }

    //! Get the actual height of the outer gaps
    const double getGapActualHeight(void) const { return m_GapActualHeight; }

    //! Get the width (at the outer radius) of the adjacent structural iron on either side of a gap
    const double getGapIronWidth(void) const { return m_GapIronWidth; }

    //! Get the radius of the inner tangent circle of gap 1 (next-to-innermost)
    const double getGapInnerRadius(void) { return m_GapInnerRadius; }

    //! Get the length along z of the module
    const double getModuleLength(void) { return m_ModuleLength; }

    //! Get the length along z of the module
    const double getModuleLengthChimney(void) { return m_ModuleLengthChimney; }

    //! Get the height of the module's aluminum cover (2 per module)
    const double getModuleCoverHeight(void) { return m_ModuleCoverHeight; }

    //! Get the height of the module's readout or ground copper plane (4 per module)
    const double getModuleCopperHeight(void) { return m_ModuleCopperHeight; }

    //! Get the height of the module's transmission-line foam (2 per module)
    const double getModuleFoamHeight(void) { return m_ModuleFoamHeight; }

    //! Get the height of the module's insulating mylar (2 per module)
    const double getModuleMylarHeight(void) { return m_ModuleMylarHeight; }

    //! Get the height of the module's glass electrode (4 per module)
    const double getModuleGlassHeight(void) { return m_ModuleGlassHeight; }

    //! Get the height of the module's gas gap (2 per module)
    const double getModuleGasHeight(void) { return m_ModuleGasHeight; }

    //! Get the height of the module
    const double getModuleHeight(void) { return m_ModuleHeight; }

    //! Get the width of the module's perimeter aluminum frame
    const double getModuleFrameWidth(void) { return m_ModuleFrameWidth; }

    //! Get the width of the module's gas-gap's perimeter spacer
    const double getModuleGasSpacerWidth(void) { return m_ModuleGasSpacerWidth; }

    //! Get the size of the border between a detector module's perimeter and electrode
    const double getModuleElectrodeBorder(void) { return m_ModuleElectrodeBorder; }

    //! Get the size of the chimney hole in the specified layer
    const CLHEP::Hep3Vector getChimneySize(int layer) const;

    //! Get the position of the chimney hole in the specified layer
    const CLHEP::Hep3Vector getChimneyPosition(int layer) const;

    //! Get the thickness of the chimney cover plate
    const double getChimneyCoverThickness(void) { return m_ChimneyCoverThickness; }

    //! Get the inner radius of the chimney housing
    const double getChimneyHousingInnerRadius(void) { return m_ChimneyHousingInnerRadius; }

    //! Get the outer radius of the chimney housing
    const double getChimneyHousingOuterRadius(void) { return m_ChimneyHousingOuterRadius; }

    //! Get the inner radius of the chimney radiation shield
    const double getChimneyShieldInnerRadius(void) { return m_ChimneyShieldInnerRadius; }

    //! Get the outer radius of the chimney radiation shield
    const double getChimneyShieldOuterRadius(void) { return m_ChimneyShieldOuterRadius; }

    //! Get the inner radius of the chimney pipe
    const double getChimneyPipeInnerRadius(void) { return m_ChimneyPipeInnerRadius; }

    //! Get the outer radius of the chimney pipe
    const double getChimneyPipeOuterRadius(void) { return m_ChimneyPipeOuterRadius; }

    //! Get the thickness of the radial rib that supports the solenoid / inner detectors
    const double getRibThickness(void) { return m_RibThickness; }

    //! Get the width of the cable-services channel at each end
    const double getCablesWidth(void) { return m_CablesWidth; }

    //! Get the width of the brace in the middle of the cable-services channel
    const double getBraceWidth(void) { return m_BraceWidth; }

    //! Get the width of the brace in the middle of the cable-services channel in the chimney sector
    const double getBraceWidthChimney(void) { return m_BraceWidthChimney; }

    //! Get the size of the layer-0 support plate
    const CLHEP::Hep3Vector getSupportPlateSize(bool) const;

    //! Get the pointer to the definition of a sector
    const BKLMSector* findSector(int frontBack, int sector) const;

    //! Print all sector and module definitions
    void printTree(void) const;

  private:

    //! Hidden constructor
    BKLMGeometryPar();

    //! Hidden copy constructor
    BKLMGeometryPar(BKLMGeometryPar&);

    //! Hidden copy assignment
    BKLMGeometryPar& operator=(const BKLMGeometryPar&);

    //! Hidden destructor
    ~BKLMGeometryPar();

    //! variable for the global rotation about z of the BKLM
    double m_Rotation;

    //! variable for the global offset along z of the BKLM
    double m_OffsetZ;

    //! variable for the starting angle of the polygon shape
    double m_Phi;

    //! variable for the number of sectors (=8 : octagonal)
    int m_NSector;

    //! variable for the outer radius of the solenoid
    double m_SolenoidOuterRadius;

    //! variable for the radius of the circle tangent to the sides of the outer polygon
    double m_OuterRadius;

    //! variable for the half-length along z of the BKLM
    double m_HalfLength;

    //! variable for the number of layers in one sector
    int m_NLayer;

    //! variable for the nominal height of a layer's structural iron
    double m_IronNominalHeight;

    //! variable for the actual height of a layer's stuctural iron
    double m_IronActualHeight;

    //! variable for the radius of the inner tangent circle of the innermost gap
    double m_Gap0InnerRadius;

    //! variable for the nominal height of the innermost gap
    double m_Gap0NominalHeight;

    //! variable for the actual height of the innermost gap
    double m_Gap0ActualHeight;

    //! variable for the height of layer 0: internal use only
    double m_Layer0Height;

    //! variable for the height of a layer: internal use only
    double m_LayerHeight;

    //! variable for width (at the outer radius) of the adjacent structural iron on either side of innermost gap
    double m_Gap0IronWidth;

    //! variable for the length along z of each gap
    double m_GapLength;

    //! variable for the nominal height of outer gaps
    double m_GapNominalHeight;

    //! variable for the actual height of outer gaps
    double m_GapActualHeight;

    //! variable for the width (at the outer radius) of the adjacent structural iron on either side of a gap
    double m_GapIronWidth;

    //! variable for the radius of the inner tangent circle of virtual gap 0 (assuming equal-height layers)
    double m_GapInnerRadius;

    //! variable for the length along z of the module
    double m_ModuleLength;

    //! variable for the length along z of the module in the chimney sector
    double m_ModuleLengthChimney;

    //! variable for the height of a detector module's aluminum cover
    double m_ModuleCoverHeight;

    //! variable for the height of a detector module's copper readout or ground plane
    double m_ModuleCopperHeight;

    //! variable for the height of a detector module's transmission-line foam
    double m_ModuleFoamHeight;

    //! variable for the height of a detector module's mylar insulation
    double m_ModuleMylarHeight;

    //! variable for the height of a detector module's readout
    double m_ModuleReadoutHeight;

    //! variable for the height of a detector module's glass electrode
    double m_ModuleGlassHeight;

    //! variable for the height of a detector module's gas gap
    double m_ModuleGasHeight;

    //! variable for the height of a detector module
    double m_ModuleHeight;

    //! variable for the width of a detector module's frame
    double m_ModuleFrameWidth;

    //! variable for the width of a detector module's spacer
    double m_ModuleGasSpacerWidth;

    //! variable for the size of the border between a detector module's perimeter and electrode
    double m_ModuleElectrodeBorder;

    //! variable for the length along z of the chimney hole
    double m_ChimneyLength;

    //! variable for the width of the chimney hole
    double m_ChimneyWidth;

    //! variable for the thickness of the chimney's iron cover plate
    double m_ChimneyCoverThickness;

    //! variable for the inner radius of the chimney housing
    double m_ChimneyHousingInnerRadius;

    //! variable for the outer radius of the chimney housing
    double m_ChimneyHousingOuterRadius;

    //! variable for the inner radius of the chimney shield
    double m_ChimneyShieldInnerRadius;

    //! variable for the outer radius of the chimney shield
    double m_ChimneyShieldOuterRadius;

    //! variable for the inner radius of the chimney pipe
    double m_ChimneyPipeInnerRadius;

    //! variable for the outer radius of the chimney pipe
    double m_ChimneyPipeOuterRadius;

    //! variable for the thickness of the radial rib that supports the solenoid / inner detectors
    double m_RibThickness;

    //! variable for the width of the cable-services channel at each end
    double m_CablesWidth;

    //! variable for the width of the central brace in the middle of the cable-services channel
    double m_BraceWidth;

    //! variable for the width of the central brace in the middle of the cable-services channel in the chimney sector
    double m_BraceWidthChimney;

    //! variable for the width of the innermost-module support plate
    double m_SupportPlateWidth;

    //! variable for the height of the innermost-module support plate
    double m_SupportPlateHeight;

    //! vector of pointers to defined sectors
    std::vector<BKLMSector*> m_Sectors;

    //! static pointer to the singleton instance of this class
    static BKLMGeometryPar* m_Instance;

  };

} // end of namespace Belle2

#endif // BKLMGEOMETRYPAR_H
