//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : EventTime.h
// Section  : TRG CDC
// Owner    : KyungTae KIM (K.U.)
// Email    : ktkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class to get Event Time information
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------
#ifndef TRGCDCEventTime_FLAG_
#define TRGCDCEventTime_FLAG_

#include <vector>
#include <string>
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/Cell.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCEventTime TRGCDCEventTime
#endif

namespace Belle2{
  class TRGCDC;
  class TRGCDCLink;

  class TRGCDCEventTime {
    public:

      TRGCDCEventTime(const TRGCDC&);

      virtual ~TRGCDCEventTime();
    
    public : 
      
      void initialize(void);
      void terminate(void);
      double getT0(void)const;

    
    private : 
      const TRGCDC & _cdc;
  };
} // namespace Belle2

#endif
