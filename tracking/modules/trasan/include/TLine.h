//-----------------------------------------------------------------------------
// $Id: TLine.h 10021 2007-03-03 05:43:02Z katayama $
//-----------------------------------------------------------------------------
// Filename : TLine.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a line in tracking.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.19  2005/03/11 03:58:35  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.18  2002/01/03 11:04:58  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.17  2001/12/23 09:58:56  katayama
// removed Strings.h
//
// Revision 1.16  2001/12/19 02:59:55  katayama
// Uss find,istring
//
// Revision 1.15  2001/12/14 02:54:50  katayama
// For gcc-3.0
//
// Revision 1.14  2001/04/11 01:10:03  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.13  2000/10/05 23:54:31  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.12  2000/01/28 06:30:32  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.11  1999/10/30 10:12:49  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.10  1999/03/11 23:27:27  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.9  1999/01/11 03:03:26  yiwasaki
// Fitters added
//
// Revision 1.8  1998/12/24 08:46:58  yiwasaki
// stereo building modified by J.Suzuki
//
// Revision 1.7  1998/08/12 16:33:01  yiwasaki
// Trasan 1.08 release : stereo finder updated by J.Suzuki, new MC classes added by Y.Iwasaki
//
// Revision 1.6  1998/07/29 04:35:22  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.4  1998/07/02 09:04:46  yiwasaki
// Trasan 1.0 official release
//
// Revision 1.3  1998/06/19 12:17:12  yiwasaki
// Trasan 1 beta 4.1 release, TBuilder::buildStereo updated
//
// Revision 1.2  1998/06/14 11:09:58  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
// Revision 1.1  1998/06/11 08:15:47  yiwasaki
// Trasan 1 beta 1 release
//
//-----------------------------------------------------------------------------

#ifndef TLine_FLAG_
#define TLine_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>

#define HEP_SHORT_NAMES

#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TLineFitter.h"

namespace Belle {



/// A class to represent a track in tracking.
  class TLine : public TTrackBase {

  public:
    /// Constructor.
    TLine();

    /// Constructor.
    TLine(const AList<TLink> &);

    /// Destructor
    virtual ~TLine();

  public:// Selectors
    /// returns type.
    virtual unsigned objectType(void) const;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// returns coefficient a.
    double a(void) const;

    /// returns coefficient b.
    double b(void) const;

    /// returns chi2.
    double chi2(void) const;

    /// returns reduced-chi2.
    double reducedChi2(void) const;

  public:// Utilities
    /// returns distance to a position of TLink itself. (not to a wire)
    double distance(const TLink&) const;
    double distance(const HepGeom::Point3D<double>  &) const;

  public:// Modifiers
    /// fits itself. Error was happened if return value is not zero.
    // int fitx(void);

    /// fits itself using isolated hits. Error was happened if return value is not zero.
    int fit2();

    /// fits itself using single hits in a wire-layer. Error was happened if return value is not zero.
    int fit2s();

    /// fits itself using isolated hits. Error was happened if return value is not zero.
    int fit2p();

    /// fits itself using single hits in a wire-layer. Error was happened if return value is not zero.
    int fit2sp();

    /// remove extremly bad points.
    void removeChits();

    /// remove bad points by chi2. Bad points are returned in a 'list'. fit() should be called before calling this function.
    void refine(AList<TLink> & list, float maxSigma);

    ///
    void removeSLY(AList<TLink> & list);

    ///
    void appendSLY(AList<TLink> & list);

    ///
    void appendByszdistance(AList<TLink> & list, unsigned isl, float maxSigma);

    /// sets line properties.
    void property(double a, double b, double det);

  private:// Always updated
    mutable bool _fittedUpdated;

  private:// Updated when fitted
    double _a;
    double _b;
    double _det;
    static const TLineFitter _fitter;

  private:// Updated when fitted and accessed
    mutable double _chi2;
    mutable double _reducedChi2;
  };

/// Sorter
#if defined(__GNUG__)
  int
  SortByB(const TLine** a, const TLine** b);
#else
  extern "C" int
  SortByB(const void* a, const void* b);
#endif

//-----------------------------------------------------------------------------

#ifdef TLine_NO_INLINE
#define inline
#else
#undef inline
#define TLine_INLINE_DEFINE_HERE
#endif

#ifdef TLine_INLINE_DEFINE_HERE

  inline
  double
  TLine::a(void) const
  {
#ifdef TRASAN_DEBUG
    if (! _fitted) std::cout << "TLine::a !!! fit not performed" << std::endl;
#endif
    return _a;
  }

  inline
  double
  TLine::b(void) const
  {
#ifdef TRASAN_DEBUG
    if (! _fitted) std::cout << "TLine::b !!! fit not performed" << std::endl;
#endif
    return _b;
  }

  inline
  double
  TLine::distance(const TLink& l) const
  {
#ifdef TRASAN_DEBUG
    if (! _fitted) std::cout << "TLine::distance !!! fit not performed" << std::endl;
#endif
    double dy = fabs(_a * l.position().x() + _b - l.position().y());
    double invCos = sqrt(1. + _a * _a);
    return dy / invCos;
  }

  inline
  double
  TLine::distance(const HepGeom::Point3D<double>  & p) const
  {
#ifdef TRASAN_DEBUG
    if (! _fitted) std::cout << "TLine::distance !!! fit not performed" << std::endl;
#endif
    double dy = fabs(_a * p.x() + _b - p.y());
    double invCos = sqrt(1. + _a * _a);
    return dy / invCos;
  }

  inline
  void
  TLine::property(double a, double b, double det)
  {
    _a = a;
    _b = b;
    _det = det;
  }

  inline
  unsigned
  TLine::objectType(void) const
  {
    return Line;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TLine_FLAG_ */
