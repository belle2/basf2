//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGTOPSlotTiming.h
// Section  : TRG TOP
// Owner    : Tong Pang  Vladimir Savinov
// Email    : top16@pitt.edu, vladimirsavinov@gmail.com
//-----------------------------------------------------------
// Description : A class to represent TOP L1 slot-level timing decision
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#ifndef TRGTOPSLOTTIMING_H
#define TRGTOPSLOTTIMING_H

#include <TObject.h>

#include <vector>

#include "trg/top/dataobjects/TRGTOPSlotTiming.h"

namespace Belle2 {

  //! Example Detector
  class TRGTOPSlotTiming : public TObject {

  public:

    // Empty constructor
    // Recommended for ROOT IO
    TRGTOPSlotTiming()
    {
    }

    //! A Useful Constructor
    TRGTOPSlotTiming(
      int slotId
    )
    {
      m_slotId  = slotId;
      m_slotNErrors = 0;
    }

    //! Another Useful Constructor
    TRGTOPSlotTiming(
      int slotId,
      int slotTiming,
      int slotSegment,
      int slotNHits,
      int slotLogL,
      int slotNErrors
    )
    {
      m_slotId      = slotId;
      m_slotTiming  = slotTiming;
      m_slotSegment = slotSegment;
      m_slotNHits   = slotNHits;
      m_slotLogL    = slotLogL;
      m_slotNErrors = slotNErrors;
    }

    //! Destructor
    ~TRGTOPSlotTiming() {}

    int getSlotId()      const { return m_slotId;     }
    int getSlotTiming()  const { return m_slotTiming; }
    int getSlotSegment() const { return m_slotSegment;}
    int getSlotNHits()   const { return m_slotNHits;  }
    int getSlotLogL()    const { return m_slotLogL;   }
    int getSlotNErrors() const { return m_slotNErrors;}
    //int get() const { return m_;}

    void setSlotId(int slotId);
    void setSlotTiming(int slotTiming);
    void setSlotSegment(int slotSegment);
    void setSlotNHits(int slotNHits);
    void setSlotLogL(int slotLogL);
    void setSlotNErrors(int slotNErrors);
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
    //! slot errors
    int m_slotNErrors;

    /** the class title*/
    ClassDef(TRGTOPSlotTiming, 2);

  };

} //end namespace Belle2

#endif

