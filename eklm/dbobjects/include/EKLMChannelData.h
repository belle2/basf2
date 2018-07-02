/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMCHANNELDATA_H
#define EKLMCHANNELDATA_H

/* External headers. */
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
    EKLMChannelData();

    /**
     * Destructor.
     */
    ~EKLMChannelData();

    /**
     * Get if channel is active or not.
     */
    bool getActive() const;

    /**
     * Set if channel is active or not.
     */
    void setActive(bool active);

    /**
     * Get pedestal.
     */
    float getPedestal() const;

    /**
     * Set pedestal.
     */
    void setPedestal(float pedestal);

    /**
     * Get photoelectron amplitude.
     */
    float getPhotoelectronAmplitude() const;

    /**
     * Set photoelectron amplitude.
     */
    void setPhotoelectronAmplitude(float photoelectronAmplitude);

    /**
     * Get threshold.
     */
    int getThreshold() const;

    /**
     * Set threshold.
     */
    void setThreshold(int threshold);

    /**
     * Get voltage.
     */
    float getVoltage() const;

    /**
     * Set voltage.
     */
    void setVoltage(float voltage);

    /**
     * Get adjustment voltage.
     */
    int getAdjustmentVoltage() const;

    /**
     * Set adjustment voltage.
     */
    void setAdjustmentVoltage(int adjustmentVoltage);

    /**
     * Get lookback time (unit is 32 TDC counts).
     */
    int getLookbackTime() const;

    /**
     * Set lookback time (unit is 32 TDC counts).
     */
    void setLookbackTime(int lookbackTime);

    /**
     * Get lookback window width (unit is 32 TDC counts).
     */
    int getLookbackWindowWidth() const;

    /**
     * Set lookback window width (unit is 32 TDC counts).
     */
    void setLookbackWindowWidth(int lookbackWindowWidth);

  private:

    /** True if channel is active. */
    bool m_Active;

    /** Pedestal. */
    float m_Pedestal;

    /** Photoelectron amplitude. */
    float m_PhotoelectronAmplitude;

    /** Threshold (maximal EKLMDigit.m_Charge). */
    int m_Threshold;

    /** Voltage. */
    int m_Voltage;

    /** Adjustment voltage. */
    int m_AdjustmentVoltage;

    /** Lookback time (unit is 32 TDC counts). */
    int m_LookbackTime;

    /** Lookback window width (unit is 32 TDC counts). */
    int m_LookbackWindowWidth;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMChannelData, 4);

  };

}

#endif

