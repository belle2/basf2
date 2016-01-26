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
      CDCHit(), m_segmentID(0), m_priorityPosition(0), m_leftRight(0)
    { }

    /** copy constructor. */
    CDCTriggerSegmentHit(const CDCHit& centerHit,
                         unsigned short segmentID,
                         unsigned short priorityPosition,
                         unsigned short leftRight):
      CDCHit(centerHit.getTDCCount(), centerHit.getADCCount(),
             centerHit.getISuperLayer(), centerHit.getILayer(), centerHit.getIWire()),
      m_segmentID(segmentID), m_priorityPosition(priorityPosition), m_leftRight(leftRight)
    { }

    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTriggerSegmentHit() { }

    //modifiers
    /** set left/right information (e.g. if left/right is updated during fit) */
    void setLeftRight(unsigned short LR) { m_leftRight = LR; }

    //accessors
    /** get continuous ID of the track segment [0, 2335] */
    unsigned short getSegmentID() const { return m_segmentID; }
    /** get position of the priority cell within the track segment
     *  (0: no hit, 3: 1st priority, 1: 2nd right, 2: 2nd left) */
    unsigned short getPriorityPosition() const { return m_priorityPosition; }
    /** get position of the priority cell relative to the track
     *  (0: no hit, 1: right, 2: left, 3: not determined) */
    unsigned short getLeftRight() const { return m_leftRight; }
    /** get TDC count without offset.
     * default offset and binwidth are current values applied by the CDCDigitizer. */
    short getTDCCountWithoutOffset(short offset = 8192, double binwidth = 1. / 1.017774) const
    {
      return offset - m_tdcCount * binwidth;
    }
    /** true if LeftRight position is determined */
    bool LRknown() const { return (m_leftRight == 1 || m_leftRight == 2); }

  protected:
    /** continuous ID of the track segment */
    unsigned short m_segmentID;
    /** position of the priority cell as an id:
     *  0: no hit, 3: 1st priority, 1: 2nd right, 2: 2nd left */
    unsigned short m_priorityPosition;
    /** position of the priority cell relative to the track:
     *  0: no hit, 1: right, 2: left, 3: not determined */
    unsigned short m_leftRight;

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerSegmentHit, 2);
  };
}
#endif
