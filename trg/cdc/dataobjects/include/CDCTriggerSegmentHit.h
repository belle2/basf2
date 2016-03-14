#ifndef CDCTRIGGERSEGMENTHIT_H
#define CDCTRIGGERSEGMENTHIT_H

#include <cdc/dataobjects/CDCHit.h>

namespace Belle2 {
  /** Combination of several CDCHits to a track segment hit for the trigger.
   *  Result of the track segment finder.
   *  Contains CDCHit information from the priority hit
   *  plus additional TS information.
   */
  class CDCTriggerSegmentHit : public CDCHit {
  public:
    /** default constructor. */
    CDCTriggerSegmentHit():
      CDCHit(), m_segmentID(0), m_priorityPosition(0), m_leftRight(0), m_priorityTime(0), m_fastestTime(0), m_foundTime(0)
    { }

    /** copy constructor. */
    CDCTriggerSegmentHit(const CDCHit& centerHit,
                         unsigned short segmentID,
                         unsigned short priorityPosition,
                         unsigned short leftRight,
                         short priorityTime,
                         short fastestTime,
                         short foundTime):
      CDCHit(centerHit.getTDCCount(), centerHit.getADCCount(),
             centerHit.getISuperLayer(), centerHit.getILayer(), centerHit.getIWire()),
      m_segmentID(segmentID), m_priorityPosition(priorityPosition), m_leftRight(leftRight),
      m_priorityTime(priorityTime), m_fastestTime(fastestTime), m_foundTime(foundTime)
    { }

    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTriggerSegmentHit() { }

    //accessors
    /** get continuous ID of the track segment [0, 2335] */
    unsigned short getSegmentID() const { return m_segmentID; }
    /** get position of the priority cell within the track segment
     *  (0: no hit, 3: 1st priority, 1: 2nd right, 2: 2nd left) */
    unsigned short getPriorityPosition() const { return m_priorityPosition; }
    /** get position of the priority cell relative to the track
     *  (0: no hit, 1: right, 2: left, 3: not determined) */
    unsigned short getLeftRight() const { return m_leftRight; }
    /** true if LeftRight position is determined */
    bool LRknown() const { return (m_leftRight == 1 || m_leftRight == 2); }
    /** get hit time of priority cell in trigger clocks */
    short priorityTime() const { return m_priorityTime; }
    /** get time of first hit in the track segment in trigger clocks */
    short fastestTime() const { return m_fastestTime; }
    /** get time when segment hit was found in trigger clocks */
    short foundTime() const { return m_foundTime; }

  protected:
    /** continuous ID of the track segment */
    unsigned short m_segmentID;
    /** position of the priority cell as an id:
     *  0: no hit, 3: 1st priority, 1: 2nd right, 2: 2nd left */
    unsigned short m_priorityPosition;
    /** position of the priority cell relative to the track:
     *  0: no hit, 1: right, 2: left, 3: not determined */
    unsigned short m_leftRight;
    /** hit time of priority cell in trigger clocks (~ 2ns) */
    short m_priorityTime;
    /** time of first hit in the track segment in trigger clocks (~ 2ns) */
    short m_fastestTime;
    /** time when segment hit was found in trigger clocks (~ 2ns) */
    short m_foundTime;

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerSegmentHit, 3);
  };
}
#endif
