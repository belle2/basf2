//-----------------------------------------------------------------------------
// $Id: TLine0.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TLine0.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a line in tracking.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.7  2003/12/25 12:03:34  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.6  2001/12/23 09:58:48  katayama
// removed Strings.h
//
// Revision 1.5  2001/12/19 02:59:46  katayama
// Uss find,istring
//
// Revision 1.4  2000/10/05 23:54:23  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.3  2000/04/11 13:05:45  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.2  2000/01/08 09:44:38  yiwasaki
// Trasan 1.66d : debug info. modified
//
// Revision 1.1  1999/11/19 09:13:08  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.18  1999/10/30 10:12:20  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.17  1999/01/11 03:03:11  yiwasaki
// Fitters added
//
// Revision 1.16  1999/01/07 05:09:57  yiwasaki
// initializers added
//
// Revision 1.15  1998/12/24 08:46:55  yiwasaki
// stereo building modified by J.Suzuki
//
// Revision 1.14  1998/09/29 01:24:24  yiwasaki
// Trasan 1.1 beta 1 relase : TBuilderCurl added
//
// Revision 1.13  1998/08/17 01:00:32  yiwasaki
// TSegmentLinker::roughSelection added, TRASAN_DEBUG option works
//
// Revision 1.12  1998/08/12 16:32:48  yiwasaki
// Trasan 1.08 release : stereo finder updated by J.Suzuki, new MC classes added by Y.Iwasaki
//
// Revision 1.11  1998/07/29 04:34:53  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.9  1998/07/06 15:48:50  yiwasaki
// Trasan 1.01 release:CurlFinder default on, bugs in TLine and TTrack::fit fixed
//
// Revision 1.7  1998/06/29 02:15:08  yiwasaki
// Trasan 1 release candidate 1
//
// Revision 1.6  1998/06/19 12:17:11  yiwasaki
// Trasan 1 beta 4.1 release, TBuilder::buildStereo updated
//
// Revision 1.5  1998/06/17 20:22:59  yiwasaki
// Trasan 1 beta 4 release again, KS effi. improved?
//
// Revision 1.4  1998/06/17 20:12:37  yiwasaki
// Trasan 1 beta 4 release, KS effi. improved?
//
// Revision 1.3  1998/06/15 09:58:15  yiwasaki
// Trasan 1 beta 3.2, bug fixed
//
// Revision 1.2  1998/06/14 11:09:50  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
// Revision 1.1  1998/06/11 08:14:09  yiwasaki
// Trasan 1 beta 1 release
//
//-----------------------------------------------------------------------------




#include "tracking/modules/trasan/TLine0.h"

#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/TrackMC.h"

namespace Belle {

  const TLineFitter
  TLine0::_fitter = TLineFitter("TLine0 Default Line Fitter");

  TLine0::TLine0()
    : TTrackBase(),
      _fittedUpdated(false),
      _a(0.),
      _b(0.),
      _det(0.),
      _chi2(0.),
      _reducedChi2(0.)
  {

    //...Set a defualt fitter...
    fitter(& TLine0::_fitter);
  }

  TLine0::TLine0(const AList<TLink> & a)
    : TTrackBase(a),
      _fittedUpdated(false),
      _a(0.),
      _b(0.),
      _det(0.),
      _chi2(0.),
      _reducedChi2(0.)
  {

    //...Set a defualt fitter...
    fitter(& TLine0::_fitter);
  }

  TLine0::~TLine0()
  {
  }

