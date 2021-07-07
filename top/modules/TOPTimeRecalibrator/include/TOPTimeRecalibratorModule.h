/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRecBunch.h>

#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPCalTimebase.h>
#include <top/dbobjects/TOPCalChannelT0.h>
#include <top/dbobjects/TOPCalAsicShift.h>
#include <top/dbobjects/TOPCalModuleT0.h>
#include <top/dbobjects/TOPCalCommonT0.h>
#include <top/dbobjects/TOPFrontEndSetting.h>
#include <top/dbobjects/TOPCalTimeWalk.h>

namespace Belle2 {

  /**
   * Utility module for re-calibrating time of TOPDigits
   * pulseWidth and timeError are not changed although they may depend no calibration!
   */
  class TOPTimeRecalibratorModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPTimeRecalibratorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    // steering parameters
    bool m_useSampleTimeCalibration;   /**< if true, use sample time calibration */
    bool m_useAsicShiftCalibration;    /**< if true, use asic shifts calibration */
    bool m_useChannelT0Calibration;    /**< if true, use channel T0 calibration */
    bool m_useModuleT0Calibration;     /**< if true, use module T0 calibration */
    bool m_useCommonT0Calibration;     /**< if true, use common T0 calibration */
    bool m_useTimeWalkCalibration;     /**< if true, use time-walk calibration */
    bool m_subtractBunchTime;          /**< if true, subtract reconstructed bunch time */

    // front-end settings (lookback, storage depths etc)
    DBObjPtr<TOPFrontEndSetting> m_feSetting;   /**< front-end settings */

    // time calibration
    DBObjPtr<TOPCalTimebase> m_timebase;   /**< sample time calibration constants */
    DBObjPtr<TOPCalChannelT0> m_channelT0; /**< channel T0 calibration constants */
    DBObjPtr<TOPCalAsicShift> m_asicShift; /**< ASIC shifts calibration constants */
    DBObjPtr<TOPCalModuleT0> m_moduleT0;   /**< module T0 calibration constants */
    DBObjPtr<TOPCalCommonT0> m_commonT0;   /**< common T0 calibration constants */
    OptionalDBObjPtr<TOPCalTimeWalk> m_timeWalk;   /**< time-walk calibration constants */
    TOPSampleTimes m_sampleTimes; /**< equidistant in case no calibration required */

    // collections
    StoreArray<TOPDigit> m_digits;       /**< collection of digits */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */

    // other
    double m_syncTimeBase = 0; /**< SSTin period */

  };

} // Belle2 namespace

