//-----------------------------------------------------------------------------
// $Id: TBuilderCurl.h 10137 2007-05-25 04:25:24Z katayama $
//-----------------------------------------------------------------------------
// Filename : TBuilderCurl.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to build a curl track.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.24  2004/03/26 06:07:26  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.23  2003/12/25 12:04:43  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.22  2002/01/03 11:04:57  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.21  2001/12/23 09:58:54  katayama
// removed Strings.h
//
// Revision 1.20  2001/12/19 02:59:53  katayama
// Uss find,istring
//
// Revision 1.19  2001/12/14 02:54:46  katayama
// For gcc-3.0
//
// Revision 1.18  2001/04/11 01:10:01  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.17  2000/09/27 07:45:21  yiwasaki
// Trasan 2.05 : updates of curl and pm finders by JT
//
// Revision 1.16  2000/02/28 01:59:01  yiwasaki
// Trasan 2.00RC11 : curl updates only
//
// Revision 1.15  2000/02/15 13:46:48  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.14  2000/02/01 11:24:46  yiwasaki
// Trasan 1.68j : curl finder modified, new stereo finder modified
//
// Revision 1.13  1999/10/30 10:12:35  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.12  1999/10/15 04:28:07  yiwasaki
// TWindow is hidden, curl finder parameter
//
// Revision 1.11  1999/08/25 06:25:53  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.10  1999/06/14 05:51:06  yiwasaki
// Trasan 1.41 release : curl finder updates
//
// Revision 1.9  1999/04/02 09:15:26  yiwasaki
// Trasan 1.29 release : tighter cuts for curl finder by J.Tanaka
//
// Revision 1.8  1999/03/15 07:57:18  yiwasaki
// Trasan 1.27 release : curl finder update
//
// Revision 1.7  1999/03/12 06:56:49  yiwasaki
// Trasan 1.25 release : curl finder updated, salvage 2 is default, minor bug fixes
//
// Revision 1.6  1999/03/10 12:55:09  yiwasaki
// Trasan 1.23 release : curl finder updated
//
// Revision 1.5  1999/01/11 03:03:23  yiwasaki
// Fitters added
//
// Revision 1.4  1998/11/10 09:09:17  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.3  1998/10/13 04:04:50  yiwasaki
// Trasan 1.1 beta 7 release : memory leak fixed by J.Tanaka, TCurlFinderParameters.h added by J.Tanaka
//
// Revision 1.2  1998/10/06 02:30:12  yiwasaki
// Trasan 1.1 beta 3 relase : only minor change
//
// Revision 1.1  1998/09/29 01:24:31  yiwasaki
// Trasan 1.1 beta 1 relase : TBuilderCurl added
//
//-----------------------------------------------------------------------------

#ifndef TBuilderCurl_FLAG_
#define TBuilderCurl_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>

#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/AList.h"
#include "tracking/modules/trasan/TBuilder0.h"
#include "tracking/modules/trasan/TCurlFinderParameters.h"
//cnv #include "tracking/modules/trasan/TSvdFinder.h"
//cnv #include "tracking/modules/trasan/TSvdAssociator.h"

namespace Belle {

  class TRGCDC;


/// A class to build a Curl track.
  class TBuilderCurl : public TBuilder0 {

  public:
    /// Constructor.
    TBuilderCurl(const std::string& name);

    /// Destructor
    virtual ~TBuilderCurl();

  public:// Selectors
    /// appends stereo hits to a track.
    TTrack* buildStereo(TTrack& track, const AList<TLink> &) const;
    TTrack* buildStereoMC(TTrack& track, const AList<TLink> &) const;

    //const TSelector & trackSelector(const TSelector &);// not used
    void setParam(const TCurlFinderParameter&);

    TTrack* buildStereo(TTrack& track, const AList<TLink> &, const AList<TLink> &) const;
    bool buildStereo(TTrack& track, double& dZ, double& tanL) const;

