//-----------------------------------------------------------------------------
// $Id: TTrack.h 10677 2008-10-06 07:48:09Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TTrack.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track in tracking.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.83  2004/04/08 22:02:51  yiwasaki
// MC quality check added
//
// Revision 1.82  2004/03/26 06:07:27  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.81  2002/01/03 11:04:59  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.80  2001/12/23 09:58:57  katayama
// removed Strings.h
//
// Revision 1.79  2001/12/19 02:59:56  katayama
// Uss find,istring
//
// Revision 1.78  2001/12/14 02:54:51  katayama
// For gcc-3.0
//
// Revision 1.77  2001/04/11 01:10:04  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.76  2001/04/04 00:56:38  katayama
// Fixes for CC and gcc2.95.3
//
// Revision 1.75  2001/02/16 00:47:05  yiwasaki
// Trasan 2.29 : mc info for associate hits added, mc info. bug fix
//
// Revision 1.74  2001/01/31 11:38:29  yiwasaki
// Trasan 2.24 : curl finder fix
//
// Revision 1.73  2001/01/30 20:51:52  yiwasaki
// Trasan 2.22 : bug fixes
//
// Revision 1.72  2001/01/30 04:54:17  yiwasaki
// Trasan 2.21 release : bug fixes
//
// Revision 1.71  2001/01/29 09:27:48  yiwasaki
// Trasan 2.20 release : 2D & cosmic tracks are output, curler decision is made by dr and dz
//
// Revision 1.70  2000/10/11 11:30:46  yiwasaki
// trasan 2.12 : #hits protection, chi2 sorting modified
//
// Revision 1.69  2000/10/07 01:44:54  yiwasaki
// Trasan 2.10 : updates of trkmgr from JT
//
// Revision 1.68  2000/10/05 23:54:32  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.67  2000/09/27 07:45:23  yiwasaki
// Trasan 2.05 : updates of curl and pm finders by JT
//
// Revision 1.66  2000/08/31 23:51:51  yiwasaki
// Trasan 2.04 : pefect finder added
//
// Revision 1.65  2000/07/31 04:47:34  yiwasaki
// Minor changes
//
// Revision 1.64  2000/04/25 02:53:28  yiwasaki
// Trasan 2.00rc31 : Definition of nhits, nster, and ndf in RECCDC_WIRHIT
//
// Revision 1.63  2000/04/13 02:53:47  yiwasaki
// Trasan 2.00rc29 : minor changes
//
// Revision 1.62  2000/04/07 14:24:40  yiwasaki
// blank output fixed
//
// Revision 1.61  2000/04/04 20:52:26  katayama
// md used twice
//
// Revision 1.60  2000/04/04 07:40:10  yiwasaki
// Trasan 2.00RC26 : pm finder update, salvage in conf. finder modified
//
// Revision 1.59  2000/03/30 08:30:36  katayama
// mods for CC5.0
//
// Revision 1.58  2000/03/30 06:40:08  yiwasaki
// debug info. added
//
// Revision 1.57  2000/03/28 22:57:15  yiwasaki
// Trasan 2.00RC22 : PM finder from J.Tanaka
//
// Revision 1.56  2000/03/24 10:23:03  yiwasaki
// Trasan 2.00RC20 : track manager bug fix
//
// Revision 1.55  2000/03/23 13:27:57  yiwasaki
// Trasan 2.00RC18 : bug fixes
//
// Revision 1.54  2000/03/21 11:55:20  yiwasaki
// Trasan 2.00RC17 : curler treatments
//
// Revision 1.53  2000/03/21 07:01:33  yiwasaki
// tmp updates
//
// Revision 1.52  2000/03/17 11:01:43  yiwasaki
// Trasan 2.00RC16 : updates for new tracking scheme
//
// Revision 1.51  2000/03/17 07:00:52  katayama
// Module function modified
//
// Revision 1.50  2000/02/28 01:59:03  yiwasaki
// Trasan 2.00RC11 : curl updates only
//
// Revision 1.49  2000/02/25 08:10:01  yiwasaki
// Trasan 2.00RC9 : stereo bug fix
//
// Revision 1.48  2000/02/24 06:19:31  yiwasaki
// Trasan 2.00RC7 : bug fix again
//
// Revision 1.47  2000/02/15 13:46:52  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.46  2000/02/09 03:27:42  yiwasaki
// Trasan 1.68l : curl and new conf stereo updated
//
// Revision 1.45  2000/02/01 11:24:47  yiwasaki
// Trasan 1.68j : curl finder modified, new stereo finder modified
//
// Revision 1.44  1999/11/19 09:13:16  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.43  1999/10/30 10:12:52  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.42  1999/08/25 06:25:54  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.41  1999/07/09 01:47:25  yiwasaki
// Trasan 1.53a release : cathode updates by T.Matsumoto, minor change of Conformal finder
//
// Revision 1.40  1999/06/16 08:29:56  yiwasaki
// Trasan 1.44 release : new THelixFitter
//
// Revision 1.39  1999/06/14 05:51:08  yiwasaki
// Trasan 1.41 release : curl finder updates
//
// Revision 1.38  1999/06/09 15:09:57  yiwasaki
// Trasan 1.38 release : changes for lp
//
// Revision 1.37  1999/05/28 07:11:22  yiwasaki
// Trasan 1.35 alpha release : cathdoe test version
//
// Revision 1.36  1999/03/21 15:45:51  yiwasaki
// Trasan 1.28 release : TrackManager bug fix, CosmicFitter added in BuilderCosmic, parameter salvage level added
//
// Revision 1.35  1999/03/11 23:27:30  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.34  1999/03/10 12:55:16  yiwasaki
// Trasan 1.23 release : curl finder updated
//
// Revision 1.33  1999/02/09 06:24:02  yiwasaki
// Trasan 1.17 release : cathode codes updated by T.Matsumoto, FPE error fixed by J.Tanaka
//
// Revision 1.32  1999/02/03 06:23:21  yiwasaki
// Trasan 1.14 release : bugs in curl finder and trasan fixed, new salvage installed
//
// Revision 1.31  1999/01/25 03:16:19  yiwasaki
// salvage improved, movePivot problem fixed again
//
// Revision 1.30  1999/01/11 03:03:30  yiwasaki
// Fitters added
//
// Revision 1.29  1998/12/26 03:43:10  yiwasaki
// cathode updates from S.Suzuki
//
// Revision 1.28  1998/11/27 08:15:48  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.27  1998/11/12 12:27:41  yiwasaki
// Trasan 1.1 RC 1 release : salvaging installed, basf_if/refit.cc added
//
// Revision 1.26  1998/10/09 03:01:11  yiwasaki
// Trasan 1.1 beta 4 release : memory leak stopped, and minor changes
//
// Revision 1.25  1998/09/29 01:24:35  yiwasaki
// Trasan 1.1 beta 1 relase : TBuilderCurl added
//
// Revision 1.24  1998/09/28 16:52:15  yiwasaki
// TBuilderCosmic added
//
// Revision 1.23  1998/09/28 16:11:17  yiwasaki
// fitter with cathode added
//
// Revision 1.22  1998/09/25 02:14:45  yiwasaki
// modification for cosmic
//
// Revision 1.21  1998/09/17 16:05:28  yiwasaki
// Trasan 1.1 alpha 1 release : TTrackManager added to manage reconstructed tracks, TTrack::P() added, TTrack::_charge no longer constant
//
// Revision 1.20  1998/08/31 05:16:05  yiwasaki
// Trasan 1.09 release : curl finder updated by J.Tanaka, MC classes updated by Y.Iwasaki
//
// Revision 1.19  1998/08/12 16:33:04  yiwasaki
// Trasan 1.08 release : stereo finder updated by J.Suzuki, new MC classes added by Y.Iwasaki
//
// Revision 1.18  1998/08/03 15:01:17  yiwasaki
// Trasan 1.07 release : cluster finder from S.Suzuki-san added
//
// Revision 1.17  1998/07/29 04:35:27  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.14  1998/07/06 15:48:58  yiwasaki
// Trasan 1.01 release:CurlFinder default on, bugs in TLine and TTrack::fit fixed
//
// Revision 1.13  1998/06/21 18:38:27  yiwasaki
// Trasan 1 beta 5 release, rphi improved?
//
// Revision 1.12  1998/06/15 03:34:21  yiwasaki
// Trasan 1 beta 3.1, compiler error fixed
//
// Revision 1.11  1998/06/14 11:09:59  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
// Revision 1.10  1998/06/11 08:15:48  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.9  1998/06/08 14:39:22  yiwasaki
// Trasan 1 alpha 8 release, Stereo append bug fixed, TCurlFinder added
//
// Revision 1.8  1998/06/03 17:16:56  yiwasaki
// const added to TRGCDC::hits,axialHits,stereoHits,hitsMC, symbols WireHitNeghborHit* added in TRGCDCWireHit, TCluster::innerWidth,outerWidth,innerMostLayer,outerMostLayer,type,split,split2,widht,outer,updateType added, TLink::conf added, TTrack::appendStereo3,refineStereo2,aa,bb,Zchisqr added
//
// Revision 1.7  1998/05/26 05:09:18  yiwasaki
// cvs repair
//
// Revision 1.6  1998/05/24 15:00:08  yiwasaki
// Trasan 1 alpha 5 release, pivot is moved to the first hit
//
// Revision 1.5  1998/05/11 10:16:58  yiwasaki
// TTrack::assign -> TTrack::assert, WireHitUsedMask is set in TRGCDCWireHit
//
// Revision 1.4  1998/05/08 09:47:07  yiwasaki
// Trasan 1 alpha 2 relase, stereo recon. added, off-vtx recon. added
//
// Revision 1.3  1998/04/23 17:25:07  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.2  1998/04/16 16:51:10  yiwasaki
// minor changes
//
// Revision 1.1  1998/04/10 09:38:21  yiwasaki
// TTrack added, TRGCDC becomes Singleton
//
// Revision 1.1  1998/04/10 00:51:13  yiwasaki
// TTrack, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
//-----------------------------------------------------------------------------

