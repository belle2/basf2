/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHAEROHIT_H
#define ARICHAEROHIT_H

#include <framework/datastore/RelationsObject.h>
#include <Math/Vector3D.h>

namespace Belle2 {


  //! Datastore class that holds information on track parameters at the entrance in aerogel.
  /*!  For now this information comes from "ARICHSensitiveAero" sensitive detector. This should be replaced with the information from tracking.
  */


  class ARICHAeroHit : public RelationsObject {
  public:

    //! Empty constructor
    /*! Recommended for ROOT IO */
    ARICHAeroHit():
      m_particleID(-1),
      m_x(0.0),
      m_y(0.0),
      m_z(0.0),
      m_px(0.0),
      m_py(0.0),
      m_pz(0.0)
    {
      /*! does nothing */
    }

    //! Useful Constructor
    /*!
      \param trackId geant4 track id
      \param particleId particle PDG id number
      \param position vector of track position on aerogel plane
      \param momentum vector of track momentum on aerogel plane
    */
    ARICHAeroHit(int particleId,
                 ROOT::Math::XYZVector position,
                 ROOT::Math::XYZVector momentum)
    {
      m_particleID = particleId;
      m_x = (float) position.X();
      m_y = (float) position.Y();
      m_z = (float) position.Z();
      m_px = (float) momentum.X();
      m_py = (float) momentum.Y();
      m_pz = (float) momentum.Z();
    }

    //! Get particle PDG identity number
    int getPDG() const { return m_particleID; }

    //! Get track position (at entrance in 1. aerogel plane)
    ROOT::Math::XYZVector getPosition() const { return ROOT::Math::XYZVector(m_x, m_y, m_z); }

    //! Get track momentum  (at entrance in 1. aerogel plane)
    ROOT::Math::XYZVector getMomentum() const { return ROOT::Math::XYZVector(m_px, m_py, m_pz); }

  private:

    int m_particleID;  /**< PDG code of particle */
    float  m_x;        /**< impact point, x component (at entrance in 1. aerogel plane) */
    float  m_y;        /**< impact point, x component (at entrance in 1. aerogel plane) */
    float  m_z;        /**< impact point, x component (at entrance in 1. aerogel plane) */
    float  m_px;       /**< impact momentum, x component (at entrance in 1. aerogel plane) */
    float  m_py;       /**< impact momentum, x component (at entrance in 1. aerogel plane) */
    float  m_pz;       /**< impact momentum, x component (at entrance in 1. aerogel plane) */

    ClassDef(ARICHAeroHit, 1); /**< the class title */

  };

} // end namespace Belle2

#endif  // ARICHAEROHIT_H
