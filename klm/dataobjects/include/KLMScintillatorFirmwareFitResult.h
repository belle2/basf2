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

/* Belle 2 headers. */
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
    KLMScintillatorFirmwareFitResult()
    {
    }

    /**
     * Destructor.
     */
    ~KLMScintillatorFirmwareFitResult()
    {
    }

    /**
     * Get signal start time (in TDC counts).
     */
    int getStartTime() const
    {
      return m_StartTime;
    }

    /**
     * Set signal start time (in TDC counts).
     * @param[in] startTime Signal start time.
     */
    void setStartTime(int startTime)
    {
      m_StartTime = startTime;
    }

    /**
     * Get amplitude.
     */
    float getAmplitude() const
    {
      return m_Amplitude;
    }

    /**
     * Set amplitude.
     * @param[in] amplitude Amplitide.
     */
    void setAmplitude(float amplitude)
    {
      m_Amplitude = amplitude;
    }

    /**
     * Get background amplitude.
     */
    float getBackgroundAmplitude() const
    {
      return m_BackgroundAmplitude;
    }

    /**
     * Set background amplitude.
     * @param[in] amplitude Background amplitide.
     */
    void setBackgroundAmplitude(float amplitude)
    {
      m_BackgroundAmplitude = amplitude;
    }

    /**
     * Get minimal amplitude (ADC output).
     */
    int getMinimalAmplitude() const
    {
      return m_MinimalAmplitude;
    }

    /**
     * Set minimal amplitude.
     */
    void setMinimalAmplitude(int amplitude)
    {
      m_MinimalAmplitude = amplitude;
    }

  private:

    /** Start of signal. */
    int m_StartTime = -1;

    /** Amplitude. */
    float m_Amplitude = -1;

    /** Background amplitude. */
    float m_BackgroundAmplitude = -1;

    /** Minimal amplitude. */
    int m_MinimalAmplitude = -1;

    /** Class version. */
    ClassDef(Belle2::KLMScintillatorFirmwareFitResult, 1);

  };

}
