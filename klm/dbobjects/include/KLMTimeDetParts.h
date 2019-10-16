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

/* C++ headers. */
#include <cstdint>
#include <map>

/* External headers. */
#include <TObject.h>


namespace Belle2 {

  /**
   * Class to store BKLM delay time coused by cable in the database.
   */
  class KLMTimeDetParts : public TObject {

  public:

    /**
     * Constructor.
     */
    KLMTimeDetParts();

    /**
     * Destructor.
     */
    ~KLMTimeDetParts();

    /**
     * Get effective light speed of scintillators.
     * @param[in] isRPC get value for RPC(true) or Scintillator(false).
     */
    double getEffLightSpeed(bool isRPC) const;

    /**
     * Set effective light speed of scintillators.
     * @param[in] lightSpeed Effective light speed.
     * @param[in] isRPC set value for RPC(true) or Scintillator(false).
     */
    void setEffLightSpeed(double lightSpeed, bool isRPC);

    /**
     * Get amplitude dependence time constant.
     * This item is not supported by the firmwire so far.
     * @param[in] isRPC get value for RPC(true) or Scintillator(false).
     */
    double getAmpTimeConstant(bool isRPC) const;

    /**
     * Set amplitude dependence time constant.
     * This item is not supported by the firmwire so far.
     * @param[in] amplitudeTimeConstant Amplitude dependence time constant.
     * @param[in] isRPC set value for RPC(true) or Scintillator(false).
     */
    void setAmpTimeConstant(double amplitudeTimeConstant, bool isRPC);


  private:
    /** Effective light speed of scintillators. */
    double m_effLightSpeed;

    /** Effective light speed of RPCs. */
    double m_effLightSpeedRPC;

    /** Amplitude dependence time constant of scintillators. */
    double m_ampTimeConstant;

    /** Amplitude dependence time constant of RPCs. */
    double m_ampTimeConstantRPC;

    /** Class version. */
    ClassDef(KLMTimeDetParts, 1);

  };

}
