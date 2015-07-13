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
#include <stdint.h>

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
      m_bitmap(0)
    {
      /*! does nothing */
    }

    //! Useful constructor
    /*!
      \param moduleID Id number of module containing hit
      \param channelID Id number of hit channel
      \param globalTime global time of hit
     */
    ARICHDigit(int moduleID, int channelID, uint8_t bitmap)
    {
      m_moduleID = moduleID;
      m_channelID = channelID;
      m_bitmap = bitmap;
    }

    //! Destructor
    ~ARICHDigit()
    {
      /*! Does nothing */
    }

    //! Get ID number of module that registered hit
    int getModuleID() const { return m_moduleID; }

    //! Get ID number of hit channel
    int getChannelID() const { return m_channelID; }

    //! Get global time of hit
    uint8_t getBitmap() const { return m_bitmap; }

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
    uint8_t m_bitmap;          /**< bitmap */
    ClassDef(ARICHDigit, 2); /**< the class title */

  };

} // end namespace Belle2

#endif
