//-----------------------------------------------------------------------------
// $Id: TLine2D.cc 10002 2007-02-26 06:56:17Z katayama $
//-----------------------------------------------------------------------------
// Filename : TLine2D.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a line in 2D.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.1  1999/11/19 09:13:08  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
//
//-----------------------------------------------------------------------------

#define TLINE2D_INLINE_DEFINE_HERE
#define HEP_SHORT_NAMES

#include "tracking/modules/trasan/ConstAList.h"
#include "tracking/modules/trasan/TPoint2D.h"
#include "tracking/modules/trasan/TLine2D.h"

namespace Belle {

  TLine2D::TLine2D() : _slope(1), _yOffset(0), _det(0), _list(0)
  {
  }

  TLine2D::TLine2D(double a, double b)
    : _slope(a),
      _yOffset(b),
      _det(0),
      _list(0)
  {
  }

  TLine2D::TLine2D(const AList<TPoint2D> & a) : _slope(1), _yOffset(0), _det(0)
  {
    _list = new CAList<TPoint2D>();
    _list->append(a);
  }

  TLine2D::~TLine2D()
  {
    if (_list) delete _list;
  }

  void
  TLine2D::append(const TPoint2D& a)
  {
    if (! _list)
      _list = new CAList<TPoint2D>();
    _list->append(a);
  }

  void
  TLine2D::remove(const TPoint2D& a)
  {
    if (! _list) return;
    _list->remove(a);
  }

  const CAList<TPoint2D> &
  TLine2D::list(void) const
  {
    if (! _list)
      _list = new CAList<TPoint2D>();
    return * _list;
  }

  int
  TLine2D::fit(void)
  {
    if (! _list) return -1;

    unsigned n = _list->length();
    if (! n) return -1;

    if (n == 2) {
      double x0 = (* _list)[0]->x();
      double y0 = (* _list)[0]->y();
      double x1 = (* _list)[1]->x();
      double y1 = (* _list)[1]->y();
      if (x0 == x1) return -2;
      _slope = (y0 - y1) / (x0 - x1);
      _yOffset = - _slope * x1 + y1;

      return 0;
    }

    double sum = double(n);
    double sumX = 0., sumY = 0., sumX2 = 0., sumXY = 0., sumY2 = 0.;
    for (unsigned i = 0; i < n; i++) {
      const TPoint2D& p = * (* _list)[i];
      double x = p.x();
      double y = p.y();
      sumX  += x;
      sumY  += y;
      sumX2 += x * x;
      sumXY += x * y;
      sumY2 += y * y;
    }

    _det = sum * sumX2 - sumX * sumX;
    if (_det == 0.) return -3;

    _slope = (sumXY * sum - sumX * sumY) / _det;
    _yOffset = (sumX2 * sumY - sumX * sumXY) / _det;

    return 0;
  }

  double
  TLine2D::distance(const TPoint2D& p) const
  {
    double ydif = p.y() - _yOffset;
    double vmag = sqrt(1. + _slope * _slope);
    double dot = (p.x() + ydif * _slope) / vmag;
    double xmag2 = p.x() * p.x() + ydif * ydif;
    return sqrt(xmag2 - dot * dot);
  }

} // namespace Belle

