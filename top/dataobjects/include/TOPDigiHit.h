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


  /*! This is a class to store TOP digitized hits in datastore.
   *  It is also the output for digitization module (TOPDigi).
   */

  class TOPDigiHit : public TObject {
  public:

    /*! Default constructor
     */
    TOPDigiHit():
      m_barID(0),
      m_channelID(0),
      m_TDC(0),
      m_energy(0.),
      m_parentID(0),
      m_trackID(0) {
    }

    /*!  Full constructor.
     * @param barID is the ID of the bar
     * @param channelID is the ID of the digitised channel
     * @param TDC it the TDC of global time
     * @param energy energy of photon
     * @param parentID geant4 id of photon parent particle
     * @param trackID geant4 id of photon track
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
    }

    /*! -- Function for getting parameters out of TOPDigiHit -- */

    /*! Get ID number of bar that registered hit
     * @return ID of the bar in which the hit took place
     */
    int getBarID() const { return m_barID; }

    /*! Get channel ID in which the hit was registered
     * @return channel ID of hit
     */
    int getChannelID() const { return m_channelID; }

    /*! Get TDC of global time
     * @return TDC of hot
     */
    int getTDC() const { return m_TDC; }

    /*! get energy of the detected photon
     * @return energy of detected photon
     */
    double getEnergy() const { return m_energy; }

    /*! get the geant ID of the parent
     * @return GeantID of parent particle
     */
    int getParentID() const { return m_parentID; }

    /*! get track id of the particle
     * @return track ID of the hit
     */
    int getTrackID() const { return m_trackID; }

    /*! -- Function for parameters parameters in TOPDigiHit -- */

    /*! Set ID number of bar in which the hit was registered
     */
    void setBarID(int barID) { m_barID = barID; }

    /*! Set channel ID in which the hit was registered
     */
    void setChannelID(int channelID) { m_channelID = channelID; }

    /*! Set TDC of hit
     */
    void setTDC(int TDC) { m_TDC = TDC; }

    /*! Set photon energy of detected photon
     */
    void setEnergy(double energy) { m_energy = energy; }

    /*! Set geant ID of parent particle
     */
    void setParentID(int parentID) { m_parentID = parentID; }

    /*! Set track ID of detected photon
     */
    void setTrackID(int trackID) { m_trackID = trackID; }

  private:
    int m_barID;             /**< ID number of bar that registered hit*/
    int m_channelID;         /**< Channel ID of the hit */
    int m_TDC;               /**< digitised time */
    double m_energy;         /**< Energy of detected photon */
    int m_parentID;          /**< G4 ID number of photons parent particle */
    int m_trackID;           /**< G4 ID number of photons track */

    ClassDef(TOPDigiHit, 1); /**< the class title */

  };

} // end namespace Belle2

#endif
