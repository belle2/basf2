/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>

namespace Belle2 {
  /**
   * Storage element for information from the Trigger Timing Distribution (TTD)
   * and EventT0-related information.
   *
   * Data is required to study injections and injection backgrounds,
   * as well as for the analysis and simulation of PXD Gated Mode.
   **/
  class EventLevelTriggerTimeInfo : public TObject {

  public:

    /** Flags for the EventT0 source. */
    enum EventT0Source : unsigned short {
      // In case additional subdetectors are included in the EventT0 determination,
      // they must be added here.
      // Flags are set in the EventT0Combiner module.
      c_fromSVD = (1 << 0),
      c_fromCDC = (1 << 1),
      c_fromECL = (1 << 2)
    };

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
      m_isValid(isValid), m_isHER(isHER), m_revo2(revo2), m_eventT0Source(EventT0Source{0}),
      m_timeSinceLastInjection(timeSinceLastInjection), m_timeSincePrevTrigger(timeSincePrevTrigger),
      m_bunchNumber(bunchNumber) {}

    /// Destructor
    ~EventLevelTriggerTimeInfo() {}

    // Simple Getters
    /// get if stored information is valid
    bool isValid() const {return m_isValid;}
    /// get if last injection was in HER/LER
    bool isHER() const {return m_isHER;}
    /// get lowest bit of revolution counter
    bool isRevo2() const {return m_revo2;}
    /// get time since the last injection (i.e. the injection-pre-kick signal) in clock ticks (FTSW clock)
    unsigned int getTimeSinceLastInjection() const {return m_timeSinceLastInjection;}
    /// get time since the previous trigger in clock ticks (FTSW clock)
    unsigned int getTimeSincePrevTrigger() const {return m_timeSincePrevTrigger;}
    /// get number of triggered bunch as provided by TTD
    unsigned int getBunchNumber() const {return m_bunchNumber;}
    /// get if an injection happened recently (and the corresponding stored data is actually valid)
    bool hasInjection() const {return m_timeSinceLastInjection != c_flagNoInjection;}

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
    double getTimeSinceLastInjectionInMicroSeconds() const;
    /// get time since the previous trigger in microseconds
    double getTimeSincePrevTriggerInMicroSeconds() const;
    /// get time since the injected bunch passed the detector in clock ticks (FTSW clock)
    int getTimeSinceInjectedBunch() const;
    /// get time since the injected bunch passed the detector in microseconds
    double getTimeSinceInjectedBunchInMicroSeconds() const;
    /// get the actual (=global) number of the triggered bunch
    int getTriggeredBunchNumberGlobal() const;

    // Setters and getters for EventT0Source flags
    /// Add the flag c_fromSVD to the EventT0 source mask
    void addEventT0SourceFromSVD() { setSources<EventT0Source::c_fromSVD>(true); }

    /// Remove the flag c_fromSVD from the EventT0 source mask
    void unsetEventT0SourceFromSVD() { setSources<EventT0Source::c_fromSVD>(false); }

    /// Return if the c_fromSVD flag is set in the EventT0 source mask
    bool hasEventT0SourceFromSVD() const { return hasEventT0Sources(EventT0Source::c_fromSVD); }

    /// Add the flag c_fromCDC to the EventT0 source mask
    void addEventT0SourceFromCDC() { setSources<EventT0Source::c_fromCDC>(true); }

    /// Remove the flag c_fromCDC from the EventT0 source mask
    void unsetEventT0SourceFromCDC() { setSources<EventT0Source::c_fromCDC>(false); }

    /// Return if the c_fromCDC flag is set in the EventT0 source mask
    bool hasEventT0SourceFromCDC() const { return hasEventT0Sources(EventT0Source::c_fromCDC); }

    /// Add the flag c_fromECL to the EventT0 source mask
    void addEventT0SourceFromECL() { setSources<EventT0Source::c_fromECL>(true); }

    /// Remove the flag c_fromECL from the EventT0 source mask
    void unsetEventT0SourceFromECL() { setSources<EventT0Source::c_fromECL>(false); }

    /// Return if the c_fromECL flag is set in the EventT0 source mask
    bool hasEventT0SourceFromECL() const { return hasEventT0Sources(EventT0Source::c_fromECL); }

    /// Add a specific flag (or combination of flags) to the EventT0 source mask
    void addEventT0Sources(EventT0Source sources) { m_eventT0Source = EventT0Source(m_eventT0Source bitor sources); }

    /// Unset a specific flag (or combination of flags) from the EventT0 source mask
    void unsetEventT0Sources(EventT0Source sources) { m_eventT0Source = EventT0Source(m_eventT0Source bitand ~sources); }

    /// Return if the specific flag is set in the EventT0 source mask
    bool hasEventT0Sources(EventT0Source sources) const { return EventT0Source(m_eventT0Source bitand sources); }

    /// Reset all the flags from the EventT0 source mask (aka: set the "total" mask to 0)
    void resetEventT0Sources() { unsetEventT0Sources(c_fromAllSubdetectors); }

    /// Return if at least one flag is set in the EventT0 source mask
    bool hasAnyEventT0Sources() const { return hasEventT0Sources(c_fromAllSubdetectors); }

    /// Return the EventT0 source mask
    const EventT0Source& getEventT0Sources() const { return m_eventT0Source; }

  private:

    /**
     * Generic setter for the EventT0Source flags. Used only internally.
     */
    template<EventT0Source source>
    void setSources(bool setTo)
    {
      if (setTo) {
        addEventT0Sources(source);
      } else {
        unsetEventT0Sources(source);
      }
    }

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
     * Mask for storing which subdetectors contributed to the EventT0
     */
    EventT0Source m_eventT0Source;
    /**
    * Time since the last injection in clock ticks (127MHz=RF/4 clock)
    * Note: A value of 0x7FFFFFFF (see `c_flagNoInjection`) means no injection took place recently
    *       (value is defined by the RawFTSW)
    * Note: This is actually the time since the injection-pre-kick signal was received
    *       so there is some offset (different for HER/LER) that will be handled by the analysis variable
    **/
    unsigned int m_timeSinceLastInjection;
    /**
    * Time since the previous trigger in clock ticks (127MHz=RF/4 clock)
    **/
    unsigned int m_timeSincePrevTrigger;
    /**
    * Number of triggered bunch, ranging from 0-1279 (in 127MHz clock ticks)
    * Note: There are a maximum of 5120 buckets, which could each carry one bunch of e+/e-,
    *       but we only have 1280 clock ticks (=5120/4) to identify the bunches
    * Note: This is the bunch number as given by the TTD. This might not be the 'global' bunch number,
    *       but the offset is taken into account by the method `getTriggeredBunchNumberGlobal()`.
    **/
    unsigned int m_bunchNumber;

    /// This number is defined by the FTSW to indicate no injection happened recently
    static const unsigned int c_flagNoInjection = 0x7FFFFFFF; //! tells ROOT not to write it to file (transient)

    static constexpr EventT0Source c_fromAllSubdetectors = EventT0Source(EventT0Source::c_fromSVD +
                                                           EventT0Source::c_fromCDC +
                                                           EventT0Source::c_fromECL); //! tells ROOT not to write it to file (transient)

    ClassDef(EventLevelTriggerTimeInfo, 2) ///< Storage element for TTD information
  };
}
