/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPQUARTZHIT_H
#define TOPQUARTZHIT_H

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  /*! This is a class to store TOP Geant4 hits in datastore.
   *  It is also the input for digitization module (TOPDigi).
   */


  class TOPTrack : public TObject {
  public:

    /*! Default constructor
     */
    TOPTrack():
      m_trackID(-1),
      m_particleID(-1),
      m_charge(0),
      m_position(0, 0, 0),
      m_vposition(0, 0, 0),
      m_momentum(0, 0, 0),
      m_vmomentum(0, 0, 0),
      m_barID(-1),
      m_length(0.),
      m_globaltime(0.),
      m_localtime(0.) {
    }

    /*!  Full constructor.
     * @param geant4 track id
     * @param particle PDG id number
     * @param the charge of the track
     * @param position of the detected track
     * @param production vertex of the detected track
     * @param momentum vector of the detected track
     * @param momentum of the detected particle at vertex position
     * @param ID of the bar which was hit
     * @param length of the track from vertex to this point
     * @param global time at which the track was detected
     * @param time at which the track was generated
     */
    TOPTrack(
      int trackID,
      int particleID,
      int charge,
      TVector3 position,
      TVector3 vposition,
      TVector3 momentum,
      TVector3 vmomentum,
      int barID,
      double length,
      double globaltime,
      double localtime
    ) {
      m_trackID = trackID;
      m_particleID = particleID;
      m_charge = charge;
      m_position = position;
      m_vposition = vposition;
      m_momentum = momentum;
      m_vmomentum = vmomentum;
      m_barID = barID;
      m_length = length;
      m_globaltime = globaltime;
      m_localtime = localtime;
    }

    /*! -- Function for getting parameters out of TOPTrack -- */

    /*! Get the track ID of the detected track
     * @return track ID of detected track
     */
    int getTrackID() const { return m_trackID; }

    /*! Get the PDG code of the track
     * @return PDG code
     */
    int getParticleID() const { return m_particleID; }

    /*! Get the charge of the particle
     * @return charge of the track
     */
    int getCharge() const { return m_charge;}

    /*! Get the position at which the track was detected
     * @return detection position
     */
    const TVector3& getPosition() const { return m_position; }

    /*! Get position of vertex of detected track
     * @return vertex position
     */
    const TVector3& getVPosition() const { return m_vposition; }

    /*! Get momentum of the detected track
     * @return momentum of detected track
     */
    const TVector3& getMomentum() const { return m_momentum; }

    /*! Get vertex momentum of the detected track
     * @return vertex momentum
     */
    const TVector3& getVMomentum() const { return m_vmomentum; }

    /*! Get ID of bar that was hit by track
     * @return ID of hit bar
     */
    int getBarID() const { return m_barID;}

    /*! Get the length from vertex point to detection point
     * @return track length
     */
    double getLength() const { return m_length; }

    /*! Get the time since the begining of the evetn
     * @return time since start of event
     */
    double getGlobalTime() const { return m_globaltime; }

    /*! Get the time since the creation of the particle
     * @return local time of particle
     */
    double getLocalTime() const { return m_localtime; }


    /*! -- Function for parameters parameters in TOPTrack -- */


    /*! Set track ID of track
     */
    void setTrackID(int trackID) { m_trackID = trackID; }

    /*! Set charge of track
     */
    void setCharge(int charge) {m_charge = charge;}

    /*! Set PDG number of track
     */
    void setParticleID(int particleId) { m_particleID = particleId; }

    /*! Set detection position of track
     */
    void setPosition(TVector3 position) { m_position = position; }

    /*! Set vertex position of track
     */
    void setVPosition(TVector3 vposition) { m_vposition = vposition; }

    /*! Set momentum of tract at detection position
     */
    void setMomentum(TVector3 momentum) { m_momentum = momentum; }

    /*! Set momentum of track at vertex position
     */
    void setVMomentum(TVector3 vmomentum) { m_vmomentum = vmomentum; }

    /*! Set ID of bar that was hit by track
     */
    void setBarID(int barID) {m_barID = barID;}

    /*! Set legth of track from vertex point to detection point
     */
    void setLength(double length) { m_length = length; }

    /*! Set global time
     */
    void setGlobalTime(double globaltime)  {m_globaltime = globaltime; }

    /*! Set local time - time since creation of particle
     */
    void setLocalTime(double localtime)  { m_localtime = localtime; }



  private:

    int m_trackID;             /**< G4 id of track */
    int m_particleID;          /**< particle PDG id number */
    int m_charge;              /**< charge of track */
    TVector3 m_position;       /**< track position at detection point */
    TVector3 m_vposition;      /**< track momentum at detection point */
    TVector3 m_momentum;       /**< track vertex point */
    TVector3 m_vmomentum;      /**< track momentum at vertex point */
    int m_barID;               /**< ID of bar that was hit */
    double m_length;           /**< track length */
    double m_globaltime;       /**< global time - time since begining of event */
    double m_localtime;        /**< local time - time since creaton of particle */


    ClassDef(TOPTrack, 1);     /**< the class title */

  };

} // end namespace Belle2

#endif  // TOPQUARTZHIT_H