#ifndef TTrack_FLAG_
#define TTrack_FLAG_
#define TTrackHEP_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif

//...Definition of finder/manager...
//   Use FinderName() to obtain a string of finder name.
#define TrackOldConformalFinder 1
#define TrackAnyFinder          1
#define TrackFastFinder         2
#define TrackSlowFinder         4
#define TrackCurlFinder         8
#define TrackTrackManager      16
#define TrackPMCurlFinder      32
#define TrackSVDAssociator     64
#define TrackHoughFinder      128
#define TrackFinderMask       255

//...Definition of type/kind...
#define TrackTypeUndefined        0
#define TrackTypeNormal           1
#define TrackTypeCurl             2
#define TrackTypeCircle           4
#define TrackTypeCosmic           8
#define TrackTypeIncomingCosmic   8
#define TrackTypeOutgoingCosmic  16
#define TrackTypeKink            32
#define TrackTypeSVDOnly        256

//...Definition of quality...
#define TrackQualityOutsideCurler   1
#define TrackQualityAfterKink       2
#define TrackQualityCosmic          4
#define TrackQuality2D              8
#define TrackQualityShift           8
#define TrackQualityMask          255

//...Definition of fitting status...
#define TrackFitGlobal         1
#define TrackFitCosmic         2
#define TrackFitCdcKalman      4
#define TrackFitSvdCdcKalman   8
#define TrackFitShift         16
#define TrackFitMask         255

