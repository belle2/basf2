//-----------------------------------------------------------------------------
// $Id: THistogram.h 10002 2007-02-26 06:56:17Z katayama $
//-----------------------------------------------------------------------------
// Filename : THistogram.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class for a histogram used in tracking.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.19  2002/01/03 11:04:58  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.18  2001/12/23 09:58:56  katayama
// removed Strings.h
//
// Revision 1.17  2001/12/19 02:59:55  katayama
// Uss find,istring
//
// Revision 1.16  2001/12/14 02:54:49  katayama
// For gcc-3.0
//
// Revision 1.15  2001/04/11 01:10:03  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.14  2000/01/28 06:30:31  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.13  1999/10/30 10:12:49  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.12  1998/07/29 04:35:20  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.10  1998/07/02 09:04:46  yiwasaki
// Trasan 1.0 official release
//
// Revision 1.9  1998/06/11 08:15:46  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.8  1998/05/26 05:09:17  yiwasaki
// cvs repair
//
// Revision 1.7  1998/05/22 08:21:50  yiwasaki
// Trasan 1 alpha 4 release, TSegment added, TConformalLink no longer used
//
// Revision 1.6  1998/05/18 08:08:56  yiwasaki
// preparation for alpha 3
//
// Revision 1.5  1998/05/08 09:47:06  yiwasaki
// Trasan 1 alpha 2 relase, stereo recon. added, off-vtx recon. added
//
// Revision 1.4  1998/04/23 17:25:04  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.3  1998/04/16 16:51:08  yiwasaki
// minor changes
//
// Revision 1.2  1998/04/14 01:05:54  yiwasaki
// TRGCDCWireHitMC added
//
// Revision 1.1  1998/04/10 00:51:15  yiwasaki
// TCircle, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
// Revision 1.1  1998/02/09 04:12:20  yiwasaki
// Trasan 0.1 beta
//
//
//-----------------------------------------------------------------------------

#ifndef THistogram_FLAG_
#define THistogram_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>

#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/AList.h"

#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TSegment0.h"
#include "tracking/modules/trasan/TSegment.h"

namespace Belle {

  class TCircle;

/// A class for a histogram used in tracking.
  class THistogram {

  public:
    /// Constructor.
    THistogram(unsigned nBins);

    /// Destructor
    virtual ~THistogram();

  public:// Selectors
    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// returns a pointer to i'th AList<TLink>.
    const AList<TLink> * const bin(unsigned i) const;

    /// returns a pointer to i'th AList<TLink>.
    const AList<TLink> * const bin(int i) const;

    /// returns number in i'th bin.
    unsigned nBin(unsigned i) const;

    /// returns \# of bins.
    unsigned nBins(void) const;

    /// returns an AList of TLinks within 'center' bin +- 'width' bin region.
    AList<TLink> contents(unsigned center, unsigned width) const;

    /// returns an AList of TLinks from 'start' bin to 'end' bin region.
    AList<TLink> contents(int start, int end) const;

    /// returns an AList<TLink> of all contents.
    const AList<TLink> & contents(void) const;

    /// returns an AList<TSegment0> of clusters.
    AList<TSegment0> clusters0(void) const;

    /// returns an AList<TSegment0> of clusters.
    AList<TSegment> clusters(void) const;

    /// returns an AList<TSegment0> using clusters() function.
    AList<TSegment0> segments0(void) const;

    /// returns an AList<TSegment0> using clusters() function.
    AList<TSegment> segments(void) const;

  public:// Modifiers
    /// fills with hits.
    void fillX(const AList<TLink> & links);

    /// fills with hits.
    void fillY(const AList<TLink> & links);

    /// fills with hits.
    void fillPhi(const AList<TLink> & links);

    /// removes links.
    void remove(const AList<TLink> & links);

    /// masks a bin. Masked bin will not be returned by ::findPeak.
    void mask(unsigned binN);

    /// remove masks.
    void unmask(void);

  private:
    unsigned _nBins;
    unsigned* _bins;
    bool* _masks;
    AList<TLink> ** _links;
    AList<TLink> _all;
    float _binSize;
  };

//-----------------------------------------------------------------------------

#ifdef THistogram_NO_INLINE
#define inline
#else
#undef inline
#define THistogram_INLINE_DEFINE_HERE
#endif

#ifdef THistogram_INLINE_DEFINE_HERE

  inline
  const AList<TLink> * const
  THistogram::bin(unsigned i) const
  {
    if (i < _nBins) return _links[i];
    return 0;
  }

  inline
  const AList<TLink> * const
  THistogram::bin(int i) const
  {
    while (i < 0) i += _nBins;
    return _links[i % _nBins];
  }

  inline
  unsigned
  THistogram::nBins(void) const
  {
    return _nBins;
  }

  inline
  void
  THistogram::mask(unsigned a)
  {
    _masks[a] = true;
  }

  inline
  void
  THistogram::unmask(void)
  {
    for (unsigned i = 0; i < _nBins; i++) _masks[i] = false;
  }

  inline
  const AList<TLink> &
  THistogram::contents(void) const
  {
    return _all;
  }

  inline
  unsigned
  THistogram::nBin(unsigned i) const
  {
    if (i < _nBins) return _bins[i];
    return 0;
  }

#endif

#undef inline

} // namespace Belle

#endif /* THistogram_FLAG_ */
