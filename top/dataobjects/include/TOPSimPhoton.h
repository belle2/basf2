/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPSIMPHOTON_H
#define TOPSIMPHOTON_H

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {
  /** \addtogroup dataobjects
   * @{
   */

  /*! Class to store G4 Cerenkov photons entering PMT's (under developement!)
   * relation to TOPSimHit
   * filled in top/simulation/src/SensitivePMT.cc
   */

  class TOPSimPhoton : public TObject {
  public:

    /*! Default constructor
     */
    TOPSimPhoton():
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
     * @param pmtID         PMT ID
     * @param barID         bar ID
     * @param position      detection position in local (PMT) frame
     * @param gposition     detection point
     * @param direction     detection direction
     * @param vposition     emission point
     * @param vdirection    emission direction
     * @param globalTime    detection time
     * @param emissionTime  emission time
     * @param length        photon track length
     * @param energy        photon energy in [eV]
     * @param parentID      geant4 id of parent particle
     * @param trackID       geant4 id of photon
     */
    TOPSimPhoton(
      int pmtID,
      int barID,
      TVector3 position,
      TVector3 gposition,
      TVector3 direction,
      TVector3 vposition,
      TVector3 vdirection,
      double globalTime,
      double emissionTime,
      double length,
      double energy,
      int parentID,
      int trackID):
      m_moduleID(pmtID),
      m_barID(barID),
      m_position(position),
      m_gposition(gposition),
      m_direction(direction),
      m_vposition(vposition),
      m_vdirection(vdirection),
      m_globalTime(globalTime),
      m_localTime(emissionTime),
      m_length(length),
      m_energy(energy),
      m_parentID(parentID),
      m_trackID(trackID) {
    }

    /*! Get PMT ID
     * @return PMT ID
     */
    int getPmtID() const { return m_moduleID; }

    /*! Get PMT ID (for backward comp. - same as getPmtID)
     * @return PMT ID
     */
    int getModuleID() const { return m_moduleID; }

    /*! Get bar ID
     * @return bar ID
     */
    int getBarID() const { return m_barID; }

    /*! Get local position of hit (in PMT coordinates)
     * @return local position
     */
    const TVector3& getPosition() const { return m_position; }

    /*! Get global position of hit
     * @return detection point
     */
    const TVector3& getGPosition() const { return m_gposition; }

    /*! Get photon direction at detection (global frame)
     * @return direction vector at detection
     */
    const TVector3& getDirection() const { return m_direction; }

    /*! Get position at emission (global frame)
     * @return emission point
     */
    const TVector3& getVposition() const { return m_vposition; }

    /*! Get photon direction at emission (global frame)
     * @return direction vector at emission
     */
    const TVector3& getVdirection() const { return m_vdirection; }

    /*! Get detection time
     * @return detection time
     */
    double getTime() const { return m_globalTime; }

    /*! Get emission time
     * @return emission time
     */
    double getEmissionTime() const { return m_localTime; }

    /*! Get propagation length
     * @return propagation length
     */
    double getLength() const { return m_length; }

    /*! Get photon energy
     * @return photon energy in [eV]
     */
    double getEnergy() const { return m_energy; }

    /*! Get G4 ID of parent particle
     * @return GeantID
     */
    int getParentID() const { return m_parentID; }

    /*! Get G4 ID of photon
     * @return GeantID
     */
    int getTrackID() const { return m_trackID; }

  private:
    int m_moduleID;           /**< PMT ID */
    int m_barID;              /**< bar ID */
    TVector3 m_position;      /**< detection position in local (PMT) frame */
    TVector3 m_gposition;     /**< detection position  */
    TVector3 m_direction;     /**< detection direction vector */
    TVector3 m_vposition;     /**< emission point */
    TVector3 m_vdirection;    /**< emission direction vector */
    double m_globalTime;      /**< detection time */
    double m_localTime;       /**< emission time */
    double m_length;          /**< photon propagation length */
    double m_energy;          /**< photon energy in [eV] */
    int m_parentID;           /**< G4 ID of parent particle */
    int m_trackID;            /**< G4 ID of photon */

    ClassDef(TOPSimPhoton, 1); /**< ClassDef */

  };

  /** @}*/

} // end namespace Belle2

#endif

