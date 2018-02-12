#ifndef CDCTRIGGERSEGMENTHIT_H
#define CDCTRIGGERSEGMENTHIT_H

#include <framework/datastore/RelationsObject.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/dataobjects/CDCHit.h>

namespace Belle2 {
  /** Combination of several CDCHits to a track segment hit for the trigger.
   *  Result of the track segment finder.
   */
  class CDCTriggerSegmentHit : public RelationsObject {
  public:
    /** default constructor. */
    CDCTriggerSegmentHit():
      m_segmentID(0), m_priorityPosition(0), m_leftRight(0),
      m_priorityTime(0), m_fastestTime(0), m_foundTime(0),
      m_eWire(65535)
    { }

    /** constructor using continuous TS ID. */
    CDCTriggerSegmentHit(unsigned short segmentID,
                         unsigned short priorityPosition,
                         unsigned short leftRight,
                         short priorityTime,
                         short fastestTime,
                         short foundTime);

    /** constructor using super layer ID and TS ID in layer (== central wire ID). */
    CDCTriggerSegmentHit(unsigned short iSL,
                         unsigned short iWire,
                         unsigned short priorityPosition,
                         unsigned short leftRight,
                         short priorityTime,
                         short fastestTime,
                         short foundTime);

    /** constructor using continuous TS ID and a reference to the priority hit
     *  (to save some calculations). */
    CDCTriggerSegmentHit(const CDCHit& priorityHit,
                         unsigned short segmentID,
                         unsigned short priorityPosition,
                         unsigned short leftRight,
                         short priorityTime,
                         short fastestTime,
                         short foundTime);

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

    /** get super layer number. */
    unsigned short getISuperLayer() const
    {
      return WireID(m_eWire).getISuperLayer();
    }
    /** get priority layer number within super layer. */
    unsigned short getILayer() const
    {
      return WireID(m_eWire).getILayer();
    }
    /** get wire number of priority wire within layer. */
    unsigned short getIWire() const
    {
      return WireID(m_eWire).getIWire();
    }
    /** get the encoded wire number of the priority wire.
     *
     *  This number can be used directly e.g. with the = operator to create a WireID object.
     */
    unsigned short getID() const
    {
      return m_eWire;
    }

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

    /** Wire encoding of the priority wire.
     *  Details are explained in the separate WireID object. */
    unsigned short m_eWire;

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerSegmentHit, 4);
  };
}
#endif