  void
  TLine0::dump(const std::string& msg, const std::string& pre) const
  {
    bool def = false;
    if (msg == "") def = true;

    if (def || msg.find("line") != std::string::npos || msg.find("detail") != std::string::npos) {
      std::cout << pre;
      std::cout << "#links=" << _links.length();
      std::cout << ",a=" << _a;
      std::cout << ",b=" << _b;
      std::cout << ",det=" << _det;
      std::cout << std::endl;
    }
    if (! def) TTrackBase::dump(msg, pre);
  }

//  int
//  TLine0::fitx(void) {
//      if (_fitted) return 0;

//      unsigned n = _links.length();
//      double sum = double(n);
//      double sumX = 0., sumY = 0., sumX2 = 0., sumXY = 0., sumY2 = 0.;
//      for (unsigned i = 0; i < n; i++) {
//    TLink & l = * _links[i];

//    double x = l.position().x();
//    double y = l.position().y();
//    sumX  += x;
//    sumY  += y;
//    sumX2 += x * x;
//    sumXY += x * y;
//    sumY2 += y * y;
//      }

//      _det = sum * sumX2 - sumX * sumX;
//  #ifdef TRASAN_DEBUG_DETAIL
// std::cout << "    TLine0::fit ... det=" << _det << std::endl;
//  #endif
//      if(_det == 0. && n != 2){
//    return -1;
//      }else if(_det == 0. && n == 2){
//    double x0 = _links[0]->position().x();
//    double y0 = _links[0]->position().y();
//    double x1 = _links[1]->position().x();
//    double y1 = _links[1]->position().y();
//    if(x0 == x1)return -1;
//    _a = (y0-y1)/(x0-x1);
//    _b = -_a*x1 + y1;

//    _fitted = true;
//    return 0;
//      }
//      _a = (sumXY * sum - sumX * sumY) / _det;
//      _b = (sumX2 * sumY - sumX * sumXY) / _det;

//      _fitted = true;
//      return 0;
//  }

  double
  TLine0::chi2(void) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    if (! _fitted) std::cout << "TLine0::chi2 !!! fit not performed" << std::endl;
#endif

