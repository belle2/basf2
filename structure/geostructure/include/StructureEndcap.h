/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef STRUCTUREENDCAP_H
#define STRUCTUREENDCAP_H

#include <vector>
#include <string>

#include "TVector3.h"

namespace Belle2 {

//! The Class for CDC Geometry Parameters
  /*! This class provides CDC gemetry paramters for simulation, reconstruction and so on.
      These parameters are gotten from gearbox.
  */
  class StructureEndcap {

  public:

    //! Constructor
    StructureEndcap();

    //! Destructor
    virtual ~StructureEndcap();

    //! Static method to get a reference to the CDCGeometryPar instance.
    /*!
        \return A reference to an instance of this class.
    */

    ///////////////static CDCGeometryPar* Instance();

    //! Clears
    void clear();

    //! Gets geometry parameters from gearbox.
    void read();


    const TVector3 endcapModulePos(int, int) const;

    const std::string &matname(void) const {return m_matname;}
    const double &phi(void) const {return m_phi;}
    const double &dphi(void) const {return m_dphi;}
    const int &nsides(void) const {return m_nsides;}
    const int &nBoundary(void) const {return m_nBoundary;}
    const double &z(int id) const {return m_z[id];}
    const double &rmin(int id) const {return m_rmin[id];}
    const double &rmax(int id) const {return m_rmax[id];}

    const std::string &matnamesub(void) const {return m_matnamesub;}
    const double &zsub(void) const {return m_zsub;}
    const double &rminsub(void) const {return m_rminsub;}
    const double &rmaxsub(void) const {return m_rmaxsub;}

    const double &rmin_layer(void) const {return m_rmin_layer;}
    const double &rmax_layer(void) const {return m_rmax_layer;}

    const std::string &matname4slot(void) const {return m_matname4slot;}
    const std::string &matname4module(void) const {return m_matname4module;}

    const double &thick_eiron(void) const {return m_thick_eiron;}
    const double &thick_eiron_meas(void) const {return m_thick_eiron_meas;}

    const double &thick_eslot(void) const {return m_thick_eslot;}
    const double &thick_eslot_meas(void) const {return m_thick_eslot_meas;}

    const double &thick_emod(void) const {return m_thick_emod;}
    const double &rmin_emod(void) const {return m_rmin_emod;}
    const double &rmax_emod(void) const {return m_rmax_emod;}
    const double &rmin_eslot(void) const {return m_rmin_eslot;}
    const double &rmax_eslot(void) const {return m_rmax_eslot;}

    const double &rshift_eslot(void) const {return m_rshift_eslot;}
    const double &rmax_glass(void) const {return m_rmax_glass;}

  private:

    //Endcap Vessel
    std::string m_matname;
    double m_phi;
    double m_dphi;
    int m_nsides;
    int m_nBoundary;
    double m_z[11];
    double m_rmin[11];
    double m_rmax[11];

    //Endcap tube shape which should be subtracted from the octagonal Endcap Vessel
    std::string m_matnamesub;
    double m_zsub;
    double m_rminsub;
    double m_rmaxsub;

    double m_rmin_layer;
    double m_rmax_layer;

    //Endcap slot for KLM module
    std::string m_matname4slot;
    std::string m_matname4module;

    double m_thick_eiron;
    double m_thick_eiron_meas;

    double m_thick_eslot;
    double m_thick_eslot_meas;

    double m_thick_emod;
    double m_rmin_emod;
    double m_rmax_emod;
    double m_rmin_eslot;
    double m_rmax_eslot;

    double m_rshift_eslot;
    double m_rmax_glass;
  };

//-----------------------------------------------------------------------------

} // end of namespace Belle2

#endif
