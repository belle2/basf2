/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGTOPSLOTTIMING_H
#define TRGTOPSLOTTIMING_H

//#include <TObject.h>
#include <framework/datastore/RelationsObject.h>

#include <vector>
#include <limits>


namespace Belle2 {

  //! Example Detector
  //  class TRGTOPSlotTiming : public TObject {
  class TRGTOPSlotTiming : public RelationsObject {

  public:

    // Empty constructor
    // Recommended for ROOT IO
    TRGTOPSlotTiming() :
      m_slotId(0),
      m_slotTiming(0),
      m_slotSegment(0),
      m_slotNHits(0),
      m_slotLogL(0),
      m_slotDecisionClockCycle(0),
      m_slotNErrors(0),
      m_slotThisBoard(0),
      m_slotFirstTS(0)
    {}

    //! A Useful Constructor
    explicit TRGTOPSlotTiming(
      int slotId
    ) :
      m_slotId(slotId),
      m_slotTiming(0),
      m_slotSegment(0),
      m_slotNHits(0),
      m_slotLogL(0),
      m_slotDecisionClockCycle(0),
      m_slotNErrors(0),
      m_slotThisBoard(1),
      m_slotFirstTS(-1)
    {}

    //! Another Useful Constructor
    TRGTOPSlotTiming(
      int slotId,
      int slotTiming,
      int slotSegment,
      int slotNHits,
      int slotLogL,
      int slotDecisionClockCycle,
      int slotNErrors,
      int slotThisBoard,
      int slotFirstTS
    ) :
      m_slotId(slotId),
      m_slotTiming(slotTiming),
      m_slotSegment(slotSegment),
      m_slotNHits(slotNHits),
      m_slotLogL(slotLogL),
      m_slotDecisionClockCycle(slotDecisionClockCycle),
      m_slotNErrors(slotNErrors),
      m_slotThisBoard(slotThisBoard),
      m_slotFirstTS(slotFirstTS)
    {}

    //! Destructor
    ~TRGTOPSlotTiming() {}

    int getSlotId()      const { return m_slotId;     }
    int getSlotTiming()  const { return m_slotTiming; }
    int getSlotSegment() const { return m_slotSegment;}
    int getSlotNHits()   const { return m_slotNHits;  }
    int getSlotLogL()    const { return m_slotLogL;   }
    int getSlotDecisionClockCycle() const { return m_slotDecisionClockCycle;}
    int getSlotNErrors() const { return m_slotNErrors;}
    int getSlotThisBoard() const { return m_slotThisBoard;}
    int getSlotFirstTS() const { return m_slotFirstTS;}
    //int get() const { return m_;}

    void setSlotId(int slotId);
    void setSlotTiming(int slotTiming);
    void setSlotSegment(int slotSegment);
    void setSlotNHits(int slotNHits);
    void setSlotLogL(int slotLogL);
    void setSlotDecisionClockCycle(int slotDecisionClockCycle);
    void setSlotNErrors(int slotNErrors);
    void setSlotThisBoard(int slotThisBoard);
    void setSlotFirstTS(int slotFirstTS);
    //void set(int );

    bool isThisBoard() const;

    bool isFirstTSAvailable() const;

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
    //! slot errors
    int m_slotNErrors;
    //! source of slot information (1 means this UT3, 0 means the other UT3)
    int m_slotThisBoard;
    //! first TS for the decision (when available)
    int m_slotFirstTS;

    /** the class title*/
    ClassDef(TRGTOPSlotTiming, 5);

  };

} //end namespace Belle2

#endif
