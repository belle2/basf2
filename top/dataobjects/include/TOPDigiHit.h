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


  //! Class TOPDigiHit - Stores hits after digitisation
  /*!
   This is a class to store TOP geant4 hit in datastore.
   It is also the input for digitization module (TOPDigi).
   */

  class TOPDigiHit : public TObject {
  public:

    //! Default constructor
    TOPDigiHit():
        m_barID(0),
        m_channelID(0),
        m_TDC(0),
        m_energy(0.),
        m_parentID(0),
        m_trackID(0) {
      /*! Does nothing */
    }

    //! Full constructor.
    /*!
     \param barID is the ID of the bar
     \param channelID is the ID of the digitised channel
     \param TDC it the TDC of global time
     \param energy energy of photon
     \param parentID geant4 id of photon parent particle
     \param trackID geant4 id of photon track
     */

    TOPDigiHit(
      int barID,
      int channelID,
      int TDC,
      double energy,
      int parentID,
      int trackID):
        m_barID(barID),
        m_channelID(channelID),
        m_TDC(TDC),
        m_energy(energy),
        m_parentID(parentID),
        m_trackID(trackID) {
      /* Does nothing */
    }


    //! Get ID number of bar that registered hit
    int getBarID() const { return m_barID; }

    //! Get the channel ID of the hit in the PMT
    int getChannelID() const { return m_channelID; }

    //! Get TDC time of hit
    int getTDC() const { return m_TDC; }

    //! Get detected photon energy
    double getEnergy() const { return m_energy; }

    //! Get G4 ID number of photons parent particle
    int getParentID() const { return m_parentID; }

    //! Get G4 ID number of photons track
    int getTrackID() const { return m_trackID; }

    //! Set ID number of module that registered hit
    void setBarID(int barID) { m_barID = barID; }

    //! Set ID number of module that registered hit
    void setChannelID(int channelID) { m_channelID = channelID; }

    //! Set global time of hit
    void setTDC(int TDC) { m_TDC = TDC; }

    //! Get detected photon energy
    void setEnergy(double energy) { m_energy = energy; }

    //! Get G4 ID number of photons parent particle
    void setParentID(int parentID) { m_parentID = parentID; }

    //! Get G4 ID number of photons track
    void setTrackID(int trackID) { m_trackID = trackID; }

  private:
    int m_barID;           /**< ID number of bar that registered hit*/
    int m_channelID;      /**< Channel ID of the hit in the PMT */
    int m_TDC;              /**< digitised time */
    double m_energy;          /**< Energy of detected photon */
    int m_parentID;        /**< G4 ID number of photons parent particle */
    int m_trackID;        /**< G4 ID number of photons track */

    ClassDef(TOPDigiHit, 1); /**< the class title */

  };

} // end namespace Belle2

#endif
