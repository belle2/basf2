#ifndef TELEVENTINFO_H
#define TELEVENTINFO_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {
  /** Store event number, trigger ID, and time stamp.
   */
  class TelEventInfo : public RelationsObject {
  public:

    /** Constructor. */
    explicit TelEventInfo(ULong64_t event = 0,
                          ULong64_t triggerID = 0,
                          ULong64_t timeStamp = 0);

    /** Destructor. */
    ~TelEventInfo() {}

    /** Event Setter.
     *
     *  @param event The current event number.
     */
    void setEventNumber(ULong64_t event) {
      m_event = event;
    }

    /** Trigger ID Setter.
     *
     *  @param triggerID The number of the current trigger ID.
     */
    void setTriggerID(ULong64_t triggerID) {
      m_triggerID = triggerID;
    }

    /** Timestamp Setter.
     *
     *  @param timeStamp The current time stamp.
     */
    void setTimeStamp(ULong64_t timeStamp) {
      m_timeStamp = timeStamp;
    }

    /** Event Getter.
     *
     *  @return The number of the current event.
     */
    ULong64_t getEventNumber() const {
      return m_event;
    }

    /** Trigger ID Getter.
     *
     *  @return The number current trigger ID:
     */
    ULong64_t getTriggerID() const {
      return m_triggerID;
    }

    /** Timestamp Getter.
     *
     *  @return The current time stamp:
     */
    ULong64_t getTimeStamp() const {
      return m_timeStamp;
    }

    /** Comparison Operator.
     *
     *  @param telEvtInfo The telescope event data to compare with.
     *  @return True if event and trigger ID numbers and the
     *  time stamp are the same
     */
    bool operator==(const TelEventInfo& telEvtInfo) const;

    /** Comparison Operator.
     *
     *  @param telEvtInfo The event meta data to compare with.
     *  @return True if event, or trigger ID numbers or the
     *  time stamp are different
     */
    bool operator!=(const TelEventInfo& telEvtInfo) const;

  private:

    /** Event number.
     */
    ULong64_t m_event;

    /** Trigger ID.
     */
    ULong64_t m_triggerID;

    /** Timestamp.
     */
    ULong64_t m_timeStamp;

    ClassDef(TelEventInfo, 2) /**< Store event number, run number, and trigger ID. */
  }; //class
} // namespace Belle2
#endif // EVENTMETADATA