    if (_fittedUpdated) return _chi2;
    _chi2 = 0.;
    unsigned n = _links.length();
    for (unsigned i = 0; i < n; i++) {
      TLink& l = * _links[i];

      double x = l.position().x();
      double y = l.position().y();
      double c = y - _a * x - _b;
      _chi2 += c * c;
    }
    _fittedUpdated = true;
    return _chi2;
  }

  void
  TLine0::refine(AList<TLink> & list, float maxSigma)
  {
    AList<TLink> bad;
    unsigned n = _links.length();
    for (unsigned i = 0; i < n; i++) {
      TLink& l = * _links[i];
      double dist = distance(l);
      if (dist > maxSigma) bad.append(l);
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    TLine0::refine ... rejected hits:max distance=" << maxSigma;
    std::cout << std::endl;
    bad.sort(TLink::sortByWireId);
    for (unsigned i = 0; i < (unsigned) bad.length(); i++) {
      TLink& l = * _links[i];
      std::cout << "        ";
      std::cout << l.wire()->layerId() << "-";
      std::cout << l.wire()->localId();
      std::cout << "(";
      if (l.hit()->mc()) {
        if (l.hit()->mc()->hep()) std::cout << l.hit()->mc()->hep()->id();
        else std::cout << "0";
      }
      std::cout << "),";
      std::cout << l.position() << "," << distance(l);
      if (distance(l) > maxSigma) std::cout << " X";
      std::cout << std::endl;
    }
#endif

    if (bad.length()) {
      _links.remove(bad);
      list.append(bad);
      _fitted = false;
      _fittedUpdated = false;
    }
  }

  int
  TLine0::fit2()
  {
    //    if (_fitted) return 0;

    unsigned n = _links.length();
    int mask[100] = {0};
    int nsl[11] = {64, 80, 96, 128, 144, 160, 192, 208, 240, 256, 288};
    int npos = 0, nneg = 0;
    for (unsigned i = 0; i < n - 1 ; i++) {
      TLink& l = * _links[i];
      for (unsigned j = i + 1; j < n  ; j++) {
        TLink& s = * _links[j];
        if (l.hit()->wire().layerId() == s.hit()->wire().layerId()) {
          //... Check 3 consective hits
          if (i > 0 && (mask[i - 1] == 1 && mask[j] == 1)) {
            TLink& t = * _links[i - 1];
            if (l.hit()->wire().layerId() == t.hit()->wire().layerId()) {
              mask[i] = 1;
            }
          }
          int ilast = nsl[l.hit()->wire().superLayerId()] - 1;
          int ilocal = l.hit()->wire().localId();
          int jlocal = s.hit()->wire().localId();
          if (ilocal > 0 && ilocal < ilast) {
            if (abs(jlocal - ilocal) > 1) {
              mask[i] = 1;
              mask[j] = 1;
            }
          } else if (ilocal == 0) {
            if (jlocal > 1 && jlocal < ilast) {
              mask[i] = 1;
              mask[j] = 1;
            }
          } else if (ilocal == ilast) {
            if (jlocal > 0 && jlocal < ilast - 1) {
              mask[i] = 1;
              mask[j] = 1;
            }
          }
        }
      }
      //...
      if (mask[i] == 0) {
        if (l.position().y() >= 0) npos += 1;
        if (l.position().y() <  0) nneg += 1;
      }
    }

    //....
    double sumX = 0., sumY = 0., sumX2 = 0., sumXY = 0., sumY2 = 0.;
    int nused = 0;
    int lyid[2];
    for (unsigned i = 0; i < n; i++) {

      if (mask[i] == 1) continue;

      TLink& l = * _links[i];

      double x = l.position().x();
      double y = l.position().y();
      if (abs(npos - nneg) > 3) {
        if (npos > nneg && y < 0) continue;
        if (npos < nneg && y > 0) continue;
      }
      sumX  += x;
      sumY  += y;
      sumX2 += x * x;
      sumXY += x * y;
      sumY2 += y * y;
      if (nused < 2) {
        lyid[nused] = l.hit()->wire().layerId();
      }
      nused += 1;
    }

    if (nused < 2 || (nused == 2 && lyid[0] == lyid[1])) {
      return -2;
    }
    double sum = double(nused);
    _det = sum * sumX2 - sumX * sumX;
    if (_det == 0.) {
      return -1;
    }
    _a = (sumXY * sum - sumX * sumY) / _det;
    _b = (sumX2 * sumY - sumX * sumXY) / _det;

    _fitted = true;
    return 0;
  }

  int
  TLine0::fit2s()
  {
    //    if (_fitted) return 0;

    unsigned n = _links.length();
    int mask[100] = {0};
    int npos = 0, nneg = 0;
    for (unsigned i = 0; i < n - 1 ; i++) {
      TLink& l = * _links[i];
      for (unsigned j = i + 1; j < n  ; j++) {
        TLink& s = * _links[j];
        if (l.hit()->wire().layerId() == s.hit()->wire().layerId()) {
          mask[i] = 1;
          mask[j] = 1;
        }
      }
      //...
      if (mask[i] == 0) {
        if (l.position().y() >= 0) npos += 1;
        if (l.position().y() <  0) nneg += 1;
      }
    }

    //....
    double sumX = 0., sumY = 0., sumX2 = 0., sumXY = 0., sumY2 = 0.;
    int nused = 0;
//  int lyid[2];
    for (unsigned i = 0; i < n; i++) {

      if (mask[i] == 1) continue;

      TLink& l = * _links[i];

      double x = l.position().x();
      double y = l.position().y();
      if (npos > nneg && y < 0) continue;
      if (npos < nneg && y > 0) continue;

      sumX  += x;
      sumY  += y;
      sumX2 += x * x;
      sumXY += x * y;
      sumY2 += y * y;
      nused += 1;
    }

    if (nused < 4) {
      return -2;
    }
    double sum = double(nused);
    _det = sum * sumX2 - sumX * sumX;
    if (_det == 0.) {
      return -1;
    }
    _a = (sumXY * sum - sumX * sumY) / _det;
    _b = (sumX2 * sumY - sumX * sumXY) / _det;

    _fitted = true;
    return 0;
  }
  int
  TLine0::fit2sp()
  {
    //    if (_fitted) return 0;

    unsigned n = _links.length();
    int mask[100] = {0};
    double phi_ave = 0.;
    int nphi = 0;
    double Crad = 180. / 3.141592;
    for (unsigned i = 0; i < n - 1 ; i++) {
      TLink& l = * _links[i];
      for (unsigned j = i + 1; j < n  ; j++) {
        TLink& s = * _links[j];
        if (l.hit()->wire().layerId() == s.hit()->wire().layerId()) {
          mask[i] = 1;
          mask[j] = 1;
        }
      }
      //...
      if (mask[i] != 1) {
        double phi = Crad * atan2(l.position().y(), l.position().x());
        phi_ave += phi;
        nphi += 1;
      }
    }

    //...
    if (mask[n - 1] != 1) {
      TLink& l = * _links[n - 1];
      double phi = Crad * atan2(l.position().y(), l.position().x());
      phi_ave += phi;
      nphi += 1;
    }
    double phi_max = 0.;
    double phi_min = 0.;
    if (nphi > 0) {
      phi_max = phi_ave / n + 40;
      phi_min = phi_ave / n - 40;
    }

    //....
    double sumX = 0., sumY = 0., sumX2 = 0., sumXY = 0., sumY2 = 0.;
    int nused = 0;
//  int lyid[2];
    for (unsigned i = 0; i < n; i++) {

      if (mask[i] == 1) continue;

      TLink& l = * _links[i];

      double x = l.position().x();
      double y = l.position().y();
      double phi = Crad * atan2(l.position().y(), l.position().x());
      if (phi > phi_max && phi < phi_min) continue;

      sumX  += x;
      sumY  += y;
      sumX2 += x * x;
      sumXY += x * y;
      sumY2 += y * y;
      nused += 1;
    }

    if (nused < 4) {
      return -2;
    }
    double sum = double(nused);
    _det = sum * sumX2 - sumX * sumX;
    if (_det == 0.) {
      return -1;
    }
    _a = (sumXY * sum - sumX * sumY) / _det;
    _b = (sumX2 * sumY - sumX * sumXY) / _det;

    _fitted = true;
    return 0;
  }

  int
  TLine0::fit2p()
  {
    //    if (_fitted) return 0;

    unsigned n = _links.length();
    int mask[100] = {0};
    int nsl[11] = {64, 80, 96, 128, 144, 160, 192, 208, 240, 256, 288};
    double phi_ave = 0.;
    int nphi = 0;
    double Crad = 180. / 3.141592;
    for (unsigned i = 0; i < n - 1 ; i++) {
      TLink& l = * _links[i];
      for (unsigned j = i + 1; j < n  ; j++) {
        TLink& s = * _links[j];
        if (l.hit()->wire().layerId() == s.hit()->wire().layerId()) {
          //... Check 3 consective hits
          if (i > 0 && (mask[i - 1] == 1 && mask[j] == 1)) {
            TLink& t = * _links[i - 1];
            if (l.hit()->wire().layerId() == t.hit()->wire().layerId()) {
              mask[i] = 1;
            }
          }
          int ilast = nsl[l.hit()->wire().superLayerId()] - 1;
          int ilocal = l.hit()->wire().localId();
          int jlocal = s.hit()->wire().localId();
          if (ilocal > 0 && ilocal < ilast) {
            if (abs(jlocal - ilocal) > 1) {
              mask[i] = 1;
              mask[j] = 1;
            }
          } else if (ilocal == 0) {
            if (jlocal > 1 && jlocal < ilast) {
              mask[i] = 1;
              mask[j] = 1;
            }
          } else if (ilocal == ilast) {
            if (jlocal > 0 && jlocal < ilast - 1) {
              mask[i] = 1;
              mask[j] = 1;
            }
          }
        }
      }
      //...
      //...
      if (mask[i] != 1) {
        double phi = Crad * atan2(l.position().y(), l.position().x());
        phi_ave += phi;
        nphi += 1;
      }
    }

    //...
    if (mask[n - 1] != 1) {
      TLink& l = * _links[n - 1];
      double phi = Crad * atan2(l.position().y(), l.position().x());
      phi_ave += phi;
      nphi += 1;
    }
    double phi_max = 0.;
    double phi_min = 0.;
    if (nphi > 0) {
      phi_max = phi_ave / n + 40;
      phi_min = phi_ave / n - 40;
    }

    //....
    double sumX = 0., sumY = 0., sumX2 = 0., sumXY = 0., sumY2 = 0.;
    int nused = 0;
    int lyid[2];
    for (unsigned i = 0; i < n; i++) {

      if (mask[i] == 1) continue;

      TLink& l = * _links[i];

      double x = l.position().x();
      double y = l.position().y();
      double phi = Crad * atan2(l.position().y(), l.position().x());
      if (phi > phi_max && phi < phi_min) continue;

      sumX  += x;
      sumY  += y;
      sumX2 += x * x;
      sumXY += x * y;
      sumY2 += y * y;
      if (nused < 2) {
        lyid[nused] = l.hit()->wire().layerId();
      }
      nused += 1;
    }

    if (nused < 2 || (nused == 2 && lyid[0] == lyid[1])) {
      return -2;
    }
    double sum = double(nused);
    _det = sum * sumX2 - sumX * sumX;
    if (_det == 0.) {
      return -1;
    }
    _a = (sumXY * sum - sumX * sumY) / _det;
    _b = (sumX2 * sumY - sumX * sumXY) / _det;

    _fitted = true;
    return 0;
  }

  void
  TLine0::removeChits()
  {

    unsigned n = _links.length();
    int nlyr[50] = {0};
    int nneg = 0, npos = 0;
    for (unsigned i = 0; i < n - 1 ; i++) {
      TLink& l = * _links[i];
      nlyr[l.hit()->wire().layerId()] += 1;
      if (l.position().y() < 0.) {
        nneg += 1;
      } else {
        npos += 1;
      }
    }

    //...
    AList<TLink> bad;
    for (unsigned i = 0; i < n; i++) {

      TLink& l = * _links[i];

      //...if # of hits in a wire layer, don't use...
      if (nlyr[l.hit()->wire().layerId()] > 3) {
        bad.append(l);
        continue;
      }
      //...remove extremely bad poinits
      if (abs(nneg - npos) > 3) {
        if (npos > nneg && l.position().y() < 0) bad.append(l);
        if (npos < nneg && l.position().y() > 0) bad.append(l);
      }
    }
    //...
    if (bad.length() > 0 && (unsigned) bad.length() < n) {
      _links.remove(bad);
    }

    //... For the next fit
    _fitted = false;
    _fittedUpdated = false;
  }

  void
  TLine0::removeSLY(AList<TLink> & list)
  {
    _links.remove(list);
    _fitted = false;
    _fittedUpdated = false;
  }

  void
  TLine0::appendSLY(AList<TLink> & list)
  {
    _links.append(list);
    _fitted = false;
    _fittedUpdated = false;
  }

  void
  TLine0::appendByszdistance(AList<TLink> & list, unsigned isl, float maxSigma)
  {

    //... intialize
    unsigned nb = _links.length();

    //....Select good hit
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      TLink& l = * list[i];
      if (l.hit()->wire().superLayerId() == isl) {
        double dist = distance(l);
        if (dist < maxSigma) {
          _links.append(l);
        }
      }
    }

    unsigned na = _links.length();
    if (nb != na) {
      AList<TLink> bad;
      //... remove duplicated hits
      for (unsigned i = 0; i < na ; i++) {
        TLink& l = * _links[i];
        if (i < na - 1) {
          TLink& lnext = * _links[i + 1];
          if (l.hit()->wire().layerId() == lnext.hit()->wire().layerId()) {
            if (l.hit()->wire().localId() == lnext.hit()->wire().localId()) {
              bad.append(l);
            }
          }
        }
      }
      if (bad.length() > 0) _links.remove(bad);
      _fitted = false;
      _fittedUpdated = false;
    }
  }

  double
  TLine0::reducedChi2(void) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    if (! _fitted) std::cout << "TLine0::reducedChi2 !!! fit not performed" << std::endl;
#endif

    if (_fittedUpdated) return _reducedChi2;
    double chi2 = 0.;
    double scale = 20.;
    unsigned n = _links.length();
    for (unsigned i = 0; i < n; i++) {
      TLink& l = * _links[i];

      double x = l.position().x();
      double y = l.position().y();
      double c = y - _a * x - _b;
      double err = scale * l.hit()->dDrift();
      chi2 += c * c / err / err;
    }

    _reducedChi2 = chi2 / (n - 2);
    _fittedUpdated = true;
    return _reducedChi2;
  }

} // namespace Belle

