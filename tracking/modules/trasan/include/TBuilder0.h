//-----------------------------------------------------------------------------
// $Id: TBuilder0.h 10002 2007-02-26 06:56:17Z katayama $
//-----------------------------------------------------------------------------
// Filename : TBuilder0.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to build a track.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.6  2004/03/26 06:07:25  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.5  2001/12/23 09:58:53  katayama
// removed Strings.h
//
// Revision 1.4  2001/12/19 02:59:52  katayama
// Uss find,istring
//
// Revision 1.3  2001/12/14 02:54:46  katayama
// For gcc-3.0
//
// Revision 1.2  2001/04/11 01:10:01  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.1  1999/10/30 10:12:34  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.24  1999/09/21 02:01:37  yiwasaki
// Trasan 1.63b release : conformal finder minor changes, TWindow added for debug
//
// Revision 1.23  1999/09/10 09:20:00  yiwasaki
// Trasan 1.61b release : new parameters added, TTrackMC bug fixed
//
// Revision 1.22  1999/08/25 06:25:52  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.21  1999/08/04 01:01:43  yiwasaki
// Trasan 1.59b release : putting parameters from basf enabled
//
// Revision 1.20  1999/03/21 15:45:47  yiwasaki
// Trasan 1.28 release : TrackManager bug fix, CosmicFitter added in BuilderCosmic, parameter salvage level added
//
// Revision 1.19  1999/03/15 07:57:16  yiwasaki
// Trasan 1.27 release : curl finder update
//
// Revision 1.18  1999/03/11 23:27:24  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.17  1999/03/08 05:47:55  yiwasaki
// Trasan 1.20 release : Fitter in TBuilder is modified
//
// Revision 1.16  1999/02/03 06:23:17  yiwasaki
// Trasan 1.14 release : bugs in curl finder and trasan fixed, new salvage installed
//
// Revision 1.15  1999/01/27 11:52:37  yiwasaki
// very minor changes
//
// Revision 1.14  1999/01/11 03:03:20  yiwasaki
// Fitters added
//
// Revision 1.13  1998/11/27 08:15:41  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.12  1998/10/06 02:30:11  yiwasaki
// Trasan 1.1 beta 3 relase : only minor change
//
// Revision 1.11  1998/09/29 01:24:30  yiwasaki
// Trasan 1.1 beta 1 relase : TBuilderCurl added
//
// Revision 1.10  1998/09/28 16:52:13  yiwasaki
// TBuilderCosmic added
//
// Revision 1.9  1998/08/12 16:32:56  yiwasaki
// Trasan 1.08 release : stereo finder updated by J.Suzuki, new MC classes added by Y.Iwasaki
//
// Revision 1.8  1998/07/29 04:35:02  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.6  1998/07/07 01:15:33  yiwasaki
// comment changed
//
// Revision 1.5  1998/07/06 15:48:54  yiwasaki
// Trasan 1.01 release:CurlFinder default on, bugs in TLine and TTrack::fit fixed
//
// Revision 1.4  1998/07/02 09:04:43  yiwasaki
// Trasan 1.0 official release
//
// Revision 1.3  1998/06/21 18:38:23  yiwasaki
// Trasan 1 beta 5 release, rphi improved?
//
// Revision 1.2  1998/06/15 09:58:18  yiwasaki
// Trasan 1 beta 3.2, bug fixed
//
// Revision 1.1  1998/06/14 11:09:55  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
//-----------------------------------------------------------------------------

#ifndef TBuilder0_FLAG_
#define TBuilder0_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>

#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/AList.h"
#include "tracking/modules/trasan/TSelector0.h"
#include "tracking/modules/trasan/THelixFitter.h"

namespace Belle {

  class TTrack;
  class TLink;
  class TSegment;

/// A class to build a track.
  class TBuilder0 {

  public:
    /// Constructor.
    TBuilder0(const std::string& name);

    /// Constructor with salvage level.
    TBuilder0(const std::string& name,
              float salvageLevel);

    /// Constructor with parameters.
    TBuilder0(const std::string& name,
              float stereoZ3,
              float stereoZ4,
              float stereoChisq3,
              float stereoChisq4,
              float stereoMaxSigma,
              unsigned fittingCorrections,
              float salvageLevel);

    /// Destructor
    virtual ~TBuilder0();

  public:// Selectors
    /// returns name.
    const std::string& name(void) const;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// returns a track selector.
    const TSelector0& trackSelector(void) const;

    /// builds a r/phi track from TLinks or from Segments.
    TTrack* buildRphi(const AList<TLink> &) const;

    /// appends stereo hits to a track. (old version)
    TTrack* buildStereo0(TTrack& track, const AList<TLink> &) const;

    /// appends stereo hits to a track.
    virtual TTrack* buildStereo(TTrack& track, const AList<TLink> &) const;

    /// appends TLinks in a list.
    void appendClusters(TTrack& track, const AList<TLink> &) const;

    /// salvages links in a list. Used links will be removed from a list.
    void salvage(TTrack& track, AList<TLink> & list) const;

    /// fits a track using a private fitter.
    virtual int fit(TTrackBase&) const;

  public:// Modifiers
    /// sets a track selector.
    virtual const TSelector0& trackSelector(const TSelector0&);

  private://
    void selectHits(AList<TLink> & list) const;
    // -50:bad consective, -20:not consective, -1:not in the same layer, 0:consective and chrg<=0, 1:consective and chrg>0
    int consectiveHits(TLink& l, TLink& s, int ichg) const;
    int check2CnHits(TLink& l, TLink& s, int ichg) const;
    int checkHits(unsigned i, unsigned j, unsigned k) const;
    void salvageNormal(TTrack& track, AList<TLink> & list) const;

  private:
    std::string _name;
    THelixFitter _fitter;
    double _salvageLevel;

  protected:// for buildRphi
    TSelector0 _circleSelector;
    TSelector0 _trackSelector;

  protected:// for buildStereo
    TSelector0 _lineSelector;
    float _stereoZ3;
    float _stereoZ4;
    float _stereoChisq3;
    float _stereoChisq4;
    float _stereoMaxSigma;
  };

//-----------------------------------------------------------------------------

#ifdef TBuilder0_NO_INLINE
#define inline
#else
#undef inline
#define TBuilder0_INLINE_DEFINE_HERE
#endif

#ifdef TBuilder0_INLINE_DEFINE_HERE

  inline
  const TSelector0&
  TBuilder0::trackSelector(void) const
  {
    return _trackSelector;
  }

  inline
  const std::string&
  TBuilder0::name(void) const
  {
    return _name;
  }

  inline
  int
  TBuilder0::fit(TTrackBase& a) const
  {
    return _fitter.fit(a);
  }

#endif

#undef inline

} // namespace Belle

#endif /* TBuilder0_FLAG_ */
