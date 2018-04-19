/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Rikuya Okuto                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <map>


#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPCalTimebase.h>
#include <top/dbobjects/TOPCalChannelT0.h>
#include <top/dbobjects/TOPCalModuleT0.h>
#include <top/dbobjects/TOPCalCommonT0.h>


namespace Belle2 {
  /**
   * Crosstalk & chargeshare flag setter
   *
   *
   */
  class TOPXTalkChargeShareSetterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TOPXTalkChargeShareSetterModule();

    /** destructor */
    virtual ~TOPXTalkChargeShareSetterModule() override;

    /** initialize */
    virtual void initialize() override;

    /** beginRun */
    virtual void beginRun() override;

    /** event */
    virtual void event() override;

    /** endRun */
    virtual void endRun() override;

    /** terminate */
    virtual void terminate() override;


  private:

    StoreArray<TOPDigit> m_digits;
    std::string m_outputDigitsName;    /**< name of TOPDigit store array */
    float m_timeCut;                  /**< cut range of hittiming for chargeshare */
    double m_nCrossTalkRingingSamples;  /**< the number of samples to identify the hit as a cross talk hit when there is another cross talk hit in this number of samples before */
    int m_preValleyDepthLow = 20; /**< loose threshold for depth of pre valley [ADC counts], for corss talk identification */
    int m_preValleyDepthHigh =
      50; /**< tight threshold for depth of pre valley [ADC counts], identified as cross talk with loose threshold for the second peak amplitude */
    int m_2ndPeakAmplitudeLow =
      30; /**< loose threshold for amplitude of the second peak [ADC counts] for cross talk identification. Defined as ADC count difference between the valley just after the main peak and the second peak, Used when the \"preValleyDepthHigh\" was satisfied */
    double m_2ndPeakAmplitudeRatioHigh =
      0.2; /**< tight threshold for amplitude ratio of the second peak to the main peak height [ADC counts] */
    int m_nSampleBefore =
      5; /**< the number of samples by which the pre-valley should exist from the CFD timing, used for cross talk identification */
    int m_nSampleAfter =
      10; /**< the number of samples by which the second peak should exist from the CFD timing, used for cross talk identification */
  };


};
}
