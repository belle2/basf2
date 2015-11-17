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
      float time; /**< time */
      float adc;  /**< adc value */
      float err;  /**< uncertainty of adc value */

      /**
       * Default constructor
       */
      WFSample(): time(0), adc(0), err(0)
      {}

      /**
       * Full constructor
       */
      WFSample(float Time, float ADC, float Err): time(Time), adc(ADC), err(Err)
      {}
    };


    /**
     * Default constructor
     */
    TOPWaveform()
    {}

    /**
     * Constructor with barID, pixelID and channelID - constructs empty waveform
     * @param barID module ID (1-based)
     * @param pixelID software channel ID (1-based)
     * @param channelID hardware channel ID (0-based)
     */
    TOPWaveform(int barID, int pixelID, unsigned channelID):
      m_barID(barID), m_pixelID(pixelID), m_channelID(channelID)
    {}

    /**
     * Append waveform sample to waveform
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
     * Sets digital waveform, emulating comparator with hysteresis
     * @param upperThr upper threshold level [number of sigma]
     * @param lowerThr lower threshold level [number of sigma]
     * @param minWidth minimal required width of digital pulse [number of samples]
     * @return true if at least one digital pulse has width > minWidth
     */
    bool setDigital(float upperThr, float lowerThr, int minWidth = 0);

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
     * Returns waveform size (number of samples)
     * @return number of samples
     */
    int getSize() const {return m_data.size();}

    /**
     * Returns waveform
     * @return vector of waveform samples
     */
    const std::vector<WFSample>& getWaveform() const
    {
      return m_data;
    }

    /**
     * Returns digital waveform
     * @return vector of samples over threshold
     */
    const std::vector<bool>& getDigital() const
    {
      return m_digital;
    }

  private:

    int m_barID = 0;                    /**< quartz bar ID */
    int m_pixelID = 0;                  /**< software channel ID */
    unsigned m_channelID = 0;           /**< hardware channel ID */
    std::vector<WFSample> m_data;       /**< waveform samples */

    std::vector<bool> m_digital;        /**< samples over threshold */

    ClassDef(TOPWaveform, 1); /**< ClassDef */

  };


} // end namespace Belle2

