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
   * Storage element for information from the Trigger Timing Distribution (TTD).
   **/
  class TTDEventInfo : public TObject {

  public:
    /// Constructor
    explicit TTDEventInfo(bool isValid = false, bool isHER = false, bool revo2 = false,
                          unsigned int timeSinceLastInjection = 0x7FFFFFF,
                          unsigned int timeSincePrevTrigger = 0,
                          unsigned int bunchNumber = 0) :
      m_isValid(isValid), m_isHER(isHER), m_revo2(revo2), m_timeSinceLastInjection(timeSinceLastInjection),
      m_timeSincePrevTrigger(timeSincePrevTrigger), m_bunchNumber(bunchNumber) {}
    /// Destructor
    ~TTDEventInfo() {}

    // Simple Getters
    /// get if stored information is valid
    bool isValid() {return m_isValid;}
    /// get if injection in HER/LER
    bool isHER() {return m_isHER;}
    /// get lowest bit of revolution counter
    bool isRevo2() {return m_revo2;}
    /// get time since the last injection in clock ticks (FTSW clock)
    unsigned int getTimeSinceLastInjection() {return m_timeSinceLastInjection;}
    /// get time since the last injection in clock ticks (FTSW clock)
    unsigned int getTimeSincePrevTrigger() {return m_timeSincePrevTrigger;}
    /// get number of triggered bunch
    unsigned int getBunchNumber() {return m_bunchNumber;}
    /// get if an injection happened recently (and the corresponding stored data is actually valid)
    bool hasInjection() {return m_timeSinceLastInjection == 0x7FFFFFF;}

    // Simple Setters
    /// set that stored information is valid
    void setValid() {m_isValid = true;}
    /// set that stored information is valid
    void setInvalid() {m_isValid = false;}
    /// set if injection in HER/LER
    void setIsHER(bool isHER) {m_isHER = isHER;}
    /// set lowest bit of revolution counter
    void setRevo2(bool revo2) {m_revo2 = revo2;}
    /// set time since the last injection in clock ticks (FTSW clock)
    void setTimeSinceLastInjection(unsigned int timeSinceLastInjection) {m_timeSinceLastInjection = timeSinceLastInjection;}
    /// set time since the previous trigger in clock ticks (FTSW clock)
    void setTimeSincePrevTrigger(unsigned int timeSincePrevTrigger) {m_timeSincePrevTrigger = timeSincePrevTrigger;}
    /// set number of triggered bunch
    void setBunchNumber(unsigned int bunchNumber) {m_bunchNumber = bunchNumber;}
    /// set that no injection happened recently (and the corresponding stored data is actually invalid)
    void setNoInjection() {m_timeSinceLastInjection = 0x7FFFFFF;}

    // Additional Functions (not inline)
    /// get time since the last injection in microseconds
    Double_t getTimeSinceLastInjectionInMicroSeconds();
    /// get time since the previous trigger in microseconds
    Double_t getTimeSincePrevTriggerInMicroSeconds();

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
    * Time since the last injection in clock ticks (FTSW clock)
    * Note: A value of '0x7FFFFFF' means no injection took place recently,
    *       which is defined by the RawFTSW
    **/
    unsigned int m_timeSinceLastInjection;
    /**
    * Time since the previous trigger in clock ticks (FTSW clock)
    **/
    unsigned int m_timeSincePrevTrigger;
    /**
    * Number of triggered bunch, ranging from 0-1279
    * Note: There are a maximum of 5120 buckets, which could each carry one bunch of e+/e-,
    *       but we only have 1280 clock ticks (=5120/4) to identify the bunches
    **/
    unsigned int m_bunchNumber;

    /// The clock, to translate clock ticks to microseconds
    DBObjPtr<HardwareClockSettings> m_clockSettings; //! tells ROOT not to write it to file (transient)

    ClassDef(TTDEventInfo, 1) ///< Storage element for TTD information
  };
}
