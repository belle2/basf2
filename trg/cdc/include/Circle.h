/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to represent a circle
//-----------------------------------------------------------------------------

#ifndef TRGCDCCircle_FLAG_
#define TRGCDCCircle_FLAG_

#include "trg/trg/Point2D.h"
#include "trg/cdc/TrackBase.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCCircle TRGCDCCircle
#endif

namespace Belle2 {

  class TRGCDCHoughPlane;
  class TRGCDCCircleFitter;

/// A class to represent a circle.
  class TRGCDCCircle : public TRGCDCTrackBase {

  public:

    /// Constructor with links.
    explicit TRGCDCCircle(const std::vector<TRGCDCLink*>& links);

    /// Constructor with a circle center, assuming the origin is on a circle.
    TRGCDCCircle(double r,
                 double phi,
                 double charge,
                 const TRGCDCHoughPlane& plane);

    /// Destructor
    virtual ~TRGCDCCircle();

  public:

    /// returns type.
    virtual unsigned objectType(void) const override;

    /// returns the circle center.
    const TRGPoint2D& center(void) const;

    /// returns Pt.
    double pt(void) const override;

    /// returns radius.
    double radius(void) const;

    /// dumps debug information.
    virtual void dump(const std::string& message = std::string(""),
                      const std::string& prefix = std::string("")) const override;

    /// sets circle properties.
    void property(double charge,
                  double radius,
                  HepGeom::Point3D<double> center);
//  void property(double charge, double radius, TPoint2D & center);

  public: // Utility functions

    /// calculate closest approach. Error was happened if return value is not zero.
    int approach2D(TRGCDCLink&) const override;

  private:

    /// Position of the circle center.
    TRGPoint2D _center;

    /// Radius multiplied by charge.
    double _radius;

    /// Hough plane.
    const TRGCDCHoughPlane* _plane;

    /// Default fitter.
    static const TRGCDCCircleFitter _fitter;

  };

//-----------------------------------------------------------------------------

  inline
  double
  TRGCDCCircle::pt(void) const
  {
    const double ConstantAlpha = 222.376063; // for 1.5T
    return _radius / ConstantAlpha;
  }

  inline
  const TRGPoint2D&
  TRGCDCCircle::center(void) const
  {
    return _center;
  }

  inline
  double
  TRGCDCCircle::radius(void) const
  {
    return _radius;
  }

  inline
  unsigned
  TRGCDCCircle::objectType(void) const
  {
    return TRGCDCCircleType;
  }

  inline
  void
  TRGCDCCircle::property(double c, double r, HepGeom::Point3D<double> e)
  {
    charge(c);
    _radius = r;
    _center = e;
  }

} // namespace Belle2

#endif /* TRGCDCCircle_FLAG_ */
