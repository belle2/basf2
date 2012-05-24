//-----------------------------------------------------------------------------
// $Id: TCosmicFitter.h 10002 2007-02-26 06:56:17Z katayama $
//-----------------------------------------------------------------------------
// Filename : TCosmicFitter.h
// Section  : Tracking
// Owner    : Jun-ichi Suzuki
// Email    : jsuzuki@bmail.kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object to a helix.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.9  2001/12/23 09:58:55  katayama
// removed Strings.h
//
// Revision 1.8  2001/12/19 02:59:54  katayama
// Uss find,istring
//
// Revision 1.7  2001/12/14 02:54:49  katayama
// For gcc-3.0
//
// Revision 1.6  2001/04/11 01:10:03  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.5  1999/10/30 10:12:47  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.4  1999/07/09 01:47:25  yiwasaki
// Trasan 1.53a release : cathode updates by T.Matsumoto, minor change of Conformal finder
//
// Revision 1.3  1999/04/09 11:36:56  yiwasaki
// Trasan 1.32 release : TCosmicFitter minor change
//
// Revision 1.2  1999/04/07 06:14:13  yiwasaki
// Trasan 1.30 release : curl finder mask updated
//
// Revision 1.1  1999/03/21 15:45:51  yiwasaki
// Trasan 1.28 release : TrackManager bug fix, CosmicFitter added in BuilderCosmic, parameter salvage level added
//
// Revision 1.1  1999/01/11 03:03:26  yiwasaki
// Fitters added
//
//-----------------------------------------------------------------------------

#ifndef TCOSMICFITTER_FLAG_
#define TCOSMICFITTER_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif

#include "tracking/modules/trasan/TFitter.h"
#define HEP_SHORT_NAMES
#include "CLHEP/Matrix/Vector.h"
#include "tracking/modules/trasan/THelix.h"

namespace Belle {

  class TLink;

/// A class to fit a TTrackBase object to a helix.
  class TCosmicFitter : public TFitter {

  public:
    /// Constructor.
    TCosmicFitter(const std::string& name);

    /// Destructor
    virtual ~TCosmicFitter();

  public:// Selectors
    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  public:// Modifiers
    int fit(TTrackBase&) const;
    int fit(TTrackBase&, float t0Offset) const;

    int fitWithCathode(TTrackBase&, float t0Offset = 0.,
                       float windowSize = 0.6, int SysCorr = 0) ; // added by matsu ( 1999/07/05 )

  private:
    /// calculates dXda. 'link' and 'dPhi' are inputs. Others are outputs.
    int dxda(const TLink& link,
             const THelix& helix,
             double dPhi,
             CLHEP::HepVector& dxda,
             CLHEP::HepVector& dyda,
             CLHEP::HepVector& dzda,
             int doSagCorrection) const;

  private:
  };

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define TCOSMICFITTER_INLINE_DEFINE_HERE
#endif

#ifdef TCOSMICFITTER_INLINE_DEFINE_HERE

#endif

#undef inline

} // namespace Belle

#endif /* TCOSMICFITTER_FLAG_ */
