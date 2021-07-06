/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /**
   * Waveform feature extractor: module adds rawDigits that are found in waveforms
   * by feature extraction but are not already present in RawDigits.
   * The module will be obsolete when multi-hit firmware becomes available.
   */
  class TOPWaveformFeatureExtractorModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPWaveformFeatureExtractorModule();

    /**
     * Destructor
     */
    virtual ~TOPWaveformFeatureExtractorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

  private:

    std::string m_inputRawDigitsName;  /**< name of TOPRawDigit store array */
    int m_threshold = 0; /**< pulse height threshold [ADC counts] */
    int m_hysteresis = 0; /**< pulse height threshold hysteresis [ADC counts] */
    int m_thresholdCount = 0; /**< minimal number of samples above threshold */
    bool m_setIntegral = false; /**< calculate and set integral for FW-extracted hits */

  };

} // Belle2 namespace

