//-----------------------------------------------------------------------------
// $Id: TTrackManager.h 10700 2008-11-06 08:47:27Z hitoshi $
//-----------------------------------------------------------------------------
// Filename : TTrackManager.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A manager of TTrack information to make outputs as Reccdc_trk.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.47  2004/04/08 22:02:51  yiwasaki
// MC quality check added
//
// Revision 1.46  2004/03/26 06:07:27  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.45  2001/12/23 09:58:57  katayama
// removed Strings.h
//
// Revision 1.44  2001/12/19 02:59:56  katayama
// Uss find,istring
//
// Revision 1.43  2001/12/14 02:54:51  katayama
// For gcc-3.0
//
// Revision 1.42  2001/04/11 01:10:05  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.41  2001/02/01 06:15:53  yiwasaki
// Trasan 2.25 : conf bug fix for chisq=0
//
// Revision 1.40  2001/01/31 11:38:30  yiwasaki
// Trasan 2.24 : curl finder fix
//
// Revision 1.39  2001/01/30 20:51:52  yiwasaki
// Trasan 2.22 : bug fixes
//
// Revision 1.38  2001/01/30 04:54:17  yiwasaki
// Trasan 2.21 release : bug fixes
//
// Revision 1.37  2001/01/29 09:27:49  yiwasaki
// Trasan 2.20 release : 2D & cosmic tracks are output, curler decision is made by dr and dz
//
// Revision 1.36  2000/10/11 11:30:47  yiwasaki
// trasan 2.12 : #hits protection, chi2 sorting modified
//
// Revision 1.35  2000/09/29 22:11:25  yiwasaki
// Trasan 2.06 : trkmgr updates from JT
//
// Revision 1.34  2000/09/27 07:45:23  yiwasaki
// Trasan 2.05 : updates of curl and pm finders by JT
//
// Revision 1.33  2000/06/13 05:19:05  yiwasaki
// Trasan 2.02 : RECCDC_MCTRK sorting
//
// Revision 1.32  2000/05/03 11:14:00  yiwasaki
// Trasan 2.00 : version up without any change in trasan, trkmgr rectrk_tof fixed
//
// Revision 1.31  2000/04/29 02:16:22  yiwasaki
// oichan update, trasan unchaged
//
// Revision 1.30  2000/04/15 13:41:02  katayama
// Add/remove const so that they compile
//
// Revision 1.29  2000/04/14 05:20:49  yiwasaki
// Trasan 2.00rc30 : memory leak fixed, multi-track assignment to a hit fixed, helix parameter checks added
//
// Revision 1.28  2000/04/13 02:53:48  yiwasaki
// Trasan 2.00rc29 : minor changes
//
// Revision 1.27  2000/04/07 14:24:40  yiwasaki
// blank output fixed
//
// Revision 1.26  2000/04/07 05:45:20  yiwasaki
// Trasan 2.00rc28 : curl finder bad point rejection, table clear, chisq info
//
// Revision 1.25  2000/04/04 12:14:18  yiwasaki
// Trasan 2.00RC27 : bad point rejection in conf., association check for dE/dx
//
// Revision 1.24  2000/04/04 07:40:11  yiwasaki
// Trasan 2.00RC26 : pm finder update, salvage in conf. finder modified
//
// Revision 1.23  2000/03/24 10:23:04  yiwasaki
// Trasan 2.00RC20 : track manager bug fix
//
// Revision 1.22  2000/03/21 07:01:34  yiwasaki
// tmp updates
//
// Revision 1.21  2000/03/17 11:01:43  yiwasaki
// Trasan 2.00RC16 : updates for new tracking scheme
//
// Revision 1.20  1999/10/30 10:12:55  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.19  1999/08/25 06:25:55  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.18  1999/07/15 08:43:22  yiwasaki
// Trasan 1.55b release : Curl finder # of hits protection, bug in TManager for MC, helix fitter # of hits protection, fast finder improved
//
// Revision 1.17  1999/07/01 08:15:26  yiwasaki
// Trasan 1.51a release : builder bug fix, TRGCDC bug fix again, T0 determination has more parameters
//
// Revision 1.16  1999/06/17 09:45:02  yiwasaki
// Trasan 1.45 release : T0 determination by 2D fitting
//
// Revision 1.15  1999/06/09 15:09:58  yiwasaki
// Trasan 1.38 release : changes for lp
//
// Revision 1.14  1999/05/28 18:18:04  yiwasaki
// Trasan 1.35 release : cathode and sakura updates
//
// Revision 1.13  1999/05/26 05:03:52  yiwasaki
// Trasan 1.34 release : Track merge option added my T.Matsumoto, masking bug fixed, RecCDC_trk.stat is filled
//
// Revision 1.12  1999/05/18 04:44:34  yiwasaki
// Trasan 1.33 release : bugs in salvage and masking are fixed, T0 calculation option is added
//
// Revision 1.11  1999/03/15 07:57:23  yiwasaki
// Trasan 1.27 release : curl finder update
//
// Revision 1.10  1999/03/11 23:27:33  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.9  1999/02/03 06:23:22  yiwasaki
// Trasan 1.14 release : bugs in curl finder and trasan fixed, new salvage installed
//
// Revision 1.8  1999/01/25 03:16:21  yiwasaki
// salvage improved, movePivot problem fixed again
//
// Revision 1.7  1999/01/20 01:02:52  yiwasaki
// Trasan 1.12 release : movePivot problem avoided temporary, new sakura
//
// Revision 1.6  1999/01/11 03:03:32  yiwasaki
// Fitters added
//
// Revision 1.5  1998/11/27 08:15:52  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.4  1998/11/10 09:09:29  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.3  1998/09/25 02:14:46  yiwasaki
// modification for cosmic
//
// Revision 1.2  1998/09/24 22:56:51  yiwasaki
// Trasan 1.1 alpha 2 release
//
// Revision 1.1  1998/09/17 16:05:30  yiwasaki
// Trasan 1.1 alpha 1 release : TTrackManager added to manage reconstructed tracks, TTrack::P() added, TTrack::_charge no longer constant
//
//-----------------------------------------------------------------------------

