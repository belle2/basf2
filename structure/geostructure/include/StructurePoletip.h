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

    const std::string &matname(void) const {return m_matname;}
    const double &phi(void) const {return m_phi;}
    const double &dphi(void) const {return m_dphi;}
    const int &nBoundary(void) const {return m_nBoundary;}
    const double &z(int id) const {return m_z[id];}
    const double &rmin(int id) const {return m_rmin[id];}
    const double &rmax(int id) const {return m_rmax[id];}

  private:

    std::string m_matname;
    double m_phi;
    double m_dphi;
    int m_nBoundary;
    double m_z[11];
    double m_rmin[11];
    double m_rmax[11];

    /*
    const int &nBoundary(void) const {return m_nBoundary;}
    const double &z (int id) const {return m_z[id];}
    const double &rmin (int id) const {return m_rmin[id];}
    const double &rmax (int id) const {return m_rmax[id];}
    */

    //std::string m_version; /*!< The version of geometry parameters. */
    //int m_nSLayer;         /*!< The number of sense wire layer. */
    //int m_nFLayer;         /*!< The number of field wire layer. */
    //double m_rWall[4];     /*!< The array to store radius of inner wall and outer wall. */
    //double m_zWall[4][2];  /*!< The array to store z position of inner wall and outer wall. */

    //double m_rSLayer[MAX_N_SLAYERS];          /*!< The array to store radius of sense wire layers. */
    //double m_zSForwardLayer[MAX_N_SLAYERS];   /*!< The array to store forward z position of sense wire layers. */
    //double m_zSBackwardLayer[MAX_N_SLAYERS];  /*!< The array to store backward z position of sense wire layers. */
    //double m_rFLayer[MAX_N_FLAYERS];          /*!< The array to store radius of field wire layers. */
    //double m_zFForwardLayer[MAX_N_FLAYERS];   /*!< The array to store forward z position of field wire layers. */
    //double m_zFBackwardLayer[MAX_N_FLAYERS];  /*!< The array to store backward z position of field wire layers. */
    //double m_offSet[MAX_N_SLAYERS];           /*!< The array to store z offset of sense wire layers. */
    //double m_cellSize[MAX_N_SLAYERS];         /*!< The array to store cell size in each sense wire layer. */
    //int m_nShifts[MAX_N_SLAYERS];             /*!< The array to store shifted cell number in each sense wire layer. */
    //unsigned m_nWires[MAX_N_SLAYERS];         /*!< The array to store the wire number in each sense wire layre. */

    //double m_senseWireDiameter;                   /*!< The diameter of sense wires. */
    //double m_fieldWireDiameter;                   /*!< The diameter of field wires. */

    //static CDCGeometryPar* m_B4CDCGeometryParDB; /*!< Pointer that saves the instance of this class. */
  };

//-----------------------------------------------------------------------------

} // end of namespace Belle2

#endif
