//-----------------------------------------------------------------------------
// $Id: TTrackBase.h 10019 2007-03-02 05:04:42Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TTrackBase.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A virtual class for a track class in tracking.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.47  2004/03/26 06:07:27  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.46  2002/01/03 11:04:59  katayama
// HepGeom::Point3D<double> and other header files are cleaned
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
// Revision 1.42  2001/04/11 01:10:04  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.41  2000/04/13 02:53:47  yiwasaki
// Trasan 2.00rc29 : minor changes
//
// Revision 1.40  2000/04/11 13:05:57  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.39  2000/04/04 07:52:39  yiwasaki
// Trasan 2.00RC26 : additions
//
// Revision 1.38  2000/04/04 07:40:11  yiwasaki
// Trasan 2.00RC26 : pm finder update, salvage in conf. finder modified
//
// Revision 1.37  2000/03/21 11:55:20  yiwasaki
// Trasan 2.00RC17 : curler treatments
//
// Revision 1.36  2000/03/21 07:01:33  yiwasaki
// tmp updates
//
// Revision 1.35  2000/03/17 11:01:43  yiwasaki
// Trasan 2.00RC16 : updates for new tracking scheme
//
// Revision 1.34  1999/10/30 10:12:53  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.33  1999/10/21 15:45:20  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
// Revision 1.32  1999/08/25 06:25:55  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.31  1999/06/29 00:03:05  yiwasaki
// Trasan 1.48a release : TFastFinder added
//
// Revision 1.30  1999/06/17 01:39:19  yiwasaki
// Trasan 1.441 release : default mom. cut = 0. to recon. cosmics
//
// Revision 1.29  1999/06/10 09:44:55  yiwasaki
// Trasan 1.40 release : minor changes only
//
// Revision 1.28  1999/05/26 05:03:51  yiwasaki
// Trasan 1.34 release : Track merge option added my T.Matsumoto, masking bug fixed, RecCDC_trk.stat is filled
//
// Revision 1.27  1999/04/10 03:49:27  katayama
// Ask to be a friend
//
// Revision 1.26  1999/04/07 06:14:13  yiwasaki
// Trasan 1.30 release : curl finder mask updated
//
// Revision 1.25  1999/03/11 23:27:32  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.24  1999/01/11 03:03:31  yiwasaki
// Fitters added
//
// Revision 1.23  1998/12/14 16:59:13  yiwasaki
// bad hits removed from outputs
//
// Revision 1.22  1998/11/27 08:15:49  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.21  1998/11/12 12:27:42  yiwasaki
// Trasan 1.1 RC 1 release : salvaging installed, basf_if/refit.cc added
//
// Revision 1.20  1998/11/10 09:09:28  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.19  1998/10/09 03:01:13  yiwasaki
// Trasan 1.1 beta 4 release : memory leak stopped, and minor changes
//
// Revision 1.18  1998/09/28 16:11:18  yiwasaki
// fitter with cathode added
//
// Revision 1.17  1998/09/28 14:54:19  yiwasaki
// MC tables, TUpdater, oichan added
//
// Revision 1.16  1998/09/24 22:56:49  yiwasaki
// Trasan 1.1 alpha 2 release
//
// Revision 1.15  1998/08/31 05:16:06  yiwasaki
// Trasan 1.09 release : curl finder updated by J.Tanaka, MC classes updated by Y.Iwasaki
//
// Revision 1.14  1998/07/29 04:35:28  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.10  1998/06/17 20:23:06  yiwasaki
// Trasan 1 beta 4 release again, KS effi. improved?
//
// Revision 1.9  1998/06/17 20:12:43  yiwasaki
// Trasan 1 beta 4 release, KS effi. improved?
//
// Revision 1.8  1998/06/14 11:10:00  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
// Revision 1.7  1998/06/11 08:15:48  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.6  1998/05/22 08:21:53  yiwasaki
// Trasan 1 alpha 4 release, TCluster added, TConformalLink no longer used
//
// Revision 1.5  1998/05/08 09:47:07  yiwasaki
// Trasan 1 alpha 2 relase, stereo recon. added, off-vtx recon. added
//
// Revision 1.4  1998/04/23 17:25:08  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.3  1998/04/16 16:51:10  yiwasaki
// minor changes
//
// Revision 1.2  1998/04/10 09:38:20  yiwasaki
// TTrack added, TRGCDC becomes Singleton
//
// Revision 1.1  1998/04/10 00:51:16  yiwasaki
// TCircle, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
//-----------------------------------------------------------------------------

