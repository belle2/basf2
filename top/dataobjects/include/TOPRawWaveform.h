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
#include <top/FeatureExtractionData.h>
#include <vector>
#include <string>
#include <algorithm>

namespace Belle2 {

  /**
   * Class to store raw IRS waveforms.
   * IRS3B: http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format
   * IRSX: https://confluence.desy.de/download/attachments/34035431/data_format_v2_0.xlsx
   */
  class TOPRawWaveform : public RelationsObject {
  public:

    /**
     * Various constants
     */
    enum {c_WindowSize = 64 /**< number of samples per window */
         };

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
     * @param windows storage windows of waveform segments
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
                   const std::vector<unsigned short>& windows,
                   unsigned electronicType,
                   const std::string& electronicName,
                   const std::vector<short>& data):
      m_moduleID(moduleID), m_pixelID(pixelID), m_channel(channel),
      m_freezeDate(freezeDate), m_triggerType(triggerType), m_flags(flags),
      m_referenceASIC(referenceASIC), m_segmentASIC(segmentASIC),
      m_windows(windows), m_data(data),  m_electronicType(electronicType),
      m_electronicName(electronicName)
    {
      m_scrodID = scrod & 0xFFFF;
      m_scrodRevision = (scrod >> 16) & 0x00FF;
    }

    /**
     * Sets pedestal subtracted flag
     */
    void setPedestalSubtractedFlag(bool value) {m_pedestalSubtracted = value;}

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
     * Tells whether pedestal already subtracted or not
     * @return true if subtracted
     */
    bool isPedestalSubtracted() const {return m_pedestalSubtracted;}

    /**
     * Returns IRS reference analog storage window.
     * This corresponds to the last window in the analog memory sampled.
     * All timing is a "look-back" from this window.
     * @return reference window number
     */
    unsigned getReferenceWindow() const { return m_referenceASIC; }

    /**
     * Returns IRS analog storage window this waveform was taken from.
     * @return first segment window number
     */
    unsigned getStorageWindow() const { return (m_segmentASIC & 0x01FF); }

    /**
     * Returns storage window numbers of waveform segments
     * @return window numbers
     */
    const std::vector<unsigned short>& getStorageWindows() const { return m_windows; }

    /**
     * Returns ASIC channel number
     * @return channel number
     */
    unsigned getASICChannel() const { return ((m_segmentASIC >> 9) & 0x0007);}

    /**
     * Returns ASIC number
     * @return ASIC number
     */
    unsigned getASICNumber() const { return ((m_segmentASIC >> 12) & 0x0003);}

    /**
     * Returns carrier board number
     * @return carrier number
     */
    unsigned getCarrierNumber() const { return ((m_segmentASIC >> 14) & 0x0003);}

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
    const std::vector<short>& getWaveform() const
    {
      return m_data;
    }

    /**
     * Checks if storage windows come in the consecutive order before the last sample
     * (no gaps in between before the last sample)
     * @param lastSample last sample
     * @param storageDepth storage depth
     * @return true, if no gaps before the last sample
     */
    bool areWindowsInOrder(unsigned lastSample = 0xFFFFFFFF,
                           unsigned short storageDepth = 508) const
    {
      unsigned last = lastSample / c_WindowSize + 1;
      unsigned size = m_windows.size();
      for (unsigned i = 1; i < std::min(last, size); i++) {
        int diff = m_windows[i] - m_windows[i - 1];
        if (diff < 0) diff += storageDepth;
        if (diff != 1) return false;
      }
      return true;
    }

    /**
     * Returns integral of a peak
     * @param sampleRise sample number just before 50% CFD crossing at leading edge
     * @param samplePeak sample number at maximum
     * @param sampleFall sample number just before 50% CFD crossing at falling edge
     * @return integral
     */
    int getIntegral(int sampleRise, int samplePeak, int sampleFall) const;

    /**
     * Do feature extraction
     * @param threshold pulse height threshold [ADC counts]
     * @param hysteresis threshold hysteresis [ADC counts]
     * @param thresholdCount minimal number of samples above threshold
     * @return number of feature extraction data (hits found)
     */
    int featureExtraction(int threshold, int hysteresis, int thresholdCount) const;

    /**
     * Returns feature extraction data
     * @return FE data
     */
    const std::vector<TOP::FeatureExtractionData>& getFeatureExtractionData() const
    {
      return m_features;
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
    std::vector<unsigned short> m_windows; /**< storage windows of waveform segments */
    std::vector<short> m_data;      /**< waveform ADC values */
    unsigned m_electronicType = 0;  /**< electronic type (see ChannelMapper::EType) */
    std::string m_electronicName;   /**< electronic name */
    bool m_pedestalSubtracted = false; /**< true, if pedestals already subtracted */

    /** cache for feature extraction data */
    mutable std::vector<TOP::FeatureExtractionData> m_features; //!

    ClassDef(TOPRawWaveform, 7); /**< ClassDef */

  };


} // end namespace Belle2
