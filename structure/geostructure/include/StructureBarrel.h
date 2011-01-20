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


    const TVector3 barrelSlotSize(int) const;
    const TVector3 barrelModuleSize(int) const;
    const double barrelSlotR(int) const;
    const double barrelModuleR(int) const;

    const std::string &matname(void) const {return m_matname;}
    const double &phi(void) const {return m_phi;}
    const double &dphi(void) const {return m_dphi;}
    const int &nsides(void) const {return m_nsides;}
    const int &nBoundary(void) const {return m_nBoundary;}
    const double &z(int id) const {return m_z[id];}
    const double &rmin(int id) const {return m_rmin[id];}
    const double &rmax(int id) const {return m_rmax[id];}

    const std::string &matname4slot(void) const {return m_matname4slot;}
    const std::string &matname4module(void) const {return m_matname4module;}
    const int &nsector(void) const {return m_nsector;}
    const int &nmodule(void) const {return m_nmodule;}
    const double &thick_biron(void) const {return m_thick_biron;}
    const double &thick_biron_meas(void) const {return m_thick_biron_meas;}
    const double &rmin_bslot0(void) const {return m_rmin_bslot0;}
    const double &thick_bslot0(void) const {return m_thick_bslot0;}
    const double &thick_bslot0_meas(void) const {return m_thick_bslot0_meas;}
    const double &dsx_bslot0(void) const {return m_dsx_bslot0;}
    const double &length_bslot(void) const {return m_length_bslot;}
    const double &thick_bslot(void) const {return m_thick_bslot;}
    const double &thick_bslot_meas(void) const {return m_thick_bslot_meas;}
    const double &dsx_bslot(void) const {return m_dsx_bslot;}
    const double &rmin_bslot1(void) {return m_rmin_bslot1;}
    const double &thick_bmod(void) {return m_thick_bmod;}
    const double &length_bmod(void) {return m_length_bmod;}
    const double &length_chole(void) {return m_length_chole;}
    const double &cov_chole(void) {return m_cov_chole;}
    const double &width_chole(void) {return m_width_chole;}

  private:

    //Barrel Vessel
    std::string m_matname;
    double m_phi;
    double m_dphi;
    int m_nsides;
    int m_nBoundary;
    double m_z[11];
    double m_rmin[11];
    double m_rmax[11];

    //Barrel slot for KLM module
    std::string m_matname4slot;
    std::string m_matname4module;
    int m_nsector;
    int m_nmodule;

    double m_thick_biron;
    double m_thick_biron_meas;

    double m_rmin_bslot0;
    double m_thick_bslot0;
    double m_thick_bslot0_meas;
    double m_dsx_bslot0;

    double m_length_bslot;
    double m_thick_bslot;
    double m_thick_bslot_meas;
    double m_dsx_bslot;

    double m_rmin_bslot1;
    double m_thick_bmod;
    double m_length_bmod;

    double m_length_chole;
    double m_cov_chole;
    double m_width_chole;
  };

//-----------------------------------------------------------------------------

} // end of namespace Belle2

#endif
