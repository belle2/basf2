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

#include <vector>
#include <string>

#include "TVector3.h"

namespace Belle2 {

//! The Class for BKLM Geometry Parameters
  /*! This class provides BKLM geometry parameters for simulation, reconstruction and so on.
      These parameters are obtained from Gearbox.
      Length is measured along the z axis.
      Height is measured in the xy plane along a radial axis at the centre of a polygon side.
      Width is measured in the xy plane along the tangent to a polygon side.
  */
  class BKLMGeometryPar {

  public:

    //! Constructor
    BKLMGeometryPar();

    //! Destructor
    virtual ~BKLMGeometryPar();

    //! Static method to get a reference to the BKLMGeometryPar instance.
    /*!
        \return A reference to an instance of this class.
    */

    static BKLMGeometryPar* Instance();

    //! Clear all geometry parameters
    void clear();

    //! Get geometry parameters from Gearbox.
    void read();

    //! to get the inner radius of specified (int input) layer
    const double layerRmin(int) const;

    //! to get the outer radius of specified (int input) layer
    const double layerRmax(int) const;

    //! To get the sizes (x,y,z) of the gap [=slot] of specified (int input) layer
    const TVector3 gapSize(int, bool) const;

    //! To get the sizes (x,y,z) of the detector module of specified (int input) layer
    const TVector3 moduleSize(int, bool) const;

    //! To get the sizes (x,y,z) of the detector module's readout of specified (int input) layer
    const TVector3 readoutSize(int, bool) const;

    //! To get the sizes (x,y,z) of the detector module's electrode of specified (int input) layer
    const TVector3 electrodeSize(int, bool) const;

    //! To get the sizes (x,y,z) of the detector module's gas gap (2 per module) of specified (int input) layer
    const TVector3 gasSize(int, bool) const;

    //! to get the midpoint radius of the gap [=slot] of specified (int input) layer
    const double gapR(int) const;

    //! to get the midpoint radius of the detector module of specified (int input) layer
    const double moduleR(int) const;

    //! to get the global rotation about Z of the entire BKLM
    const double &rotation(void) const {return m_rotation;}

    //! to get the global shift along Z of the entire BKLM
    const double &offsetZ(void) const {return m_offsetZ;}

    //! to get the starting angle of the polygon shape
    const double &phi(void) const {return m_phi;}

    //! to get the outer radius of the solenoid
    const double &rsolenoid(void) const {return m_rsolenoid;}

    //! to get the number of sides of the polygon [8]
    const int &nsides(void) const {return m_nsides;}

    //! to get the half-length along Z of the BKLM
    const double &length(void) const {return m_length;}

    //! to get the radius of the circle tangent to the sides of the outer polygon
    const double &rmax(void) const {return m_rmax;}

    //! to get the number of sectors(=nsides) of the BKLM
    const int &nsector(void) const {return m_nsides;}

    //! to get the number of modules in one sector
    const int &nlayer(void) const {return m_nlayer;}

    //! to get the nominal height of a layer's structural iron
    const double &height_iron(void) const {return m_height_iron;}

    //! to get the measured height of a layer's structural iron
    const double &height_iron_meas(void) const {return m_height_iron_meas;}

    //! to get the radius of the inner tangent circle of gap 0 (innermost)
    const double &rmin_gap0(void) const {return m_rmin_gap0;}

    //! to get the nominal height of the innermost gap
    const double &height_gap0(void) const {return m_height_gap0;}

    //! to get the measured height of the innermost gap
    const double &height_gap0_meas(void) const {return m_height_gap0_meas;}

    //! to get the half-width of the adjacent structural iron at the inner radius of innermost gap
    const double &dsx_gap0(void) const {return m_dsx_gap0;}

    //! to get the length along Z of the module gap
    const double &length_gap(void) const {return m_length_gap;}

    //! to get the nominal height of the outer gaps
    const double &height_gap(void) const {return m_height_gap;}

