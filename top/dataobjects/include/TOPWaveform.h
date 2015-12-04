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
   * Class to store pedestal-subtracted, gain-corrected and merged IRS waveforms.
   */
  class TOPWaveform : public RelationsObject {
  public:

    /**
     * Waveform sample (basic waveform element)
     */
    struct WFSample {
      float time = 0; /**< time */
      float adc = 0;  /**< adc value */
      float err = 0;  /**< uncertainty of adc value */
    };

    /**
     * Reconstructed hit
     */
    struct Hit {
      float time = 0;    /**< time */
      float height = 0;  /**< pulse height */
      float width = 0;   /**< pulse width */
      float area = 0;    /**< pulse area */
      float timeErr = 0;    /**< time uncertainty (rms) */
      float heightErr = 0;  /**< pulse height uncertainty (rms) */
      float widthErr = 0;   /**< pulse width uncertainty (rms) */
      float areaErr = 0;    /**< pulse area uncertainty (rms) */
      int imax = 0;         /**< sample number at the waveform maximum */
    };

    /**
     * Default class constructor
     */
    TOPWaveform()
    {}

    /**
     * Constructor with module ID, pixel ID and channel number - constructs empty waveform
     * @param barID module ID (1-based)
     * @param pixelID software channel ID (1-based)
     * @param channel hardware channel number (0-based)
     */
    TOPWaveform(int barID, int pixelID, unsigned channel):
      m_barID(barID), m_pixelID(pixelID), m_channel(channel)
    {}

    /**
     * Appends waveform sample to waveform
     * @param sample waveform sample
     */
    void appendSample(WFSample sample)
    {
      if (m_data.empty()) {
        sample.time = 0;
      } else {
        sample.time += m_data[m_data.size() - 1].time;
      }
      m_data.push_back(sample);
    }

    /**
     * Appends ASIC window number of the raw waveform
     * @param window window number
     */
    void appendWindowNumber(unsigned window) { m_windows.push_back(window);}

    /**
     * Sets reference ASIC window number
     * @param window window number
     */
    void setReferenceWindow(unsigned window) {m_refWindow = window;}

    /**
     * Sets digital waveform, emulating comparator with hysteresis
     * @param upperThr upper threshold level [number of sigma]
     * @param lowerThr lower threshold level [number of sigma]
     * @param minWidth minimal required width of digital pulse [number of samples]
     * @return number of digital pulses
     */
    int setDigital(float upperThr, float lowerThr, unsigned minWidth = 0);

    /**
     * Reconstructs hits using CFD method (setDigital must be already called)
     * @param fraction CFD fraction
     * @return number of hits
     */
    int convertToHits(float fraction);

    /**
     * Returns module ID
     * @return module ID
     */
    int getBarID() const { return m_barID; }

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
     * Returns waveform size (number of samples)
     * @return number of samples
     */
    int getSize() const { return m_data.size(); }

    /**
     * Returns number of bad samples.
     * Bad samples are those with undefined (un-valid) pedestals.
     * @return number of bad samples
     */
    int getNumofBadSamples() const
    {
      int n = 0;
      for (const auto& data : m_data) {
        if (data.err == 0) n++;
      }
      return n;
    }

    /**
     * Returns waveform
     * @return vector of waveform samples
     */
    const std::vector<WFSample>& getWaveform() const {return m_data;}

    /**
     * Returns digital waveform (the result of setDigital)
     * @return digital waveform
     */
    const std::vector<bool>& getDigital() const {return m_digital;}

    /**
     * Returns number of hits (the result of convertToHits)
     * @return number of hits
     */
    int getNumofHits() const {return m_hits.size();}

    /**
     * Returns reconstructed hits (the result of convertToHits)
     * @return reconstructed hits
     */
    const std::vector<Hit>& getHits() const {return m_hits;}

    /**
     * Returns ASIC window numbers
     * @return vector of window numbers
     */
    const std::vector<unsigned short>& getWindowNumbers() const { return m_windows;}

    /**
     * Returns first ASIC window number
     * @return first window number
     */
    unsigned getFirstWindow() const
    {
      if (m_windows.empty()) return 0;
      return m_windows[0];
    }

    /**
     * Returns reference ASIC window number
     * @return reference window number
     */
    unsigned getReferenceWindow() const {return m_refWindow;}


  private:

    int m_barID = 0;                    /**< quartz bar ID */
    int m_pixelID = 0;                  /**< software channel ID */
    unsigned m_channel = 0;             /**< hardware channel number */
    std::vector<WFSample> m_data;       /**< waveform samples */
    std::vector<bool> m_digital;        /**< digital waveform (samples over threshold) */
    std::vector<Hit> m_hits;            /**< reconstructed hits */
    std::vector<unsigned short> m_windows; /**< ASIC window numbers */
    unsigned short m_refWindow = 0; /**< reference ASIC window number */

    ClassDef(TOPWaveform, 3); /**< ClassDef */

  };


} // end namespace Belle2