//...Definition of relation...
#define TrackHasMother         1
#define TrackHasDaughter       2
#define TrackRelationShift    24
#define TrackRelationMask    255


#include <string>

#define HEP_SHORT_NAMES
#include "CLHEP/Vector/ThreeVector.h"
#include "tracking/modules/trasan/THelix.h"
#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TBuilder0.h"
#include "tracking/modules/trasan/TBuilderCosmic.h"
#include "tracking/modules/trasan/TBuilderCurl.h"
//cnv #include "trg/cdc/CatHit.h"
#include "tracking/modules/trasan/TPoint2D.h"
#include "tracking/modules/trasan/TSvdHit.h"

struct reccdc_trk;
struct reccdc_trk_add;
struct rectrk_localz;
// struct gen_hepevt;
struct mdst_trk_fit;

namespace Belle {


  class Trasan;
  class TTrackManager;
  class TCircle;
  class TCosmicFitter;
  class Refit;
  class THelixFitter;
  class TBuilder;
  class TPMCurlFinder;
  class TCurlFinder;


/// A class to represent a track in tracking.
  class TTrack : public TTrackBase {

  public:
    /// Default constructor
    TTrack();

    /// Constructor
    TTrack(const TCircle&);

    /// Copy constructor
    TTrack(const TTrack&);

