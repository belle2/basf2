/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to represent a reconstructed charged track in TRGCDC.
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include "cdc/dataobjects/CDCSimHit.h"
#include "trg/trg/Debug.h"
#include "trg/trg/Constants.h"
#include "trg/cdc/TRGCDCTrack.h"
#include "trg/cdc/Circle.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/Link.h"
#include "trg/cdc/Helix.h"
#include "trg/cdc/TRGCDC.h"

using namespace std;

namespace Belle2 {

  vector<const TRGCDCTrack*>
  TRGCDCTrack::_list = vector<const TRGCDCTrack*>();

  TRGCDCTrack::TRGCDCTrack()
    : TCTBase("unknown", 0),
      _helix(ORIGIN, CLHEP::HepVector(5, 0), CLHEP::HepSymMatrix(5, 0)), m_2DFitChi2(9999), m_3DFitChi2(9999), m_debugValue(0)
  {
  }

  TRGCDCTrack::TRGCDCTrack(const TCCircle& c)
    : TCTBase((const TCTBase&) c),
      _helix(ORIGIN, CLHEP::HepVector(5, 0), CLHEP::HepSymMatrix(5, 0)), m_2DFitChi2(9999), m_3DFitChi2(9999), m_debugValue(0)
  {

    //...Basic stuff...
//    const string newName = "CopyOF" + c.name();
    name("ConvFrom" + c.name());
    charge(c.charge());

    //...Set a defualt fitter...
//    fitter(& TTrack::_fitter);

    //...Calculate helix parameters...
    CLHEP::HepVector a(5);
    a[1] = fmod(atan2(_charge * (c.center().y() - ORIGIN.y()),
                      _charge * (c.center().x() - ORIGIN.x()))
                + 4. * M_PI,
                2. * M_PI);
    a[2] = TCHelix::ConstantAlpha / c.radius();
    a[0] = (c.center().x() - ORIGIN.x()) / cos(a[1]) - c.radius();
    a[3] = 0.;
    a[4] = 0.;
    _helix.a(a);

    //...Update links...
    unsigned n = _tsAll.size();
    for (unsigned i = 0; i < n; i++)
      _tsAll[i]->track(this);

    _fitted = false;
  }

  TRGCDCTrack::~TRGCDCTrack()
  {
    if (_list.size()) {
      for (unsigned i = 0; i < _list.size(); i++)
        if (_list[i] == this)
          _list.erase(_list.begin(), _list.begin() + i);
    }
  }

  vector<const TRGCDCTrack*>
  TRGCDCTrack::list(void)
  {
    vector<const TRGCDCTrack*> t;
    t.assign(_list.begin(), _list.end());
    return t;
  }

