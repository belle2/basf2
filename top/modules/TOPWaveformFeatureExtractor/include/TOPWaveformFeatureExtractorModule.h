/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <top/dataobjects/TOPRawWaveform.h>
#include <top/dataobjects/TOPWaveformSegment.h>

#include <top/FeatureExtractionData.h>

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
    int getIntegral(TOPWaveformSegment* waveformSegment, int sampleRise, int samplePeak, int sampleFall);

    std::vector<TOP::FeatureExtractionData> featureExtraction(TOPWaveformSegment* waveformSegment, int threshold, int hysteresis,
                                                              int thresholdCount);


    std::string m_inputRawDigitsName;  /**< name of TOPRawDigit store array */
    int m_threshold = 0; /**< pulse height threshold [ADC counts] */
    int m_hysteresis = 0; /**< pulse height threshold hysteresis [ADC counts] */
    int m_thresholdCount = 0; /**< minimal number of samples above threshold */
    bool m_setIntegral = false; /**< calculate and set integral for FW-extracted hits */

  };

} // Belle2 namespace