#ifndef TTrackBase_FLAG_
#define TTrackBase_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif

#define TrackAxialLayer 0
#define TrackStereoLayer 1
#define TrackBothLayer 2

//...Definition of objectType...
#define TrackBase            1
#define Circle               2
#define Line                 4
#define Track                8
#define Segment             16


#include <string>

#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/AList.h"

namespace Belle2 {
  class TRGCDCTrackMC;
}

namespace Belle {

  class TLink;
  class TTrackMC;
  class TFitter;

/// A virtual class for a track class in tracking.
  class TTrackBase {

  public:
    /// Constructor.
    TTrackBase();

    /// Constructor.
    TTrackBase(const AList<TLink> & links);

    /// Destructor
    virtual ~TTrackBase();

  public:// General information
    /// returns object type.
    virtual unsigned objectType(void) const;

    /// returns type. Definition is depending on an object class.
    virtual unsigned type(void) const;

    /// dumps debug information.
    virtual void dump(const std::string& message = std::string(""),
                      const std::string& prefix = std::string("")) const;

  public:// Hit information
    /// returns a list of masked TLinks assigned to this track. 'mask' will be applied if mask is not 0.
    const AList<TLink> & links(unsigned mask = 0) const;

    /// returns \# of masked TLinks assigned to this track object.
    unsigned nLinks(unsigned mask = 0) const;

    /// returns a list of masked TLinks for fit. 'mask' will be applied if mask is not 0.
    const AList<TLink> & cores(unsigned mask = 0) const;

    /// returns \# of masked TLinks for fit. 'mask' will be applied if mask is not 0.
    unsigned nCores(unsigned mask = 0) const;

    /// returns fraction of used layers.
    float fractionUsedLayers(void) const;

    /// returns fraction of used super layers.
    float fractionUsedSuperLayers(void) const;

    /// returns \# of missing super layers.
    unsigned nMissingSuperLayers(void) const;

  public:// Hit manipulation
    /// update cache.
    void update(void) const;

    /// appends a TLink.
    void append(TLink&);

    /// appends TLinks.
    void append(const AList<TLink> &);

    /// appends TLinks by approach. 'list' is an input. Unappended TLinks will be removed from 'list' when returned.
    void appendByApproach(AList<TLink> & list, double maxSigma);

    /// appends TLinks by distance. 'list' is an input. Unappended TLinks will be removed from 'list' when returned.
    void appendByDistance(AList<TLink> & list, double maxDistance);

    /// removes a TLink.
    void remove(TLink& a);

    /// removes TLinks.
    void remove(const AList<TLink> &);

    /// removes bad points by pull. The bad points are removed from the track, and are returned in 'list'.
    virtual void refine(AList<TLink> & list, double maxSigma);

    /// removes bad points by pull. The bad points are masked not to be used in fit.
    virtual void refine(double maxSigma);

    /// returns distance to a position of TLink in TLink space.
    virtual double distance(const TLink&) const;

    /// calculates the closest approach to a wire in real space. Results are stored in TLink. Return value is negative if error happened.
    virtual int approach(TLink&) const;

    /// returns \# of good hits to be appended.
    unsigned testByApproach(const TLink& list, double sigma) const;
    unsigned testByApproach(const AList<TLink> & list, double sigma) const;

    /// fits itself by a default fitter. Error was happened if return value is not zero.
    virtual int fit(void);

