/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGTOPTIMINGISIM_H
#define TRGTOPTIMINGISIM_H

//#include <TObject.h>
#include <framework/datastore/RelationsObject.h>

#include <vector>
#include <limits>


namespace Belle2 {

  //! Example Detector
  //  class TRGTOPTimingISim : public TObject {
  class TRGTOPTimingISim : public RelationsObject {

  public:

    // Empty constructor
    // Recommended for ROOT IO
    TRGTOPTimingISim() :
      m_slotId(0),
      m_slotTiming(0),
      m_slotSegment(0),
      m_slotNHits(0),
      m_slotLogL(0),
      m_slotDecisionClockCycle(0)
    {}

    //! A Useful Constructor
    explicit TRGTOPTimingISim(
      int slotId
    ) :
      m_slotId(slotId),
      m_slotTiming(0),
      m_slotSegment(0),
      m_slotNHits(0),
      m_slotLogL(0),
      m_slotDecisionClockCycle(0)
    {}

    //! Another Useful Constructor
    TRGTOPTimingISim(
      int slotId,
      int slotTiming,
      int slotSegment,
      int slotNHits,
      int slotLogL,
      int slotDecisionClockCycle
    ) :
      m_slotId(slotId),
      m_slotTiming(slotTiming),
      m_slotSegment(slotSegment),
      m_slotNHits(slotNHits),
      m_slotLogL(slotLogL),
      m_slotDecisionClockCycle(slotDecisionClockCycle)
    {}

    //! Destructor
    ~TRGTOPTimingISim() {}

    int getSlotId()      const { return m_slotId;     }
    int getSlotTiming()  const { return m_slotTiming; }
    int getSlotSegment() const { return m_slotSegment;}
    int getSlotNHits()   const { return m_slotNHits;  }
    int getSlotLogL()    const { return m_slotLogL;   }
    int getSlotDecisionClockCycle() const { return m_slotDecisionClockCycle;}
    //int get() const { return m_;}

    void setSlotId(int slotId);
    void setSlotTiming(int slotTiming);
    void setSlotSegment(int slotSegment);
    void setSlotNHits(int slotNHits);
    void setSlotLogL(int slotLogL);
    void setSlotDecisionClockCycle(int slotDecisionClockCycle);
    //void set(int );

  private:

    //! slot Id (1 through 16)
    int m_slotId;
    //! slot timing
    int m_slotTiming;
    //! slot segment
    int m_slotSegment;
    //! slot N hits (timestamps)
    int m_slotNHits;
    //! slot logL
    int m_slotLogL;
    //! clock cycle when the decision was made
    int m_slotDecisionClockCycle;

    /** the class title*/
    ClassDef(TRGTOPTimingISim, 1);

  };

} //end namespace Belle2

#endif
