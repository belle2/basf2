//-----------------------------------------------------------------------------
// $Id: TLineFitter.h 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : TLineFitter.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object to a line.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.8  2001/12/23 09:58:56  katayama
// removed Strings.h
//
// Revision 1.7  2001/12/19 02:59:55  katayama
// Uss find,istring
//
// Revision 1.6  2001/12/14 02:54:50  katayama
// For gcc-3.0
//
// Revision 1.5  2001/04/11 01:10:03  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.4  2000/02/03 06:18:07  yiwasaki
// Trasan 1.68k : slow finder in conf. finder temporary off
//
// Revision 1.3  1999/11/19 09:13:16  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.2  1999/10/30 10:12:50  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.1  1999/01/11 03:03:27  yiwasaki
// Fitters added
//
//-----------------------------------------------------------------------------

#ifndef TLINEFITTER_FLAG_
#define TLINEFITTER_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>

#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/TFitter.h"

namespace Belle {

/// A class to fit a TTrackBase object to a line.
  class TLineFitter : public TFitter {

  public:
    /// Constructor.
    TLineFitter(const std::string& name);

    /// Destructor
    virtual ~TLineFitter();

  public:// Selectors
    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;
    double a(void) const;
    double b(void) const;
    double det(void) const;

  public:// Modifiers
    virtual int fit(TTrackBase&) const;

  private:
    mutable double _a;
    mutable double _b;
    mutable double _det;
  };

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define TLINEFITTER_INLINE_DEFINE_HERE
#endif

#ifdef TLINEFITTER_INLINE_DEFINE_HERE

  inline
  double
  TLineFitter::a(void) const
  {
    return _a;
  }

  inline
  double
  TLineFitter::b(void) const
  {
    return _b;
  }

  inline
  double
  TLineFitter::det(void) const
  {
    return _det;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TLINEFITTER_FLAG_ */