    /// Constructor
    TTrack(const THelix&);

    /// Destructor
    virtual ~TTrack();

  public:// General information
    /// returns/sets name.
    const std::string& name(void) const;
    const std::string& name(const std::string& newName);

    /// sets/returns mother/daughter.
    TTrack* mother(void) const;
    TTrack* mother(TTrack*);
    TTrack* daughter(void) const;
    TTrack* daughter(TTrack*);

    /// returns type.
    unsigned objectType(void) const;

    /// returns type. Definition is depending on an object type.
    unsigned type(void) const;

    /// sets/returns finder.
    unsigned finder(void) const;
    unsigned finder(unsigned finderMask);

    /// sets/returns quality.
    unsigned quality(void) const;
    unsigned quality(unsigned qualityMask);

    /// sets/returns fitting status.
    unsigned fitting(void) const;
    unsigned fitting(unsigned fitMask);

    /// returns/sets internal state.(for bank output)
    unsigned state(void) const;

    /// assigns wire hits to this track.
    void assign(unsigned maskForWireHit);

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  public:// Kinematics
    /// returns charge.
    double charge(void) const;

    /// returns helix parameter.
    const THelix& helix(void) const;

    /// returns momentum.
    CLHEP::Hep3Vector p(void) const;

    /// returns magnitude of momentum.
    double ptot(void) const;

    /// returns Pt.
    double pt(void) const;

    /// returns Pz.
    double pz(void) const;

    /// returns signed impact parameter to the origin.
    double impact(void) const;

    /// returns position of helix center.
    TPoint2D center(void) const;

    /// returns signed radius.
    double radius(void) const;

    /// returns NDF.
    unsigned ndf(void) const;

    /// returns chi2.
    double chi2(void) const;

    /// returns confidence level.
    double confidenceLevel(void) const;

  public:// Utilities for segment operations.
    /// returns AList<TSegment>.
    AList<TSegment> & segments(void);
    const AList<TSegment> & segments(void) const;

  public:// Utilities
    /// calculates the closest approach to a wire in real space. Results are stored in TLink. Return value is negative if error happened.
    int approach(TLink&) const;
    int approach(TLink&, bool sagCorrection) const;
    int approach2D(TLink&) const;

    /// calculates arc length and z for a stereo hit.
    int szPosition(TLink& link) const;

    /// calculates arc length and z for a segment. Results are stored in TLink.
    int szPosition(const TSegment& segment, TLink& link) const;

    /// calculates arc length for a point.
    int szPosition(const HepGeom::Point3D<double>  & p, HepGeom::Point3D<double>  & szPosition) const;

    /// calculates arc length and z for a stereo hit. uses these functions for curl tracks.
#define OLD_STEREO 1
#if OLD_STEREO
    int stereoHitForCurl(TLink& link, AList<HepGeom::Point3D<double> > & arcZList) const;
    int stereoHitForCurl(TLink& link, TLink& link1) const;
    int stereoHitForCurl(TLink& link, TLink& link1, TLink& link2) const;
    void deleteListForCurl(AList<HepGeom::Point3D<double> > &l1,
                           AList<HepGeom::Point3D<double> > &l2) const;
    void deleteListForCurl(AList<HepGeom::Point3D<double> > &l1,
                           AList<HepGeom::Point3D<double> > &l2,
                           AList<HepGeom::Point3D<double> > &l3) const;
#endif
    /// calculates arc length and z for a stereo hit. uses these functions for curl tracks(included svd version).
    int stereoHitForCurl(AList<TLink>&) const;
    //int svdHitForCurl(AList<TSvdHit>&) const;

  public:// Modifiers
    /// sets charge.
    double charge(double);

    /// fits itself. Error was happened if return value is not zero.
    // int fitx(void);

    /// fits itself with r-phi view. Error was happened if return value is not zero.
    int fit2D(unsigned = 0, double = 0.1, double = 0.015);

    /// fits itself with cathode hits.
    int fitWithCathode(float windowSize = 0.6 , int SysCorr = 0);   // mod by matsu ( 1999/07/05 )

    /// remove bad points by chi2. Bad points are returned in a 'list'. fit() should be called before calling this function. (using stereo wire as axial wire(z=0))
    void refine2D(AList<TLink> & list, float maxSigma);

