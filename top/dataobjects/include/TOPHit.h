/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPHIT_H
#define TOPHIT_H

#include <TObject.h>

namespace Belle2 {


  //! Class TOPHit - Geant4 simulated hit for TOP.
  /*!
   This is a class to store TOP geant4 hit in datastore.
   It is also the input for digitization module (TOPDigi).
   */

  class TOPHit : public TObject {
  public:

    //! Default constructor
    TOPHit():
        m_moduleID(0),
        m_barID(0),
        m_channelID(0),
        m_globalTime(0),
        m_energy(0),
        m_parentID(0) {
      /*! Does nothing */
    }

    //! Full constructor.
    /*!
     \param moduleID ID of hapd module containing hit
     \param position vector of hit local position (in module coor. sys.)
     \param globalTime global time of photon hit
     \param energy energy of photon
     \param parentID geant4 id of photon parent particle
     */

    TOPHit(
      int moduleID,
      int barID,
      int channelID,
      double globalTime,
      double energy,
      int parentID):
        m_moduleID(moduleID),
        m_channelID(channelID),
        m_globalTime(globalTime),
        m_energy(energy),
        m_parentID(parentID) {
      /* Does nothing */
    }

    //! Get ID number of module that registered hit
    int getModuleID() const { return m_moduleID; }

    //! Get ID number of bar that registered hit
    int getBarID() const { return m_barID; }

    //! Get the channel ID of the hit in the PMT
    int getChannelID() const { return m_channelID; }

    //! Get global time of hit
    double getGlobalTime() const { return m_globalTime; }

    //! Get detected photon energy
    double getEnergy() const { return m_energy; }

    //! Get G4 ID number of photons parent particle
    int getParentID() const { return m_parentID; }

    //! Set ID number of module that registered hit
    void setModuleID(int moduleID) { m_moduleID = moduleID; }

    //! Set global time of hit
    void setGlobalTime(double globalTime) { m_globalTime = globalTime; }

    //! Get detected photon energy
    void setEnergy(double energy) { m_energy = energy; }

    //! Get G4 ID number of photons parent particle
    void setParentID(int parentID) { m_parentID = parentID; }

  private:
    int m_moduleID;           /**< ID number of module that registered hit*/
    int m_barID;           /**< ID number of bar that registered hit*/
    int m_channelID;      /**< Channel ID of the hit in the PMT */
    double m_globalTime;      /**< Global time of hit */
    double m_energy;          /**< Energy of detected photon */
    int m_parentID;        /**< G4 ID number of photons parent particle */


    ClassDef(TOPHit, 1); /**< the class title */

  };

} // end namespace Belle2

#endif
