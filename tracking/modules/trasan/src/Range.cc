//-----------------------------------------------------------fmt version 0.00--
// $Id: Range.cc 9944 2006-11-29 07:36:07Z katayama $
//-----------------------------------------------------------------------------
// Header file for Multi-TDC data
//-----------------------------------------------------------------------------
// Filename : MultiTDC.h
// Section  : CDC TSF
// Owner    : Yoshihito Iwasaki
// Email    : yiwaskai@kekvax.kek.jp
//-----------------------------------------------------------------------------
// Description :
//-----------------------------------------------------------------------------
// Version |    Date     | Explanation of changes
//   00.00 | 03-Aug-1996 | Creation
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.2  2000/04/13 13:22:50  katayama
// Added std:: to cout,cerr,endl etc.
//
// Revision 1.1  1998/05/22 18:05:44  yiwasaki
// Pulse, Range added
//
// Revision 1.3  1997/10/15 08:38:52  yiwasaki
// Debug information removed
//
// Revision 1.2  1997/10/15 08:31:08  yiwasaki
// Slightly modified
//
// Revision 1.1  1997/06/10 08:10:11  katayama
// The first version from Y. Iwasaki
//
//-----------------------------------------------------------------------------


#include <iostream>
#include "tracking/modules/trasan/Range.h"


namespace Belle {

  Range::Range()
  {
    _low = -999.;
    _high = -999.;
  }

  Range::Range(const Range& ib)
  {
    _low = ib.low();
    _high = ib.high();
  }

  Range::Range(const float iLow, const float iHigh)
  {
    if (iHigh > iLow) {
      _low = iLow;
      _high = iHigh;
    } else {
      _low = iHigh;
      _high = iLow;
    }
  }

  Range::~Range()
  {
  }

  bool
  Range::operator == (const Range& a) const
  {
    if (_low != a.low()) return false;
    if (_high != a.high()) return false;
    return true;
  }

  int
  Range::dump(void) const
  {
    std::cout << _low << "~" << _high;
    return 0;
  }

} // namespace Belle

