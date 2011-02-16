/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
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

  class TOPB4VHit;

  /** Class TOPSimHit - Geant4 simulated hit for TOP.
   *
   *  This is a class to store TOP geant4 hit in datastore.
   *  It is also the input for digitization module (TOPDigi).
   */

  class TOPSimHit : public TObject {
  public:

    /* Default constructor */
    TOPSimHit():
        m_moduleID(0),
        m_position(0, 0, 0),
        m_globalTime(0),
        m_energy(0),
        m_parentID(0) {
      /*! Does nothing */
    }

    /** Full constructor.*/
    TOPSimHit(
      int moduleID,
      TVector3 position,
      double globalTime,
      double energy,
      int parentID):
        m_moduleID(moduleID),
        m_position(position),
        m_globalTime(globalTime),
        m_energy(energy),
        m_parentID(parentID) {
      /* Does nothing */
    }


    //! Destructor
    ~TOPSimHit() {
      /* Does nothing */
    }

    //! Get ID number of module that registered hit
    int getModuleID() const { return m_moduleID; }

    //! Get local position of hit (in module coordinates)
    const TVector3& getLocalPosition() const { return m_position; }

    //! Get global time of hit
    double getGlobalTime() const { return m_globalTime; }

    //! Get detected photon energy
    double getEnergy() const { return m_energy; }

    //! Get G4 ID number of photons parent particle
    int getParentID() const { return (int)m_parentID; }

    //! Set ID number of module that registered hit
    void setModuleID(int moduleID) { m_moduleID = moduleID; }

    //! Set local position of hit (in module coordinates)
    void setLocalPosition(double x, double y, double z) { m_position.SetXYZ(x, y, z); }

    //! Set global time of hit
    void setGlobalTime(double globalTime) { m_globalTime = globalTime; }

    //! Get detected photon energy
    void setEnergy(double energy) { m_energy = energy; }

    //! Get G4 ID number of photons parent particle
    void setParentID(int parentID) { m_parentID = parentID; }

  private:
    int m_moduleID;           /*!< ID number of module that registered hit*/
    TVector3 m_position;      /*!< Local position of hit (in module coordinates) */
    double m_globalTime;      /*!< Global time of hit */
    double m_energy;          /*!< Energy of detected photon */
    double m_parentID;        /*!< G4 ID number of photons parent particle */


    ClassDef(TOPSimHit, 1); /*!< the class title */

  };

} // end namespace Belle2

#endif
