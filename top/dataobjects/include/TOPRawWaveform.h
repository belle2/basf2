/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <vector>

namespace Belle2 {

  /**
   * Class to store IRS waveforms
   * http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format
   */
  class TOPRawWaveform : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPRawWaveform():
      m_barID(0),
      m_pixelID(0),
      m_channelID(0),
      m_scrodID(0),
      m_scrodRevision(0),
      m_freezeDate(0),
      m_triggerType(0),
      m_flags(0),
      m_referenceASIC(0),
      m_segmentASIC(0)
    {}

    /**
     * Full constructor
     */
    TOPRawWaveform(int barID,
                   int pixelID,
                   unsigned channelID,
                   unsigned scrod,
                   unsigned freezeDate,
                   unsigned triggerType,
                   unsigned flags,
                   unsigned referenceASIC,
                   unsigned segmentASIC,
                   const std::vector<unsigned short>& data):
      m_data(data)
    {
      m_barID = barID;
      m_pixelID = pixelID;
      m_channelID = channelID;
      m_scrodID = scrod & 0xFFFF;
      m_scrodRevision = (scrod >> 16) & 0x00FF;
      m_freezeDate = freezeDate;
      m_triggerType = triggerType;
      m_flags = flags;
      m_referenceASIC = referenceASIC;
      m_segmentASIC = segmentASIC;
    }

    /**
     * Returns quartz bar ID
     * @return bar ID
     */
    int getBarID() const { return m_barID; }

    /**
     * Returns software channel ID (1-based)
     * @return pixel ID
     */
    int getPixelID() const { return m_pixelID; }

    /**
     * Returns hardware channel ID
     * @return channel ID
     */
    unsigned getChannelID() const { return m_channelID; }

    /**
     * Returns SCROD ID
     * @return SCROD ID
     */
    unsigned getScrodID() const { return m_scrodID; }

    /**
     * Returns SCROD revision number
     * @return revision number
     */
    unsigned getScrodRevision() const { return m_scrodRevision; }

    /**
     * Returns protocol freeze date
     * @return date as YYYYMMDD in BCD
     */
    unsigned getFreezeDate() const { return m_freezeDate; }

    /**
     * Returns trigger type bits
     * @return trigger type bits
     */
    unsigned getTriggerType() const { return m_triggerType; }

    /**
     * Checks if trigger is hardware issued
     * @return true, if hardware trigger
     */
    bool isHardwareTrigger() const { return (m_triggerType & 0x0001);}

    /**
     * Checks if trigger is software issued
     * @return true, if software trigger
     */
    bool isSoftwareTrigger() const { return (m_triggerType & 0x0002);}

    /**
     * Checks for truncated events
     * @return true for truncated events
     */
    bool isTruncatedEvents() const { return (m_triggerType & 0x0004);}

    /**
     * Returns event flag bits
     * @return event flag bits
     */
    unsigned getEventFlags() const { return m_flags; }

    /**
     * Checks if run in pedestal mode
     * @return true, if pedestal mode
     */
    bool isPedestalMode() const { return (m_flags & 0x0001);}

    /**
     * Returns IRS reference analog storage window.
     * This corresponds to the last window in the analog memory sampled.
     * All timing is a "look-back" from this window.
     * @return reference window ID
     */
    unsigned getReferenceWindow() const { return m_referenceASIC; }

    /**
     * Returns IRS analog storage window this waveform was taken from.
     * @return segment window ID
     */
    unsigned getStorageWindow() const { return (m_segmentASIC & 0x01FF); }

    /**
     * Returns ASIC channel
     * @return channel number
     */
    unsigned getAsicChannel() const { return ((m_segmentASIC >> 9) & 0x0007);}

    /**
     * Returns ASIC row
     * @return row number
     */
    unsigned getAsicRow() const { return ((m_segmentASIC >> 12) & 0x0003);}

    /**
     * Returns ASIC column
     * @return column number
     */
    unsigned getAsicCol() const { return ((m_segmentASIC >> 14) & 0x0003);}

    /**
     * Returns waveform size
     * @return size
     */
    unsigned getSize() const {return m_data.size();}

    /**
     * Returns waveform
     * @return vector of ADC values
     */
    const std::vector<unsigned short>& getWaveform() const
    {
      return m_data;
    }

  private:

    int m_barID;                    /**< quartz bar ID */
    int m_pixelID;                  /**< software channel ID */
    unsigned m_channelID;           /**< hardware channel ID */
    unsigned short m_scrodID;       /**< SCROD ID */
    unsigned short m_scrodRevision; /**< SCROD revision number */
    unsigned m_freezeDate;          /**< protocol freeze date (YYYYMMDD in BCD) */
    unsigned short m_triggerType;   /**< trigger type (bits 0:7) */
    unsigned short m_flags;         /**< event flags (bits 0:7) */
    unsigned short m_referenceASIC; /**< reference ASIC window */
    unsigned short m_segmentASIC;   /**< segment ASIC window (storage window) */
    std::vector<unsigned short> m_data;  /**< waveform ADC values */

    ClassDef(TOPRawWaveform, 1); /**< ClassDef */

  };


} // end namespace Belle2

