/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * Class to store debug info of raw data in Interim FE format
   */

  class TOPInterimFEInfo : public RelationsObject {
  public:

    /**
     * Error flags
     */
    enum ErrorFlags {c_InvalidFEHeader    = 0x0001, /**< invalid FE header word */
                     c_DifferentScrodIDs  = 0x0002, /**< in HLSB and FE header */
                     c_InvalidMagicWord   = 0x0004, /**< at the end of FE header */
                     c_DifferentCarriers  = 0x0008, /**< in FE and WF header */
                     c_DifferentAsics     = 0x0010, /**< in FE and WF header */
                     c_DifferentChannels  = 0x0020, /**< in FE and WF header */
                     c_DifferentWindows   = 0x0040, /**< in FE and WF header */
                     c_InsufficientWFData = 0x0080, /**< too few words for waveform data */
                     c_InvalidScrodID     = 0x0100  /**< no front-end map available */
                    };

    /**
     * Default constructor
     */
    TOPInterimFEInfo()
    {}

    /**
     * Full constructor
     * @param scrodID scrod ID
     * @param dataSize data buffer size [32-bit words]
     */
    TOPInterimFEInfo(unsigned short scrodID, int dataSize):
      m_scrodID(scrodID), m_dataSize(dataSize)
    {}

    /**
     * Increments FE headers counter
     */
    void incrementFEHeadersCount() {m_FEHeaders++;}

    /**
     * Increments empty FE headers counter
     */
    void incrementEmptyFEHeadersCount() {m_emptyFEHeaders++;}

    /**
     * Increments waveforms counter
     */
    void incrementWaveformsCount() {m_waveforms++;}

    /**
     * Sets error flag
     * @param flag bit flag as defined in enum
     */
    void setErrorFlag(ErrorFlags flag) {m_errorFlags |= flag;}

    /**
     * Returns SCROD ID
     * @return SCROD ID
     */
    unsigned getScrodID() const {return m_scrodID;}

    /**
     * Returns data buffer size
     * @return size [32-bit words]
     */
    int getDataSize() const {return m_dataSize;}

    /**
     * Returns the number of FE headers found in data buffer
     * @return count
     */
    unsigned getFEHeadersCount() const {return m_FEHeaders;}

    /**
     * Returns the number of empty FE headers found in data buffer
     * @return count
     */
    unsigned getEmptyFEHeadersCount() const {return m_emptyFEHeaders;}

    /**
     * Returns the number of waveforms found in data buffer
     * @return count
     */
    unsigned getWaveformsCount() const {return m_waveforms;}

    /**
     * Returns error flags
     * @return bit flags
     */
    unsigned getErrorFlags() const {return m_errorFlags;}

    /**
     * Returns true if particular error flag is set
     * @param flag bit flag
     * @return true if set
     */
    bool isErrorBitSet(ErrorFlags flag) const {return (m_errorFlags & flag) == flag;}

  private:

    unsigned short m_scrodID = 0; /**< SCROD ID */
    int m_dataSize = 0; /**< data buffer size [32-bit words] */
    unsigned m_FEHeaders = 0; /**< number of FE headers in data buffer */
    unsigned m_emptyFEHeaders = 0; /**< number of empty FE headers in data buffer */
    unsigned m_waveforms = 0; /**< number of waveforms in data buffer */
    unsigned m_errorFlags = 0; /**< error flags, see enum */

    ClassDef(TOPInterimFEInfo, 1); /**< ClassDef */


  };

}
