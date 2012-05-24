//-----------------------------------------------------------------------------
// $Id: TRobustLineFitter.h 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : TRobustLineFitter.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object to a line.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.5  2001/12/23 09:58:57  katayama
// removed Strings.h
//
// Revision 1.4  2001/12/19 02:59:55  katayama
// Uss find,istring
//
// Revision 1.3  2001/12/14 02:54:50  katayama
// For gcc-3.0
//
// Revision 1.2  2001/04/11 01:10:04  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.1  2000/02/03 06:18:07  yiwasaki
// Trasan 1.68k : slow finder in conf. finder temporary off
//
//-----------------------------------------------------------------------------

#ifndef TROBUSTLINEFITTER_FLAG_
#define TROBUSTLINEFITTER_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>

#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/TLineFitter.h"

namespace Belle {

/// A class to fit a TTrackBase object to a line.
  class TRobustLineFitter : public TLineFitter {

  public:
    /// Constructor.
    TRobustLineFitter(const std::string& name);

    /// Destructor
    virtual ~TRobustLineFitter();

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
    double rofunc(const TTrackBase&, double) const;
    double select(unsigned k, unsigned n, double*) const;

  private:
    mutable double _a;
    mutable double _b;
    mutable double _det;
    mutable unsigned _n;
  };

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define TROBUSTLINEFITTER_INLINE_DEFINE_HERE
#endif

#ifdef TROBUSTLINEFITTER_INLINE_DEFINE_HERE

  inline
  double
  TRobustLineFitter::a(void) const
  {
    return _a;
  }

  inline
  double
  TRobustLineFitter::b(void) const
  {
    return _b;
  }

  inline
  double
  TRobustLineFitter::det(void) const
  {
    return _det;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TROBUSTLINEFITTER_FLAG_ */
