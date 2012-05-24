//-----------------------------------------------------------------------------
// $Id: TLineFitter.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TLineFitter.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object to a line.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.9  2001/12/23 09:58:48  katayama
// removed Strings.h
//
// Revision 1.8  2001/12/19 02:59:46  katayama
// Uss find,istring
//
// Revision 1.7  2001/04/11 01:09:10  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.6  2000/02/03 06:18:03  yiwasaki
// Trasan 1.68k : slow finder in conf. finder temporary off
//
// Revision 1.5  2000/01/28 06:30:24  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.4  1999/10/30 10:12:20  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.3  1999/09/21 02:01:34  yiwasaki
// Trasan 1.63b release : conformal finder minor changes, TWindow added for debug
//
// Revision 1.2  1999/03/11 23:27:16  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.1  1999/01/11 03:03:13  yiwasaki
// Fitters added
//
//-----------------------------------------------------------------------------





#include "tracking/modules/trasan/TLineFitter.h"
#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TLine.h"
#include "tracking/modules/trasan/TLink.h"
#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif

namespace Belle {

  TLineFitter::TLineFitter(const std::string& name)
    : TFitter(name), _a(0.), _b(0.), _det(0.)
  {
  }

  TLineFitter::~TLineFitter()
  {
  }

  int
  TLineFitter::fit(TTrackBase& t) const
  {

    //...Already fitted ?...
    if (t.fitted()) return TFitAlreadyFitted;

    //...Check # of hits...
    if (t.links().length() < 2) return TFitErrorFewHits;

    unsigned n = t.links().length();
    if (_det == 0. && n == 2) {
      double x0 = t.links()[0]->position().x();
      double y0 = t.links()[0]->position().y();
      double x1 = t.links()[1]->position().x();
      double y1 = t.links()[1]->position().y();
      if (x0 == x1) return TFitFailed;
      _a = (y0 - y1) / (x0 - x1);
      _b = - _a * x1 + y1;
    } else {
      double sum = (double) n;
      double sumX = 0., sumY = 0., sumX2 = 0., sumXY = 0., sumY2 = 0.;
      for (unsigned i = 0; i < n; i++) {
        const HepGeom::Point3D<double> & p = t.links()[i]->position();
        double x = p.x();
        double y = p.y();
        sumX  += x;
        sumY  += y;
        sumX2 += x * x;
        sumXY += x * y;
        sumY2 += y * y;
      }

      _det = sum * sumX2 - sumX * sumX;
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "TLineFitter::fit ... det=" << _det << std::endl;
#endif
      if (_det == 0.) {
        return TFitFailed;
      } else {
        _a = (sumXY * sum - sumX * sumY) / _det;
        _b = (sumX2 * sumY - sumX * sumXY) / _det;
      }
    }

    if (t.objectType() == Line)
      ((TLine&) t).property(_a, _b, _det);
    fitDone(t);
    return 0;
  }

} // namespace Belle