    /// returns a pointer to a default fitter.
    const TFitter* const fitter(void) const;

    /// sets a default fitter.
    const TFitter* const fitter(const TFitter*);

    /// false Fit
    void falseFit(); // added by matsu ( 1999/05/24 )

  public:// Operators
    TLink* operator[](unsigned i) const;

  public:// MC information
    /// returns Belle2::TRGCDCTrackMC.
    const Belle2::TRGCDCTrackMC* const hep(void) const;

    /// returns \# of contributed Belle2::TRGCDCTrackMC tracks.
    unsigned nHeps(void) const;

    /// returns a pointer to TTrackMC.
    const TTrackMC* const mc(void) const;

  public:// Obsolete functions. Followings should be removed in Trasan 2.
    /// returns true if fitted.
    bool fitted(void) const;

    /// returns true if fitted with cathode hits(TEMPORARY).
    bool fittedWithCathode(void) const;

  public:// Static utility functions
    /// dumps TLinks.
    static void dump(const AList<TTrackBase> & list,
                     const std::string& message = std::string(""),
                     const std::string& prefix = std::string(""));

  private:
    /// refine core part.
    AList<TLink> refineMain(double maxSigma);

  protected:// Always updated
    AList<TLink> _links;
    mutable bool _fitted;
    mutable bool _fittedWithCathode;
    // unsigned _state;
    TTrackMC* _mc;

  private:// Always updated
    mutable bool _updated;

  private:// Updated when accessed
    mutable AList<TLink> _cores;

  private:// Always updated when accessed
    mutable const Belle2::TRGCDCTrackMC* _hep;
    mutable unsigned _nHeps;

  private:
    const TFitter* _fitter;
    float _time;

    friend class TFitter;
    friend class THelixFitter;
    friend class TCosmicFitter;

#ifdef TRASAN_DEBUG
  public:
    bool fitted(bool) const;
#endif
  };

//-----------------------------------------------------------------------------

#ifdef TTrackBase_NO_INLINE
#define inline
#else
#undef inline
#define TTrackBase_INLINE_DEFINE_HERE
#endif

#ifdef TTrackBase_INLINE_DEFINE_HERE

  inline
  void
  TTrackBase::remove(TLink& a)
  {
    _links.remove(a);
    _updated = false;
    _fitted = false;
    _fittedWithCathode = false; // mod. by matsu ( 1999/05/24 )
  }

  inline
  void
  TTrackBase::remove(const AList<TLink> & a)
  {
    _links.remove(a);
    _updated = false;
    _fitted = false;
    _fittedWithCathode = false; // mod. by matsu ( 1999/05/24 )
  }

  inline
  bool
  TTrackBase::fitted(void) const
  {
    return _fitted;
  }

// added by matsu ( 1999/05/24 )
  inline
  void
  TTrackBase::falseFit()
  {
    _fitted = false;
    _fittedWithCathode = false;
  }
// end of addition

  inline
  TLink*
  TTrackBase::operator[](unsigned i) const
  {
    return _links[i];
  }

  inline
  bool
  TTrackBase::fittedWithCathode(void) const
  {
    return _fittedWithCathode;
  }

  inline
  const TTrackMC* const
  TTrackBase::mc(void) const
  {
    return _mc;
  }

  inline
  const TFitter* const
  TTrackBase::fitter(void) const
  {
    return _fitter;
  }

  inline
  const TFitter* const
  TTrackBase::fitter(const TFitter* a)
  {
    _fitted = false;
    return _fitter = a;
  }

  inline
  unsigned
  TTrackBase::objectType(void) const
  {
    return TrackBase;
  }

  inline
  unsigned
  TTrackBase::type(void) const
  {
    return 0;
  }

#ifdef TRASAN_DEBUG
  inline
  bool
  TTrackBase::fitted(bool a) const
  {
    return _fitted = a;
  }
#endif

#endif

#undef inline

} // namespace Belle

#endif /* TTrackBase_FLAG_ */
