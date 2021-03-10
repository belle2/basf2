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

      /** RPC phi plane. */
      c_RPCPhi = 3,

      /** RPC z plane. */
      c_RPCZ = 4,

    };

    /**
     * Constructor.
     */
    KLMTimeConstants()
    {
    }

    /**
     * Destructor.
     */
    ~KLMTimeConstants()
    {
    }

    /**
     * Get effective light speed of scintillators.
     * @param[in] cType Flag of channel type.
     */
    float getDelay(int cType) const;

    /**
     * Set effective light speed of scintillators.
     * @param[in] lightSpeed Effective light speed.
     * @param[in] cType      Flag of channel type.
     */
    void setDelay(float delay, int cType);

    /**
     * Get amplitude dependence time constant.
     * This item is not supported by the firmwire so far.
     * @param[in] cType Flag of channel type.
     */
    float getAmpTimeConstant(int cType) const;

    /**
     * Set amplitude dependence time constant.
     * This item is not supported by the firmwire so far.
     * @param[in] amplitudeTimeConstant Amplitude dependence time constant.
     * @param[in] cType                 Flag of channel type.
     */
    void setAmpTimeConstant(float amplitudeTimeConstant, int cType);

  private:

    /** Delay (ns / cm) for EKLM scintillators. */
    float m_DelayEKLMScintillators = 0.0;

    /** Delay (ns / cm) for BKLM scintillators. */
    float m_DelayBKLMScintillators = 0.0;

    /** Delay (ns / cm) for RPC phi plane. */
    float m_DelayRPCPhi = 0.0;

    /** Delay (ns / cm) for RPC Z plane. */
    float m_DelayRPCZ = 0.0;

    /** Amplitude dependence time constant of scintillators for EKLM. */
    float m_ampTimeConstant_end = 0.0;

    /** Amplitude dependence time constant of scintillators for BKLM. */
    float m_ampTimeConstant = 0.0;

    /** Amplitude dependence time constant of RPCs. */
    float m_ampTimeConstantRPC = 0.0;

    /** Class version. */
    ClassDef(KLMTimeConstants, 1);

  };

}
