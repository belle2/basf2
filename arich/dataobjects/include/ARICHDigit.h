/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHDIGIT_H
#define ARICHDIGIT_H

// ROOT
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {


  //! ARICHDigit class for storing photon hit information.
  /*! This class holds the ARICH hit information after digitization (obtained from ARICHSimHit after ARICHDigitizer module). Contains only module number, channel number and global time.
  */

  class ARICHDigit : public RelationsObject {

  public:

    //! Default constructor for ROOT IO.
    ARICHDigit():
      m_moduleID(-1),
      m_channelID(-1),
      m_globalTime(0.0) {
      /*! does nothing */
    }

    //! Useful constructor
    /*!
      \param moduleID Id number of module containing hit
      \param channelID Id number of hit channel
      \param globalTime global time of hit
     */
    ARICHDigit(int moduleID, int channelID, double globalTime) {
      m_moduleID = moduleID;
      m_channelID = channelID;
      m_globalTime = (float) globalTime;
    }

    //! Destructor
    ~ARICHDigit() {
      /*! Does nothing */
    }

    //! Get ID number of module that registered hit
    int getModuleID() const { return m_moduleID; }

    //! Get ID number of hit channel
    int getChannelID() const { return m_channelID; }

    //! Get global time of hit
    double getGlobalTime() const { return m_globalTime; }
    /*
    //! Set ID number of module that registered hit
    void setModuleID(int moduleID) { m_moduleID = moduleID; }

    //! Set ID number of hit channel
    void setChannelID(int channelID) { m_channelID = channelID; }

    //! Set global time of hit
    void setGlobalTime(double globalTime) { m_globalTime = globalTime; }
    */
  private:

    int m_moduleID;           /**< ID number of module that registered hit */
    int m_channelID;          /**< ID number of hit channel */
    float m_globalTime;      /**< Global time of hit */

    ClassDef(ARICHDigit, 1); /**< the class title */

  };

} // end namespace Belle2

#endif