  int
  TRGCDCTrack::approach(TRGCDCLink& link, bool doSagCorrection) const
  {

    //...Cal. dPhi to rotate...
    const Belle2::TRGCDCWire& w = * link.wire();
    double wp[3]; w.xyPosition(wp);
    double wb[3]; w.backwardPosition(wb);
    double v[3];
    v[0] = w.direction().x();
    v[1] = w.direction().y();
    v[2] = w.direction().z();

    //...Sag correction...
    if (doSagCorrection) {
      std::cout << "TTrack::approach !!! sag correction is not implemented"
                << std::endl;
//  Vector3D dir = w.direction();
//  Point3D xw(wp[0], wp[1], wp[2]);
//  Point3D wireBackwardPosition(wb[0], wb[1], wb[2]);
//  w.wirePosition(link.positionOnTrack().z(),
//           xw,
//           wireBackwardPosition,
//           dir);
//  v[0] = dir.x();
//  v[1] = dir.y();
//  v[2] = dir.z();
//  wp[0] = xw.x();
//  wp[1] = xw.y();
//  wp[2] = xw.z();
//  wb[0] = wireBackwardPosition.x();
//  wb[1] = wireBackwardPosition.y();
//  wb[2] = wireBackwardPosition.z();
    }

    //...Cal. dPhi to rotate...
    double dPhi;
    const HepGeom::Point3D<double>& xc = helix().center();
    double xt[3]; _helix.x(0., xt);
    double x0 = - xc.x();
    double y0 = - xc.y();
    double x1 = wp[0] + x0;
    double y1 = wp[1] + y0;
    x0 += xt[0];
    y0 += xt[1];
    dPhi = atan2(x0 * y1 - y0 * x1, x0 * x1 + y0 * y1);

    //...Setup...
    double kappa = _helix.kappa();
    double phi0 = _helix.phi0();

    //...Axial case...
    if (w.axial()) {
      link.positionOnTrack(_helix.x(dPhi));
      Point3D x(wp[0], wp[1], wp[2]);
      x.setZ(link.positionOnTrack().z());
      link.positionOnWire(x);
      link.dPhi(dPhi);
      return 0;
    }

#ifdef TRASAN_DEBUG
    double firstdfdphi = 0.;
    static bool first = true;
    if (first) {
//cnv   extern BelleTupleManager * BASF_Histogram;
//  BelleTupleManager * m = BASF_Histogram;
//  h_nTrial = m->histogram("TTrack::approach nTrial", 100, 0., 100.);
    }
#endif

    //...Stereo case...
    double rho = TCHelix::ConstantAlpha / kappa;
    double tanLambda = _helix.tanl();
    static CLHEP::HepVector x(3);
    double t_x[3];
    double t_dXdPhi[3];
    const double convergence = 1.0e-5;
    double l;
    unsigned nTrial = 0;
    while (nTrial < 100) {

      x = link.positionOnTrack(_helix.x(dPhi));
      t_x[0] = x[0];
      t_x[1] = x[1];
      t_x[2] = x[2];

      l = v[0] * t_x[0] + v[1] * t_x[1] + v[2] * t_x[2]
          - v[0] * wb[0] - v[1] * wb[1] - v[2] * wb[2];

      double rcosPhi = rho * cos(phi0 + dPhi);
      double rsinPhi = rho * sin(phi0 + dPhi);
      t_dXdPhi[0] =   rsinPhi;
      t_dXdPhi[1] = - rcosPhi;
      t_dXdPhi[2] = - rho * tanLambda;

      //...f = d(Distance) / d phi...
      double t_d2Xd2Phi[2];
      t_d2Xd2Phi[0] = rcosPhi;
      t_d2Xd2Phi[1] = rsinPhi;

      //...iw new...
      double n[3];
      n[0] = t_x[0] - wb[0];
      n[1] = t_x[1] - wb[1];
      n[2] = t_x[2] - wb[2];

      double a[3];
      a[0] = n[0] - l * v[0];
      a[1] = n[1] - l * v[1];
      a[2] = n[2] - l * v[2];
      double dfdphi = a[0] * t_dXdPhi[0]
                      + a[1] * t_dXdPhi[1]
                      + a[2] * t_dXdPhi[2];

#ifdef TRASAN_DEBUG
      if (nTrial == 0) {
//      break;
        firstdfdphi = dfdphi;
      }

      //...Check bad case...
      if (nTrial > 3) {
        std::cout << Tab() << "TTrack::approach:" << w.name() << " "
                  << "dfdphi(0)=" << firstdfdphi
                  << ",(" << nTrial << ")=" << dfdphi << std::endl;
      }
#endif

      //...Is it converged?...
      if (fabs(dfdphi) < convergence)
        break;

      double dv = v[0] * t_dXdPhi[0]
                  + v[1] * t_dXdPhi[1]
                  + v[2] * t_dXdPhi[2];
      double t0 = t_dXdPhi[0] * t_dXdPhi[0]
                  + t_dXdPhi[1] * t_dXdPhi[1]
                  + t_dXdPhi[2] * t_dXdPhi[2];
      double d2fd2phi = t0 - dv * dv
                        + a[0] * t_d2Xd2Phi[0]
                        + a[1] * t_d2Xd2Phi[1];
//      + a[2] * t_d2Xd2Phi[2];

      dPhi -= dfdphi / d2fd2phi;

//    std::cout<< "nTrial=" << nTrial << std::endl;
//      std::cout<< "iw f,df,dphi=" << dfdphi << "," << d2fd2phi << "," << dPhi << std::endl;

      ++nTrial;
    }

    //...Cal. positions...
    link.positionOnWire(Point3D(wb[0] + l * v[0],
                                wb[1] + l * v[1],
                                wb[2] + l * v[2]));
    link.dPhi(dPhi);

#ifdef TRASAN_DEBUG
//cnv    h_nTrial->accumulate((float) nTrial + .5);
#endif

    return nTrial;
  }

  std::vector<HepGeom::Point3D<double> >
  TRGCDCTrack::perfectPosition(void) const
  {

    //...CDC...
    const TRGCDC& cdc = * TRGCDC::getTRGCDC();

    //...Return value...
    vector<HepGeom::Point3D<double> > posv;

    //...Super layer loop...
    for (unsigned i = 0; i < cdc.nSuperLayers(); i++) {

      //...Check links to be one...
      if ((links(i).size() == 0) || (links(i).size() > 1)) {
        if (TRGDebug::level() > 1) {
          cout << TRGDebug::tab() << "TRGCDCTrack::perfectPosition !!! #links in superlayer "
               << i << " is " << links(i).size() << endl;
        }
        continue;
      }

      //...Track segment hit...
      const TCSHit* h = dynamic_cast<const TCSHit*>(links(i)[0]->hit());
      if (! h) {
        cout << "TRGCDCTrack::perfectPosition !!! hit is not a TCSHit"
             << endl;
        continue;
      }

      //...CDCSimHit...
      const CDCSimHit* s = h->simHit();
      if (! s) {
        cout << "TRGCDCTrack::perfectPosition !!! no CDCSimHit found"
             << endl;
        continue;
      }

      //...Position...
      posv.push_back(HepGeom::Point3D<double>(s->getPosTrack().x(),
                                              s->getPosTrack().y(),
                                              s->getPosTrack().z()));

      if (TRGDebug::level() > 1) {
        cout << TRGDebug::tab() << "Perfect position TSLayer " << i
             << " : " << posv.back() << endl;
      }
    }

    return posv;
  }


} // namespace Belle2
