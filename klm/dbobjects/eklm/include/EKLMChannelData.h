/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * EKLM channel data.
   */
  class EKLMChannelData : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMChannelData()
    {
    }

    /**
     * Destructor.
     */
    ~EKLMChannelData()
    {
    }

    /**
     * Get pedestal.
     */
    float getPedestal() const
    {
      return m_Pedestal;
    }

    /**
     * Set pedestal.
     */
    void setPedestal(float pedestal)
    {
      m_Pedestal = pedestal;
    }

    /**
     * Get photoelectron amplitude.
     */
    float getPhotoelectronAmplitude() const
    {
      return m_PhotoelectronAmplitude;
    }

    /**
     * Set photoelectron amplitude.
     */
    void setPhotoelectronAmplitude(float photoelectronAmplitude)
    {
      m_PhotoelectronAmplitude = photoelectronAmplitude;
    }

    /**
     * Get threshold.
     */
    int getThreshold() const
    {
      return m_Threshold;
    }

    /**
     * Set threshold.
     */
    void setThreshold(int threshold)
    {
      m_Threshold = threshold;
    }

    /**
     * Get voltage.
     */
    float getVoltage() const
    {
      return m_Voltage;
    }

    /**
     * Set voltage.
     */
    void setVoltage(float voltage)
    {
      m_Voltage = voltage;
    }

    /**
     * Get adjustment voltage.
     */
    int getAdjustmentVoltage() const
    {
      return m_AdjustmentVoltage;
    }

    /**
     * Set adjustment voltage.
     */
    void setAdjustmentVoltage(int adjustmentVoltage)
    {
      m_AdjustmentVoltage = adjustmentVoltage;
    }

    /**
     * Get lookback time (unit is 32 TDC counts).
     */
    int getLookbackTime() const
    {
      return m_LookbackTime;
    }

    /**
     * Set lookback time (unit is 32 TDC counts).
     */
    void setLookbackTime(int lookbackTime)
    {
      m_LookbackTime = lookbackTime;
    }

    /**
     * Get lookback window width (unit is 32 TDC counts).
     */
    int getLookbackWindowWidth() const
    {
      return m_LookbackWindowWidth;
    }

    /**
     * Set lookback window width (unit is 32 TDC counts).
     */
    void setLookbackWindowWidth(int lookbackWindowWidth)
    {
      m_LookbackWindowWidth = lookbackWindowWidth;
    }

  private:

    /** True if channel is active. */
    bool m_Active = false;

    /** Pedestal. */
    float m_Pedestal = 0;

    /** Photoelectron amplitude. */
    float m_PhotoelectronAmplitude = 0;

    /** Threshold (maximal EKLMDigit.m_Charge). */
    int m_Threshold = 0;

    /** Voltage. */
    float m_Voltage = 0;

    /** Adjustment voltage. */
    int m_AdjustmentVoltage = 0;

    /** Lookback time (unit is 32 TDC counts). */
    int m_LookbackTime = 0;

    /** Lookback window width (unit is 32 TDC counts). */
    int m_LookbackWindowWidth = 0;

    /** Class version. */
    ClassDef(Belle2::EKLMChannelData, 4);

  };

}