    /// moves pivot to the inner most hit.
    void movePivot(void);

  public:// Static utility functions


  public:// temporary added
    /// returns a cathode hit list.
//cnv    const AList<TRGCDCCatHit> & catHits(void) const;

    /// calculates an intersection of this track and a cylinder.
    int HelCyl(double rhole,
               double rcyl,
               double zb,
               double zf,
               double epsl,
               double& phi,
               HepGeom::Point3D<double>  & xp) const;

    /// finds cathode hits associated to this track.
    void findCatHit(unsigned trackid);

    /// relation between cluster and wire
    void relationClusterWithWire();

    /// relation between cluster and layer
    void relationClusterWithLayer(int SysCorr);  // by matsu ( 1999/07/05 )

    /// appends Svd Hits(TSvdHit)
    void svdHits(AList<TSvdHit> &);

    /// returns Svd Hits List
    AList<TSvdHit> & svdHits(void);
    const AList<TSvdHit> & svdHits(void) const;

    /// sets/returns a list of TLink which are used for table output.
    const AList<TLink> & finalHits(void) const;
    const AList<TLink> & finalHits(const AList<TLink> & hits);

    /// sets/returns a list of associated TLink which are used for table output.
    const AList<TLink> & associateHits(void) const;
    const AList<TLink> & associateHits(const AList<TLink> & hits);

  public:// Static utility functions
    /// Sorter
#if defined(__GNUG__)
    static int sortByPt(const TTrack** a, const TTrack** b);
#else
//    extern "C" int
//        SortByPt(const void* a, const void* b);
#endif

    /// returns the finder name.
    static std::string finderName(unsigned);

    /// to dump a track.
    static std::string trackDump(const TTrack&);
    static void dump(const AList<TTrack> &,
//         const std::string & message,
//         const std::string & prefix);
                     const std::string& message = std::string(""),
                     const std::string& prefix = std::string(""));

    /// returns string of track type.
    static std::string trackType(const TTrack&);
    static std::string trackType(unsigned type);

    /// returns string of track status.
    static std::string trackStatus(const TTrack&);
    static std::string trackStatus(const reccdc_trk&);
    static std::string trackStatus(const reccdc_trk_add&);
    static std::string trackStatus(unsigned md,
                                   unsigned mk,
                                   unsigned mq,
                                   unsigned ms,
                                   unsigned mm,
                                   unsigned ma);

    /// returns track information
    static std::string trackKinematics(const TTrack&);
    static std::string trackKinematics(const THelix&);
    static std::string trackInformation(const TTrack&);
    static std::string trackInformation(const reccdc_trk&);
    static std::string trackInformation(unsigned nA,
                                        unsigned nS,
                                        unsigned n,
                                        float chisq);
    static std::string trackLayerUsage(const TTrack&);

    /// returns helix.
    static THelix track2THelix(const reccdc_trk&);
    static THelix track2THelix(const rectrk_localz&);
//cnv    static THelix track2THelix(const gen_hepevt &);
    static THelix track2THelix(const mdst_trk_fit&);

    /// THelix parameter validity
    static bool helixHasNan(const THelix&);

    /// Error matrix validity
    static bool positiveDefinite(const THelix&);

    /// calculates distance between two tracks.
    static float distance(const TTrack&, const TTrack&);
    static float distanceB(const TTrack&, const TTrack&);

    static double chisq2confLevel(int, double);
    static int intersection(const HepGeom::Point3D<double>  & c1,
                            double r1,
                            const HepGeom::Point3D<double>  & c2,
                            double r2,
                            double eps,
                            HepGeom::Point3D<double>  & x1,
                            HepGeom::Point3D<double>  & x2);

  private:
    /// calculates dXda. 'link' and 'dPhi' are inputs. Others are outputs.
    int dxda(const TLink& link,
             double dPhi,
             CLHEP::HepVector& dxda,
             CLHEP::HepVector& dyda,
             CLHEP::HepVector& dzda) const;

    int dxda2D(const TLink& link,
               double dPhi,
               CLHEP::HepVector& dxda,
               CLHEP::HepVector& dyda,
               CLHEP::HepVector& dzda) const;

    int dxda2D(const TLink& link,
               double dPhi,
               CLHEP::HepVector& dxda,
               CLHEP::HepVector& dyda) const;

