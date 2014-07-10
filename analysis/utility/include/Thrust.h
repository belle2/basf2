/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Mikihiko Nakao (KEK), Pablo Goldenzweig (KIT)           *
 *   Original module writen by M. Nakao for Belle                         *
 *   Ported to Belle II by P. Goldenzweig                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef THRUST_H
#define THRUST_H

#include <vector>
#include <map>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>

#include <analysis/VariableManager/Variables.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/utility/FuncPtr.h>

namespace Belle2 {

// ----------------------------------------------------------------------
// struct Thrust
//
// hold various quantities when more than thrust value and thrust axis
// are needed in thrustall function template.
// ----------------------------------------------------------------------
// TODO: Find out if 'axis(0,0,0)' is correct. Belle code has 'axis(0)'.
  struct Thrust {
    Thrust() : tmax(0), tmin(0), obl(0), axis(0, 0, 0), thru(0) {}
    double tmax;   // TMA
    double tmin;   // TMI
    double obl;
    TVector3 axis;
    double thru;
  };


// ----------------------------------------------------------------------
// thrust function template
//
// This function returns thrust vector, whose direction is the thrust
// axis and whose magnitude is the thrust value.
//
// example:
//   TVector3 thr = thrust(vlist.begin(), vlist.end(), Copyptr(Vector3()));
//   thr.mag(); // Thrust value
//   thr.unit(); // Thrust axis
//
// The first and second arguments are "iterator" of any container class,
// such as STL list, vector or HepAList.  The elements in the container
// can also be anything, if one can deduce a TVector3 in some way.
//
// The third argument is a name of function which deduce a TVector3
// from iterator.  If it is an external function, you can just place
// its name.  There are two help functions:
//
//   SelfFunc(Vector3())        // for "list" of Vector3 (or Vector3 *)
//   MemberFunc(&Particle::vec) // for "list" of Particle class (or Particle *)
//                              //  with a member function vec
//
// These help functions are defined in "FuncPtr.h".
// ----------------------------------------------------------------------

  template <class Iterator, class Function>
  TVector3 thrust(Iterator begin, Iterator end, Function func)
  {
    // Temporary variables
    Iterator p, q;
    TVector3 rvec, Axis;

    double sump = 0;
    for (p = begin; p != end; p++)
      sump += ((TVector3)func(*p)).Mag();

    // Thrust and thrust vectors

    double Thru = 0;
    for (p = begin; p != end; p++) {
      TVector3 rvec(func(*p));
      if (rvec.z() <= 0.0) rvec = -rvec;

      double s = rvec.Mag();
      if (s != 0.0) rvec *= (1 / s);

      for (Iterator loopcount = begin; loopcount != end; loopcount++) {
        TVector3 rprev(rvec);
        rvec = TVector3(); // clear

        for (q = begin; q != end; q++) {
          const TVector3 qvec(func(*q));
          rvec += (qvec.Dot(rprev) >= 0) ? qvec : - qvec;
        }

        for (q = begin; q != end; q++) {
          const TVector3 qvec(func(*q));
          if (qvec.Dot(rvec) * qvec.Dot(rprev) < 0) break;
        }

        if (q == end) break;
      }

      double ttmp = 0.0;
      for (q = begin; q != end; q++) {
        const TVector3 qvec = func(*q);
        ttmp += std::fabs(qvec.Dot(rvec));
      }
      ttmp /= (sump * rvec.Mag());
      rvec *= 1 / rvec.Mag();
      if (ttmp > Thru) {
        Thru = ttmp;
        Axis = rvec;
      }
    }
    Axis *= Thru;
    return Axis;
  }


// ----------------------------------------------------------------------
// thrustall function template
//
// This function returns Thrust struct, which contains thrust value,
// thrust axis, T-major, T-minor and oblateness.
//
// Thrust thr = thrust(vlist.begin(), vlist.end(), Copyptr(Vector3()));
// thr.thru; // Thrust value
// thr.axis; // Thrust axis
//
// For the description of arguments, see thrust function template.
// ----------------------------------------------------------------------

