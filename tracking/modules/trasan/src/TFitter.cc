//-----------------------------------------------------------------------------
// $Id: TFitter.cc 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : TFitter.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.5  2003/12/25 12:03:33  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.4  2001/12/23 09:58:47  katayama
// removed Strings.h
//
// Revision 1.3  2001/12/19 02:59:46  katayama
// Uss find,istring
//
// Revision 1.2  1999/06/16 08:29:50  yiwasaki
// Trasan 1.44 release : new THelixFitter
//
// Revision 1.1  1999/01/11 03:03:11  yiwasaki
// Fitters added
//
//-----------------------------------------------------------------------------

#define TFITTER_INLINE_DEFINE_HERE

#include "tracking/modules/trasan/TFitter.h"
#include "tracking/modules/trasan/TTrackBase.h"

namespace Belle {

  TFitter::TFitter(const std::string& name) : _name(name)
  {
  }

  TFitter::~TFitter()
  {
  }

  void
  TFitter::fitDone(TTrackBase& t) const
  {
    t._fitted = true;
  }

  void
  TFitter::dump(const std::string&, const std::string&) const
  {
  }

} // namespace Belle