    void setSvdClusters(void) {
      /* cnv     m_svdAssociator->clearClusters(); */
      /*     m_svdAssociator->fillClusters(); */
    }
  private:
    static void set_smallcell(bool s) {
      ms_smallcell = s;
    }
    static void set_superb(bool s) {
      ms_superb = s;
    }
    static int offsetBorder(TLink*);
    static void makeList(AList<TLink> &layer, AList<TLink> &list, double q, int border, int checkB, TLink* layer0);
    static unsigned findMaxLocalId(unsigned superLayerId);
    static unsigned isIsolation(unsigned localId, unsigned maxLocalId, unsigned layerId, int lr, const AList<TLink> &allStereoList);
    static void findTwoHits(AList<TLink> &twoOnLayer, const AList<TLink> &hitsOnLayer, const AList<TLink> &allStereoList);
    static void setLR(AList<TLink> &hitsOnLayer, unsigned LR = 0);
    static bool moveLR(AList<TLink> &hitsOnLayer);
    static void selectGoodWires(const AList<TLink> &allWires, AList<TLink> &goodWires);
    static void calVirtualCircle(const TLink& hit, const TTrack& track, const int LR, HepGeom::Point3D<double>  &center, double& radius);
    static void moveLR(AList<TLink> &hits, const AList<TLink> &hitsOnLayerOrg, const TTrack& track);

    friend class TRGCDC;


  private:
    //
    void setArcZ(TTrack&, AList<TLink>&) const;
    void setArcZ(TTrack&, AList<TLink>&, AList<TLink>&, AList<TLink>&,
                 unsigned) const;
    void setArcZ(TTrack&, AList<TLink>&, AList<TLink>&, AList<TLink>&,
                 AList<TLink>&,
                 unsigned) const;
    void setArcZ(TTrack&, AList<TLink>&, AList<TLink>&, AList<TLink>&,
                 AList<TLink>&, AList<TLink>&,
                 unsigned) const;
    void setArcZ(TTrack&, AList<TLink>&, AList<TLink>&, AList<TLink>&,
                 AList<TLink>&, AList<TLink>&, AList<TLink>&,
                 unsigned) const;
    void setArcZ(TTrack&, AList<TLink>&, AList<TLink>&, AList<TLink>&,
                 AList<TLink>&, AList<TLink>&, AList<TLink>&, AList<TLink>&,
                 unsigned) const;

    //
    unsigned appendPoints(AList<TLink>&, AList<TLink>&, double, double, TTrack&, double) const;

    //
    void resetTHelixFit(THelixFitter*) const;

    //
    void fitLine(AList<TLink>&, double&, double&, double&, AList<TLink>&, AList<HepGeom::Point3D<double> >&, int&) const;
    void fitLine2(const AList<TLink>&, double&, double&, double&, AList<TLink>&, AList<HepGeom::Point3D<double> >&, int&) const;

    //
    unsigned check(const TTrack&) const;

    //
    bool fitWDD(double& xc, double& yc, double& r,
                AList<TLink> &list) const;
    int stereoHit(double& xc, double& yc, double& r, double& q,
                  AList<TLink> & list) const;

    void makeLine(TTrack&, AList<TLink>&, const AList<TLink>&,
                  AList<TLink>&, double&, double&, double&, AList<HepGeom::Point3D<double> >&) const;
    int sortByLocalId(AList<TLink> &list) const;

    //
#if DEBUG_CURL_GNUPLOT
    void plotArcZ(AList<TLink> &,
                  double = 0.,
                  double = 0.,
                  const int = 1) const;
#endif

  private:
    mutable THelixFitter _fitter;
    TCurlFinderParameter m_param;
//cnv    TSvdFinder * m_svdFinder;
//cnv    TSvdAssociator * m_svdAssociator;
    //
    // for quick dicision
    //
    static bool ms_smallcell;
    static bool ms_superb;
  };

//-----------------------------------------------------------------------------

#ifdef TBuilderCurl_NO_INLINE
#define inline
#else
#undef inline
#define TBuilderCurl_INLINE_DEFINE_HERE
#endif

#ifdef TBuilderCurl_INLINE_DEFINE_HERE

#endif

#undef inline

} // namespace Belle

#endif /* TBuilderCurl_FLAG_ */
