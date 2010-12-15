//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Merger.h
// Section  : TRG
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
#include "trg/trg/Link.h"

#ifdef TRGCDC_SHORT_NAMES
#define CTMerger TRGCDCMerger
#endif

namespace Belle2 {

class TRGCDCFrontEnd;

/// A class to represent a CDC front-end board
class TRGCDCMerger
    : public TRGBoard,
      public std::vector<const TRGCDCFrontEnd *> {
    
  public:
    /// Constructor.
    TRGCDCMerger(const std::string & name, const TRGClock &);

    /// Destructor
    virtual ~TRGCDCMerger();

  public:// Selectors.

//     /// dumps contents. "message" is to select information to dump. "pre" will be printed in head of each line.
//     void dump(const std::string & message = "",
// 	      const std::string & pre = "") const;

  public:// Modifiers

};

//-----------------------------------------------------------------------------

#ifdef TRGCDCMerger_NO_INLINE
#define inline
#else
#undef inline
#define TRGCDCMerger_INLINE_DEFINE_HERE
#endif

#ifdef TRGCDCMerger_INLINE_DEFINE_HERE

#endif

#undef inline

} // namespace Belle2

#endif /* TRGCDCMerger_FLAG_ */
