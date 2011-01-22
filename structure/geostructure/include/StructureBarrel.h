/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef STRUCTUREBARREL_H
#define STRUCTUREBARREL_H

#include <vector>
#include <string>

#include "TVector3.h"

namespace Belle2 {

//! The Class for CDC Geometry Parameters
  /*! This class provides CDC gemetry paramters for simulation, reconstruction and so on.
      These parameters are gotten from gearbox.
  */
  class StructureBarrel {

  public:

    //! Constructor
    StructureBarrel();

    //! Destructor
    virtual ~StructureBarrel();

    //! Static method to get a reference to the CDCGeometryPar instance.
    /*!
        \return A reference to an instance of this class.
    */

    ///////////////static CDCGeometryPar* Instance();

    //! Clears
    void clear();

    //! Gets geometry parameters from gearbox.
    void read();

    //! To get the sizes (x,y,z) of the mother vessel (Iron) for barrel KLM module slot
    const TVector3 barrelSlotSize(int) const;

    //! To get the sizes (x,y,z) of the barrel KLM module slot
    const TVector3 barrelModuleSize(int) const;

    //! to get the radial position of the mother vessel of specified (int input) layer
    const double barrelSlotR(int) const;

    //! to get the radial position of module of specified (int input) layer
    const double barrelModuleR(int) const;

    //! to get the material name for the entire Barrel Structure
    const std::string &matname(void) const {return m_matname;}

    //! to get the starting angle of the octagon shape
    const double &phi(void) const {return m_phi;}

    //! to get the opening angle (shape is extended from phi to phi+dphi)
    const double &dphi(void) const {return m_dphi;}

    //! to get the number of sides (=8 : octagonal)
    const int &nsides(void) const {return m_nsides;}

    //! to get the number of booundaries perpendicular to the z-axis
    const int &nBoundary(void) const {return m_nBoundary;}

    //! to get the z coordinate of the section specified by input id (=boundary id)
    const double &z(int id) const {return m_z[id];}

    //! to get the radius of the circle tangent to the sides of the inner polygon
    const double &rmin(int id) const {return m_rmin[id];}

    //! to get the radius of the circle tangent to the sides of the outer polygon
    const double &rmax(int id) const {return m_rmax[id];}

    //! to get the material name for the slot for the Barrel KLM module
    const std::string &matname4slot(void) const {return m_matname4slot;}

    //! to get the material name for the Barrel KLM module
    const std::string &matname4module(void) const {return m_matname4module;}

    //! to get the number of sectors of the Barrel KLM (=nsides : 8)
    const int &nsector(void) const {return m_nsector;}

    //! to get the number of modules in one sector
    const int &nmodule(void) const {return m_nmodule;}

    //! to get the thickness of the mother vessel for the Barrel KLM module slot
    const double &thick_biron(void) const {return m_thick_biron;}

    //! to get the difference between the designed thickness and the real measurement
    const double &thick_biron_meas(void) const {return m_thick_biron_meas;}

    //! to get the radial position of the 0-th KLM barrel module slot (= 1st layer)
    const double &rmin_bslot0(void) const {return m_rmin_bslot0;}

    //! to get the thickness of the 0-th KLM barrel module slot (= 1st layer)
    const double &thick_bslot0(void) const {return m_thick_bslot0;}

    //! to get the difference between the designed thickness of the 0-th slot and the real measurement
    const double &thick_bslot0_meas(void) const {return m_thick_bslot0_meas;}

    //! to get the parameter which is used to calculate the Barrel KLM module 0-th slot
    const double &dsx_bslot0(void) const {return m_dsx_bslot0;}

    //! to get the length of the Barrel KLM module slot
    const double &length_bslot(void) const {return m_length_bslot;}

    //! to get the thickness of the Barrel KLM module slot
    const double &thick_bslot(void) const {return m_thick_bslot;}

    //! to get the difference between the designed thickness of the module slot and the real measurement
    const double &thick_bslot_meas(void) const {return m_thick_bslot_meas;}

    //! to get the parameter which is used to calculate the Barrel KLM module slot
    const double &dsx_bslot(void) const {return m_dsx_bslot;}

    //! to get the radial position of the 1st KLM barrel module slot (= 2nd layer)
    const double &rmin_bslot1(void) {return m_rmin_bslot1;}

    //! to get the thickness of the Barrel KLM module
    const double &thick_bmod(void) {return m_thick_bmod;}

    //! to get the length of the Barrel KLM module
    const double &length_bmod(void) {return m_length_bmod;}

    //! to get the length of the Barrel KLM module with taking chimney hole into accout : not used at this moment
    const double &length_chole(void) {return m_length_chole;}

    //! to get a parameter for chimney hole : not used at this moment
    const double &cov_chole(void) {return m_cov_chole;}

    //! to get the width of the chimney hole : not used at this moment
    const double &width_chole(void) {return m_width_chole;}

  private:
    //
    //Barrel Vessel
    //

    //! variable for the material name for the entire Barrel Structure
    std::string m_matname;

    //! variable for the starting angle of the octagon shape
    double m_phi;

    //! variable for the opening angle (shape is extended from phi to phi+dphi)
    double m_dphi;

    //! variable for the number of sides (=8 : octagonal)
    int m_nsides;

    //! variable for the number of booundaries perpendicular to the z-axis
    int m_nBoundary;

    //! variable for the z coordinate of the section
    double m_z[11];

    //! variable for the radius of the circle tangent to the sides of the inner polygon
    double m_rmin[11];

    //! variable for the radius of the circle tangent to the sides of the outer polygon
    double m_rmax[11];

    //
    //Barrel slot for KLM module
    //
    //! variable for the material name for the slot for the Barrel KLM module
    std::string m_matname4slot;

    //! variable for the material name for the Barrel KLM module
    std::string m_matname4module;

    //! variable for the number of sectors of the Barrel KLM (=nsides : 8)
    int m_nsector;

    //! variable for the number of modules in one sector
    int m_nmodule;

    //! variable for the thickness of the mother vessel for the Barrel KLM module slot
    double m_thick_biron;

    //! variable for the difference between the designed thickness and the real measurement
    double m_thick_biron_meas;

    //! variable for the radial position of the 0-th KLM barrel module slot (= 1st layer)
    double m_rmin_bslot0;

    //! variable for the thickness of the 0-th KLM barrel module slot (= 1st layer)
    double m_thick_bslot0;

    //! variable for the difference between the designed thickness of the 0-th slot and the real measurement
    double m_thick_bslot0_meas;

    //! variable for the parameter which is used to calculate the Barrel KLM module 0-th slot
    double m_dsx_bslot0;

    //! variable for the length of the Barrel KLM module slot
    double m_length_bslot;

    //! variable for the thickness of the Barrel KLM module slot
    double m_thick_bslot;

    //! variable for the difference between the designed thickness of the module slot and the real measurement
    double m_thick_bslot_meas;

    //! variable for the parameter which is used to calculate the Barrel KLM module slot
    double m_dsx_bslot;

    //! variable for the radial position of the 1st KLM barrel module slot (= 2nd layer)
    double m_rmin_bslot1;

    //! variable for the thickness of the Barrel KLM module
    double m_thick_bmod;

    //! variable for the length of the Barrel KLM module
    double m_length_bmod;

    //! variable for get the length of the Barrel KLM module with taking chimney hole into accout : not used at this moment
    double m_length_chole;

    //! variable for a parameter for chimney hole : not used at this moment
    double m_cov_chole;

    //! variable for the width of the chimney hole : not used at this moment
    double m_width_chole;
  };

//-----------------------------------------------------------------------------

} // end of namespace Belle2

#endif
