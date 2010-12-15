//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : FrontEnd.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a CDC front-end board
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCFrontEnd_FLAG_
#define TRGCDCFrontEnd_FLAG_

#include <float.h>
#include <string>
#include "trg/trg/Board.h"
#include "trg/trg/Link.h"

#ifdef TRGCDC_SHORT_NAMES
#define CTFrontEnd TRGCDCFrontEnd
#endif

namespace Belle2 {

class TRGCDCWire;

/// A class to represent a CDC front-end board
class TRGCDCFrontEnd
    : public TRGBoard,
      public std::vector<const TRGCDCWire *> {
    
  public:
    /// Constructor.
    TRGCDCFrontEnd(const std::string & name, const TRGClock &);

    /// Destructor
    virtual ~TRGCDCFrontEnd();

  public:// Selectors.

//     /// dumps contents. "message" is to select information to dump. "pre" will be printed in head of each line.
//     void dump(const std::string & message = "",
// 	      const std::string & pre = "") const;

  public:// Modifiers

    /// appends wire
//    void append(const TRGCDCWire *);
};

//-----------------------------------------------------------------------------

#ifdef TRGCDCFrontEnd_NO_INLINE
#define inline
#else
#undef inline
#define TRGCDCFrontEnd_INLINE_DEFINE_HERE
#endif

#ifdef TRGCDCFrontEnd_INLINE_DEFINE_HERE

// inline
// void
// TRGCDCFrontEnd::append(const TRGCDCWire * w) {
//     _wires.push_back(w);
// }

#endif

#undef inline

} // namespace Belle2

#endif /* TRGCDCFrontEnd_FLAG_ */