#ifndef TTRACKMANAGER_FLAG_
#define TTRACKMANAGER_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif

#define HEP_SHORT_NAMES

#include <string>
#include "framework/datastore/StoreArray.h"
#include "GFTrackCand.h"
#include "tracking/modules/trasan/ConstAList.h"
#include "tracking/modules/trasan/TUpdater.h"
#include "tracking/modules/trasan/THelixFitter.h"
//cnv #include "tracking/modules/trasan/TCosmicFitter.h"

struct reccdc_trk;
struct reccdc_trk_add;
struct reccdc_mctrk;
struct rectrk;

namespace Belle2 {
  class TRGCDCWireHit;
}

namespace Belle {

  class TTrack;
  class TProfiler;

/// A manager of TTrack information to make outputs as Reccdc_trk.
  class TTrackManager : public TUpdater {

  public:
    /// Constructor.
    TTrackManager();

    /// Destructor.
    virtual ~TTrackManager();

  public:// to access information
    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// returns a list of all reconstructed tracks including junks.
    const AList<TTrack> & allTracks(void) const;

    /// returns a list of reconstructed tracks.
    const AList<TTrack> & tracks(void) const;

    /// returns a list of 2D tracks.
    const AList<TTrack> & tracks2D(void) const;

    /// returns a list of tracks writen to reccdc_trk.
    const AList<TTrack> & tracksFinal(void) const;

  public:// parameters
    /// sets fitting flag.
    void fittingFlag(unsigned);

    /// sets the max. momentum.
    double maxMomentum(double);

    /// sets the max. pt.
    double minPt(double);

    /// sets the max. tanLambda.
    double maxTanl(double);

    /// returns/sets debug level.
    int debugLevel(void) const;
    int debugLevel(int);

    /// defines histograms used in monitor() function.
    void defineHistograms(void);

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  public:// track manipulations
    /// clears all internal information.
    void clear(bool termination);

    /// checks goodness of a track.
    static bool goodTrack(const TTrack&, bool track2D = false);

    /// appends (2D) tracks. 'list' will be cleaned up.
    void append(AList<TTrack> & list);
    void append2D(AList<TTrack> & list);

    /// refits tracks.
    void refit(void);

    /// finishes tracks.
    void finish(void);

    /// moves pivot of tracks.
    void movePivot(void);

    /// determines T0 and refit all tracks.
    void determineT0(unsigned level, unsigned nMaxTracks);

    /// tests for curlers.
    void setCurlerFlags(void);

    /// sorts tracks.
    void sortTracksByQuality(void);
    void sortTracksByPt(void);

    /// gathers statistical information.
    void statistics(bool doMCAnalysis);

    /// merges tracks if two are very similar.
    void mergeTracks(int level, float threshold);

  public:// table manipulations
    /// clears tables.
    void clearTables(void) const;

    /// stores track info. into Panther table.
    void saveTables(void);

    /// stores MC track info. into Panther table.
    void saveMCTables(void) const;

    /// stores track info. into GFTrackCand.
    void saveBelle2(Belle2::StoreArray<GFTrackCand> &);

    /// sorts RECCDC_TRK tables.
    void sortBanksByPt(void) const;

  private:// table manipulations in private

    /// copies a track. Non-zero will be returned if error happens.
    int copyTrack(TTrack& t,
                  reccdc_trk** r,
                  reccdc_trk_add** a) const;

    /// copies a track. Non-zero will be returned if error happens.
    int copyTrack(Belle2::StoreArray<GFTrackCand> &, TTrack& t) const;

    /// sorts banks.
    void swapReccdc(reccdc_trk& cdc0,
                    reccdc_trk_add& add0,
                    reccdc_mctrk& mc0,
                    reccdc_trk& cdc1,
                    reccdc_trk_add& add1,
                    reccdc_mctrk& mc1) const;
    void swapRectrk(rectrk& trk0, rectrk& tkr1) const;
    void tagReccdc(unsigned* id, unsigned n) const;
    void tagRectrk(unsigned* id, unsigned n) const;

