/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPSIMHIT_H
#define TOPSIMHIT_H

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {


  //! Class TOPSimHit - Geant4 simulated hit for TOP.
  /*!
   This is a class to store TOP geant4 hit in datastore.
   It is also the input for digitization module (TOPDigi).
   */

  class TOPSimHit : public TObject {
  public:

    //! Default constructor
    TOPSimHit():
        m_moduleID(0),
        m_barID(0),
        m_position(0, 0, 0),
        m_gposition(0, 0, 0),
        m_direction(0, 0, 0),
        m_vposition(0, 0, 0),
        m_vdirection(0, 0, 0),
        m_globalTime(0),
        m_localTime(0),
        m_length(0),
        m_energy(0),
        m_parentID(0),
        m_trackID(0) {
      /*! Does nothing */
    }

    //! Full constructor.
    /*!
     \param moduleID ID of PMT module containing hit
     \param barID is the ID of the bar in which the PMT was hit
     \param globalTime global time of photon hit
     \param energy energy of photon
     \param parentID geant4 id of photon parent particle
     \param trackID geant4 id of photon track
     */

    TOPSimHit(
      int moduleID,
      int barID,
      TVector3 position,
      TVector3 gposition,
      TVector3 direction,
      TVector3 vposition,
      TVector3 vdirection,
      double globalTime,
      double localTime,
      double length,
      double energy,
      int parentID,
      int trackID):
        m_moduleID(moduleID),
        m_barID(barID),
        m_position(position),
        m_gposition(gposition),
        m_direction(direction),
        m_vposition(vposition),
        m_vdirection(vdirection),
        m_globalTime(globalTime),
        m_localTime(localTime),
        m_length(length),
        m_energy(energy),
        m_parentID(parentID),
        m_trackID(trackID) {
      /* Does nothing */
    }

    //! Get ID number of module that registered hit
    int getModuleID() const { return m_moduleID; }

    //! Get ID number of bar that registered hit
    int getBarID() const { return m_barID; }

    //! Get local position of hit (in module coordinates)
    const TVector3& getPosition() const { return m_position; }

    //! Get global position of hit (in module coordinates)
    const TVector3& getGPosition() const { return m_gposition; }

    //! Get global position of hit (in module coordinates)
    const TVector3& getDirection() const { return m_direction; }

    //! Get local position of hit (in module coordinates)
    const TVector3& getVposition() const { return m_vposition; }

    //! Get global position of hit (in module coordinates)
    const TVector3& getVdirection() const { return m_vdirection; }

    //! Get global time of hit
    double getTime() const { return m_globalTime; }

    //! Get global time of hit
    double getEmissionTime() const { return m_localTime; }

    //! Get track length
    double getLength() const { return m_length; }

    //! Get detected photon energy
    double getEnergy() const { return m_energy; }

    //! Get G4 ID number of photons parent particle
    int getParentID() const { return m_parentID; }

    //! Get G4 ID number of photons track
    int getTrackID() const { return m_trackID; }

    //! Set ID number of module that registered hit
    void setModuleID(int moduleID) { m_moduleID = moduleID; }

    //! Set ID number of bar that registered hit
    void setBarID(int barID) { m_barID = barID; }

    //! Set local position of hit (in module coordinates)
    void setPosition(double x, double y, double z) { m_position.SetXYZ(x, y, z); }

    //! Set global position of hit (in module coordinates)
    void setGPosition(double x, double y, double z) { m_gposition.SetXYZ(x, y, z); }

    //! Set global time of hit
    void setTime(double globalTime) { m_globalTime = globalTime; }

    //! Get detected photon energy
    void setEnergy(double energy) { m_energy = energy; }

    //! Get G4 ID number of photons parent particle
    void setParentID(int parentID) { m_parentID = parentID; }

    //! Get G4 ID number of photons parent particle
    void setTrackID(int trackID) { m_trackID = trackID; }

  private:
    int m_moduleID;           /**< ID number of module that registered hit*/
    int m_barID;           /**< ID number of bar that registered hit*/
    TVector3 m_position;
    TVector3 m_gposition;
    TVector3 m_direction;
    TVector3 m_vposition;
    TVector3 m_vdirection;
    double m_globalTime;      /**< Global time of hit */
    double m_localTime;
    double m_length;
    double m_energy;          /**< Energy of detected photon */
    int m_parentID;        /**< G4 ID number of photons parent particle */
    int m_trackID;        /**< G4 ID number of photons track */


    ClassDef(TOPSimHit, 1); /**< the class title */

  };

} // end namespace Belle2

#endif
