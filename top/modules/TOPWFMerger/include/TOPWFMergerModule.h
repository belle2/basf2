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
#include <top/dbobjects/TOPSampleTime.h>
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
     * Return a key used in m_pedestalMap
     * @param moduleID module ID
     * @param channel hardware channel number
     * @return a key
     */
    unsigned getKey(int moduleID, unsigned channel) const
    {
      return channel + (moduleID << 16);
    }

    /**
     * Return module ID from key
     * @param key a key number
     * @return module ID
     */
    int getModuleID(unsigned key) const {return (key >> 16);}

    /**
     * Return channel number from key
     * @param key a key number
     * @return channel number
     */
    int getChannel(unsigned key) const {return (key & 0xFFFF);}

    /**
     * Appends raw waveform to waveform
     * @param rawWaveform raw waveform
     * @param calibration ASIC channel calibration constants (pedestals, gains)
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
    int m_activeWindows; /**< number of active ASIC windows */

    DBArray<TOPASICChannel> m_asicChannels; /**< pedestal calibration constants */
    /** map of (module, channel) and pedestals */
    std::map<unsigned, const TOPASICChannel*> m_pedestalMap;

    DBArray<TOPSampleTime> m_sampleTimes;  /**< sample time calibration */
    /** map of (module, channel) and sample time calibration */
    std::map<unsigned, const TOPSampleTime*> m_sampleTimeMap;
    TOPSampleTime* m_sampleTime = 0; /**< default in case no calibration available */

    TOP::TOPGeometryPar* m_topgp = TOP::TOPGeometryPar::Instance();  /**< geometry */

    unsigned m_falseWindows = 0; /**< false ASIC window count */

  };

} // Belle2 namespace

