/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  namespace KLM {

    /**
     * Scintillator firmware fit status.
     */
    enum ScintillatorFirmwareFitStatus {

      /** Successful fit. */
      c_ScintillatorFirmwareSuccessfulFit,

      /** Signal is too small to do any fitting. */
      c_ScintillatorFirmwareNoSignal,

    };

  }

  /**
   * FPGA fit simulation data.
   */
  class KLMScintillatorFirmwareFitResult : public RelationsObject {

  public:

    /**
     * Constructor.
     */
    KLMScintillatorFirmwareFitResult();

    /**
     * Destructor.
     */
    ~KLMScintillatorFirmwareFitResult();

    /**
     * Get signal start time (in TDC counts).
     */
    int getStartTime() const;

    /**
     * Set signal start time (in TDC counts).
     * @param[in] startTime Signal start time.
     */
    void setStartTime(int startTime);

    /**
     * Get amplitude.
     */
    float getAmplitude() const;

    /**
     * Set amplitude.
     * @param[in] amplitude Amplitide.
     */
    void setAmplitude(float amplitude);

    /**
     * Get background amplitude.
     */
    float getBackgroundAmplitude() const;

    /**
     * Set background amplitude.
     * @param[in] amplitude Background amplitide.
     */
    void setBackgroundAmplitude(float amplitude);

    /**
     * Get minimal amplitude (ADC output).
     */
    int getMinimalAmplitude() const;

    /**
     * Set minimal amplitude.
     */
    void setMinimalAmplitude(int amplitude);

  private:

    /** Start of signal. */
    int m_StartTime;

    /** Amplitude. */
    float m_Amplitude;

    /** Background amplitude. */
    float m_BackgroundAmplitude;

    /** Minimal amplitude. */
    int m_MinimalAmplitude;

    /** Class version. */
    ClassDef(Belle2::KLMScintillatorFirmwareFitResult, 1);

  };

}
