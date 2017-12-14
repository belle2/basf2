/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMFPGAFIT_H
#define EKLMFPGAFIT_H

/* Belle2 headers. */
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * FPGA fit status.
     */
    enum FPGAFitStatus {
      c_FPGASuccessfulFit, /**< Successful fit. */
      c_FPGANoSignal,      /**< Signal is too small to do any fitting. */
    };

  }

  /**
   * FPGA fit simulation data.
   */
  class EKLMFPGAFit : public RelationsObject {

  public:

    /**
     * Constructor.
     */
    EKLMFPGAFit();

    /**
     * Destructor.
     */
    ~EKLMFPGAFit();

    /**
     * Get signal start time.
     */
    float getStartTime() const;

    /**
     * Set signal start time.
     * @param[in] startTime Signal start time.
     */
    void setStartTime(float startTime);

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
     * Get maximal amplitude (ADC output).
     */
    int getMaximalAmplitude() const;

    /**
     * Set maximal amplitude.
     */
    void setMaximalAmplitude(int amplitude);

  private:

    /** Start of signal. */
    float m_StartTime;

    /** Amplitude. */
    float m_Amplitude;

    /** Background amplitude. */
    float m_BackgroundAmplitude;

    /** Maximal amplitude. */
    int m_MaximalAmplitude;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMFPGAFit, 1);

  };

}

#endif

