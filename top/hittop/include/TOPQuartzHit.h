/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPQUARTZHIT_H
#define TOPQUARTZHIT_H

#include <framework/datastore/DataStore.h>

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  /** Datastore class that holds information on track parameters at the entrance into the quartz
   *  For now this information comes from ("TOPSensitiveQuartz" sensitive detector). This should be replaced with the information from tracking.
   */


  class TOPQuartzHit : public TObject {
  public:

    //! Empty constructor
    /*! Recommended for ROOT IO */
    TOPQuartzHit() {;}

    //! Useful Constructor
    TOPQuartzHit(
      int trackId,
      int particleId,
      TVector3 position,
      TVector3 momentum
    ) {
      m_trackID = trackId;
      m_particleID = particleId;
      m_position = position;
      m_momentum = momentum;
    }


    //! Set Geant4 track ID
    void setTrackID(int trackId) { m_trackID = trackId; }

    //! Set particle PDG identity number
    void setParticleID(int particleId) { m_particleID = particleId; }

    //! Set track position (at entrance in quartzl plane)
    void setPosition(TVector3 position) { m_position = position; }

    //! Set track momentum  (at entrance in quartzl plane)
    void setMomentum(TVector3 momentum) { m_momentum = momentum; }

    //! Get Geant4 track ID
    int getTrackID() const { return m_trackID; }

    //! Get particle PDG identity number
    int getParticleID() const { return m_particleID; }

    //! Get track position (at entrance in quartzl plane)
    const TVector3 &getPosition() const { return m_position; }

    //! Get track momentum  (at entrance in quartzl plane)
    const TVector3 &getMomentum() const { return m_momentum; }

  private:

    int m_trackID;             /*!< G4 id of track */
    int m_particleID;          /*!< particle PDG id number */
    TVector3 m_position;       /*!< track position (at entrance in quartzl plane) */
    TVector3 m_momentum;       /*!< track position (at entrance in quartzl plane) */


    ClassDef(TOPQuartzHit, 1); /*!< the class title */

  };

} // end namespace Belle2

#endif  // TOPQUARTZHIT_H
