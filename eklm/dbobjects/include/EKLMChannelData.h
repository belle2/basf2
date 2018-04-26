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
     * Get threshold.
     */
    int getThreshold() const;

    /**
     * Set threshold.
     */
    void setThreshold(int threshold);

    /**
     * Get adjustment voltage.
     */
    int getAdjustmentVoltage() const;

    /**
     * Set adjustment voltage.
     */
    void setAdjustmentVoltage(int adjustmentVoltage);

    /**
     * Get lookback window.
     */
    int getLookbackWindow() const;

    /**
     * Set lookback window.
     */
    void setLookbackWindow(int lookbackWindow);

  private:

    /** True if channel is active. */
    bool m_Active;

    /** Threshold (maximal EKLMDigit.m_Charge). */
    int m_Threshold;

    /** Adjustment voltage. */
    int m_AdjustmentVoltage;

    /** Lookback window. */
    int m_LookbackWindow;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMChannelData, 2);

  };

}

#endif

