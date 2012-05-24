//-----------------------------------------------------------------------------
// $Id: THelixFitter.h 10002 2007-02-26 06:56:17Z katayama $
//-----------------------------------------------------------------------------
// Filename : THelixFitter.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object to a helix.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.13  2001/12/23 09:58:56  katayama
// removed Strings.h
//
// Revision 1.12  2001/12/19 02:59:55  katayama
// Uss find,istring
//
// Revision 1.11  2001/12/14 02:54:49  katayama
// For gcc-3.0
//
// Revision 1.10  2001/04/18 01:29:28  yiwasaki
// helix fitter speed up by jt
//
// Revision 1.9  2001/04/11 01:10:03  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.8  2000/10/10 06:04:26  yiwasaki
// Trasan 2.11 : curl builder, helix fitter, perfect finder
//
// Revision 1.7  2000/10/05 23:54:31  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.6  1999/10/30 10:12:49  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.5  1999/08/25 06:25:54  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.4  1999/07/17 06:43:01  yiwasaki
// THelixFitter has tof correction, Trasan time has option for tof correction
//
// Revision 1.3  1999/06/16 08:29:55  yiwasaki
// Trasan 1.44 release : new THelixFitter
//
// Revision 1.2  1999/06/10 09:44:54  yiwasaki
// Trasan 1.40 release : minor changes only
//
// Revision 1.1  1999/01/11 03:03:26  yiwasaki
// Fitters added
//
//-----------------------------------------------------------------------------

#ifndef THELIXFITTER_FLAG_
#define THELIXFITTER_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif
#define HEP_SHORT_NAMES

#include "CLHEP/Matrix/Vector.h"
#include "tracking/modules/trasan/THelix.h"
#include "tracking/modules/trasan/TFitter.h"
#include "tracking/modules/trasan/TTrackBase.h"

namespace Belle {

  class TLink;

//...Drift time correction method...
#define TrackFitNoCorrection          0
#define TrackFitCorrectOnce           1
#define TrackFitCorrectEveryIteration 2

/// A class to fit a TTrackBase object to a helix.
  class THelixFitter : public TFitter {

  public:
    /// Constructor.
    THelixFitter(const std::string& name);

    /// Destructor
    virtual ~THelixFitter();

  public:// Informations
    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  public:// Options
    /// sets/returns 2D flag.
    bool fit2D(void) const;
    bool fit2D(bool);

    /// sets/returns free T0 flag.
    bool freeT0(void) const;
    bool freeT0(bool);

    /// sets/returns correctin flag.
    unsigned corrections(void) const;
    unsigned corrections(unsigned);

  public:// Options (obsolete)
    /// sets/returns sag correction flag.
    bool sag(void) const;
    bool sag(bool);

    /// sets/returns propagation-delay correction flag.
    bool propagation(void) const;
    bool propagation(bool);

    /// sets/returns propagation-delay correction flag.
    bool tof(void) const;
    bool tof(bool);

    /// sets/returns tanLambda correction flag.
    bool tanl(void) const;
    bool tanl(bool);

    /// returns sum of chi2 before fit.
    double preChi2(void) const;

    /// returns sum of chi2 aftter fit.
    double chi2(void) const;

  public:// Fitting
    int fit(TTrackBase&) const;
    int fit(TTrackBase&, double* pre_chi2, double* fitted_chi2) const;
    int fit(TTrackBase&, float t0Offset, double* pre_chi2 = NULL, double* fitted_chi2 = NULL) const;
    int fit(TTrackBase&, float& tev, float& tev_err, double* pre_chi2 = NULL, double* fitted_chi2 = NULL) const;

  private:
    /// main routine for fixed T0.
    int main(TTrackBase&, float t0Offset, double* pre_chi2 = NULL, double* fitted_chi2 = NULL) const;

    /// main routine for free T0.
    int main(TTrackBase&, float& tev, float& tev_err, double* pre_chi2 = NULL, double* fitted_chi2 = NULL) const;

    /// calculates drift distance and its error.
    void drift(const TTrack&,
               const TLink&,
               float t0Offset,
               double& distance,
               double& itsError) const;