  public:// functions for after trak
    /// final decision for a curler.
    void treatCurler(rectrk& curl, reccdc_trk_add& cdc, unsigned flag) const;

    /// determines IP.
    void determineIP(void);

  public:// hit manipulations
    /// masks hits on found curl tracks.
    void maskCurlHits(const CAList<Belle2::TRGCDCWireHit> & axial,
                      const CAList<Belle2::TRGCDCWireHit> & stereo,
                      const AList<TTrack> & tracks) const;

    /// masks hits with large chisq as associated hits. Pull in TLink is used.
    static void maskBadHits(const AList<TTrack> &, float maxSigma2);

    /// salvages hits for dE/dx(not for track fitting).
    void salvageAssociateHits(const CAList<Belle2::TRGCDCWireHit> &,
                              float maxSigma2);

    /// associates SVD and then adds track information.
    void addSvd(const int) const;

  private:// internal functions
    /// checks track quality.
    AList<TTrack> selectGoodTracks(const AList<TTrack> &,
                                   bool track2D = false) const;
    static bool checkNumberOfHits(const TTrack&, bool track2D = false);

    /// monitors quality of tracks.
    void monitor(void) const;

    /// names tracks.
    void nameTracks(void);

    /// returns T0 by old methode(expensive).
    float T0(unsigned n);

    /// returns T0 by T0 fitting.
    float T0Fit(unsigned n);
    float minimum(float y0, float y1, float y2) const;

  public:// obsolete
    TLink& divide(const TTrack& t, AList<TLink> * l) const;
    TLink& divideByIp(const TTrack& t, AList<TLink> * l) const;
    void removeHitsAcrossOverIp(AList<TLink> &) const;
    /// returns a track which is the closest to a hit.
    TTrack* closest(const AList<TTrack> &, const Belle2::TRGCDCWireHit&) const;
    /// salvages remaining hits.
    void salvage(const CAList<Belle2::TRGCDCWireHit> &) const;
    /// masks hits out which are in tail of curly tracks.
    void mask(void) const;
    void maskNormal(TTrack&) const;
    void maskCurl(TTrack&) const;
    void maskOut(TTrack&, const AList<TLink> &) const;
    void maskMultiHits(TTrack&) const;
    void merge(void);

  private:
    int _debugLevel;
    double _maxMomentum;
    double _minPt;
    double _maxTanl;
    double _sigmaCurlerMergeTest;
    unsigned _nCurlerMergeTest;

    THelixFitter _fitter;
    THelixFitter _cFitter;

    AList<TTrack> _tracksAll;   // all tracks found by the finders
    AList<TTrack> _tracks;      // good 3D tracks determined by the mgr
    AList<TTrack> _tracks2D;    // good 2D tracks determined by the mgr
    AList<TTrack> _tracksFinal; // final tracks output to tables

    AList<TLink> _associateHits;

    struct summary {
      unsigned _nEvents;
      unsigned _nTracks[8];
      unsigned _nTracksAll[8];
      unsigned _nTracks2D[8];
      unsigned _nTracksFinal[8];
      unsigned _nSuperMoms[8];
      unsigned _nPtCut[8];
      unsigned _nTanlCut[8];
      unsigned _nToBeMerged;
      unsigned _nToBeMergedMoreThanTwo;
      unsigned _nMCQuality[8][5];
    };
    struct summary* _s;

    TProfiler* _profiler[4];
  };

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define TTRACKMANAGER_INLINE_DEFINE_HERE
#endif

#ifdef TTRACKMANAGER_INLINE_DEFINE_HERE

  inline
  std::string
  TTrackManager::name(void) const
  {
    return std::string("Track Manager");
  }

  inline
  const AList<TTrack> &
  TTrackManager::tracks(void) const
  {
    return _tracks;
  }

  inline
  const AList<TTrack> &
  TTrackManager::tracks2D(void) const
  {
    return _tracks2D;
  }

  inline
  const AList<TTrack> &
  TTrackManager::allTracks(void) const
  {
    return _tracksAll;
  }

  inline
  double
  TTrackManager::maxMomentum(double a)
  {
    return _maxMomentum = a;
  }

  inline
  double
  TTrackManager::minPt(double a)
  {
    return _minPt = a;
  }

  inline
  double
  TTrackManager::maxTanl(double a)
  {
    return _maxTanl = a;
  }

  inline
  int
  TTrackManager::debugLevel(void) const
  {
    return _debugLevel;
  }

  inline
  int
  TTrackManager::debugLevel(int a)
  {
    return _debugLevel = a;
  }

  inline
  void
  TTrackManager::fittingFlag(unsigned a)
  {
    if (a & 1) _fitter.sag(true);
    if (a & 2) _fitter.propagation(true);
    if (a & 4) _fitter.tof(true);
    if (a & 8) _fitter.freeT0(true);
  }

  inline
  const AList<TTrack> &
  TTrackManager::tracksFinal(void) const
  {
    return _tracksFinal;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TTRACKMANAGER_FLAG_ */
