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

#include <framework/core/Module.h>
#include <string>
#include <map>

#include <framework/database/DBArray.h>
#include <top/dbobjects/TOPASICChannel.h>
#include <top/dataobjects/TOPRawWaveform.h>
#include <top/dataobjects/TOPWaveform.h>
#include <top/geometry/TOPGeometryPar.h>

namespace Belle2 {

  /**
   * Merging raw waveforms, pedestal subtraction and gain correction.
   * Conversion to hits.
   */
  class TOPWFMergerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPWFMergerModule();

    /**
     * Destructor
     */
    virtual ~TOPWFMergerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    /**
     * Return a key used in m_map
     * @param barID module ID
     * @param channelID hardware channel ID
     * @return a key
     */
    unsigned getKey(int barID, unsigned channelID) const
    {
      return channelID + (barID << 16);
    }

    /**
     * Return the mapped value
     * @param barID module ID
     * @param channelID hardware channel ID
     * @return mapped value or NULL
     */
    const TOPASICChannel* getConstants(int barID, unsigned channelID) const
    {
      unsigned key = getKey(barID, channelID);
      std::map<unsigned, const TOPASICChannel*>::const_iterator it = m_map.find(key);
      if (it == m_map.end()) return 0;
      return it->second;
    }

    /**
     * Appends raw waveform to waveform
     * @param rawWaveform raw waveform
     * @param calibration ASIC channel calibration constants
     * @param waveform waveform to append to
     * @return true on success
     */
    bool appendRawWavefrom(const TOPRawWaveform* rawWaveform,
                           const TOPASICChannel* calibration,
                           TOPWaveform* waveform);

    /**
     * Returns remaining pedestal offset after pedestal subtraction.
     * @param rawWaveform raw waveform
     * @param pedestals pedestals
     * @return offset
     */
    float getPedestalOffset(const TOPRawWaveform* rawWaveform,
                            const TOPASICPedestals* pedestals);

    double m_outlayerCut; /**< outlayer cut in number of sigma for pedestal offset */
    double m_threshold;   /**< threshold value in number of sigma */
    double m_hysteresis;  /**< hysteresis value in number of sigma */
    int m_minWidth;    /**< minimal width of a digital pulse in number of samlpes */
    double m_fraction;    /**< constant fraction discrimination: fraction */
    bool m_useFTSW;  /**< if true add FTSW time to hit times when making TOPDigits */

    DBArray<TOPASICChannel> m_asicChannels; /**< ASIC calibration constants */
    std::map<unsigned, const TOPASICChannel*> m_map; /**< map of (bar,channel) and ptr */

    TOP::TOPGeometryPar* m_topgp = TOP::TOPGeometryPar::Instance();  /**< geometry */

  };

} // Belle2 namespace

