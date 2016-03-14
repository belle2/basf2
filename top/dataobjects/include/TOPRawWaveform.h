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
#include <string>

namespace Belle2 {

  /**
   * Class to store raw IRS waveforms.
   * IRS3B: http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format
   * IRSX: https://belle2.cc.kek.jp/~twiki/pub/Detector/TOP/Module01Firmware/data_format_v1_5.xlsx
   */
  class TOPRawWaveform : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPRawWaveform()
    {}

    /**
     * Full constructor
     * @param moduleID module ID
     * @param pixelID pixel (e.g. software channel) ID
     * @param channel hardware channel number
     * @param scrod SCROD ID
     * @param freezeDate protocol freeze date (YYYYMMDD in BCD)
     * @param triggerType trigger type
     * @param flags event flags
     * @param referenceASIC reference ASIC window number
     * @param segmentASIC segment ASIC window number (storage window)
     * @param electronicType electronic type (see ChannelMapper::EType)
     * @param electronicName electronic name (e.g. "IRS3B", "IRSX", etc.)
     * @param data waveform ADC values (samples)
     */
    TOPRawWaveform(int moduleID,
                   int pixelID,
                   unsigned channel,
                   unsigned scrod,
                   unsigned freezeDate,
                   unsigned triggerType,
                   unsigned flags,
                   unsigned referenceASIC,
                   unsigned segmentASIC,
                   unsigned electronicType,
                   std::string electronicName,
                   const std::vector<unsigned short>& data):
      m_data(data),  m_electronicName(electronicName)
    {
      m_moduleID = moduleID;
      m_pixelID = pixelID;
      m_channel = channel;
      m_scrodID = scrod & 0xFFFF;
      m_scrodRevision = (scrod >> 16) & 0x00FF;
      m_freezeDate = freezeDate;
      m_triggerType = triggerType;
      m_flags = flags;
      m_referenceASIC = referenceASIC;
      m_segmentASIC = segmentASIC;
      m_electronicType = electronicType;
    }

    /**
     * Returns module ID
     * @return module ID
     */
    int getModuleID() const { return m_moduleID; }

    /**
     * Returns software channel ID (1-based)
     * @return pixel ID
     */
    int getPixelID() const { return m_pixelID; }

    /**
     * Returns hardware channel number
     * @return channel number
     */
    unsigned getChannel() const { return m_channel; }

    /**
     * Returns SCROD ID
     * @return SCROD ID
     */
    unsigned getScrodID() const { return m_scrodID; }

    /**
     * Returns SCROD revision number (only given for IRS3B)
     * @return revision number
     */
    unsigned getScrodRevision() const { return m_scrodRevision; }

    /**
     * Returns protocol freeze date (only given for IRS3B)
     * @return date as YYYYMMDD in BCD
     */
    unsigned getFreezeDate() const { return m_freezeDate; }

    /**
     * Returns trigger type bits
     * @return trigger type bits
     */
    unsigned getTriggerType() const { return m_triggerType; }

    /**
     * Checks if trigger is hardware issued (according to IRS3B definition)
     * @return true, if hardware trigger
     */
    bool isHardwareTrigger() const { return (m_triggerType & 0x0001);}

    /**
     * Checks if trigger is software issued (according to IRS3B definition)
     * @return true, if software trigger
     */
    bool isSoftwareTrigger() const { return (m_triggerType & 0x0002);}

    /**
     * Checks for truncated events (according to IRS3B definition)
     * @return true for truncated events
     */
    bool isTruncatedEvents() const { return (m_triggerType & 0x0004);}

    /**
     * Returns event flag bits (only given for IRS3B)
     * @return event flag bits
     */
    unsigned getEventFlags() const { return m_flags; }

    /**
     * Checks if run in pedestal mode (only given for IRS3B)
     * @return true, if pedestal mode
     */
    bool isPedestalMode() const { return (m_flags & 0x0001);}

    /**
     * Returns IRS reference analog storage window.
     * This corresponds to the last window in the analog memory sampled.
     * All timing is a "look-back" from this window.
     * @return reference window number
     */
    unsigned getReferenceWindow() const { return m_referenceASIC; }

    /**
     * Returns IRS analog storage window this waveform was taken from.
     * @return segment window number
     */
    unsigned getStorageWindow() const { return (m_segmentASIC & 0x01FF); }

    /**
     * Returns ASIC channel number
     * @return channel number
     */
    unsigned getASICChannel() const { return ((m_segmentASIC >> 9) & 0x0007);}

    /**
     * Returns ASIC number
     * @return ASIC number
     */
    unsigned getASICNumber() const { return ((m_segmentASIC >> 14) & 0x0003);}

    /**
     * Returns carrier board number
     * @return carrier number
     */
    unsigned getCarrierNumber() const { return ((m_segmentASIC >> 12) & 0x0003);}

    /**
     * Returns ASIC row (IRS3B naming convention)
     * @return row number
     */
    unsigned getASICRow() const { return getCarrierNumber();}

    /**
     * Returns ASIC column (IRS3B naming convention)
     * @return column number
     */
    unsigned getASICCol() const { return getASICNumber();}

    /**
     * Returns type of electronic used to measure this waveform
     * @return type (see ChannelMapper::EType)
     */
    unsigned getElectronicType() const {return m_electronicType;}

    /**
     * Returns the name of electronic used to measure this waveform
     * @return name
     */
    std::string getElectronicName() const {return m_electronicName;}

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

    int m_moduleID = 0;                 /**< module ID */
    int m_pixelID = 0;                  /**< software channel ID */
    unsigned m_channel = 0;             /**< hardware channel number */
    unsigned short m_scrodID = 0;       /**< SCROD ID */
    unsigned short m_scrodRevision = 0; /**< SCROD revision number */
    unsigned m_freezeDate = 0;          /**< protocol freeze date (YYYYMMDD in BCD) */
    unsigned short m_triggerType = 0;   /**< trigger type (bits 0:7) */
    unsigned short m_flags = 0;         /**< event flags (bits 0:7) */
    unsigned short m_referenceASIC = 0; /**< reference ASIC window */
    unsigned short m_segmentASIC = 0;   /**< segment ASIC window (storage window) */
    std::vector<unsigned short> m_data;  /**< waveform ADC values */
    unsigned m_electronicType = 0;      /**< electronic type (see ChannelMapper::EType) */
    std::string m_electronicName;   /**< electronic name */

    ClassDef(TOPRawWaveform, 4); /**< ClassDef */

  };


} // end namespace Belle2

