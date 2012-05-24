//-----------------------------------------------------------------------------
// $Id: TRobustLineFitter.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TRobustLineFitter.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object to a line.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.11  2005/01/14 00:55:40  katayama
// uninitialized variable
//
// Revision 1.10  2004/04/15 05:34:10  yiwasaki
// Trasan 3.11 : trkmgr supports tracks found by other than trasan, Hough finder updates
//
// Revision 1.9  2002/02/22 06:37:43  katayama
// Use __sparc
//
// Revision 1.8  2002/02/21 23:49:16  katayama
// For -ansi and other warning flags
//
// Revision 1.7  2001/12/23 09:58:48  katayama
// removed Strings.h
//
// Revision 1.6  2001/12/19 02:59:47  katayama
// Uss find,istring
//
// Revision 1.5  2001/04/11 01:09:10  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.4  2000/02/25 00:14:25  yiwasaki
// Trasan 2.00RC8 : robust fitter bug fix
//
// Revision 1.3  2000/02/17 12:55:46  yiwasaki
// Bug fix by A.Ishikawa
//
// Revision 1.2  2000/02/09 03:27:39  yiwasaki
// Trasan 1.68l : curl and new conf stereo updated
//
// Revision 1.1  2000/02/03 06:18:03  yiwasaki
// Trasan 1.68k : slow finder in conf. finder temporary off
//
//-----------------------------------------------------------------------------
/* for copysign */

#if defined(__sparc)
#  if defined(__EXTENSIONS__)
#    include <cmath>
#  else
#    define __EXTENSIONS__
#    include <cmath>
#    undef __EXTENSIONS__
#  endif
#elif defined(__GNUC__)
#  if defined(_XOPEN_SOURCE)
#    include <cmath>
#  else
#    define _XOPEN_SOURCE
#    include <cmath>
#    undef _XOPEN_SOURCE
#  endif
#endif



#include "tracking/modules/trasan/TRobustLineFitter.h"
#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TLine.h"
#include "tracking/modules/trasan/TLink.h"

namespace Belle {

  TRobustLineFitter::TRobustLineFitter(const std::string& name)
    : TLineFitter(name), _a(0.), _b(0.), _det(0.)
  {
  }

  TRobustLineFitter::~TRobustLineFitter()
  {
  }

  int
  TRobustLineFitter::fit(TTrackBase& t) const
  {

    //...Initial guess...
    int err = TLineFitter::fit(t);
    if (err) return err;

    //...Check # of hits...
    const AList<TLink> & links = t.links();
    _n = links.length();
    if (_n < 3) return 0;

    //...Standard deviation...
    _a = TLineFitter::a();
    _b = TLineFitter::b();
    _det = TLineFitter::det();
    double chisq = 0.;
    for (unsigned i = 0; i < _n; i++) {
      const HepGeom::Point3D<double> & p = links[i]->position();
      double tmp = p.y() - (_a * p.x() + _b);
      chisq += tmp * tmp;
    }
    double siga = sqrt(chisq / _det);

    //...Decide iteration step...
    double a1 = _a;
    double f1 = rofunc(t, a1);
    double a2 = _a + copysign(3.0 * siga, f1);
    double f2 = rofunc(t, a2);

    // if initial value f2 >f1, change the search direction
    if (f1 * f2 > 0. && fabs(f2) > fabs(f1)) {
      a2 = _a - copysign(3.0 * siga, f1);
      f2 = rofunc(t, a2);
    }

    int backwardSearch = 0;
    while (f1 * f2 > 0.) {
      _a = 2.0 * a2 - a1;
      a1 = a2;
      f1 = f2;
      a2 = _a;
      f2 = rofunc(t, a2);

      if (f1 * f2 > 0. && (fabs(f2) > fabs(f1) || fabs(f2 - f1) < 0.01)) {
        backwardSearch++;
        if (backwardSearch == 2) {
          break;
        }

        double f = f2;
        a2 = a1;
        f2 = f1;
        a1 = _a;
        f1 = f;
      }
    }

    if (backwardSearch == 2) {
      // for case of  no zero cross
      //search minimun fabs(f)

      double siga1 = 0.01 * siga;
      double a21(fabs(a2 - a1));
      while (a21 > siga1) {
        _a = 0.5 * (a1 + a2);
        if (_a == a1 || _a == a2) break;
        double f = rofunc(t, _a);

        if (f * f1 < 0) {
          f1 = f;
          a1 = _a;
          backwardSearch--;
          break;
        }

        if (fabs(f) <= fabs(f1) && fabs(f) <= fabs(f2)) {
          if (fabs(f - f1) > fabs(f - f2)) {
            f1 = f;
            a1 = _a;
          } else {
            f2 = f;
            a2 = _a;
          }
        } else if (fabs(f) <= fabs(f1)) {
          f1 = f;
          a1 = _a;
        } else if (fabs(f) <= fabs(f2)) {
          f2 = f;
          a2 = _a;
        } else {
          if (fabs(f2) > fabs(f1)) {
            f1 = f;
            a1 = _a;
          } else {
            f2 = f;
            a2 = _a;
          }
        }
        if (fabs(a2 - a1) >= a21) break;
        a21 = fabs(a2 - a1);
      }
    }

    if (backwardSearch <= 1) {

      //search zero cross
      siga = 0.01 * siga;

      double a21(fabs(a2 - a1));
      while (a21 > siga) {
        _a = 0.5 * (a1 + a2);
        if (_a == a1 || _a == a2) break;
        double f = rofunc(t, _a);
        if (f* f1 >= 0.) {
          f1 = f;
          a1 = _a;
        } else {
          f2 = f;
          a2 = _a;
        }
        if (fabs(a2 - a1) >= a21) break;
        a21 = fabs(a2 - a1);
      }
    }

    _det = _det / double(_n);

    if (t.objectType() == Line)
      ((TLine&) t).property(_a, _b, _det);
    fitDone(t);
    return 0;
  }

