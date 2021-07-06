/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHDIGIT_H
#define ARICHDIGIT_H

// ROOT
#include <framework/dataobjects/DigitBase.h>
#include <stdint.h>

namespace Belle2 {


  //! ARICHDigit class for storing photon hit information.
  /*! This class holds the ARICH hit information after digitization (obtained from ARICHSimHit after ARICHDigitizer module). Contains only module number, channel number and hit bitmap (4 bits).
  */

  class ARICHDigit : public DigitBase {

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

    /**
     * Implementation of the base class function.
     * Enables BG overlay module to identify uniquely the physical channel of this Digit.
     * @return unique channel ID, composed of channel ID (1-512) and bar ID (1-16)
     */
    unsigned int getUniqueChannelID() const override {return m_channelID + (m_moduleID << 16);}

    /**
     * Implementation of the base class function
     * @param bg BG digit
     * @return append status
     *
     * NOTE: Implementation is provisional (number of "1"s in channel bitmap is equal to number of p.e. that hit channel).
     * Should be updated together with digitizer update, to emulate more proper response.
     */
    DigitBase::EAppendStatus addBGDigit(const DigitBase* bg) override;

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
    ClassDefOverride(ARICHDigit, 3); /**< the class title */

  };

} // end namespace Belle2

#endif
