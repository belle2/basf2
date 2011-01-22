/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef STRUCTUREPOLETIP_H
#define STRUCTUREPOLETIP_H

#include <vector>
#include <string>

namespace Belle2 {

//! The Class for CDC Geometry Parameters
  /*! This class provides CDC gemetry paramters for simulation, reconstruction and so on.
      These parameters are gotten from gearbox.
  */
  class StructurePoletip {

  public:

    //! Constructor
    StructurePoletip();

    //! Destructor
    virtual ~StructurePoletip();

    //! Static method to get a reference to the CDCGeometryPar instance.
    /*!
        \return A reference to an instance of this class.
    */

    ///////////////static CDCGeometryPar* Instance();

    //! Clears
    void clear();

    //! Gets geometry parameters from gearbox.
    void read();

    //! to get the material name for the Poletip
    const std::string &matname(void) const {return m_matname;}

    //! to get the starting angle of the phi
    const double &phi(void) const {return m_phi;}

    //! to get the opening angle (shape is extended from phi to phi+dphi)
    const double &dphi(void) const {return m_dphi;}

    //! to get the number of boundaries perpendicular to the z-axis
    const int &nBoundary(void) const {return m_nBoundary;}

    //! to get the z coordinate of the section specified by input id (=boundary id)
    const double &z(int id) const {return m_z[id];}

    //! to get the inner radius of the section specified by input id (=boundary id)
    const double &rmin(int id) const {return m_rmin[id];}

    //! to get the outer radius of the section specified by input id (=boundary id)
    const double &rmax(int id) const {return m_rmax[id];}

  private:

    //! variable for the material name for the Poletip
    std::string m_matname;

    //! variable for the starting angle of the phi
    double m_phi;

    //! variable for the opening angle (shape is extended from phi to phi+dphi)
    double m_dphi;

    //! variable for the number of boundaries perpendicular to the z-axis
    int m_nBoundary;

    //! variable for the z coordinate of the section specified by input id (=boundary id)
    double m_z[11];

    //! variable for the inner radius of the section specified by input id (=boundary id)
    double m_rmin[11];

    //! variable for the outer radius of the section specified by input id (=boundary id)
    double m_rmax[11];
  };

//-----------------------------------------------------------------------------

} // end of namespace Belle2

#endif