    int dxda2D(double dPhi,
               CLHEP::HepVector& dxda,
               CLHEP::HepVector& dyda) const;

    unsigned defineType(void) const;

  private:// Always updated
    unsigned _state;
    double _charge;
    AList<TSegment> _segments;
    TTrack* _mother;
    TTrack* _daughter;
    AList<TSvdHit> _svdHits;
    AList<TLink> _finalHits;
    AList<TLink> _associateHits;

  private:// Updated when fitted
    static const THelixFitter _fitter;
    THelix* const _helix;
    unsigned _ndf;
    double _chi2;
    std::string _name;
    mutable unsigned _type;
    static TPoint2D _points0[100];
    static TPoint2D _points1[100];

  private:// temporary added
//cnv    AList<TRGCDCCatHit> _catHits;

    friend class TTrackManager;
    friend class Trasan;
    friend class THelixFitter;
    friend class TCosmicFitter;
    friend class Refit;
    friend class TBuilder;
    friend class TBuilderConformal;
    friend class TBuilder0;
    friend class TBuilderCosmic;
    friend class TBuilderCurl;
    friend class TPMCurlFinder;
    friend class TCurlFinder;

#ifdef TRASAN_DEBUG
  public:
    static unsigned nTTracks(void);
    static unsigned nTTracksMax(void);
  private:
    static unsigned _nTTracks;
    static unsigned _nTTracksMax;
#endif
  };

//-----------------------------------------------------------------------------

#ifdef TTrack_NO_INLINE
#define inline
#else
#undef inline
#define TTrack_INLINE_DEFINE_HERE
#endif

#ifdef TTrack_INLINE_DEFINE_HERE

  inline
  const THelix&
  TTrack::helix(void) const
  {
#ifdef TRASAN_DEBUG
    // if (! _fitted) std::cout << "TTrack::helix !!! helix not updated" << std::endl;
#endif
    return * _helix;
  }

  inline
  unsigned
  TTrack::ndf(void) const
  {
#ifdef TRASAN_DEBUG
    if (! _fitted) std::cout << "TTrack::ndf !!! ndf not updated" << std::endl;
#endif
    return _ndf;
  }

  inline
  double
  TTrack::chi2(void) const
  {
#ifdef TRASAN_DEBUG
    if (! _fitted) std::cout << "TTrack::chi2 !!! chi2 not updated" << std::endl;
#endif
    return _chi2;
  }

  inline
  double
  TTrack::charge(void) const
  {
    return _charge;
  }

  inline
  double
  TTrack::charge(double a)
  {
    return _charge = a;
  }

//cnv inline
// const AList<TRGCDCCatHit> &
// TTrack::catHits(void) const {
//     return _catHits;
// }

  inline
  const std::string&
  TTrack::name(void) const
  {
    return _name;
  }

  inline
  const std::string&
  TTrack::name(const std::string& a)
  {
//  return _name = a + "[" + _name + "]";
    return _name = a;
  }

  inline
  double
  TTrack::pt(void) const
  {
    return 1. / fabs(_helix->a()[2]);
  }

  inline
  double
  TTrack::pz(void) const
  {
    return (1. / fabs(_helix->a()[2])) * _helix->a()[4];
  }

  inline
  double
  TTrack::ptot(void) const
  {
    return (1. / fabs(_helix->a()[2])) *
           sqrt(1. + _helix->a()[4] * _helix->a()[4]);
  }

  inline
  double
  TTrack::confidenceLevel(void) const
  {
    return chisq2confLevel((int) _ndf, _chi2);
  }

  inline
  CLHEP::Hep3Vector
  TTrack::p(void) const
  {
    return _helix->momentum(0.);
  }

  inline
  unsigned
  TTrack::objectType(void) const
  {
    return Track;
  }

  inline
  unsigned
  TTrack::type(void) const
  {
    return defineType();
  }

  inline
  double
  TTrack::impact(void) const
  {
    return fabs(_helix->radius()) - _helix->center().mag();
  }

  inline
  double
  TTrack::radius(void) const
  {
    return _helix->radius();
  }

  inline
  AList<TSegment> &
  TTrack::segments(void)
  {
    return _segments;
  }

  inline
  const AList<TSegment> &
  TTrack::segments(void) const
  {
    return _segments;
  }

