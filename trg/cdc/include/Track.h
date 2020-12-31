//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Track.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a reconstructed charged track in TRGCDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCTrack_FLAG_
#define TRGCDCTrack_FLAG_

#include "trg/cdc/TrackBase.h"
#include "trg/cdc/Helix.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCTrack TRGCDCTrack
#endif

//...Definition of status...
#define TCTrack_2DTrack       1
#define TCTrack_2DTrackFitted 2
#define TCTrack_3DTrack       4
#define TCTrack_3DTrackFitted 8

namespace Belle2 {

  class TRGCDCCircle;

/// A class to represent a reconstructed charged track in TRGCDC.
  class TRGCDCTrack : public TRGCDCTrackBase {

  public:
    /// Enum for returnValue types
    enum EDebugValueType { fit2D = 1 << 3,
                           find3D = 1 << 4,
                           fit3D = 1 << 5,
                           any = fit2D | find3D | fit3D
                         };


    /// returns a list of TRGCDCTrack's.
    static std::vector<const TRGCDCTrack*> list(void);

    /// Constructor
    TRGCDCTrack();

    /// Constructor from a Circle.
    explicit TRGCDCTrack(const TRGCDCCircle&);

    /// Destructor
    virtual ~TRGCDCTrack();

    /// returns helix parameter.
    const TRGCDCHelix& helix(void) const;

    /// Set helix parameter
    void setHelix(const TRGCDCHelix& helix);

    /// calculates the closest approach to a wire in real space. Results are stored in TLink. Return value is negative if error happened.
    int approach(TRGCDCLink&, bool sagCorrection = false) const;

    /// returns momentum vector.
    virtual const CLHEP::Hep3Vector& p(void) const override;

    /// returns Pt.
    virtual double pt(void) const override;

    /// returns position vector.
    virtual const CLHEP::Hep3Vector& x(void) const override;

    /// Set 2D fit chi2
    void set2DFitChi2(double);

    /// Returns 2D fit chi2
    double get2DFitChi2(void) const;

    /// Set 3D fit chi2
    void set3DFitChi2(double);

    /// Returns 3D fit chi2
    double get3DFitChi2(void) const;

    /// Set debug value
    void setDebugValue(EDebugValueType const& moduleName, bool flag);

    /// Get debug value
    int getDebugValue(EDebugValueType const& moduleName) const;

  public:

    /// returns perfect position from GEANT.
    std::vector<HepGeom::Point3D<double> > perfectPosition(void) const;

  private:// static members

    /// a vector to keep all TRGCDCTrack objects.
    static std::vector<const TRGCDCTrack*> _list;

  private:

    /// Helix parameter.
    TRGCDCHelix _helix;

    /// Momentum.
    mutable CLHEP::Hep3Vector _p;

    /// Position.
    mutable CLHEP::Hep3Vector _x;

    /// 2D fit chi2
    double m_2DFitChi2;

    /// 3D fit chi2
    double m_3DFitChi2;

    /// Debugging variable
    int m_debugValue;
  };

//-----------------------------------------------------------------------------

// inline
// unsigned
// TRGCDCTrack::id(void) const{
//     if (_list.size()) {
//         for (unsigned i = 0; i < _list.size(); i++)
//             if (_list[i] == this)
//                 return unsigned(i);
//     }
//     return 999;
// }

  inline
  const TRGCDCHelix&
  TRGCDCTrack::helix(void) const
  {
    return _helix;
  }

  inline
  void
  TRGCDCTrack::setHelix(const TRGCDCHelix& helix)
  {
    _helix = helix;
  }

  inline
  const CLHEP::Hep3Vector&
  TRGCDCTrack::p(void) const
  {
    _p = _helix.momentum();
    return _p;
  }

  inline
  double
  TRGCDCTrack::pt(void) const
  {
    _p = _helix.momentum();
    return _p.perp();
  }

  inline
  const CLHEP::Hep3Vector&
  TRGCDCTrack::x(void) const
  {
    _x = _helix.x();
    return _x;
  }

  inline
  void TRGCDCTrack::set2DFitChi2(double in2DFitChi2)
  {
    m_2DFitChi2 = in2DFitChi2;
  }

  inline
  double TRGCDCTrack::get2DFitChi2(void) const
  {
    return m_2DFitChi2;
  }

  inline
  void TRGCDCTrack::set3DFitChi2(double in3DFitChi2)
  {
    m_3DFitChi2 = in3DFitChi2;
  }

  inline
  double TRGCDCTrack::get3DFitChi2(void) const
  {
    return m_3DFitChi2;
  }

  inline
  void TRGCDCTrack::setDebugValue(EDebugValueType const& moduleName, bool flag)
  {
    if (flag) m_debugValue |= moduleName;
    else m_debugValue &= ~moduleName;
  }

  inline
  int TRGCDCTrack::getDebugValue(EDebugValueType const& moduleName) const
  {
    return m_debugValue & moduleName;
  }



} // namespace Belle2

#endif /* TRGCDCTrack_FLAG_ */
