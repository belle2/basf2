/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGTOPSLOTTIMING_H
#define TRGTOPSLOTTIMING_H

#include <TObject.h>

#include <vector>
#include <limits>


namespace Belle2 {

  static const int intNaN = std::numeric_limits<int>::quiet_NaN();

  //! Example Detector
  class TRGTOPSlotTiming : public TObject {

  public:

    // Empty constructor
    // Recommended for ROOT IO
    TRGTOPSlotTiming() :
      m_slotId(intNaN),
      m_slotTiming(intNaN),
      m_slotSegment(intNaN),
      m_slotNHits(intNaN),
      m_slotLogL(intNaN),
      m_slotNErrors(intNaN)
    {}

    //! A Useful Constructor
    explicit TRGTOPSlotTiming(
      int slotId
    ) :
      m_slotId(slotId),
      m_slotTiming(intNaN),
      m_slotSegment(intNaN),
      m_slotNHits(intNaN),
      m_slotLogL(intNaN),
      m_slotNErrors(0)
    {}

    //! Another Useful Constructor
    TRGTOPSlotTiming(
      int slotId,
      int slotTiming,
      int slotSegment,
      int slotNHits,
      int slotLogL,
      int slotNErrors
    ) :
      m_slotId(slotId),
      m_slotTiming(slotTiming),
      m_slotSegment(slotSegment),
      m_slotNHits(slotNHits),
      m_slotLogL(slotLogL),
      m_slotNErrors(slotNErrors)
    {}

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
