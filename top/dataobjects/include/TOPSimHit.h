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


  /*! This is a class to store TOP Geant4 hits in datastore.
   *  It is also the input for digitization module (TOPDigi).
   */

  class TOPSimHit : public TObject {
  public:

    /*! Default constructor
     */
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
    }

    /*!  Full constructor.
     * @param moduleID ID of PMT module containing hit
     * @param barID is the ID of the bar in which the PMT was hit
     * @param position position of the hit on the hit on PMT pad
     * @param gposition postion of the hit on the pad in the lab frame
     * @param direction momentum direction of the hit
     * @param vposition vertex position in lab frame
     * @param vdirection momentum direction of vertex position
     * @param globalTime global time of photon hit
     * @param localTime time that pased since the the photon was created
     * @param length tract lenth of the photon
     * @param energy energy of photon
     * @param parentID geant4 id of photon parent particle
     * @param trackID geant4 id of photon track
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
    }

    /*! -- Function for getting parameters out of TOPSimHit -- */

    /*! Get ID number of module that registered hit
     * @return moduleID of the hit
     */
    int getModuleID() const { return m_moduleID; }

    /*! Get ID number of bar that registered hit
     * @return ID of the bar in which the hit took place
     */
    int getBarID() const { return m_barID; }

    /*! Get local position of hit (in module coordinates)
     * @return vector of the local position of the hit on the pad
     */
    const TVector3& getPosition() const { return m_position; }

    /*! Get global position of hit (in module coordinates)
     * @return position of the hit in global coordinates
     */
    const TVector3& getGPosition() const { return m_gposition; }

    /*! Get global position of hit (in module coordinates)
     * @return get direction vector of the momentum of the hit
     */
    const TVector3& getDirection() const { return m_direction; }

    /*! Get local position of hit (in module coordinates)
     * @return vertex position of the hit particle
     */
    const TVector3& getVposition() const { return m_vposition; }

    /*! Get global position of hit (in module coordinates)
     * @return vector for momentum direction at vertex position
     */
    const TVector3& getVdirection() const { return m_vdirection; }

    /*! Get global time of hit
     * @return global time
     */
    double getTime() const { return m_globalTime; }

    /*! Get global time of hit
     * @return time at which the photon was emitted
     */
    double getEmissionTime() const { return m_localTime; }

    /*! Get track length
     * @return length of the photon track
     */
    double getLength() const { return m_length; }

    /*! Get detected photon energy
     * @return energy of the detected photon
     */
    double getEnergy() const { return m_energy; }

    /*! Get G4 ID number of photons parent particle
     * @return GeantID of the parent particle
     */
    int getParentID() const { return m_parentID; }

    /*! Get G4 ID number of photons track
     * @return GeantID of the photon track
     */
    int getTrackID() const { return m_trackID; }

    /*! -- Function for parameters parameters in TOPSimHit -- */


    /*! Set ID number of module that registered hit
     */
    void getModuleID(int ModuleID) { m_moduleID = ModuleID; }

    /*! Set ID number of bar that registered hit
     */
    void setBarID(int BarID) { m_barID = BarID; }

    /*! Set local position of hit (in module coordinates)
     */
    void setPosition(double x, double y, double z) { m_position.SetXYZ(x, y, z); }

    /*! Set global position of hit (in module coordinates)
     */
    void setGPosition(double x, double y, double z) { m_gposition.SetXYZ(x, y, z); }

    /*! Set global position of hit (in module coordinates)
     */
    void setDirection(double x, double y, double z) { m_direction.SetXYZ(x, y, z); }

    /*! Set local position of hit (in module coordinates)
     */
    void setVposition(double x, double y, double z) { m_vposition.SetXYZ(x, y, z); }

    /*! Set global position of hit (in module coordinates)
     */
    void setVdirection(double x, double y, double z) { m_vdirection.SetXYZ(x, y, z); }

    /*! Set global time of hit
     */
    void setTime(double time) { m_globalTime = time; }

    /*! Set global time of hit
     */
    void setEmissionTime(double EmissionTime) {  m_localTime = EmissionTime; }

    /*! Set track length
     */
    void setLength(double Length) {  m_length = Length; }

    /*! Set detected photon energy
     */
    void setEnergy(double Energy) {  m_energy = Energy; }

    /*! Set G4 ID number of photons parent particle
     */
    void setParentID(int ParentID)  { m_parentID = ParentID; }

    /*! Set G4 ID number of photons track
     */
    void setTrackID(int TrackID) {  m_trackID = TrackID; }


  private:
    int m_moduleID;           /**< ID number of module that registered hit*/
    int m_barID;              /**< ID number of bar in which the hit was registered*/
    TVector3 m_position;      /**< local position of the hit no the pad*/
    TVector3 m_gposition;     /**< global position of hit in lab frame*/
    TVector3 m_direction;     /**< momentum direction of the hit*/
    TVector3 m_vposition;     /**< vertex position of the photon*/
    TVector3 m_vdirection;    /**< momentum direction of photon at vertex position*/
    double m_globalTime;      /**< Global time of hit */
    double m_localTime;       /**< time that passes since the photon was created*/
    double m_length;          /**< track length of the photon*/
    double m_energy;          /**< Energy of detected photon */
    int m_parentID;           /**< G4 ID number of photons parent particle */
    int m_trackID;            /**< G4 ID number of photons track */


    ClassDef(TOPSimHit, 1); /**< the class title */

  };

} // end namespace Belle2

#endif
