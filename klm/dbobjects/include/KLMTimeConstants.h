/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jincheng Mei                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * Class to store KLM constants related to time.
   */
  class KLMTimeConstants : public TObject {

  public:

    /**
     * Channel type.
     */
    enum ChannelType {

      /** EKLM scintillator. */
      c_EKLM = 1,

      /** BKLM scintillator. */
      c_BKLM = 2,

      /** RPC. */
      c_RPC = 3,

    };

    /**
     * Constructor.
     */
    KLMTimeConstants();

    /**
     * Destructor.
     */
    ~KLMTimeConstants();

    /**
     * Get effective light speed of scintillators.
     * @param[in] cType Flag of channel type.
     */
    double getEffLightSpeed(int cType) const;

    /**
     * Set effective light speed of scintillators.
     * @param[in] lightSpeed Effective light speed.
     * @param[in] cType Flag of channel type.
     */
    void setEffLightSpeed(double lightSpeed, int cType);

    /**
     * Get amplitude dependence time constant.
     * This item is not supported by the firmwire so far.
     * @param[in] cType Flag of channel type.
     */
    double getAmpTimeConstant(int cType) const;

    /**
     * Set amplitude dependence time constant.
     * This item is not supported by the firmwire so far.
     * @param[in] amplitudeTimeConstant Amplitude dependence time constant.
     * @param[in] cType Flag of channel type.
     */
    void setAmpTimeConstant(double amplitudeTimeConstant, int cType);

  private:

    /** Effective light speed of scintillators for EKLM. */
    double m_effLightSpeed_end;

    /** Effective light speed of scintillators for BKLM. */
    double m_effLightSpeed;

    /** Effective light speed of RPCs. */
    double m_effLightSpeedRPC;

    /** Amplitude dependence time constant of scintillators for EKLM. */
    double m_ampTimeConstant_end;

    /** Amplitude dependence time constant of scintillators for BKLM. */
    double m_ampTimeConstant;

    /** Amplitude dependence time constant of RPCs. */
    double m_ampTimeConstantRPC;

    /** Class version. */
    ClassDef(KLMTimeConstants, 1);

  };

}