  struct thrustall_PtPhi {
    thrustall_PtPhi() {}
    thrustall_PtPhi(double a, double b, double c) : pt1(a), pt2(b), phi(c) {}
    double pt1, pt2, phi;
  };

  template <class Iterator, class Function>
  Thrust thrustall(Iterator begin, Iterator end, Function func,
                   TVector3 axis = TVector3())
  {
    Thrust t;
    TVector3 rvec;

    if (axis.Mag2() == 0)
      axis = thrust(begin, end, func);

    t.thru = axis.Mag();
    t.axis = axis * (1 / t.thru);

    if (std::fabs(t.axis.x()) < 0.9)
      rvec = TVector3(1, 0, 0);
    else
      rvec = TVector3(0, 1, 0);

    TVector3 ptu1 = rvec - (rvec.Dot(t.axis)) * t.axis;
    ptu1 *= (1 / ptu1.Mag());
    TVector3 ptu2 = ptu1.Cross(t.axis);
    // cout.form("ptu1 = %f %f %f\n", ptu1.x(), ptu1.y(), ptu1.z());
    // cout.form("ptu2 = %f %f %f\n", ptu2.x(), ptu2.y(), ptu2.z());

    // Following part should be in the different member function.
    std::vector<thrustall_PtPhi> ptlist;
    std::vector<thrustall_PtPhi>::const_iterator pt, qt;

    // make a list of pt and phi
    double sump = 0;
    Iterator p;
    for (p = begin, pt = ptlist.begin(); p != end; p++, pt++) {
      TVector3 pvec(func(*p));
      sump += pvec.Mag();
      double pt1 = pvec.Dot(ptu1);
      double pt2 = pvec.Dot(ptu2);
      double phi = (pt1 * pt1 + pt2 * pt2 < 1e-16) ? 0 : atan2(pt2, pt1);
      ptlist.push_back(thrustall_PtPhi(pt1, pt2, phi));
      // cout.form("pt1/pt2/phi = %f %f %f\n", pt1, pt2, phi);
    }

    double smx, pht;
    t.tmax = 0.0;

    for (pt = ptlist.begin(); pt != ptlist.end(); pt++) {
      double r1  = 0.0, r2 = 0.0;
      double ph1 = 20.0, ph2 = 0.0;
      double ptphi = (*pt).phi;

      for (qt = ptlist.begin(); qt != ptlist.end(); qt++) {
        double qtphi = (*qt).phi;
        if (qtphi >= ptphi && qtphi < ptphi + M_PI) {
          r1 += (*qt).pt1;
          r2 += (*qt).pt2;
          if (qtphi < ph1) ph1 = qtphi;
          if (qtphi > ph2) ph2 = qtphi;
        }
      }

      double rn  = std::sqrt(r1 * r1 + r2 * r2);
      if (rn == 0) continue;
      double phr = std::atan2(r2, r1);
      if (phr < 0 || (phr < M_PI && ptphi > M_PI))
        phr += M_2_PI;

      double phx;
      if ((phr - ph1) > M_PI_2)
        phx = ph1 + M_PI_2;
      else if ((ph2 - phr) > M_PI_2)
        phx = ph2 - M_PI_2;
      else
        phx = phr;
      // cout << "rn/phr/phx = " << rn << ", " << phr << ", " << phx << "\n";
      smx = 2 * rn * std::cos(phr - phx);
      if (t.tmax < smx) {
        t.tmax = smx;
        pht  = phx;
      }
    }
    t.tmax /= sump;

    double r1 = -std::sin(pht);  // -sin
    double r2 =  std::cos(pht);  // +cos
    smx = 0;
    for (pt = ptlist.begin(); pt != ptlist.end(); pt++) {
      smx += std::fabs(r1 * (*pt).pt1 + r2 * (*pt).pt2);
    }
    t.tmin = smx / sump;
    t.obl = t.tmax - t.tmin;

    return t;
  }

} // Belle2 namespace

#endif