    /// calculates drift distance and its error for free T0 case.
    void drift(const TTrack&,
               const TLink&,
               float t0Offset,
               double& distance,
               double& itsError,
               double& ddda) const;

    /// calculates dXda. 'link' and 'dPhi' are inputs. Others are outputs.
    // virtual
    int dxda(const TLink& link,
             const THelix& helix,
             double dPhi,
             CLHEP::HepVector& dxda,
             CLHEP::HepVector& dyda,
             CLHEP::HepVector& dzda) const;

  private:
    bool _fit2D;
    bool _freeT0;
    unsigned _corrections;

    bool _sag;
    int _propagation;
    bool _tof;
    bool _tanl;

    mutable double _pre_chi2;
    mutable double _fitted_chi2;
  };

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define THELIXFITTER_INLINE_DEFINE_HERE
#endif

#ifdef THELIXFITTER_INLINE_DEFINE_HERE

  inline
  bool
  THelixFitter::fit2D(void) const
  {
    return _fit2D;
  }

  inline
  bool
  THelixFitter::fit2D(bool a)
  {
    return _fit2D = a;
  }

  inline
  bool
  THelixFitter::sag(void) const
  {
    return _sag;
  }

  inline
  bool
  THelixFitter::sag(bool a)
  {
    return _sag = a;
  }

  inline
  bool
  THelixFitter::propagation(void) const
  {
    return (bool) _propagation;
  }

  inline
  bool
  THelixFitter::propagation(bool a)
  {
    if (a) _propagation = 1;
    else   _propagation = 0;
    return propagation();
  }

  inline
  bool
  THelixFitter::tof(void) const
  {
    return _tof;
  }

  inline
  bool
  THelixFitter::tof(bool a)
  {
    return _tof = a;
  }

  inline
  bool
  THelixFitter::tanl(void) const
  {
    return _tanl;
  }

  inline
  bool
  THelixFitter::tanl(bool a)
  {
    return _tanl = a;
  }

  inline
  int
  THelixFitter::fit(TTrackBase& a) const
  {
    if (! _freeT0) return main(a, 0.);
    else {
      float tev = 0.;
      float tevError;
      return main(a, tev, tevError);
    }
  }

  inline
  int
  THelixFitter::fit(TTrackBase& a,
                    double* pre_chi2, double* fitted_chi2) const
  {
    if (! _freeT0) return main(a, 0., pre_chi2, fitted_chi2);
    else {
      float tev = 0.;
      float tevError;
      return main(a, tev, tevError, pre_chi2, fitted_chi2);
    }
  }

  inline
  int
  THelixFitter::fit(TTrackBase& a, float t0Offset,
                    double* pre_chi2, double* fitted_chi2) const
  {
    a._fitted = false;
    if (! _freeT0) return main(a, t0Offset, pre_chi2, fitted_chi2);
    else {
      float tev = t0Offset;
      float tevError;
      return main(a, tev, tevError, pre_chi2, fitted_chi2);
    }
  }

  inline
  int
  THelixFitter::fit(TTrackBase& a, float& tev, float& tev_err,
                    double* pre_chi2, double* fitted_chi2) const
  {
    a._fitted = false;
    return main(a, tev, tev_err, pre_chi2, fitted_chi2);
  }

  inline
  bool
  THelixFitter::freeT0(void) const
  {
    return _freeT0;
  }

  inline
  bool
  THelixFitter::freeT0(bool a)
  {
    return _freeT0 = a;
  }

  inline
  unsigned
  THelixFitter::corrections(void) const
  {
    return _corrections;
  }

  inline
  unsigned
  THelixFitter::corrections(unsigned a)
  {
    return _corrections = a;
  }

  inline
  double
  THelixFitter::preChi2(void) const
  {
    return _pre_chi2;
  }

  inline
  double
  THelixFitter::chi2(void) const
  {
    return _fitted_chi2;
  }

#endif

#undef inline


} // namespace Belle

#endif /* THELIXFITTER_FLAG_ */
