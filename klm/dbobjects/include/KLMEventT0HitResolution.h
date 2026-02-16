/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)
 * Author: The Belle II Collaboration
 *
 * See git log for contributors and copyright holders.
 * This file is licensed under LGPL-3.0, see LICENSE.md.
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * Class to store per-hit time resolution (sigma)
   * for KLM EventT0, separated by detector type.
   *
   * All sigmas are in nanoseconds.
   */
  class KLMEventT0HitResolution : public TObject {

  public:

    /**
     * Detector category for per-hit resolution.
     */
    enum Category {

      /** EKLM scintillator. */
      c_EKLMScint = 1,

      /** BKLM scintillator. */
      c_BKLMScint = 2,

      /** BKLM RPC (combined phi/z). */
      c_RPC       = 3,

      /** BKLM RPC Phi-readout. */
      c_RPCPhi    = 4,

      /** BKLM RPC Z-readout. */
      c_RPCZ      = 5,
    };

    /**
     * Constructor.
     */
    KLMEventT0HitResolution();

    /**
     * Destructor.
     */
    ~KLMEventT0HitResolution();


    /**
     * Set per-hit sigma for EKLM scintillator.
     * @param[in] sigma     Resolution [ns].
     * @param[in] sigmaErr  Uncertainty on resolution [ns].
     */
    void setSigmaEKLMScint(float sigma, float sigmaErr);

    /**
     * Set per-hit sigma for BKLM scintillator.
     * @param[in] sigma     Resolution [ns].
     * @param[in] sigmaErr  Uncertainty on resolution [ns].
     */
    void setSigmaBKLMScint(float sigma, float sigmaErr);

    /**
     * Set per-hit sigma for BKLM RPC (combined).
     * @param[in] sigma     Resolution [ns].
     * @param[in] sigmaErr  Uncertainty on resolution [ns].
     */
    void setSigmaRPC(float sigma, float sigmaErr);

    /**
     * Set per-hit sigma for BKLM RPC Phi-readout.
     * @param[in] sigma     Resolution [ns].
     * @param[in] sigmaErr  Uncertainty on resolution [ns].
     */
    void setSigmaRPCPhi(float sigma, float sigmaErr);

    /**
     * Set per-hit sigma for BKLM RPC Z-readout.
     * @param[in] sigma     Resolution [ns].
     * @param[in] sigmaErr  Uncertainty on resolution [ns].
     */
    void setSigmaRPCZ(float sigma, float sigmaErr);

    /**
     * Generic setter by category.
     * @param[in] sigma     Resolution [ns].
     * @param[in] sigmaErr  Uncertainty on resolution [ns].
     * @param[in] category  Detector category.
     */
    void setSigma(float sigma, float sigmaErr, int category);


    /** Get per-hit sigma for EKLM scintillator [ns]. */
    float getSigmaEKLMScint() const;

    /** Get per-hit sigma uncertainty for EKLM scintillator [ns]. */
    float getSigmaEKLMScintErr() const;

    /** Get per-hit sigma for BKLM scintillator [ns]. */
    float getSigmaBKLMScint() const;

    /** Get per-hit sigma uncertainty for BKLM scintillator [ns]. */
    float getSigmaBKLMScintErr() const;

    /** Get per-hit sigma for BKLM RPC (combined) [ns]. */
    float getSigmaRPC() const;

    /** Get per-hit sigma uncertainty for BKLM RPC (combined) [ns]. */
    float getSigmaRPCErr() const;

    /** Get per-hit sigma for BKLM RPC Phi-readout [ns]. */
    float getSigmaRPCPhi() const;

    /** Get per-hit sigma uncertainty for BKLM RPC Phi-readout [ns]. */
    float getSigmaRPCPhiErr() const;

    /** Get per-hit sigma for BKLM RPC Z-readout [ns]. */
    float getSigmaRPCZ() const;

    /** Get per-hit sigma uncertainty for BKLM RPC Z-readout [ns]. */
    float getSigmaRPCZErr() const;


    /**
     * Get per-hit sigma for a given category.
     * @param[in] category Detector category.
     * @return Resolution [ns].
     */
    float getSigma(int category) const;

    /**
     * Get per-hit sigma uncertainty for a given category.
     * @param[in] category Detector category.
     * @return Uncertainty on resolution [ns].
     */
    float getSigmaErr(int category) const;

  private:

    /** Per-hit sigma for EKLM scintillator [ns]. */
    float m_SigmaEKLMScint = 0.0;

    /** Uncertainty on per-hit sigma for EKLM scintillator [ns]. */
    float m_SigmaEKLMScintErr = 0.0;

    /** Per-hit sigma for BKLM scintillator [ns]. */
    float m_SigmaBKLMScint = 0.0;

    /** Uncertainty on per-hit sigma for BKLM scintillator [ns]. */
    float m_SigmaBKLMScintErr = 0.0;

    /** Per-hit sigma for BKLM RPC (combined) [ns]. */
    float m_SigmaRPC = 0.0;

    /** Uncertainty on per-hit sigma for BKLM RPC (combined) [ns]. */
    float m_SigmaRPCErr = 0.0;

    /** Per-hit sigma for BKLM RPC Phi-readout [ns]. */
    float m_SigmaRPCPhi = 0.0;

    /** Uncertainty on per-hit sigma for BKLM RPC Phi-readout [ns]. */
    float m_SigmaRPCPhiErr = 0.0;

    /** Per-hit sigma for BKLM RPC Z-readout [ns]. */
    float m_SigmaRPCZ = 0.0;

    /** Uncertainty on per-hit sigma for BKLM RPC Z-readout [ns]. */
    float m_SigmaRPCZErr = 0.0;

    /** Class version. */
    ClassDef(KLMEventT0HitResolution, 1);

  };

} // namespace Belle2