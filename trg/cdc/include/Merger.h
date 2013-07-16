//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Merger.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a CDC front-end board
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCMerger_FLAG_
#define TRGCDCMerger_FLAG_

#include <float.h>
#include <string>
#include "trg/trg/Board.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCMerger TRGCDCMerger
#endif

namespace Belle2 {

class TRGCDCFrontEnd;

/// A class to represent a CDC merger board
class TRGCDCMerger
    : public TRGBoard,
      public std::vector<const TRGCDCFrontEnd *> {
    
  public:
    /// Constructor.
    TRGCDCMerger(const std::string & name,
		 const TRGClock & systemClock,
		 const TRGClock & dataClock,
		 const TRGClock & userClockInput,
		 const TRGClock & userClockOutput);

    /// Destructor
    virtual ~TRGCDCMerger();

  public:// Selectors.

//     /// dumps contents. "message" is to select information to dump. "pre" will be printed in head of each line.
//     void dump(const std::string & message = "",
//               const std::string & pre = "") const;

  public:// Modifiers

};

//-----------------------------------------------------------------------------

} // namespace Belle2

#endif /* TRGCDCMerger_FLAG_ */
