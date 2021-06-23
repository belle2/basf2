/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>

#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/HardwareClockSettings.h>


namespace Belle2 {
  /**
   * Storage element for information from the Trigger Timing Distribution (TTD)
   *
   * Data is required to study injections and injection backgrounds,
   * as well as for the analysis and simulation of PXD Gated Mode
   **/
  class EventLevelTriggerTimeInfo : public TObject {

  public:
    /** Constructor
     *  @param isValid  The information stored in this object is valid
     *  @param isHER    Injection in HER/LER
     *  @param revo2    Lowest bit of the revolution counter
     *  @param timeSinceLastInjection   Time since the last injection in clock ticks (127MHz=RF/4 clock)
     *  @param timeSincePrevTrigger     Time since the previous trigger in clock ticks (127MHz=RF/4 clock)
     *  @param bunchNumber  Number of the triggered bunch (ranging from 0-1279)
     **/
    explicit EventLevelTriggerTimeInfo(bool isValid = false, bool isHER = false, bool revo2 = false,
                                       unsigned int timeSinceLastInjection = c_flagNoInjection,
                                       unsigned int timeSincePrevTrigger = 0,
                                       unsigned int bunchNumber = 0) :
      m_isValid(isValid), m_isHER(isHER), m_revo2(revo2), m_timeSinceLastInjection(timeSinceLastInjection),
      m_timeSincePrevTrigger(timeSincePrevTrigger), m_bunchNumber(bunchNumber) {}
    /// Destructor
    ~EventLevelTriggerTimeInfo() {}

    // Simple Getters
    /// get if stored information is valid
    bool isValid() {return m_isValid;}
    /// get if injection in HER/LER
    bool isHER() {return m_isHER;}
    /// get lowest bit of revolution counter
    bool isRevo2() {return m_revo2;}
    /// get time since the last injection (i.e. the injection-pre-kick signal) in clock ticks (FTSW clock)
    unsigned int getTimeSinceLastInjection() {return m_timeSinceLastInjection;}
    /// get time since the previous trigger in clock ticks (FTSW clock)
    unsigned int getTimeSincePrevTrigger() {return m_timeSincePrevTrigger;}
    /// get number of triggered bunch
    unsigned int getBunchNumber() {return m_bunchNumber;}
    /// get if an injection happened recently (and the corresponding stored data is actually valid)
    bool hasInjection() {return m_timeSinceLastInjection == c_flagNoInjection;}

    // Simple Setters
    /// set that stored information is valid
    void setValid() {m_isValid = true;}
    /// set that stored information is valid
    void setInvalid() {m_isValid = false;}
    /// set if injection in HER/LER
    void setIsHER(bool isHER) {m_isHER = isHER;}
    /// set lowest bit of revolution counter
    void setRevo2(bool revo2) {m_revo2 = revo2;}
    /// set time since the last injection (i.e. the injection-pre-kick signal) in clock ticks (FTSW clock)
    void setTimeSinceLastInjection(unsigned int timeSinceLastInjection) {m_timeSinceLastInjection = timeSinceLastInjection;}
    /// set time since the previous trigger in clock ticks (FTSW clock)
    void setTimeSincePrevTrigger(unsigned int timeSincePrevTrigger) {m_timeSincePrevTrigger = timeSincePrevTrigger;}
    /// set number of triggered bunch
    void setBunchNumber(unsigned int bunchNumber) {m_bunchNumber = bunchNumber;}
    /// set that no injection happened recently (and the corresponding stored data is actually invalid)
    void setNoInjection() {m_timeSinceLastInjection = c_flagNoInjection;}

    // Additional Functions (not inline)
    /// get time since the last injection (i.e. the injection-pre-kick signal) in microseconds
    double getTimeSinceLastInjectionInMicroSeconds();
    /// get time since the previous trigger in microseconds
    double getTimeSincePrevTriggerInMicroSeconds();

  private:
    /**
    * Data stored in this TTD info object is actually valid
    **/
    bool m_isValid;
    /**
    * Injection is in HER/LER
    **/
    bool m_isHER;
    /**
    * Lowest bit of revolution counter
    * (PXD needs ~2 revolutions to readout one frame)
    **/
    bool m_revo2;
    /**
    * Time since the last injection in clock ticks (127MHz=RF/4 clock)
    * Note: A value of 0x7FFFFFFF (see `c_flagNoInjection`) means no injection took place recently
    *       (value is defined by the RawFTSW)
    **/
    unsigned int m_timeSinceLastInjection;
    /**
    * Time since the previous trigger in clock ticks (127MHz=RF/4 clock)
    * Note: This is actually the time since the injection-pre-kick signal was received
    *       so there is some offset (different for HER/LER) that will be handled by the analysis variable
    **/
    unsigned int m_timeSincePrevTrigger;
    /**
    * Number of triggered bunch, ranging from 0-1279
    * Note: There are a maximum of 5120 buckets, which could each carry one bunch of e+/e-,
    *       but we only have 1280 clock ticks (=5120/4) to identify the bunches
    **/
    unsigned int m_bunchNumber;

    /// This number is defined by the FTSW to indicate no injection happened recently
    static const unsigned int c_flagNoInjection = 0x7FFFFFFF; //! tells ROOT not to write it to file (transient)
    /// The clock, to translate clock ticks to microseconds
    DBObjPtr<HardwareClockSettings> m_clockSettings; //! tells ROOT not to write it to file (transient)

    ClassDef(EventLevelTriggerTimeInfo, 1) ///< Storage element for TTD information
  };
}
