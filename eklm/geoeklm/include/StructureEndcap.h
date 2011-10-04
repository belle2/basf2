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

  //  namespace eklm {

  //! The Class for Structure Geometry Parameters
  /*! This class provides Structure gemetry paramters for simulation, reconstruction and so on.
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

    //! to get the position of the Endcap KLM module slot of the specified layer and sector
    const TVector3 endcapModulePos(int, int) const;

    //! Endcap KLM shape = octagonal - tube
    //! to get the material name for the entire Endcap Structure (octagonal shape)
    const std::string &matname(void) const {return m_matname;}

    //! to get the starting angle of the octagonal Endcap KLM shape
    const double &phi(void) const {return m_phi;}

    //! to get the opening angle (shape is extended from phi to phi+dphi)
    const double &dphi(void) const {return m_dphi;}

    //! to get the number of sides (=8 : octagonal)
    const int &nsides(void) const {return m_nsides;}

    //! to get the number of boundaries perpendicular to the z-axis
    const int &nBoundary(void) const {return m_nBoundary;}

    //! to get the z coordinate of the section specified by input id (=boundary id)
    const double &z(int id) const {return m_z[id];}

    //! to get the radius of the circle tangent to the sides of the inner polygon
    const double &rmin(int id) const {return m_rmin[id];}

    //! to get the radius of the circle tangent to the sides of the outer polygon
    const double &rmax(int id) const {return m_rmax[id];}

    //! to get the material name for the tube shape which is subtracted from the octagonal shape
    const std::string &matnamesub(void) const {return m_matnamesub;}

    //! to get the length of the tube
    const double &zsub(void) const {return m_zsub;}

    //! to get the inner radius of the tube
    const double &rminsub(void) const {return m_rminsub;}

    //! to get the outer radius of the tube
    const double &rmaxsub(void) const {return m_rmaxsub;}

    //! to get the minimum radius of the complecated (=octagonal - tube) Endcap KLM structure
    const double &rmin_layer(void) const {return m_rmin_layer;}

    //! to get the maximum radius of the complecated (=octagonal - tube) Endcap KLM structure
    const double &rmax_layer(void) const {return m_rmax_layer;}

    //! to get the material name for the Endcap KLM module slot
    const std::string &matname4slot(void) const {return m_matname4slot;}

    //! to get the material name for the Endcap KLM module
    const std::string &matname4module(void) const {return m_matname4module;}

    //! to get the thickness of the mother vessel of the Endcap KLM module slot
    const double &thick_eiron(void) const {return m_thick_eiron;}

    //! to get the difference between the designed thickness and the real measurement
    const double &thick_eiron_meas(void) const {return m_thick_eiron_meas;}

    //! to get the thickness of the Endcap KLM module slot
    const double &thick_eslot(void) const {return m_thick_eslot;}

    //! to get the difference between the designed thickness of the module slot and the real measurement
    const double &thick_eslot_meas(void) const {return m_thick_eslot_meas;}

    //! to get the thickness of the Endcap KLM module
    const double &thick_emod(void) const {return m_thick_emod;}

    //! to get the minimum radius of the Endcap KLM module
    const double &rmin_emod(void) const {return m_rmin_emod;}

    //! to get the maximum radius of the Endcap KLM module
    const double &rmax_emod(void) const {return m_rmax_emod;}

    //! to get the minimum radius of the Endcap KLM module slot
    const double &rmin_eslot(void) const {return m_rmin_eslot;}

    //! to get the maximum radius of the Endcap KLM module slot
    const double &rmax_eslot(void) const {return m_rmax_eslot;}

    //! to get the shift value in the radial direction of the Endcap KLM module slot (?)
    const double &rshift_eslot(void) const {return m_rshift_eslot;}

    //! to get the maximum radius of the glass used in KLM module ??? : not used now.
    const double &rmax_glass(void) const {return m_rmax_glass;}

  private:

    //
    //Endcap Vessel
    //
    //! variable for the material name for the entire Endcap Structure
    std::string m_matname;

    //! variable for the starting angle of the octagonal Endcap KLM shape
    double m_phi;

    //! variable for the opening angle (shape is extended from phi to phi+dphi)
    double m_dphi;

    //! variable for the number of sides (=8 : octagonal)
    int m_nsides;

    //! variable for the number of boundaries perpendicular to the z-axis
    int m_nBoundary;

    //! variable for the z coordinate of the section specified by input id (=boundary id)
    double m_z[11];

    //! variable for the radius of the circle tangent to the sides of the inner polygon
    double m_rmin[11];

    //! variable for the radius of the circle tangent to the sides of the outer polygon
    double m_rmax[11];

    //
    //Endcap tube shape which should be subtracted from the octagonal Endcap Vessel
    //
    //! variable for the material name for the tube shape which is subtracted from the octagonal shape
    std::string m_matnamesub;

    //! variable for the length of the tube
    double m_zsub;

    //! variable for the inner radius of the tube
    double m_rminsub;

    //! variable for the outer radius of the tube
    double m_rmaxsub;

    //! variable for the minimum radius of the complecated (=octagonal - tube) Endcap KLM structure
    double m_rmin_layer;

    //! variable for the maximum radius of the complecated (=octagonal - tube) Endcap KLM structure
    double m_rmax_layer;

    //
    //Endcap slot for KLM module
    //
    //! variable for the material name for the Endcap KLM module slot
    std::string m_matname4slot;

    //! variable for the material name for the Endcap KLM module
    std::string m_matname4module;

    //! variable for the thickness of the mother vessel of the Endcap KLM module slot
    double m_thick_eiron;

    //! variable for the difference between the designed thickness and the real measurement
    double m_thick_eiron_meas;

    //! variable for the thickness of the Endcap KLM module slot
    double m_thick_eslot;

    //! variable for the difference between the designed thickness of the module slot and the real measurement
    double m_thick_eslot_meas;

    //! variable for the thickness of the Endcap KLM module
    double m_thick_emod;

    //! variable for the minimum radius of the Endcap KLM module
    double m_rmin_emod;

    //! variable for the maximum radius of the Endcap KLM module
    double m_rmax_emod;

    //! variable for the minimum radius of the Endcap KLM module slot
    double m_rmin_eslot;

    //! variable for the maximum radius of the Endcap KLM module slot
    double m_rmax_eslot;

    //! variable for the shift value in the radial direction of the Endcap KLM module slot (?)
    double m_rshift_eslot;

    //! variable for the maximum radius of the glass used in KLM module ??? : not used now.
    double m_rmax_glass;
  };

  //-----------------------------------------------------------------------------
  //  }
} // end of namespace Belle2

#endif
