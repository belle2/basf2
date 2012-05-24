//-----------------------------------------------------------------------------
// $Id: TCircleFitter.h 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : TCircleFitter.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object to a circle.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.7  2002/01/03 11:04:58  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.6  2001/12/23 09:58:55  katayama
// removed Strings.h
//
// Revision 1.5  2001/12/19 02:59:54  katayama
// Uss find,istring
//
// Revision 1.4  2001/12/14 02:54:49  katayama
// For gcc-3.0
//
// Revision 1.3  2001/04/11 01:10:02  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.2  1999/10/30 10:12:45  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.1  1999/01/11 03:03:25  yiwasaki
// Fitters added
//
//-----------------------------------------------------------------------------

#ifndef TCIRCLEFITTER_FLAG_
#define TCIRCLEFITTER_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif
#define HEP_SHORT_NAMES


#include "tracking/modules/trasan/TFitter.h"
#include "tracking/modules/trasan/THelix.h"

namespace Belle {

  class TLink;
  class TCircle;

/// A class to fit a TTrackBase object to a circle.
  class TCircleFitter : public TFitter {

  public:
    /// Constructor.
    TCircleFitter(const std::string& name);

    /// Destructor
    virtual ~TCircleFitter();

  public:// Selectors
    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  public:// Modifiers
    virtual int fit(TTrackBase&) const;

  private:
    mutable double _charge;
    mutable double _radius;
    mutable HepGeom::Point3D<double>  _center;
  };

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define TCIRCLEFITTER_INLINE_DEFINE_HERE
#endif

#ifdef TCIRCLEFITTER_INLINE_DEFINE_HERE

#endif

#undef inline

} // namespace Belle

#endif /* TCIRCLEFITTER_FLAG_ */