  double
  TRobustLineFitter::rofunc(const TTrackBase& t, double a) const
  {
    double* arr;
    if (NULL == (arr = (double*) malloc(_n * sizeof(double)))) {
//      perror("$Id: TRobustLineFitter.cc 10129 2007-05-18 12:44:41Z katayama $:arr:malloc");
      exit(1);
    }
    for (unsigned i = 0; i < _n; i++)
      arr[i] = t.links()[i]->position().y()
               - a * t.links()[i]->position().x();
    if (_n & 1) {
      _b = select((_n + 1) >> 1, _n, arr);
    } else {
      unsigned j = _n >> 1;
      _b = 0.5 * (select(j, _n, arr) + select(j + 1, _n, arr));
    }

    _det = 0.;
    double sum = 0.;
    for (unsigned i = 0; i < _n; i++) {
      double x = t.links()[i]->position().x();
      double y = t.links()[i]->position().y();
      double d = y - (a * x + _b);
      _det += fabs(d);
      if (y != 0.) d /= fabs(y);
      if (fabs(d) > 1.0e-7) sum += d > 0.0 ? x : - x;
    }
    free(arr);
    return sum;
  }

#define SWAP(a,b) tmp=(a);(a)=(b);(b)=tmp;

  double
  TRobustLineFitter::select(unsigned k, unsigned n, double* arr) const
  {
    --k;
    double tmp;
    unsigned l = 0;
    unsigned ir = n - 1;
    while (1) {
      if (ir <= l + 1) {
        if (ir == l + 1 && arr[ir] < arr[l]) {
          SWAP(arr[l], arr[ir]);
        }

//        std::cout << "k = " << k << std::endl;
//        for (unsigned i = 0; i < _n; i++)
//      std::cout << i << " : " << arr[i] << std::endl;

        return arr[k];
      } else {
        unsigned mid = (l + ir) >> 1;
        SWAP(arr[mid], arr[l + 1]);
        if (arr[l + 1] > arr[ir]) {
          SWAP(arr[l + 1], arr[ir]);
        }
        if (arr[l] > arr[ir]) {
          SWAP(arr[l], arr[ir]);
        }
        if (arr[l + 1] > arr[l]) {
          SWAP(arr[l + 1], arr[l]);
        }
        unsigned i = l + 1;
        unsigned j = ir;
        double a = arr[l];
        while (1) {
//      do i++; while (arr[i] < a);
//      do j--; while (arr[j] > a);
//      while (arr[i] < a) ++i;
//      while (arr[j] > a) --j;
          while (arr[++i] < a);
          while (arr[--j] > a);
          if (j < i) break;
          SWAP(arr[i], arr[j]);
        }
        arr[l] = arr[j];
        arr[j] = a;
        if (j >= k) ir = j - 1;
        if (j <= k) l = i;
      }
    }
  }

} // namespace Belle

