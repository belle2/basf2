/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHAEROHIT_H
#define ARICHAEROHIT_H

#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>

namespace Belle2 {

  /** \addtogroup dataobjects
   *@ {
   */

  //! Datastore class that holds information on track parameters at the entrance in aerogel.
  /*!  For now this information comes from "ARICHSensitiveAero" sensitive detector. This should be replaced with the information from tracking.
  */


  class ARICHAeroHit : public RelationsObject {
  public:

    //! Empty constructor
    /*! Recommended for ROOT IO */
    ARICHAeroHit():
      m_trackID(-1),
      m_particleID(-1),
      m_position(0, 0, 0),
      m_momentum(0, 0, 0) {
      /*! does nothing */
    }

    //! Useful Constructor
    /*!
      \param trackId geant4 track id
      \param particleId particle PDG id number
      \param position vector of track position on aerogel plane
      \param momentum vector of track momentum on aerogel plane
    */
    ARICHAeroHit(int trackId,
                 int particleId,
                 TVector3 position,
                 TVector3 momentum) :
      m_trackID(trackId),
      m_particleID(particleId),
      m_position(position),
      m_momentum(momentum) {
    }

    //! Set Geant4 track ID
    void setTrackID(int trackId) { m_trackID = trackId; }

    //! Set particle PDG identity number
    void setPDG(int particleId) { m_particleID = particleId; }

    //! Set track position (at entrance in 1. aerogel plane)
    void setPosition(TVector3 position) { m_position = position; }

    //! Set track momentum  (at entrance in 1. aerogel plane)
    void setMomentum(TVector3 momentum) { m_momentum = momentum; }

    //! Get Geant4 track ID
    int getTrackID() const { return m_trackID; }

    //! Get particle PDG identity number
    int getPDG() const { return m_particleID; }

    //! Get track position (at entrance in 1. aerogel plane)
    const TVector3& getPosition() const { return m_position; }

    //! Get track momentum  (at entrance in 1. aerogel plane)
    const TVector3& getMomentum() const { return m_momentum; }

  private:

    int m_trackID;             /**< G4 id of track */
    int m_particleID;          /**< particle PDG id number */
    TVector3 m_position;       /**< track position (at entrance in 1. aerogel plane) */
    TVector3 m_momentum;       /**< track position (at entrance in 1. aerogel plane) */


    ClassDef(ARICHAeroHit, 1); /**< the class title */

  };

} // end namespace Belle2

#endif  // ARICHAEROHIT_H
