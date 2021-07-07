/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
     * @param[in] delay Delay.
     * @param[in] cType Flag of channel type.
     */
    void setDelay(float delay, int cType);

  private:

    /** Delay (ns / cm) for EKLM scintillators. */
    float m_DelayEKLMScintillators = 0.0;

    /** Delay (ns / cm) for BKLM scintillators. */
    float m_DelayBKLMScintillators = 0.0;

    /** Delay (ns / cm) for RPC phi plane. */
    float m_DelayRPCPhi = 0.0;

    /** Delay (ns / cm) for RPC Z plane. */
    float m_DelayRPCZ = 0.0;

    /** Class version. */
    ClassDef(KLMTimeConstants, 1);

  };

}
