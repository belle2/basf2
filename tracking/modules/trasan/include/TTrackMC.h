//-----------------------------------------------------------------------------
// $Id: TTrackMC.h 10099 2007-05-07 13:59:16Z katayama $
//-----------------------------------------------------------------------------
// Filename : TTrackMC.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to have MC information of TTrack.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.13  2002/01/03 11:04:59  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.12  2001/12/23 09:58:57  katayama
// removed Strings.h
//
// Revision 1.11  2001/12/19 02:59:56  katayama
// Uss find,istring
//
// Revision 1.10  2001/12/14 02:54:51  katayama
// For gcc-3.0
//
// Revision 1.9  2001/04/11 01:10:05  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.8  2001/01/17 06:27:57  yiwasaki
// hists in sakura modified
//
// Revision 1.7  2001/01/11 04:40:26  yiwasaki
// minor changes
//
// Revision 1.6  2001/01/11 03:40:51  yiwasaki
// minor changes
//
// Revision 1.5  1999/10/30 10:12:54  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.4  1998/11/27 08:15:50  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.3  1998/09/28 14:54:21  yiwasaki
// MC tables, TUpdater, oichan added
//
// Revision 1.2  1998/08/31 05:16:09  yiwasaki
// Trasan 1.09 release : curl finder updated by J.Tanaka, MC classes updated by Y.Iwasaki
//
// Revision 1.1  1998/08/12 16:33:06  yiwasaki
// Trasan 1.08 release : stereo finder updated by J.Suzuki, new MC classes added by Y.Iwasaki
//
//-----------------------------------------------------------------------------

#ifndef TTrackMC_FLAG_
#define TTrackMC_FLAG_

#define TTrackClassified 1

#define TTrackGarbage 1
#define TTrackCharge 2
#define TTrackBad 4
#define TTrackGhost 8
#define TTrackGood 16

#define TTrackHep 65536
#define TTrackWire 131072
#define TTrackPt 524288
#define TTrackPz 1048576
#define TTrackUnique 2097152
#define TTrackMatchingLoose 4194304
#define TTrackMatchingTight 8388608

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>

#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/AList.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Vector/LorentzVector.h"
struct reccdc_mctrk;

namespace Belle {
  class TRGCDCTrackMC;
}

namespace Belle {

  class Trasan;
  class TTrack;

/// A class to have MC information of TTrack.
  class TTrackMC {

  private:
    /// Constructor
    TTrackMC(const TTrack&);

  public:
    /// Destructor
    virtual ~TTrackMC();

  public:
    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// returns HEP ID.
    int hepId(void) const;

    /// returns a pointer to Belle2::TRGCDCTrackMC.
    const Belle2::TRGCDCTrackMC* const hep(void) const;

    /// returns charge matching.
    bool charge(void) const;

    /// returns wire fraction(F1).
    double wireFraction(void) const;

    /// returns wire fraction(F2).
    double wireFractionHEP(void) const;

    /// returns pt fraction.
    double ptFraction(void) const;

    /// returns pz fraction.
    double pzFraction(void) const;

    /// returns residual of momentum.
    const CLHEP::Hep3Vector& residual(void) const;

    /// returns residual of pt.
    double ptResidual(void) const;

    /// returns residual of pz.
    double pzResidual(void) const;

    /// returns pull of pt.
    double ptPull(void) const;

    /// returns pull of pz.
    double pzPull(void) const;

    /// returns state.
    unsigned state(void) const;

    /// returns quality.
    unsigned quality(void) const;

    /// returns quality.
    std::string qualityString(void) const;

  public:
    /// updates information.
    void update(void);

  private:
    /// checks matching of charge.
    void compare(void);

    /// classifies itself.
    void classify(void);

  private:
    unsigned _state;
    unsigned _quality;

    const TTrack& _t;
    Belle2::TRGCDCTrackMC* _hep;
    int _hepID;
    double _wireFraction;
    double _wireFractionHEP;
    bool _charge;
    double _ptFraction;
    double _pzFraction;
    CLHEP::Hep3Vector _residual;
    double _cosOpen;
    double _ptResidual;
    double _pzResidual;
    double _ptPull;
    double _pzPull;

    friend class Trasan;
  };

  std::string
  TrackMCStatus(const TTrackMC&);

  std::string
  TrackMCStatus(const reccdc_mctrk&);

  std::string
  TrackMCQualityString(unsigned quality);

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define TTrackMC_INLINE_DEFINE_HERE
#endif

#ifdef TTrackMC_INLINE_DEFINE_HERE

  inline
  int
  TTrackMC::hepId(void) const
  {
    return _hepID;
  }

  inline
  const Belle2::TRGCDCTrackMC* const
  TTrackMC::hep(void) const
  {
    return _hep;
  }

  inline
  bool
  TTrackMC::charge(void) const
  {
    return _charge;
  }

  inline
  double
  TTrackMC::ptFraction(void) const
  {
    return _ptFraction;
  }

  inline
  double
  TTrackMC::pzFraction(void) const
  {
    return _pzFraction;
  }

  inline
  double
  TTrackMC::wireFraction(void) const
  {
    return _wireFraction;
  }

  inline
  double
  TTrackMC::wireFractionHEP(void) const
  {
    return _wireFractionHEP;
  }

  inline
  unsigned
  TTrackMC::state(void) const
  {
    return _state;
  }

  inline
  unsigned
  TTrackMC::quality(void) const
  {
    return _quality;
  }

  inline
  const CLHEP::Hep3Vector&
  TTrackMC::residual(void) const
  {
    return _residual;
  }

  inline
  double
  TTrackMC::ptResidual(void) const
  {
    return _ptResidual;
  }

  inline
  double
  TTrackMC::pzResidual(void) const
  {
    return _pzResidual;
  }

  inline
  double
  TTrackMC::ptPull(void) const
  {
    return _ptPull;
  }

  inline
  double
  TTrackMC::pzPull(void) const
  {
    return _pzPull;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TTrackMC_FLAG_ */