  inline
  TPoint2D
  TTrack::center(void) const
  {
    return TPoint2D(_helix->center());
  }

  inline
  unsigned
  TTrack::finder(void) const
  {
    return _state & TrackFinderMask;
  }

  inline
  unsigned
  TTrack::finder(unsigned a)
  {
    _state |= (a & TrackFinderMask);
    return finder();
  }

  inline
  unsigned
  TTrack::quality(void) const
  {
    return (_state >> TrackQualityShift) & TrackQualityMask;
  }

  inline
  unsigned
  TTrack::quality(unsigned a)
  {
//      _state = ((a << TrackQualityShift) & TrackQualityMask) |
//    (_state & (~ (TrackQualityMask << TrackQualityShift)));
    _state = ((a & TrackQualityMask) << TrackQualityShift) |
             (_state & (~(TrackQualityMask << TrackQualityShift)));
    return quality();
  }

  inline
  unsigned
  TTrack::fitting(void) const
  {
    return (_state >> TrackFitShift) & TrackFitMask;
  }

  inline
  unsigned
  TTrack::fitting(unsigned a)
  {
    _state |= ((a << TrackFitShift) & TrackFitMask);
    return fitting();
  }

  inline
  TTrack*
  TTrack::mother(void) const
  {
    return _mother;
  }

  inline
  TTrack*
  TTrack::mother(TTrack* a)
  {
    if (a) _state |= (TrackHasMother << TrackRelationShift);
    else   _state &= (~(TrackHasMother << TrackRelationShift));
    return _mother = a;
  }

  inline
  TTrack*
  TTrack::daughter(void) const
  {
    return _daughter;
  }

  inline
  TTrack*
  TTrack::daughter(TTrack* a)
  {
    if (a) _state |= (TrackHasDaughter << TrackRelationShift);
    else   _state &= (~(TrackHasDaughter << TrackRelationShift));
    return _daughter = a;
  }

  inline
  unsigned
  TTrack::state(void) const
  {
    return _state;
  }

  inline
  void
  TTrack::svdHits(AList<TSvdHit> &list)
  {
    _svdHits.append(list);
  }

  inline
  AList<TSvdHit> &
  TTrack::svdHits(void)
  {
    return _svdHits;
  }

  inline
  const AList<TSvdHit> &
  TTrack::svdHits(void) const
  {
    return _svdHits;
  }

  inline
  std::string
  TTrack::trackType(const TTrack& t)
  {
    return trackType(t.type());
  }

  inline
  std::string
  TTrack::trackKinematics(const TTrack& t)
  {
    return trackKinematics(t.helix());
  }

  inline
  std::string
  TTrack::trackDump(const TTrack& t)
  {
    return t.name() + " " + trackStatus(t) + " " + trackKinematics(t) + " " +
           trackInformation(t);
  }

  inline
  const AList<TLink> &
  TTrack::finalHits(const AList<TLink> & list)
  {
    _finalHits = list;
    return _finalHits;
  }

  inline
  const AList<TLink> &
  TTrack::finalHits(void) const
  {
    return _finalHits;
  }

  inline
  const AList<TLink> &
  TTrack::associateHits(const AList<TLink> & list)
  {
    _associateHits = list;
    return _associateHits;
  }

  inline
  const AList<TLink> &
  TTrack::associateHits(void) const
  {
    return _associateHits;
  }

  inline
  std::string
  TTrack::finderName(unsigned i)
  {
    if (i & TrackHoughFinder) return "Hough Finder";
    else if (i & TrackSVDAssociator) return "SVD Associator";
    else if (i & TrackPMCurlFinder) return "Pattern Matching Curl Finder";
    else if (i & TrackTrackManager) return "Track Manager";
    else if (i & TrackCurlFinder) return "Curl Finder";
    else if (i & TrackSlowFinder) return "Conformal Slow Finder";
    else if (i & TrackFastFinder) return "Conformal Fast Finder";
    return "All Finders(old definition is Unknown Finder)";
  }

  inline
  void
  TTrack::dump(const AList<TTrack> & list,
               const std::string& message,
               const std::string& prefix)
  {
    TTrackBase::dump((const AList<TTrackBase> &) list, message, prefix);
  }

#endif

#undef inline

} // namespace Belle

#endif /* TTrack_FLAG_ */