    //! to get the measured height of the outer gaps
    const double &height_gap_meas(void) const {return m_height_gap_meas;}

    //! to get the half-width of the adjacent structural iron at the inner radius of the outer gaps
    const double &dsx_gap(void) const {return m_dsx_gap;}

    //! to get the radius of the inner tangent circle of gap 1 (next-to-innermost)
    const double &rmin_gap1(void) {return m_rmin_gap1;}

    //! to get the length along Z of the module
    const double &length_mod(void) {return m_length_mod;}

    //! to get the length along Z of the module
    const double &length_mod_chimney(void) {return m_length_mod_chimney;}

    //! to get the height of the module's aluminum cover (2 per module)
    const double &height_cover(void) {return m_height_cover;}

    //! to get the height of the module's readout or ground copper plane (2 per module)
    const double &height_copper(void) {return m_height_copper;}

    //! to get the height of the module's transmission-line foam (2 per module)
    const double &height_foam(void) {return m_height_foam;}

    //! to get the height of the module's insulating mylar (2 per module)
    const double &height_mylar(void) {return m_height_mylar;}

    //! to get the height of the module's glass electrode (4 per module)
    const double &height_glass(void) {return m_height_glass;}

    //! to get the height of the module's gas gap (2 per module)
    const double &height_gas(void) {return m_height_gas;}

    //! to get the height of the module
    const double &height_mod(void) {return m_height_mod;}

    //! to get the width of the module's perimeter aluminum frame
    const double &width_frame(void) {return m_width_frame;}

    //! to get the width of the module's gas-gap's perimeter spacer
    const double &width_spacer(void) {return m_width_spacer;}

    //! to get the size of the chimney hole by layer
    const TVector3 size_chimney(int) const;

    //! to get the position of the chimney hole by layer
    const TVector3 pos_chimney(int) const;

    //! to get the thickness of the chimney cover plate
    const double &cover_chimney(void) {return m_cover_chimney;}

    //! to get the inner radius of the chimney housing
    const double &chimney_housing_rmin(void) {return m_chimney_housing_rmin;}

    //! to get the outer radius of the chimney housing
    const double &chimney_housing_rmax(void) {return m_chimney_housing_rmax;}

    //! to get the inner radius of the chimney radiation shield
    const double &chimney_shield_rmin(void) {return m_chimney_shield_rmin;}

    //! to get the outer radius of the chimney radiation shield
    const double &chimney_shield_rmax(void) {return m_chimney_shield_rmax;}

    //! to get the inner radius of the chimney pipe
    const double &chimney_pipe_rmin(void) {return m_chimney_pipe_rmin;}

    //! to get the outer radius of the chimney pipe
    const double &chimney_pipe_rmax(void) {return m_chimney_pipe_rmax;}

    //! to get the thickness of the radial rib that supports the solenoid / inner detectors
    const double &thickness_rib(void) {return m_thickness_rib;}

    //! to get the width of the cable-services channel at each end
    const double &width_cables(void) {return m_width_cables;}

    //! to get the width of the brace in the middle of the cable-services channel
    const double &width_brace(void) {return m_width_brace;}

    //! to get the width of the brace in the middle of the cable-services channel in the chimney sector
    const double &width_brace_chimney(void) {return m_width_brace_chimney;}

    //! to get the size of the layer-0 support plate
    const TVector3 size_support_plate(bool) const;

  private:

    //
    //Vessel
    //

    //! variable for the global rotation about Z of the BKLM
    double m_rotation;

    //! variable for the global offset along Z of the BKLM
    double m_offsetZ;

    //! variable for the starting angle of the polygon shape
    double m_phi;

    //! variable for the number of sides (=8 : octagonal)
    int m_nsides;

    //! variable for the outer radius of the solenoid
    double m_rsolenoid;

    //! variable for the radius of the circle tangent to the sides of the outer polygon
    double m_rmax;

    //! variable for the half-length along Z of the BKLM
    double m_length;

