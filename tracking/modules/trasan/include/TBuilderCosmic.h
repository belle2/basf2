//-----------------------------------------------------------------------------
// $Id: TBuilderCosmic.h 10002 2007-02-26 06:56:17Z katayama $
//-----------------------------------------------------------------------------
// Filename : TBuilderCosmic.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to build a cosmic track.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.9  2004/03/26 06:07:26  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.8  2001/12/23 09:58:54  katayama
// removed Strings.h
//
// Revision 1.7  2001/12/19 02:59:53  katayama
// Uss find,istring
//
// Revision 1.6  2001/12/14 02:54:46  katayama
// For gcc-3.0
//
// Revision 1.5  2001/04/11 01:10:01  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.4  1999/10/30 10:12:35  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.3  1999/03/21 15:45:48  yiwasaki
// Trasan 1.28 release : TrackManager bug fix, CosmicFitter added in BuilderCosmic, parameter salvage level added
//
// Revision 1.2  1999/01/11 03:03:21  yiwasaki
// Fitters added
//
// Revision 1.1  1998/09/28 16:52:14  yiwasaki
// TBuilderCosmic added
//
//-----------------------------------------------------------------------------

#ifndef TBuilderCosmic_FLAG_
#define TBuilderCosmic_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>

#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/AList.h"
#include "tracking/modules/trasan/TBuilder0.h"
#include "tracking/modules/trasan/TCosmicFitter.h"

namespace Belle {

/// A class to build a cosmic track.
  class TBuilderCosmic : public TBuilder0 {

  public:
    /// Constructor.
    TBuilderCosmic(const std::string& name, float salvageLevel);

    /// Destructor
    virtual ~TBuilderCosmic();

  public:// Selectors
    /// appends stereo hits to a track.
    TTrack* buildStereo(TTrack& track, const AList<TLink> &) const;

  private:
    // THelixFitter _fitter;
    TCosmicFitter _fitter;
  };

//-----------------------------------------------------------------------------

#ifdef TBuilderCosmic_NO_INLINE
#define inline
#else
#undef inline
#define TBuilderCosmic_INLINE_DEFINE_HERE
#endif

#ifdef TBuilderCosmic_INLINE_DEFINE_HERE

#endif

#undef inline

} // namespace Belle

#endif /* TBuilderCosmic_FLAG_ */
