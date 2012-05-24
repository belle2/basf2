//-----------------------------------------------------------------------------
// $Id: TBuilder.h 10504 2008-04-24 21:57:18Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TBuilder.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to build a track.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.42  2005/11/03 23:20:35  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.41  2005/03/11 03:58:34  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.40  2004/04/23 09:48:24  yiwasaki
// Trasan 3.12 : curlVersion=2 is default
//
// Revision 1.39  2004/03/26 06:07:25  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.38  2001/12/23 09:58:53  katayama
// removed Strings.h
//
// Revision 1.37  2001/12/19 02:59:52  katayama
// Uss find,istring
//
// Revision 1.36  2001/12/14 02:54:46  katayama
// For gcc-3.0
//
// Revision 1.35  2001/04/11 01:10:00  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.34  2000/03/24 10:23:01  yiwasaki
// Trasan 2.00RC20 : track manager bug fix
//
// Revision 1.33  2000/02/29 07:16:17  yiwasaki
// Trasan 2.00RC13 : default stereo param. changed
//
// Revision 1.32  2000/02/10 13:11:41  yiwasaki
// Trasan 2.00RC1 : conformal bug fix, parameters added
//
// Revision 1.31  2000/02/09 03:27:41  yiwasaki
// Trasan 1.68l : curl and new conf stereo updated
//
// Revision 1.30  2000/02/03 06:18:06  yiwasaki
// Trasan 1.68k : slow finder in conf. finder temporary off
//
// Revision 1.29  2000/02/01 11:24:45  yiwasaki
// Trasan 1.68j : curl finder modified, new stereo finder modified
//
// Revision 1.28  2000/01/30 08:17:14  yiwasaki
// Trasan 1.67i = Trasan 2.00 RC1 : new conf modified
//
// Revision 1.27  2000/01/28 06:30:29  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.26  1999/11/19 09:13:14  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.25  1999/10/30 10:12:34  yiwasaki
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

#ifndef TBuilder_FLAG_
#define TBuilder_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>

#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/AList.h"
#include "tracking/modules/trasan/THelixFitter.h"

namespace Belle {

  class TTrack;
  class TLink;
  class TLine;
  class TSegment;
  class THoughTransformation;
  class THoughPlane;

/// A class to build a track.
  class TBuilder {

  public:
    /// Constructor with salvage level.
    TBuilder(const std::string& name,
             float maxSigma,
             float maxSigmaStereo,
             float salvageLevel,
             float szLinkDistance,
             unsigned fittingFlag);

    /// Destructor
    virtual ~TBuilder();

  public:// Debug information
    /// returns name.
    const std::string& name(void) const;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// returns minimum \# of core links.
    unsigned minNCores(void) const;

    /// returns range of sz to search for in stereo reconstruction
    float szLinkDistance(void) const;

    /// returns max. sigma for stereo reconstruction.
    float maxSigmaStereo(void) const;

    /// returns a fitter.
    const THelixFitter& helixFitter(void) const;

  public:// Track building

    /// builds a r/phi track.
    TTrack* buildRphi(const AList<TLink> &) const;

    /// builds a 3D track.
    TTrack* buildStereo(TTrack& track, const AList<TLink> & list) const;

    /// salvages hits.
    void salvage(TTrack& t, AList<TLink> & hits) const;

  protected:
    TLine* initialLine(const TTrack&, const AList<TLink> &) const;
    AList<TLine> initialLines(const TTrack&, const AList<TLink> &) const;
    AList<TLine> initialLines2(const TTrack&, const AList<TLink> &) const;
    AList<TLink> selectStereoHits(const TTrack&,
                                  const TLine&,
                                  const AList<TLink> &) const;
    void houghTransformation(const AList<TLink> & hits,
                             const THoughTransformation& trans,
                             THoughPlane& plane) const;
    void refine(TTrack& t, AList<TLink> & list, double maxSigma) const;


    /// makes a track (standard version)
    TTrack* buildRphiFast(AList<TLink> & list) const;

    /// makes a track (slow version)
    TTrack* buildRphiSlow(AList<TLink> & list) const;

  public:// Obsolete functions
    TTrack* build(TTrack& t, const TLine& l) const;
    TTrack* buildStereoOld(TTrack& t,
                           const AList<TLink> & allLinks,
                           const AList<TLink> & isolatedLinks) const;
    TTrack* buildStereoOld(TTrack& track, const AList<TLink> & list) const;
    AList<TLine> initialLinesOld(const TTrack&, const AList<TLink> &) const;

  private:
    std::string _name;
    THelixFitter _fitter;
    const float _maxSigma;
    const float _maxSigmaStereo;
    const float _salvageLevel;
    const float _szLinkDistance;

  private:// Parameters
    unsigned _minNLinksForCircle;
    unsigned _minNCores;

    friend class TBuilderConformal;
  };

//-----------------------------------------------------------------------------

#ifdef TBuilder_NO_INLINE
#define inline
#else
#undef inline
#define TBuilder_INLINE_DEFINE_HERE
#endif

#ifdef TBuilder_INLINE_DEFINE_HERE

  inline
  const std::string&
  TBuilder::name(void) const
  {
    return _name;
  }

  inline
  unsigned
  TBuilder::minNCores(void) const
  {
    return _minNCores;
  }

  inline
  float
  TBuilder::szLinkDistance(void) const
  {
    return _szLinkDistance;
  }

  inline
  const THelixFitter&
  TBuilder::helixFitter(void) const
  {
    return _fitter;
  }

  inline
  float
  TBuilder::maxSigmaStereo(void) const
  {
    return _maxSigmaStereo;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TBuilder_FLAG_ */