    //
    //BKLM layer
    //

    //! variable for the number of layers in one sector
    int m_nlayer;

    //! variable for the nominal height of a layer's structural iron
    double m_height_iron;

    //! variable for the measured height of a layer's stuctural iron
    double m_height_iron_meas;

    //! variable for the radius of the inner tangent circle of the innermost gap
    double m_rmin_gap0;

    //! variable for the nominal height of the innermost gap
    double m_height_gap0;

    //! variable for the measured height of the innermost gap
    double m_height_gap0_meas;

    //! variable for the height of layer 0: internal use only
    double m_height_layer0;

    //! variable for the height of a layer: internal use only
    double m_height_layer;

    //! variable for half-width of the adjacent structural iron at the inner radius of the innermost gap
    double m_dsx_gap0;

    //! variable for the length along Z of each gap
    double m_length_gap;

    //! variable for the nominal height of outer gaps
    double m_height_gap;

    //! variable for the measured height of outer gaps
    double m_height_gap_meas;

    //! variable for the half-width of the adjacent structural iron at the inner radius of outer gaps
    double m_dsx_gap;

    //! variable for the radius of the inner tangent circle of gap 1 (next-to-innermost)
    double m_rmin_gap1;

    //! variable for the length along Z of the module
    double m_length_mod;

    //! variable for the length along Z of the module in the chimney sector
    double m_length_mod_chimney;

    //! variable for the height of a detector module's aluminum cover
    double m_height_cover;

    //! variable for the height of a detector module's copper readout or ground plane
    double m_height_copper;

    //! variable for the height of a detector module's transmission-line foam
    double m_height_foam;

    //! variable for the height of a detector module's mylar insulation
    double m_height_mylar;

    //! variable for the height of a detector module's readout
    double m_height_readout;

    //! variable for the height of a detector module's glass electrode
    double m_height_glass;

    //! variable for the height of a detector module's gas gap
    double m_height_gas;

    //! variable for the height of a detector module
    double m_height_mod;

    //! variable for the width of a detector module's frame
    double m_width_frame;

    //! variable for the width of a detector module's spacer
    double m_width_spacer;

    //! variable for the length along Z of the chimney hole
    double m_length_chimney;

    //! variable for the height (per layer) of the chimney hole
    double m_height_chimney;

    //! variable for the width of the chimney hole
    double m_width_chimney;

    //! variable for the thickness of the chimney's iron cover plate
    double m_cover_chimney;

    //! variable for the x position of the chimney hole
    double m_x_chimney;

    //! variable for the y position of the chimney hole
    double m_y_chimney;

    //! variable for the z position of the chimney hole
    double m_z_chimney;

    //! variable for the inner radius of the chimney housing
    double m_chimney_housing_rmin;

    //! variable for the outer radius of the chimney housing
    double m_chimney_housing_rmax;

    //! variable for the inner radius of the chimney shield
    double m_chimney_shield_rmin;

    //! variable for the outer radius of the chimney shield
    double m_chimney_shield_rmax;

    //! variable for the inner radius of the chimney pipe
    double m_chimney_pipe_rmin;

    //! variable for the outer radius of the chimney pipe
    double m_chimney_pipe_rmax;

    //! variable for the thickness of the radial rib that supports the solenoid / inner detectors
    double m_thickness_rib;

    //! variable for the width of the cable-services channel at each end
    double m_width_cables;

    //! variable for the width of the central brace in the middle of the cable-services channel
    double m_width_brace;

    //! variable for the width of the central brace in the middle of the cable-services channel in the chimney sector
    double m_width_brace_chimney;

    //! variable for the width of the innermost-module support plate
    double m_width_support_plate;

    //! variable for the height of the innermost-module support plate
    double m_height_support_plate;

    //! static pointer to the singleton instance of this class
    static BKLMGeometryPar* m_BKLMGeometryParDB;

  };

//-----------------------------------------------------------------------------

} // end of namespace Belle2

#endif
