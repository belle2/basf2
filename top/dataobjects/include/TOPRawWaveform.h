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
#include <algorithm>

namespace Belle2 {

  /**
   * Class to store raw data waveforms.
   */
  class TOPRawWaveform : public RelationsObject {
  public:

    /**
     * Various constants
     */
    enum {c_WindowSize = 64 /**< number of samples per ASIC window */
         };

    /**
     * Feature extraction data
     */
    struct FeatureExtraction {
      int sampleRise = 0; /**< sample number just before 50% CFD crossing */
      int samplePeak = 0; /**< sample number at maximum */
      int sampleFall = 0; /**< same for falling edge */
      short vRise0 = 0;   /**< ADC value at sampleRise */
      short vRise1 = 0;   /**< ADC value at sampleRise + 1 */
      short vPeak = 0;    /**< ADC value at samplePeak */
      short vFall0 = 0;   /**< ADC value at sampleFall */
      short vFall1 = 0;   /**< ADC value at sampleFall + 1 */
      int integral = 0;   /**< integral of a pulse (e.g. \propto charge) */
    };


    /**
     * Default constructor
     */
    TOPRawWaveform()
    {}


    /**
     * Useful constructor
     * @param moduleID module (slot) ID (1-based)
     * @param pixelID pixel ID (1-based)
     * @param channel hardware channel number (0-based)
     * @param scrodID SCROD ID
     * @param window hardware logic window number (storage window)
     * @param startSample sample number of the first waveform sample
     * @param data waveform ADC values
     */
    TOPRawWaveform(int moduleID,
                   int pixelID,
                   unsigned channel,
                   unsigned scrodID,
                   unsigned window,
                   unsigned startSample,
                   const std::vector<short>& data):
      m_moduleID(moduleID), m_pixelID(pixelID), m_channel(channel), m_scrodID(scrodID),
      m_window(window), m_startSample(startSample), m_data(data),
      m_physicalWindow(window)
    {}

    /**
     * Sets pedestal subtracted flag
     */
    void setPedestalSubtractedFlag(bool value) {m_pedestalSubtracted = value;}

    /**
     * Sets hardware physical window number
     */
    void setPhysicalWindow(unsigned window) {m_physicalWindow = window;}

    /**
     * Sets current (reference) window number
     */
    void setLastWriteAddr(unsigned lastWriteAddr) {m_lastWriteAddr = lastWriteAddr;}

    /**
     * Sets storage window numbers
     */
    void setStorageWindows(const std::vector<unsigned short>& windows)
    {
      m_windows = windows;
    }

    /**
     * Returns module ID
     * @return module ID
     */
    int getModuleID() const { return m_moduleID; }

    /**
     * Returns pixel ID (1-based)
     * @return pixel ID
     */
    int getPixelID() const { return m_pixelID; }

    /**
     * Returns hardware channel number (0-based)
     * @return channel number
     */
    unsigned getChannel() const { return m_channel; }

    /**
     * Returns SCROD ID
     * @return SCROD ID
     */
    unsigned getScrodID() const { return m_scrodID; }

    /**
     * Returns hardware logic window number (storage window)
     * @return window number
     */
    unsigned getStorageWindow() const { return m_window; }

    /**
     * Returns sample number of the first waveform sample
     * @return sample number
     */
    unsigned getStartSample() const { return m_startSample;}

    /**
     * Tells whether pedestal already subtracted or not
     * @return true if subtracted
     */
    bool isPedestalSubtracted() const {return m_pedestalSubtracted;}

    /**
     * Returns hardware physical window number
     * @return window number
     */
    unsigned getPhysicalWindow() const {return m_physicalWindow;}

    /**
     * Returns IRS reference analog storage window (last write address).
     * This corresponds to the last window in the analog memory sampled.
     * All timing is a "look-back" from this window.
     * @return window number
     */
    unsigned getReferenceWindow() const { return m_lastWriteAddr; }

    /**
     * Returns storage window numbers of waveform segments
     * @return window numbers
     */
    const std::vector<unsigned short>& getStorageWindows() const { return m_windows; }

    /**
     * Returns ASIC channel number
     * @return channel number
     */
    unsigned getASICChannel() const {return m_channel & 0x07;}

    /**
     * Returns ASIC number
     * @return ASIC number
     */
    unsigned getASICNumber() const {return (m_channel >> 3) & 0x03;}

    /**
     * Returns carrier board number
     * @return carrier number
     */
    unsigned getCarrierNumber() const {return (m_channel >> 5) & 0x03;}

    /**
     * Returns boardstack number
     * @return boardstack number
     */
    unsigned getBoardstackNumber() const {return (m_channel >> 7) & 0x03;}

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
    const std::vector<FeatureExtraction>& getFeatureExtractionData() const
    {
      return m_features;
    }


  private:

    int m_moduleID = 0;                 /**< module ID */
    int m_pixelID = 0;                  /**< software channel ID */
    unsigned m_channel = 0;             /**< hardware channel number */
    unsigned short m_scrodID = 0;       /**< SCROD ID */
    unsigned short m_window = 0;   /**< hardware logic window number (storage window) */
    unsigned short m_startSample = 0;   /**< sample number of the first waveform sample */
    std::vector<short> m_data;      /**< waveform ADC values */
    bool m_pedestalSubtracted = false; /**< true, if pedestals already subtracted */

    unsigned short m_physicalWindow = 0;/**< hardware physical window number */
    unsigned short m_lastWriteAddr = 0; /**< current (reference) window number */
    std::vector<unsigned short> m_windows; /**< storage windows of waveform segments */

    /** cache for feature extraction data */
    mutable std::vector<FeatureExtraction> m_features; //!

    ClassDef(TOPRawWaveform, 8); /**< ClassDef */

  };


} // end namespace Belle2
