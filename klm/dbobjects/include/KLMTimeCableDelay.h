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
  class KLMTimeCableDelay : public TObject {

  public:

    /**
     * Constructor.
     */
    KLMTimeCableDelay();

    /**
     * Destructor.
     */
    ~KLMTimeCableDelay();

    /**
     * Set time calibration constant value.
     * @param[in] KLM strip global element number.
     * @param[in] calibration constant value for the strip.
     */
    void setTimeShift(uint16_t, double);

    /**
     * Get time calibration data.
     * @param[in] KLM strip global element number.
     */
    double getTimeShift(uint16_t) const;

    /**
     * Clean time calibration constant.
     */
    void cleanTimeShift();

    /**
     * Get effective light speed of scintillators.
     */
    double getEffLightSpeed() const;

    /**
     * Get effective light speed of RPCs.
     */
    double getEffLightSpeedRPC() const;

    /**
     * Set effective light speed of scintillators.
     * @param[in] lightSpeed Effective light speed.
     */
    void setEffLightSpeed(double lightSpeed);

    /**
     * Set effective light speed of RPCs.
     * @param[in] lightSpeed Effective light speed.
     */
    void setEffLightSpeedRPC(double lightSpeed);

    /**
     * Get amplitude dependence time constant.
     * This item is not supported by the firmwire so far.
     */
    double getAmpTimeConstant() const;
    double getAmpTimeConstantRPC() const;

    /**
     * Set amplitude dependence time constant.
     * This item is not supported by the firmwire so far.
     * @param[in] amplitudeTimeConstant Amplitude dependence time constant.
     */
    void setAmpTimeConstant(double amplitudeTimeConstant);
    void setAmpTimeConstantRPC(double amplitudeTimeConstant);


  private:

    /** Container of time calibration constant value. */
    std::map<uint16_t, double> m_timeShift;

    /** Effective light speed of scintillators. */
    double m_effLightSpeed;

    /** Effective light speed of RPCs. */
    double m_effLightSpeedRPC;

    /** Amplitude dependence time constant of scintillators. */
    double m_ampTimeConstant;

    /** Amplitude dependence time constant of RPCs. */
    double m_ampTimeConstantRPC;

    /** Class version. */
    ClassDef(KLMTimeCableDelay, 1);

  };

}
