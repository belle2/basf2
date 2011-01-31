/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPHIT_H
#define TOPHIT_H

// ROOT
#include <TObject.h>

namespace Belle2 {

  /** This class holds the TOP hit information after digitization (obtained from TOPSimHit after TOPDigi). Only module number, channel number and global time.
  */

  class TOPHit : public TObject {

  public:

    //! Default constructor for ROOT IO.
    TOPHit() {;}

    //! Useful constructor
    TOPHit(int moduleID, int channelID, double globalTime):
        m_moduleID(moduleID),
        m_channelID(channelID),
        m_globalTime(globalTime) {
      /*! Does nothing */
    }

    //! Get ID number of module that registered hit
    int getModuleID() const { return m_moduleID; }

    //! Get ID number of hit channel
    int getChannelID() const { return m_channelID; }

    //! Get global time of hit
    double getGlobalTime() const { return m_globalTime; }

    //! Set ID number of module that registered hit
    void setModuleID(int moduleID) { m_moduleID = moduleID; }

    //! Set ID number of hit channel
    void setChannelID(int channelID) { m_channelID = channelID; }

    //! Set global time of hit
    void setGlobalTime(double globalTime) { m_globalTime = globalTime; }

  private:

    int m_moduleID;           /*!< ID number of module that registered hit */
    int m_channelID;          /*!< ID number of hit channel */
    double m_globalTime;      /*!< Global time of hit */

    ClassDef(TOPHit, 1); /*!< the class title */

  };

} // end namespace Belle2

#endif
